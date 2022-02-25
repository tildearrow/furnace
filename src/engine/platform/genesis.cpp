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

#include "genesis.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "genesisshared.h"

static unsigned char konOffs[6]={
  0, 1, 2, 4, 5, 6
};

#define CHIP_FREQBASE 9440540

const char* DivPlatformGenesis::getEffectName(unsigned char effect) {
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
    case 0x20:
      return "20xy: Set PSG noise mode (x: preset freq/ch3 freq; y: thin pulse/noise)";
      break; 
  }
  return NULL;
}

void DivPlatformGenesis::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  static short o[2];
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    if (dacMode && dacSample!=-1) {
      dacPeriod-=6;
      if (dacPeriod<1) {
        DivSample* s=parent->getSample(dacSample);
        if (s->samples>0) {
          if (!isMuted[5]) {
            immWrite(0x2a,(unsigned char)s->data8[dacPos]+0x80);
          }
          if (++dacPos>=s->samples) {
            if (s->loopStart>=0 && s->loopStart<=(int)s->samples) {
              dacPos=s->loopStart;
            } else {
              dacSample=-1;
            }
          }
          dacPeriod+=MAX(40,dacRate);
        } else {
          dacSample=-1;
        }
      }
    }
  
    os[0]=0; os[1]=0;
    for (int i=0; i<6; i++) {
      if (!writes.empty() && --delay<0) {
        delay=0;
        QueuedWrite& w=writes.front();
        if (w.addrOrVal) {
          OPN2_Write(&fm,0x1+((w.addr>>8)<<1),w.val);
          //printf("write: %x = %.2x\n",w.addr,w.val);
          lastBusy=0;
          regPool[w.addr&0x1ff]=w.val;
          writes.pop();
        } else {
          lastBusy++;
          if (fm.write_busy==0) {
            //printf("busycounter: %d\n",lastBusy);
            OPN2_Write(&fm,0x0+((w.addr>>8)<<1),w.addr);
            w.addrOrVal=true;
          }
        }
      }
      
      OPN2_Clock(&fm,o); os[0]+=o[0]; os[1]+=o[1];
      //OPN2_Write(&fm,0,0);
      }
    
    psgClocks+=psg.rate;
    while (psgClocks>=rate) {
      psgOut=(psg.acquireOne()*3)>>3;
      psgClocks-=rate;
    }

    os[0]=(os[0]<<5)+psgOut;
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=(os[1]<<5)+psgOut;
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformGenesis::acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    if (dacMode && dacSample!=-1) {
      dacPeriod-=24;
      if (dacPeriod<1) {
        DivSample* s=parent->getSample(dacSample);
        if (s->samples>0) {
          if (!isMuted[5]) {
            immWrite(0x2a,(unsigned char)s->data8[dacPos]+0x80);
          }
          if (++dacPos>=s->samples) {
            if (s->loopStart>=0 && s->loopStart<=(int)s->samples) {
              dacPos=s->loopStart;
            } else {
              dacSample=-1;
            }
          }
          dacPeriod+=MAX(40,dacRate);
        } else {
          dacSample=-1;
        }
      }
    }
  
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      QueuedWrite& w=writes.front();
      fm_ymfm->write(0x0+((w.addr>>8)<<1),w.addr);
      fm_ymfm->write(0x1+((w.addr>>8)<<1),w.val);
      regPool[w.addr&0x1ff]=w.val;
      writes.pop();
      lastBusy=1;
    }
    
    if (ladder) {
      fm_ymfm->generate(&out_ymfm);
    } else {
      ((ymfm::ym3438*)fm_ymfm)->generate(&out_ymfm);
    }
    os[0]=out_ymfm.data[0];
    os[1]=out_ymfm.data[1];
    //OPN2_Write(&fm,0,0);
    
    psgClocks+=psg.rate;
    while (psgClocks>=rate) {
      psgOut=(psg.acquireOne()*3)>>3;
      psgClocks-=rate;
    }

    os[0]=os[0]+psgOut;
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=os[1]+psgOut;
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformGenesis::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (useYMFM) {
    acquire_ymfm(bufL,bufR,start,len);
  } else {
    acquire_nuked(bufL,bufR,start,len);
  }
}

void DivPlatformGenesis::tick() {
  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    chan[i].std.next();

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

    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(0x28,0x00|konOffs[i]);
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,octave(chan[i].baseFreq));
      if (chan[i].freq>262143) chan[i].freq=262143;
      int freqt=toFreq(chan[i].freq);
      immWrite(chanOffs[i]+ADDR_FREQH,freqt>>8);
      immWrite(chanOffs[i]+ADDR_FREQ,freqt&0xff);
      if (chan[i].furnaceDac && dacMode) {
        double off=1.0;
        if (dacSample>=0 && dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=8363.0/(double)s->centerRate;
          }
        }
        dacRate=(1280000*1.25*off)/MAX(1,chan[i].baseFreq);
        if (dacRate<1) dacRate=1;
        if (dumpWrites) addWrite(0xffff0001,1280000/dacRate);
      }
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      immWrite(0x28,0xf0|konOffs[i]);
      chan[i].keyOn=false;
    }
  }

  psg.tick();
  
  for (DivRegWrite& i: psg.getRegisterWrites()) {
    if (dumpWrites) addWrite(i.addr,i.val);
  }
  psg.getRegisterWrites().clear();
}

int DivPlatformGenesis::octave(int freq) {
  if (freq>=82432) {
    return 128;
  } else if (freq>=41216) {
    return 64;
  } else if (freq>=20608) {
    return 32;
  } else if (freq>=10304) {
    return 16;
  } else if (freq>=5152) {
    return 8;
  } else if (freq>=2576) {
    return 4;
  } else if (freq>=1288) {
    return 2;
  } else {
    return 1;
  }
  return 1;
}

int DivPlatformGenesis::toFreq(int freq) {
  if (freq>=82432) {
    return 0x3800|((freq>>7)&0x7ff);
  } else if (freq>=41216) {
    return 0x3000|((freq>>6)&0x7ff);
  } else if (freq>=20608) {
    return 0x2800|((freq>>5)&0x7ff);
  } else if (freq>=10304) {
    return 0x2000|((freq>>4)&0x7ff);
  } else if (freq>=5152) {
    return 0x1800|((freq>>3)&0x7ff);
  } else if (freq>=2576) {
    return 0x1000|((freq>>2)&0x7ff);
  } else if (freq>=1288) {
    return 0x800|((freq>>1)&0x7ff);
  } else {
    return freq&0x7ff;
  }
}

void DivPlatformGenesis::muteChannel(int ch, bool mute) {
  if (ch>5) {
    psg.muteChannel(ch-6,mute);
    return;
  }
  isMuted[ch]=mute;
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
}

int DivPlatformGenesis::dispatch(DivCommand c) {
  if (c.chan>5) {
    c.chan-=6;
    return psg.dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.chan==5) {
        if (ins->type==DIV_INS_AMIGA) {
          dacMode=1;
          rWrite(0x2b,1<<7);
        } else if (chan[c.chan].furnaceDac) {
          dacMode=0;
          rWrite(0x2b,0<<7);
        }
      }
      if (c.chan==5 && dacMode) {
        if (skipRegisterWrites) break;
        if (ins->type==DIV_INS_AMIGA) { // Furnace mode
          dacSample=ins->amiga.initSample;
          if (dacSample<0 || dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002,0);
            break;
          } else {
            if (dumpWrites) addWrite(0xffff0000,dacSample);
          }
          dacPos=0;
          dacPeriod=0;
          chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
          chan[c.chan].freqChanged=true;
          chan[c.chan].furnaceDac=true;
        } else { // compatible mode
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          dacSample=12*sampleBank+chan[c.chan].note%12;
          if (dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002,0);
            break;
          } else {
            if (dumpWrites) addWrite(0xffff0000,dacSample);
          }
          dacPos=0;
          dacPeriod=0;
          dacRate=1280000/MAX(1,parent->getSample(dacSample)->rate);
          if (dumpWrites) addWrite(0xffff0001,parent->getSample(dacSample)->rate);
          chan[c.chan].furnaceDac=false;
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
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[c.chan].state.alg][i]) {
            if (!chan[c.chan].active || chan[c.chan].insChanged) {
              rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
            }
          } else {
            if (chan[c.chan].insChanged) {
              rWrite(baseAddr+ADDR_TL,op.tl);
            }
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      }
      if (chan[c.chan].insChanged) {
        rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
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
      if (c.chan==5) {
        dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      if (c.chan==5) {
        dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
      }
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
      rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
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
    case DIV_CMD_SAMPLE_MODE: {
      dacMode=c.value;
      rWrite(0x2b,c.value<<7);
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
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
      rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
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
      break;
    }
    case DIV_CMD_FM_AR: {
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
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformGenesis::forceIns() {
  for (int i=0; i<6; i++) {
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
  psg.forceIns();
}

void DivPlatformGenesis::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
  psg.toggleRegisterDump(enable);
}

void* DivPlatformGenesis::getChanState(int ch) {
  if (ch>5) return psg.getChanState(ch-6);
  return &chan[ch];
}

unsigned char* DivPlatformGenesis::getRegisterPool() {
  return regPool;
}

int DivPlatformGenesis::getRegisterPoolSize() {
  return 512;
}

void DivPlatformGenesis::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,512);
  if (useYMFM) {
    fm_ymfm->reset();
  }
  OPN2_Reset(&fm);
  OPN2_SetChipType(ladder?ym3438_mode_ym2612:0);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<10; i++) {
    chan[i]=DivPlatformGenesis::Channel();
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  dacMode=0;
  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;
  lfoValue=8;

  extMode=false;

  // LFO
  immWrite(0x22,lfoValue);
  
  delay=0;
  
  // PSG
  psg.reset();
  psg.getRegisterWrites().clear();
  psgClocks=0;
  psgOut=0;
}

bool DivPlatformGenesis::isStereo() {
  return true;
}

bool DivPlatformGenesis::keyOffAffectsArp(int ch) {
  return (ch>5);
}

bool DivPlatformGenesis::keyOffAffectsPorta(int ch) {
  return (ch>5);
}

void DivPlatformGenesis::notifyInsChange(int ins) {
  for (int i=0; i<10; i++) {
    if (i>5) {
      psg.notifyInsChange(ins);
    } else if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGenesis::notifyInsDeletion(void* ins) {
  psg.notifyInsDeletion(ins);
}

void DivPlatformGenesis::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformGenesis::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformGenesis::getPortaFloor(int ch) {
  return (ch>5)?12:0;
}

void DivPlatformGenesis::setYMFM(bool use) {
  useYMFM=use;
}

void DivPlatformGenesis::setFlags(unsigned int flags) {
  if (flags==3) {
    chipClock=COLOR_NTSC*12.0/7.0;
  } else if (flags==2) {
    chipClock=8000000.0;
  } else if (flags==1) {
    chipClock=COLOR_PAL*12.0/7.0;
  } else {
    chipClock=COLOR_NTSC*15.0/7.0;
  }
  psg.setFlags(flags==1);
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
  }
}

int DivPlatformGenesis::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  ladder=false;
  skipRegisterWrites=false;
  for (int i=0; i<10; i++) {
    isMuted[i]=false;
  }
  fm_ymfm=NULL;
  psg.init(p,4,sugRate,flags==1);
  setFlags(flags);

  reset();
  return 10;
}

void DivPlatformGenesis::quit() {
  if (fm_ymfm!=NULL) delete fm_ymfm;
  psg.quit();
}

DivPlatformGenesis::~DivPlatformGenesis() {
}
