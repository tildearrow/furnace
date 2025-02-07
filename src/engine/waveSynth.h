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

#ifndef _WAVESYNTH_H
#define _WAVESYNTH_H

#include "instrument.h"
#include "wavetable.h"

class DivEngine;

class DivWaveSynth {
  DivEngine* e;
  DivInstrumentWaveSynth state;
  int pos, stage, divCounter, width, height, subDivCounter;
  bool first, activeChangedB, stageDir;
  unsigned char wave1[256];
  unsigned char wave2[256];
  public:
    /**
     * the output.
     */
    int output[256];
    /**
     * check whether the "active" status has changed.
     * @return truth.
     */
    bool activeChanged();
    /**
     * tick this DivWaveSynth.
     * @return whether the wave has changed.
     */
    bool tick(bool skipSubDiv=false);
    /**
     * set the wave width.
     * @param value the width.
     */
    void setWidth(int val);
    /**
     * change the first wave.
     * @param num wavetable number.
     * @param force whether to force overwriting the current wave.
     */
    void changeWave1(int num, bool force=false);
    /**
     * change the second wave.
     * @param num wavetable number.
     */
    void changeWave2(int num);
    /**
     * initialize this DivWaveSynth.
     * @param which the instrument.
     * @param width the system's wave width.
     * @param height the system's wave height.
     * @param insChanged whether the instrument has changed.
     */
    void init(DivInstrument* which, int width, int height, bool insChanged=false);
    void setEngine(DivEngine* engine, int waveFloor=0);
    DivWaveSynth():
      e(NULL),
      pos(0),
      stage(0),
      divCounter(0),
      width(32),
      height(31),
      subDivCounter(0),
      first(false),
      activeChangedB(false),
      stageDir(false) {
      memset(wave1,0,256);
      memset(wave2,0,256);
      memset(output,0,sizeof(int)*256);
    }
};

#endif
