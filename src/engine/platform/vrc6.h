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

#ifndef _VRC6_H
#define _VRC6_H

#include "../../fixedQueue.h"
#include "../dispatch.h"
#include "vgsound_emu/src/vrcvi/vrcvi.hpp"


class DivPlatformVRC6: public DivDispatch, public vrcvi_intf {
  struct Channel: public SharedChannel<signed char> {
    int dacPeriod, dacRate, dacOut;
    unsigned int dacPos;
    int dacSample;
    unsigned char duty;
    bool pcm, furnaceDac, setPos;
    Channel():
      SharedChannel<signed char>(15),
      dacPeriod(0),
      dacRate(0),
      dacOut(0),
      dacPos(0),
      dacSample(-1),
      duty(0),
      pcm(false),
      furnaceDac(false),
      setPos(false) {}
  };
  Channel chan[3];
  DivDispatchOscBuffer* oscBuf[3];
  bool isMuted[3];
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,64> writes;
  unsigned char sampleBank;
  vrcvi_core vrc6;
  int prevSample;
  unsigned char regPool[13];

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquireDirect(blip_buffer_t** bb, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivSamplePos getSamplePos(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool hasAcquireDirect();
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformVRC6() : vrc6(*this) {};
    ~DivPlatformVRC6();
};

#endif
