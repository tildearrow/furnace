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

#ifndef _SCC_H
#define _SCC_H

#include "../dispatch.h"
#include <queue>
#include "../waveSynth.h"
#include "vgsound_emu/src/scc/scc.hpp"

class DivPlatformSCC: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    bool freqInit;
    signed short wave;
    signed char waveROM[32] = {0}; // 8 bit signed waveform
    DivWaveSynth ws;
    Channel():
      SharedChannel<signed char>(15),
      freqInit(false),
      wave(-1) {}
  };
  Channel chan[5];
  DivDispatchOscBuffer* oscBuf[5];
  bool isMuted[5];
  unsigned char writeOscBuf;
  int lastUpdated34;

  scc_core* scc;
  bool isPlus;
  unsigned char regBase;
  unsigned char regPool[225];
  void updateWave(int ch);
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
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void setChipModel(bool isPlus);
    void quit();
    ~DivPlatformSCC();
};

#endif
