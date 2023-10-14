/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
// Rounding it up to 0x260 (608) bytes, the nearest multiple of 16.
#define ESFM_REG_POOL_SIZE 0x260

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
    Channel():
      SharedChannel<int>(0),
      freqL{0, 0, 0, 0},
      freqH{0, 0, 0, 0},
      globalPan(3),
      macroVolMul(64) {}
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

  unsigned char regPool[ESFM_REG_POOL_SIZE];
  short oldWrites[ESFM_REG_POOL_SIZE];
  short pendingWrites[ESFM_REG_POOL_SIZE];

  int octave(int freq);
  int toFreq(int freq);
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

#ifdef KVS
#undef KVS
#endif

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
  inline bool KVS(int c, int o) {
    if (c < 0 || c >= 18 || o < 0 || o >= 4) return false;

    if (chan[c].state.fm.op[o].kvs==1) return true;

    if (chan[c].state.fm.op[o].kvs==2) {
      if (chan[c].state.esfm.op[o].outLvl==7) return true;
      else if (chan[c].state.esfm.op[o].outLvl>0) {
        if (o==3) return true;
        else if ((chan[c].state.esfm.op[o].outLvl-chan[c].state.esfm.op[o+1].modIn) >= 2) {
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
    void toggleRegisterDump(bool enable);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformESFM();
};
