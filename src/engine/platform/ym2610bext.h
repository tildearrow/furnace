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

#ifndef _YM2610BEXT_H
#define _YM2610BEXT_H

#include "../dispatch.h"

#include "ym2610b.h"

class DivPlatformYM2610BExt: public DivPlatformYM2610B {
  OPNOpChannelStereo opChan[4];
  bool isOpMuted[4];
  friend void putDispatchChip(void*,int);
  inline void commitStateExt(int ch, DivInstrument* ins);
  public:
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    int mapVelocity(int ch, float vel);
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformYM2610BExt();
};

#endif
