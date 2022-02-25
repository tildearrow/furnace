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

#ifndef _YM2610_H
#define _YM2610_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/ymfm/ymfm_opn.h"

class DivYM2610Interface: public ymfm::ymfm_interface {
  public:
    DivEngine* parent;
    int sampleBank;
    uint8_t ymfm_external_read(ymfm::access_class type, uint32_t address);
    void ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data);
    DivYM2610Interface(): parent(NULL), sampleBank(0) {}
};

class DivPlatformYM2610: public DivDispatch {
  protected:
    struct Channel {
      DivInstrumentFM state;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, note;
      unsigned char ins, psgMode, autoEnvNum, autoEnvDen;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta, furnacePCM;
      int vol, outVol;
      unsigned char pan;
      DivMacroInt std;
      Channel():
        freqH(0),
        freqL(0),
        freq(0),
        baseFreq(0),
        pitch(0),
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
        vol(0),
        outVol(15),
        pan(3) {}
    };
    Channel chan[14];
    bool isMuted[14];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    ymfm::ym2610* fm;
    ymfm::ym2610::output_data fmout;
    DivYM2610Interface iface;
    unsigned char regPool[512];
    unsigned char lastBusy;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    int dacPos;
    int dacSample;
    int ayNoiseFreq;
    unsigned char sampleBank;

    int delay;

    bool extMode;
  
    short oldWrites[512];
    short pendingWrites[512];
    unsigned char ayEnvMode;
    unsigned short ayEnvPeriod;
    short ayEnvSlideLow;
    short ayEnvSlide;

    int octave(int freq);
    int toFreq(int freq);
    double NOTE_ADPCMB(int note);
    friend void putDispatchChan(void*,int,int);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformYM2610();
};
#endif
