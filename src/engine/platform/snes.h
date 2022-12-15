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

#ifndef _SNES_H
#define _SNES_H

#include "../dispatch.h"
#include "../waveSynth.h"
#include <queue>
#include "sound/snes/SPC_DSP.h"

class DivPlatformSNES: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned int audPos;
    int sample, wave;
    int panL, panR;
    bool useWave, setPos, noise, echo, pitchMod, invertL, invertR, shallWriteVol, shallWriteEnv;
    int wtLen;
    DivInstrumentSNES state;
    DivWaveSynth ws;
    Channel():
      SharedChannel<int>(127),
      audPos(0),
      sample(-1),
      wave(-1),
      panL(127),
      panR(127),
      useWave(false),
      setPos(false),
      noise(false),
      echo(false),
      pitchMod(false),
      invertL(false),
      invertR(false),
      shallWriteVol(false),
      shallWriteEnv(false),
      wtLen(16) {} 
  };
  Channel chan[8];
  DivDispatchOscBuffer* oscBuf[8];
  bool isMuted[8];
  int globalVolL, globalVolR;
  unsigned char noiseFreq;
  signed char delay;
  signed char echoVolL, echoVolR, echoFeedback;
  signed char echoFIR[8];
  unsigned char echoDelay;
  size_t sampleTableBase;
  bool writeControl;
  bool writeNoise;
  bool writePitchMod;
  bool writeEcho;
  bool echoOn;

  bool initEchoOn;
  signed char initEchoVolL;
  signed char initEchoVolR;
  signed char initEchoFeedback;
  signed char initEchoFIR[8];
  unsigned char initEchoDelay;
  unsigned char initEchoMask;

  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;

  signed char sampleMem[65536];
  signed char copyOfSampleMem[65536];
  size_t sampleMemLen;
  unsigned int sampleOff[256];
  bool sampleLoaded[256];
  unsigned char regPool[0x80];
  SPC_DSP dsp;
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
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
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
    void updateWave(int ch);
    void writeOutVol(int ch);
    void writeEnv(int ch);
    void initEcho();
};

#endif
