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

#ifndef _NES_H
#define _NES_H

#include "../dispatch.h"

#include "sound/nes_nsfplay/nes_apu.h"
#include "sound/nes_nsfplay/5e01_apu.h"
#include "../../fixedQueue.h"

class DivPlatformNES: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int prevFreq;
    unsigned char duty, sweep, envMode, len;
    bool sweepChanged, furnaceDac, setPos;
    Channel():
      SharedChannel<signed char>(15),
      prevFreq(65535),
      duty(0),
      sweep(8),
      envMode(3),
      len(0x1f),
      sweepChanged(false),
      furnaceDac(false),
      setPos(false) {}
  };
  Channel chan[5];
  DivDispatchOscBuffer* oscBuf[5];
  bool isMuted[5];
  struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      QueuedWrite(): addr(0), val(0) {}
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,128> writes;
  int dacPeriod, dacRate, dpcmPos;
  unsigned int dacPos, dacAntiClick;
  int dacSample;
  unsigned char* dpcmMem;
  size_t dpcmMemLen;
  bool sampleLoaded[256];
  unsigned char dpcmBank;
  unsigned char sampleBank;
  unsigned char writeOscBuf;
  unsigned char apuType;
  unsigned char linearCount;
  signed char nextDPCMFreq;
  signed char nextDPCMDelta;
  signed char lastDPCMFreq;
  bool dpcmMode;
  bool dpcmModeDefault;
  bool dacAntiClickOn;
  bool useNP;
  bool goingToLoop;
  bool countMode;
  bool isE;
  struct NESAPU* nes;
  xgm::NES_APU* nes1_NP;
  xgm::NES_DMC* nes2_NP;
  xgm::I5E01_APU* e1_NP;
  xgm::I5E01_DMC* e2_NP;
  unsigned char regPool[128];
  unsigned int sampleOffDPCM[256];
  DivMemoryComposition memCompo;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void doWrite(unsigned short addr, unsigned char data);
  unsigned char calcDPCMRate(int inRate);
  void acquire_puNES(short** buf, size_t len);
  void acquire_NSFPlay(short** buf, size_t len);
  void acquire_NSFPlayE(short** buf, size_t len);

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
    bool keyOffAffectsArp(int ch);
    float getPostAmp();
    unsigned char readDMC(unsigned short addr);
    void setNSFPlay(bool use);
    void set5E01(bool use);
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
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformNES();
};

#endif
