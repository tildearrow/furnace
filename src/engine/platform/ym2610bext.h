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

#include "../dispatch.h"

#include "ym2610b.h"

class DivPlatformYM2610BExt: public DivPlatformYM2610B {
  struct OpChannel {
    DivMacroInt std;
    unsigned char freqH, freqL;
    int freq, baseFreq, pitch;
    unsigned char ins;
    signed char konCycles;
    bool active, insChanged, freqChanged, keyOn, keyOff, portaPause;
    int vol;
    unsigned char pan;
    OpChannel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), vol(0), pan(3) {}
  };
  OpChannel opChan[4];
  bool isOpMuted[4];
  friend void putDispatchChan(void*,int,int);
  public:
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void notifyInsChange(int ins);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformYM2610BExt();
};
