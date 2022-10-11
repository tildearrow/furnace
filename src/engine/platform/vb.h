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

#ifndef _PLATFORM_VB_H
#define _PLATFORM_VB_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "../waveSynth.h"
#include "sound/vsu.h"

class DivPlatformVB: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, pitch2, note;
    int ins;
    unsigned char pan, envLow, envHigh;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, noise, deferredWaveUpdate;
    signed char vol, outVol, wave;
    DivMacroInt std;
    DivWaveSynth ws;
    void macroInit(DivInstrument* which) {
      std.init(which);
      pitch2=0;
    }
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      pitch2(0),
      note(0),
      ins(-1),
      pan(255),
      envLow(0),
      envHigh(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      noise(false),
      deferredWaveUpdate(false),
      vol(15),
      outVol(15),
      wave(-1) {}
  };
  Channel chan[6];
  DivDispatchOscBuffer* oscBuf[6];
  bool isMuted[6];
  struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char lastPan;

  int cycles, curChan, delay;
  int tempL;
  int tempR;
  unsigned char modulation;
  bool modType;
  signed char modTable[32];
  VSU* vb;
  unsigned char regPool[0x600];
  void updateWave(int ch);
  void writeEnv(int ch, bool upperByteToo=false);
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
    float getPostAmp();
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformVB();
};

#endif
