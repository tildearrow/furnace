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

#ifndef _FDS_H
#define _FDS_H

#include "../dispatch.h"
#include "../waveSynth.h"

#include "sound/nes_nsfplay/nes_fds.h"

class DivPlatformFDS: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int prevFreq, modFreq;
    unsigned char duty, sweep, modDepth, modPos;
    bool sweepChanged, modOn;
    signed short wave;
    signed char modTable[32];
    Channel():
      SharedChannel<signed char>(32),
      prevFreq(65535),
      modFreq(0),
      duty(0),
      sweep(8),
      modDepth(0),
      modPos(0),
      sweepChanged(false),
      modOn(false),
      wave(-1) {
      memset(modTable,0,32);
    }
  };
  Channel chan[1];
  DivDispatchOscBuffer* oscBuf;
  bool isMuted[1];
  DivWaveSynth ws;
  unsigned char writeOscBuf;
  bool useNP;
  struct _fds* fds;
  xgm::NES_FDS* fds_NP;
  unsigned char regPool[128];

  void updateWave();
  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void doWrite(unsigned short addr, unsigned char data);
  void acquire_puNES(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_NSFPlay(short* bufL, short* bufR, size_t start, size_t len);

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
    bool keyOffAffectsArp(int ch);
    void setNSFPlay(bool use);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    float getPostAmp();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformFDS();
};

#endif
