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

#ifndef _OPLL_H
#define _OPLL_H

#include "../dispatch.h"
#include "../../fixedQueue.h"

extern "C" {
#include "../../../extern/Nuked-OPLL/opll.h"
}
#include "../../../extern/emu2413/emu2413.h"

class DivPlatformOPLL: public DivDispatch {
  protected:
    struct Channel: public SharedChannel<int> {
      DivInstrumentFM state;
      unsigned char freqH, freqL;
      int fixedFreq;
      unsigned char pan;
      Channel():
        SharedChannel<int>(0),
        freqH(0),
        freqL(0),
        fixedFreq(0),
        pan(3) {}
    };
    Channel chan[11];
    bool isMuted[11];
    DivDispatchOscBuffer* oscBuf[11];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    FixedQueue<QueuedWrite,512> writes;
    opll_t fm;
    OPLL* fm_emu;
    int delay, lastCustomMemory;
    unsigned char lastBusy;
    unsigned char drumState;
    unsigned char drumVol[5];
    bool drumActivated[5];
    
    // -1: undefined
    // 0: snare/tom
    // 1: hi-hat/top
    signed char lastFreqSH, lastFreqTT;

    unsigned char regPool[256];

    unsigned char selCore;
    bool crapDrums;
    bool properDrums, properDrumsSys, noTopHatFreq, fixedAll;
    bool vrc7;

    unsigned char patchSet;
  
    short oldWrites[256];
    short pendingWrites[256];

    int octave(int freq, int fixedBlock);
    int toFreq(int freq, int fixedBlock);
    void commitState(int ch, DivInstrument* ins);
    void switchMode(bool mode);

    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);

    void acquire_nuked(short** buf, size_t len);
    void acquire_ymfm(short** buf, size_t len);
    void acquire_emu(short** buf, size_t len);
  
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    int mapVelocity(int ch, float vel);
    float getGain(int ch, int vol);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setCore(unsigned char which);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    bool getLegacyAlwaysSetVolume();
    float getPostAmp();
    void toggleRegisterDump(bool enable);
    void setVRC7(bool vrc);
    void setProperDrums(bool pd);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int getPortaFloor(int ch);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformOPLL();
};
#endif
