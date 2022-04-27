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

#ifndef _SU_H
#define _SU_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "sound/su.h"

class DivPlatformSoundUnit: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, note;
    int ins, cutoff, res, control;
    signed char pan;
    unsigned char duty;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, noise, pcm;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      note(0),
      ins(-1),
      cutoff(65535),
      res(0),
      control(0),
      pan(0),
      duty(63),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      noise(false),
      pcm(false),
      vol(127),
      outVol(127),
      wave(-1) {}
  };
  Channel chan[8];
  bool isMuted[8];
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char lastPan;

  int cycles, curChan, delay;
  short tempL;
  short tempR;
  unsigned char sampleBank, lfoMode, lfoSpeed;
  SoundUnit* su;
  unsigned char regPool[128];
  void writeControl(int ch);

  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void setFlags(unsigned int flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformSoundUnit();
};

#endif
