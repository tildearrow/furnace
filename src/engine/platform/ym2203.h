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

#ifndef _YM2203_H
#define _YM2203_H
#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/ymfm/ymfm_opn.h"

#include "ay.h"
#include "fmshared_OPN.h"

class DivYM2203Interface: public ymfm::ymfm_interface {

};

class DivPlatformYM2203: public DivDispatch, public DivPlatformOPNBase {
  protected:
    const unsigned short chanOffs[3]={
      0x00, 0x01, 0x02
    };

    const unsigned char konOffs[3]={
      0, 1, 2
    };

    struct Channel {
      DivInstrumentFM state;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, pitch2, portaPauseFreq, note, ins;
      unsigned char psgMode, autoEnvNum, autoEnvDen;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta, furnacePCM, hardReset;
      int vol, outVol;
      int sample;
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
        sample(-1) {}
    };
    Channel chan[6];
    DivDispatchOscBuffer* oscBuf[6];
    bool isMuted[6];
    ymfm::ym2203* fm;
    ymfm::ym2203::output_data fmout;
    DivYM2203Interface iface;
    unsigned char regPool[256];
  
    DivPlatformAY8910* ay;
    unsigned char sampleBank;

    bool extMode;
  
    short oldWrites[256];
    short pendingWrites[256];

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
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void setSkipRegisterWrites(bool val);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    void setFlags(unsigned int flags);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    DivPlatformYM2203():
      DivDispatch(),
      DivPlatformOPNBase(4720270.0, 36, 16) {}
    ~DivPlatformYM2203();
};
#endif
