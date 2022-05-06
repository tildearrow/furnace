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

#ifndef _OPL_H
#define _OPL_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "../../../extern/opl/opl3.h"
#include "multipcm.h"

class DivPlatformOPL: public DivDispatch {
  protected:
    struct Channel {
      DivInstrumentFM state;
      DivMacroInt std;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, pitch2, note, ins;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, furnaceDac, inPorta, fourOp;
      int vol, outVol;
      unsigned char pan;
      void macroInit(DivInstrument* which) {
        std.init(which);
        pitch2=0;
      }
      Channel():
        freqH(0),
        freqL(0),
        freq(0),
        baseFreq(0),
        pitch(0),
        pitch2(0),
        note(0),
        ins(-1),
        active(false),
        insChanged(true),
        freqChanged(false),
        keyOn(false),
        keyOff(false),
        portaPause(false),
        furnaceDac(false),
        inPorta(false),
        fourOp(false),
        vol(0),
        pan(3) {
        state.ops=2;
      }
    };
    Channel chan[20];
    DivDispatchOscBuffer* oscBuf[42];
    bool isMuted[20];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    opl3_chip fm;
    DivPlatformOPL4PCM pcm;
    const unsigned char** slotsNonDrums;
    const unsigned char** slotsDrums;
    const unsigned char** slots;
    const unsigned short* chanMap;
    const unsigned char* outChanMap;
    double chipFreqBase;
    int delay, oplType, chans, melodicChans, totalChans;
    unsigned char lastBusy;
    unsigned char drumState;
    unsigned char drumVol[5];

    unsigned char regPool[512];
  
    bool properDrums, properDrumsSys, dam, dvb;

    unsigned char lfoValue;

    bool useYMFM, update4OpMask, pretendYMU;
  
    short oldWrites[512];
    short pendingWrites[512];

    int octave(int freq);
    int toFreq(int freq);

    friend void putDispatchChan(void*,int,int);

    void acquire_nuked(short* bufL, short* bufR, size_t start, size_t len);
    //void acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void takePCMRegisterWrites();
    bool isStereo();
    void setYMFM(bool use);
    void setOPLType(int type, bool drums);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    void toggleRegisterDump(bool enable);
    void setFlags(unsigned int flags);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int getPortaFloor(int ch);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char* getEffectName(unsigned char effect);
    void setSkipRegisterWrites(bool value);
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    void renderSamples();
    void renderInstruments();
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformOPL();
};
#endif
