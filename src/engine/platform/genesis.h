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

#ifndef _GENESIS_H
#define _GENESIS_H
#include "../dispatch.h"
#include <deque>
#include "../../../extern/Nuked-OPN2/ym3438.h"
#include "sound/ymfm/ymfm_opn.h"

#include "sms.h"

class DivYM2612Interface: public ymfm::ymfm_interface {

};

class DivPlatformGenesis: public DivDispatch {
  protected:
    struct Channel {
      DivInstrumentFM state;
      DivMacroInt std;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, note;
      unsigned char ins;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, furnaceDac, inPorta, hardReset;
      int vol, outVol;
      unsigned char pan;
      Channel():
        freqH(0),
        freqL(0),
        freq(0),
        baseFreq(0),
        pitch(0),
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
        hardReset(false),
        vol(0),
        pan(3) {}
    };
    Channel chan[10];
    bool isMuted[10];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::deque<QueuedWrite> writes;
    ym3438_t fm;
    int delay;
    unsigned char lastBusy;

    ymfm::ym2612* fm_ymfm;
    ymfm::ym2612::output_data out_ymfm;
    DivYM2612Interface iface;
    unsigned char regPool[512];
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    unsigned int dacPos;
    int dacSample;
    unsigned char sampleBank;
    unsigned char lfoValue;

    bool extMode, useYMFM;
    bool ladder;
  
    short oldWrites[512];
    short pendingWrites[512];

    int octave(int freq);
    int toFreq(int freq);

    friend void putDispatchChan(void*,int,int);

    void acquire_nuked(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len);
  
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
    bool isStereo();
    void setYMFM(bool use);
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
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformGenesis();
};
#endif
