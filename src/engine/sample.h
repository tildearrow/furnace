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

#include "../ta-utils.h"

struct DivSample {
  String name;
  int length, rate, centerRate, loopStart, loopOffP;
  signed char vol, pitch;
  // valid values are:
  // - 0: ZX Spectrum overlay drum (1-bit)
  // - 1: 1-bit NES DPCM (1-bit)
  // - 4: QSound ADPCM ()
  // - 5: ADPCM-A
  // - 6: ADPCM-B
  // - 7: X68000 ADPCM
  // - 8: 8-bit PCM
  // - 9: BRR (SNES)
  // - 10: VOX
  // - 16: 16-bit PCM
  unsigned char depth;

  // TODO: drop
  short* data;

  // these are the new data structures.
  signed char* data8;
  short* data16;
  unsigned char* data1;
  unsigned char* dataDPCM;
  unsigned char* dataQSound;
  unsigned char* dataA;
  unsigned char* dataB;
  unsigned char* dataX68;
  unsigned char* dataBRR;
  unsigned char* dataVOX;


  unsigned int rendLength, adpcmRendLength, rendOff, rendOffP, rendOffContiguous, rendOffQsound;
  short* rendData;
  unsigned char* adpcmRendData;

  bool save(const char* path);
  DivSample():
    name(""),
    length(0),
    rate(32000),
    centerRate(8363),
    loopStart(-1),
    loopOffP(0),
    vol(0),
    pitch(0),
    depth(16),
    data(NULL),
    rendLength(0),
    adpcmRendLength(0),
    rendOff(0),
    rendOffP(0),
    rendOffContiguous(0),
    rendOffQsound(0),
    rendData(NULL),
    adpcmRendData(NULL) {}
  ~DivSample();
};
