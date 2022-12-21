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

#include "engine.h"
#include "../ta-log.h"
#include "../utfutils.h"
#include "song.h"

constexpr int MASTER_CLOCK_PREC=(sizeof(void*)==8)?8:0;

void DivEngine::performVGMWrite(SafeWriter* w, DivSystem sys, DivRegWrite& write, int streamOff, double* loopTimer, double* loopFreq, int* loopSample, bool* sampleDir, bool isSecond, bool directStream) {
  unsigned char baseAddr1=isSecond?0xa0:0x50;
  unsigned char baseAddr2=isSecond?0x80:0;
  unsigned short baseAddr2S=isSecond?0x8000:0;
  unsigned char smsAddr=isSecond?0x30:0x50;
  unsigned char rf5c68Addr=isSecond?0xb1:0xb0;
  if (write.addr==0xffffffff) { // Furnace fake reset
    switch (sys) {
      case DIV_SYSTEM_YM2612:
      case DIV_SYSTEM_YM2612_EXT:
      case DIV_SYSTEM_YM2612_DUALPCM:
      case DIV_SYSTEM_YM2612_DUALPCM_EXT:
        for (int i=0; i<3; i++) { // set SL and RR to highest
          w->writeC(2|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0xff);
          w->writeC(2|baseAddr1);
          w->writeC(0x84+i);
          w->writeC(0xff);
          w->writeC(2|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0xff);
          w->writeC(2|baseAddr1);
          w->writeC(0x8c+i);
          w->writeC(0xff);

          w->writeC(3|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0xff);
          w->writeC(3|baseAddr1);
          w->writeC(0x84+i);
          w->writeC(0xff);
          w->writeC(3|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0xff);
          w->writeC(3|baseAddr1);
          w->writeC(0x8c+i);
          w->writeC(0xff);
        }
        for (int i=0; i<3; i++) { // note off
          w->writeC(2|baseAddr1);
          w->writeC(0x28);
          w->writeC(i);
          w->writeC(2|baseAddr1);
          w->writeC(0x28);
          w->writeC(4+i);
        }
        w->writeC(2|baseAddr1); // disable DAC
        w->writeC(0x2b);
        w->writeC(0);
        break;
      case DIV_SYSTEM_SMS:
        for (int i=0; i<4; i++) {
          w->writeC(smsAddr);
          w->writeC(0x90|(i<<5)|15);
        }
        break;
      case DIV_SYSTEM_T6W28:
        for (int i=0; i<4; i++) {
          w->writeC(0x30);
          w->writeC(0x90|(i<<5)|15);
          w->writeC(0x50);
          w->writeC(0x90|(i<<5)|15);
        }
        break;
      case DIV_SYSTEM_GB:
        // square 1
        w->writeC(0xb3);
        w->writeC(2|baseAddr2);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(4|baseAddr2);
        w->writeC(0x80);

        // square 2
        w->writeC(0xb3);
        w->writeC(7|baseAddr2);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(9|baseAddr2);
        w->writeC(0x80);

        // wave
        w->writeC(0xb3);
        w->writeC(0x0c|baseAddr2);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(0x0e|baseAddr2);
        w->writeC(0x80);

        // noise
        w->writeC(0xb3);
        w->writeC(0x11|baseAddr2);
        w->writeC(0);
        w->writeC(0xb3);
        w->writeC(0x13|baseAddr2);
        w->writeC(0x80);
        break;
      case DIV_SYSTEM_PCE:
        for (int i=0; i<6; i++) {
          w->writeC(0xb9);
          w->writeC(0|baseAddr2);
          w->writeC(i);
          w->writeC(0xb9);
          w->writeC(4|baseAddr2);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_NES:
        w->writeC(0xb4);
        w->writeC(0x15|baseAddr2);
        w->writeC(0);
        break;
      case DIV_SYSTEM_YM2151:
        for (int i=0; i<8; i++) {
          w->writeC(4|baseAddr1);
          w->writeC(0xe0+i);
          w->writeC(0xff);
          w->writeC(4|baseAddr1);
          w->writeC(0xe8+i);
          w->writeC(0xff);
          w->writeC(4|baseAddr1);
          w->writeC(0xf0+i);
          w->writeC(0xff);
          w->writeC(4|baseAddr1);
          w->writeC(0xf8+i);
          w->writeC(0xff);

          w->writeC(4|baseAddr1);
          w->writeC(0x08);
          w->writeC(i);
        }
        break;
      case DIV_SYSTEM_SEGAPCM:
      case DIV_SYSTEM_SEGAPCM_COMPAT:
        for (int i=0; i<16; i++) {
          w->writeC(0xc0);
          w->writeS((0x86|baseAddr2S)+(i<<3));
          w->writeC(3);
        }
        break;
      case DIV_SYSTEM_X1_010:
        for (int i=0; i<16; i++) {
          w->writeC(0xc8);
          w->writeS_BE(baseAddr2S+(i<<3));
          w->writeC(0);
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
          w->writeC(8|baseAddr1);
          w->writeC(0x81+i);
          w->writeC(0xff);
          w->writeC(8|baseAddr1);
          w->writeC(0x85+i);
          w->writeC(0xff);
          w->writeC(8|baseAddr1);
          w->writeC(0x89+i);
          w->writeC(0xff);
          w->writeC(8|baseAddr1);
          w->writeC(0x8d+i);
          w->writeC(0xff);

          w->writeC(9|baseAddr1);
          w->writeC(0x81+i);
          w->writeC(0xff);
          w->writeC(9|baseAddr1);
          w->writeC(0x85+i);
          w->writeC(0xff);
          w->writeC(9|baseAddr1);
          w->writeC(0x89+i);
          w->writeC(0xff);
          w->writeC(9|baseAddr1);
          w->writeC(0x8d+i);
          w->writeC(0xff);
        }
        for (int i=0; i<2; i++) { // note off
          w->writeC(8|baseAddr1);
          w->writeC(0x28);
          w->writeC(1+i);
          w->writeC(8|baseAddr1);
          w->writeC(0x28);
          w->writeC(5+i);
        }
        
        // reset AY
        w->writeC(8|baseAddr1);
        w->writeC(7);
        w->writeC(0x3f);

        w->writeC(8|baseAddr1);
        w->writeC(8);
        w->writeC(0);

        w->writeC(8|baseAddr1);
        w->writeC(9);
        w->writeC(0);

        w->writeC(8|baseAddr1);
        w->writeC(10);
        w->writeC(0);

        // reset sample
        w->writeC(9|baseAddr1);
        w->writeC(0);
        w->writeC(0xbf);
        break;
      case DIV_SYSTEM_OPLL:
      case DIV_SYSTEM_OPLL_DRUMS:
      case DIV_SYSTEM_VRC7:
        for (int i=0; i<9; i++) {
          w->writeC(1|baseAddr1);
          w->writeC(0x20+i);
          w->writeC(0);
          w->writeC(1|baseAddr1);
          w->writeC(0x30+i);
          w->writeC(0);
          w->writeC(1|baseAddr1);
          w->writeC(0x10+i);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_YM2203:
      case DIV_SYSTEM_YM2203_EXT:
        for (int i=0; i<3; i++) { // set SL and RR to highest
          w->writeC(5|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0xff);
          w->writeC(5|baseAddr1);
          w->writeC(0x84+i);
          w->writeC(0xff);
          w->writeC(5|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0xff);
          w->writeC(5|baseAddr1);
          w->writeC(0x8c+i);
          w->writeC(0xff);
        }
        for (int i=0; i<3; i++) { // note off
          w->writeC(5|baseAddr1);
          w->writeC(0x28);
          w->writeC(i);
        }

        // SSG
        w->writeC(5|baseAddr1);
        w->writeC(7);
        w->writeC(0x3f);

        w->writeC(5|baseAddr1);
        w->writeC(8);
        w->writeC(0);

        w->writeC(5|baseAddr1);
        w->writeC(9);
        w->writeC(0);

        w->writeC(5|baseAddr1);
        w->writeC(10);
        w->writeC(0);
        break;
      case DIV_SYSTEM_AY8910:
        w->writeC(0xa0);
        w->writeC(7|baseAddr2);
        w->writeC(0x3f);

        w->writeC(0xa0);
        w->writeC(8|baseAddr2);
        w->writeC(0);

        w->writeC(0xa0);
        w->writeC(9|baseAddr2);
        w->writeC(0);

        w->writeC(0xa0);
        w->writeC(10|baseAddr2);
        w->writeC(0);
        break;
      case DIV_SYSTEM_AY8930:
        w->writeC(0xa0);
        w->writeC(0x0d|baseAddr2);
        w->writeC(0);
        w->writeC(0xa0);
        w->writeC(0x0d|baseAddr2);
        w->writeC(0xa0);
        break;
      case DIV_SYSTEM_SAA1099:
        w->writeC(0xbd);
        w->writeC(0x1c|baseAddr2);
        w->writeC(0x02);
        w->writeC(0xbd);
        w->writeC(0x14|baseAddr2);
        w->writeC(0);
        w->writeC(0xbd);
        w->writeC(0x15|baseAddr2);
        w->writeC(0);

        for (int i=0; i<6; i++) {
          w->writeC(0xbd);
          w->writeC((0|baseAddr2)+i);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_POKEY:
        for (int i=0; i<9; i++) {
          w->writeC(0xbb);
          w->writeC(i|baseAddr2);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_LYNX:
        w->writeC(0x4e);
        w->writeC(0x44);
        w->writeC(0xff); //stereo attenuation select
        w->writeC(0x4e);
        w->writeC(0x50);
        w->writeC(0x00); //stereo channel disable
        for (int i=0; i<4; i++) { //stereo attenuation value
          w->writeC(0x4e);
          w->writeC(0x40+i);
          w->writeC(0xff);
        }
        break;
      case DIV_SYSTEM_QSOUND:
        for (int i=0; i<16; i++) {
          w->writeC(0xc4);
          w->writeC(0);
          w->writeC(0);
          w->writeC(2+(i*8));
          w->writeC(0xc4);
          w->writeC(0);
          w->writeC(0);
          w->writeC(6+(i*8));
        }
        for (int i=0; i<3; i++) {
          w->writeC(0xc4);
          w->writeC(0);
          w->writeC(0);
          w->writeC(0xcd+(i*4));
          w->writeC(0xc4);
          w->writeC(0x00);
          w->writeC(0x01);
          w->writeC(0xd6+i);
        }
        break;
      case DIV_SYSTEM_OPL:
      case DIV_SYSTEM_OPL_DRUMS:
        // disable envelope
        for (int i=0; i<6; i++) {
          w->writeC(0x0b|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0x0f);
          w->writeC(0x0b|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0x0f);
          w->writeC(0x0b|baseAddr1);
          w->writeC(0x90+i);
          w->writeC(0x0f);
        }
        // key off + freq reset
        for (int i=0; i<9; i++) {
          w->writeC(0x0b|baseAddr1);
          w->writeC(0xa0+i);
          w->writeC(0);
          w->writeC(0x0b|baseAddr1);
          w->writeC(0xb0+i);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_Y8950:
      case DIV_SYSTEM_Y8950_DRUMS:
        // disable envelope
        for (int i=0; i<6; i++) {
          w->writeC(0x0c|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0x0f);
          w->writeC(0x0c|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0x0f);
          w->writeC(0x0c|baseAddr1);
          w->writeC(0x90+i);
          w->writeC(0x0f);
        }
        // key off + freq reset
        for (int i=0; i<9; i++) {
          w->writeC(0x0c|baseAddr1);
          w->writeC(0xa0+i);
          w->writeC(0);
          w->writeC(0x0c|baseAddr1);
          w->writeC(0xb0+i);
          w->writeC(0);
        }
        // TODO: ADPCM
        break;
      case DIV_SYSTEM_OPL2:
      case DIV_SYSTEM_OPL2_DRUMS:
        // disable envelope
        for (int i=0; i<6; i++) {
          w->writeC(0x0a|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0x0f);
          w->writeC(0x0a|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0x0f);
          w->writeC(0x0a|baseAddr1);
          w->writeC(0x90+i);
          w->writeC(0x0f);
        }
        // key off + freq reset
        for (int i=0; i<9; i++) {
          w->writeC(0x0a|baseAddr1);
          w->writeC(0xa0+i);
          w->writeC(0);
          w->writeC(0x0a|baseAddr1);
          w->writeC(0xb0+i);
          w->writeC(0);
        }
        break;
      case DIV_SYSTEM_OPL3:
      case DIV_SYSTEM_OPL3_DRUMS:
        // disable envelope
        for (int i=0; i<6; i++) {
          w->writeC(0x0e|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0x0f);
          w->writeC(0x0e|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0x0f);
          w->writeC(0x0e|baseAddr1);
          w->writeC(0x90+i);
          w->writeC(0x0f);
          w->writeC(0x0f|baseAddr1);
          w->writeC(0x80+i);
          w->writeC(0x0f);
          w->writeC(0x0f|baseAddr1);
          w->writeC(0x88+i);
          w->writeC(0x0f);
          w->writeC(0x0f|baseAddr1);
          w->writeC(0x90+i);
          w->writeC(0x0f);
        }
        // key off + freq reset
        for (int i=0; i<9; i++) {
          w->writeC(0x0e|baseAddr1);
          w->writeC(0xa0+i);
          w->writeC(0);
          w->writeC(0x0e|baseAddr1);
          w->writeC(0xb0+i);
          w->writeC(0);
          w->writeC(0x0f|baseAddr1);
          w->writeC(0xa0+i);
          w->writeC(0);
          w->writeC(0x0f|baseAddr1);
          w->writeC(0xb0+i);
          w->writeC(0);
        }
        // reset 4-op
        w->writeC(0x0f|baseAddr1);
        w->writeC(0x04);
        w->writeC(0x00);
        break;
      case DIV_SYSTEM_SCC:
      case DIV_SYSTEM_SCC_PLUS:
        w->writeC(0xd2);
        w->writeC(baseAddr2|3);
        w->writeC(0);
        w->writeC(0);
        break;
      case DIV_SYSTEM_RF5C68:
        w->writeC(rf5c68Addr);
        w->writeC(7);
        w->writeC(0);
        w->writeC(rf5c68Addr);
        w->writeC(8);
        w->writeC(0xff);
        break;
      case DIV_SYSTEM_MSM6295:
        w->writeC(0xb8); // disable all channels
        w->writeC(baseAddr2|0);
        w->writeC(0x78);
        w->writeC(0xb8); // select rate
        w->writeC(baseAddr2|12);
        w->writeC(1);
        break;
      case DIV_SYSTEM_VBOY:
        // isn't it amazing when a chip has a built-in reset command?
        w->writeC(0xc7);
        w->writeS_BE(baseAddr2S|(0x580>>2));
        w->writeC(0xff);
        break;
      case DIV_SYSTEM_GA20:
        for (int i=0; i<3; i++) {
          w->writeC(0xbf); // mute
          w->writeC((baseAddr2|5)+(i*8));
          w->writeC(0);
          w->writeC(0xbf); // keyoff
          w->writeC((baseAddr2|6)+(i*8));
          w->writeC(0);
        }
        break;
      default:
        break;
    }
  }
  if (write.addr>=0xffff0000) { // Furnace special command
    if (!directStream) {
      unsigned char streamID=streamOff+((write.addr&0xff00)>>8);
      logD("writing stream command %x:%x with stream ID %d",write.addr,write.val,streamID);
      switch (write.addr&0xff) {
        case 0: // play sample
          if (write.val<song.sampleLen) {
            DivSample* sample=song.sample[write.val];
            w->writeC(0x95);
            w->writeC(streamID);
            w->writeS(write.val); // sample number
            w->writeC((sample->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT)==0)|(sampleDir[streamID]?0x10:0)); // flags
            if (sample->isLoopable() && !sampleDir[streamID]) {
              loopTimer[streamID]=sample->length8;
              loopSample[streamID]=write.val;
            }
          }
          break;
        case 1: // set sample freq
          w->writeC(0x92);
          w->writeC(streamID);
          w->writeI(write.val);
          loopFreq[streamID]=write.val;
          break;
        case 2: // stop sample
          w->writeC(0x94);
          w->writeC(streamID);
          loopSample[streamID]=-1;
          break;
        case 3: // set sample direction
          sampleDir[streamID]=write.val;
          break;
      }
    }
    return;
  }
  switch (sys) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
      switch (write.addr>>8) {
        case 0: // port 0
          w->writeC(2|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // port 1
          w->writeC(3|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 2: // PSG
          w->writeC(smsAddr);
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_SMS:
      w->writeC(smsAddr);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_T6W28:
      if (write.addr) {
        w->writeC(0x30);
      } else {
        w->writeC(0x50);
      }
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_GB:
      w->writeC(0xb3);
      w->writeC(baseAddr2|((write.addr-16)&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_PCE:
      w->writeC(0xb9);
      w->writeC(baseAddr2|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_NES:
      w->writeC(0xb4);
      w->writeC(baseAddr2|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_FDS: // yeah
      w->writeC(0xb4);
      if ((write.addr&0xff)==0x23) {
        w->writeC(baseAddr2|0x3f);
      } else if ((write.addr&0xff)>=0x80) {
        w->writeC(baseAddr2|(0x20+(write.addr&0x7f)));
      } else {
        w->writeC(baseAddr2|(write.addr&0xff));
      }
      
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_YM2151:
      w->writeC(4|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      w->writeC(0xc0);
      w->writeS(baseAddr2S|(write.addr&0xffff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_X1_010:
      w->writeC(0xc8);
      w->writeS_BE(baseAddr2S|(write.addr&0x1fff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B_EXT:
      switch (write.addr>>8) {
        case 0: // port 0
          w->writeC(8|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // port 1
          w->writeC(9|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
      w->writeC(5|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_EXT:
      switch (write.addr>>8) {
        case 0: // port 0
          w->writeC(6|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // port 1
          w->writeC(7|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7:
      w->writeC(1|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      w->writeC(0xa0);
      w->writeC(baseAddr2|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_SAA1099:
      w->writeC(0xbd);
      w->writeC(baseAddr2|(write.addr&0xff));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_POKEY:
      w->writeC(0xbb);
      w->writeC(baseAddr2|(write.addr&0x0f));
      w->writeC(write.val&0xff);
      break;
    case DIV_SYSTEM_LYNX:
      w->writeC(0x4e);
      w->writeC(write.addr&0xff);
      w->writeC(write.val&0xff);
      break;
    case DIV_SYSTEM_QSOUND:
      w->writeC(0xc4);
      w->writeC((write.val>>8)&0xff);
      w->writeC(write.val&0xff);
      w->writeC(write.addr&0xff);
      break;
    case DIV_SYSTEM_SWAN:
      if ((write.addr&0x7f)<0x40) {
        w->writeC(0xbc);
        w->writeC(baseAddr2|(write.addr&0x3f));
        w->writeC(write.val&0xff);
      } else {
        // (Wave) RAM write
        w->writeC(0xc6);
        w->writeS_BE(baseAddr2S|(write.addr&0x3f));
        w->writeC(write.val&0xff);
      }
      break;
    case DIV_SYSTEM_VBOY:
      w->writeC(0xc7);
      w->writeS_BE(baseAddr2S|(write.addr>>2));
      w->writeC(write.val&0xff);
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
      w->writeC(0x0b|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_Y8950_DRUMS:
      w->writeC(0x0c|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
      w->writeC(0x0a|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
      switch (write.addr>>8) {
        case 0: // port 0
          w->writeC(0x0e|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
        case 1: // port 1
          w->writeC(0x0f|baseAddr1);
          w->writeC(write.addr&0xff);
          w->writeC(write.val);
          break;
      }
      break;
    case DIV_SYSTEM_SCC:
      if (write.addr<0x80) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|0);
        w->writeC(write.addr&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr<0x8a) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|1);
        w->writeC((write.addr-0x80)&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr<0x8f) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|2);
        w->writeC((write.addr-0x8a)&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr<0x90) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|3);
        w->writeC((write.addr-0x8f)&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr>=0xe0) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|5);
        w->writeC((write.addr-0xe0)&0x7f);
        w->writeC(write.val&0xff);
      } else {
        logW("SCC: writing to unmapped address %.2x!",write.addr);
      }
      break;
    case DIV_SYSTEM_SCC_PLUS:
      if (write.addr<0x80) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|0);
        w->writeC(write.addr&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr<0xa0) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|4);
        w->writeC(write.addr);
        w->writeC(write.val&0xff);
      } else if (write.addr<0xaa) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|1);
        w->writeC((write.addr-0xa0)&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr<0xaf) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|2);
        w->writeC((write.addr-0xaa)&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr<0xb0) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|3);
        w->writeC((write.addr-0xaf)&0x7f);
        w->writeC(write.val&0xff);
      } else if (write.addr>=0xe0) {
        w->writeC(0xd2);
        w->writeC(baseAddr2|5);
        w->writeC((write.addr-0xe0)&0x7f);
        w->writeC(write.val&0xff);
      } else {
        logW("SCC+: writing to unmapped address %.2x!",write.addr);
      }
      break;
    case DIV_SYSTEM_YMZ280B:
      w->writeC(0x0d|baseAddr1);
      w->writeC(write.addr&0xff);
      w->writeC(write.val&0xff);
      break;
    case DIV_SYSTEM_RF5C68:
      w->writeC(rf5c68Addr);
      w->writeC(write.addr&0xff);
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_MSM6295:
      w->writeC(0xb8);
      w->writeC(baseAddr2|(write.addr&0x7f));
      w->writeC(write.val);
      break;
    case DIV_SYSTEM_GA20:
      w->writeC(0xbf);
      w->writeC(baseAddr2|(write.addr&0x7f));
      w->writeC(write.val);
      break;
    default:
      logW("write not handled!");
      break;
  }
}

#define CHIP_VOL(_id,_mult) { \
  double _vol=fabs((float)song.systemVol[i])*4.0*_mult; \
  if (_vol<0.0) _vol=0.0; \
  if (_vol>32767.0) _vol=32767.0; \
  chipVol.push_back((_id)|(0x80000000)|(((unsigned int)_vol)<<16)); \
}

#define CHIP_VOL_SECOND(_id,_mult) { \
  double _vol=fabs((float)song.systemVol[i])*4.0*_mult; \
  if (_vol<0.0) _vol=0.0; \
  if (_vol>32767.0) _vol=32767.0; \
  chipVol.push_back((_id)|(0x80000100)|(((unsigned int)_vol)<<16)); \
}

SafeWriter* DivEngine::saveVGM(bool* sysToExport, bool loop, int version, bool patternHints, bool directStream) {
  if (version<0x150) {
    lastError="VGM version is too low";
    return NULL;
  }
  stop();
  repeatPattern=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;
  double origRate=got.rate;
  got.rate=44100;
  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);
  warnings="";

  curOrder=0;
  freelance=false;
  playing=false;
  extValuePresent=false;
  remainingLoops=-1;

  // play the song ourselves
  bool done=false;
  int writeCount=0;

  int gd3Off=0;

  int hasSN=0;
  int snNoiseConfig=9;
  int snNoiseSize=16;
  int snFlags=0;
  int hasOPLL=0;
  int hasOPN2=0;
  int hasOPM=0;
  int hasSegaPCM=0;
  int segaPCMOffset=0xf8000d;
  int hasRFC=0;
  int hasOPN=0;
  int hasOPNA=0;
  int hasOPNB=0;
  int hasOPL2=0;
  int hasOPL=0;
  int hasY8950=0;
  int hasOPL3=0;
  int hasOPL4=0;
  int hasOPX=0;
  int hasZ280=0;
  int hasRFC1=0;
  int hasPWM=0;
  int hasAY=0;
  int ayConfig=0;
  int ayFlags=0;
  int hasGB=0;
  int hasNES=0;
  int hasMultiPCM=0;
  int hasuPD7759=0;
  int hasOKIM6258=0;
  int hasK054539=0;
  int hasOKIM6295=0;
  int hasK051649=0;
  int hasPCE=0;
  int hasNamco=0;
  int hasK053260=0;
  int hasPOKEY=0;
  int hasQSound=0;
  int hasSCSP=0;
  int hasSwan=0;
  int hasVSU=0;
  int hasSAA=0;
  int hasES5503=0;
  int hasES5505=0;
  int hasX1=0;
  int hasC352=0;
  int hasGA20=0;
  int hasLynx=0;

  int howManyChips=0;

  int loopPos=-1;
  int loopTick=-1;

  unsigned int sampleOff8[256];
  unsigned int sampleOffSegaPCM[256];

  SafeWriter* w=new SafeWriter;
  w->init();

  // write header
  w->write("Vgm ",4);
  w->writeI(0); // will be written later
  w->writeI(version);

  bool willExport[DIV_MAX_CHIPS];
  bool isSecond[DIV_MAX_CHIPS];
  int streamIDs[DIV_MAX_CHIPS];
  double loopTimer[DIV_MAX_CHANS];
  double loopFreq[DIV_MAX_CHANS];
  int loopSample[DIV_MAX_CHANS];
  bool sampleDir[DIV_MAX_CHANS];
  std::vector<unsigned int> chipVol; 
  std::vector<DivDelayedWrite> delayedWrites[DIV_MAX_CHIPS];
  std::vector<std::pair<int,DivDelayedWrite>> sortedWrites;

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    loopTimer[i]=0;
    loopFreq[i]=0;
    loopSample[i]=-1;
    sampleDir[i]=false;
  }

  bool writeDACSamples=false;
  bool writeNESSamples=false;
  bool writePCESamples=false;
  DivDispatch* writeADPCM_OPNA[2]={NULL,NULL};
  DivDispatch* writeADPCM_OPNB[2]={NULL,NULL};
  DivDispatch* writeADPCM_Y8950[2]={NULL,NULL};
  int writeSegaPCM=0;
  DivDispatch* writeX1010[2]={NULL,NULL};
  DivDispatch* writeQSound[2]={NULL,NULL};
  DivDispatch* writeZ280[2]={NULL,NULL};
  DivDispatch* writeRF5C68[2]={NULL,NULL};
  DivDispatch* writeMSM6295[2]={NULL,NULL};
  DivDispatch* writeGA20[2]={NULL,NULL};

  for (int i=0; i<song.systemLen; i++) {
    willExport[i]=false;
    isSecond[i]=false;
    streamIDs[i]=0;
    if (sysToExport!=NULL) {
      if (!sysToExport[i]) continue;
    }
    if (minVGMVersion(song.system[i])>version) continue;
    switch (song.system[i]) {
      case DIV_SYSTEM_SMS:
        if (!hasSN) {
          hasSN=disCont[i].dispatch->chipClock;
          CHIP_VOL(0,1.0);
          willExport[i]=true;
          switch (song.systemFlags[i].getInt("chipType",0)) {
            case 1: // real SN
              snNoiseConfig=3;
              snNoiseSize=15;
              break;
            case 2: // real SN atari bass (seemingly unsupported)
              snNoiseConfig=3;
              snNoiseSize=15;
              break;
            default: // Sega VDP
              snNoiseConfig=9;
              snNoiseSize=16;
              break;
          }
        } else if (!(hasSN&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          CHIP_VOL_SECOND(0,1.0);
          hasSN|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_GB:
        if (!hasGB) {
          hasGB=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasGB&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasGB|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_PCE:
        if (!hasPCE) {
          hasPCE=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writePCESamples=true;
        } else if (!(hasPCE&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasPCE|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_NES:
        if (!hasNES) {
          hasNES=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeNESSamples=true;
        } else if (!(hasNES&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasNES|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_SEGAPCM:
      case DIV_SYSTEM_SEGAPCM_COMPAT:
        if (!hasSegaPCM) {
          hasSegaPCM=4000000;
          willExport[i]=true;
          writeSegaPCM=1;
        } else if (!(hasSegaPCM&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeSegaPCM=2;
          hasSegaPCM|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_X1_010:
        if (!hasX1) {
          hasX1=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeX1010[0]=disCont[i].dispatch;
        } else if (!(hasX1&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeX1010[1]=disCont[i].dispatch;
          hasX1|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2610:
      case DIV_SYSTEM_YM2610_FULL:
      case DIV_SYSTEM_YM2610B:
      case DIV_SYSTEM_YM2610_EXT:
      case DIV_SYSTEM_YM2610_FULL_EXT:
      case DIV_SYSTEM_YM2610B_EXT:
        if (!hasOPNB) {
          hasOPNB=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeADPCM_OPNB[0]=disCont[i].dispatch;
        } else if (!(hasOPNB&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeADPCM_OPNB[1]=disCont[i].dispatch;
          hasOPNB|=0x40000000;
          howManyChips++;
        }
        if (((song.system[i]==DIV_SYSTEM_YM2610B) || (song.system[i]==DIV_SYSTEM_YM2610B_EXT)) && (!(hasOPNB&0x80000000))) { // YM2610B flag
          hasOPNB|=0x80000000;
        }
        break;
      case DIV_SYSTEM_AY8910:
      case DIV_SYSTEM_AY8930: {
        if (!hasAY) {
          bool hasClockDivider=false; // Configurable clock divider
          bool hasStereo=true; // Stereo
          hasAY=disCont[i].dispatch->chipClock;
          ayFlags=1;
          if (song.system[i]==DIV_SYSTEM_AY8930) { // AY8930
            ayConfig=0x03;
            hasClockDivider=true;
          } else {
            switch (song.systemFlags[i].getInt("chipType",0)) {
              case 1: // YM2149
                ayConfig=0x10;
                hasClockDivider=true;
                break;
              case 2: // Sunsoft 5B
                ayConfig=0x10;
                ayFlags|=0x12; // Clock internally divided, Single sound output
                hasStereo=false; // due to above, can't be per-channel stereo configurable
                break;
              case 3: // AY8914
                ayConfig=0x04;
                break;
              default: // AY8910
                ayConfig=0x00;
                break;
            }
          }
          if (hasClockDivider && song.systemFlags[i].getBool("halfClock",false)) {
            ayFlags|=0x10;
          }
          if (hasStereo && song.systemFlags[i].getBool("stereo",false)) {
            ayFlags|=0x80;
          }
          willExport[i]=true;
        } else if (!(hasAY&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasAY|=0x40000000;
          howManyChips++;
        }
        break;
      }
      case DIV_SYSTEM_SAA1099:
        if (!hasSAA) {
          hasSAA=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasSAA&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasSAA|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2612:
      case DIV_SYSTEM_YM2612_EXT:
      case DIV_SYSTEM_YM2612_DUALPCM:
      case DIV_SYSTEM_YM2612_DUALPCM_EXT:
        if (!hasOPN2) {
          hasOPN2=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeDACSamples=true;
        } else if (!(hasOPN2&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPN2|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2151:
        if (!hasOPM) {
          hasOPM=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasOPM&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPM|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2203:
      case DIV_SYSTEM_YM2203_EXT:
        if (!hasOPN) {
          hasOPN=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeDACSamples=true;
        } else if (!(hasOPN&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPN|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YM2608:
      case DIV_SYSTEM_YM2608_EXT:
        if (!hasOPNA) {
          hasOPNA=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeADPCM_OPNA[0]=disCont[i].dispatch;
        } else if (!(hasOPNA&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeADPCM_OPNA[1]=disCont[i].dispatch;
          hasOPNA|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_OPLL:
      case DIV_SYSTEM_OPLL_DRUMS:
      case DIV_SYSTEM_VRC7:
        if (!hasOPLL) {
          hasOPLL=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasOPLL&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPLL|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_FDS:
        if (!hasNES) {
          hasNES=0x80000000|disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasNES&0x80000000)) {
          hasNES|=0x80000000;
          willExport[i]=true;
        } else if (!(hasNES&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasNES|=0xc0000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_POKEY:
        if (!hasPOKEY) {
          hasPOKEY=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasPOKEY&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasPOKEY|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_LYNX:
        if (!hasLynx) {
          hasLynx=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasLynx&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasLynx|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_QSOUND:
        if (!hasQSound) {
          // could set chipClock to 4000000 here for compatibility
          // However I think it it not necessary because old VGM players will still
          // not be able to handle the 64kb sample bank trick
          hasQSound=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeQSound[0]=disCont[i].dispatch;
        } else if (!(hasQSound&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=false;
          writeQSound[1]=disCont[i].dispatch;
          addWarning("dual QSound is not supported by the VGM format");
        }
        break;
      case DIV_SYSTEM_SWAN:
        if (!hasSwan) {
          hasSwan=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          // funny enough, VGM doesn't have support for WSC's sound DMA by design
          // so DAC stream it goes
          // since WS has the same PCM format as YM2612 DAC, I can just reuse this flag
          writeDACSamples=true;
        } else if (!(hasSwan&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasSwan|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_VBOY:
        if (!hasVSU) {
          hasVSU=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasVSU&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasVSU|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_OPL:
      case DIV_SYSTEM_OPL_DRUMS:
        if (!hasOPL) {
          hasOPL=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasOPL&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPL|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_Y8950:
      case DIV_SYSTEM_Y8950_DRUMS:
        if (!hasY8950) {
          hasY8950=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeADPCM_Y8950[0]=disCont[i].dispatch;
        } else if (!(hasY8950&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeADPCM_Y8950[1]=disCont[i].dispatch;
          hasY8950|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_OPL2:
      case DIV_SYSTEM_OPL2_DRUMS:
        if (!hasOPL2) {
          hasOPL2=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasOPL2&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPL2|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_OPL3:
      case DIV_SYSTEM_OPL3_DRUMS:
        if (!hasOPL3) {
          hasOPL3=disCont[i].dispatch->chipClock;
          willExport[i]=true;
        } else if (!(hasOPL3&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasOPL3|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_SCC:
      case DIV_SYSTEM_SCC_PLUS:
        if (!hasK051649) {
          hasK051649=disCont[i].dispatch->chipClock;
          if (song.system[i]==DIV_SYSTEM_SCC_PLUS) {
            hasK051649|=0x80000000;
          }
          willExport[i]=true;
        } else if (!(hasK051649&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          hasK051649|=0x40000000;
          if (song.system[i]==DIV_SYSTEM_SCC_PLUS) {
            hasK051649|=0x80000000;
          }
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_YMZ280B:
        if (!hasZ280) {
          hasZ280=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeZ280[0]=disCont[i].dispatch;
        } else if (!(hasZ280&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeZ280[1]=disCont[i].dispatch;
          hasZ280|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_RF5C68:
        // here's the dumb part: VGM thinks RF5C68 and RF5C164 are different
        // chips even though the only difference is the output resolution
        // these system types are currently handled by reusing isSecond flag
        // also this system is not dual-able
        if (song.systemFlags[i].getInt("chipType",0)==1) {
          if (!hasRFC1) {
            hasRFC1=disCont[i].dispatch->chipClock;
            isSecond[i]=true;
            willExport[i]=true;
            writeRF5C68[1]=disCont[i].dispatch;
          }
        } else if (!hasRFC) {
          hasRFC=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeRF5C68[0]=disCont[i].dispatch;
        }
        break;
      case DIV_SYSTEM_MSM6295:
        if (!hasOKIM6295) {
          hasOKIM6295=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeMSM6295[0]=disCont[i].dispatch;
        } else if (!(hasOKIM6295&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeMSM6295[1]=disCont[i].dispatch;
          hasOKIM6295|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_GA20:
        if (!hasGA20) {
          hasGA20=disCont[i].dispatch->chipClock;
          willExport[i]=true;
          writeGA20[0]=disCont[i].dispatch;
        } else if (!(hasGA20&0x40000000)) {
          isSecond[i]=true;
          willExport[i]=true;
          writeGA20[1]=disCont[i].dispatch;
          hasGA20|=0x40000000;
          howManyChips++;
        }
        break;
      case DIV_SYSTEM_T6W28:
        if (!hasSN) {
          hasSN=0xc0000000|disCont[i].dispatch->chipClock;
          CHIP_VOL(0,1.0);
          snNoiseConfig=3;
          snNoiseSize=15;
          willExport[i]=true;
        }
        break;
      default:
        break;
    }
    if (willExport[i]) {
      disCont[i].dispatch->toggleRegisterDump(true);
    }
  }

  // write chips and stuff
  w->writeI(hasSN);
  w->writeI(hasOPLL);
  w->writeI(0);
  w->writeI(0); // length. will be written later
  w->writeI(0); // loop. will be written later
  w->writeI(0); // loop length. why is this necessary?
  w->writeI(0); // tick rate
  w->writeS(snNoiseConfig);
  w->writeC(snNoiseSize);
  if (version>=0x151) {
    w->writeC(snFlags);
  } else {
    w->writeC(0);
  }
  w->writeI(hasOPN2);
  w->writeI(hasOPM);
  w->writeI(0); // data pointer. will be written later
  if (version>=0x151) {
    w->writeI(hasSegaPCM);
    w->writeI(segaPCMOffset);
    w->writeI(hasRFC);
    w->writeI(hasOPN);
    w->writeI(hasOPNA);
    w->writeI(hasOPNB);
    w->writeI(hasOPL2);
    w->writeI(hasOPL);
    w->writeI(hasY8950);
    w->writeI(hasOPL3);
    w->writeI(hasOPL4);
    w->writeI(hasOPX);
    w->writeI(hasZ280);
    w->writeI(hasRFC1);
    w->writeI(hasPWM);
    w->writeI(hasAY);
    w->writeC(ayConfig);
    w->writeC(ayFlags);
    w->writeC(ayFlags); // OPN
    w->writeC(ayFlags); // OPNA
  } else {
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeC(0);
    w->writeC(0);
    w->writeC(0); // OPN
    w->writeC(0); // OPNA
  }
  if (version>=0x160) {
    int calcVolume=32.0*(log(song.masterVol)/log(2.0));
    if (calcVolume<-63) calcVolume=-63;
    if (calcVolume>192) calcVolume=192;
    w->writeC(calcVolume&0xff); // volume
  } else {
    w->writeC(0); // volume
  }
  // currently not used but is part of 1.60
  w->writeC(0); // reserved
  w->writeC(0); // loop count
  // 1.51
  w->writeC(0); // loop modifier
  
  if (version>=0x161) {
    w->writeI(hasGB);
    w->writeI(hasNES);
    w->writeI(hasMultiPCM);
    w->writeI(hasuPD7759);
    w->writeI(hasOKIM6258);
    w->writeC(0); // flags
    w->writeC(0); // K flags
    w->writeC(0); // C140 chip type
    w->writeC(0); // reserved
    w->writeI(hasOKIM6295);
    w->writeI(hasK051649);
    w->writeI(hasK054539);
    w->writeI(hasPCE);
    w->writeI(hasNamco);
    w->writeI(hasK053260);
    w->writeI(hasPOKEY);
    w->writeI(hasQSound);
  } else {
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeC(0); // flags
    w->writeC(0); // K flags
    w->writeC(0); // C140 chip type
    w->writeC(0); // reserved
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
  }
  if (version>=0x171) {
    w->writeI(hasSCSP);
  } else {
    w->writeI(0);
  }
  // 1.70
  w->writeI(0); // extra header
  // 1.71
  if (version>=0x171) {
    w->writeI(hasSwan);
    w->writeI(hasVSU);
    w->writeI(hasSAA);
    w->writeI(hasES5503);
    w->writeI(hasES5505);
    w->writeC(0); // 5503 chans
    w->writeC(0); // 5505 chans
    w->writeC(0); // C352 clock divider
    w->writeC(0); // reserved
    w->writeI(hasX1);
    w->writeI(hasC352);
    w->writeI(hasGA20);
    w->writeI(hasLynx);
  } else {
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeC(0); // 5503 chans
    w->writeC(0); // 5505 chans
    w->writeC(0); // C352 clock divider
    w->writeC(0); // reserved
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
    w->writeI(0);
  }
  for (int i=0; i<6; i++) { // reserved
    w->writeI(0);
  }

  unsigned int exHeaderOff=w->tell();
  if (version>=0x170) {
    logD("writing extended header...");
    w->writeI(12);
    w->writeI(0);
    w->writeI(4);

    // write chip volumes
    logD("writing chip volumes (%ld)...",chipVol.size());
    w->writeC(chipVol.size());
    for (unsigned int& i: chipVol) {
      logV("- %.8x",i);
      w->writeI(i);
    }
  }

  unsigned int songOff=w->tell();

  // initialize sample offsets
  memset(sampleOff8,0,256*sizeof(unsigned int));
  memset(sampleOffSegaPCM,0,256*sizeof(unsigned int));

  // write samples
  unsigned int sampleSeek=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    logI("setting seek to %d",sampleSeek);
    sampleOff8[i]=sampleSeek;
    sampleSeek+=sample->length8;
  }

  if (writeDACSamples && !directStream) for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(0);
    w->writeI(sample->length8);
    for (unsigned int j=0; j<sample->length8; j++) {
      w->writeC((unsigned char)sample->data8[j]+0x80);
    }
  }

  if (writeNESSamples && !directStream) for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(7);
    w->writeI(sample->length8);
    for (unsigned int j=0; j<sample->length8; j++) {
      w->writeC(((unsigned char)sample->data8[j]+0x80)>>1);
    }
  }

  if (writePCESamples && !directStream) for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    w->writeC(0x67);
    w->writeC(0x66);
    w->writeC(5);
    w->writeI(sample->length8);
    for (unsigned int j=0; j<sample->length8; j++) {
      w->writeC(((unsigned char)sample->data8[j]+0x80)>>3);
    }
  }

  if (writeSegaPCM>0) {
    unsigned char* pcmMem=new unsigned char[16777216];

    size_t memPos=0;
    for (int i=0; i<song.sampleLen; i++) {
      DivSample* sample=song.sample[i];
      unsigned int alignedSize=(sample->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)+0xff)&(~0xff);
      if (alignedSize>65536) alignedSize=65536;
      if ((memPos&0xff0000)!=((memPos+alignedSize)&0xff0000)) {
        memPos=(memPos+0xffff)&0xff0000;
      }
      logV("- sample %d will be at %x with length %x",i,memPos,alignedSize);
      if (memPos>=16777216) break;
      sampleOffSegaPCM[i]=memPos;
      unsigned int readPos=0;
      for (unsigned int j=0; j<alignedSize; j++) {
        if (readPos>=(unsigned int)sample->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)) {
          if (sample->isLoopable()) {
            readPos=sample->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT);
            pcmMem[memPos++]=((unsigned char)sample->data8[readPos]+0x80);
          } else {
            pcmMem[memPos++]=0x80;
          }
        } else {
          pcmMem[memPos++]=((unsigned char)sample->data8[readPos]+0x80);
        }
        readPos++;
        if (memPos>=16777216) break;
      }
      sample->loopOffP=readPos-sample->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT);
      if (memPos>=16777216) break;
    }

    for (int i=0; i<writeSegaPCM; i++) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x80);
      w->writeI((memPos+8)|(i*0x80000000));
      w->writeI(memPos);
      w->writeI(0);
      w->write(pcmMem,memPos);
    }

    delete[] pcmMem;
  }

  for (int i=0; i<2; i++) {
    // ADPCM (OPNA)
    if (writeADPCM_OPNA[i]!=NULL && writeADPCM_OPNA[i]->getSampleMemUsage(0)>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x81);
      w->writeI((writeADPCM_OPNA[i]->getSampleMemUsage(0)+8)|(i*0x80000000));
      w->writeI(writeADPCM_OPNA[i]->getSampleMemCapacity(0));
      w->writeI(0);
      w->write(writeADPCM_OPNA[i]->getSampleMem(0),writeADPCM_OPNA[i]->getSampleMemUsage(0));
    }
    // ADPCM-A (OPNB)
    if (writeADPCM_OPNB[i]!=NULL && writeADPCM_OPNB[i]->getSampleMemUsage(0)>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x82);
      w->writeI((writeADPCM_OPNB[i]->getSampleMemUsage(0)+8)|(i*0x80000000));
      w->writeI(writeADPCM_OPNB[i]->getSampleMemCapacity(0));
      w->writeI(0);
      w->write(writeADPCM_OPNB[i]->getSampleMem(0),writeADPCM_OPNB[i]->getSampleMemUsage(0));
    }
    // ADPCM-B (OPNB)
    if (writeADPCM_OPNB[i]!=NULL && writeADPCM_OPNB[i]->getSampleMemUsage(1)>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x83);
      w->writeI((writeADPCM_OPNB[i]->getSampleMemUsage(1)+8)|(i*0x80000000));
      w->writeI(writeADPCM_OPNB[i]->getSampleMemCapacity(1));
      w->writeI(0);
      w->write(writeADPCM_OPNB[i]->getSampleMem(1),writeADPCM_OPNB[i]->getSampleMemUsage(1));
    }
    // ADPCM (Y8950)
    if (writeADPCM_Y8950[i]!=NULL && writeADPCM_Y8950[i]->getSampleMemUsage(0)>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x88);
      w->writeI((writeADPCM_Y8950[i]->getSampleMemUsage(0)+8)|(i*0x80000000));
      w->writeI(writeADPCM_Y8950[i]->getSampleMemCapacity(0));
      w->writeI(0);
      w->write(writeADPCM_Y8950[i]->getSampleMem(0),writeADPCM_Y8950[i]->getSampleMemUsage(0));
    }
    if (writeQSound[i]!=NULL && writeQSound[i]->getSampleMemUsage()>0) {
      unsigned int blockSize=(writeQSound[i]->getSampleMemUsage()+0xffff)&(~0xffff);
      if (blockSize > 0x1000000) {
        blockSize = 0x1000000;
      }
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x8F);
      w->writeI((blockSize+8)|(i*0x80000000));
      w->writeI(writeQSound[i]->getSampleMemCapacity());
      w->writeI(0);
      w->write(writeQSound[i]->getSampleMem(),blockSize);
    }
    if (writeX1010[i]!=NULL && writeX1010[i]->getSampleMemUsage()>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x91);
      w->writeI((writeX1010[i]->getSampleMemUsage()+8)|(i*0x80000000));
      w->writeI(writeX1010[i]->getSampleMemCapacity());
      w->writeI(0);
      w->write(writeX1010[i]->getSampleMem(),writeX1010[i]->getSampleMemUsage());
    }
    if (writeZ280[i]!=NULL && writeZ280[i]->getSampleMemUsage()>0) {
      // In VGM, YMZ280B's 16-bit PCM has an endianness swapped
      // which have been fixed in the upstream MAME since 2013
      // in order to get Konami FireBeat working
      // The reason given for VGM not applying this change was
      // "It matches OPL4 and MAME probably did an endianness optimization"
      size_t sampleMemLen=writeZ280[i]->getSampleMemUsage();
      unsigned char* sampleMem=new unsigned char[sampleMemLen];
      memcpy(sampleMem,writeZ280[i]->getSampleMem(),sampleMemLen);
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x86);
      w->writeI((writeZ280[i]->getSampleMemUsage()+8)|(i*0x80000000));
      w->writeI(writeZ280[i]->getSampleMemCapacity());
      w->writeI(0);
      w->write(sampleMem,sampleMemLen);
      delete[] sampleMem;
    }
  }

  for (int i=0; i<2; i++) {
    if (writeRF5C68[i]!=NULL && writeRF5C68[i]->getSampleMemUsage()>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0xc0+i);
      w->writeI(writeRF5C68[i]->getSampleMemUsage()+8);
      w->writeI(writeRF5C68[i]->getSampleMemCapacity());
      w->writeI(0);
      w->write(writeRF5C68[i]->getSampleMem(),writeRF5C68[i]->getSampleMemUsage());
    }
    if (writeMSM6295[i]!=NULL && writeMSM6295[i]->getSampleMemUsage()>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x8b);
      w->writeI((writeMSM6295[i]->getSampleMemUsage()+8)|(i*0x80000000));
      w->writeI(writeMSM6295[i]->getSampleMemCapacity());
      w->writeI(0);
      w->write(writeMSM6295[i]->getSampleMem(),writeMSM6295[i]->getSampleMemUsage());
    }
    if (writeGA20[i]!=NULL && writeGA20[i]->getSampleMemUsage()>0) {
      w->writeC(0x67);
      w->writeC(0x66);
      w->writeC(0x93);
      w->writeI((writeGA20[i]->getSampleMemUsage()+8)|(i*0x80000000));
      w->writeI(writeGA20[i]->getSampleMemCapacity());
      w->writeI(0);
      w->write(writeGA20[i]->getSampleMem(),writeGA20[i]->getSampleMemUsage());
    }
  }

  // initialize streams
  int streamID=0;
  if (!directStream) {
    for (int i=0; i<song.systemLen; i++) {
      if (!willExport[i]) continue;
      streamIDs[i]=streamID;
      switch (song.system[i]) {
        case DIV_SYSTEM_YM2612:
        case DIV_SYSTEM_YM2612_EXT:
        case DIV_SYSTEM_YM2612_DUALPCM:
        case DIV_SYSTEM_YM2612_DUALPCM_EXT:
          w->writeC(0x90);
          w->writeC(streamID);
          w->writeC(0x02);
          w->writeC(0); // port
          w->writeC(0x2a); // DAC

          w->writeC(0x91);
          w->writeC(streamID);
          w->writeC(0);
          w->writeC(1);
          w->writeC(0);

          w->writeC(0x92);
          w->writeC(streamID);
          w->writeI(32000); // default
          streamID++;
          break;
        case DIV_SYSTEM_NES:
          w->writeC(0x90);
          w->writeC(streamID);
          w->writeC(20);
          w->writeC(0); // port
          w->writeC(0x11); // DAC

          w->writeC(0x91);
          w->writeC(streamID);
          w->writeC(7);
          w->writeC(1);
          w->writeC(0);

          w->writeC(0x92);
          w->writeC(streamID);
          w->writeI(32000); // default
          streamID++;
          break;
        case DIV_SYSTEM_PCE:
          for (int j=0; j<6; j++) {
            w->writeC(0x90);
            w->writeC(streamID);
            w->writeC(27);
            w->writeC(j); // port
            w->writeC(0x06); // select+DAC

            w->writeC(0x91);
            w->writeC(streamID);
            w->writeC(5);
            w->writeC(1);
            w->writeC(0);

            w->writeC(0x92);
            w->writeC(streamID);
            w->writeI(16000); // default
            streamID++;
          }
          break;
        case DIV_SYSTEM_SWAN:
          w->writeC(0x90);
          w->writeC(streamID);
          w->writeC(isSecond[i]?0xa1:0x21);
          w->writeC(0); // port
          w->writeC(0x09); // DAC

          w->writeC(0x91);
          w->writeC(streamID);
          w->writeC(0);
          w->writeC(1);
          w->writeC(0);

          w->writeC(0x92);
          w->writeC(streamID);
          w->writeI(24000); // default
          streamID++;
          break;
        default:
          break;
      }
    }
  }

  // write song data
  playSub(false);
  size_t tickCount=0;
  bool writeLoop=false;
  int ord=-1;
  int exportChans=0;
  for (int i=0; i<chans; i++) {
    if (!willExport[dispatchOfChan[i]]) continue;
    exportChans++;
  }
  while (!done) {
    if (loopPos==-1) {
      if (loopOrder==curOrder && loopRow==curRow && ticks==1) {
        writeLoop=true;
      }
    }
    if (nextTick(false,true) || !playing) {
      done=true;
      if (!loop) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].dispatch->getRegisterWrites().clear();
        }
        break;
      }
      // stop all streams
      if (!directStream) {
        for (int i=0; i<streamID; i++) {
          w->writeC(0x94);
          w->writeC(i);
          loopSample[i]=-1;
        }
      }

      if (!playing) {
        writeLoop=false;
        loopPos=-1;
      }
    } else {
      // check for pattern change
      if (prevOrder!=ord) {
        logI("registering order change %d on %d",prevOrder, prevRow);
        ord=prevOrder;

        if (patternHints) {
          w->writeC(0x67);
          w->writeC(0x66);
          w->writeC(0xfe);
          w->writeI(3+exportChans);
          w->writeC(0x01);
          w->writeC(prevOrder);
          w->writeC(prevRow);
          for (int i=0; i<chans; i++) {
            if (!willExport[dispatchOfChan[i]]) continue;
            w->writeC(curSubSong->orders.ord[i][prevOrder]);
          }
        }
      }
    }
    // get register dumps
    for (int i=0; i<song.systemLen; i++) {
      std::vector<DivRegWrite>& writes=disCont[i].dispatch->getRegisterWrites();
      for (DivRegWrite& j: writes) {
        performVGMWrite(w,song.system[i],j,streamIDs[i],loopTimer,loopFreq,loopSample,sampleDir,isSecond[i],directStream);
        writeCount++;
      }
      writes.clear();
    }
    // check whether we need to loop
    int totalWait=cycles>>MASTER_CLOCK_PREC;
    if (directStream) {
      // render stream of all chips
      for (int i=0; i<song.systemLen; i++) {
        disCont[i].dispatch->fillStream(delayedWrites[i],44100,totalWait);
        for (DivDelayedWrite& j: delayedWrites[i]) {
          sortedWrites.push_back(std::pair<int,DivDelayedWrite>(i,j));
        }
        delayedWrites[i].clear();
      }

      if (!sortedWrites.empty()) {
        // sort if more than one chip
        if (song.systemLen>1) {
          std::sort(sortedWrites.begin(),sortedWrites.end(),[](const std::pair<int,DivDelayedWrite>& a, const std::pair<int,DivDelayedWrite>& b) -> bool {
            return a.second.time<b.second.time;
          });
        }

        // write it out
        int lastOne=0;
        for (std::pair<int,DivDelayedWrite>& i: sortedWrites) {
          if (i.second.time>lastOne) {
            // write delay
            int delay=i.second.time-lastOne;
            if (delay>16) {
              w->writeC(0x61);
              w->writeS(totalWait);
            } else if (delay>0) {
              w->writeC(0x70+delay-1);
            }
            lastOne=i.second.time;
          }
          // write write
          performVGMWrite(w,song.system[i.first],i.second.write,streamIDs[i.first],loopTimer,loopFreq,loopSample,sampleDir,isSecond[i.first],directStream);
          writeCount++;
        }
        sortedWrites.clear();
        totalWait-=lastOne;
        tickCount+=lastOne;
      }
    } else {
      for (int i=0; i<streamID; i++) {
        if (loopSample[i]>=0) {
          loopTimer[i]-=(loopFreq[i]/44100.0)*(double)totalWait;
        }
      }
      bool haveNegatives=false;
      for (int i=0; i<streamID; i++) {
        if (loopSample[i]>=0) {
          if (loopTimer[i]<0) {
            haveNegatives=true;
          }
        }
      }
      while (haveNegatives) {
        // finish all negatives
        int nextToTouch=-1;
        for (int i=0; i<streamID; i++) {
          if (loopSample[i]>=0) {
            if (loopTimer[i]<0) {
              if (nextToTouch>=0) {
                if (loopTimer[nextToTouch]>loopTimer[i]) nextToTouch=i;
              } else {
                nextToTouch=i;
              }
            }
          }
        }
        if (nextToTouch>=0) {
          double waitTime=totalWait+(loopTimer[nextToTouch]*(44100.0/MAX(1,loopFreq[nextToTouch])));
          if (waitTime>0) {
            w->writeC(0x61);
            w->writeS(waitTime);
            logV("wait is: %f",waitTime);
            totalWait-=waitTime;
            tickCount+=waitTime;
          }
          if (loopSample[nextToTouch]<song.sampleLen) {
            DivSample* sample=song.sample[loopSample[nextToTouch]];
            // insert loop
            if (sample->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT)<sample->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)) {
              w->writeC(0x93);
              w->writeC(nextToTouch);
              w->writeI(sampleOff8[loopSample[nextToTouch]]+sample->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT));
              w->writeC(0x81);
              w->writeI(sample->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)-sample->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT));
            }
          }
          loopSample[nextToTouch]=-1;
        } else {
          haveNegatives=false;
        }
      }
    }
    // write wait
    if (totalWait>0) {
      if (totalWait==735) {
        w->writeC(0x62);
      } else if (totalWait==882) {
        w->writeC(0x63);
      } else {
        w->writeC(0x61);
        w->writeS(totalWait);
      }
      tickCount+=totalWait;
    }
    if (writeLoop) {
      writeLoop=false;
      loopPos=w->tell();
      loopTick=tickCount;
    }
  }
  // end of song
  w->writeC(0x66);

  got.rate=origRate;

  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->toggleRegisterDump(false);
  }

  // write GD3 tag
  gd3Off=w->tell();
  w->write("Gd3 ",4);
  w->writeI(0x100);
  w->writeI(0); // length. will be written later

  WString ws;
  ws=utf8To16(song.name.c_str());
  w->writeWString(ws,false); // name
  ws=utf8To16(song.nameJ.c_str());
  w->writeWString(ws,false); // japanese name
  ws=utf8To16(song.category.c_str());
  w->writeWString(ws,false); // game name
  ws=utf8To16(song.categoryJ.c_str());
  w->writeWString(ws,false); // japanese game name
  ws=utf8To16(song.systemName.c_str());
  w->writeWString(ws,false); // system name
  ws=utf8To16(song.systemNameJ.c_str());
  w->writeWString(ws,false); // japanese system name
  ws=utf8To16(song.author.c_str());
  w->writeWString(ws,false); // author name
  ws=utf8To16(song.authorJ.c_str());
  w->writeWString(ws,false); // japanese author name
  w->writeS(0); // date
  w->writeWString(L"Furnace Tracker",false); // ripper
  w->writeS(0); // notes

  int gd3Len=w->tell()-gd3Off-12;

  w->seek(gd3Off+8,SEEK_SET);
  w->writeI(gd3Len);

  // finish file
  size_t len=w->size()-4;
  w->seek(4,SEEK_SET);
  w->writeI(len);
  w->seek(0x14,SEEK_SET);
  w->writeI(gd3Off-0x14);
  w->writeI(tickCount);
  if (loop) {
    if (loopPos==-1) {
      w->writeI(0);
      w->writeI(0);
    } else {
      w->writeI(loopPos-0x1c);
      w->writeI(tickCount-loopTick-1);
    }
  } else {
    w->writeI(0);
    w->writeI(0);
  }
  w->seek(0x34,SEEK_SET);
  w->writeI(songOff-0x34);
  if (version>=0x170) {
    w->seek(0xbc,SEEK_SET);
    w->writeI(exHeaderOff-0xbc);
  }

  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;

  logI("%d register writes total.",writeCount);

  BUSY_END;
  return w;
}
