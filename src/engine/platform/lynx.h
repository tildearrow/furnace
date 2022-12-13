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

#ifndef _LYNX_H
#define _LYNX_H

#include "../dispatch.h"
#include "sound/lynx/Mikey.hpp"

class DivPlatformLynx: public DivDispatch {

  struct MikeyFreqDiv {
    unsigned char clockDivider;
    unsigned char backup;

    MikeyFreqDiv(int frequency);
  };

  struct MikeyDuty {
    unsigned char int_feedback7;
    unsigned char feedback;

    MikeyDuty(int duty);
  };

  struct Channel: public SharedChannel<signed char> {
    MikeyFreqDiv fd;
    MikeyDuty duty;
    int actualNote, lfsr, sample, samplePos, sampleAccum, sampleBaseFreq, sampleFreq;
    unsigned char pan;
    bool pcm;
    int macroVolMul;
    Channel():
      SharedChannel<signed char>(127),
      fd(0),
      duty(0),
      actualNote(0),
      lfsr(-1),
      sample(-1),
      samplePos(0),
      sampleAccum(0),
      sampleBaseFreq(0),
      sampleFreq(0),
      pan(0xff),
      pcm(false),
      macroVolMul(127) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  std::unique_ptr<Lynx::Mikey> mikey;  
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
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    //int getPortaFloor(int ch);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformLynx();
};

#endif
