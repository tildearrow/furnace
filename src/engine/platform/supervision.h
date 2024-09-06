/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#ifndef _SUPERVISION_H
#define _SUPERVISION_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "sound/supervision.h"

class DivPlatformSupervision: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    unsigned int duty, len, pan, pcm; // pcm is channel 3 ONLY
    int sample, hasOffset; // again, for channel 3 ONLY
    bool setPos, kon, initWrite;
    Channel():
      SharedChannel<signed char>(63),
      duty(0),
      len(0x1f),
      pan(3),
      pcm(false),
      hasOffset(0),
      setPos(false),
      kon(false),
      initWrite(true) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(9) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,512> writes;

  int curChan;
  int tempL[32];
  int tempR[32];
  int coreQuality;
  unsigned char regPool[64];
  unsigned int sampleOff[256];
  unsigned int sampleLen[256];
  bool sampleLoaded[256];
  DivMemoryComposition memCompo;
  unsigned char* sampleMem;
  size_t sampleMemLen;
  unsigned char dutySwap;
  unsigned char otherFlags;
  unsigned int sampleOffset;
  unsigned char noiseReg[3];
  struct svision_t svision;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
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
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    bool getDCOffRequired();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformSupervision();
};

#endif
