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

#ifndef _MULTIPCM_H
#define _MULTIPCM_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/ymf278b/YMF278.h"

class DivYMF278MemoryInterface: public MemoryInterface {
  public:
    DivEngine* parent;
    DivYMF278MemoryInterface(unsigned size_) : parent(NULL), size(size_) {};
    byte operator[](unsigned address) const override;
    unsigned getSize() const override { return size; };
    void write(unsigned address, byte value) override {};
    void clear(byte value) override {};
  private:
    unsigned size;
};

class DivPlatformYMF278: public DivDispatch {
  protected:
    struct Param {
      int value;
      bool initialized, changed;
      Param(): value(0), initialized(false), changed(false) {};
      explicit Param(int val): value(val), initialized(false), changed(false) {};
      void set(int val, bool force = false) {
        if (value != val || force || !initialized) {
          value = val;
          initialized = true;
          changed = true;
        }
      }
    };
    struct Channel {
      struct State {
        Param ins, freq, sus, tl, tlDirect, key, damp, lfoReset, pan;
        Param lfoRate, pm, am, ar, d1r, dl, d2r, rc, rr;
      } state;
      DivMacroInt std;
      Param ins, key, note, pitch, porta, pitchOffset, vol, pan, muted;
      Param lfoRate, pm, am;
      Channel():
        vol(0x7f), pan(0x77) {
      }
    };

    virtual void writeGlobalState() = 0;
    virtual void writeChannelState(int i, Channel::State& ch) = 0;

    Param fmLevel, pcmLevel;
    int channelCount;
    Channel* chan;

    friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    virtual void generate(short& left, short& right) = 0;
    void tick(bool sysTick=true);
    int dispatch(DivCommand c);
    void muteChannel(int ch, bool mute);
    void forceIns();
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int calcFreq(int basePitch);
    const char* getEffectName(unsigned char effect);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    void* getChanState(int chan);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void reset();
    void quit();
    DivPlatformYMF278(int channels) : channelCount(channels), chan(new Channel[channels]) {}
    ~DivPlatformYMF278() { delete[] chan; };
};

class DivPlatformMultiPCM final : public DivPlatformYMF278 {
  public:
    DivPlatformMultiPCM() : DivPlatformYMF278(28), memory(0x200000), chip(memory) {};
    ~DivPlatformMultiPCM() {};
    void reset();
    void setFlags(unsigned int flags);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);

    void generate(short& left, short& right) {
      chip.generate(left, right);
    }

  protected:
    void writeGlobalState();
    void writeChannelState(int i, Channel::State& ch);

  private:
    void immWrite(int ch, int reg, unsigned char v);

    DivYMF278MemoryInterface memory;
    MultiPCM chip;

    unsigned char regPool[0x100];
};

class DivPlatformOPL4PCM final : public DivPlatformYMF278 {
  public:
    DivPlatformOPL4PCM() : DivPlatformYMF278(24), memory(0x400000), chip(memory) {};
    ~DivPlatformOPL4PCM() {};
    void reset();
    void setFlags(unsigned int flags);
    const char* getEffectName(unsigned char effect);
    YMF278& getChip();

    void generate(short& left, short& right) {
      chip.generate(left, right);
    }

  protected:
    void writeGlobalState();
    void writeChannelState(int i, Channel::State& ch);

  private:
    void immWrite(int a, unsigned char v);
    unsigned char immRead(int a);

    DivYMF278MemoryInterface memory;
    YMF278 chip;
};
#endif
