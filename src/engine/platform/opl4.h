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

#ifndef _OPL4_H
#define _OPL4_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/ymf278b/YMF278.h"

class DivOPL4MemoryInterface: public MemoryInterface {
  public:
    DivEngine* parent;
    DivOPL4MemoryInterface(unsigned size_) : parent(nullptr), size(size_) {};
    byte operator[](unsigned address) const override;
    unsigned getSize() const override { return size; };
    void write(unsigned address, byte value) override {};
    void clear(byte value) override {};
  private:
    unsigned size;
};

class DivPlatformOPL4: public DivDispatch {
  protected:
    struct Channel {
//       DivInstrumentFM state;
      DivMacroInt std;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch, note;
      int ins;
      int sample;
//       bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, furnaceDac, inPorta, fourOp;
      bool active, insChanged, freqChanged, keyOn, keyOff;
      int vol;  //, outVol;
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
//         portaPause(false),
//         furnaceDac(false),
//         inPorta(false),
//         fourOp(false),
        vol(0x7f),
//         outVol(0x7f),
        pan(3) {
//         state.ops=2;
      }
    };
    Channel chan[24];
    bool isMuted[24];
    DivOPL4MemoryInterface rom;
    DivOPL4MemoryInterface ram;
    YMF278 chip;

    unsigned char regPool[0x300];

    int toOctave(int freq);
    int toFreq(int freq);

    friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    double calcBaseFreq(int ch, int note);
    int dispatch(DivCommand c);
//     void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
//     void forceIns();
    void tick(bool sysTick=true);
//     void muteChannel(int ch, bool mute);
    bool isStereo();
//     bool keyOffAffectsArp(int ch);
    // bool keyOffAffectsPorta(int ch);
    void toggleRegisterDump(bool enable);
    void setFlags(unsigned int flags);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
//     int getPortaFloor(int ch);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
//     const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
//     void quit();
    DivPlatformOPL4();
    ~DivPlatformOPL4();
};
#endif
