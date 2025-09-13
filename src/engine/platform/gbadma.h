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

#ifndef _GBA_DMA_H
#define _GBA_DMA_H

#include "../dispatch.h"
#include "../waveSynth.h"

class DivPlatformGBADMA: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned int audLoc;
    unsigned short audLen;
    int audDat;
    int audPos;
    int audSub;
    int dmaCount;
    int sample, wave;
    int pan;
    bool useWave, setPos;
    int envVol;
    DivWaveSynth ws;
    Channel():
      SharedChannel<int>(2),
      audLoc(0),
      audLen(0),
      audDat(0),
      audPos(0),
      audSub(0),
      dmaCount(0),
      sample(-1),
      wave(-1),
      pan(3),
      useWave(false),
      setPos(false),
      envVol(2) {}
  };
  Channel chan[2];
  DivDispatchOscBuffer* oscBuf[2];
  bool isMuted[2];
  unsigned int* sampleOff;
  bool* sampleLoaded;
  int outDepth;

  signed char* sampleMem;
  size_t sampleMemLen;
  // maximum wavetable length is currently hardcoded to 256
  signed char wtMem[256*2];
  DivMemoryComposition romMemCompo;
  DivMemoryComposition wtMemCompo;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivSamplePos getSamplePos(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    size_t getSampleMemNum();
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformGBADMA();
    ~DivPlatformGBADMA();

  private:
    void updateWave(int ch);
};

#endif
