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

#ifndef _AY_H
#define _AY_H
#include "../dispatch.h"
#include "../fixedQueue.h"
#include "sound/ay8910.h"

class DivPlatformAY8910: public DivDispatch {
  protected:
    const unsigned char AY8914RegRemap[16]={
      0,4,1,5,2,6,9,8,11,12,13,3,7,10,14,15
    };
    inline unsigned char regRemap(unsigned char reg) { return intellivision?AY8914RegRemap[reg&0x0f]:reg&0x0f; }
    struct Channel: public SharedChannel<int> {
      struct PSGMode {
        // bit 3: DAC
        // bit 2: envelope
        // bit 1: noise
        // bit 0: tone
        unsigned char val;

        unsigned char getTone() {
          return (val&8)?0:(val&1);
        }

        unsigned char getNoise() {
          return (val&8)?0:(val&2);
        }

        unsigned char getEnvelope() {
          return (val&8)?0:(val&4);
        }

        PSGMode(unsigned char v=1):
          val(v) {}
      };
      PSGMode curPSGMode;
      PSGMode nextPSGMode;

      struct DAC {
        int sample, rate, period, pos, out;
        bool furnaceDAC;

        DAC():
          sample(-1),
          rate(0),
          period(0),
          pos(0),
          out(0),
          furnaceDAC(false) {}
      } dac;

      unsigned char autoEnvNum, autoEnvDen;
      signed char konCycles;
      Channel():
        SharedChannel<int>(15),
        curPSGMode(PSGMode(0)),
        nextPSGMode(PSGMode(1)),
        dac(DAC()),
        autoEnvNum(0),
        autoEnvDen(0),
        konCycles(0) {}
    };
    Channel chan[3];
    bool isMuted[3];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    FixedQueue<QueuedWrite,128> writes;
    ay8910_device* ay;
    DivDispatchOscBuffer* oscBuf[3];
    unsigned char regPool[16];
    unsigned char lastBusy;
  
    unsigned char sampleBank;
    unsigned char stereoSep;

    int delay;

    bool extMode;
    unsigned int extClock;
    int dacRate;
    unsigned char extDiv;
    unsigned char dacRateDiv;

    bool stereo, sunsoft, intellivision, clockSel;
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

    void checkWrites();
    void updateOutSel(bool immediate=false);
  
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);
  
  public:
    void runDAC();
    void setExtClockDiv(unsigned int eclk=COLOR_NTSC, unsigned char ediv=8);
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void flushWrites();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    DivMacroInt* getChanMacroInt(int ch);
    DivSamplePos getSamplePos(int ch);
    bool getDCOffRequired();
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformAY8910(bool useExtMode=false, unsigned int eclk=COLOR_NTSC, unsigned char ediv=8, unsigned char ddiv=24):
      DivDispatch(),
      extMode(useExtMode),
      extClock(eclk),
      extDiv(ediv),
      dacRateDiv(ddiv) {}
};
#endif
