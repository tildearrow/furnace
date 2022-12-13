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

#ifndef _GB_H
#define _GB_H

#include "../dispatch.h"
#include "../waveSynth.h"
#include "sound/gb/gb.h"
#include <queue>

class DivPlatformGB: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    unsigned char duty, sweep;
    bool sweepChanged, released, softEnv, killIt;
    bool soManyHacksToMakeItDefleCompatible;
    signed short wave;
    signed char lastKill;
    unsigned char envVol, envDir, envLen, soundLen;
    unsigned short hwSeqPos;
    short hwSeqDelay;
    Channel():
      SharedChannel<signed char>(15),
      duty(0),
      sweep(0),
      sweepChanged(false),
      released(false),
      softEnv(false),
      killIt(false),
      soManyHacksToMakeItDefleCompatible(false),
      wave(-1),
      lastKill(0),
      envVol(0),
      envDir(0),
      envLen(0),
      soundLen(0),
      hwSeqPos(0),
      hwSeqDelay(0) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  bool antiClickEnabled;
  bool enoughAlready;
  unsigned char lastPan;
  DivWaveSynth ws;
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;

  int antiClickPeriodCount, antiClickWavePos;

  GB_gameboy_t* gb;
  GB_model_t model;
  unsigned char regPool[128];
  
  unsigned char procMute();
  void updateWave();  
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
    int getPortaFloor(int ch);
    bool isStereo();
    bool getDCOffRequired();
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformGB();
};

#endif
