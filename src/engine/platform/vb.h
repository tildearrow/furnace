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

#ifndef _PLATFORM_VB_H
#define _PLATFORM_VB_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../waveSynth.h"
#include "sound/vsu.h"

class DivPlatformVB: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int antiClickPeriodCount, antiClickWavePos;
    unsigned char pan, envLow, envHigh;
    bool noise, deferredWaveUpdate, intWritten;
    signed short wave;
    DivWaveSynth ws;
    Channel():
      SharedChannel<signed char>(15),
      antiClickPeriodCount(0),
      antiClickWavePos(0),
      pan(255),
      envLow(0),
      envHigh(0),
      noise(false),
      deferredWaveUpdate(false),
      intWritten(false),
      wave(-1) {}
  };
  Channel chan[6];
  DivDispatchOscBuffer* oscBuf[6];
  bool isMuted[6];
  bool antiClickEnabled, screwThis;
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,2048> writes;
  unsigned char lastPan;

  int cycles, curChan, delay;
  int tempL;
  int tempR;
  unsigned char modulation;
  bool modType;
  bool romMode;
  signed char modTable[32];
  int coreQuality;
  VSU* vb;
  unsigned char regPool[0x600];
  void updateWave(int ch);
  void updateROMWaves();
  void writeEnv(int ch, bool upperByteToo=false);
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    int getRegisterPoolDepth();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    float getPostAmp();
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setCoreQuality(unsigned char q);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformVB();
};

#endif
