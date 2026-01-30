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

#ifndef _MSM6258_H
#define _MSM6258_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "sound/oki/okim6258.h"

class DivPlatformMSM6258: public DivDispatch {
  protected:
    struct Channel: public SharedChannel<int> {
      int sample;
      unsigned char pan;
      Channel():
        SharedChannel<int>(8),
        sample(-1),
        pan(3) {}
    };
    Channel chan[1];
    DivDispatchOscBuffer* oscBuf[1];
    bool isMuted[1];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      QueuedWrite(): addr(0), val(0) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
    };
    FixedQueue<QueuedWrite,256> writes;
    okim6258_device* msm;

    unsigned char msmPan, msmDivider, rateSel, msmClock, clockSel;
    signed char msmDividerCount, msmClockCount;
    bool updateSampleFreq;
    bool variableRate;
    short msmOut;

    int delay, updateOsc, sample, samplePos;

    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);

    int calcVGMRate();
  
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    bool getLegacyAlwaysSetVolume();
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    void setFlags(const DivConfig& flags);
    const char** getRegisterSheet();

    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformMSM6258();
};
#endif
