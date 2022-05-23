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

#ifndef _MSM6295_H
#define _MSM6295_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/oki/msm6295.hpp"

class DivMSM6295Interface: public vgsound_emu_mem_intf {
  public:
    unsigned char* adpcmMem;
    int sampleBank;
    u8 read_byte(u32 address);
    DivMSM6295Interface(): adpcmMem(NULL), sampleBank(0) {}
};

class DivPlatformMSM6295: public DivDispatch {
  protected:
    const unsigned short chanOffs[6]={
      0x00, 0x01, 0x02, 0x100, 0x101, 0x102
    };

    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, pitch2, portaPauseFreq, note, ins;
      unsigned char psgMode, autoEnvNum, autoEnvDen;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta, furnacePCM, hardReset;
      int vol, outVol;
      int sample;
      unsigned char pan;
      DivMacroInt std;
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
        portaPauseFreq(0),
        note(0),
        ins(-1),
        psgMode(1),
        autoEnvNum(0),
        autoEnvDen(0),
        active(false),
        insChanged(true),
        freqChanged(false),
        keyOn(false),
        keyOff(false),
        portaPause(false),
        inPorta(false),
        furnacePCM(false),
        hardReset(false),
        vol(0),
        outVol(15),
        sample(-1),
        pan(3) {}
    };
    Channel chan[4];
    DivDispatchOscBuffer* oscBuf[4];
    bool isMuted[4];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    msm6295_core* msm;
    unsigned char regPool[512];
    unsigned char lastBusy;

    unsigned char* adpcmMem;
    size_t adpcmMemLen;
    DivMSM6295Interface iface;
    unsigned char sampleBank;

    int delay, updateOsc;

    bool extMode;
  
    short oldWrites[512];
    short pendingWrites[512];

    friend void putDispatchChan(void*,int,int);
  
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
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    void setFlags(unsigned int flags);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    void renderSamples();
    
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformMSM6295();
};
#endif
