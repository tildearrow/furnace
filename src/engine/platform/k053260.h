/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#ifndef _K053260_H
#define _K053260_H

#include "../dispatch.h"
#include <queue>
#include "vgsound_emu/src/k053260/k053260.hpp"

class DivPlatformK053260: public DivDispatch, public k053260_intf {
  struct Channel: public SharedChannel<int> {
    unsigned int audPos;
    int sample, wave;
    int panning;
    bool setPos, reverse;
    int macroVolMul;
    Channel():
      SharedChannel<int>(127),
      audPos(0),
      sample(-1),
      wave(-1),
      panning(4),
      setPos(false),
      reverse(false),
      macroVolMul(64) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  int chipType;
  unsigned char curChan;
  unsigned int* sampleOff;
  bool* sampleLoaded;

  unsigned char* sampleMem;
  size_t sampleMemLen;
  k053260_core k053260;
  DivMemoryComposition memCompo;
  unsigned char regPool[64];
  void updatePanning(unsigned char mask);

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    virtual u8 read_sample(u32 address) override;
    virtual void acquire(short** buf, size_t len) override;
    virtual int dispatch(DivCommand c) override;
    virtual void* getChanState(int chan) override;
    virtual DivMacroInt* getChanMacroInt(int ch) override;
    virtual unsigned short getPan(int chan) override;
    virtual DivDispatchOscBuffer* getOscBuffer(int chan) override;
    virtual unsigned char* getRegisterPool() override;
    virtual int getRegisterPoolSize() override;
    virtual void reset() override;
    virtual void forceIns() override;
    virtual void tick(bool sysTick=true) override;
    virtual void muteChannel(int ch, bool mute) override;
    virtual int getOutputCount() override;
    virtual void notifyInsChange(int ins) override;
    virtual void notifyWaveChange(int wave) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void setFlags(const DivConfig& flags) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual const char** getRegisterSheet() override;
    virtual const void* getSampleMem(int index = 0) override;
    virtual size_t getSampleMemCapacity(int index = 0) override;
    virtual size_t getSampleMemUsage(int index = 0) override;
    virtual bool isSampleLoaded(int index, int sample) override;
    virtual const DivMemoryComposition* getMemCompo(int index) override;
    virtual void renderSamples(int chipID) override;
    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) override;
    virtual void quit() override;
    DivPlatformK053260();
    ~DivPlatformK053260();
  private:
    void chWrite(unsigned char ch, unsigned int addr, unsigned char val);
};

#endif
