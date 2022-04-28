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

#ifndef _SAA_H
#define _SAA_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/saa1099.h"
#include "../../../extern/SAASound/src/SAASound.h"

enum DivSAACores {
  DIV_SAA_CORE_MAME=0,
  DIV_SAA_CORE_SAASOUND,
  DIV_SAA_CORE_E
};

class DivPlatformSAA1099: public DivDispatch {
  protected:
    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, pitch2, note, ins;
      unsigned char psgMode;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta;
      int vol, outVol;
      unsigned char pan;
      DivMacroInt std;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), pitch2(0), note(0), ins(-1), psgMode(1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), inPorta(false), vol(0), outVol(15), pan(255) {}
    };
    Channel chan[6];
    bool isMuted[6];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    DivSAACores core;
    saa1099_device saa;
    CSAASound* saa_saaSound;
    unsigned char regPool[32];
    unsigned char lastBusy;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    int dacPos;
    int dacSample;
    unsigned char sampleBank;

    int delay;

    bool extMode;
  
    short oldWrites[16];
    short pendingWrites[16];
    short* saaBuf[2];
    size_t saaBufLen;
    unsigned char saaEnv[2];
    unsigned char saaNoise[2];
    friend void putDispatchChan(void*,int,int);

    void acquire_e(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_saaSound(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_mame(short* bufL, short* bufR, size_t start, size_t len);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setCore(DivSAACores core);
    void setFlags(unsigned int flags);
    bool isStereo();
    int getPortaFloor(int ch);
    bool keyOffAffectsArp(int ch);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
};
#endif
