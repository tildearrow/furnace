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

#ifndef _PCSPKR_H
#define _PCSPKR_H

#include "../dispatch.h"
#include "../macroInt.h"

class DivPlatformPCSpeaker: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, note;
    unsigned char ins, duty, sweep;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta, furnaceDac;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      note(0),
      ins(-1),
      duty(0),
      sweep(8),
      active(false),
      insChanged(true),
      freqChanged(false),
      sweepChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      furnaceDac(false),
      vol(15),
      outVol(15),
      wave(-1) {}
  };
  Channel chan[1];
  bool isMuted[1];
  bool on, flip, lastOn;
  int pos, speakerType, beepFD;
  float low, band;
  float low2, high2, band2;
  float low3, band3;
  unsigned short freq, lastFreq;
  unsigned char regPool[2];

  friend void putDispatchChan(void*,int,int);

  void beepFreq(int freq);

  void acquire_unfilt(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_cone(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_piezo(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_real(short* bufL, short* bufR, size_t start, size_t len);

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
    bool keyOffAffectsArp(int ch);
    void setFlags(unsigned int flags);
    void notifyInsDeletion(void* ins);
    void notifyPlaybackStop();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const char* getEffectName(unsigned char effect);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformPCSpeaker();
};

#endif
