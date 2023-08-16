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
  OPN2_Write((ym3438_t*)fmPreviewOPN,0,(addr)); \
  do { \
    OPN2_Clock((ym3438_t*)fmPreviewOPN,out); \
  } while (((ym3438_t*)fmPreviewOPN)->write_busy); \
  OPN2_Write((ym3438_t*)fmPreviewOPN,1,(val)); \
  do { \
    OPN2_Clock((ym3438_t*)fmPreviewOPN,out); \
  } while (((ym3438_t*)fmPreviewOPN)->write_busy); \

const unsigned char dtTableFMP[8]={
  7,6,5,0,1,2,3,4
};

void FurnaceGUI::renderFMPreview(const DivInstrumentFM& params, int pos) {
  if (fmPreviewOPN==NULL) {
    fmPreviewOPN=new ym3438_t;
  }
  short out[2];
  int aOut=0;
  bool mult0=false;

  if (pos==0) {
    OPN2_Reset((ym3438_t*)fmPreviewOPN); 
    OPN2_SetChipType((ym3438_t*)fmPreviewOPN,ym3438_mode_opn);

    // set params
    for (int i=0; i<4; i++) {
      if ((params.op[i].mult&15)==0) {
        mult0=true;
        break;
      }
    }
    for (int i=0; i<4; i++) {
      const DivInstrumentFM::Operator& op=params.op[i];
      unsigned short baseAddr=i*4;
      FM_WRITE(baseAddr+0x40,op.tl);
      FM_WRITE(baseAddr+0x30,(op.mult&15)|(dtTableFMP[op.dt&7]<<4));
      FM_WRITE(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
      FM_WRITE(baseAddr+0x60,(op.dr&31)|(op.am<<7));
      FM_WRITE(baseAddr+0x70,op.d2r&31);
      FM_WRITE(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
      FM_WRITE(baseAddr+0x90,op.ssgEnv&15);
    }
    FM_WRITE(0xb0,(params.alg&7)|((params.fb&7)<<3));
    FM_WRITE(0xb4,0xc0|(params.fms&7)|((params.ams&3)<<4));
    FM_WRITE(0xa4,mult0?0x1c:0x14); // frequency
    FM_WRITE(0xa0,0);
    FM_WRITE(0x28,0xf0); // key on
  }

  // render
  for (int i=0; i<FM_PREVIEW_SIZE; i++) {
    aOut=0;
    for (int j=0; j<24; j++) {
      OPN2_Clock((ym3438_t*)fmPreviewOPN,out);
    }
    aOut+=((ym3438_t*)fmPreviewOPN)->ch_out[0];
    if (aOut<-32768) aOut=-32768;
    if (aOut>32767) aOut=32767;
    fmPreview[i]=aOut;
  }
}
