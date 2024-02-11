/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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
    bool setPos, pcm;
    int macroVolMul;
    Channel():
      SharedChannel<int>(127),
      audPos(0),
      sample(-1),
      wave(-1),
      panning(8),
      duty(0),
      setPos(false),
      pcm(false),
      macroVolMul(64) {}
  };
  Channel chan[16];
  DivDispatchOscBuffer* oscBuf[16];
  bool isMuted[16];
  bool isDSi;
  unsigned int sampleOff[256];
  bool sampleLoaded[256];

  unsigned char* sampleMem;
  size_t sampleMemLen;
  nds_sound_t nds;
  unsigned char regPool[288];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  virtual inline u8 read_byte(u32 addr) override;
  virtual inline void write_byte(u32 addr, u8 data) override;

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    float getPostAmp();
    int getOutputCount();
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformNDS():
      DivDispatch(),
      nds_sound_intf(),
      nds(*this) {}
  private:
    void writeOutVol(int ch);
};

#endif
