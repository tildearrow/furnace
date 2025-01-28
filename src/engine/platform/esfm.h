/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../../../extern/ESFMu/esfm.h"

// ESFM register address space technically spans 0x800 (2048) bytes,
// but we only need the first 0x254 (596) during normal use.
// Rounding it up to 0x400 bytes, the nearest power of 2.
#define ESFM_REG_POOL_SIZE 0x400

class DivPlatformESFM: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    struct {
      DivInstrumentFM fm;
      DivInstrumentESFM esfm;
    } state;
    unsigned char freqL[4], freqH[4];
    bool hardReset;
    unsigned char globalPan;
    int macroVolMul;
    struct {
      int baseNoteOverride;
      bool fixedArp;
      int arpOff;
      int pitch2;
      bool hasOpArp;
      bool hasOpPitch;
    } opsState[4];

    void handleArpFmOp(int offset=0, int o=0) {
      DivMacroInt::IntOp& m=this->std.op[o];
      if (m.ssg.had) {
        opsState[o].hasOpArp=true;

        if (m.ssg.val<0) {
          if (!(m.ssg.val&0x40000000)) {
            opsState[o].baseNoteOverride=(m.ssg.val|0x40000000)+offset;
            opsState[o].fixedArp=true;
          } else {
            opsState[o].arpOff=m.ssg.val;
            opsState[o].fixedArp=false;
          }
        } else {
          if (m.ssg.val&0x40000000) {
            opsState[o].baseNoteOverride=(m.ssg.val&(~0x40000000))+offset;
            opsState[o].fixedArp=true;
          } else {
            opsState[o].arpOff=m.ssg.val;
            opsState[o].fixedArp=false;
          }
        }
        freqChanged=true;
      }

      else
      {
        opsState[o].hasOpArp=false;
      }
    }

    void handlePitchFmOp(int o)
    {
      DivMacroInt::IntOp& m=this->std.op[o];

      if (m.dt.had) {
        opsState[o].hasOpPitch=true;

        if (m.dt.mode) {
          opsState[o].pitch2+=m.dt.val;
          CLAMP_VAR(opsState[o].pitch2,-131071,131071);
        } else {
          opsState[o].pitch2=m.dt.val;
        }
        this->freqChanged=true;
      }

      else
      {
        opsState[o].hasOpPitch=false;
      }
    }

    Channel():
      SharedChannel<int>(0),
      freqL{0, 0, 0, 0},
      freqH{0, 0, 0, 0},
      hardReset(false),
      globalPan(3),
      macroVolMul(64) {
        memset(opsState, 0, sizeof(opsState));
      }
  };
  Channel chan[18];
  DivDispatchOscBuffer* oscBuf[18];
  bool isMuted[18];
  struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
  FixedQueue<QueuedWrite,2048> writes;
  esfm_chip chip;
  bool isFast;

  unsigned char regPool[ESFM_REG_POOL_SIZE];
  short oldWrites[ESFM_REG_POOL_SIZE];
  short pendingWrites[ESFM_REG_POOL_SIZE];

  int octave(int freq, int fixedBlock);
  int toFreq(int freq, int fixedBlock);
  void commitState(int ch, DivInstrument* ins);

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  inline void rWrite(unsigned short a, short v) {
    if (!skipRegisterWrites && a<ESFM_REG_POOL_SIZE) {
      pendingWrites[a]=v;
    }
  }

  inline void immWrite(unsigned short a, unsigned char v) {
    if (!skipRegisterWrites) {
      writes.push_back(QueuedWrite(a,v));
      if (dumpWrites) {
        addWrite(a,v);
      }
    }
  }

  /**
   * ESFM doesn't have predef algorithms, so a custom KVS heuristic for auto mode is needed.
   * This is a bit too complex for a macro.
   * The heuristic for auto mode is expressed as:
   *   true for an operator o
   *   where op[o].outLvl = 7,
   *      or op[o].outLvl > 0 and o == 3 (last operator),
   *      or op[o].outLvl > 0 and (op[o].outLvl - op[o + 1].modIn) >= 2,
   *      or op[o].outLvl > 0 and op[o + 1].modIn == 0.
   */
  inline bool KVS_ES(int c, int o) {
    if (c<0 || c>=18 || o<0 || o>=4) return false;

    if (chan[c].state.fm.op[o].kvs==1) return true;

    if (chan[c].state.fm.op[o].kvs==2) {
      if (chan[c].state.esfm.op[o].outLvl==7) return true;
      else if (chan[c].state.esfm.op[o].outLvl>0) {
        if (o==3) return true;
        else if ((chan[c].state.esfm.op[o].outLvl-chan[c].state.esfm.op[o+1].modIn)>=2) {
          return true;
        }
        else if (chan[c].state.esfm.op[o+1].modIn==0) {
          return true;
        }
      }
    }
    return false;
  }

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    int getOutputCount();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    bool getLegacyAlwaysSetVolume();
    void toggleRegisterDump(bool enable);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int mapVelocity(int ch, float vel);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    void setFlags(const DivConfig& flags);
    void setFast(bool fast);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformESFM();
};
