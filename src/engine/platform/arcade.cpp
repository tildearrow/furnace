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

#include "arcade.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "fmshared_OPM.h"

static unsigned short chanOffs[8]={
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};
static unsigned short opOffs[4]={
  0x00, 0x08, 0x10, 0x18
};
static bool isOutput[8][4]={
  // 1     3     2    4
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,false,true ,true},
  {false,true ,true ,true},
  {false,true ,true ,true},
  {true ,true ,true ,true},
};
static unsigned char dtTable[8]={
  7,6,5,0,1,2,3,0
};

static int orderedOps[4]={
  0,2,1,3
};

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

const char* regCheatSheetOPM[]={
  "Test", "00",
  "NoteCtl", "08",
  "NoiseCtl", "0F",
  "ClockA1", "10",
  "ClockA2", "11",
  "ClockB", "12",
  "Control", "14",
  "LFOFreq", "18",
  "AMD_PMD", "19",
  "LFOWave", "1B",
  "L_R_FB_ALG", "20",
  "KC", "28",
  "KF", "30",
  "PMS_AMS", "38",
  "DT_MULT", "40",
  "TL", "60",
  "KS_AR", "80",
  "AM_DR", "A0",
  "DT2_SR", "C0",
  "SL_RR", "E0",
  NULL
};

const char** DivPlatformArcade::getRegisterSheet() {
  return regCheatSheetOPM;
}

const char* DivPlatformArcade::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Set noise frequency (xx: value; 0 disables noise)";
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
      return "17xx: Set LFO speed";
      break;
    case 0x18:
      return "18xx: Set LFO waveform (0 saw, 1 square, 2 triangle, 3 noise)";
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
    case 0x1e:
      return "1Exx: Set AM depth (0 to 7F)";
      break;
    case 0x1f:
      return "1Fxx: Set PM depth (0 to 7F)";
      break;
    case 0x20:
      return "20xx: Set PCM frequency";
      break; 
  }
  return NULL;
}

void DivPlatformArcade::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  static int o[2];

  for (size_t h=start; h<start+len; h++) {
    if (!writes.empty() && !fm.write_busy) {
      QueuedWrite& w=writes.front();
      if (w.addrOrVal) {
        OPM_Write(&fm,1,w.val);
        //printf("write: %x = %.2x\n",w.addr,w.val);
        writes.pop();
      } else {
        OPM_Write(&fm,0,w.addr);
        w.addrOrVal=true;
      }
    }
    
    OPM_Clock(&fm,NULL,NULL,NULL,NULL);
    OPM_Clock(&fm,NULL,NULL,NULL,NULL);
    OPM_Clock(&fm,NULL,NULL,NULL,NULL);
    OPM_Clock(&fm,o,NULL,NULL,NULL);

    pcmCycles+=31250;
    if (pcmCycles>=rate) {
      pcmCycles-=rate;

      // do a PCM cycle
      pcmL=0; pcmR=0;
      for (int i=8; i<13; i++) {
        if (chan[i].pcm.sample>=0) {
          DivSample* s=parent->song.sample[chan[i].pcm.sample];
          if (s->rendLength<=0) {
            chan[i].pcm.sample=-1;
            continue;
          }
          if (!isMuted[i]) {
            if (s->depth==8) {
              pcmL+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolL);
              pcmR+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolR);
            } else {
              pcmL+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolL)>>8;
              pcmR+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolR)>>8;
            }
          }
          chan[i].pcm.pos+=chan[i].pcm.freq;
          if (chan[i].pcm.pos>=(s->rendLength<<8)) {
            if (s->loopStart>=0 && s->loopStart<=(int)s->rendLength) {
              chan[i].pcm.pos=s->loopStart<<8;
            } else {
              chan[i].pcm.sample=-1;
            }
          }
        }
      }
    }

    o[0]+=pcmL;
    o[1]+=pcmR;
    
    if (o[0]<-32768) o[0]=-32768;
    if (o[0]>32767) o[0]=32767;

    if (o[1]<-32768) o[1]=-32768;
    if (o[1]>32767) o[1]=32767;
  
    bufL[h]=o[0];
    bufR[h]=o[1];
  }
}

void DivPlatformArcade::acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      if (--delay<1) {
        QueuedWrite& w=writes.front();
        fm_ymfm->write(0x0+((w.addr>>8)<<1),w.addr);
        fm_ymfm->write(0x1+((w.addr>>8)<<1),w.val);
        writes.pop();
        delay=1;
      }
    }
    
    fm_ymfm->generate(&out_ymfm);

    pcmCycles+=31250;
    if (pcmCycles>=rate) {
      pcmCycles-=rate;

      // do a PCM cycle
      pcmL=0; pcmR=0;
      for (int i=8; i<13; i++) {
        if (chan[i].pcm.sample>=0 && chan[i].pcm.sample<parent->song.sampleLen) {
          DivSample* s=parent->song.sample[chan[i].pcm.sample];
          if (s->rendLength<=0) {
            chan[i].pcm.sample=-1;
            continue;
          }
          if (!isMuted[i]) {
            if (s->depth==8) {
              pcmL+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolL);
              pcmR+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolR);
            } else {
              pcmL+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolL)>>8;
              pcmR+=(s->rendData[chan[i].pcm.pos>>8]*chan[i].chVolR)>>8;
            }
          }
          chan[i].pcm.pos+=chan[i].pcm.freq;
          if (chan[i].pcm.pos>=(s->rendLength<<8)) {
            if (s->loopStart>=0 && s->loopStart<=(int)s->rendLength) {
              chan[i].pcm.pos=s->loopStart<<8;
            } else {
              chan[i].pcm.sample=-1;
            }
          }
        }
      }
    }

    os[0]=out_ymfm.data[0]+pcmL;
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=out_ymfm.data[1]+pcmR;
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformArcade::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (useYMFM) {
    acquire_ymfm(bufL,bufR,start,len);
  } else {
    acquire_nuked(bufL,bufR,start,len);
  }
}

unsigned char noteMap[12]={
  0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14
};

int hScale(int note) {
  return ((note/12)<<4)+(noteMap[note%12]);
}

void DivPlatformArcade::tick() {
  for (int i=0; i<8; i++) {
    chan[i].std.next();

    if (chan[i].std.hadVol) {
      chan[i].outVol=(chan[i].vol*MIN(127,chan[i].std.vol))/127;
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
    }

    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=(chan[i].std.arp<<6)+baseFreqOff;
        } else {
          chan[i].baseFreq=((chan[i].note+(signed char)chan[i].std.arp)<<6)+baseFreqOff;
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=(chan[i].note<<6)+baseFreqOff;
        chan[i].freqChanged=true;
      }
    }

    if (chan[i].std.hadDuty) {
      if (chan[i].std.duty>0) {
        rWrite(0x0f,0x80|(0x20-chan[i].std.duty));
      } else {
        rWrite(0x0f,0);
      }
    }

    if (chan[i].std.hadWave) {
      rWrite(0x1b,chan[i].std.wave&3);
    }

    if (chan[i].std.hadEx1) {
      amDepth=chan[i].std.ex1;
      immWrite(0x19,amDepth);
    }

    if (chan[i].std.hadEx2) {
      pmDepth=chan[i].std.ex2;
      immWrite(0x19,0x80|pmDepth);
    }

    if (chan[i].std.hadEx3) {
      immWrite(0x18,chan[i].std.ex3);
    }

    if (chan[i].std.hadAlg) {
      chan[i].state.alg=chan[i].std.alg;
      if (isMuted[i]) {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      } else {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
      }
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
      if (isMuted[i]) {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      } else {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
      }
    }
    if (chan[i].std.hadFms) {
      chan[i].state.fms=chan[i].std.fms;
      rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    }
    if (chan[i].std.hadAms) {
      chan[i].state.ams=chan[i].std.ams;
      rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
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
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
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
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
      if (m.hadDt2) {
        op.dt2=m.dt2;
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
    }
    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(0x08,i);
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+(chan[i].pitch>>1)-64;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>=(95<<6)) chan[i].freq=(95<<6)-1;
      immWrite(i+0x28,hScale(chan[i].freq>>6));
      immWrite(i+0x30,chan[i].freq<<2);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      immWrite(0x08,0x78|i);
      chan[i].keyOn=false;
    }
  }

  for (int i=8; i<13; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+(chan[i].pitch>>1)-64;
      if (chan[i].furnacePCM) {
        double off=1.0;
        if (chan[i].pcm.sample>=0 && chan[i].pcm.sample<parent->song.sampleLen) {
          DivSample* s=parent->song.sample[chan[i].pcm.sample];
          off=(double)s->centerRate/8363.0;
        }
        chan[i].pcm.freq=MIN(255,((off*parent->song.tuning*pow(2.0,double(chan[i].freq+256)/(64.0*12.0)))*255)/31250);
        if (dumpWrites && i>=8) {
          addWrite(0x10007+((i-8)<<3),chan[i].pcm.freq);
        }
      }
      chan[i].freqChanged=false;
    }
  }
}

void DivPlatformArcade::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (ch<8) {
    if (isMuted[ch]) {
      rWrite(chanOffs[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3));
    } else {
      rWrite(chanOffs[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3)|((chan[ch].chVolL&1)<<6)|((chan[ch].chVolR&1)<<7));
    }
  }
}

int DivPlatformArcade::dispatch(DivCommand c) {
  int pcmChan=c.chan-8;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.chan>7) {
        if (skipRegisterWrites) break;
        if (ins->type==DIV_INS_AMIGA) {
          chan[c.chan].pcm.sample=ins->amiga.initSample;
          if (chan[c.chan].pcm.sample<0 || chan[c.chan].pcm.sample>=parent->song.sampleLen) {
            chan[c.chan].pcm.sample=-1;
            if (dumpWrites) {
              addWrite(0x10086+(pcmChan<<3),3);
            }
            break;
          }
          chan[c.chan].pcm.pos=0;
          chan[c.chan].baseFreq=(c.value<<6);
          chan[c.chan].freqChanged=true;
          chan[c.chan].furnacePCM=true;
          if (dumpWrites) { // Sega PCM writes
            DivSample* s=parent->song.sample[chan[c.chan].pcm.sample];
            addWrite(0x10086+(pcmChan<<3),3+((s->rendOffP>>16)<<3));
            addWrite(0x10084+(pcmChan<<3),(s->rendOffP)&0xff);
            addWrite(0x10085+(pcmChan<<3),(s->rendOffP>>8)&0xff);
            addWrite(0x10006+(pcmChan<<3),MIN(255,((s->rendOffP&0xffff)+s->rendLength-1)>>8));
            if (s->loopStart<0 || s->loopStart>=(int)s->rendLength) {
              addWrite(0x10086+(pcmChan<<3),2+((s->rendOffP>>16)<<3));
            } else {
              int loopPos=(s->rendOffP&0xffff)+s->loopStart+s->loopOffP;
              addWrite(0x10004+(pcmChan<<3),loopPos&0xff);
              addWrite(0x10005+(pcmChan<<3),(loopPos>>8)&0xff);
              addWrite(0x10086+(pcmChan<<3),((s->rendOffP>>16)<<3));
            }
          }
        } else {
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          chan[c.chan].pcm.sample=12*sampleBank+chan[c.chan].note%12;
          if (chan[c.chan].pcm.sample>=parent->song.sampleLen) {
            chan[c.chan].pcm.sample=-1;
            if (dumpWrites) {
              addWrite(0x10086+(pcmChan<<3),3);
            }
            break;
          }
          chan[c.chan].pcm.pos=0;
          chan[c.chan].pcm.freq=MIN(255,(parent->song.sample[chan[c.chan].pcm.sample]->rate*255)/31250);
          chan[c.chan].furnacePCM=false;
          if (dumpWrites) { // Sega PCM writes
            DivSample* s=parent->song.sample[chan[c.chan].pcm.sample];
            addWrite(0x10086+(pcmChan<<3),3+((s->rendOffP>>16)<<3));
            addWrite(0x10084+(pcmChan<<3),(s->rendOffP)&0xff);
            addWrite(0x10085+(pcmChan<<3),(s->rendOffP>>8)&0xff);
            addWrite(0x10006+(pcmChan<<3),MIN(255,((s->rendOffP&0xffff)+s->rendLength-1)>>8));
            if (s->loopStart<0 || s->loopStart>=(int)s->rendLength) {
              addWrite(0x10086+(pcmChan<<3),2+((s->rendOffP>>16)<<3));
            } else {
              int loopPos=(s->rendOffP&0xffff)+s->loopStart+s->loopOffP;
              addWrite(0x10004+(pcmChan<<3),loopPos&0xff);
              addWrite(0x10005+(pcmChan<<3),(loopPos>>8)&0xff);
              addWrite(0x10086+(pcmChan<<3),((s->rendOffP>>16)<<3));
            }
            addWrite(0x10007+(pcmChan<<3),chan[c.chan].pcm.freq);
          }
        }
        break;
      }

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }

      chan[c.chan].std.init(ins);
      if (!chan[c.chan].std.willVol) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          if (!chan[c.chan].active || chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
          }
        } else {
          if (chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      }
      if (chan[c.chan].insChanged) {
        if (isMuted[c.chan]) {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        } else {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
        }
        rWrite(chanOffs[c.chan]+ADDR_FMS_AMS,((chan[c.chan].state.fms&7)<<4)|(chan[c.chan].state.ams&3));
      }
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=(c.value<<6)+baseFreqOff;
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan>7) {
        chan[c.chan].pcm.sample=-1;
        if (dumpWrites) {
          addWrite(0x10086+(pcmChan<<3),3);
        }
      }
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
      if (c.chan>7) {
        chan[c.chan].chVolL=c.value;
        chan[c.chan].chVolR=c.value;
        if (dumpWrites) {
          addWrite(0x10002+(pcmChan<<3),chan[c.chan].chVolL);
          addWrite(0x10003+(pcmChan<<3),chan[c.chan].chVolR);
        }
        break;
      }
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
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
    case DIV_CMD_PANNING: {
      // TODO
      if (c.chan>7) {
        chan[c.chan].chVolL=(c.value>>4)|(((c.value>>4)>>1)<<4);
        chan[c.chan].chVolR=(c.value&15)|(((c.value&15)>>1)<<4);
        if (dumpWrites) {
          addWrite(0x10002+(pcmChan<<3),chan[c.chan].chVolL);
          addWrite(0x10003+(pcmChan<<3),chan[c.chan].chVolR);
        }
      } else {
        chan[c.chan].chVolL=((c.value>>4)==1);
        chan[c.chan].chVolR=((c.value&15)==1);
        if (isMuted[c.chan]) {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        } else {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
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
      int destFreq=(c.value2<<6)+baseFreqOff;
      int newFreq;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value;
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value;
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].baseFreq=newFreq;
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=(c.value<<6)+baseFreqOff;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      if (c.chan>7) break;
      rWrite(0x18,c.value);
      break;
    }
    case DIV_CMD_FM_LFO_WAVE: {
      if (c.chan>7) break;
      rWrite(0x1b,c.value&3);
      break;
    }
    case DIV_CMD_FM_FB: {
      if (c.chan>7) break;
      chan[c.chan].state.fb=c.value&7;
      if (isMuted[c.chan]) {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      } else {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
      }
      break;
    }
    case DIV_CMD_FM_MULT: {
      if (c.chan>7) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>7) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOutput[chan[c.chan].state.alg][c.value]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.chan>7) break;
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
      break;
    }
    case DIV_CMD_FM_AM_DEPTH: {
      amDepth=c.value;
      immWrite(0x19,amDepth);
      break;
    }
    case DIV_CMD_FM_PM_DEPTH: {
      pmDepth=c.value;
      immWrite(0x19,0x80|pmDepth);
      break;
    }
    case DIV_CMD_STD_NOISE_FREQ: {
      if (c.chan!=7) break;
      if (c.value) {
        if (c.value>0x1f) {
          rWrite(0x0f,0x80);
        } else {
          rWrite(0x0f,0x80|(0x1f-c.value));
        }
      } else {
        rWrite(0x0f,0);
      }
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    case DIV_CMD_SAMPLE_FREQ:
      chan[c.chan].pcm.freq=c.value;
      if (dumpWrites) {
        addWrite(0x10007+(pcmChan<<3),chan[c.chan].pcm.freq);
      }
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformArcade::forceIns() {
  for (int i=0; i<8; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator op=chan[i].state.op[j];
      if (isOutput[chan[i].state.alg][j]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
    }
    if (isMuted[i]) {
      rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    } else {
      rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
    }
    rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  for (int i=8; i<13; i++) {
    chan[i].insChanged=true;
  }
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
}

void DivPlatformArcade::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void* DivPlatformArcade::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformArcade::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformArcade::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformArcade::reset() {
  while (!writes.empty()) writes.pop();
  if (useYMFM) {
    fm_ymfm->reset();
  } else {
    memset(&fm,0,sizeof(opm_t));
    OPM_Reset(&fm);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<13; i++) {
    chan[i]=DivPlatformArcade::Channel();
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }

  for (int i=0; i<256; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  pcmCycles=0;
  pcmL=0;
  pcmR=0;
  sampleBank=0;
  delay=0;
  amDepth=0x7f;
  pmDepth=0x7f;

  //rWrite(0x18,0x10);
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
  //rWrite(0x1b,0x00);
  if (dumpWrites) {
    for (int i=0; i<5; i++) {
      addWrite(0x10086+(i<<3),3);
      addWrite(0x10002+(i<<3),0x7f);
      addWrite(0x10003+(i<<3),0x7f);
    }
  }

  extMode=false;
}

void DivPlatformArcade::setFlags(unsigned int flags) {
  if (flags==2) {
    chipClock=4000000.0;
    baseFreqOff=-122;
  } else if (flags==1) {
    chipClock=COLOR_PAL*4.0/5.0;
    baseFreqOff=12;
  } else {
    chipClock=COLOR_NTSC;
    baseFreqOff=0;
  }
  if (useYMFM) {
    rate=chipClock/64;
  } else {
    rate=chipClock/8;
  }
}

bool DivPlatformArcade::isStereo() {
  return true;
}

void DivPlatformArcade::setYMFM(bool use) {
  useYMFM=use;
}

int DivPlatformArcade::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<13; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  if (useYMFM) fm_ymfm=new ymfm::ym2151(iface);
  reset();

  return 13;
}

void DivPlatformArcade::quit() {
  if (useYMFM) {
    delete fm_ymfm;
  }
}

DivPlatformArcade::~DivPlatformArcade() {
}
