/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#ifndef _ZXBEEPERQUADTONE_H
#define _ZXBEEPERQUADTONE_H

#include "../dispatch.h"

class DivPlatformZXBeeperQuadTone: public DivDispatch {
  struct Channel: public SharedChannel<unsigned char> {
    unsigned short sPosition;
    unsigned char duty;
    unsigned char out;
    Channel():
      SharedChannel<unsigned char>(2),
      sPosition(0),
      duty(128),
      out(0) {}
  };
  Channel chan[5];
  DivDispatchOscBuffer* oscBuf[5];
  bool isMuted[5];
  bool noHiss;
  bool deHisser[8];

  int cycles, curChan, sOffTimer, delay, curSample, curSamplePeriod;
  unsigned int curSamplePos;
  unsigned int outputClock;
  unsigned char regPool[17];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
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
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformZXBeeperQuadTone();
  private:
    void writeOutVol(int ch);
};

#endif
