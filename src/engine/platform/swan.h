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

#ifndef _SWAN_H
#define _SWAN_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/swan.h"
#include <queue>

class DivPlatformSwan: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, note;
    unsigned char ins, pan;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta;
    int vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      note(0),
      ins(-1),
      pan(255),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      vol(15),
      outVol(15),
      wave(-1) {}
  };
  Channel chan[4];
  bool isMuted[4];
  bool pcm, sweep, furnaceDac;
  unsigned char sampleBank, noise;
  int dacPeriod, dacRate;
  unsigned int dacPos;
  int dacSample;

  unsigned char regPool[0x80];
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  WSwan* ws;
  void updateWave(int ch);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    bool isStereo();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformSwan();
  private:
    void calcAndWriteOutVol(int ch, int env);
    void writeOutVol(int ch);
};

#endif
