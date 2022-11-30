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

#ifndef _NES_H
#define _NES_H

#include "../dispatch.h"
#include "../macroInt.h"

#include "sound/nes_nsfplay/nes_apu.h"

class DivPlatformNES: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, pitch2, prevFreq, note, ins;
    unsigned char duty, sweep, envMode, len;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta, furnaceDac;
    signed char vol, outVol, wave;
    DivMacroInt std;
    void macroInit(DivInstrument* which) {
      std.init(which);
      pitch2=0;
    }
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      pitch2(0),
      prevFreq(65535),
      note(0),
      ins(-1),
      duty(0),
      sweep(8),
      envMode(3),
      len(0x1f),
      active(false),
      insChanged(true),
      freqChanged(false),
      sweepChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      furnaceDac(false),
      vol(15),
      outVol(15),
      wave(-1) {}
  };
  Channel chan[5];
  DivDispatchOscBuffer* oscBuf[5];
  bool isMuted[5];
  int dacPeriod, dacRate;
  unsigned int dacPos, dacAntiClick;
  int dacSample;
  unsigned char* dpcmMem;
  size_t dpcmMemLen;
  bool sampleLoaded[256];
  unsigned char dpcmBank;
  unsigned char sampleBank;
  unsigned char writeOscBuf;
  unsigned char apuType;
  bool dpcmMode;
  bool dacAntiClickOn;
  bool useNP;
  bool goingToLoop;
  bool countMode;
  struct NESAPU* nes;
  xgm::NES_APU* nes1_NP;
  xgm::NES_DMC* nes2_NP;
  unsigned char regPool[128];
  unsigned int sampleOffDPCM[256];

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void doWrite(unsigned short addr, unsigned char data);
  unsigned char calcDPCMRate(int inRate);
  void acquire_puNES(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_NSFPlay(short* bufL, short* bufR, size_t start, size_t len);

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
    bool keyOffAffectsArp(int ch);
    float getPostAmp();
    unsigned char readDMC(unsigned short addr);
    void setNSFPlay(bool use);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformNES();
};

#endif
