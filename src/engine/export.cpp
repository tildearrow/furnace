/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "engine.h"

#include "export/amigaValidation.h"
#include "export/s98.h"
#include "export/sapr.h"
#include "export/tiuna.h"
#include "export/zsm.h"

DivROMExport* DivEngine::buildROM(DivROMExportOptions sys) {
  DivROMExport* exporter=NULL;
  switch (sys) {
    case DIV_ROM_AMIGA_VALIDATION:
      exporter=new DivExportAmigaValidation;
      break;
    case DIV_ROM_TIUNA:
      exporter=new DivExportTiuna;
      break;
    case DIV_ROM_ZSM:
      exporter=new DivExportZSM;
      break;
    case DIV_ROM_SAP_R:
      exporter=new DivExportSAPR;
      break;
    case DIV_ROM_S98:
      exporter=new DivExportS98;
      break;
    default:
      exporter=new DivROMExport;
      break;
  }
  return exporter;
}

std::vector<DivRegWrite> DivEngine::generateResetWrites(DivSystem sys) {
  std::vector<DivRegWrite> w;
  bool resetAY=false;
  bool resetOPN=false;
  bool resetOPN2=false;
  bool resetOPL3=false;
  switch (sys) {
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_EXT:
    case DIV_SYSTEM_YM2608_CSM:
      resetAY=true;
      resetOPN=true;
      resetOPN2=true;
      break;
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
    case DIV_SYSTEM_YM2612_CSM:
      resetOPN=true;
      resetOPN2=true;
      w.emplace_back(0x2b,0); // disable DAC
      break;
    case DIV_SYSTEM_SMS:
      for (int i=0; i<4; i++) {
        w.emplace_back(0,0x90|(i<<5)|15);
      }
      break;
    case DIV_SYSTEM_T6W28:
      for (int i=0; i<4; i++) {
        w.emplace_back(1,0x90|(i<<5)|15);
        w.emplace_back(0,0x90|(i<<5)|15);
      }
      break;
    case DIV_SYSTEM_GB:
      // square 1
      w.emplace_back(0x12,0);
      w.emplace_back(0x14,0x80);

      // square 2
      w.emplace_back(0x17,0);
      w.emplace_back(0x19,0x80);

      // wave
      w.emplace_back(0x1c,0);
      w.emplace_back(0x1e,0x80);

      // noise
      w.emplace_back(0x21,0);
      w.emplace_back(0x23,0x80);
      break;
    case DIV_SYSTEM_PCE:
      for (int i=0; i<6; i++) {
        w.emplace_back(0,i);
        w.emplace_back(4,0x5f);
        w.emplace_back(4,0x1f);
        for (int j=0; j<32; j++) {
          w.emplace_back(6,0);
        }
      }
      break;
    case DIV_SYSTEM_NES:
      w.emplace_back(0x15,0);
      break;
    case DIV_SYSTEM_YM2151:
      for (int i=0; i<8; i++) {
        w.emplace_back(0xe0+i,0xff);
        w.emplace_back(0xe8+i,0xff);
        w.emplace_back(0xf0+i,0xff);
        w.emplace_back(0xf8+i,0xff);

        w.emplace_back(0x08,i);
      }
      break;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      for (int i=0; i<16; i++) {
        w.emplace_back(0x86+(i<<3),3);
      }
      break;
    case DIV_SYSTEM_X1_010:
      for (int i=0; i<16; i++) {
        w.emplace_back(i<<3,0);
      }
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B_EXT:
      // TODO: YM2610B channels 1 and 4 and ADPCM-B
      for (int i=0; i<2; i++) { // set SL and RR to highest
        w.emplace_back(0x81+i,0xff);
        w.emplace_back(0x85+i,0xff);
        w.emplace_back(0x89+i,0xff);
        w.emplace_back(0x8d+i,0xff);
        w.emplace_back(0x181+i,0xff);
        w.emplace_back(0x185+i,0xff);
        w.emplace_back(0x189+i,0xff);
        w.emplace_back(0x18d+i,0xff);
      }
      for (int i=0; i<2; i++) { // note off
        w.emplace_back(0x28,1+i);
        w.emplace_back(0x28,5+i);
      }
      
      // reset AY
      resetAY=true;

      // reset sample
      w.emplace_back(0x100,0xbf);
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7:
      for (int i=0; i<9; i++) {
        w.emplace_back(0x20+i,0);
        w.emplace_back(0x30+i,0);
        w.emplace_back(0x10+i,0);
      }
      break;
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
      resetAY=true;
      resetOPN=true;
      break;
    case DIV_SYSTEM_AY8910:
      resetAY=true;
      break;
    case DIV_SYSTEM_AY8930:
      w.emplace_back(0x0d,0);
      w.emplace_back(0x0d,0xa0);
      break;
    case DIV_SYSTEM_SAA1099:
      w.emplace_back(0x1c,0x02);
      w.emplace_back(0x14,0);
      w.emplace_back(0x15,0);

      for (int i=0; i<6; i++) {
        w.emplace_back(i,0);
      }
      break;
    case DIV_SYSTEM_POKEY:
      for (int i=0; i<9; i++) {
        w.emplace_back(i,0);
      }
      break;
    case DIV_SYSTEM_LYNX:
      w.emplace_back(0x44,0xff); //stereo attenuation select
      w.emplace_back(0x50,0x00); //stereo channel disable
      for (int i=0; i<4; i++) { //stereo attenuation value
        w.emplace_back(0x40+i,0xff);
      }
      break;
    case DIV_SYSTEM_QSOUND:
      for (int i=0; i<16; i++) {
        w.emplace_back(2+(i*8),0);
        w.emplace_back(6+(i*8),0);
      }
      for (int i=0; i<3; i++) {
        w.emplace_back(0xcd+(i*4),0);
        w.emplace_back(0xd6+i,1);
      }
      break;
    case DIV_SYSTEM_ES5506:
      for (int i=0; i<32; i++) {
        for (int b=0; b<4; b++) {
          w.emplace_back((0xf<<2)+b,i);
        }
        unsigned int init_cr=0x0303;
        for (int b=0; b<4; b++) {
          w.emplace_back(b,init_cr>>(24-(b<<3)));
        }
        for (int r=1; r<11; r++) {
          for (int b=0; b<4; b++) {
            w.emplace_back((r<<2)+b,((r==7 || r==9) && b&2)?0xff:0);
          }
        }
        for (int b=0; b<4; b++) {
          w.emplace_back((0xf<<2)+b,0x20|i);
        }
        for (int r=1; r<10; r++) {
          for (int b=0; b<4; b++) {
            w.emplace_back((r<<2)+b,0);
          }
        }
      }
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_Y8950_DRUMS:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
      // disable envelope
      for (int i=0; i<6; i++) {
        w.emplace_back(0x80+i,0x0f);
        w.emplace_back(0x88+i,0x0f);
        w.emplace_back(0x90+i,0x0f);
      }
      // key off + freq reset
      for (int i=0; i<9; i++) {
        w.emplace_back(0xa0+i,0);
        w.emplace_back(0xb0+i,0);
      }
      // TODO: ADPCM
      break;
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
      resetOPL3=true;
      break;
    case DIV_SYSTEM_SCC:
      w.emplace_back(0x8f,0);
      break;
    case DIV_SYSTEM_SCC_PLUS:
      w.emplace_back(0xaf,0);
      break;
    case DIV_SYSTEM_RF5C68:
      w.emplace_back(7,0);
      w.emplace_back(8,0xff);
      break;
    case DIV_SYSTEM_MSM6258:
      w.emplace_back(0,1); // stop
      break;
    case DIV_SYSTEM_MSM6295:
      w.emplace_back(0,0x78); // disable all channels
      w.emplace_back(12,1); // select rate
      break;
    case DIV_SYSTEM_VBOY:
      // isn't it amazing when a chip has a built-in reset command?
      w.emplace_back(0x580,0xff);
      break;
    case DIV_SYSTEM_GA20:
      for (int i=0; i<4; i++) {
        w.emplace_back(5+(i*8),0); // mute
        w.emplace_back(6+(i*8),0); // keyoff
      }
      break;
    case DIV_SYSTEM_K053260:
      for (int i=0; i<4; i++) {
        w.emplace_back(0x2f,0); // mute
        w.emplace_back(0x28,0); // keyoff
      }
      break;
    case DIV_SYSTEM_C140:
      for (int i=0; i<24; i++) {
        w.emplace_back((i<<4)|0,0); // mute
        w.emplace_back((i<<4)|1,0);
        w.emplace_back((i<<4)|5,0); // keyoff
      }
      break;
    case DIV_SYSTEM_C219:
      for (int i=0; i<16; i++) {
        w.emplace_back((i<<4)|0,0); // mute
        w.emplace_back((i<<4)|1,0);
        w.emplace_back((i<<4)|5,0); // keyoff
      }
      break;
    case DIV_SYSTEM_OPL4:
    case DIV_SYSTEM_OPL4_DRUMS:
      resetOPL3=true;
      for (int i=0; i<24; i++) {
        // disable envelope
        w.emplace_back(0x280+i,0x00);
        w.emplace_back(0x298+i,0x00);
        w.emplace_back(0x2b0+i,0x00);
        w.emplace_back(0x2c8+i,0x00);
        w.emplace_back(0x2e0+i,0x00);
        // key off + freq reset
        w.emplace_back(0x220+i,0);
        w.emplace_back(0x238+i,0);
        w.emplace_back(0x268+i,8);
      }
      break;
    default:
      break;
  }
  if (resetAY) {
    w.emplace_back(7,0x3f);
    w.emplace_back(8,0);
    w.emplace_back(9,0);
    w.emplace_back(10,0);
  }
  if (resetOPN) {
    for (int i=0; i<3; i++) { // set SL and RR to highest
      w.emplace_back(0x80+i,0xff);
      w.emplace_back(0x84+i,0xff);
      w.emplace_back(0x88+i,0xff);
      w.emplace_back(0x8c+i,0xff);
    }
    for (int i=0; i<3; i++) { // note off
      w.emplace_back(0x28,i);
    }
  }
  if (resetOPN2) {
    for (int i=0; i<3; i++) { // set SL and RR to highest
      w.emplace_back(0x180+i,0xff);
      w.emplace_back(0x184+i,0xff);
      w.emplace_back(0x188+i,0xff);
      w.emplace_back(0x18c+i,0xff);
    }
    for (int i=0; i<3; i++) { // note off
      w.emplace_back(0x28,4+i);
    }
  }
  if (resetOPL3) {
    // disable envelope
    for (int i=0; i<6; i++) {
      w.emplace_back(0x80+i,0x0f);
      w.emplace_back(0x88+i,0x0f);
      w.emplace_back(0x90+i,0x0f);
      w.emplace_back(0x180+i,0x0f);
      w.emplace_back(0x188+i,0x0f);
      w.emplace_back(0x190+i,0x0f);
    }
    // key off + freq reset
    for (int i=0; i<9; i++) {
      w.emplace_back(0xa0+i,0);
      w.emplace_back(0xb0+i,0);
      w.emplace_back(0x1a0+i,0);
      w.emplace_back(0x1b0+i,0);
    }
    // reset 4-op
    w.emplace_back(0x104,0x00);
  }
  return w;
}
