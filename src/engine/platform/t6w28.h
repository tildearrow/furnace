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

#ifndef _T6W28_H
#define _T6W28_H

#include "../dispatch.h"
#include <queue>
#include "sound/t6w28/T6W28_Apu.h"

class DivPlatformT6W28: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    unsigned char panL, panR, duty;
    Channel():
      SharedChannel<signed char>(15),
      panL(15),
      panR(15),
      duty(7) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  bool antiClickEnabled;
  bool easyNoise;
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char lastPan;

  int cycles, curChan, delay;
  int tempL, tempR;
  MDFN_IEN_NGP::T6W28_Apu* t6w;
  MDFN_IEN_NGP::Fake_Buffer out[4][3];
  unsigned char regPool[128];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  double NOTE_SN(int ch, int note);
  int snCalcFreq(int ch);
  
  void writeOutVol(int ch);
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
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformT6W28();
};

#endif
