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

#include "song.h"

void DivSong::clearSongData() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    pat[i].wipePatterns();
  }

  memset(orders.ord,0,DIV_MAX_CHANS*256);
  ordersLen=1;
}

void DivSong::clearInstruments() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();
  insLen=0;
}

void DivSong::clearWavetables() {
  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();
  waveLen=0;
}

void DivSong::clearSamples() {
  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();
  sampleLen=0;
}

void DivSong::unload() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();
  insLen=0;

  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();
  waveLen=0;

  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();
  sampleLen=0;

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    pat[i].wipePatterns();
  }
}
