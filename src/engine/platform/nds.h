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

#ifndef _NDS_H
#define _NDS_H

#include "../dispatch.h"
#include "sound/nds.hpp"

using namespace nds_sound_emu;

class DivPlatformNDS: public DivDispatch, public nds_sound_intf {
  struct Channel: public SharedChannel<int> {
    unsigned int audPos;
    int sample, wave;
    int panning, duty;
    bool setPos, pcm, busy;
    int macroVolMul;
    Channel():
      SharedChannel<int>(127),
      audPos(0),
      sample(-1),
      wave(-1),
      panning(64),
      duty(0),
      setPos(false),
      pcm(false),
      busy(false),
      macroVolMul(64) {}
  };
  Channel chan[16];
  DivDispatchOscBuffer* oscBuf[16];
  bool isMuted[16];
  bool isDSi;
  int globalVolume;
  int lastOut[2];
  unsigned int sampleOff[256];
  bool sampleLoaded[256];
  struct QueuedWrite {
    unsigned short addr;
    unsigned char size;
    unsigned int val;
    QueuedWrite(): addr(0), size(0), val(0) {}
    QueuedWrite(unsigned short a, unsigned char s, unsigned int v): addr(a), size(s), val(v) {}
  };
  FixedQueue<QueuedWrite,2048> writes;

  unsigned char* sampleMem;
  size_t sampleMemLen;
  nds_sound_t nds;
  DivMemoryComposition memCompo;
  unsigned char regPool[288];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    virtual u8 read_byte(u32 addr) override;
    virtual void write_byte(u32 addr, u8 data) override;

    virtual void acquireDirect(blip_buffer_t** bb, size_t len) override;
    virtual void postProcess(short* buf, int outIndex, size_t len, int sampleRate) override;
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
    virtual float getPostAmp() override;
    virtual int getOutputCount() override;
    virtual bool hasAcquireDirect() override;
    virtual void notifyInsChange(int ins) override;
    virtual void notifyWaveChange(int wave) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual const char** getRegisterSheet() override;
    virtual const void* getSampleMem(int index = 0) override;
    virtual size_t getSampleMemCapacity(int index = 0) override;
    virtual size_t getSampleMemUsage(int index = 0) override;
    virtual bool isSampleLoaded(int index, int sample) override;
    virtual const DivMemoryComposition* getMemCompo(int index) override;
    virtual void renderSamples(int chipID) override;
    virtual void setFlags(const DivConfig& flags) override;
    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) override;
    virtual void quit() override;
    DivPlatformNDS():
      DivDispatch(),
      nds_sound_intf(),
      nds(*this) {}
  private:
    void writeOutVol(int ch);
};

#endif
