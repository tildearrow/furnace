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

#ifndef _DAVE_H
#define _DAVE_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "sound/dave/dave.hpp"

class DivPlatformDave: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int dacPeriod, dacRate, dacOut;
    unsigned int dacPos;
    int dacSample;
    unsigned char noiseFreq;
    unsigned char panL;
    unsigned char panR;
    unsigned char wave;
    bool writeVol, highPass, ringMod, swapCounters, lowPass, resetPhase, setPos;
    Channel():
      SharedChannel<signed char>(63),
      dacPeriod(0),
      dacRate(0),
      dacOut(0),
      dacPos(0),
      dacSample(-1),
      noiseFreq(0),
      panL(63),
      panR(63),
      wave(0),
      writeVol(false),
      highPass(false),
      ringMod(false),
      swapCounters(false),
      lowPass(false),
      resetPhase(false),
      setPos(false) {}
  };
  Channel chan[6];
  DivDispatchOscBuffer* oscBuf[6];
  bool isMuted[6];
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(9) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,512> writes;
  bool writeControl;
  bool clockDiv;

  Ep128::Dave* dave;
  unsigned char regPool[32];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    void getPaired(int ch, std::vector<DivChannelPair>& ret);
    DivChannelModeHints getModeHints(int chan);
    DivSamplePos getSamplePos(int ch);
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
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformDave();
};

#endif
