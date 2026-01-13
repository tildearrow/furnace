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

#ifndef _C140_H
#define _C140_H

#include "../dispatch.h"
#include "sound/c140_c219.h"
#include "../../fixedQueue.h"

class DivPlatformC140: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned int audPos;
    int sample, wave;
    bool setPos, invert, surround, noise, volChangedL, volChangedR, writeCtrl;
    int chPanL, chPanR;
    int chVolL, chVolR;
    int macroVolMul;
    int macroPanMul;
    Channel():
      SharedChannel<int>(255),
      audPos(0),
      sample(-1),
      wave(-1),
      setPos(false),
      invert(false),
      surround(false),
      noise(false),
      volChangedL(false),
      volChangedR(false),
      writeCtrl(false),
      chPanL(255),
      chPanR(255),
      chVolL(255),
      chVolR(255),
      macroVolMul(64),
      macroPanMul(127) {}
  };
  Channel chan[24];
  DivDispatchOscBuffer* oscBuf[24];
  bool isMuted[24];
  unsigned int* sampleOff;
  bool* sampleLoaded;
  bool is219;
  int totalChans;
  unsigned char groupBank[4];
  unsigned char bankType;

  unsigned char* sampleMem;
  size_t sampleMemLen;
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    bool addrOrVal;
    QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
  };
  FixedQueue<QueuedWrite,2048> writes;
  struct c140_t c140;
  struct c219_t c219;
  DivMemoryComposition memCompo;
  unsigned char regPool[512];
  char bankLabel[4][4];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void acquire_219(short** buf, size_t len);
  void acquire_140(short** buf, size_t len);

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    void getPaired(int ch, std::vector<DivChannelPair>& ret);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    float getPostAmp();
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
    int getClockRangeMin();
    int getClockRangeMax();
    void set219(bool is_219);
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformC140();
    ~DivPlatformC140();
};

#endif
