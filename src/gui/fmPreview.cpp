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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../../extern/opn/ym3438.h"

#define FM_WRITE(addr,val) \
  OPN2_Write(&fm,0,(addr)); \
  do { \
    OPN2_Clock(&fm,out); \
  } while (fm.write_busy); \
  OPN2_Write(&fm,1,(val)); \
  do { \
    OPN2_Clock(&fm,out); \
  } while (fm.write_busy); \

void FurnaceGUI::renderFMPreview(const DivInstrumentFM& params, int pos) {
  ym3438_t fm;
  short out[2];
  int aOut=0;

  OPN2_Reset(&fm); 
  OPN2_SetChipType(&fm,ym3438_mode_opn);

  // set params
  FM_WRITE(0x50,31); // AR
  FM_WRITE(0x54,31);
  FM_WRITE(0x58,31);
  FM_WRITE(0x5c,31);
  FM_WRITE(0x60,0); // DR
  FM_WRITE(0x64,0);
  FM_WRITE(0x68,0);
  FM_WRITE(0x6c,0);
  FM_WRITE(0x70,0); // D2R
  FM_WRITE(0x74,0);
  FM_WRITE(0x78,0);
  FM_WRITE(0x7c,0);
  FM_WRITE(0x80,0); // SL/RR
  FM_WRITE(0x84,0);
  FM_WRITE(0x88,0);
  FM_WRITE(0x8c,0);
  FM_WRITE(0xa4,0x0c); // frequency
  FM_WRITE(0xa0,0);
  FM_WRITE(0xb0,(params.alg&7)|((params.fb&7)<<3)); // ALG/FB
  FM_WRITE(0xb4,0xc0); // pan
  FM_WRITE(0x30,params.op[0].mult&15); // MULT
  FM_WRITE(0x34,params.op[1].mult&15);
  FM_WRITE(0x38,params.op[2].mult&15);
  FM_WRITE(0x3c,params.op[3].mult&15);
  FM_WRITE(0x40,params.op[0].tl&127); // TL
  FM_WRITE(0x44,params.op[1].tl&127);
  FM_WRITE(0x48,params.op[2].tl&127);
  FM_WRITE(0x4c,params.op[3].tl&127);
  FM_WRITE(0x28,0xf0); // key on

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<24; j++) {
      OPN2_Clock(&fm,out);
    }
    aOut+=fm.ch_out[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}
