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

#include "genesis.h"

class DivPlatformGenesisExt: public DivPlatformGenesis {
  struct OpChannel {
    DivMacroInt std;
    unsigned char freqH, freqL;
    int freq, baseFreq, pitch, pitch2, portaPauseFreq, ins, note;
    signed char konCycles;
    bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta, mask;
    int vol, outVol;
    unsigned char pan;
    void macroInit(DivInstrument* which) {
      std.init(which);
      pitch2=0;
    }
    OpChannel():
      freqH(0),
      freqL(0),
      freq(0),
      baseFreq(0),
      pitch(0),
      pitch2(0),
      portaPauseFreq(0),
      ins(-1),
      note(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      portaPause(false),
      inPorta(false),
      mask(true),
      vol(0),
      outVol(0),
      pan(3) {}
  };
  OpChannel opChan[4];
  bool isOpMuted[4];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool keyOffAffectsPorta(int ch);
    void notifyInsChange(int ins);
    int getPortaFloor(int ch);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformGenesisExt();
};
