/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "../bsr.h"
#include "../../ta-log.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define KVSL(x,y) ((chan[x].state.op[orderedOpsL1[ops==4][y]].kvs==2 && isOutputL[ops==4][chan[x].state.alg][y]) || chan[x].state.op[orderedOpsL1[ops==4][y]].kvs==1)

#define CHIP_FREQBASE chipFreqBase
#define PCM_FREQBASE (402653184)

#define NOTE_PCM(x) parent->calcBaseFreq(chipClock,PCM_FREQBASE,x,false)

#define PCM_CHECK(ch) ((chipType==4) && (ch>=pcmChanOffs))
#define PCM_REG(ch) (ch-pcmChanOffs)

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


#define PCM_ADDR_WAVE_L 0x208 // Wavetable number LSB
#define PCM_ADDR_WAVE_H_FN_L 0x220 // Wavetable number MSB, F-number LSB
#define PCM_ADDR_FN_H_PR_OCT 0x238 // F-number MSB, Pseudo-reverb, Octave
#define PCM_ADDR_TL 0x250 // Total level, Level direct
#define PCM_ADDR_KEY_DAMP_LFORST_CH_PAN 0x268 // Key, Damp, LFO Reset, Channel select, Panpot

#define PCM_ADDR_LFO_VIB 0x280
#define PCM_ADDR_AR_D1R 0x298
#define PCM_ADDR_DL_D2R 0x2b0
#define PCM_ADDR_RC_RR 0x2c8
#define PCM_ADDR_AM 0x2e0

#define PCM_ADDR_MIX_FM 0x2f8
#define PCM_ADDR_MIX_PCM 0x2f9

void DivPlatformOPL::acquire_nuked(short** buf, size_t len) {
  thread_local short o[8];
  thread_local int os[6];
  thread_local ymfm::ymfm_output<2> aOut;
  thread_local short pcmBuf[24];

  for (int i=0; i<MAX(adpcmChan+1,totalChans); i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    os[0]=0; os[1]=0; os[2]=0; os[3]=0; os[4]=0; os[5]=0;
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        delay=1;
        if (w.addr>=0x200) {
          pcm.writeReg(w.addr&0xff,w.val);
          regPool[0x200|(w.addr&0xff)]=w.val;
        } else {
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
        }
      }
      writes.pop();
    }

    if (downsample) {
      OPL3_Generate4ChResampled(&fm,o);
    } else {
      OPL3_Generate4Ch(&fm,o);
    }
    if (chipType==4) {
      pcm.generateMix(o[0],o[1],o[4],o[5],o[6],o[7],pcmBuf);
      os[0]+=o[4]; // FM + PCM left
      os[1]+=o[5]; // FM + PCM right
      os[2]+=o[2]; // FM left
      os[3]+=o[3]; // FM right
      os[4]+=o[6]; // PCM left
      os[5]+=o[7]; // PCM right
    } else {
      os[0]+=o[0];
      os[1]+=o[1];
      os[2]+=o[2];
      os[3]+=o[3];
    }

    if (adpcmChan>=0) {
      adpcmB->clock();
      aOut.clear();
      adpcmB->output<2>(aOut,0);

      if (!isMuted[adpcmChan]) {
        os[0]-=aOut.data[0]>>3;
        os[1]-=aOut.data[0]>>3;
        oscBuf[adpcmChan]->putSample(h,aOut.data[0]>>1);
      } else {
        oscBuf[adpcmChan]->putSample(h,0);
      }
    }

    if (properDrums) {
      for (int i=0; i<melodicChans+1; i++) {
        unsigned char ch=outChanMap[i];
        int chOut=0;
        if (ch==255) continue;
        if (isMuted[i]) continue;
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
        oscBuf[i]->putSample(h,CLAMP(chOut<<(i==melodicChans?1:2),-32768,32767));
      }
      // special
      oscBuf[melodicChans+1]->putSample(h,fm.slot[16].out*4);
      oscBuf[melodicChans+2]->putSample(h,fm.slot[14].out*4);
      oscBuf[melodicChans+3]->putSample(h,fm.slot[17].out*4);
      oscBuf[melodicChans+4]->putSample(h,fm.slot[13].out*4);
    } else {
      for (int i=0; i<chans; i++) {
        unsigned char ch=outChanMap[i];
        int chOut=0;
        if (ch==255) continue;
        if (isMuted[i]) continue;
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
        oscBuf[i]->putSample(h,CLAMP(chOut<<2,-32768,32767));
      }
    }

    if (chipType==4) {
      for (int i=pcmChanOffs; i<pcmChanOffs+24; i++) {
        oscBuf[i]->putSample(h,CLAMP(pcmBuf[i-pcmChanOffs],-32768,32767));
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
  
    if (os[4]<-32768) os[4]=-32768;
    if (os[4]>32767) os[4]=32767;
  
    if (os[5]<-32768) os[5]=-32768;
    if (os[5]>32767) os[5]=32767;
  
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
      buf[4][h]=os[4];
      buf[5][h]=os[5];
    }
  }

  for (int i=0; i<MAX(adpcmChan+1,totalChans); i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformOPL::acquire_ymfm1(short** buf, size_t len) {
  ymfm::ymfm_output<1> out;

  ymfm::ym3526::fm_engine* fme=fm_ymfm1->debug_fm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<1>>* fmChan[9];

  for (int i=0; i<9; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        fm_ymfm1->write(0,w.addr);
        fm_ymfm1->write(1,w.val);
        delay=1;

        regPool[w.addr&511]=w.val;
      }
      writes.pop();
    }

    fm_ymfm1->generate(&out,1);

    buf[0][h]=out.data[0];

    if (properDrums) {
      for (int i=0; i<7; i++) {
        if (isMuted[i]) continue;
        oscBuf[i]->putSample(h,CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767));
      }
      oscBuf[7]->putSample(h,CLAMP(fmChan[7]->debug_special1()<<2,-32768,32767));
      oscBuf[8]->putSample(h,CLAMP(fmChan[8]->debug_special1()<<2,-32768,32767));
      oscBuf[9]->putSample(h,CLAMP(fmChan[8]->debug_special2()<<2,-32768,32767));
      oscBuf[10]->putSample(h,CLAMP(fmChan[7]->debug_special2()<<2,-32768,32767));
    } else {
      for (int i=0; i<9; i++) {
        if (isMuted[i]) continue;
        oscBuf[i]->putSample(h,CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767));
      }
    }
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformOPL::acquire_ymfm2(short** buf, size_t len) {
  ymfm::ymfm_output<1> out;

  ymfm::ym3812::fm_engine* fme=fm_ymfm2->debug_fm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<2>>* fmChan[9];

  for (int i=0; i<9; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        fm_ymfm2->write(0,w.addr);
        fm_ymfm2->write(1,w.val);
        delay=1;

        regPool[w.addr&511]=w.val;
      }
      writes.pop();
    }

    fm_ymfm2->generate(&out,1);

    buf[0][h]=out.data[0];

    if (properDrums) {
      for (int i=0; i<7; i++) {
        if (isMuted[i]) continue;
        oscBuf[i]->putSample(h,CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767));
      }
      oscBuf[7]->putSample(h,CLAMP(fmChan[7]->debug_special1()<<2,-32768,32767));
      oscBuf[8]->putSample(h,CLAMP(fmChan[8]->debug_special1()<<2,-32768,32767));
      oscBuf[9]->putSample(h,CLAMP(fmChan[8]->debug_special2()<<2,-32768,32767));
      oscBuf[10]->putSample(h,CLAMP(fmChan[7]->debug_special2()<<2,-32768,32767));
    } else {
      for (int i=0; i<9; i++) {
        if (isMuted[i]) continue;
        oscBuf[i]->putSample(h,CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767));
      }
    }
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->end(len);
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

  for (int i=0; i<totalChans+1; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        fm_ymfm8950->write(0,w.addr);
        fm_ymfm8950->write(1,w.val);
        delay=1;

        regPool[w.addr&511]=w.val;
      }
      writes.pop();
    }

    fm_ymfm8950->generate(&out,1);

    buf[0][h]=out.data[0];

    if (properDrums) {
      for (int i=0; i<7; i++) {
        if (isMuted[i]) continue;
        oscBuf[i]->putSample(h,CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767));
      }
      oscBuf[7]->putSample(h,CLAMP(fmChan[7]->debug_special1()<<2,-32768,32767));
      oscBuf[8]->putSample(h,CLAMP(fmChan[8]->debug_special1()<<2,-32768,32767));
      oscBuf[9]->putSample(h,CLAMP(fmChan[8]->debug_special2()<<2,-32768,32767));
      oscBuf[10]->putSample(h,CLAMP(fmChan[7]->debug_special2()<<2,-32768,32767));
      oscBuf[11]->putSample(h,CLAMP(abe->get_last_out(0)<<2,-32768,32767));
    } else {
      for (int i=0; i<9; i++) {
        if (isMuted[i]) continue;
        oscBuf[i]->putSample(h,CLAMP(fmChan[i]->debug_output(0)<<2,-32768,32767));
      }
      oscBuf[9]->putSample(h,CLAMP(abe->get_last_out(0)<<2,-32768,32767));
    }
  }

  for (int i=0; i<totalChans+1; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformOPL::acquire_ymfm3(short** buf, size_t len) {
  ymfm::ymfm_output<4> out;

  ymfm::ymf262::fm_engine* fme=fm_ymfm3->debug_fm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<3>>* fmChan[18];

  for (int i=0; i<18; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        fm_ymfm3->write((w.addr&0x100)?2:0,w.addr);
        fm_ymfm3->write(1,w.val);
        delay=1;

        regPool[w.addr&511]=w.val;
      }
      writes.pop();
    }

    fm_ymfm3->generate(&out,1);

    if (downsample) {
      // 49716-44100
      downsamplerStep+=5616;
      if (downsamplerStep>=44100) {
        downsamplerStep-=44100;
        h--;
        continue;
      }
    }

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
        if (isMuted[i]) continue;
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
          oscBuf[i]->putSample(h,CLAMP(chOut,-32768,32767));
        } else {
          oscBuf[i]->putSample(h,CLAMP(chOut<<1,-32768,32767));
        }
      }
      oscBuf[16]->putSample(h,CLAMP(fmChan[7]->debug_special2()<<1,-32768,32767));
      oscBuf[17]->putSample(h,CLAMP(fmChan[8]->debug_special1()<<1,-32768,32767));
      oscBuf[18]->putSample(h,CLAMP(fmChan[8]->debug_special2()<<1,-32768,32767));
      oscBuf[19]->putSample(h,CLAMP(fmChan[7]->debug_special1()<<1,-32768,32767));
    } else {
      for (int i=0; i<18; i++) {
        unsigned char ch=outChanMap[i];
        if (ch==255) continue;
        if (isMuted[i]) continue;
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
        oscBuf[i]->putSample(h,CLAMP(chOut<<1,-32768,32767));
      }
    }
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformOPL::acquire_ymfm4(short** buf, size_t len) {
  ymfm::ymfm_output<6> out;

  ymfm::ymf278b::fm_engine* fme=fm_ymfm4->debug_fm_engine();
  ymfm::pcm_engine* pcme=fm_ymfm4->debug_pcm_engine();
  ymfm::fm_channel<ymfm::opl_registers_base<4>>* fmChan[18];
  ymfm::pcm_channel* pcmChan[24];

  for (int i=0; i<18; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (int i=0; i<24; i++) {
    pcmChan[i]=pcme->debug_channel(i);
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        fm_ymfm4->write((w.addr&0x200)?4:(w.addr&0x100)?2:0,w.addr);
        fm_ymfm4->write((w.addr&0x200)?5:1,w.val);
        delay=1;

        regPool[(w.addr&0x200)?(0x200+(w.addr&255)):(w.addr&511)]=w.val;
      }
      writes.pop();
    }

    fm_ymfm4->generate(&out,1);

    buf[0][h]=out.data[4]>>1; // FM + PCM left
    if (totalOutputs>1) {
      buf[1][h]=out.data[5]>>1; // FM + PCM right
    }
    if (totalOutputs>2) {
      buf[2][h]=out.data[0]>>1; // FM left
    }
    if (totalOutputs>3) {
      buf[3][h]=out.data[1]>>1; // FM right
    }
    if (totalOutputs==6) {
      buf[4][h]=out.data[2]>>1; // PCM left
      buf[5][h]=out.data[3]>>1; // PCM right
    }

    if (properDrums) {
      for (int i=0; i<16; i++) {
        unsigned char ch=(i<12 && chan[i&(~1)].fourOp)?outChanMap[i^1]:outChanMap[i];
        if (ch==255) continue;
        if (isMuted[i]) continue;
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
          oscBuf[i]->putSample(h,CLAMP(chOut,-32768,32767));
        } else {
          oscBuf[i]->putSample(h,CLAMP(chOut<<1,-32768,32767));
        }
      }
      oscBuf[16]->putSample(h,CLAMP(fmChan[7]->debug_special2()<<1,-32768,32767));
      oscBuf[17]->putSample(h,CLAMP(fmChan[8]->debug_special1()<<1,-32768,32767));
      oscBuf[18]->putSample(h,CLAMP(fmChan[8]->debug_special2()<<1,-32768,32767));
      oscBuf[19]->putSample(h,CLAMP(fmChan[7]->debug_special1()<<1,-32768,32767));
    } else {
      for (int i=0; i<18; i++) {
        unsigned char ch=outChanMap[i];
        if (ch==255) continue;
        if (isMuted[i]) continue;
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
        oscBuf[i]->putSample(h,CLAMP(chOut<<1,-32768,32767));
      }
    }
    for (int i=0; i<24; i++) {
      unsigned char oscOffs=i+pcmChanOffs;
      int chOut=pcmChan[i]->debug_output(0);
      chOut+=pcmChan[i]->debug_output(1);
      chOut+=pcmChan[i]->debug_output(2);
      chOut+=pcmChan[i]->debug_output(3);
      oscBuf[oscOffs]->putSample(h,CLAMP(chOut<<1,-32768,32767));
    }
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->end(len);
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

static const int cycleMap3[36]={
  14, 12, 13, 14, 0, 2, 4, 0, 2,
  4, 1, 3, 5, 1, 3, 5, 15, 16,
  17, 15, 16, 17, 6, 8, 10, 6, 8, 10, 7, 9, 11, 7, 9, 11, 12, 13
};

static const int cycleMap3Drums[36]={
  14, 12, 13, 14, 0, 2, 4, 0, 2,
  4, 1, 3, 5, 1, 3, 5, 15, 19,
  17, 15, 16, 18, 6, 8, 10, 6, 8, 10, 7, 9, 11, 7, 9, 11, 12, 13
};

void DivPlatformOPL::acquire_nukedLLE2(short** buf, size_t len) {
  int chOut[11];
  thread_local ymfm::ymfm_output<2> aOut;

  for (int i=0; i<MAX(adpcmChan+1,totalChans); i++) {
    oscBuf[i]->begin(len);
  }

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
      if (isMuted[i]) continue;
      if (i>=6 && properDrums) {
        chOut[i]<<=1;
      } else {
        chOut[i]<<=2;
      }
      if (chOut[i]<-32768) chOut[i]=-32768;
      if (chOut[i]>32767) chOut[i]=32767;
      oscBuf[i]->putSample(h,chOut[i]);
    }

    if (chipType==8950) {
      adpcmB->clock();
      aOut.clear();
      adpcmB->output<2>(aOut,0);

      if (!isMuted[adpcmChan]) {
        dacOut-=aOut.data[0]>>3;
        oscBuf[adpcmChan]->putSample(h,aOut.data[0]>>1);
      } else {
        oscBuf[adpcmChan]->putSample(h,0);
      }
    }

    if (dacOut<-32768) dacOut=-32768;
    if (dacOut>32767) dacOut=32767;

    buf[0][h]=dacOut;
  }

  for (int i=0; i<MAX(adpcmChan+1,totalChans); i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformOPL::acquire_nukedLLE3(short** buf, size_t len) {
  int chOut[20];
  int ch=0;

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    int curCycle=0;
    unsigned char subCycle=0;

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
            delay=18;
          } else {
            fm_lle3.input.cs=0;
            fm_lle3.input.rd=1;
            fm_lle3.input.wr=0;
            fm_lle3.input.address=(w.addr&0x100)?2:0;
            fm_lle3.input.data_i=w.addr&0xff;
            w.addrOrVal=true;
            // weird. wasn't it 12?
            delay=18;
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

      if (!(++subCycle&1)) {
        if (properDrums) {
          ch=cycleMap3Drums[curCycle];
        } else {
          ch=cycleMap3[curCycle];
        }

        if (ch<12 && !(ch&1) && chan[ch&(~1)].state.alg>0) ch|=1;

        if (ch>=12 || (ch&1) || !chan[ch&(~1)].fourOp) {
          if (fm_lle3.op_value_debug&0x1000) {
            chOut[ch]+=(fm_lle3.op_value_debug|0xfffff000)<<1;
          } else {
            chOut[ch]+=(fm_lle3.op_value_debug)<<1;
          }
        }
        curCycle++;
      }

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
      if (isMuted[i]) continue;
      if (chOut[i]<-32768) chOut[i]=-32768;
      if (chOut[i]>32767) chOut[i]=32767;
      oscBuf[i]->putSample(h,chOut[i]);
    }

    for (int i=0; i<MIN(4,totalOutputs); i++) {
      if (dacOut3[i]<-32768) dacOut3[i]=-32768;
      if (dacOut3[i]>32767) dacOut3[i]=32767;

      buf[i][h]=dacOut3[i];
    }
  }

  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->end(len);
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
      case 4:
        acquire_ymfm4(buf,len);
        break;
    }
  } else { // OPL3
    acquire_nuked(buf,len);
  }
}

double DivPlatformOPL::NOTE_ADPCMB(int note) {
  if (adpcmChan<0) return 0;
  if (chan[adpcmChan].sample>=0 && chan[adpcmChan].sample<parent->song.sampleLen) {
    double off=65535.0*(double)(parent->getSample(chan[adpcmChan].sample)->centerRate)/parent->getCenterRate();
    return parent->calcBaseFreq((double)chipClock/(compatYPitch?144:72),off,note,false);
  }
  return 0;
}

void DivPlatformOPL::tick(bool sysTick) {
  for (int i=0; i<totalChans; i++) {
    if (PCM_CHECK(i)) { // OPL4 PCM
      chan[i].std.next();
      if (chan[i].std.vol.had) {
        chan[i].outVol=VOL_SCALE_LOG((chan[i].vol&0x7f),(0x7f*chan[i].std.vol.val)/chan[i].macroVolMul,0x7f);
        immWrite(PCM_ADDR_TL+(PCM_REG(i)),((0x7f-chan[i].outVol)<<1)|(chan[i].levelDirect?1:0));
      }

      if (NEW_ARP_STRAT) {
        chan[i].handleArp();
      } else if (chan[i].std.arp.had) {
        if (!chan[i].inPorta) {
          chan[i].baseFreq=NOTE_PCM(parent->calcArp(chan[i].note,chan[i].std.arp.val));
        }
        chan[i].freqChanged=true;
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
          chan[i].writeCtrl=true;
        }
      }

      if (chan[i].std.panL.had) { // panning
        chan[i].pan=chan[i].std.panL.val&0xf;
        chan[i].freqChanged=true;
        chan[i].writeCtrl=true;
      }

      if (chan[i].std.ex1.had) {
        chan[i].lfo=chan[i].std.ex1.val&0x7;
        rWrite(PCM_ADDR_LFO_VIB+PCM_REG(i),(chan[i].lfo<<3)|(chan[i].vib));
      }

      if (chan[i].std.fms.had) {
        chan[i].vib=chan[i].std.fms.val&0x7;
        rWrite(PCM_ADDR_LFO_VIB+PCM_REG(i),(chan[i].lfo<<3)|(chan[i].vib));
      }

      if (chan[i].std.ams.had) {
        chan[i].am=chan[i].std.ams.val&0x7;
        rWrite(PCM_ADDR_AM+PCM_REG(i),chan[i].am);
      }

      if (chan[i].std.ex2.had) {
        chan[i].ar=chan[i].std.ex2.val&0xf;
        rWrite(PCM_ADDR_AR_D1R+PCM_REG(i),(chan[i].ar<<4)|(chan[i].d1r));
      }

      if (chan[i].std.ex3.had) {
        chan[i].d1r=chan[i].std.ex3.val&0xf;
        rWrite(PCM_ADDR_AR_D1R+PCM_REG(i),(chan[i].ar<<4)|(chan[i].d1r));
      }

      if (chan[i].std.ex4.had) {
        chan[i].dl=chan[i].std.ex4.val&0xf;
        rWrite(PCM_ADDR_DL_D2R+PCM_REG(i),(chan[i].dl<<4)|(chan[i].d2r));
      }

      if (chan[i].std.ex5.had) {
        chan[i].d2r=chan[i].std.ex5.val&0xf;
        rWrite(PCM_ADDR_DL_D2R+PCM_REG(i),(chan[i].dl<<4)|(chan[i].d2r));
      }

      if (chan[i].std.ex6.had) {
        chan[i].rc=chan[i].std.ex6.val&0xf;
        rWrite(PCM_ADDR_RC_RR+PCM_REG(i),(chan[i].rc<<4)|(chan[i].rr));
      }

      if (chan[i].std.ex7.had) {
        chan[i].rr=chan[i].std.ex7.val&0xf;
        rWrite(PCM_ADDR_RC_RR+PCM_REG(i),(chan[i].rc<<4)|(chan[i].rr));
      }

    } else {
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
        if (isMuted[i] && i<=melodicChans) {
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
  }

  int hardResetElapsed=0;
  bool mustHardReset=false;
  bool weWillWriteRRLater[64];

  memset(weWillWriteRRLater,0,64*sizeof(bool));

  unsigned char opMask=(int)(chan[0].fourOp)|(chan[2].fourOp<<1)|(chan[4].fourOp<<2)|(chan[6].fourOp<<3)|(chan[8].fourOp<<4)|(chan[10].fourOp<<5);

  // write ops which are being enabled
  if (update4OpMask) {
    if (oplType==3) {
      immWrite(0x104,opMask|oldOpMask);
      //printf("updating opMask to %.2x\n",opMask);
    }
  }

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

  // and now the ones being disabled
  if (update4OpMask) {
    update4OpMask=false;
    if (oplType==3) {
      immWrite(0x104,opMask);
      //printf("updating opMask to %.2x\n",opMask);
    }
    oldOpMask=opMask;
  }

  // update drums
  if (properDrums) {
    bool updateDrums=false;
    for (int i=melodicChans; i<melodicChans+5; i++) {
      if (chan[i].keyOn || chan[i].keyOff) {
        drumState&=~(1<<(melodicChans+4-i));
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
        immWrite(18,(isMuted[adpcmChan]?0:chan[adpcmChan].outVol));
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
        double off=65535.0*(double)(parent->getSample(chan[adpcmChan].sample)->centerRate)/parent->getCenterRate();
        chan[adpcmChan].freq=parent->calcFreq(chan[adpcmChan].baseFreq,chan[adpcmChan].pitch,chan[adpcmChan].fixedArp?chan[adpcmChan].baseNoteOverride:chan[adpcmChan].arpOff,chan[adpcmChan].fixedArp,false,4,chan[adpcmChan].pitch2,(double)chipClock/(compatYPitch?144:72),off);
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

  for (int i=0; i<768; i++) {
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
    if (PCM_CHECK(i)) { // OPL4 PCM
      if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
        DivSample* s=parent->getSample(chan[i].sample);
        unsigned char ctrl=0;
        double off=(s->centerRate>=1)?((double)s->centerRate/parent->getCenterRate()):1.0;
        chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,(524288*768)));
        if (chan[i].freq<0x400) chan[i].freq=0x400;
        chan[i].freqH=0;
        if (chan[i].freq>0x3ffffff) {
          chan[i].freq=0x3ffffff;
          chan[i].freqH=15;
        } else if (chan[i].freq>=0x800) {
          chan[i].freqH=bsr32(chan[i].freq)-11;
        }
        chan[i].freqL=(chan[i].freq>>chan[i].freqH)&0x3ff;
        chan[i].freqH=8^chan[i].freqH;
        ctrl|=(chan[i].active?0x80:0)|(chan[i].damp?0x40:0)|(chan[i].lfoReset?0x20:0)|(chan[i].ch?0x10:0)|(isMuted[i]?8:(chan[i].pan&0xf));
        unsigned int waveNum=chan[i].sample;
        if (ramSize<=0x200000) {
          waveNum=CLAMP(waveNum,0,0x7f)|0x180;
        }
        if (chan[i].keyOn) {
          immWrite(PCM_ADDR_KEY_DAMP_LFORST_CH_PAN+PCM_REG(i),ctrl&~0x80); // force keyoff first
          immWrite(PCM_ADDR_WAVE_H_FN_L+PCM_REG(i),((chan[i].freqL&0x7f)<<1)|((waveNum>>8)&1));
          immWrite(PCM_ADDR_WAVE_L+PCM_REG(i),waveNum&0xff);
          immWrite(PCM_ADDR_LFO_VIB+PCM_REG(i),(chan[i].lfo<<3)|(chan[i].vib));
          immWrite(PCM_ADDR_AR_D1R+PCM_REG(i),(chan[i].ar<<4)|(chan[i].d1r));
          immWrite(PCM_ADDR_DL_D2R+PCM_REG(i),(chan[i].dl<<4)|(chan[i].d2r));
          immWrite(PCM_ADDR_RC_RR+PCM_REG(i),(chan[i].rc<<4)|(chan[i].rr));
          immWrite(PCM_ADDR_AM+PCM_REG(i),chan[i].am);
          if (!chan[i].std.vol.had) {
            chan[i].outVol=chan[i].vol;
            immWrite(PCM_ADDR_TL+(PCM_REG(i)),((0x7f-chan[i].outVol)<<1)|(chan[i].levelDirect?1:0));
          }
          chan[i].writeCtrl=true;
          chan[i].keyOn=false;
        }
        if (chan[i].keyOff) {
          chan[i].writeCtrl=true;
          chan[i].keyOff=false;
        }
        if (chan[i].freqChanged) {
          immWrite(PCM_ADDR_WAVE_H_FN_L+PCM_REG(i),((chan[i].freqL&0x7f)<<1)|((waveNum>>8)&1));
          immWrite(PCM_ADDR_FN_H_PR_OCT+PCM_REG(i),((chan[i].freqH&0xf)<<4)|(chan[i].pseudoReverb?0x08:0x00)|((chan[i].freqL>>7)&0x7));
          chan[i].freqChanged=false;
        }
        if (chan[i].writeCtrl) {
          immWrite(PCM_ADDR_KEY_DAMP_LFORST_CH_PAN+PCM_REG(i),ctrl);
          chan[i].writeCtrl=false;
        }
      }
    } else {
      if (chan[i].freqChanged) {
        int mul=2;
        int fixedBlock=chan[i].state.block;
        if (parent->song.linearPitch!=2) {
          mul=octave(chan[i].baseFreq,fixedBlock)*2;
        }
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,mul,chan[i].pitch2,chipClock,CHIP_FREQBASE);
        if (chan[i].fixedFreq>0) chan[i].freq=chan[i].fixedFreq;
        if (chan[i].freq<0) chan[i].freq=0;
        if (chan[i].freq>131071) chan[i].freq=131071;
        int freqt=toFreq(chan[i].freq,fixedBlock);
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
          if (!isMuted[i]) drumState|=(1<<(melodicChans+4-i));
          updateDrums=true;
          chan[i].keyOn=false;
        } else if (chan[i].freqChanged) {
          immWrite(chanMap[i]+ADDR_FREQH,chan[i].freqH);
        }
      }
      chan[i].freqChanged=false;
    }
  }

  if (updateDrums) {
    immWrite(0xbd,(dam<<7)|(dvb<<6)|(properDrums<<5)|drumState);
  }

  // hard reset handling
  if (mustHardReset) {
    immWrite(0xfffffffe,128-hardResetElapsed);
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

int DivPlatformOPL::octave(int freq, int fixedBlock) {
  if (fixedBlock>0) {
    return 1<<(fixedBlock-1);
  }
  freq/=OPLL_C_NUM;
  if (freq==0) return 1;
  return 1<<bsr(freq);
}

int DivPlatformOPL::toFreq(int freq, int fixedBlock) {
  int block=0;
  if (fixedBlock>0) {
    block=fixedBlock-1;
  } else {
    block=freq/OPLL_C_NUM;
    if (block>0) block=bsr(block);
  }
  if (block>7) block=7;
  freq>>=block;
  if (freq>0x3ff) freq=0x3ff;
  return (block<<10)|freq;
}

void DivPlatformOPL::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (PCM_CHECK(ch)) {
    chan[ch].freqChanged=true;
    chan[ch].writeCtrl=true;
    return;
  }
  if (ch==adpcmChan) {
    immWrite(18,(isMuted[adpcmChan]?0:chan[adpcmChan].outVol));
    return;
  }
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

  if (isMuted[ch] && ch<=melodicChans) {
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
  if (PCM_CHECK(ch)) {
    return;
  }
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

        oldWrites[chanMap[ch]+ADDR_LR_FB_ALG]=-1;
        rWrite(chanMap[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&1)|(chan[ch].state.fb<<1)|((chan[ch].pan&15)<<4));
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

      if (isMuted[ch] && ch<=melodicChans) {
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
  if (oplType==3 && c.chan!=adpcmChan && c.chan<14 && (c.chan&1) && c.cmd!=DIV_CMD_GET_VOLMAX && c.cmd!=DIV_CMD_INSTRUMENT) {
    if (chan[c.chan-1].fourOp) return 0;
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (PCM_CHECK(c.chan)) { // OPL4 PCM
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_MULTIPCM);
        chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:127;
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        }
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PCM(c.value);
        }
        if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
          chan[c.chan].sample=-1;
        }
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
        if (chan[c.chan].insChanged) {
          if (ins->type==DIV_INS_MULTIPCM) {
            chan[c.chan].lfo=ins->multipcm.lfo;
            chan[c.chan].vib=ins->multipcm.vib;
            chan[c.chan].am=ins->multipcm.am;
            chan[c.chan].ar=ins->multipcm.ar;
            chan[c.chan].d1r=ins->multipcm.d1r;
            chan[c.chan].dl=ins->multipcm.dl;
            chan[c.chan].d2r=ins->multipcm.d2r;
            chan[c.chan].rc=ins->multipcm.rc;
            chan[c.chan].rr=ins->multipcm.rr;
            chan[c.chan].damp=ins->multipcm.damp;
            chan[c.chan].pseudoReverb=ins->multipcm.pseudoReverb;
            chan[c.chan].levelDirect=ins->multipcm.levelDirect;
            chan[c.chan].lfoReset=ins->multipcm.lfoReset;
          } else {
            chan[c.chan].lfo=0;
            chan[c.chan].vib=0;
            chan[c.chan].am=0;
            chan[c.chan].ar=15;
            chan[c.chan].d1r=15;
            chan[c.chan].dl=0;
            chan[c.chan].d2r=0;
            chan[c.chan].rc=15;
            chan[c.chan].rr=15;
            chan[c.chan].damp=false;
            chan[c.chan].pseudoReverb=false;
            chan[c.chan].levelDirect=true;
            chan[c.chan].lfoReset=false;
          }
          chan[c.chan].insChanged=false;
        }
        chan[c.chan].active=true;
        chan[c.chan].keyOn=true;
        chan[c.chan].macroInit(ins);
        if (!chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
        break;
      } else if (c.chan==adpcmChan) { // ADPCM
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
            immWrite(18,(isMuted[adpcmChan]?0:chan[adpcmChan].outVol));
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
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].sample=-1;
        chan[c.chan].macroInit(NULL);
      }
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
      if (PCM_CHECK(c.chan)) { // OPL4 PCM
        immWrite(PCM_ADDR_TL+PCM_REG(c.chan),((0x7f-chan[c.chan].outVol)<<1)|(chan[c.chan].levelDirect?1:0));
        break;
      }
      if (c.chan==adpcmChan) { // ADPCM-B
        immWrite(18,(isMuted[adpcmChan]?0:chan[adpcmChan].outVol));
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
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].ch=false;
        chan[c.chan].pan=8^MIN(parent->convertPanSplitToLinearLR(c.value,c.value2,15)+1,15);
        chan[c.chan].freqChanged=true;
        chan[c.chan].writeCtrl=true;
        break;
      }
      if (oplType!=3) break;
      if (c.chan==adpcmChan) break;
      chan[c.chan].pan&=~3;
      if (c.value==0 && c.value2==0 && ((chipType!=4) && compatPan)) {
        chan[c.chan].pan|=3;
      } else {
        chan[c.chan].pan|=(c.value>0)|((c.value2>0)<<1);
      }
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (isMuted[c.chan] && c.chan<=melodicChans) {
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
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].ch=true;
        chan[c.chan].freqChanged=true;
        chan[c.chan].writeCtrl=true;
        break;
      }
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
      if (isMuted[c.chan] && c.chan<=melodicChans) {
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
      if (PCM_CHECK(c.chan)) {
        int destFreq=NOTE_PCM(c.value2+chan[c.chan].sampleNoteDelta);
        bool return2=false;
        if (destFreq>chan[c.chan].baseFreq) {
          chan[c.chan].baseFreq+=c.value;
          if (chan[c.chan].baseFreq>=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        } else {
          chan[c.chan].baseFreq-=c.value;
          if (chan[c.chan].baseFreq<=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        }
        chan[c.chan].freqChanged=true;
        if (return2) {
          chan[c.chan].inPorta=false;
          return 2;
        }
        break;
      }
      int destFreq=(c.chan==adpcmChan)?(NOTE_ADPCMB(c.value2)):(NOTE_FREQUENCY(c.value2));
      int newFreq;
      bool return2=false;
      int mul=1;
      int fixedBlock=0;
      if (parent->song.linearPitch!=2) {
        fixedBlock=chan[c.chan].state.block;
        mul=octave(chan[c.chan].baseFreq,fixedBlock);
      }
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*mul;
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*mul;
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      if (!chan[c.chan].portaPause && parent->song.linearPitch!=2) {
        if (mul!=octave(newFreq,fixedBlock)) {
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
      // TODO: OPL4 PCM
      if (chan[c.chan].insChanged) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
        commitState(c.chan,ins);
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].baseFreq=(PCM_CHECK(c.chan))?NOTE_PCM(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0))):
          (c.chan==adpcmChan)?(NOTE_ADPCMB(c.value)):(NOTE_FREQUENCY(c.value));
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
      if (c.chan==adpcmChan) break;
      chan[c.chan].state.fb=c.value&7;
      int ops=(slots[3][c.chan]!=255 && chan[c.chan].state.ops==4 && oplType==3)?4:2;
      if (isMuted[c.chan] && c.chan<=melodicChans) {
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
      if (PCM_CHECK(c.chan)) break;
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
        if (chipType==4) {
          pcmChanOffs=totalChans;
          totalChans+=24;
        }
      } else {
        chanMap=properDrums?chanMapOPL2Drums:chanMapOPL2;
        melodicChans=properDrums?6:9;
        totalChans=properDrums?11:9;
      }
      break;
    }
    case DIV_CMD_FM_HARD_RESET:
      if (PCM_CHECK(c.chan)) break;
      if (c.chan==adpcmChan) break;
      chan[c.chan].hardReset=c.value;
      break;
    case DIV_CMD_MULTIPCM_MIX_FM:
      if (chipType==4) {
        fmMixL=CLAMP((c.value&0x70)>>4,0,7);
        fmMixR=CLAMP((c.value&0x7),0,7);
        immWrite(PCM_ADDR_MIX_FM,((7-fmMixR)<<3)|(7-fmMixL));
      }
      break;
    case DIV_CMD_MULTIPCM_MIX_PCM:
      if (chipType==4) {
        pcmMixL=CLAMP((c.value&0x70)>>4,0,7);
        pcmMixR=CLAMP((c.value&0x7),0,7);
        immWrite(PCM_ADDR_MIX_PCM,((7-pcmMixR)<<3)|(7-pcmMixL));
      }
      break;
    case DIV_CMD_MULTIPCM_LFO:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].lfo=c.value&7;
        rWrite(PCM_ADDR_LFO_VIB+PCM_REG(c.chan),(chan[c.chan].lfo<<3)|(chan[c.chan].vib));
      }
      break;
    case DIV_CMD_MULTIPCM_VIB:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].vib=c.value&7;
        rWrite(PCM_ADDR_LFO_VIB+PCM_REG(c.chan),(chan[c.chan].lfo<<3)|(chan[c.chan].vib));
      }
      break;
    case DIV_CMD_MULTIPCM_AM:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].am=c.value&7;
        rWrite(PCM_ADDR_AM+PCM_REG(c.chan),chan[c.chan].am);
      }
      break;
    case DIV_CMD_MULTIPCM_AR:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].ar=c.value&0xf;
        rWrite(PCM_ADDR_AR_D1R+PCM_REG(c.chan),(chan[c.chan].ar<<4)|(chan[c.chan].d1r));
      }
      break;
    case DIV_CMD_MULTIPCM_D1R:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].d1r=c.value&0xf;
        rWrite(PCM_ADDR_AR_D1R+PCM_REG(c.chan),(chan[c.chan].ar<<4)|(chan[c.chan].d1r));
      }
      break;
    case DIV_CMD_MULTIPCM_DL:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].dl=c.value&0xf;
        rWrite(PCM_ADDR_DL_D2R+PCM_REG(c.chan),(chan[c.chan].dl<<4)|(chan[c.chan].d2r));
      }
      break;
    case DIV_CMD_MULTIPCM_D2R:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].d2r=c.value&0xf;
        rWrite(PCM_ADDR_DL_D2R+PCM_REG(c.chan),(chan[c.chan].dl<<4)|(chan[c.chan].d2r));
      }
      break;
    case DIV_CMD_MULTIPCM_RC:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].rc=c.value&0xf;
        rWrite(PCM_ADDR_RC_RR+PCM_REG(c.chan),(chan[c.chan].rc<<4)|(chan[c.chan].rr));
      }
      break;
    case DIV_CMD_MULTIPCM_RR:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].rr=c.value&0xf;
        rWrite(PCM_ADDR_RC_RR+PCM_REG(c.chan),(chan[c.chan].rc<<4)|(chan[c.chan].rr));
      }
      break;
    case DIV_CMD_MULTIPCM_DAMP:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].damp=c.value&1;
        chan[c.chan].freqChanged=true;
        chan[c.chan].writeCtrl=true;
      }
      break;
    case DIV_CMD_MULTIPCM_PSEUDO_REVERB:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].pseudoReverb=c.value&1;
        chan[c.chan].freqChanged=true;
      }
      break;
    case DIV_CMD_MULTIPCM_LFO_RESET:
      if (PCM_CHECK(c.chan)) {
        chan[c.chan].lfoReset=c.value&1;
        chan[c.chan].freqChanged=true;
        chan[c.chan].writeCtrl=true;
      }
      break;
    case DIV_CMD_MULTIPCM_LEVEL_DIRECT:
      if (PCM_CHECK(c.chan)) {
        immWrite(PCM_ADDR_TL+PCM_REG(c.chan),((0x7f-chan[c.chan].outVol)<<1)|(chan[c.chan].levelDirect?1:0));
      }
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    case DIV_CMD_GET_VOLMAX:
      if (PCM_CHECK(c.chan)) return 127;
      if (c.chan==adpcmChan) return 255;
      if (pretendYMU) return 127;
      return 63;
      break;
    case DIV_CMD_PRE_PORTA:
      if (PCM_CHECK(c.chan) && chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_MULTIPCM));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) {
        chan[c.chan].baseFreq=(PCM_CHECK(c.chan))?NOTE_PCM(chan[c.chan].note):
          ((c.chan==adpcmChan)?(NOTE_ADPCMB(chan[c.chan].note)):(NOTE_FREQUENCY(chan[c.chan].note)));
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
    if (chipType==4) {
      pcmChanOffs=totalChans;
      totalChans+=24;
    }
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
    /*
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
    */

    if (isMuted[i] && i<=melodicChans) {
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
  for (int i=0; i<768; i++) {
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
  if (PCM_CHECK(ch)) {
    return parent->convertPanLinearToSplit(8^chan[ch].pan,8,15);
  }
  /*if (chan[ch&(~1)].fourOp) {
    if (ch&1) {
      return ((chan[ch-1].pan&2)<<7)|(chan[ch-1].pan&1);
    } else {
      return ((chan[ch+1].pan&2)<<7)|(chan[ch+1].pan&1);
    }
  }*/
  return ((chan[ch].pan&1)<<8)|((chan[ch].pan&2)>>1);
}

void DivPlatformOPL::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (oplType==3 && ch<12 && !(ch&1)) {
    if (chan[ch].fourOp) {
      ret.push_back(DivChannelPair(_("4OP"),ch+1));
    }
  }
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

int DivPlatformOPL::mapVelocity(int ch, float vel) {
  if (PCM_CHECK(ch)) { // TODO: correct?
  // -0.375dB per step
  // -6: 64: 16
  // -12: 32: 32
  // -18: 16: 48
  // -24: 8: 64
  // -30: 4: 80
  // -36: 2: 96
  // -42: 1: 112
    if (vel==0) return 0;
    if (vel>=1.0) return 127;
    return CLAMP(round(128.0-(112.0-log2(vel*127.0)*16.0)),0,127);
  }
  if (ch==adpcmChan) return vel*255.0;
  // -0.75dB per step
  // -6: 64: 8
  // -12: 32: 16
  // -18: 16: 24
  // -24: 8: 32
  // -30: 4: 40
  // -36: 2: 48
  // -42: 1: 56
  if (vel==0) return 0;
  if (vel>=1.0) return 63;
  return CLAMP(round(64.0-(56.0-log2(vel*127.0)*8.0)),0,63);
}

float DivPlatformOPL::getGain(int ch, int vol) {
  if (vol==0) return 0;
  if (PCM_CHECK(ch)) return 1.0/pow(10.0,(float)(127-vol)*0.375/20.0);
  if (ch==adpcmChan) return (float)vol/255.0;
  return 1.0/pow(10.0,(float)(63-vol)*0.75/20.0);
}

unsigned char* DivPlatformOPL::getRegisterPool() {
  return regPool;
}

int DivPlatformOPL::getRegisterPoolSize() {
  return (chipType==4)?768:((oplType<3)?256:512);
}

void DivPlatformOPL::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,768);

  downsamplerStep=0;
  oldOpMask=0;
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
      case 4:
        fm_ymfm4->reset();
        break;
    }
  } else {
    if (downsample) {
      OPL3_Reset(&fm,downsampledRate);
    } else {
      OPL3_Reset(&fm,rate);
    }
  }
  pcm.reset();

  properDrums=properDrumsSys;
  if (oplType==3) {
    chanMap=properDrums?chanMapOPL3Drums:chanMapOPL3;
    outChanMap=outChanMapOPL3;
    melodicChans=properDrums?15:18;
    totalChans=properDrums?20:18;
    if (chipType==4) {
      pcmChanOffs=totalChans;
      totalChans+=24;
    }
  } else {
    chanMap=properDrums?chanMapOPL2Drums:chanMapOPL2;
    outChanMap=outChanMapOPL2;
    melodicChans=properDrums?6:9;
    totalChans=properDrums?11:9;
  }

  for (int i=0; i<totalChans; i++) {
    chan[i]=DivPlatformOPL::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=(PCM_CHECK(i))?0x7f:0x3f;
    chan[i].outVol=(PCM_CHECK(i))?0x7f:0x3f;
    chan[i].pan=(PCM_CHECK(i))?0:3;
  }

  if (adpcmChan>=0) {
    chan[adpcmChan]=DivPlatformOPL::Channel();
    chan[adpcmChan].std.setEngine(parent);
    chan[adpcmChan].vol=0xff;
    chan[adpcmChan].outVol=0xff;

    adpcmB->reset();

    // volume
    immWrite(18,(isMuted[adpcmChan]?0:0xff));
    // ADPCM limit
    immWrite(20,0xff);
    immWrite(19,0xff);
  }
  
  if (oplType<3) for (int i=0; i<melodicChans; i++) {
    fm.channel[outChanMap[i]].muted=isMuted[i];
  }

  for (int i=0; i<768; i++) {
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
    if (chipType==4) {
      immWrite(0x105,3);
      // Reset wavetable header
      immWrite(0x202,(ramSize<=0x200000)?0x10:0x00);
      // initialize mixer volume
      fmMixL=7;
      fmMixR=7;
      pcmMixL=7;
      pcmMixR=7;
      immWrite(PCM_ADDR_MIX_FM,((7-fmMixR)<<3)|(7-fmMixL));
      immWrite(PCM_ADDR_MIX_PCM,((7-pcmMixR)<<3)|(7-pcmMixL));
    } else {
      immWrite(0x105,1);
    }
  }

  if (dumpWrites) {
    addWrite(0xffffffff,0);
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

bool DivPlatformOPL::getLegacyAlwaysSetVolume() {
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
        pcmChanOffs=totalChans;
        totalChans+=24;
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
          if (emuCore==2) {
            rate=chipClock/684;
          } else {
            rate=chipClock/768;
          }
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
          chipClock=COLOR_NTSC*8.0;
          break;
        case 0x02:
          chipClock=COLOR_PAL*32.0/5.0;
          break;
        default:
          chipClock=33868800.0;
          break;
      }
      switch (flags.getInt("ramSize",0)) {
        case 0x01: // 2MB (512KB 512KB 512KB 512KB)
          ramSize=0x200000;
          break;
        case 0x02: // 1MB (512KB 512KB)
          ramSize=0x100000;
          break;
        case 0x03: // 640KB (512KB 128KB)
          ramSize=0xa0000;
          break;
        case 0x04: // 512KB
          ramSize=0x80000;
          break;
        case 0x05: // 256KB (128KB 128KB)
          ramSize=0x40000;
          break;
        case 0x06: // 128KB
          ramSize=0x20000;
          break;
        default:
          ramSize=0x400000;
          break;
      }
      CHECK_CUSTOM_CLOCK;
      pcm.setClockFrequency(chipClock);
      rate=chipClock/768;
      chipRateBase=chipClock/684;
      immWrite(0x202,(ramSize<=0x200000)?0x10:0x00);
      break;
    case 759:
      rate=48000;
      chipRateBase=rate;
      chipClock=rate*288;
      break;
  }
  compatPan=flags.getBool("compatPan",false);
  compatYPitch=flags.getBool("compatYPitch",false);

  for (int i=0; i<44; i++) {
    oscBuf[i]->setRate(rate);
  }
}

const void* DivPlatformOPL::getSampleMem(int index) {
  return (index==0 && pcmChanOffs>=0)?pcmMem:
          (index==0 && adpcmChan>=0)?adpcmBMem:NULL;
}

size_t DivPlatformOPL::getSampleMemCapacity(int index) {
  return (index==0 && pcmChanOffs>=0)?
          ((ramSize<=0x200000)?0x200000+ramSize:ramSize):
          ((index==0 && adpcmChan>=0)?262144:0);
}

size_t DivPlatformOPL::getSampleMemUsage(int index) {
  return (index==0 && pcmChanOffs>=0)?pcmMemLen:
          (index==0 && adpcmChan>=0)?adpcmBMemLen:0;
}

bool DivPlatformOPL::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformOPL::getMemCompo(int index) {
  if ((adpcmChan<0) && (pcmChanOffs<0)) return NULL;
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformOPL::renderSamples(int sysID) {
  if (adpcmChan<0 && pcmChanOffs<0) return;
  if (adpcmChan>=0 && adpcmBMem!=NULL) {
    memset(adpcmBMem,0,262144);
  }
  if (pcmChanOffs>=0 && pcmMem!=NULL) {
    memset(pcmMem,0,4194304);
  }
  memset(sampleOffPCM,0,256*sizeof(unsigned int));
  memset(sampleOffB,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample Memory";

  if (pcmChanOffs>=0) { // OPL4 PCM
    size_t memPos=((ramSize<=0x200000)?0x200600:0x1800);
    const int maxSample=(ramSize<=0x200000)?127:511;
    int sampleCount=parent->song.sampleLen;
    if (sampleCount>maxSample) sampleCount=maxSample;
    for (int i=0; i<sampleCount; i++) {
      DivSample* s=parent->song.sample[i];
      if (!s->renderOn[0][sysID]) {
        sampleOffPCM[i]=0;
        continue;
      }

      int length;
      int sampleLength;
      unsigned char* src=(unsigned char*)s->getCurBuf();
      switch (s->depth) {
        case DIV_SAMPLE_DEPTH_8BIT:
          sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
          length=MIN(65535,sampleLength+1);
          break;
        case DIV_SAMPLE_DEPTH_12BIT:
          sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_12BIT);
          length=MIN(98303,sampleLength+3);
          break;
        case DIV_SAMPLE_DEPTH_16BIT:
          sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_16BIT);
          length=MIN(131070,sampleLength+2);
          break;
        default:
          sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
          length=MIN(65535,sampleLength+1);
          src=(unsigned char*)s->data8;
          break;
      }
      if (sampleLength<1) length=0;
      int actualLength=MIN((int)(getSampleMemCapacity(0)-memPos),length);
      if (actualLength>0) {
        if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
          for (int i=0, j=0; i<actualLength; i++, j++) {
            if (j>=sampleLength) j=sampleLength-2;
#ifdef TA_BIG_ENDIAN
            pcmMem[memPos+i]=src[j];
#else
            pcmMem[memPos+i]=src[j^1];
#endif
          }
        } else {
          for (int i=0, j=0; i<actualLength; i++, j++) {
            if (j>=sampleLength && s->depth!=DIV_SAMPLE_DEPTH_12BIT) j=sampleLength-1;
            pcmMem[memPos+i]=src[j];
          }
        }
        sampleOffPCM[i]=memPos;
        memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+length));
        memPos+=length;
      }
      if (actualLength<length) {
        logW("out of OPL4 PCM memory for sample %d!",i);
        break;
      }
      sampleLoaded[i]=true;
    }
    pcmMemLen=memPos+256;

    // instrument table
    for (int i=0; i<sampleCount; i++) {
      DivSample* s=parent->song.sample[i];
      unsigned int insAddr=(i*12)+((ramSize<=0x200000)?0x200000:0);
      unsigned char bitDepth;
      int endPos=CLAMP(s->isLoopable()?s->loopEnd:(s->samples+1),1,0x10000);
      int loop=s->isLoopable()?CLAMP(s->loopStart,0,endPos-2):(endPos-2);
      switch (s->depth) {
        case DIV_SAMPLE_DEPTH_8BIT:
          bitDepth=0;
          break;
        case DIV_SAMPLE_DEPTH_12BIT:
          bitDepth=1;
          if (!s->isLoopable()) {
            endPos++;
            loop++;
          }
          break;
        case DIV_SAMPLE_DEPTH_16BIT:
          bitDepth=2;
          break;
        default:
          bitDepth=0;
          break;
      }
      pcmMem[insAddr]=(bitDepth<<6)|((sampleOffPCM[i]>>16)&0x3f);
      pcmMem[1+insAddr]=(sampleOffPCM[i]>>8)&0xff;
      pcmMem[2+insAddr]=(sampleOffPCM[i])&0xff;
      pcmMem[3+insAddr]=(loop>>8)&0xff;
      pcmMem[4+insAddr]=(loop)&0xff;
      pcmMem[5+insAddr]=((~(endPos-1))>>8)&0xff;
      pcmMem[6+insAddr]=(~(endPos-1))&0xff;
      // on MultiPCM this consists of instrument params, but on OPL4 this is not used
      pcmMem[7+insAddr]=0; // LFO, VIB
      pcmMem[8+insAddr]=(0xf << 4) | (0xf << 0); // AR, D1R
      pcmMem[9+insAddr]=0; // DL, D2R
      pcmMem[10+insAddr]=(0xf << 4) | (0xf << 0); // RC, RR
      pcmMem[11+insAddr]=0; // AM
    }
    if (ramSize<=0x200000) {
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_RESERVED,"ROM data",0,0,0x200000));
    }

    memCompo.used=pcmMemLen;
  } else if (adpcmChan>=0) { // ADPCM
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
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
      memPos+=paddedLen;
    }
    adpcmBMemLen=memPos+256;

    memCompo.used=adpcmBMemLen;
  }
  memCompo.capacity=getSampleMemCapacity(0);
}

int DivPlatformOPL::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<44; i++) {
    isMuted[i]=false;
  }
  for (int i=0; i<44; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  fm_ymfm1=NULL;
  fm_ymfm2=NULL;
  fm_ymfm8950=NULL;
  fm_ymfm3=NULL;
  fm_ymfm4=NULL;

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
      case 4:
        fm_ymfm4=new ymfm::ymf278b(iface);
        break;
    }
  }

  setFlags(flags);

  if (adpcmChan>=0) {
    adpcmBMem=new unsigned char[262144];
    adpcmBMemLen=0;
    iface.adpcmBMem=adpcmBMem;
    iface.sampleBank=0;
    adpcmB=new ymfm::adpcm_b_engine(iface,2);
  }

  if (pcmChanOffs>=0) {
    pcmMem=new unsigned char[4194304];
    pcmMemLen=0;
    iface.pcmMem=pcmMem;
    iface.sampleBank=0;
    pcmMemory.memory=pcmMem;
  }

  reset();
  return totalChans;
}

void DivPlatformOPL::quit() {
  for (int i=0; i<44; i++) {
    delete oscBuf[i];
  }
  if (adpcmChan>=0) {
    delete adpcmB;
    delete[] adpcmBMem;
  }
  if (pcmChanOffs>=0) {
    delete[] pcmMem;
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
  if (fm_ymfm4!=NULL) {
    delete fm_ymfm4;
    fm_ymfm4=NULL;
  }
}

DivPlatformOPL::~DivPlatformOPL() {
}
