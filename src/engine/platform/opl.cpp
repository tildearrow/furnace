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

#define CHIP_FREQBASE chipFreqBase

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

const unsigned short chanMapOPL2Drums[20]={
  0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 7, N, N, N, N, N, N, N, N, N
};

const unsigned char outChanMapOPL2[18]={
  0, 1, 2, 3, 4, 5, 6, 7, 8, N, N, N, N, N, N, N, N, N
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
  {3, 9,  4, 10,  5, 11, 21, 27, 22, 28, 23, 29, 33, 34, 35, 15,  N,  N,  N,  N}, // OP2
  {6, N,  7,  N,  8,  N, 24,  N, 25,  N, 26,  N,  N,  N,  N,  N,  N,  N,  N,  N}, // OP3
  {9, N, 10,  N, 11,  N, 27,  N, 28,  N, 29,  N,  N,  N,  N,  N,  N,  N,  N,  N}  // OP4
};

const unsigned short chanMapOPL3[20]={
  0, 3, 1, 4, 2, 5, 0x100, 0x103, 0x101, 0x104, 0x102, 0x105, 0x106, 0x107, 0x108, 6, 7, 8, N, N
};

const unsigned short chanMapOPL3Drums[20]={
  0, 3, 1, 4, 2, 5, 0x100, 0x103, 0x101, 0x104, 0x102, 0x105, 0x106, 0x107, 0x108, 6, 7, 8, 8, 7
};

const unsigned char outChanMapOPL3[18]={
  0, 3, 1, 4, 2, 5, 9, 12, 10, 13, 11, 14, 15, 16, 17, 6, 7, 8
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
      return "10xx: Set global AM depth (0: 1dB, 1: 4.8dB)";
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
      return "17xx: Set global vibrato depth (0: normal, 1: double)";
      break;
    case 0x18:
      if (properDrumsSys) {
        return "18xx: Toggle drums mode (1: enabled; 0: disabled)";
      }
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

    for (int i=0; i<chans; i++) {
      unsigned char ch=outChanMap[i];
      if (ch==255) continue;
      oscBuf[i]->data[oscBuf[i]->needle]=0;
      if (fm.channel[i].out[0]!=NULL) {
        oscBuf[i]->data[oscBuf[i]->needle]+=*fm.channel[ch].out[0];
      }
      if (fm.channel[i].out[1]!=NULL) {
        oscBuf[i]->data[oscBuf[i]->needle]+=*fm.channel[ch].out[1];
      }
      oscBuf[i]->data[oscBuf[i]->needle]<<=1;
      oscBuf[i]->needle++;
    }
    
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

void DivPlatformOPL::tick(bool sysTick) {
  for (int i=0; i<totalChans; i++) {
    int ops=(slots[3][i]!=255 && chan[i].state.ops==4 && oplType==3)?4:2;
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol*MIN(63,chan[i].std.vol.val))/63;
      for (int j=0; j<ops; j++) {
        unsigned char slot=slots[j][i];
        if (slot==255) continue;
        unsigned short baseAddr=slotMap[slot];
        DivInstrumentFM::Operator& op=chan[i].state.op[(ops==4)?orderedOpsL[j]:j];

        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          if (isOutputL[ops==4][chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-(((63-op.tl)*(chan[i].outVol&0x3f))/63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
          }
        }
      }
    }

    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note+(signed char)chan[i].std.arp.val);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
      }
    }

    if (oplType==3 && chan[i].std.panL.had) {
      chan[i].pan=((chan[i].std.panL.val&1)<<1)|((chan[i].std.panL.val&2)>>1);
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-2048,2048);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].keyOn=true;
      }
    }

    if (chan[i].std.alg.had) {
      chan[i].state.alg=chan[i].std.alg.val;
    }
    if (chan[i].std.fb.had) {
      chan[i].state.fb=chan[i].std.fb.val;
    }

    if (chan[i].std.alg.had || chan[i].std.fb.had || (oplType==3 && chan[i].std.panL.had)) {
      if (isMuted[i]) {
        rWrite(chanMap[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&1)|(chan[i].state.fb<<1));
        if (ops==4) {
          rWrite(chanMap[i+1]+ADDR_LR_FB_ALG,((chan[i].state.alg>>1)&1)|(chan[i].state.fb<<1));
        }
      } else {
        rWrite(chanMap[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&1)|(chan[i].state.fb<<1)|((chan[i].pan&3)<<4));
        if (ops==4) {
          rWrite(chanMap[i+1]+ADDR_LR_FB_ALG,((chan[i].state.alg>>1)&1)|(chan[i].state.fb<<1)|((chan[i].pan&3)<<4));
        }
      }
    }

    for (int j=0; j<ops; j++) {
      unsigned char slot=slots[j][i];
      if (slot==255) continue;
      unsigned short baseAddr=slotMap[slot];
      DivInstrumentFM::Operator& op=chan[i].state.op[(ops==4)?orderedOpsL[j]:j];
      DivMacroInt::IntOp& m=chan[i].std.op[(ops==4)?orderedOpsL[j]:j];
      if (m.am.had) {
        op.am=m.am.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      if (m.vib.had) {
        op.vib=m.vib.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      if (m.sus.had) {
        op.sus=m.sus.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      if (m.ksr.had) {
        op.ksr=m.ksr.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      if (m.mult.had) {
        op.mult=m.mult.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }

      if (m.ar.had) {
        op.ar=m.ar.val;
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
      }
      if (m.dr.had) {
        op.dr=m.dr.val;
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
      }
      if (m.sl.had) {
        op.sl=m.sl.val;
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
      }
      if (m.rr.had) {
        op.rr=m.rr.val;
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
      }

      if (oplType>1) {
        if (m.ws.had) {
          op.ws=m.ws.val;
          rWrite(baseAddr+ADDR_WS,op.ws&((oplType==3)?7:3));
        }
      }

      if (m.tl.had) {
        op.tl=63-m.tl.val;
      }
      if (m.ksl.had) {
        op.ksl=m.ksl.val;
      }
      if (m.tl.had || m.ksl.had) {
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          if (isOutputL[ops==4][chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-(((63-op.tl)*(chan[i].outVol&0x3f))/63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
          }
        }
      }
    }

    if (i<melodicChans) {
      if (chan[i].keyOn || chan[i].keyOff) {
        immWrite(chanMap[i]+ADDR_FREQH,0x00|(chan[i].freqH&31));
        chan[i].keyOff=false;
      }
    }
  }

  if (update4OpMask) {
    update4OpMask=false;
    if (oplType==3) {
      unsigned char opMask=(int)(chan[0].fourOp)|(chan[2].fourOp<<1)|(chan[4].fourOp<<2)|(chan[6].fourOp<<3)|(chan[8].fourOp<<4)|(chan[10].fourOp<<5);
      immWrite(0x104,opMask);
      //printf("updating opMask to %.2x\n",opMask);
    }
  }

  // update drums
  if (properDrums) {
    bool updateDrums=false;
    for (int i=melodicChans; i<totalChans; i++) {
      if (chan[i].keyOn || chan[i].keyOff) {
        drumState&=~(1<<(totalChans-i-1));
        updateDrums=true;
        chan[i].keyOff=false;
      }
    }

    if (updateDrums) {
      immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  bool updateDrums=false;
  for (int i=0; i<totalChans; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,octave(chan[i].baseFreq),chan[i].pitch2);
      if (chan[i].freq>131071) chan[i].freq=131071;
      int freqt=toFreq(chan[i].freq)+chan[i].pitch2;
      chan[i].freqH=freqt>>8;
      chan[i].freqL=freqt&0xff;
      immWrite(chanMap[i]+ADDR_FREQ,chan[i].freqL);
    }
    if (i<melodicChans) {
      if (chan[i].keyOn) {
        immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(0x20));
        chan[i].keyOn=false;
      } else if (chan[i].freqChanged) {
        immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(chan[i].active<<5));
      }
    } else {
      if (chan[i].keyOn) {
        immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH);
        drumState|=(1<<(totalChans-i-1));
        updateDrums=true;
        chan[i].keyOn=false;
      } else if (chan[i].freqChanged) {
        immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH);
      }
    }
    chan[i].freqChanged=false;
  }

  if (updateDrums) {
    immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
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
  int ops=(slots[3][ch]!=255 && chan[ch].state.ops==4 && oplType==3)?4:2;
  chan[ch].fourOp=(ops==4);
  update4OpMask=true;
  for (int i=0; i<ops; i++) {
    unsigned char slot=slots[i][ch];
    if (slot==255) continue;
    unsigned short baseAddr=slotMap[slot];
    DivInstrumentFM::Operator& op=chan[ch].state.op[(ops==4)?orderedOpsL[i]:i];

    if (isMuted[ch]) {
      rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
    } else {
      if (isOutputL[ops==4][chan[ch].state.alg][i]) {
        rWrite(baseAddr+ADDR_KSL_TL,(63-(((63-op.tl)*(chan[ch].outVol&0x3f))/63))|(op.ksl<<6));
      } else {
        rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
      }
    }
  }

  if (isMuted[ch]) {
    rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1));
    if (ops==4) {
      rWrite(chanMap[ch+1]+ADDR_LR_FB_ALG,((chan[ch].state.alg>>1)&1)|(chan[ch].state.fb<<1));
    }
  } else {
    rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&3)<<4));
    if (ops==4) {
      rWrite(chanMap[ch+1]+ADDR_LR_FB_ALG,((chan[ch].state.alg>>1)&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&3)<<4));
    }
  }
}

int DivPlatformOPL::dispatch(DivCommand c) {
  if (c.chan>=totalChans) return 0;
  // ineffective in 4-op mode
  if (oplType==3 && c.chan<14 && (c.chan&1) && c.cmd!=DIV_CMD_GET_VOLMAX && c.cmd!=DIV_ALWAYS_SET_VOLUME) {
    if (chan[c.chan-1].fourOp) return 0;
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_OPL);

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (chan[c.chan].insChanged) {
        int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
        chan[c.chan].fourOp=(ops==4);
        if (chan[c.chan].fourOp) {
          chan[c.chan+1].macroInit(NULL);
        }
        update4OpMask=true;
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
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
          oldWrites[chanMap[c.chan]+ADDR_LR_FB_ALG]=-1;
          rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1));
          if (ops==4) {
            oldWrites[chanMap[c.chan+1]+ADDR_LR_FB_ALG]=-1;
            rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1));
          }
        } else {
          oldWrites[chanMap[c.chan]+ADDR_LR_FB_ALG]=-1;
          rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&3)<<4));
          if (ops==4) {
            oldWrites[chanMap[c.chan+1]+ADDR_LR_FB_ALG]=-1;
            rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&3)<<4));
          }
        }
      }
      
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        if (c.chan>=melodicChans && chan[c.chan].state.opllPreset==16 && chan[c.chan].state.fixedDrums) { // drums
          if (c.chan==melodicChans) {
            chan[c.chan].baseFreq=(chan[c.chan].state.kickFreq&1023)<<(chan[c.chan].state.kickFreq>>10);
          } else if (c.chan==melodicChans+1 || c.chan==melodicChans+4) {
            chan[c.chan].baseFreq=(chan[c.chan].state.snareHatFreq&1023)<<(chan[c.chan].state.snareHatFreq>>10);
          } else if (c.chan==melodicChans+2 || c.chan==melodicChans+3) {
            chan[c.chan].baseFreq=(chan[c.chan].state.tomTopFreq&1023)<<(chan[c.chan].state.tomTopFreq>>10);
          } else {
            chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
          }
        } else {
          chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        }
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
      if (pretendYMU) {
        c.value=pow(((double)c.value/127.0),0.5)*63.0;
        if (c.value<0) c.value=0;
        if (c.value>63) c.value=63;
      }
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      for (int i=0; i<ops; i++) {
        unsigned char slot=slots[i][c.chan];
        if (slot==255) continue;
        unsigned short baseAddr=slotMap[slot];
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
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      if (pretendYMU) {
        return pow(((double)chan[c.chan].vol/63.0),2.0)*127.0;
      }
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
      if (oplType!=3) break;
      if (c.value==0 && c.value2==0) {
        chan[c.chan].pan=3;
      } else {
        chan[c.chan].pan=(c.value2>0)|((c.value>0)<<1);
      }
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
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
      if (c.value&2) {
        dvb=c.value&1;
      } else {
        dam=c.value&1;
      }
      immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[c.chan].state.fb=c.value&7;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
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
      break;
    }
    case DIV_CMD_FM_MULT: {
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value>=ops) break;
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
      op.mult=c.value2&15;
      unsigned char slot=slots[c.value][c.chan];
      if (slot==255) break;
      unsigned short baseAddr=slotMap[slot];
      rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      break;
    }
    case DIV_CMD_FM_TL: {
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value>=ops) break;
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
      op.tl=c.value2&63;
      unsigned char slot=slots[c.value][c.chan];
      if (slot==255) break;
      unsigned short baseAddr=slotMap[slot];
      if (isMuted[c.chan]) {
        rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
      } else {
        if (isOutputL[ops==4][chan[c.chan].state.alg][c.value]) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-(((63-op.tl)*(chan[c.chan].outVol&0x3f))/63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
        }
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.ar=c.value2&15;
          rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.ar=c.value2&15;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
      }      
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      if (!properDrumsSys) break;
      properDrums=c.value;
      immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
      slots=properDrums?slotsDrums:slotsNonDrums;
      if (oplType==3) {
        chanMap=properDrums?chanMapOPL3Drums:chanMapOPL3;
        melodicChans=properDrums?15:18;
        totalChans=properDrums?20:18;
      } else {
        chanMap=properDrums?chanMapOPL2Drums:chanMapOPL2;
        melodicChans=properDrums?6:9;
        totalChans=properDrums?11:9;
      }
      break;
    }
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      if (pretendYMU) return 127;
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
  if (oplType==3) {
    chanMap=properDrums?chanMapOPL3Drums:chanMapOPL3;
    melodicChans=properDrums?15:18;
    totalChans=properDrums?20:18;
  } else {
    chanMap=properDrums?chanMapOPL2Drums:chanMapOPL2;
    melodicChans=properDrums?6:9;
    totalChans=properDrums?11:9;
  }
  for (int i=0; i<totalChans; i++) {
    int ops=(slots[3][i]!=255 && chan[i].state.ops==4 && oplType==3)?4:2;
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].fourOp=(ops==4);
    for (int j=0; j<ops; j++) {
      unsigned char slot=slots[j][i];
      if (slot==255) continue;
      unsigned short baseAddr=slotMap[slot];
      DivInstrumentFM::Operator& op=chan[i].state.op[(ops==4)?orderedOpsL[j]:j];

      if (isMuted[i]) {
        rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
      } else {
        if (isOutputL[ops==4][chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-(((63-op.tl)*(chan[i].outVol&0x3f))/63))|(op.ksl<<6));
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

    if (isMuted[i]) {
      rWrite(chanMap[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&1)|(chan[i].state.fb<<1));
      if (ops==4) {
        rWrite(chanMap[i+1]+ADDR_LR_FB_ALG,((chan[i].state.alg>>1)&1)|(chan[i].state.fb<<1));
      }
    } else {
      rWrite(chanMap[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&1)|(chan[i].state.fb<<1)|((chan[i].pan&3)<<4));
      if (ops==4) {
        rWrite(chanMap[i+1]+ADDR_LR_FB_ALG,((chan[i].state.alg>>1)&1)|(chan[i].state.fb<<1)|((chan[i].pan&3)<<4));
      }
    }
  }
  immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
  update4OpMask=true;
}

void DivPlatformOPL::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void* DivPlatformOPL::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformOPL::getOscBuffer(int ch) {
  return oscBuf[ch];
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

  properDrums=properDrumsSys;
  if (oplType==3) {
    chanMap=properDrums?chanMapOPL3Drums:chanMapOPL3;
    melodicChans=properDrums?15:18;
    totalChans=properDrums?20:18;
  } else {
    chanMap=properDrums?chanMapOPL2Drums:chanMapOPL2;
    melodicChans=properDrums?6:9;
    totalChans=properDrums?11:9;
  }

  for (int i=0; i<totalChans; i++) {
    chan[i]=DivPlatformOPL::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x3f;
    chan[i].outVol=0x3f;
  }

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  lfoValue=8;
  drumState=0;

  drumVol[0]=0;
  drumVol[1]=0;
  drumVol[2]=0;
  drumVol[3]=0;
  drumVol[4]=0;

  if (oplType==2) { // enable OPL2 waveforms
    immWrite(0x01,0x20);
  }

  if (oplType==3) { // enable OPL3 features
    immWrite(0x105,1);
  }

  update4OpMask=true;
  dam=false;
  dvb=false;
  delay=0;

  immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
}

bool DivPlatformOPL::isStereo() {
  return true;
}

bool DivPlatformOPL::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformOPL::keyOffAffectsPorta(int ch) {
  return false;
}

void DivPlatformOPL::notifyInsChange(int ins) {
  for (int i=0; i<totalChans; i++) {
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
  pretendYMU=false;
  switch (type) {
    case 1: case 2:
      slotsNonDrums=slotsOPL2;
      slotsDrums=slotsOPL2Drums;
      slots=drums?slotsDrums:slotsNonDrums;
      chanMap=drums?chanMapOPL2Drums:chanMapOPL2;
      outChanMap=outChanMapOPL2;
      chipFreqBase=9440540*0.25;
      chans=9;
      melodicChans=drums?6:9;
      totalChans=drums?11:9;
      break;
    case 3: case 759:
      slotsNonDrums=slotsOPL3;
      slotsDrums=slotsOPL3Drums;
      slots=drums?slotsDrums:slotsNonDrums;
      chanMap=drums?chanMapOPL3Drums:chanMapOPL3;
      outChanMap=outChanMapOPL3;
      chipFreqBase=9440540;
      chans=18;
      melodicChans=drums?15:18;
      totalChans=drums?20:18;
      if (type==759) pretendYMU=true;
      break;
  }
  if (type==759) {
    oplType=3;
  } else {
    oplType=type;
  }
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

  if (pretendYMU) {
    rate=48000;
    chipClock=rate*288;
  }

  for (int i=0; i<18; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformOPL::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<20; i++) {
    isMuted[i]=false;
  }
  for (int i=0; i<18; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  reset();
  return totalChans;
}

void DivPlatformOPL::quit() {
  for (int i=0; i<18; i++) {
    delete oscBuf[i];
  }
}

DivPlatformOPL::~DivPlatformOPL() {
}
