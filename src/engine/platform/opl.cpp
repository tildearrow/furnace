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

#include "opl.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 9440540

// N = invalid
#define N 255

const unsigned char slotsOPL2i[4][20]={
  {0, 1, 2, 6,  7,  8, 12, 13, 14}, // OP1
  {3, 4, 5, 9, 10, 11, 15, 16, 17}, // OP2
  {N, N, N, N,  N,  N,  N,  N,  N},
  {N, N, N, N,  N,  N,  N,  N,  N}
};

const unsigned char slotsOPL2Drumsi[4][20]={
  {0, 1, 2, 6,  7,  8, 12, 16, 14, 17, 13}, // OP1
  {3, 4, 5, 9, 10, 11, 15,  N,  N,  N,  N}, // OP2
  {N, N, N, N,  N,  N,  N,  N,  N,  N,  N},
  {N, N, N, N,  N,  N,  N,  N,  N,  N,  N}
};

const unsigned short chanMapOPL2[20]={
  0, 1, 2, 3, 4, 5, 6, 7, 8, N, N, N, N, N, N, N, N, N, N, N
};

const unsigned char* slotsOPL2[4]={
  slotsOPL2i[0],
  slotsOPL2i[1],
  slotsOPL2i[2],
  slotsOPL2i[3]
};

const unsigned char* slotsOPL2Drums[4]={
  slotsOPL2Drumsi[0],
  slotsOPL2Drumsi[1],
  slotsOPL2Drumsi[2],
  slotsOPL2Drumsi[3]
};

const unsigned char slotsOPL3i[4][20]={
  {0, 6,  1,  7,  2,  8, 18, 24, 19, 25, 20, 26, 30, 31, 32, 12, 13, 14}, // OP1
  {3, 9,  4, 10,  5, 11, 21, 27, 22, 28, 23, 29, 33, 34, 35, 15, 16, 17}, // OP2
  {6, N,  7,  N,  8,  N, 24,  N, 25,  N, 26,  N,  N,  N,  N,  N,  N,  N}, // OP3
  {9, N, 10,  N, 11,  N, 27,  N, 28,  N, 29,  N,  N,  N,  N,  N,  N,  N}  // OP4
};

const unsigned char slotsOPL3Drumsi[4][20]={
  {0, 6,  1,  7,  2,  8, 18, 24, 19, 25, 20, 26, 30, 31, 32, 12, 16, 14, 17, 13}, // OP1
  {3, 9,  4, 10,  5, 11, 21, 27, 22, 28, 23, 29, 33, 34, 35,  N,  N,  N,  N,  N}, // OP2
  {6, N,  7,  N,  8,  N, 24,  N, 25,  N, 26,  N,  N,  N,  N,  N,  N,  N,  N,  N}, // OP3
  {9, N, 10,  N, 11,  N, 27,  N, 28,  N, 29,  N,  N,  N,  N,  N,  N,  N,  N,  N}  // OP4
};

const unsigned short chanMapOPL3[20]={
  0, 3, 1, 4, 2, 5, 0x100, 0x103, 0x101, 0x104, 0x102, 0x105, 0x106, 0x107, 0x108, 6, 7, 8, N, N
};

const unsigned char* slotsOPL3[4]={
  slotsOPL3i[0],
  slotsOPL3i[1],
  slotsOPL3i[2],
  slotsOPL3i[3]
};

const unsigned char* slotsOPL3Drums[4]={
  slotsOPL3Drumsi[0],
  slotsOPL3Drumsi[1],
  slotsOPL3Drumsi[2],
  slotsOPL3Drumsi[3]
};

const unsigned int slotMap[36]={
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15,

  0x100, 0x101, 0x102, 0x103, 0x104, 0x105,
  0x108, 0x109, 0x10a, 0x10b, 0x10c, 0x10d,
  0x110, 0x111, 0x112, 0x113, 0x114, 0x115,
};

const bool isOutputL[2][4][4]={
  { // 2-op
    {false, true, false, false}, // 1 > 2
    { true, true, false, false}, // 1 + 2
    {false, true, false, false}, // ditto, 0
    { true, true, false, false}, // ditto, 1
  },
  { // 4-op
    {false, false, false, true}, // 1 > 2 > 3 > 4
    { true, false, false, true}, // 1 + (2 > 3 > 4)
    {false,  true, false, true}, // (1 > 2) + (3 > 4)
    { true, false,  true, true}  // 1 + (2 > 3) + 4
  }
};

#undef N

const int orderedOpsL[4]={
  0,2,1,3
};

#define ADDR_AM_VIB_SUS_KSR_MULT 0x20
#define ADDR_KSL_TL 0x40
#define ADDR_AR_DR 0x60
#define ADDR_SL_RR 0x80
#define ADDR_WS 0xe0

#define ADDR_FREQ 0xa0
#define ADDR_FREQH 0xb0
#define ADDR_LR_FB_ALG 0xc0

const char* DivPlatformOPL::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xy: Setup LFO (x: enable; y: speed)";
      break;
    case 0x11:
      return "11xx: Set feedback (0 to 7)";
      break;
    case 0x12:
      return "12xx: Set level of operator 1 (0 highest, 3F lowest)";
      break;
    case 0x13:
      return "13xx: Set level of operator 2 (0 highest, 3F lowest)";
      break;
    case 0x14:
      return "14xx: Set level of operator 3 (0 highest, 3F lowest; 4-op only)";
      break;
    case 0x15:
      return "15xx: Set level of operator 4 (0 highest, 3F lowest; 4-op only)";
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
      return "19xx: Set attack of all operators (0 to F)";
      break;
    case 0x1a:
      return "1Axx: Set attack of operator 1 (0 to F)";
      break;
    case 0x1b:
      return "1Bxx: Set attack of operator 2 (0 to F)";
      break;
    case 0x1c:
      return "1Cxx: Set attack of operator 3 (0 to F; 4-op only)";
      break;
    case 0x1d:
      return "1Dxx: Set attack of operator 4 (0 to F; 4-op only)";
      break;
    case 0x20:
      return "20xy: Set PSG noise mode (x: preset freq/ch3 freq; y: thin pulse/noise)";
      break; 
  }
  return NULL;
}

void DivPlatformOPL::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  static short o[2];
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty() && --delay<0) {
      delay=1;
      QueuedWrite& w=writes.front();
      OPL3_WriteReg(&fm,w.addr,w.val);
      regPool[w.addr&511]=w.val;
      writes.pop();
    }
    
    OPL3_Generate(&fm,o); os[0]+=o[0]; os[1]+=o[1];
    
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformOPL::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  //if (useYMFM) {
  //  acquire_ymfm(bufL,bufR,start,len);
  //} else {
    acquire_nuked(bufL,bufR,start,len);
  //}
}

void DivPlatformOPL::tick() {
  for (int i=0; i<20; i++) {
    chan[i].std.next();

    /*
    if (chan[i].std.hadVol) {
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
    for (int j=0; j<4; j++) {
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
    }
    */

    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(chanMap[i]+ADDR_FREQH,0x00|(chan[i].freqH&31));
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<20; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,octave(chan[i].baseFreq));
      if (chan[i].freq>131071) chan[i].freq=131071;
      int freqt=toFreq(chan[i].freq);
      chan[i].freqH=freqt>>8;
      chan[i].freqL=freqt&0xff;
      immWrite(chanMap[i]+ADDR_FREQ,chan[i].freqL);
    }
    if (chan[i].keyOn) {
      immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(0x20));
      chan[i].keyOn=false;
    } else if (chan[i].freqChanged) {
      immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(chan[i].active<<5));
    }
    chan[i].freqChanged=false;
  }
}

#define OPLL_C_NUM 686

int DivPlatformOPL::octave(int freq) {
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

int DivPlatformOPL::toFreq(int freq) {
  if (freq>=OPLL_C_NUM*64) {
    return 0x1c00|((freq>>7)&0x3ff);
  } else if (freq>=OPLL_C_NUM*32) {
    return 0x1800|((freq>>6)&0x3ff);
  } else if (freq>=OPLL_C_NUM*16) {
    return 0x1400|((freq>>5)&0x3ff);
  } else if (freq>=OPLL_C_NUM*8) {
    return 0x1000|((freq>>4)&0x3ff);
  } else if (freq>=OPLL_C_NUM*4) {
    return 0xc00|((freq>>3)&0x3ff);
  } else if (freq>=OPLL_C_NUM*2) {
    return 0x800|((freq>>2)&0x3ff);
  } else if (freq>=OPLL_C_NUM) {
    return 0x400|((freq>>1)&0x3ff);
  } else {
    return freq&0x3ff;
  }
}

void DivPlatformOPL::muteChannel(int ch, bool mute) {
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
  rWrite(chanOffs[ch]+ADDR_LRAF,(isMuted[ch]?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));
  */
}

int DivPlatformOPL::dispatch(DivCommand c) {
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
        int ops=(slots[3][c.chan]!=255 && ins->fm.ops==4)?4:2;
        for (int i=0; i<ops; i++) {
          unsigned short baseAddr=slotMap[slots[i][c.chan]];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];

          if (isMuted[c.chan]) {
            rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
          } else {
            if (isOutputL[ops==4][chan[c.chan].state.alg][i]) {
              rWrite(baseAddr+ADDR_KSL_TL,(63-(((63-op.tl)*(chan[c.chan].outVol&0x3f))/63))|(op.ksl<<6));
            } else {
              rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
            }
          }

          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
          rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
          rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
          if (oplType>1) {
            rWrite(baseAddr+ADDR_WS,op.ws&((oplType==3)?7:3));
          }
        }

        if (isMuted[c.chan]) {
          rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1));
          if (ops==4) {
            rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1));
          }
        } else {
          rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&3)<<4));
          if (ops==4) {
            rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&3)<<4));
          }
        }
      }
      
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].note=c.value;
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
      /*
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[c.chan].state.alg][i]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
      */
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
    case DIV_CMD_PANNING: {
      switch (c.value) {
        case 0x01:
          chan[c.chan].pan=1;
          break;
        case 0x10:
          chan[c.chan].pan=2;
          break;
        default:
          chan[c.chan].pan=3;
          break;
      }
      //rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      break;
    }
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
      if (!chan[c.chan].portaPause) {
        if (octave(chan[c.chan].baseFreq)!=octave(newFreq)) {
          chan[c.chan].portaPause=true;
          break;
        }
      }
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
    case DIV_CMD_FM_LFO: {
      lfoValue=(c.value&7)|((c.value>>4)<<3);
      rWrite(0x22,lfoValue);
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[c.chan].state.fb=c.value&7;
      //rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      /*
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      */
      break;
    }
    case DIV_CMD_FM_TL: {
      /*
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isMuted[c.chan]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        if (isOutput[chan[c.chan].state.alg][c.value]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
      */
      break;
    }
    case DIV_CMD_FM_AR: {
      /*
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      */
      
      break;
    }
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 63;
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

void DivPlatformOPL::forceIns() {
  /*
  for (int i=0; i<20; i++) {
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
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
    rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  if (dacMode) {
    rWrite(0x2b,0x80);
  }
  immWrite(0x22,lfoValue);
  */
}

void DivPlatformOPL::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void* DivPlatformOPL::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformOPL::getRegisterPool() {
  return regPool;
}

int DivPlatformOPL::getRegisterPoolSize() {
  return 512;
}

void DivPlatformOPL::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,512);
  /*
  if (useYMFM) {
    fm_ymfm->reset();
  }
  */
  OPL3_Reset(&fm,rate);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<20; i++) {
    chan[i]=DivPlatformOPL::Channel();
    chan[i].vol=0x3f;
    chan[i].outVol=0x3f;
  }

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  lfoValue=8;
  properDrums=properDrumsSys;

  if (oplType==3) { // enable OPL3 features
    immWrite(0x105,1);
  }
  
  delay=0;
}

bool DivPlatformOPL::isStereo() {
  return true;
}

bool DivPlatformOPL::keyOffAffectsArp(int ch) {
  return (ch>5);
}

bool DivPlatformOPL::keyOffAffectsPorta(int ch) {
  return (ch>5);
}

void DivPlatformOPL::notifyInsChange(int ins) {
  for (int i=0; i<20; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformOPL::notifyInsDeletion(void* ins) {
}

void DivPlatformOPL::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformOPL::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformOPL::getPortaFloor(int ch) {
  return (ch>5)?12:0;
}

void DivPlatformOPL::setYMFM(bool use) {
  useYMFM=use;
}

void DivPlatformOPL::setOPLType(int type, bool drums) {
  switch (type) {
    case 1: case 2:
      slotsNonDrums=slotsOPL2;
      slotsDrums=slotsOPL2Drums;
      slots=drums?slotsDrums:slotsNonDrums;
      chanMap=chanMapOPL2;
      break;
    case 3:
      slotsNonDrums=slotsOPL3;
      slotsDrums=slotsOPL3Drums;
      slots=drums?slotsDrums:slotsNonDrums;
      chanMap=chanMapOPL3;
      break;
  }
  oplType=type;
  properDrumsSys=drums;
}

void DivPlatformOPL::setFlags(unsigned int flags) {
  /*
  if (flags==3) {
    chipClock=COLOR_NTSC*12.0/7.0;
  } else if (flags==2) {
    chipClock=8000000.0;
  } else if (flags==1) {
    chipClock=COLOR_PAL*12.0/7.0;
  } else {
    chipClock=COLOR_NTSC*15.0/7.0;
  }
  ladder=flags&0x80000000;
  OPN2_SetChipType(ladder?ym3438_mode_ym2612:0);
  if (useYMFM) {
    if (fm_ymfm!=NULL) delete fm_ymfm;
    if (ladder) {
      fm_ymfm=new ymfm::ym2612(iface);
    } else {
      fm_ymfm=new ymfm::ym3438(iface);
    }
    rate=chipClock/144;
  } else {
    rate=chipClock/36;
  }*/

  if (oplType==3) {
    chipClock=COLOR_NTSC*4.0;
    rate=chipClock/288;
  } else {
    chipClock=COLOR_NTSC;
    rate=chipClock/72;
  }
}

int DivPlatformOPL::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<20; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);

  reset();
  return 10;
}

void DivPlatformOPL::quit() {
}

DivPlatformOPL::~DivPlatformOPL() {
}
