/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#ifndef _BIFURCATOR_H
#define _BIFURCATOR_H

#include "../dispatch.h"
#include "../waveSynth.h"

class DivPlatformBifurcator: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    int param, curx;
    int audSub;
    bool volChangedL, volChangedR;
    int chPanL, chPanR;
    int chVolL, chVolR;
    Channel():
      SharedChannel<int>(255),
      param(47360),
      curx(1),
      audSub(0),
      volChangedL(false),
      volChangedR(false),
      chPanL(255),
      chPanR(255),
      chVolL(0),
      chVolR(0) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  unsigned char regPool[8*4];

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};

#endif
