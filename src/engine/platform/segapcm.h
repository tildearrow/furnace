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

#ifndef _SEGAPCM_H
#define _SEGAPCM_H
#include "../dispatch.h"
#include "../instrument.h"
#include <queue>
#include "../macroInt.h"

class DivPlatformSegaPCM: public DivDispatch {
  protected:
    struct Channel {
      DivMacroInt std;
      int freq, baseFreq, pitch, pitch2, note, ins;
      bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, portaPause, furnacePCM, isNewSegaPCM;
      int vol, outVol;
      unsigned char chVolL, chVolR;
      unsigned char chPanL, chPanR;
      int macroVolMul;

      struct PCMChannel {
        int sample;
        unsigned int pos; // <<8
        unsigned short len;
        unsigned char freq;
        PCMChannel(): sample(-1), pos(0), len(0), freq(0) {}
      } pcm;
      void macroInit(DivInstrument* which) {
        std.init(which);
        pitch2=0;
      }
      Channel():
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
        inPorta(false),
        portaPause(false),
        furnacePCM(false),
        isNewSegaPCM(false),
        vol(0),
        outVol(0),
        chVolL(127),
        chVolR(127),
        chPanL(127),
        chPanR(127),
        macroVolMul(64),
        pcm(PCMChannel()) {}
    };
    Channel chan[16];
    DivDispatchOscBuffer* oscBuf[16];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    int delay;
    int pcmL, pcmR, pcmCycles;
    unsigned char sampleBank;
    unsigned char lastBusy;

    unsigned char regPool[256];

    bool isMuted[16];
  
    short oldWrites[256];
    short pendingWrites[256];

    unsigned int sampleOffSegaPCM[256];
  
    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
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
    void notifyInsChange(int ins);
    void renderSamples();
    void setFlags(const DivConfig& flags);
    bool isStereo();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformSegaPCM();
};
#endif
