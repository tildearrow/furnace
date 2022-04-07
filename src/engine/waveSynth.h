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

#ifndef _WAVESYNTH_H
#define _WAVESYNTH_H

#include "instrument.h"
#include "wavetable.h"

class DivEngine;

class DivWaveSynth {
  DivEngine* e;
  DivInstrumentWaveSynth state;
  int pos, stage, divCounter, width, height;
  bool first;
  unsigned char wave1[256];
  unsigned char wave2[256];
  int output[256];
  public:
    /**
     * tick this DivWaveSynth.
     * @return whether the wave has changed.
     */
    bool tick();
    void init(DivInstrument* which, int width, int height, bool insChanged=false);
    void setEngine(DivEngine* engine);
    DivWaveSynth():
      e(NULL),
      pos(0),
      stage(0),
      divCounter(0),
      width(32),
      height(31),
      first(false) {
      memset(wave1,0,sizeof(int)*256);
      memset(wave2,0,sizeof(int)*256);
      memset(output,0,sizeof(int)*256);
    }
};

#endif
