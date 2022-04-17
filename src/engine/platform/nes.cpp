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

#include "nes.h"
#include "sound/nes/cpu_inline.h"
#include "../engine.h"
#include <cstddef>
#include <math.h>

struct _nla_table nla_table;

#define CHIP_DIVIDER 16

#define rWrite(a,v) if (!skipRegisterWrites) {apu_wr_reg(nes,a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }

const char* regCheatSheetNES[]={
  "S0Volume", "4000",
  "S0Sweep", "4001",
  "S0PeriodL", "4002",
  "S0PeriodH", "4003",
  "S1Volume", "4004",
  "S1Sweep", "4005",
  "S1PeriodL", "4006",
  "S1PeriodH", "4007",
  "TRVolume", "4008",
  "TRPeriodL", "400A",
  "TRPeriodH", "400B",
  "NSVolume", "400C",
  "NSPeriod", "400E",
  "NSLength", "400F",
  "DMCControl", "4010",
  "DMCLoad", "4011",
  "DMCAddr", "4012",
  "DMCLength", "4013",
  "APUControl", "4015",
  "APUFrameCtl", "4017",
  NULL
};

const char** DivPlatformNES::getRegisterSheet() {
  return regCheatSheetNES;
}

const char* DivPlatformNES::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x12:
      return "12xx: Set duty cycle/noise mode (pulse: 0 to 3; noise: 0 or 1)";
      break;
    case 0x13:
      return "13xy: Sweep up (x: time; y: shift)";
      break;
    case 0x14:
      return "14xy: Sweep down (x: time; y: shift)";
      break;
  }
  return NULL;
}

void DivPlatformNES::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    if (dacSample!=-1) {
      dacPeriod+=dacRate;
      if (dacPeriod>=rate) {
        DivSample* s=parent->getSample(dacSample);
        if (s->samples>0) {
          if (!isMuted[4]) {
            unsigned char next=((unsigned char)s->data8[dacPos]+0x80)>>1;
            if (dacAntiClickOn && dacAntiClick<next) {
              dacAntiClick+=8;
              rWrite(0x4011,dacAntiClick);
            } else {
              dacAntiClickOn=false;
              rWrite(0x4011,next);
            }
          }
          if (++dacPos>=s->samples) {
            if (s->loopStart>=0 && s->loopStart<(int)s->samples) {
              dacPos=s->loopStart;
            } else {
              dacSample=-1;
            }
          }
          dacPeriod-=rate;
        } else {
          dacSample=-1;
        }
      }
    }
  
    apu_tick(nes,NULL);
    nes->apu.odd_cycle=!nes->apu.odd_cycle;
    if (nes->apu.clocked) {
      nes->apu.clocked=false;
    }
    int sample=(pulse_output(nes)+tnd_output(nes));
    if (sample>32767) sample=32767;
    if (sample<-32768) sample=-32768;
    bufL[i]=sample;
  }
}

static unsigned char noiseTable[253]={
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15
};

void DivPlatformNES::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      // ok, why are the volumes like that?
      chan[i].outVol=MIN(15,chan[i].std.vol.val)-(15-(chan[i].vol&15));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (i==2) { // triangle
        rWrite(0x4000+i*4,(chan[i].outVol==0)?0:255);
        chan[i].freqChanged=true;
      } else {
        rWrite(0x4000+i*4,0x30|chan[i].outVol|((chan[i].duty&3)<<6));
      }
    }
    if (chan[i].std.arp.had) {
      if (i==3) { // noise
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=chan[i].std.arp.val;
        } else {
          chan[i].baseFreq=chan[i].note+chan[i].std.arp.val;
        }
        if (chan[i].baseFreq>255) chan[i].baseFreq=255;
        if (chan[i].baseFreq<0) chan[i].baseFreq=0;
      } else {
        if (!chan[i].inPorta) {
          if (chan[i].std.arp.mode) {
            chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp.val);
          } else {
            chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp.val);
          }
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      if (i==3) {
        if (parent->song.properNoiseLayout) {
          chan[i].duty&=1;
        } else if (chan[i].duty>1) {
          chan[i].duty=1;
        }
      }
      if (i!=2) {
        rWrite(0x4000+i*4,0x30|chan[i].outVol|((chan[i].duty&3)<<6));
      }
      if (i==3) { // noise
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }
    if (chan[i].sweepChanged) {
      chan[i].sweepChanged=false;
      if (i==0) {
        //rWrite(16+i*5,chan[i].sweep);
      }
    }
    if (i<2) if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].freqChanged=true;
        chan[i].prevFreq=-1;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (i==3) { // noise
        int ntPos=chan[i].baseFreq;
        if (ntPos<0) ntPos=0;
        if (ntPos>252) ntPos=252;
        chan[i].freq=(parent->song.properNoiseLayout)?(15-(chan[i].baseFreq&15)):(noiseTable[ntPos]);
      } else {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true)-1+chan[i].std.pitch.val;
        if (chan[i].freq>2047) chan[i].freq=2047;
        if (chan[i].freq<0) chan[i].freq=0;
      }
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
      }
      if (chan[i].keyOff) {
        //rWrite(16+i*5+2,8);
        if (i==2) { // triangle
          rWrite(0x4000+i*4,0x00);
        } else {
          rWrite(0x4000+i*4,0x30);
        }
      }
      if (i==3) { // noise
        rWrite(0x4002+i*4,(chan[i].duty<<7)|chan[i].freq);
        rWrite(0x4003+i*4,0xf0);
      } else {
        rWrite(0x4002+i*4,chan[i].freq&0xff);
        if ((chan[i].prevFreq>>8)!=(chan[i].freq>>8) || i==2) {
          rWrite(0x4003+i*4,0xf8|(chan[i].freq>>8));
        }
        if (chan[i].freq!=65535 && chan[i].freq!=0) {
          chan[i].prevFreq=chan[i].freq;
        }
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  // PCM
  if (chan[4].freqChanged) {
    chan[4].freq=parent->calcFreq(chan[4].baseFreq,chan[4].pitch,false);
    if (chan[4].furnaceDac) {
      double off=1.0;
      if (dacSample>=0 && dacSample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(dacSample);
        off=(double)s->centerRate/8363.0;
      }
      dacRate=MIN(chan[4].freq*off,32000);
      if (dumpWrites) addWrite(0xffff0001,dacRate);
    }
    chan[4].freqChanged=false;
  }
}

int DivPlatformNES::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.chan==4) { // PCM
        DivInstrument* ins=parent->getIns(chan[c.chan].ins);
        if (ins->type==DIV_INS_AMIGA) {
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
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].baseFreq=parent->song.tuning*pow(2.0f,((float)(c.value+3)/12.0f));
            chan[c.chan].freqChanged=true;
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].keyOn=true;
          chan[c.chan].furnaceDac=true;
        } else {
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
          dacRate=parent->getSample(dacSample)->rate;
          if (dumpWrites) addWrite(0xffff0001,dacRate);
          chan[c.chan].furnaceDac=false;
        }
        break;
      } else if (c.chan==3) { // noise
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=c.value;
        }
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      if (c.chan==2) {
        rWrite(0x4000+c.chan*4,0xff);
      } else {
        rWrite(0x4000+c.chan*4,0x30|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
      }
      break;
    case DIV_CMD_NOTE_OFF:
      if (c.chan==4) {
        dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
      }
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) {
          if (c.chan==2) {
            rWrite(0x4000+c.chan*4,0xff);
          } else {
            rWrite(0x4000+c.chan*4,0x30|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
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
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=c.value;
      if (c.chan==3) { // noise
        chan[c.chan].freqChanged=true;
      }
      break;
    case DIV_CMD_NES_SWEEP:
      if (c.chan>1) break;
      if (c.value2==0) {
        chan[c.chan].sweep=0x08;
      } else {
        if (!c.value) { // down
          chan[c.chan].sweep=0x88|(c.value2&0x77);
        } else { // up
          chan[c.chan].sweep=0x80|(c.value2&0x77);
        }
      }
      rWrite(0x4001+(c.chan*4),chan[c.chan].sweep);
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformNES::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  nes->muted[ch]=mute;
}

void DivPlatformNES::forceIns() {
  for (int i=0; i<5; i++) {
    chan[i].insChanged=true;
    chan[i].prevFreq=65535;
  }
  rWrite(0x4001,chan[0].sweep);
  rWrite(0x4005,chan[1].sweep);
}

void* DivPlatformNES::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformNES::getRegisterPool() {
  return regPool;
}

int DivPlatformNES::getRegisterPoolSize() {
  return 32;
}

float DivPlatformNES::getPostAmp() {
  return 128.0f;
}

void DivPlatformNES::reset() {
  for (int i=0; i<5; i++) {
    chan[i]=DivPlatformNES::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;

  apu_turn_on(nes,apuType);
  memset(regPool,0,128);
  nes->apu.cpu_cycles=0;
  nes->apu.cpu_opcode_cycle=0;

  rWrite(0x4015,0x1f);
  rWrite(0x4001,chan[0].sweep);
  rWrite(0x4005,chan[1].sweep);

  dacAntiClickOn=true;
  dacAntiClick=0;
}

bool DivPlatformNES::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformNES::setFlags(unsigned int flags) {
  if (flags==2) { // Dendy
    rate=COLOR_PAL*2.0/5.0;
    apuType=2;
    nes->apu.type=apuType;
  } else if (flags==1) { // PAL
    rate=COLOR_PAL*3.0/8.0;
    apuType=1;
    nes->apu.type=apuType;
  } else { // NTSC
    rate=COLOR_NTSC/2.0;
    apuType=0;
    nes->apu.type=apuType;
  }
  chipClock=rate;
}

void DivPlatformNES::notifyInsDeletion(void* ins) {
  for (int i=0; i<5; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformNES::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformNES::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformNES::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  apuType=flags;
  dumpWrites=false;
  skipRegisterWrites=false;
  nes=new struct NESAPU;
  for (int i=0; i<5; i++) {
    isMuted[i]=false;
    nes->muted[i]=false;
  }
  setFlags(flags);

  init_nla_table(500,500);
  reset();
  return 5;
}

void DivPlatformNES::quit() {
  delete nes;
}

DivPlatformNES::~DivPlatformNES() {
}
