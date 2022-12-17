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

#ifndef _QSOUND_H
#define _QSOUND_H

#include "../dispatch.h"
#include <queue>
#include "sound/qsound.h"

class DivPlatformQSound: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    int resVol;
    int sample, wave;
    int panning;
    int echo;
    bool useWave, surround, isNewQSound;
    Channel():
      SharedChannel<int>(255),
      resVol(4095),
      sample(-1),
      wave(-1),
      panning(0x10),
      echo(0),
      useWave(false),
      surround(true),
      isNewQSound(false) {}
  };
  Channel chan[19];
  DivDispatchOscBuffer* oscBuf[19];
  int echoDelay;
  int echoFeedback;

  unsigned char* sampleMem;
  size_t sampleMemLen;
  size_t sampleMemLenBS;
  bool sampleLoaded[256];
  bool sampleLoadedBS[256];
  struct qsound_chip chip;
  unsigned short regPool[512];

  unsigned int offPCM[256];
  unsigned int offBS[256];

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
    int getRegisterPoolDepth();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index = 0);
    const char* getSampleMemName(int index=0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};

#endif
