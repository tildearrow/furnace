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
    DivOPL4MemoryInterface(unsigned size_) : parent(NULL), size(size_) {};
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
      DivMacroInt std;
      int ins, note, pitch, vol, panL, panR;
      bool key, damp, sus;
      bool keyOn, insChanged, freqChanged;
      int basePitch, pitchOffset, freq, pan;
      Channel():
        ins(-1), note(0), pitch(0), vol(0x7f), panL(7), panR(7),
        key(false), damp(false), sus(false),
        keyOn(false), insChanged(true), freqChanged(false),
        basePitch(0), pitchOffset(0), freq(0), pan(0) {
      }
    };
    Channel chan[24];
    bool isMuted[24];
    DivOPL4MemoryInterface memory;
    YMF278 chip;

    unsigned char regPool[0x300];

    friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    void tick(bool sysTick=true);
    int dispatch(DivCommand c);
    void muteChannel(int ch, bool mute);
//     void forceIns();
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int calcFreq(int basePitch);
    const char* getEffectName(unsigned char effect);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    void* getChanState(int chan);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void setFlags(unsigned int flags);
    void reset();
    void quit();
    DivPlatformOPL4() : memory(0x200000), chip(memory) {};
    ~DivPlatformOPL4() {};
};
#endif
