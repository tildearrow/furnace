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

#ifndef _GBA_MINMOD_H
#define _GBA_MINMOD_H

#include "../dispatch.h"
#include "../waveSynth.h"

class DivPlatformGBAMinMod: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned char echo;
    unsigned int audPos, wtLen;
    int sample, wave;
    bool useWave, setPos, volChangedL, volChangedR, invertL, invertR;
    int chPanL, chPanR;
    int macroVolMul;
    int macroPanMul;
    DivWaveSynth ws;
    Channel():
      SharedChannel<int>(255),
      echo(0),
      audPos(0),
      wtLen(1),
      sample(-1),
      wave(-1),
      useWave(false),
      setPos(false),
      volChangedL(false),
      volChangedR(false),
      invertL(false),
      invertR(false),
      chPanL(255),
      chPanR(255),
      macroVolMul(256),
      macroPanMul(127) {}
  };
  Channel chan[16];
  DivDispatchOscBuffer* oscBuf[16];
  bool isMuted[16];
  unsigned int* sampleOff;
  bool* sampleLoaded;
  int volScale;
  unsigned char chanMax;

  // emulator part
  unsigned int mixBufs;
  unsigned int dacDepth;
  unsigned int sampCycles;
  unsigned int sampTimer;
  unsigned int updCycles;
  unsigned int updTimer;
  unsigned int updCyclesTotal;
  unsigned int sampsRendered;
  signed char mixBuf[15*2][1024];
  unsigned char mixOut[2][4];
  short oscOut[16][4];
  int chanOut[16][2];
  size_t mixBufPage;
  size_t mixBufReadPos;
  size_t mixBufWritePos;
  size_t mixBufOffset;
  short sampL, sampR;

  signed char* sampleMem;
  size_t sampleMemLen;
  // maximum wavetable length is currently hardcoded to 256
  unsigned short regPool[16*16];
  signed char wtMem[256*16];
  DivMemoryComposition romMemCompo;
  DivMemoryComposition mixMemCompo;
  DivMemoryComposition wtMemCompo;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivSamplePos getSamplePos(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    int getRegisterPoolDepth();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool hasSoftPan(int ch);
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
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformGBAMinMod();
    ~DivPlatformGBAMinMod();

    float maxCPU;
  private:
    void updateWave(int ch);
    // emulator part
    void resetMixer();
};

#endif
