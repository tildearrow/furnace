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

#include "opll.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 295017

const char* DivPlatformOPLL::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xy: Setup LFO (x: enable; y: speed)";
      break;
    case 0x11:
      return "11xx: Set feedback (0 to 7)";
      break;
    case 0x12:
      return "12xx: Set level of operator 1 (0 highest, 7F lowest)";
      break;
    case 0x13:
      return "13xx: Set level of operator 2 (0 highest, 7F lowest)";
      break;
    case 0x14:
      return "14xx: Set level of operator 3 (0 highest, 7F lowest)";
      break;
    case 0x15:
      return "15xx: Set level of operator 4 (0 highest, 7F lowest)";
      break;
    case 0x16:
      return "16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)";
      break;
    case 0x17:
      return "17xx: Enable channel 6 DAC";
      break;
    case 0x18:
      return "18xx: Toggle extended channel 3 mode";
      break;
    case 0x19:
      return "19xx: Set attack of all operators (0 to 1F)";
      break;
    case 0x1a:
      return "1Axx: Set attack of operator 1 (0 to 1F)";
      break;
    case 0x1b:
      return "1Bxx: Set attack of operator 2 (0 to 1F)";
      break;
    case 0x1c:
      return "1Cxx: Set attack of operator 3 (0 to 1F)";
      break;
    case 0x1d:
      return "1Dxx: Set attack of operator 4 (0 to 1F)";
      break;
  }
  return NULL;
}

const unsigned char cycleMapOPLL[18]={
  1, 2, 0, 1, 2, 3, 4, 5, 3, 4, 5, 6, 7, 8, 6, 7, 8, 0
};

void DivPlatformOPLL::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  static int o[2];
  static int os;

  for (size_t h=start; h<start+len; h++) {
    os=0;
    for (int i=0; i<9; i++) {
      if (!writes.empty() && --delay<0) {
        // 84 is safe value
        QueuedWrite& w=writes.front();
        if (w.addrOrVal) {
          OPLL_Write(&fm,1,w.val);
          //printf("write: %x = %.2x\n",w.addr,w.val);
          regPool[w.addr&0xff]=w.val;
          writes.pop();
          delay=21;
        } else {
          //printf("busycounter: %d\n",lastBusy);
          OPLL_Write(&fm,0,w.addr);
          w.addrOrVal=true;
          delay=3;
        }
      }
      
      unsigned char nextOut=cycleMapOPLL[(fm.cycles+17)%18];
      OPLL_Clock(&fm,o);
      if (!isMuted[nextOut]) {
        os+=(o[0]+o[1]);
      }
    }
    os*=50;
    if (os<-32768) os=-32768;
    if (os>32767) os=32767;
    bufL[h]=os;
  }
}

void DivPlatformOPLL::acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len) {
}

void DivPlatformOPLL::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  acquire_nuked(bufL,bufR,start,len);
}

void DivPlatformOPLL::tick() {
  for (int i=0; i<9; i++) {
    chan[i].std.next();

    /*if (chan[i].std.hadVol) {
      chan[i].outVol=(chan[i].vol*MIN(127,chan[i].std.vol))/127;
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
    }

    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].std.arp);
        } else {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note+(signed char)chan[i].std.arp);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
      }
    }

    if (chan[i].std.hadAlg) {
      chan[i].state.alg=chan[i].std.alg;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      if (!parent->song.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
    }
    if (chan[i].std.hadFb) {
      chan[i].state.fb=chan[i].std.fb;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    }
    if (chan[i].std.hadFms) {
      chan[i].state.fms=chan[i].std.fms;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].std.hadAms) {
      chan[i].state.ams=chan[i].std.ams;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    for (int j=0; j<2; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      DivMacroInt::IntOp& m=chan[i].std.op[j];
      if (m.hadAm) {
        op.am=m.am;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.hadAr) {
        op.ar=m.ar;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      if (m.hadDr) {
        op.dr=m.dr;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.hadMult) {
        op.mult=m.mult;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      if (m.hadRr) {
        op.rr=m.rr;
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      if (m.hadSl) {
        op.sl=m.sl;
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      if (m.hadTl) {
        op.tl=127-m.tl;
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
      if (m.hadRs) {
        op.rs=m.rs;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      if (m.hadDt) {
        op.dt=m.dt;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      if (m.hadD2r) {
        op.d2r=m.d2r;
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      }
      if (m.hadSsg) {
        op.ssgEnv=m.ssg;
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
      }
    }*/

    if (chan[i].keyOn || chan[i].keyOff) {
      if (i>=6 && drums) {
        drumState&=~(0x10>>(chan[i].note%12));
        immWrite(0x0e,0x20|drumState);
      } else {
        immWrite(0x20+i,(chan[i].freqH)/*|(chan[i].state.alg?0x20:0)*/);
      }
      //chan[i].keyOn=false;
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<9; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,octave(chan[i].baseFreq));
      if (chan[i].freq>262143) chan[i].freq=262143;
      int freqt=toFreq(chan[i].freq);
      chan[i].freqH=freqt>>8;
      chan[i].freqL=freqt&0xff;
      if (i<6 || !drums) {
        immWrite(0x10+i,freqt&0xff);
      }
    }
    if (chan[i].keyOn && i>=6 && drums) {
      //printf("%d\n",chan[i].note%12);
      drumState|=(0x10>>(chan[i].note%12));
      immWrite(0x0e,0x20|drumState);
      chan[i].keyOn=false;
    } else if (chan[i].keyOn || chan[i].freqChanged) {
      //immWrite(0x28,0xf0|konOffs[i]);
      immWrite(0x20+i,(chan[i].freqH)|(chan[i].active<<4)|(chan[i].state.alg?0x20:0));
      chan[i].keyOn=false;
    }
    chan[i].freqChanged=false;
  }
}

#define OPLL_C_NUM 343

int DivPlatformOPLL::octave(int freq) {
  if (freq>=OPLL_C_NUM*64) {
    return 128;
  } else if (freq>=OPLL_C_NUM*32) {
    return 64;
  } else if (freq>=OPLL_C_NUM*16) {
    return 32;
  } else if (freq>=OPLL_C_NUM*8) {
    return 16;
  } else if (freq>=OPLL_C_NUM*4) {
    return 8;
  } else if (freq>=OPLL_C_NUM*2) {
    return 4;
  } else if (freq>=OPLL_C_NUM) {
    return 2;
  } else {
    return 1;
  }
  return 1;
}

int DivPlatformOPLL::toFreq(int freq) {
  if (freq>=OPLL_C_NUM*64) {
    return 0xe00|((freq>>7)&0x1ff);
  } else if (freq>=OPLL_C_NUM*32) {
    return 0xc00|((freq>>6)&0x1ff);
  } else if (freq>=OPLL_C_NUM*16) {
    return 0xa00|((freq>>5)&0x1ff);
  } else if (freq>=OPLL_C_NUM*8) {
    return 0x800|((freq>>4)&0x1ff);
  } else if (freq>=OPLL_C_NUM*4) {
    return 0x600|((freq>>3)&0x1ff);
  } else if (freq>=OPLL_C_NUM*2) {
    return 0x400|((freq>>2)&0x1ff);
  } else if (freq>=OPLL_C_NUM) {
    return 0x200|((freq>>1)&0x1ff);
  } else {
    return freq&0x1ff;
  }
}

void DivPlatformOPLL::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  /*
  for (int j=0; j<4; j++) {
    unsigned short baseAddr=chanOffs[ch]|opOffs[j];
    DivInstrumentFM::Operator& op=chan[ch].state.op[j];
    if (isMuted[ch]) {
      rWrite(baseAddr+ADDR_TL,127);
    } else {
      if (isOutput[chan[ch].state.alg][j]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[ch].outVol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
    }
  }
  rWrite(chanOffs[ch]+ADDR_LRAF,(isMuted[ch]?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));*/
}

int DivPlatformOPLL::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }

      chan[c.chan].std.init(ins);
      if (!chan[c.chan].std.willVol) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      
      if (chan[c.chan].insChanged) {
        // update custom preset
        if (chan[c.chan].state.opllPreset==0) {
          DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
          DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
          rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
          rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
          rWrite(0x02,(car.ksl<<6)|(mod.tl&63));
          rWrite(0x03,(mod.ksl<<6)|((chan[c.chan].state.fms&1)<<4)|((chan[c.chan].state.ams&1)<<3)|chan[c.chan].state.fb);
          rWrite(0x04,(mod.ar<<4)|(mod.dr));
          rWrite(0x05,(car.ar<<4)|(car.dr));
          rWrite(0x06,(mod.sl<<4)|(mod.rr));
          rWrite(0x07,(car.sl<<4)|(car.rr));
        }
        if (chan[c.chan].state.opllPreset==16) { // compatible drums mode
          if (c.chan>=6) {
            drums=true;
            immWrite(0x16,0x20);
            immWrite(0x26,0x05);
            immWrite(0x16,0x20);
            immWrite(0x26,0x05);
            immWrite(0x17,0x50);
            immWrite(0x27,0x05);
            immWrite(0x17,0x50);
            immWrite(0x27,0x05);
            immWrite(0x18,0xC0);
            immWrite(0x28,0x01);
          }
        } else {
          if (c.chan>=6) {
            drums=false;
            immWrite(0x0e,0);
          }
          rWrite(0x30+c.chan,(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15)|(chan[c.chan].state.opllPreset<<4));
        }
      }

      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].note=c.value;

        if (c.chan>=6 && drums) {
          switch (chan[c.chan].note%12) {
            case 0: // kick
              drumVol[0]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            case 1: // snare
              drumVol[1]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            case 2: // tom
              drumVol[2]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            case 3: // top
              drumVol[3]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            default: // hi-hat
              drumVol[4]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
          }
          rWrite(0x36,drumVol[0]);
          rWrite(0x37,drumVol[1]|(drumVol[4]<<4));
          rWrite(0x38,drumVol[3]|(drumVol[2]<<4));
        }
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.hasVol) {
        chan[c.chan].outVol=c.value;
      }
      if (c.chan<6 || !drums) {
        rWrite(0x30+c.chan,(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15)|(chan[c.chan].state.opllPreset<<4));
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
      int newFreq;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*octave(chan[c.chan].baseFreq);
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*octave(chan[c.chan].baseFreq);
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      /*if (!chan[c.chan].portaPause) {
        if (octave(chan[c.chan].baseFreq)!=octave(newFreq)) {
          chan[c.chan].portaPause=true;
          break;
        }
      }*/
      chan[c.chan].baseFreq=newFreq;
      chan[c.chan].portaPause=false;
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_FB: {
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      //DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      chan[c.chan].state.fb=c.value&7;
      rWrite(0x03,(mod.ksl<<6)|((chan[c.chan].state.fms&1)<<4)|((chan[c.chan].state.ams&1)<<3)|chan[c.chan].state.fb);
      break;
    }
    
    case DIV_CMD_FM_MULT: {
      if (c.value==0) {
        DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
        mod.mult=c.value2&15;
        rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      } else {
        DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
        car.mult=c.value2&15;
        rWrite(0x30+c.chan,(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15)|(chan[c.chan].state.opllPreset<<4));
      }
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.value==0) {
        DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
        DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
        mod.tl=c.value2&63;
        rWrite(0x02,(car.ksl<<6)|(mod.tl&63));
      } else {
        DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
        car.tl=c.value2&15;
        rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.ar=c.value2&15;
        car.ar=c.value2&15;
      } else {
        if (c.value==0) {
          mod.ar=c.value2&15;
        } else {
          car.ar=c.value2&15;
        }
      }
      rWrite(0x04,(mod.ar<<4)|(mod.dr));
      rWrite(0x05,(car.ar<<4)|(car.dr));
      break;
    }
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformOPLL::forceIns() {
  for (int i=0; i<9; i++) {
    // update custom preset
    if (chan[i].state.opllPreset==0) {
      DivInstrumentFM::Operator& mod=chan[i].state.op[0];
      DivInstrumentFM::Operator& car=chan[i].state.op[1];
      rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      rWrite(0x02,(car.ksl<<6)|(mod.tl&63));
      rWrite(0x03,(mod.ksl<<6)|((chan[i].state.fms&1)<<4)|((chan[i].state.ams&1)<<3)|chan[i].state.fb);
      rWrite(0x04,(mod.ar<<4)|(mod.dr));
      rWrite(0x05,(car.ar<<4)|(car.dr));
      rWrite(0x06,(mod.sl<<4)|(mod.rr));
      rWrite(0x07,(car.sl<<4)|(car.rr));
    }
    rWrite(0x30+i,(15-(chan[i].outVol*(15-chan[i].state.op[1].tl))/15)|(chan[i].state.opllPreset<<4));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  if (drums) {
    immWrite(0x16,0x20);
    immWrite(0x26,0x05);
    immWrite(0x16,0x20);
    immWrite(0x26,0x05);
    immWrite(0x17,0x50);
    immWrite(0x27,0x05);
    immWrite(0x17,0x50);
    immWrite(0x27,0x05);
    immWrite(0x18,0xC0);
    immWrite(0x28,0x01);
  }
}

void DivPlatformOPLL::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void DivPlatformOPLL::setVRC7(bool vrc) {
  vrc7=vrc;
}

void* DivPlatformOPLL::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformOPLL::getRegisterPool() {
  return regPool;
}

int DivPlatformOPLL::getRegisterPoolSize() {
  return 64;
}

void DivPlatformOPLL::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,256);
  if (vrc7) {
    OPLL_Reset(&fm,opll_type_ds1001);
  } else {
    OPLL_Reset(&fm,opll_type_ym2413);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<9; i++) {
    chan[i]=DivPlatformOPLL::Channel();
    chan[i].vol=15;
    chan[i].outVol=15;
  }

  for (int i=0; i<256; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  drumState=0;

  drumVol[0]=0;
  drumVol[1]=0;
  drumVol[2]=0;
  drumVol[3]=0;
  drumVol[4]=0;
  
  delay=0;
}

bool DivPlatformOPLL::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformOPLL::keyOffAffectsPorta(int ch) {
  return false;
}

void DivPlatformOPLL::notifyInsChange(int ins) {
  for (int i=0; i<9; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformOPLL::notifyInsDeletion(void* ins) {
}

void DivPlatformOPLL::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformOPLL::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformOPLL::getPortaFloor(int ch) {
  return (ch>5)?12:0;
}

void DivPlatformOPLL::setYMFM(bool use) {
  useYMFM=use;
}

void DivPlatformOPLL::setFlags(unsigned int flags) {
  if (flags==3) {
    chipClock=COLOR_NTSC;
  } else if (flags==2) {
    chipClock=8000000.0;
  } else if (flags==1) {
    chipClock=COLOR_PAL*1.0/5.0;
  } else {
    chipClock=COLOR_NTSC/4.0;
  }
  rate=chipClock/9;
}

int DivPlatformOPLL::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<9; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);

  reset();
  return 10;
}

void DivPlatformOPLL::quit() {
}

DivPlatformOPLL::~DivPlatformOPLL() {
}
