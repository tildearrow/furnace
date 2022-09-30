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
#include "../macroInt.h"
#include "../waveSynth.h"
#include "vgsound_emu/src/scc/scc.hpp"

class DivPlatformSCC: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, pitch2, note, ins;
    bool active, insChanged, freqChanged, freqInit, inPorta;
    signed char vol, outVol, wave;
    signed char waveROM[32] = {0}; // 4 bit PROM per channel on bubble system
    DivMacroInt std;
    DivWaveSynth ws;
    void macroInit(DivInstrument* which) {
      std.init(which);
      pitch2=0;
    }
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      pitch2(0),
      note(0),
      ins(-1),
      active(false),
      insChanged(true),
      freqChanged(false),
      freqInit(false),
      inPorta(false),
      vol(15),
      outVol(15),
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
