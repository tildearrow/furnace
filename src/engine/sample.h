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
  int rate, centerRate, loopStart, loopOffP;
  // valid values are:
  // - 0: ZX Spectrum overlay drum (1-bit)
  // - 1: 1-bit NES DPCM (1-bit)
  // - 4: QSound ADPCM
  // - 5: ADPCM-A
  // - 6: ADPCM-B
  // - 7: X68000 ADPCM
  // - 8: 8-bit PCM
  // - 9: BRR (SNES)
  // - 10: VOX
  // - 16: 16-bit PCM
  unsigned char depth;

  // these are the new data structures.
  signed char* data8; // 8
  short* data16; // 16
  unsigned char* data1; // 0
  unsigned char* dataDPCM; // 1
  unsigned char* dataQSoundA; // 4
  unsigned char* dataA; // 5
  unsigned char* dataB; // 6
  unsigned char* dataX68; // 7
  unsigned char* dataBRR; // 9
  unsigned char* dataVOX; // 10

  unsigned int length8, length16, length1, lengthDPCM, lengthQSoundA, lengthA, lengthB, lengthX68, lengthBRR, lengthVOX;
  unsigned int off8, off16, off1, offDPCM, offQSoundA, offA, offB, offX68, offBRR, offVOX;
  unsigned int offSegaPCM, offQSound, offX1_010;

  unsigned int samples;

  bool save(const char* path);
  bool initInternal(unsigned char d, int count);
  bool init(unsigned int count);
  void render();
  void* getCurBuf();
  unsigned int getCurBufLen();
  DivSample():
    name(""),
    rate(32000),
    centerRate(8363),
    loopStart(-1),
    loopOffP(0),
    depth(16),
    data8(NULL),
    data16(NULL),
    data1(NULL),
    dataDPCM(NULL),
    dataQSoundA(NULL),
    dataA(NULL),
    dataB(NULL),
    dataX68(NULL),
    dataBRR(NULL),
    dataVOX(NULL),
    length8(0),
    length16(0),
    length1(0),
    lengthDPCM(0),
    lengthQSoundA(0),
    lengthA(0),
    lengthB(0),
    lengthX68(0),
    lengthBRR(0),
    lengthVOX(0),
    off8(0),
    off16(0),
    off1(0),
    offDPCM(0),
    offQSoundA(0),
    offA(0),
    offB(0),
    offX68(0),
    offBRR(0),
    offVOX(0),
    offSegaPCM(0),
    offQSound(0),
    samples(0) {}
  ~DivSample();
};
