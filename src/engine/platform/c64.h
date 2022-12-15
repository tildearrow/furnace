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

#ifndef _C64_H
#define _C64_H

#include "../dispatch.h"
#include <queue>
#include "sound/c64/sid.h"
#include "sound/c64_fp/SID.h"

class DivPlatformC64: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int prevFreq, testWhen;
    unsigned char sweep, wave, attack, decay, sustain, release;
    short duty;
    bool sweepChanged, filter;
    bool resetMask, resetFilter, resetDuty, ring, sync, test;
    Channel():
      SharedChannel<signed char>(15),
      prevFreq(65535),
      testWhen(0),
      sweep(0),
      wave(0),
      attack(0),
      decay(0),
      sustain(0),
      release(0),
      duty(0),
      sweepChanged(false),
      filter(false),
      resetMask(false),
      resetFilter(false),
      resetDuty(false),
      ring(false),
      sync(false),
      test(false) {}
  };
  Channel chan[3];
  DivDispatchOscBuffer* oscBuf[3];
  bool isMuted[3];
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;

  unsigned char filtControl, filtRes, vol;
  unsigned char writeOscBuf;
  int filtCut, resetTime;
  bool isFP;

  SID sid;
  reSIDfp::SID sid_fp;
  unsigned char regPool[32];
  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void acquire_classic(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_fp(short* bufL, short* bufR, size_t start, size_t len);

  void updateFilter();
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    bool getDCOffRequired();
    bool getWantPreNote();
    float getPostAmp();
    DivMacroInt* getChanMacroInt(int ch);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void setChipModel(bool is6581);
    void setFP(bool fp);
    void quit();
    ~DivPlatformC64();
};

#endif
