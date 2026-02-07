/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
extern "C" {
#include "sound/sgu.h"
}

class DivPlatformSGU: public DivDispatch {
  struct Channel: public SharedChannel<int8_t> {
    struct {
      DivInstrumentFM fm;
      DivInstrumentESFM esfm;
    } state;
    int cutoff, baseCutoff, res, control, hasOffset, sample;
    signed char pan;
    unsigned char duty;
    unsigned char wpar[SGU_OP_PER_CH];
    bool sync[SGU_OP_PER_CH];
    bool ring[SGU_OP_PER_CH];
    struct {
      int baseNoteOverride;
      bool fixedArp;
      int arpOff;
      int pitch2;
      bool hasOpArp;
      bool hasOpPitch;
    } opsState[SGU_OP_PER_CH];

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
      } else {
        opsState[o].hasOpArp=false;
      }
    }

    void handlePitchFmOp(int o) {
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
      } else {
        opsState[o].hasOpPitch=false;
      }
    }

    bool gate, pcm, phaseReset, filterPhaseReset;
    bool pcmLoop, timerSync, freqSweep, volSweep, cutSweep, released;
    unsigned short freqSweepP, volSweepP, cutSweepP;
    unsigned char freqSweepB, volSweepB, cutSweepB;
    unsigned char freqSweepV, volSweepV, cutSweepV;
    unsigned short syncTimer;
    unsigned short hwSeqPos;
    short hwSeqDelay;
    short cutoff_slide;
    short pw_slide;
    short virtual_duty;
    Channel():
      SharedChannel<int8_t>(0),
      cutoff(0x3fff),
      baseCutoff(0x3ffc),
      res(0),
      control(0),
      hasOffset(0),
      sample(-1),
      pan(0),
      duty(63),
      gate(false),
      pcm(false),
      phaseReset(false),
      filterPhaseReset(false),
      pcmLoop(false),
      timerSync(false),
      freqSweep(false),
      volSweep(false),
      cutSweep(false),
      released(false),
      freqSweepP(0),
      volSweepP(0),
      cutSweepP(0),
      freqSweepB(0),
      volSweepB(0),
      cutSweepB(0),
      freqSweepV(0),
      volSweepV(0),
      cutSweepV(0),
      syncTimer(0),
      hwSeqPos(0),
      hwSeqDelay(0),
      cutoff_slide(0),
      pw_slide(0),
      virtual_duty(0) {
        memset(opsState,0,sizeof(opsState));
        for (int i=0; i<SGU_OP_PER_CH; i++) {
          wpar[i]=0;
          sync[i]=false;
          ring[i]=false;
        }
      }
  };
  Channel chan[SGU_CHNS];
  DivDispatchOscBuffer* oscBuf[SGU_CHNS];
  bool isMuted[SGU_CHNS];
  struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
  FixedQueue<QueuedWrite,2048> writes;
  SGU chip;
  short oldOut[2];

  // Sample memory tracking (from SoundUnit)
  unsigned int* sampleOffSGU;
  bool* sampleLoaded;
  int sysIDCache;
  DivMemoryComposition memCompo;

  void writeControl(int ch);
  void writeControlUpper(int ch);
  void applyOpRegs(int ch, int o, const DivInstrumentFM::Operator& op, const DivInstrumentESFM::Operator& opE);
  void commitState(int ch, DivInstrument* ins);

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short** buf, size_t len);
    void acquireDirect(blip_buffer_t** bb, size_t len);
    int dispatch(DivCommand c);
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    bool hasAcquireDirect();
    bool hasSoftPan(int ch);
    void reset();
    void forceIns();
    int getOutputCount();
    int mapVelocity(int ch, float vel);
    void* getChanState(int chan);
    unsigned short getPan(int ch);
    void setFlags(const DivConfig& flags);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void toggleRegisterDump(bool enable);
    bool getLegacyAlwaysSetVolume();
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int sysID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformSGU();
    ~DivPlatformSGU();
};
