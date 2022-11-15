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

#ifndef _PCE_H
#define _PCE_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "../waveSynth.h"
#include "sound/pce_psg.h"

class DivPlatformPCE: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, pitch2, note, antiClickPeriodCount, antiClickWavePos;
    int dacPeriod, dacRate, dacOut;
    unsigned int dacPos;
    int dacSample, ins;
    unsigned char pan;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, noise, pcm, furnaceDac, deferredWaveUpdate;
    signed char vol, outVol, wave;
    int macroVolMul;
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
      antiClickPeriodCount(0),
      antiClickWavePos(0),
      dacPeriod(0),
      dacRate(0),
      dacOut(0),
      dacPos(0),
      dacSample(-1),
      ins(-1),
      pan(255),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      noise(false),
      pcm(false),
      furnaceDac(false),
      deferredWaveUpdate(false),
      vol(31),
      outVol(31),
      wave(-1),
      macroVolMul(31) {}
  };
  Channel chan[6];
  DivDispatchOscBuffer* oscBuf[6];
  bool isMuted[6];
  bool antiClickEnabled;
  bool updateLFO;
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char lastPan;

  int cycles, curChan, delay;
  int tempL[32];
  int tempR[32];
  unsigned char sampleBank, lfoMode, lfoSpeed;
  PCE_PSG* pce;
  unsigned char regPool[128];
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
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformPCE();
};

#endif
