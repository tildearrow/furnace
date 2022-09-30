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

#ifndef _AMIGA_H
#define _AMIGA_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "../waveSynth.h"

class DivPlatformAmiga: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, pitch2;
    unsigned int audLoc;
    unsigned short audLen;
    unsigned int audPos;
    int audSub;
    signed char audDat;
    int sample, wave;
    int ins;
    int busClock;
    int note;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, useWave, setPos, useV, useP;
    signed char vol, outVol;
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
      audLoc(0),
      audLen(0),
      audPos(0),
      audSub(0),
      audDat(0),
      sample(-1),
      wave(-1),
      ins(-1),
      busClock(0),
      note(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      useWave(false),
      setPos(false),
      useV(false),
      useP(false),
      vol(64),
      outVol(64) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  bool bypassLimits;
  bool amigaModel;
  bool filterOn;

  int filter[2][4];
  int filtConst;
  int filtConstOff, filtConstOn;

  int sep1, sep2;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    DivMacroInt* getChanMacroInt(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};

#endif
