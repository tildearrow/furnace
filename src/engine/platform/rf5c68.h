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

#ifndef _RF5C68_H
#define _RF5C68_H

#include "../dispatch.h"
#include <queue>
#include "sound/rf5c68.h"

class DivPlatformRF5C68: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned int audPos;
    int sample, wave;
    int panning;
    bool setPos;
    int macroVolMul;
    Channel():
      SharedChannel<int>(255),
      audPos(0),
      sample(-1),
      wave(-1),
      panning(255),
      setPos(false),
      macroVolMul(64) {}
  };
  Channel chan[8];
  DivDispatchOscBuffer* oscBuf[8];
  bool isMuted[8];
  int chipType;
  unsigned char curChan;
  unsigned int sampleOffRFC[256];
  bool sampleLoaded[256];

  unsigned char* sampleMem;
  size_t sampleMemLen;
  rf5c68_device rf5c68;
  unsigned char regPool[144];
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
    bool isStereo();
    void setChipModel(int type);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void setFlags(const DivConfig& flags);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
  private:
    void chWrite(unsigned char ch, unsigned int addr, unsigned char val);
};

#endif
