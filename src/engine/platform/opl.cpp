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

#include "opl.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define KVSL(x,y) ((chan[x].state.op[orderedOpsL1[ops==4][y]].kvs==2 && isOutputL[ops==4][chan[x].state.alg][y]) || chan[x].state.op[orderedOpsL1[ops==4][y]].kvs==1)

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

const int orderedOpsL1[2][4]={
  {0, 1, 0, 1}, // 2-op
  {0, 2, 1, 3}  // 4-op
};

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

void DivPlatformOPL::acquire_nuked(short** buf, size_t len) {
  thread_local short o[4];
  thread_local int os[4];
  thread_local ymfm::ymfm_output<2> aOut;

  for (size_t h=0; h<len; h++) {
    os[0]=0; os[1]=0; os[2]=0; os[3]=0;
    if (!writes.empty() && --delay<0) {
      delay=1;
      QueuedWrite& w=writes.front();
      switch (w.addr) {
        case 8:
          if (adpcmChan>=0) {
            adpcmB->write(w.addr-7,(w.val&15)|0x80);
            OPL3_WriteReg(&fm,w.addr,w.val&0xc0);
          } else {
            OPL3_WriteReg(&fm,w.addr,w.val);
          }
          break;
        case 7: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 21: case 22: case 23:
          if (adpcmChan>=0) {
            adpcmB->write(w.addr-7,w.val);
          } else {
            OPL3_WriteReg(&fm,w.addr,w.val);
          }
          break;
        default:
          OPL3_WriteReg(&fm,w.addr,w.val);
          break;
      }
      regPool[w.addr&511]=w.val;
      writes.pop();
    }

    if (downsample) {
      OPL3_Generate4ChResampled(&fm,o);
    } else {
      OPL3_Generate4Ch(&fm,o);
    }
    os[0]+=o[0];
    os[1]+=o[1];
    os[2]+=o[2];
    os[3]+=o[3];

    if (adpcmChan>=0) {
      adpcmB->clock();
      aOut.clear();
      adpcmB->output<2>(aOut,0);

      if (!isMuted[adpcmChan]) {
        os[0]-=aOut.data[0]>>3;
        os[1]-=aOut.data[0]>>3;
        oscBuf[adpcmChan]->data[oscBuf[adpcmChan]->needle++]=aOut.data[0]>>1;
      } else {
        oscBuf[adpcmChan]->data[oscBuf[adpcmChan]->needle++]=0;
      }
    }

    if (fm.rhy&0x20) {
      for (int i=0; i<melodicChans+1; i++) {
        unsigned char ch=outChanMap[i];
        int chOut=0;
        if (ch==255) continue;
        if (fm.channel[i].out[0]!=NULL) {
          chOut+=*fm.channel[ch].out[0];
        }
        if (fm.channel[i].out[1]!=NULL) {
          chOut+=*fm.channel[ch].out[1];
        }
        if (fm.channel[i].out[2]!=NULL) {
          chOut+=*fm.channel[ch].out[2];
        }
        if (fm.channel[i].out[3]!=NULL) {
          chOut+=*fm.channel[ch].out[3];
        }
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(chOut<<(i==melodicChans?1:2),-32768,32767);
      }
      // special
      oscBuf[melodicChans+1]->data[oscBuf[melodicChans+1]->needle++]=fm.slot[16].out*4;
      oscBuf[melodicChans+2]->data[oscBuf[melodicChans+2]->needle++]=fm.slot[14].out*4;
      oscBuf[melodicChans+3]->data[oscBuf[melodicChans+3]->needle++]=fm.slot[17].out*4;
      oscBuf[melodicChans+4]->data[oscBuf[melodicChans+4]->needle++]=fm.slot[13].out*4;
    } else {
      for (int i=0; i<chans; i++) {
        unsigned char ch=outChanMap[i];
        int chOut=0;
        if (ch==255) continue;
        if (fm.channel[i].out[0]!=NULL) {
          chOut+=*fm.channel[ch].out[0];
        }
        if (fm.channel[i].out[1]!=NULL) {
          chOut+=*fm.channel[ch].out[1];
        }
        if (fm.channel[i].out[2]!=NULL) {
          chOut+=*fm.channel[ch].out[2];
        }
        if (fm.channel[i].out[3]!=NULL) {
          chOut+=*fm.channel[ch].out[3];
        }
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(chOut<<2,-32768,32767);
      }
    }
    
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;

    if (os[2]<-32768) os[2]=-32768;
    if (os[2]>32767) os[2]=32767;

    if (os[3]<-32768) os[3]=-32768;
    if (os[3]>32767) os[3]=32767;
  
    buf[0][h]=os[0];
    if (totalOutputs>1) {
      buf[1][h]=os[1];
    }
    if (totalOutputs>2) {
      buf[2][h]=os[2];
    }
    if (totalOutputs>3) {
      buf[3][h]=os[3];
    }
    if (totalOutputs==6) {
      // placeholder for OPL4
      buf[4][h]=0;
      buf[5][h]=0;
    }
  }
}

void DivPlatformOPL::acquire_ymfm1(short** buf, size_t len) {
  ymfm::ymfm_output<1> out;

  ymfm::ym3526::fm_engine* fme=fm_ymfm1->debug_fm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<1>>* fmChan[9];

  for (int i=0; i<9; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      delay=1;
      QueuedWrite& w=writes.front();

      fm_ymfm1->write(0,w.addr);
      fm_ymfm1->write(1,w.val);

      regPool[w.addr&511]=w.val;
      writes.pop();
    }

    fm_ymfm1->generate(&out,1);

    buf[0][h]=out.data[0];

    if (properDrums) {
      for (int i=0; i<7; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767);
      }
      oscBuf[7]->data[oscBuf[7]->needle++]=CLAMP(fmChan[7]->debug_special1()<<2,-32768,32767);
      oscBuf[8]->data[oscBuf[8]->needle++]=CLAMP(fmChan[8]->debug_special1()<<2,-32768,32767);
      oscBuf[9]->data[oscBuf[9]->needle++]=CLAMP(fmChan[8]->debug_special2()<<2,-32768,32767);
      oscBuf[10]->data[oscBuf[10]->needle++]=CLAMP(fmChan[7]->debug_special2()<<2,-32768,32767);
    } else {
      for (int i=0; i<9; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767);
      }
    }
  }
}

void DivPlatformOPL::acquire_ymfm2(short** buf, size_t len) {
  ymfm::ymfm_output<1> out;

  ymfm::ym3812::fm_engine* fme=fm_ymfm2->debug_fm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<2>>* fmChan[9];

  for (int i=0; i<9; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      delay=1;
      QueuedWrite& w=writes.front();

      fm_ymfm2->write(0,w.addr);
      fm_ymfm2->write(1,w.val);

      regPool[w.addr&511]=w.val;
      writes.pop();
    }

    fm_ymfm2->generate(&out,1);

    buf[0][h]=out.data[0];

    if (properDrums) {
      for (int i=0; i<7; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767);
      }
      oscBuf[7]->data[oscBuf[7]->needle++]=CLAMP(fmChan[7]->debug_special1()<<2,-32768,32767);
      oscBuf[8]->data[oscBuf[8]->needle++]=CLAMP(fmChan[8]->debug_special1()<<2,-32768,32767);
      oscBuf[9]->data[oscBuf[9]->needle++]=CLAMP(fmChan[8]->debug_special2()<<2,-32768,32767);
      oscBuf[10]->data[oscBuf[10]->needle++]=CLAMP(fmChan[7]->debug_special2()<<2,-32768,32767);
    } else {
      for (int i=0; i<9; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767);
      }
    }
  }
}

void DivPlatformOPL::acquire_ymfm8950(short** buf, size_t len) {
  ymfm::ymfm_output<1> out;

  ymfm::y8950::fm_engine* fme=fm_ymfm8950->debug_fm_engine();
  ymfm::adpcm_b_engine* abe=fm_ymfm8950->debug_adpcm_b_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<1>>* fmChan[9];

  for (int i=0; i<9; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      delay=1;
      QueuedWrite& w=writes.front();

      fm_ymfm8950->write(0,w.addr);
      fm_ymfm8950->write(1,w.val);

      regPool[w.addr&511]=w.val;
      writes.pop();
    }

    fm_ymfm8950->generate(&out,1);

    buf[0][h]=out.data[0];

    if (properDrums) {
      for (int i=0; i<7; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767);
      }
      oscBuf[7]->data[oscBuf[7]->needle++]=CLAMP(fmChan[7]->debug_special1()<<2,-32768,32767);
      oscBuf[8]->data[oscBuf[8]->needle++]=CLAMP(fmChan[8]->debug_special1()<<2,-32768,32767);
      oscBuf[9]->data[oscBuf[9]->needle++]=CLAMP(fmChan[8]->debug_special2()<<2,-32768,32767);
      oscBuf[10]->data[oscBuf[10]->needle++]=CLAMP(fmChan[7]->debug_special2()<<2,-32768,32767);
      oscBuf[11]->data[oscBuf[11]->needle++]=CLAMP(abe->get_last_out(0),-32768,32767);
    } else {
      for (int i=0; i<9; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767);
      }
      oscBuf[9]->data[oscBuf[9]->needle++]=CLAMP(abe->get_last_out(0),-32768,32767);
    }
  }
}

void DivPlatformOPL::acquire_ymfm3(short** buf, size_t len) {
  ymfm::ymfm_output<4> out;

  ymfm::ymf262::fm_engine* fme=fm_ymfm3->debug_fm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<3>>* fmChan[18];

  for (int i=0; i<18; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      delay=1;
      QueuedWrite& w=writes.front();

      fm_ymfm3->write((w.addr&0x100)?2:0,w.addr);
      fm_ymfm3->write(1,w.val);

      regPool[w.addr&511]=w.val;
      writes.pop();
    }

    fm_ymfm3->generate(&out,1);

    buf[0][h]=out.data[0]>>1;
    if (totalOutputs>1) {
      buf[1][h]=out.data[1]>>1;
    }
    if (totalOutputs>2) {
      buf[2][h]=out.data[2]>>1;
    }
    if (totalOutputs>3) {
      buf[3][h]=out.data[3]>>1;
    }
    if (totalOutputs==6) {
      // placeholder for OPL4
      buf[4][h]=0;
      buf[5][h]=0;
    }

    if (properDrums) {
      for (int i=0; i<16; i++) {
        unsigned char ch=(i<12 && chan[i&(~1)].fourOp)?outChanMap[i^1]:outChanMap[i];
        if (ch==255) continue;
        int chOut=fmChan[ch]->debug_output(0);
        if (chOut==0) {
          chOut=fmChan[ch]->debug_output(1);
        }
        if (chOut==0) {
          chOut=fmChan[ch]->debug_output(2);
        }
        if (chOut==0) {
          chOut=fmChan[ch]->debug_output(3);
        }
        if (i==15) {
          oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(chOut,-32768,32767);
        } else {
          oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(chOut<<1,-32768,32767);
        }
      }
      oscBuf[16]->data[oscBuf[16]->needle++]=CLAMP(fmChan[7]->debug_special2()<<1,-32768,32767);
      oscBuf[17]->data[oscBuf[17]->needle++]=CLAMP(fmChan[8]->debug_special1()<<1,-32768,32767);
      oscBuf[18]->data[oscBuf[18]->needle++]=CLAMP(fmChan[8]->debug_special2()<<1,-32768,32767);
      oscBuf[19]->data[oscBuf[19]->needle++]=CLAMP(fmChan[7]->debug_special1()<<1,-32768,32767);
    } else {
      for (int i=0; i<18; i++) {
        unsigned char ch=outChanMap[i];
        if (ch==255) continue;
        int chOut=fmChan[ch]->debug_output(0);
        if (chOut==0) {
          chOut=fmChan[ch]->debug_output(1);
        }
        if (chOut==0) {
          chOut=fmChan[ch]->debug_output(2);
        }
        if (chOut==0) {
          chOut=fmChan[ch]->debug_output(3);
        }
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(chOut<<1,-32768,32767);
      }
    }
  }
}

static const int cycleMap[18]={
  6, 7, 8, 6, 7, 8, 0, 1, 2,
  0, 1, 2, 3, 4, 5, 3, 4, 5,
};

static const int cycleMapDrums[18]={
  6, 10, 8, 6, 7, 9, 0, 1, 2,
  0, 1, 2, 3, 4, 5, 3, 4, 5,
};

void DivPlatformOPL::acquire_nukedLLE2(short** buf, size_t len) {
  int chOut[11];
  thread_local ymfm::ymfm_output<2> aOut;

  for (size_t h=0; h<len; h++) {
    int curCycle=0;
    unsigned char subCycle=0;

    for (int i=0; i<11; i++) {
      chOut[i]=0;
    }
    
    while (true) {
      lastSH=fm_lle2.o_sh;
      lastSY=fm_lle2.o_sy;

      // register control
      if (waitingBusy) {
        fm_lle2.input.cs=0;
        fm_lle2.input.rd=0;
        fm_lle2.input.wr=1;
        fm_lle2.input.address=0;
      } else {
        if (!writes.empty()) {
          QueuedWrite& w=writes.front();

          if (w.addrOrVal) {
            regPool[w.addr&511]=w.val;
            fm_lle2.input.cs=0;
            fm_lle2.input.rd=1;
            fm_lle2.input.wr=0;
            fm_lle2.input.address=1;
            fm_lle2.input.data_i=w.val;
            writes.pop();
            delay=84;
          } else {
            if (chipType==8950) {
              switch (w.addr) {
                case 8:
                  adpcmB->write(w.addr-7,(w.val&15)|0x80);
                  fm_lle2.input.cs=0;
                  fm_lle2.input.rd=1;
                  fm_lle2.input.wr=0;
                  fm_lle2.input.address=0;
                  fm_lle2.input.data_i=w.addr;
                  w.addrOrVal=true;
                  // weird. wasn't it 12?
                  delay=24;
                  break;
                case 7: case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 21: case 22: case 23:
                  adpcmB->write(w.addr-7,w.val);
                  regPool[w.addr&511]=w.val;
                  writes.pop();
                  delay=108;
                  break;
                default:
                  fm_lle2.input.cs=0;
                  fm_lle2.input.rd=1;
                  fm_lle2.input.wr=0;
                  fm_lle2.input.address=0;
                  fm_lle2.input.data_i=w.addr;
                  w.addrOrVal=true;
                  // weird. wasn't it 12?
                  delay=24;
                  break;
              }
            } else {
              fm_lle2.input.cs=0;
              fm_lle2.input.rd=1;
              fm_lle2.input.wr=0;
              fm_lle2.input.address=0;
              fm_lle2.input.data_i=w.addr;
              w.addrOrVal=true;
              // weird. wasn't it 12?
              delay=24;
            }
          }

          waitingBusy=true;
        }
      }

      fm_lle2.input.mclk=1;
      FMOPL2_Clock(&fm_lle2);
      fm_lle2.input.mclk=0;
      FMOPL2_Clock(&fm_lle2);

      if (waitingBusy) {
        if (--delay<0) waitingBusy=false;
      }

      if (!(++subCycle&3)) {
        if (properDrums) {
          chOut[cycleMapDrums[curCycle]]+=fm_lle2.op_value_debug;
        } else {
          chOut[cycleMap[curCycle]]+=fm_lle2.op_value_debug;
        }
        curCycle++;
      }

      if (fm_lle2.o_sy && !lastSY) {
        dacVal>>=1;
        dacVal|=(fm_lle2.o_mo&1)<<17;
      }

      if (!fm_lle2.o_sh && lastSH) {
        int e=(dacVal>>15)&7;
        int m=(dacVal>>5)&1023;
        m-=512;
        dacOut=(m<<e)>>1;
        break;
      }
    }

    for (int i=0; i<11; i++) {
      if (i>=6 && properDrums) {
        chOut[i]<<=1;
      } else {
        chOut[i]<<=2;
      }
      if (chOut[i]<-32768) chOut[i]=-32768;
      if (chOut[i]>32767) chOut[i]=32767;
      oscBuf[i]->data[oscBuf[i]->needle++]=chOut[i];
    }

    if (chipType==8950) {
      adpcmB->clock();
      aOut.clear();
      adpcmB->output<2>(aOut,0);

      if (!isMuted[adpcmChan]) {
        dacOut-=aOut.data[0]>>3;
        oscBuf[adpcmChan]->data[oscBuf[adpcmChan]->needle++]=aOut.data[0]>>1;
      } else {
        oscBuf[adpcmChan]->data[oscBuf[adpcmChan]->needle++]=0;
      }
    }

    if (dacOut<-32768) dacOut=-32768;
    if (dacOut>32767) dacOut=32767;

    buf[0][h]=dacOut;
  }
}

void DivPlatformOPL::acquire_nukedLLE3(short** buf, size_t len) {
  int chOut[20];

  for (size_t h=0; h<len; h++) {
    //int curCycle=0;
    //unsigned char subCycle=0;

    for (int i=0; i<20; i++) {
      chOut[i]=0;
    }
    
    while (true) {
      lastSH=fm_lle3.o_smpac;
      lastSH2=fm_lle3.o_smpbd;
      lastSY=fm_lle3.o_sy;

      // register control
      if (waitingBusy) {
        if (delay<15) {
          fm_lle3.input.cs=0;
          fm_lle3.input.rd=0;
          fm_lle3.input.wr=1;
          fm_lle3.input.address=0;
        }
      } else {
        if (!writes.empty()) {
          QueuedWrite& w=writes.front();

          if (w.addrOrVal) {
            regPool[w.addr&511]=w.val;
            fm_lle3.input.cs=0;
            fm_lle3.input.rd=1;
            fm_lle3.input.wr=0;
            fm_lle3.input.address=(w.addr&0x100)?3:1;
            fm_lle3.input.data_i=w.val;
            writes.pop();
            delay=16;
          } else {
            fm_lle3.input.cs=0;
            fm_lle3.input.rd=1;
            fm_lle3.input.wr=0;
            fm_lle3.input.address=(w.addr&0x100)?2:0;
            fm_lle3.input.data_i=w.addr&0xff;
            w.addrOrVal=true;
            // weird. wasn't it 12?
            delay=16;
          }

          waitingBusy=true;
        }
      }

      fm_lle3.input.mclk=1;
      FMOPL3_Clock(&fm_lle3);
      fm_lle3.input.mclk=0;
      FMOPL3_Clock(&fm_lle3);

      if (waitingBusy) {
        if (--delay<0) waitingBusy=false;
      }

      /*if (!(++subCycle&3)) {
        // TODO: chan osc
        curCycle++;
      }*/

      if (fm_lle3.o_sy && !lastSY) {
        dacVal>>=1;
        dacVal|=(fm_lle3.o_doab&1)<<17;
        dacVal2>>=1;
        dacVal2|=(fm_lle3.o_docd&1)<<17;
      }

      if (!fm_lle3.o_smpbd && lastSH2) {
        dacOut3[0]=((dacVal>>1)&0xffff)-0x8000;
        dacOut3[2]=((dacVal2>>1)&0xffff)-0x8000;
      }

      if (!fm_lle3.o_smpac && lastSH) {
        dacOut3[1]=((dacVal>>1)&0xffff)-0x8000;
        dacOut3[3]=((dacVal2>>1)&0xffff)-0x8000;
        break;
      }
    }

    for (int i=0; i<20; i++) {
      if (i>=15 && properDrums) {
        chOut[i]<<=1;
      } else {
        chOut[i]<<=2;
      }
      if (chOut[i]<-32768) chOut[i]=-32768;
      if (chOut[i]>32767) chOut[i]=32767;
      oscBuf[i]->data[oscBuf[i]->needle++]=chOut[i];
    }

    for (int i=0; i<MIN(4,totalOutputs); i++) {
      if (dacOut3[i]<-32768) dacOut3[i]=-32768;
      if (dacOut3[i]>32767) dacOut3[i]=32767;

      buf[i][h]=dacOut3[i];
    }
  }
}

void DivPlatformOPL::acquire(short** buf, size_t len) {
  if (emuCore==2) { // LLE
    switch (chipType) {
      case 1: case 2: case 8950:
        acquire_nukedLLE2(buf,len);
        break;
      case 3: case 759:
        acquire_nukedLLE3(buf,len);
        break;
    }
  } else if (emuCore==1) { // ymfm
    switch (chipType) {
      case 1:
        acquire_ymfm1(buf,len);
        break;
      case 2:
        acquire_ymfm2(buf,len);
        break;
      case 8950:
        acquire_ymfm8950(buf,len);
        break;
      case 3: case 759:
        acquire_ymfm3(buf,len);
        break;
    }
  } else { // OPL3
    acquire_nuked(buf,len);
  }
}

double DivPlatformOPL::NOTE_ADPCMB(int note) {
  if (adpcmChan<0) return 0;
  if (chan[adpcmChan].sample>=0 && chan[adpcmChan].sample<parent->song.sampleLen) {
    double off=65535.0*(double)(parent->getSample(chan[adpcmChan].sample)->centerRate)/8363.0;
    return parent->calcBaseFreq((double)chipClock/144,off,note,false);
  }
  return 0;
}

void DivPlatformOPL::tick(bool sysTick) {
  for (int i=0; i<totalChans; i++) {
    int ops=(slots[3][i]!=255 && chan[i].state.ops==4 && oplType==3)?4:2;
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(63,chan[i].std.vol.val),63);
      for (int j=0; j<ops; j++) {
        unsigned char slot=slots[j][i];
        if (slot==255) continue;
        unsigned short baseAddr=slotMap[slot];
        DivInstrumentFM::Operator& op=chan[i].state.op[(ops==4)?orderedOpsL[j]:j];

        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          if (KVSL(i,j) || i>melodicChans) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
          }
        }
      }
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }

    if (oplType==3 && chan[i].std.panL.had) {
      chan[i].pan=((chan[i].std.panL.val&1)<<1)|((chan[i].std.panL.val&2)>>1)|((chan[i].std.panL.val&4)<<1)|((chan[i].std.panL.val&8)>>1);
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-131071,131071);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
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
        rWrite(chanMap[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&1)|(chan[i].state.fb<<1)|((chan[i].pan&15)<<4));
        if (ops==4) {
          rWrite(chanMap[i+1]+ADDR_LR_FB_ALG,((chan[i].state.alg>>1)&1)|(chan[i].state.fb<<1)|((chan[i].pan&15)<<4));
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
        op.tl=m.tl.val&63;
      }
      if (m.ksl.had) {
        op.ksl=m.ksl.val;
      }
      if (m.tl.had || m.ksl.had) {
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          if (KVSL(i,j) || i>melodicChans) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
          }
        }
      }
    }
  }

  int hardResetElapsed=0;
  bool mustHardReset=false;
  bool weWillWriteRRLater[64];

  memset(weWillWriteRRLater,0,64*sizeof(bool));

  for (int i=0; i<melodicChans; i++) {
    int ops=(slots[3][i]!=255 && chan[i].state.ops==4 && oplType==3)?4:2;

    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(chanMap[i]+ADDR_FREQH,0x00|(chan[i].freqH&31));
      chan[i].keyOff=false;
    }
    if (chan[i].hardReset && chan[i].keyOn) {
      mustHardReset=true;
      for (int j=0; j<ops; j++) {
        unsigned char slot=slots[j][i];
        if (slot==255) continue;
        unsigned short baseAddr=slotMap[slot];
        if (baseAddr>0x100) {
          weWillWriteRRLater[(baseAddr&0xff)|32]=true;
        } else {
          weWillWriteRRLater[(baseAddr&0xff)]=true;
        }
        immWrite(baseAddr+ADDR_SL_RR,0x0f);
        hardResetElapsed++;
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

  // ADPCM
  if (adpcmChan>=0) {
    if (chan[adpcmChan].furnacePCM) {
      chan[adpcmChan].std.next();

      if (chan[adpcmChan].std.vol.had) {
        chan[adpcmChan].outVol=(chan[adpcmChan].vol*MIN(chan[adpcmChan].macroVolMul,chan[adpcmChan].std.vol.val))/chan[adpcmChan].macroVolMul;
        immWrite(18,chan[adpcmChan].outVol);
      }

      if (NEW_ARP_STRAT) {
        chan[adpcmChan].handleArp();
      } else if (chan[adpcmChan].std.arp.had) {
        if (!chan[adpcmChan].inPorta) {
          chan[adpcmChan].baseFreq=NOTE_ADPCMB(parent->calcArp(chan[adpcmChan].note,chan[adpcmChan].std.arp.val));
        }
        chan[adpcmChan].freqChanged=true;
      }
      if (chan[adpcmChan].std.phaseReset.had) {
        if ((chan[adpcmChan].std.phaseReset.val==1) && chan[adpcmChan].active) {
          chan[adpcmChan].keyOn=true;
        }
      }
    }
    if (chan[adpcmChan].freqChanged || chan[adpcmChan].keyOn || chan[adpcmChan].keyOff) {
      if (chan[adpcmChan].sample>=0 && chan[adpcmChan].sample<parent->song.sampleLen) {
        double off=65535.0*(double)(parent->getSample(chan[adpcmChan].sample)->centerRate)/8363.0;
        chan[adpcmChan].freq=parent->calcFreq(chan[adpcmChan].baseFreq,chan[adpcmChan].pitch,chan[adpcmChan].fixedArp?chan[adpcmChan].baseNoteOverride:chan[adpcmChan].arpOff,chan[adpcmChan].fixedArp,false,4,chan[adpcmChan].pitch2,(double)chipClock/144,off);
      } else {
        chan[adpcmChan].freq=0;
      }
      if (chan[adpcmChan].fixedFreq>0) chan[adpcmChan].freq=chan[adpcmChan].fixedFreq;
      if (pretendYMU) { // YMU759 only does 4KHz or 8KHz
        if (chan[adpcmChan].freq>7500) {
          chan[adpcmChan].freq=10922; // 8KHz
        } else {
          chan[adpcmChan].freq=5461; // 4KHz
        }
      }
      if (chan[adpcmChan].freq<0) chan[adpcmChan].freq=0;
      if (chan[adpcmChan].freq>65535) chan[adpcmChan].freq=65535;
      immWrite(16,chan[adpcmChan].freq&0xff);
      immWrite(17,(chan[adpcmChan].freq>>8)&0xff);
      if (chan[adpcmChan].keyOn || chan[adpcmChan].keyOff) {
        immWrite(7,0x01); // reset
        if (chan[adpcmChan].active && chan[adpcmChan].keyOn && !chan[adpcmChan].keyOff) {
          if (chan[adpcmChan].sample>=0 && chan[adpcmChan].sample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(chan[adpcmChan].sample);
            immWrite(7,(s->isLoopable())?0xb0:0xa0); // start/repeat
          }
        }
        chan[adpcmChan].keyOn=false;
        chan[adpcmChan].keyOff=false;
      }
      chan[adpcmChan].freqChanged=false;
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      if ((i>=0x80 && i<0xa0)) {
        if (weWillWriteRRLater[i-0x80]) continue;
      } else if ((i>=0x180 && i<0x1a0)) {
        if (weWillWriteRRLater[32|(i-0x180)]) continue;
      }
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  bool updateDrums=false;
  for (int i=0; i<totalChans; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,octave(chan[i].baseFreq)*2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].fixedFreq>0) chan[i].freq=chan[i].fixedFreq;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>131071) chan[i].freq=131071;
      int freqt=toFreq(chan[i].freq);
      chan[i].freqH=freqt>>8;
      chan[i].freqL=freqt&0xff;
      immWrite(chanMap[i]+ADDR_FREQ,chan[i].freqL);
    }
    if (i<melodicChans) {
      if (chan[i].keyOn && !chan[i].hardReset) {
        immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(0x20));
        chan[i].keyOn=false;
      } else if (chan[i].freqChanged) {
        if (chan[i].keyOn && chan[i].hardReset) {
          immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|0);
        } else {
          immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(chan[i].active<<5));
        }
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

  // hard reset handling
  if (mustHardReset) {
    for (unsigned int i=hardResetElapsed; i<128; i++) {
      immWrite(0x3f,i&0xff);
    }
    for (int i=0x80; i<0xa0; i++) {
      if (weWillWriteRRLater[i-0x80]) {
        immWrite(i,pendingWrites[i]&0xff);
        oldWrites[i]=pendingWrites[i];
      }
    }
    for (int i=0x180; i<0x1a0; i++) {
      if (weWillWriteRRLater[32|(i-0x180)]) {
        immWrite(i,pendingWrites[i]&0xff);
        oldWrites[i]=pendingWrites[i];
      }
    }
    for (int i=0; i<melodicChans; i++) {
      if (chan[i].hardReset) {
        if (chan[i].keyOn) {
          immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(0x20));
          chan[i].keyOn=false;
        } else if (chan[i].freqChanged) {
          immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH|(chan[i].active<<5));
        }
      }
    }
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
  if (ch==adpcmChan) return;
  if (oplType<3 && ch<melodicChans) {
    fm.channel[outChanMap[ch]].muted=mute;
  }
  int ops=(slots[3][ch]!=255 && chan[ch].state.ops==4 && oplType==3)?4:2;
  if (ch&1 && ch<12) {
    if (chan[ch-1].fourOp) return;
  }
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
      if (KVSL(ch,i) || ch>melodicChans) {
        rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
      } else {
        rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
      }
    }
  }

  if (properDrums && ch>melodicChans) {
    return;
  }

  if (isMuted[ch]) {
    rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1));
    if (ops==4) {
      rWrite(chanMap[ch+1]+ADDR_LR_FB_ALG,((chan[ch].state.alg>>1)&1)|(chan[ch].state.fb<<1));
    }
  } else {
    rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&15)<<4));
    if (ops==4) {
      rWrite(chanMap[ch+1]+ADDR_LR_FB_ALG,((chan[ch].state.alg>>1)&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&15)<<4));
    }
  }
}

void DivPlatformOPL::commitState(int ch, DivInstrument* ins) {
  if (chan[ch].insChanged) {
    if (ch>melodicChans && ins->type==DIV_INS_OPL_DRUMS) {
      for (int i=0; i<4; i++) {
        chan[melodicChans+i+1].state.alg=ins->fm.alg;
        chan[melodicChans+i+1].state.fb=ins->fm.fb;
        chan[melodicChans+i+1].state.opllPreset=ins->fm.opllPreset;
        chan[melodicChans+i+1].state.fixedDrums=ins->fm.fixedDrums;
        chan[melodicChans+i+1].state.kickFreq=ins->fm.kickFreq;
        chan[melodicChans+i+1].state.snareHatFreq=ins->fm.snareHatFreq;
        chan[melodicChans+i+1].state.tomTopFreq=ins->fm.tomTopFreq;
        chan[melodicChans+i+1].state.op[0]=ins->fm.op[i];
      }
    } else {
      chan[ch].state=ins->fm;
    }
  }

  if (chan[ch].insChanged) {
    if (ch>melodicChans && ins->type==DIV_INS_OPL_DRUMS) {
      for (int i=0; i<4; i++) {
        int ch=melodicChans+1+i;
        unsigned char slot=slots[0][ch];
        if (slot==255) continue;
        unsigned short baseAddr=slotMap[slot];
        DivInstrumentFM::Operator& op=chan[ch].state.op[0];
        chan[ch].fourOp=false;

        if (isMuted[ch]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
        }

        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
        if (oplType>1) {
          rWrite(baseAddr+ADDR_WS,op.ws&((oplType==3)?7:3));
        }

        if (isMuted[ch]) {
          oldWrites[chanMap[ch]+ADDR_LR_FB_ALG]=-1;
          rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1));
        } else {
          oldWrites[chanMap[ch]+ADDR_LR_FB_ALG]=-1;
          rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&15)<<4));
        }
      }
    } else {
      int ops=(slots[3][ch]!=255 && chan[ch].state.ops==4 && oplType==3)?4:2;
      chan[ch].fourOp=(ops==4);
      if (chan[ch].fourOp) {
        /*
        if (chan[ch+1].active) {
          chan[ch+1].keyOff=true;
          chan[ch+1].keyOn=false;
          chan[ch+1].active=false;
        }*/
        chan[ch+1].insChanged=true;
        chan[ch+1].macroInit(NULL);
      }
      update4OpMask=true;
      for (int i=0; i<ops; i++) {
        unsigned char slot=slots[i][ch];
        if (slot==255) continue;
        unsigned short baseAddr=slotMap[slot];
        DivInstrumentFM::Operator& op=chan[ch].state.op[(ops==4)?orderedOpsL[i]:i];

        if (isMuted[ch]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          if (KVSL(ch,i) || ch>melodicChans) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
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

      if (isMuted[ch]) {
        oldWrites[chanMap[ch]+ADDR_LR_FB_ALG]=-1;
        rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1));
        if (ops==4) {
          oldWrites[chanMap[ch+1]+ADDR_LR_FB_ALG]=-1;
          rWrite(chanMap[ch+1]+ADDR_LR_FB_ALG,((chan[ch].state.alg>>1)&1)|(chan[ch].state.fb<<1));
        }
      } else {
        oldWrites[chanMap[ch]+ADDR_LR_FB_ALG]=-1;
        rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&15)<<4));
        if (ops==4) {
          oldWrites[chanMap[ch+1]+ADDR_LR_FB_ALG]=-1;
          rWrite(chanMap[ch+1]+ADDR_LR_FB_ALG,((chan[ch].state.alg>>1)&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&15)<<4));
        }
      }
    }
  }
}

int DivPlatformOPL::dispatch(DivCommand c) {
  if (c.chan>=totalChans && c.chan!=adpcmChan) return 0;
  // ineffective in 4-op mode
  if (oplType==3 && c.chan!=adpcmChan && c.chan<14 && (c.chan&1) && c.cmd!=DIV_CMD_GET_VOLMAX && c.cmd!=DIV_ALWAYS_SET_VOLUME) {
    if (chan[c.chan-1].fourOp) return 0;
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan==adpcmChan) { // ADPCM
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
        chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
        if (ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_ADPCMB) {
          chan[c.chan].furnacePCM=true;
        } else {
          chan[c.chan].furnacePCM=false;
        }
        if (skipRegisterWrites) break;
        if (chan[c.chan].furnacePCM) {
          chan[c.chan].macroInit(ins);
          chan[c.chan].fixedFreq=0;
          if (!chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
            immWrite(18,chan[c.chan].outVol);
          }
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].sample=ins->amiga.getSample(c.value);
            c.value=ins->amiga.getFreq(c.value);
          }
          if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(chan[c.chan].sample);
            immWrite(8,0);
            immWrite(9,(sampleOffB[chan[c.chan].sample]>>2)&0xff);
            immWrite(10,(sampleOffB[chan[c.chan].sample]>>10)&0xff);
            int end=sampleOffB[chan[c.chan].sample]+s->lengthB-1;
            immWrite(11,(end>>2)&0xff);
            immWrite(12,(end>>10)&0xff);
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].note=c.value;
              chan[c.chan].baseFreq=NOTE_ADPCMB(chan[c.chan].note);
              chan[c.chan].freqChanged=true;
            }
            chan[c.chan].active=true;
            chan[c.chan].keyOn=true;
          } else {
            immWrite(7,0x01); // reset
            immWrite(9,0);
            immWrite(10,0);
            immWrite(11,0);
            immWrite(12,0);
            break;
          }
        } else {
          chan[c.chan].sample=-1;
          chan[c.chan].macroInit(NULL);
          chan[c.chan].outVol=chan[c.chan].vol;
          if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
            break;
          }
          chan[c.chan].sample=12*sampleBank+c.value%12;
          if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(12*sampleBank+c.value%12);
            immWrite(8,0);
            immWrite(9,(sampleOffB[chan[c.chan].sample]>>2)&0xff);
            immWrite(10,(sampleOffB[chan[c.chan].sample]>>10)&0xff);
            int end=sampleOffB[chan[c.chan].sample]+s->lengthB-1;
            immWrite(11,(end>>2)&0xff);
            immWrite(12,(end>>10)&0xff);
            int freq=(65536.0*(double)s->rate)/(double)chipRateBase;
            chan[c.chan].fixedFreq=freq;
            immWrite(16,freq&0xff);
            immWrite(17,(freq>>8)&0xff);
            chan[c.chan].active=true;
            chan[c.chan].keyOn=true;
          } else {
            immWrite(7,0x01); // reset
            immWrite(9,0);
            immWrite(10,0);
            immWrite(11,0);
            immWrite(12,0);
          }
        }
        break;
      }
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,c.chan>melodicChans?DIV_INS_OPL_DRUMS:DIV_INS_OPL);

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      commitState(c.chan,ins);
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        if (c.chan>melodicChans && ins->type==DIV_INS_OPL_DRUMS && chan[c.chan].state.fixedDrums) {
          chan[melodicChans+1].fixedFreq=(chan[melodicChans+1].state.snareHatFreq&1023)<<(chan[melodicChans+1].state.snareHatFreq>>10);
          chan[melodicChans+2].fixedFreq=(chan[melodicChans+2].state.tomTopFreq&1023)<<(chan[melodicChans+2].state.tomTopFreq>>10);
          chan[melodicChans+3].fixedFreq=chan[melodicChans+2].fixedFreq;
          chan[melodicChans+4].fixedFreq=chan[melodicChans+1].fixedFreq;

          chan[melodicChans+1].freqChanged=true;
          chan[melodicChans+2].freqChanged=true;
          chan[melodicChans+3].freqChanged=true;
          chan[melodicChans+4].freqChanged=true;
        } else {
          if (c.chan>=melodicChans && (chan[c.chan].state.opllPreset==16 || ins->type==DIV_INS_OPL_DRUMS) && chan[c.chan].state.fixedDrums) { // drums
            if (c.chan==melodicChans) {
              chan[c.chan].fixedFreq=(chan[c.chan].state.kickFreq&1023)<<(chan[c.chan].state.kickFreq>>10);
            } else if (c.chan==melodicChans+1 || c.chan==melodicChans+4) {
              chan[c.chan].fixedFreq=(chan[c.chan].state.snareHatFreq&1023)<<(chan[c.chan].state.snareHatFreq>>10);
            } else if (c.chan==melodicChans+2 || c.chan==melodicChans+3) {
              chan[c.chan].fixedFreq=(chan[c.chan].state.tomTopFreq&1023)<<(chan[c.chan].state.tomTopFreq>>10);
            } else {
              chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
              chan[c.chan].fixedFreq=0;
            }
          } else {
            chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
            chan[c.chan].fixedFreq=0;
          }
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
      if (pretendYMU && c.chan!=adpcmChan) {
        c.value=pow(((double)c.value/127.0),0.5)*63.0;
        if (c.value<0) c.value=0;
        if (c.value>63) c.value=63;
      }
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (c.chan==adpcmChan) { // ADPCM-B
        immWrite(18,chan[c.chan].outVol);
        break;
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
          if (KVSL(c.chan,i) || c.chan>melodicChans) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
          }
        }
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      if (pretendYMU && c.chan!=adpcmChan) {
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
      if (c.chan==adpcmChan) break;
      chan[c.chan].pan&=~3;
      if (c.value==0 && c.value2==0 && compatPan) {
        chan[c.chan].pan|=3;
      } else {
        chan[c.chan].pan|=(c.value>0)|((c.value2>0)<<1);
      }
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (isMuted[c.chan]) {
        rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1));
        if (ops==4) {
          rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1));
        }
      } else {
        rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&15)<<4));
        if (ops==4) {
          rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&15)<<4));
        }
      }
      break;
    }
    case DIV_CMD_SURROUND_PANNING: {
      if (oplType!=3) break;
      if (c.chan==adpcmChan) break;

      if (c.value==2) {
        chan[c.chan].pan&=3;
        if (c.value2>0) chan[c.chan].pan|=4;
      } else if (c.value==3) {
        if (c.value2>0) chan[c.chan].pan|=8;
      } else {
        break;
      }

      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (isMuted[c.chan]) {
        rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1));
        if (ops==4) {
          rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1));
        }
      } else {
        rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&15)<<4));
        if (ops==4) {
          rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&15)<<4));
        }
      }
      break;
    }
    case DIV_CMD_PITCH: {
      if (c.chan==adpcmChan) {
        if (!chan[c.chan].furnacePCM) break;
      }
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=(c.chan==adpcmChan)?(NOTE_ADPCMB(c.value2)):(NOTE_FREQUENCY(c.value2));
      int newFreq;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*((parent->song.linearPitch==2)?1:octave(chan[c.chan].baseFreq));
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*((parent->song.linearPitch==2)?1:octave(chan[c.chan].baseFreq));
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      if (!chan[c.chan].portaPause && parent->song.linearPitch!=2) {
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
    case DIV_CMD_SAMPLE_BANK:
      if (adpcmChan<0) break;
      sampleBank=c.value;
      if (sampleBank>(int)(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      iface.sampleBank=sampleBank;
      break;
    case DIV_CMD_LEGATO: {
      if (chan[c.chan].insChanged) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
        commitState(c.chan,ins);
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].baseFreq=(c.chan==adpcmChan)?(NOTE_ADPCMB(c.value)):(NOTE_FREQUENCY(c.value));
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      if (c.chan==adpcmChan) break;
      if (c.value&2) {
        dvb=c.value&1;
      } else {
        dam=c.value&1;
      }
      immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
      break;
    }
    case DIV_CMD_FM_FB: {
      if (c.chan==adpcmChan) break;
      chan[c.chan].state.fb=c.value&7;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (isMuted[c.chan]) {
        rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1));
        if (ops==4) {
          rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1));
        }
      } else {
        rWrite(chanMap[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&15)<<4));
        if (ops==4) {
          rWrite(chanMap[c.chan+1]+ADDR_LR_FB_ALG,((chan[c.chan].state.alg>>1)&1)|(chan[c.chan].state.fb<<1)|((chan[c.chan].pan&15)<<4));
        }
      }
      break;
    }
    case DIV_CMD_FM_MULT: {
      if (c.chan==adpcmChan) break;
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
      if (c.chan==adpcmChan) break;
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
        if (KVSL(c.chan,c.value) || c.chan>melodicChans) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
        }
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.chan==adpcmChan) break;
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
    case DIV_CMD_FM_DR: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.dr=c.value2&15;
          rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.dr=c.value2&15;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|op.dr);
      }      
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.sl=c.value2&15;
          rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.sl=c.value2&15;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
      }      
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.rr=c.value2&15;
          rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.rr=c.value2&15;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|op.rr);
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.am=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.am=c.value2&1;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      break;
    }
    case DIV_CMD_FM_VIB: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.vib=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.vib=c.value2&1;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      break;
    }
    case DIV_CMD_FM_SUS: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.sus=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.sus=c.value2&1;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      break;
    }
    case DIV_CMD_FM_KSR: {
      if (c.chan==adpcmChan) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.ksr=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.ksr=c.value2&1;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,(op.am<<7)|(op.vib<<6)|(op.sus<<5)|(op.ksr<<4)|op.mult);
      }
      break;
    }
    case DIV_CMD_FM_WS: {
      if (c.chan==adpcmChan) break;
      if (oplType<2) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.ws=c.value2&7;
          rWrite(baseAddr+ADDR_WS,op.ws&((oplType==3)?7:3));
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.ws=c.value2&7;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        rWrite(baseAddr+ADDR_WS,op.ws&((oplType==3)?7:3));
      }
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.chan==adpcmChan) break;
      if (oplType<2) break;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (c.value<0) {
        for (int i=0; i<ops; i++) {
          unsigned char slot=slots[i][c.chan];
          if (slot==255) continue;
          unsigned short baseAddr=slotMap[slot];
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[i]:i];
          op.ksl=c.value2&3;
          if (isMuted[c.chan]) {
            rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
          } else {
            if (KVSL(c.chan,i) || c.chan>melodicChans) {
              rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
            } else {
              rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
            }
          }
        }
      } else {
        if (c.value>=ops) break;
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[(ops==4)?orderedOpsL[c.value]:c.value];
        op.ksl=c.value2&3;
        unsigned char slot=slots[c.value][c.chan];
        if (slot==255) break;
        unsigned short baseAddr=slotMap[slot];
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
        } else {
          if (KVSL(c.chan,c.value) || c.chan>melodicChans) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,op.tl|(op.ksl<<6));
          }
        }
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
    case DIV_CMD_FM_HARD_RESET:
      if (c.chan==adpcmChan) break;
      chan[c.chan].hardReset=c.value;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      if (c.chan==adpcmChan) return 255;
      if (pretendYMU) return 127;
      return 63;
      break;
    case DIV_CMD_PRE_PORTA:
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) {
        chan[c.chan].baseFreq=(c.chan==adpcmChan)?(NOTE_ADPCMB(chan[c.chan].note)):(NOTE_FREQUENCY(chan[c.chan].note));
      }
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
    //int ops=(slots[3][i]!=255 && chan[i].state.ops==4 && oplType==3)?4:2;
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    /*
    chan[i].fourOp=(ops==4);
    for (int j=0; j<ops; j++) {
      unsigned char slot=slots[j][i];
      if (slot==255) continue;
      unsigned short baseAddr=slotMap[slot];
      DivInstrumentFM::Operator& op=chan[i].state.op[(ops==4)?orderedOpsL[j]:j];

      if (isMuted[i]) {
        rWrite(baseAddr+ADDR_KSL_TL,63|(op.ksl<<6));
      } else {
        if (KVSL(i,j) || i>melodicChans) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
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
      rWrite(chanMap[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&1)|(chan[i].state.fb<<1)|((chan[i].pan&15)<<4));
      if (ops==4) {
        rWrite(chanMap[i+1]+ADDR_LR_FB_ALG,((chan[i].state.alg>>1)&1)|(chan[i].state.fb<<1)|((chan[i].pan&15)<<4));
      }
    }
    */
  }
  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
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

DivMacroInt* DivPlatformOPL::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformOPL::getPan(int ch) {
  if (totalOutputs<=1) return 0;
  /*if (chan[ch&(~1)].fourOp) {
    if (ch&1) {
      return ((chan[ch-1].pan&2)<<7)|(chan[ch-1].pan&1);
    } else {
      return ((chan[ch+1].pan&2)<<7)|(chan[ch+1].pan&1);
    }
  }*/
  return ((chan[ch].pan&1)<<8)|((chan[ch].pan&2)>>1);
}

DivChannelPair DivPlatformOPL::getPaired(int ch) {
  if (oplType==3 && ch<12 && !(ch&1)) {
    if (chan[ch].fourOp) {
      return DivChannelPair("4OP",ch+1);
    }
  }
  return DivChannelPair();
}

DivDispatchOscBuffer* DivPlatformOPL::getOscBuffer(int ch) {
  if (oplType==759 || chipType==8950) {
    if (ch>=totalChans+1) return NULL;
  } else {
    if (ch>=totalChans) return NULL;
  }
  if (oplType==3 && ch<12) {
    if (chan[ch&(~1)].fourOp) {
      if (ch&1) {
        return oscBuf[ch-1];
      } else {
        return oscBuf[ch+1];
      }
    }
  }
  return oscBuf[ch];
}

unsigned char* DivPlatformOPL::getRegisterPool() {
  return regPool;
}

int DivPlatformOPL::getRegisterPoolSize() {
  return (oplType<3)?256:512;
}

void DivPlatformOPL::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,512);

  dacVal=0;
  dacVal2=0;
  dacOut=0;
  dacOut3[0]=0;
  dacOut3[1]=0;
  dacOut3[2]=0;
  dacOut3[3]=0;
  lastSH=false;
  lastSH2=false;
  lastSY=false;
  waitingBusy=true;
  
  const unsigned int downsampledRate=(unsigned int)((double)rate*round(COLOR_NTSC/72.0)/(double)chipRateBase);
  
  if (emuCore==2) {
    if (chipType==3 || chipType==759 || chipType==4) {
      // reset 3
      memset(&fm_lle3,0,sizeof(fmopl3_t));
      fm_lle3.input.ic=0;
      for (int i=0; i<400; i++) {
        fm_lle3.input.mclk=1;
        FMOPL3_Clock(&fm_lle3);
        fm_lle3.input.mclk=0;
        FMOPL3_Clock(&fm_lle3);
      }
      fm_lle3.input.ic=1;
    } else {
      // reset 2
      memset(&fm_lle2,0,sizeof(fmopl2_t));
      fm_lle2.input.ic=0;
      for (int i=0; i<80; i++) {
        fm_lle2.input.mclk=1;
        FMOPL2_Clock(&fm_lle2);
        fm_lle2.input.mclk=0;
        FMOPL2_Clock(&fm_lle2);
      }
      fm_lle2.input.ic=1;
    }
  } else if (emuCore==1) {
    switch (chipType) {
      case 1:
        fm_ymfm1->reset();
        break;
      case 2:
        fm_ymfm2->reset();
        break;
      case 8950:
        fm_ymfm8950->reset();
        break;
      case 3: case 759:
        fm_ymfm3->reset();
        break;
    }
  } else {
    if (downsample) {
      OPL3_Reset(&fm,downsampledRate);
    } else {
      OPL3_Reset(&fm,rate);
    }
  }

  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  properDrums=properDrumsSys;
  if (oplType==3) {
    chanMap=properDrums?chanMapOPL3Drums:chanMapOPL3;
    outChanMap=outChanMapOPL3;
    melodicChans=properDrums?15:18;
    totalChans=properDrums?20:18;
  } else {
    chanMap=properDrums?chanMapOPL2Drums:chanMapOPL2;
    outChanMap=outChanMapOPL2;
    melodicChans=properDrums?6:9;
    totalChans=properDrums?11:9;
  }

  for (int i=0; i<totalChans; i++) {
    chan[i]=DivPlatformOPL::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x3f;
    chan[i].outVol=0x3f;
  }

  if (adpcmChan>=0) {
    chan[adpcmChan]=DivPlatformOPL::Channel();
    chan[adpcmChan].std.setEngine(parent);
    chan[adpcmChan].vol=0xff;
    chan[adpcmChan].outVol=0xff;

    adpcmB->reset();

    // volume
    immWrite(18,0xff);
    // ADPCM limit
    immWrite(20,0xff);
    immWrite(19,0xff);
  }
  
  if (oplType<3) for (int i=0; i<melodicChans; i++) {
    fm.channel[outChanMap[i]].muted=isMuted[i];
  }

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  lfoValue=8;
  drumState=0;
  sampleBank=0;

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

int DivPlatformOPL::getOutputCount() {
  return totalOutputs;
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
  for (int i=0; i<totalChans; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
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

void DivPlatformOPL::setCore(unsigned char which) {
  emuCore=which;
}

void DivPlatformOPL::setOPLType(int type, bool drums) {
  pretendYMU=false;
  downsample=false;
  adpcmChan=-1;
  switch (type) {
    case 1: case 2: case 8950:
      slotsNonDrums=slotsOPL2;
      slotsDrums=slotsOPL2Drums;
      slots=drums?slotsDrums:slotsNonDrums;
      chanMap=drums?chanMapOPL2Drums:chanMapOPL2;
      outChanMap=outChanMapOPL2;
      chipFreqBase=32768*72;
      chans=9;
      melodicChans=drums?6:9;
      totalChans=drums?11:9;
      if (type==8950) {
        adpcmChan=drums?11:9;
      }
      totalOutputs=1;
      break;
    case 3: case 4: case 759:
      slotsNonDrums=slotsOPL3;
      slotsDrums=slotsOPL3Drums;
      slots=drums?slotsDrums:slotsNonDrums;
      chanMap=drums?chanMapOPL3Drums:chanMapOPL3;
      outChanMap=outChanMapOPL3;
      chipFreqBase=32768*288;
      chans=18;
      melodicChans=drums?15:18;
      totalChans=drums?20:18;
      if (type==759) {
        pretendYMU=true;
        adpcmChan=16;
      } else if (type==4) {
        chipFreqBase=32768*684;
        downsample=true;
      }
      totalOutputs=(type==4)?6:4;
      break;
  }
  chipType=type;
  if (type==759 || type==4) {
    oplType=3;
  } else if (type==8950) {
    oplType=1;
  } else {
    oplType=type;
  }
  properDrumsSys=drums;
}

void DivPlatformOPL::setFlags(const DivConfig& flags) {
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

  compatPan=false;

  switch (chipType) {
    default:
    case 1: case 2: case 8950:
      switch (flags.getInt("clockSel",0)) {
        case 0x01:
          chipClock=COLOR_PAL*4.0/5.0;
          break;
        case 0x02:
          chipClock=4000000.0;
          break;
        case 0x03:
          chipClock=3000000.0;
          break;
        case 0x04:
          chipClock=38400*13*8; // 31948800/8
          break;
        case 0x05:
          chipClock=3500000.0;
          break;
        default:
          chipClock=COLOR_NTSC;
          break;
      }
      CHECK_CUSTOM_CLOCK;
      rate=chipClock/72;
      chipRateBase=rate;
      break;
    case 3:
      switch (flags.getInt("clockSel",0)) {
        case 0x01:
          chipClock=COLOR_PAL*16.0/5.0;
          break;
        case 0x02:
          chipClock=14000000.0;
          break;
        case 0x03:
          chipClock=16000000.0;
          break;
        case 0x04:
          chipClock=15000000.0;
          break;
        case 0x05:
          chipClock=33868800.0;
          break;
        default:
          chipClock=COLOR_NTSC*4.0;
          break;
      }
      CHECK_CUSTOM_CLOCK;
      switch (flags.getInt("chipType",0)) {
        case 1: // YMF289B
          chipFreqBase=32768*684;
          rate=chipClock/768;
          chipRateBase=chipClock/684;
          downsample=true;
          totalOutputs=2; // Stereo output only
          break;
        default: // YMF262
          chipFreqBase=32768*288;
          rate=chipClock/288;
          chipRateBase=rate;
          downsample=false;
          totalOutputs=4;
          break;
      }
      if (emuCore!=1 && emuCore!=2) {
        if (downsample) {
          const unsigned int downsampledRate=(unsigned int)((double)rate*round(COLOR_NTSC/72.0)/(double)chipRateBase);
          OPL3_Resample(&fm,downsampledRate);
        } else {
          OPL3_Resample(&fm,rate);
        }
      }
      break;
    case 4:
      switch (flags.getInt("clockSel",0)) {
        case 0x01:
          chipClock=COLOR_PAL*32.0/5.0;
          break;
        case 0x02:
          chipClock=33868800.0;
          break;
        default:
          chipClock=COLOR_NTSC*8.0;
          break;
      }
      CHECK_CUSTOM_CLOCK;
      rate=chipClock/768;
      chipRateBase=chipClock/684;
      break;
    case 759:
      rate=48000;
      chipRateBase=rate;
      chipClock=rate*288;
      break;
  }
  compatPan=flags.getBool("compatPan",false);

  for (int i=0; i<20; i++) {
    oscBuf[i]->rate=rate;
  }
}

const void* DivPlatformOPL::getSampleMem(int index) {
  return (index==0 && adpcmChan>=0) ? adpcmBMem : NULL;
}

size_t DivPlatformOPL::getSampleMemCapacity(int index) {
  return (index==0 && adpcmChan>=0) ? 262144 : 0;
}

size_t DivPlatformOPL::getSampleMemUsage(int index) {
  return (index==0 && adpcmChan>=0) ? adpcmBMemLen : 0;
}

bool DivPlatformOPL::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformOPL::renderSamples(int sysID) {
  if (adpcmChan<0) return;
  memset(adpcmBMem,0,getSampleMemCapacity(0));
  memset(sampleOffB,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOffB[i]=0;
      continue;
    }

    int paddedLen=(s->lengthB+255)&(~0xff);
    if ((memPos&0xf00000)!=((memPos+paddedLen)&0xf00000)) {
      memPos=(memPos+0xfffff)&0xf00000;
    }
    if (memPos>=getSampleMemCapacity(0)) {
      logW("out of ADPCM memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=getSampleMemCapacity(0)) {
      memcpy(adpcmBMem+memPos,s->dataB,getSampleMemCapacity(0)-memPos);
      logW("out of ADPCM memory for sample %d!",i);
    } else {
      memcpy(adpcmBMem+memPos,s->dataB,paddedLen);
      sampleLoaded[i]=true;
    }
    sampleOffB[i]=memPos;
    memPos+=paddedLen;
  }
  adpcmBMemLen=memPos+256;
}

int DivPlatformOPL::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<20; i++) {
    isMuted[i]=false;
  }
  for (int i=0; i<20; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  fm_ymfm1=NULL;
  fm_ymfm2=NULL;
  fm_ymfm8950=NULL;
  fm_ymfm3=NULL;

  if (emuCore==1) {
    switch (chipType) {
      case 1:
        fm_ymfm1=new ymfm::ym3526(iface);
        break;
      case 2:
        fm_ymfm2=new ymfm::ym3812(iface);
        break;
      case 8950:
        fm_ymfm8950=new ymfm::y8950(iface);
        break;
      case 3: case 759:
        fm_ymfm3=new ymfm::ymf262(iface);
        break;
    }
  }

  setFlags(flags);

  if (adpcmChan>=0) {
    adpcmBMem=new unsigned char[getSampleMemCapacity(0)];
    adpcmBMemLen=0;
    iface.adpcmBMem=adpcmBMem;
    iface.sampleBank=0;
    adpcmB=new ymfm::adpcm_b_engine(iface,2);
  }

  reset();
  return totalChans;
}

void DivPlatformOPL::quit() {
  for (int i=0; i<20; i++) {
    delete oscBuf[i];
  }
  if (adpcmChan>=0) {
    delete adpcmB;
    delete[] adpcmBMem;
  }
  if (fm_ymfm1!=NULL) {
    delete fm_ymfm1;
    fm_ymfm1=NULL;
  }
  if (fm_ymfm2!=NULL) {
    delete fm_ymfm2;
    fm_ymfm2=NULL;
  }
  if (fm_ymfm8950!=NULL) {
    delete fm_ymfm8950;
    fm_ymfm8950=NULL;
  }
  if (fm_ymfm3!=NULL) {
    delete fm_ymfm3;
    fm_ymfm3=NULL;
  }
}

DivPlatformOPL::~DivPlatformOPL() {
}
