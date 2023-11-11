/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#pragma once

#ifndef _ES5503_H
#define _ES5503_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../waveSynth.h"
#include "sound/es5503.h"

class DivPlatformES5503: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int antiClickPeriodCount, antiClickWavePos;
    int dacPeriod, dacRate, dacOut;
    unsigned int dacPos;
    int dacSample;
    unsigned char pan;
    bool noise, pcm, furnaceDac, deferredWaveUpdate;
    signed short wave;
    int macroVolMul, noiseSeek;
    unsigned int wave_pos, wave_size;
    DivWaveSynth ws;
    Channel():
      SharedChannel<signed char>(255),
      antiClickPeriodCount(0),
      antiClickWavePos(0),
      dacPeriod(0),
      dacRate(0),
      dacOut(0),
      dacPos(0),
      dacSample(-1),
      pan(255),
      noise(false),
      pcm(false),
      furnaceDac(false),
      deferredWaveUpdate(false),
      wave(-1),
      macroVolMul(255),
      noiseSeek(0),
      wave_pos(0),
      wave_size(256) {}
  };
  Channel chan[32];
  DivDispatchOscBuffer* oscBuf[32];
  bool isMuted[32];
  bool antiClickEnabled;
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,1024> writes;
  unsigned char lastPan;

  int curChan;
  unsigned char sampleBank, lfoMode, lfoSpeed;

  es5503_core es5503;
  unsigned char regPool[256];

  void updateWave(int ch);
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    //unsigned short getPan(int chan);
    //DivChannelModeHints getModeHints(int chan);
    DivSamplePos getSamplePos(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void writeSampleMemoryByte(int address, unsigned char value);
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformES5503();
};

#endif
