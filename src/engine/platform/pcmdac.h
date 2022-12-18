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

#ifndef _PCM_DAC_H
#define _PCM_DAC_H

#include "../dispatch.h"
#include <queue>
#include "../waveSynth.h"

class DivPlatformPCMDAC: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    bool audDir;
    unsigned int audLoc;
    unsigned short audLen;
    int audPos;
    int audSub;
    int sample, wave;
    int panL, panR;
    bool useWave, setPos;
    int envVol;
    DivWaveSynth ws;
    Channel():
      SharedChannel<int>(255),
      audDir(false),
      audLoc(0),
      audLen(0),
      audPos(0),
      audSub(0),
      sample(-1),
      wave(-1),
      panL(255),
      panR(255),
      useWave(false),
      setPos(false),
      envVol(64) {}
  };
  Channel chan[1];
  DivDispatchOscBuffer* oscBuf;
  bool isMuted;
  int outDepth;
  // valid values:
  // - 0: none
  // - 1: linear
  // - 2: cubic spline
  // - 3: sinc
  int interp;
  bool outStereo;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool isStereo();
    DivMacroInt* getChanMacroInt(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};

#endif
