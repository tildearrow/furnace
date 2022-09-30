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

#ifndef _N163_H
#define _N163_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "../waveSynth.h"
#include "vgsound_emu/src/n163/n163.hpp"

class DivPlatformN163: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, pitch2, note;
    short ins, wave, wavePos, waveLen;
    unsigned char waveMode;
    short loadWave, loadPos, loadLen;
    unsigned char loadMode;
    bool active, insChanged, freqChanged, volumeChanged, waveChanged, waveUpdated, keyOn, keyOff, inPorta;
    signed char vol, outVol, resVol;
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
      wave(-1),
      wavePos(0),
      waveLen(0),
      waveMode(0),
      loadWave(-1),
      loadPos(0),
      loadLen(0),
      loadMode(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      volumeChanged(false),
      waveChanged(false),
      waveUpdated(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      vol(15),
      outVol(15),
      resVol(15) {}
  };
  Channel chan[8];
  DivDispatchOscBuffer* oscBuf[8];
  bool isMuted[8];
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      unsigned char mask;
      QueuedWrite(unsigned char a, unsigned char v, unsigned char m=~0): addr(a), val(v), mask(m) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char initChanMax;
  unsigned char chanMax;
  short loadWave, loadPos, loadLen;
  unsigned char loadMode;
  bool multiplex;

  n163_core n163;
  unsigned char regPool[128];
  void updateWave(int ch, int wave, int pos, int len);
  void updateWaveCh(int ch);
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
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformN163();
};

#endif
