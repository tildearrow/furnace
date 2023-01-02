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

#include "mmc5.h"
#include "sound/nes/mmc5.h"
#include "../engine.h"
#include <math.h>

#define CHIP_DIVIDER 16

#define rWrite(a,v) if (!skipRegisterWrites) {extcl_cpu_wr_mem_MMC5(mmc5,a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }

const char* regCheatSheetMMC5[]={
  "S0Volume", "5000",
  "S0PeriodL", "5002",
  "S0PeriodH", "5003",
  "S1Volume", "5004",
  "S1PeriodL", "5006",
  "S1PeriodH", "5007",
  "PCMControl", "4010",
  "PCMWrite", "4011",
  "APUControl", "4015",
  NULL
};

const char** DivPlatformMMC5::getRegisterSheet() {
  return regCheatSheetMMC5;
}

void DivPlatformMMC5::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    if (dacSample!=-1) {
      dacPeriod+=dacRate;
      if (dacPeriod>=rate) {
        DivSample* s=parent->getSample(dacSample);
        if (s->samples>0) {
          if (!isMuted[2]) {
            rWrite(0x5011,((unsigned char)s->data8[dacPos]+0x80));
          }
          dacPos++;
          if (s->isLoopable() && dacPos>=(unsigned int)s->loopEnd) {
            dacPos=s->loopStart;
          } else if (dacPos>=s->samples) {
            dacSample=-1;
          }
          dacPeriod-=rate;
        } else {
          dacSample=-1;
        }
      }
    }
  
    extcl_envelope_clock_MMC5(mmc5);
    extcl_length_clock_MMC5(mmc5);
    extcl_apu_tick_MMC5(mmc5);
    if (mmc5->clocked) {
      mmc5->clocked=false;
    }
    int sample=isMuted[0]?0:(mmc5->S3.output*10);
    if (!isMuted[1]) {
      sample+=mmc5->S4.output*10;
    }
    if (!isMuted[2]) {
      sample+=mmc5->pcm.output*2;
    }
    if (sample>32767) sample=32767;
    if (sample<-32768) sample=-32768;
    bufL[i]=sample;

    if (++writeOscBuf>=32) {
      writeOscBuf=0;
      oscBuf[0]->data[oscBuf[0]->needle++]=isMuted[0]?0:((mmc5->S3.output*10)<<7);
      oscBuf[1]->data[oscBuf[1]->needle++]=isMuted[1]?0:((mmc5->S4.output*10)<<7);
      oscBuf[2]->data[oscBuf[2]->needle++]=isMuted[2]?0:((mmc5->pcm.output*2)<<6);
    }
  }
}

void DivPlatformMMC5::tick(bool sysTick) {
  for (int i=0; i<2; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      // ok, why are the volumes like that?
      chan[i].outVol=VOL_SCALE_LINEAR_BROKEN(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      if (chan[i].outVol<0) chan[i].outVol=0;
      rWrite(0x5000+i*4,0x30|chan[i].outVol|((chan[i].duty&3)<<6));
    }
    // TODO: arp macros on NES PCM?
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      rWrite(0x5000+i*4,0x30|chan[i].outVol|((chan[i].duty&3)<<6));
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].freqChanged=true;
        chan[i].prevFreq=-1;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
      if (chan[i].freq>2047) chan[i].freq=2047;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
      }
      if (chan[i].keyOff) {
        //rWrite(16+i*5+2,8);
        rWrite(0x5000+i*4,0x30);
      }
      rWrite(0x5002+i*4,chan[i].freq&0xff);
      if ((chan[i].prevFreq>>8)!=(chan[i].freq>>8)) {
        rWrite(0x5003+i*4,0xf8|(chan[i].freq>>8));
      }
      if (chan[i].freq!=65535 && chan[i].freq!=0) {
        chan[i].prevFreq=chan[i].freq;
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  // PCM
  if (chan[2].freqChanged) {
    chan[2].freq=parent->calcFreq(chan[2].baseFreq,chan[2].pitch,chan[2].fixedArp?chan[2].baseNoteOverride:chan[2].arpOff,chan[2].fixedArp,false,0,chan[2].pitch2,1,1);
    if (chan[2].furnaceDac) {
      double off=1.0;
      if (dacSample>=0 && dacSample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(dacSample);
        off=(double)s->centerRate/8363.0;
      }
      dacRate=MIN(chan[2].freq*off,32000);
      if (dumpWrites) addWrite(0xffff0001,dacRate);
    }
    chan[2].freqChanged=false;
  }
}

int DivPlatformMMC5::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.chan==2) { // PCM
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_STD);
        if (ins->type==DIV_INS_AMIGA) {
          if (c.value!=DIV_NOTE_NULL) dacSample=ins->amiga.getSample(c.value);
          if (dacSample<0 || dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002,0);
            break;
          } else {
            if (dumpWrites) addWrite(0xffff0000,dacSample);
          }
          dacPos=0;
          dacPeriod=0;
          chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value,false);
          if (c.value!=DIV_NOTE_NULL) {
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
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      rWrite(0x5000+c.chan*4,0x30|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
      break;
    case DIV_CMD_NOTE_OFF:
      if (c.chan==2) {
        dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
      }
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
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
          rWrite(0x5000+c.chan*4,0x30|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
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
      int destFreq=(c.chan==2)?(parent->calcBaseFreq(1,1,c.value2,false)):(NOTE_PERIODIC(c.value2));
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
      rWrite(0x5000+c.chan*4,0x30|(chan[c.chan].active?chan[c.chan].outVol:0)|((chan[c.chan].duty&3)<<6));
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_LEGATO:
      if (c.chan==2) {
        chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)),false);
      } else {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      }
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformMMC5::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformMMC5::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].prevFreq=65535;
  }
}

void* DivPlatformMMC5::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformMMC5::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformMMC5::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformMMC5::getRegisterPool() {
  return regPool;
}

int DivPlatformMMC5::getRegisterPoolSize() {
  return 32;
}

float DivPlatformMMC5::getPostAmp() {
  return 64.0f;
}

void DivPlatformMMC5::reset() {
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformMMC5::Channel();
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

  map_init_MMC5(mmc5);
  memset(regPool,0,128);

  rWrite(0x5015,0x03);
  rWrite(0x5010,0x00);
}

bool DivPlatformMMC5::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformMMC5::setFlags(const DivConfig& flags) {
  int clockSel=flags.getInt("clockSel",0);
  if (clockSel==2) { // Dendy
    chipClock=COLOR_PAL*2.0/5.0;
  } else if (clockSel==1) { // PAL
    chipClock=COLOR_PAL*3.0/8.0;
  } else { // NTSC
    chipClock=COLOR_NTSC/2.0;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  for (int i=0; i<3; i++) {
    oscBuf[i]->rate=rate/32;
  }
}

void DivPlatformMMC5::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformMMC5::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformMMC5::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformMMC5::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  writeOscBuf=0;
  mmc5=new struct _mmc5;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
    //mmc5->muted[i]=false; // TODO
  }
  setFlags(flags);

  init_nla_table(500,500);
  reset();
  return 5;
}

void DivPlatformMMC5::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
  delete mmc5;
}

DivPlatformMMC5::~DivPlatformMMC5() {
}
