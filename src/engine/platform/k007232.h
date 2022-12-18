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

#ifndef _K007232_H
#define _K007232_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "vgsound_emu/src/k007232/k007232.hpp"

class DivPlatformK007232: public DivDispatch, public k007232_intf {
  struct Channel: public SharedChannel<int> {
    int prevFreq;
    unsigned int audPos;
    int prevBank;
    int sample;
    int panning, prevPan;
    bool volumeChanged, setPos;
    int resVol, lvol, rvol;
    int macroVolMul;
    Channel():
      SharedChannel<int>(15),
      prevFreq(-1),
      audPos(0),
      prevBank(-1),
      sample(-1),
      panning(255),
      prevPan(-1),
      volumeChanged(false),
      setPos(false),
      resVol(15),
      lvol(15),
      rvol(15),
      macroVolMul(64) {}
  };
  Channel chan[2];
  DivDispatchOscBuffer* oscBuf[2];
  bool isMuted[2];
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    unsigned short delay;
    QueuedWrite(unsigned short a, unsigned char v, unsigned short d=1):
      addr(a),
      val(v),
      delay(d) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned int sampleOffK007232[256];
  bool sampleLoaded[256];

  int delay;
  unsigned char lastLoop, lastVolume;
  bool stereo;

  unsigned char* sampleMem;
  size_t sampleMemLen;
  k007232_core k007232;
  unsigned char regPool[20];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void chWrite(unsigned char ch, unsigned int addr, unsigned char val);
  public:
    u8 read_sample(u8 ne, u32 address);
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
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void setFlags(const DivConfig& flags);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformK007232():
      DivDispatch(),
      k007232_intf(),
      k007232(*this) {}
};

#endif
