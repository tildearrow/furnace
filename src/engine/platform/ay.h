/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#ifndef _AY_H
#define _AY_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/ay8910.h"

class DivPlatformAY8910: public DivDispatch {
  protected:
    const unsigned char AY8914RegRemap[16]={
      0,4,1,5,2,6,9,8,11,12,13,3,7,10,14,15
    };
    inline unsigned char regRemap(unsigned char reg) { return intellivision?AY8914RegRemap[reg&0x0f]:reg&0x0f; }
    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, note, pitch;
      unsigned char ins, psgMode, autoEnvNum, autoEnvDen;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta;
      int vol, outVol;
      unsigned char pan;
      DivMacroInt std;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), note(0), pitch(0), ins(-1), psgMode(1), autoEnvNum(0), autoEnvDen(0), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), inPorta(false), vol(0), outVol(15), pan(3) {}
    };
    Channel chan[3];
    bool isMuted[3];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    ay8910_device* ay;
    unsigned char regPool[16];
    unsigned char lastBusy;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    int dacPos;
    int dacSample;
    unsigned char sampleBank;

    int delay;

    bool extMode;
    bool stereo, sunsoft, intellivision;
    bool ioPortA, ioPortB;
    unsigned char portAVal, portBVal;
  
    short oldWrites[16];
    short pendingWrites[16];
    unsigned char ayEnvMode;
    unsigned short ayEnvPeriod;
    short ayEnvSlideLow;
    short ayEnvSlide;
    short* ayBuf[3];
    size_t ayBufLen;

    void updateOutSel(bool immediate=false);

    friend void putDispatchChan(void*,int,int);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void flushWrites();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(unsigned int flags);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    bool getDCOffRequired();
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
};
#endif
