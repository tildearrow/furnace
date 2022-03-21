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
#include "../macroInt.h"
#include <queue>
#include "sound/snes/SPC_DSP.h"

class DivPlatformSNES: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch;
    unsigned int audLoc;
    unsigned short audLen;
    unsigned int audPos;
    int audSub;
    signed char audDat;
    int sample, wave;
    unsigned char ins;
    int busClock;
    int note;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, useWave, setPos;
    signed char vol, outVol;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      audLoc(0),
      audLen(0),
      audPos(0),
      audSub(0),
      audDat(0),
      sample(-1),
      wave(0),
      ins(-1),
      busClock(0),
      note(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      useWave(false),
      setPos(false),
      vol(64),
      outVol(64) {}
  };
  Channel chan[8];
  bool isMuted[8];

  unsigned char regPool[0x80];
  unsigned char aram[0x10000];
  SPC_DSP* dsp;
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
};

#endif
