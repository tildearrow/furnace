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

#ifndef _TIA_H
#define _TIA_H

#include "../dispatch.h"
#include "sound/tia/Audio.h"

class DivPlatformTIA: public DivDispatch {
  protected:
    struct Channel: public SharedChannel<int> {
      unsigned char shape;
      unsigned char curFreq, tuneCtr, tuneFreq;
      int tuneAcc;
      Channel():
        SharedChannel<int>(15),
        shape(4),
        curFreq(0),
        tuneCtr(0),
        tuneFreq(0),
        tuneAcc(0) {}
    };
    Channel chan[2];
    DivDispatchOscBuffer* oscBuf[2];
    bool isMuted[2];
    bool softwarePitch;
    bool oldPitch;
    unsigned char mixingType;
    unsigned char chanOscCounter;
    TIA::Audio tia;
    unsigned char regPool[16];
    int tuneCounter;
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);

    unsigned char dealWithFreq(unsigned char shape, int base, int pitch);
    int dealWithFreqNew(int shape, int bp);
  
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    float getPostAmp();
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    bool getLegacyAlwaysSetVolume();
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};
#endif
