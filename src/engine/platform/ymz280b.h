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

#ifndef _YMZ280B_H
#define _YMZ280B_H

#include "../dispatch.h"
#include <queue>
#include "sound/ymz280b.h"

class DivPlatformYMZ280B: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned int audPos;
    int sample, wave;
    int panning;
    bool setPos, isNewYMZ;
    int macroVolMul;
    Channel():
      SharedChannel<int>(255),
      audPos(0),
      sample(-1),
      wave(-1),
      panning(8),
      setPos(false),
      isNewYMZ(false),
      macroVolMul(64) {}
  };
  Channel chan[8];
  DivDispatchOscBuffer* oscBuf[8];
  bool isMuted[8];
  int chipType;
  unsigned int sampleOff[256];
  bool sampleLoaded[256];

  unsigned char* sampleMem;
  size_t sampleMemLen;
  ymz280b_device ymz280b;
  unsigned char regPool[256];
  friend void putDispatchChip(void*,int);
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
    float getPostAmp();
    bool isStereo();
    void setChipModel(int type);
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
  private:
    void writeOutVol(int ch);
};

#endif
