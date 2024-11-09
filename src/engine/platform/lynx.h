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
    int val;

    MikeyDuty(int duty);
  };

  struct Channel: public SharedChannel<signed char> {
    MikeyFreqDiv fd;
    MikeyDuty duty;
    int actualNote, lfsr, sample, samplePos, sampleAccum, sampleBaseFreq, sampleFreq;
    unsigned char pan;
    bool pcm, setPos, updateLFSR;
    int macroVolMul;
    Channel():
      SharedChannel<signed char>(127),
      fd(0),
      duty(0),
      actualNote(0),
      lfsr(0),
      sample(-1),
      samplePos(0),
      sampleAccum(0),
      sampleBaseFreq(0),
      sampleFreq(0),
      pan(0xff),
      pcm(false),
      setPos(false),
      updateLFSR(false),
      macroVolMul(127) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  bool tuned;
  std::unique_ptr<Lynx::Mikey> mikey;  
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(9) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,512> writes;
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void processDAC(int sRate);
  public:
    void acquire(short** buf, size_t len);
    void fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
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
    bool keyOffAffectsPorta(int ch);
    bool getLegacyAlwaysSetVolume();
    //int getPortaFloor(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformLynx();
};

#endif
