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

#include "fds.h"
#include "sound/nes/cpu_inline.h"
#include "../engine.h"
#include <math.h>

#define CHIP_FREQBASE 262144

#define rWrite(a,v) if (!skipRegisterWrites) {fds_wr_mem(fds,a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }

const char* regCheatSheetFDS[]={
  "IOCtrl", "4023",
  "Wave", "4040",
  "Volume", "4080",
  "FreqL", "4082",
  "FreqH", "4083",
  "ModCtrl", "4084",
  "ModCount", "4085",
  "ModFreqL", "4086",
  "ModFreqH", "4087",
  "ModWrite", "4088",
  "WaveCtrl", "4089",
  "EnvSpeed", "408A",
  "ReadVol", "4090",
  "ReadPos", "4091",
  "ReadModV", "4092",
  "ReadModP", "4093",
  "ReadModCG", "4094",
  "ReadModInc", "4095",
  "ReadWave", "4096",
  "ReadModCount", "4097",
  NULL
};

const char** DivPlatformFDS::getRegisterSheet() {
  return regCheatSheetFDS;
}

const char* DivPlatformFDS::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Change waveform";
      break;
    case 0x11:
      return "11xx: Set modulation depth";
      break;
    case 0x12:
      return "12xy: Set modulation speed high byte (x: enable; y: value)";
      break;
    case 0x13:
      return "13xx: Set modulation speed low byte";
      break;
    case 0x14:
      return "14xx: Set modulator position";
      break;
    case 0x15:
      return "15xx: Set modulator table to waveform";
      break;
  }
  return NULL;
}

void DivPlatformFDS::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    extcl_apu_tick_FDS(fds);
    int sample=isMuted[0]?0:fds->snd.main.output;
    if (sample>32767) sample=32767;
    if (sample<-32768) sample=-32768;
    bufL[i]=sample;
  }
}

void DivPlatformFDS::updateWave() {
  // TODO: master volume
  rWrite(0x4089,0x80);
  for (int i=0; i<64; i++) {
    rWrite(0x4040+i,ws.output[i]);
  }
  rWrite(0x4089,0);
}

void DivPlatformFDS::tick(bool sysTick) {
  for (int i=0; i<1; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      // ok, why are the volumes like that?
      chan[i].outVol=MIN(32,chan[i].std.vol.val)-(32-MIN(32,chan[i].vol));
      if (chan[i].outVol<0) chan[i].outVol=0;
      rWrite(0x4080,0x80|chan[i].outVol);
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note+chan[i].std.arp.val);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    /*
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
    }*/
    if (chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        ws.changeWave1(chan[i].wave);
        //if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }
    if (chan[i].active) {
      if (ws.tick()) {
        updateWave();
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].std.ex1.had) { // mod depth
      chan[i].modOn=chan[i].std.ex1.val;
      chan[i].modDepth=chan[i].std.ex1.val;
      rWrite(0x4084,(chan[i].modOn<<7)|0x40|chan[i].modDepth);
    }
    if (chan[i].std.ex2.had) { // mod speed
      chan[i].modFreq=chan[i].std.ex2.val;
      rWrite(0x4086,chan[i].modFreq&0xff);
      rWrite(0x4087,chan[i].modFreq>>8);
    }
    if (chan[i].std.ex3.had) { // mod position
      chan[i].modPos=chan[i].std.ex3.val;
      rWrite(0x4087,0x80|chan[i].modFreq>>8);
      rWrite(0x4085,chan[i].modPos);
      rWrite(0x4087,chan[i].modFreq>>8);
    }
    if (chan[i].sweepChanged) {
      chan[i].sweepChanged=false;
      if (i==0) {
        //rWrite(16+i*5,chan[i].sweep);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false)+chan[i].std.pitch.val;
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].keyOn) {
        // ???
      }
      if (chan[i].keyOff) {
        rWrite(0x4080,0x80);
      }
      rWrite(0x4082,chan[i].freq&0xff);
      rWrite(0x4083,(chan[i].freq>>8)&15);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformFDS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FDS);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      if (chan[c.chan].insChanged) {
        if (ins->fds.initModTableWithFirstWave) { // compatible
          if (chan[c.chan].wave==-1) {
            DivWavetable* wt=parent->getWave(0);
            for (int i=0; i<32; i++) {
              if (wt->max<1 || wt->len<1) {
                chan[c.chan].modTable[i]=0;
              } else {
                int data=wt->data[i*MIN(32,wt->len)/32]*7/wt->max;
                if (data<0) data=0;
                if (data>7) data=7;
                chan[c.chan].modTable[i]=data;
              }
            }
            rWrite(0x4087,0x80|chan[c.chan].modFreq>>8);
            for (int i=0; i<32; i++) {
              rWrite(0x4088,chan[c.chan].modTable[i]);
            }
            rWrite(0x4087,chan[c.chan].modFreq>>8);
          }
        } else { // The Familiar Way
          chan[c.chan].modDepth=ins->fds.modDepth;
          chan[c.chan].modOn=ins->fds.modDepth;
          chan[c.chan].modFreq=ins->fds.modSpeed;
          rWrite(0x4084,(chan[c.chan].modOn<<7)|0x40|chan[c.chan].modDepth);
          rWrite(0x4086,chan[c.chan].modFreq&0xff);

          rWrite(0x4087,0x80|chan[c.chan].modFreq>>8);
          for (int i=0; i<32; i++) {
            chan[c.chan].modTable[i]=ins->fds.modTable[i]&7;
            rWrite(0x4088,chan[c.chan].modTable[i]);
          }
          rWrite(0x4087,chan[c.chan].modFreq>>8);
        }
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(ins);
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        ws.changeWave1(chan[c.chan].wave);
      }
      ws.init(ins,64,63,chan[c.chan].insChanged);
      rWrite(0x4080,0x80|chan[c.chan].vol);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        rWrite(0x4080,0x80|chan[c.chan].vol);
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      if (chan[c.chan].wave!=c.value) {
        chan[c.chan].wave=c.value;
        ws.changeWave1(chan[c.chan].wave);
      }
      break;
    case DIV_CMD_FDS_MOD_DEPTH:
      chan[c.chan].modDepth=c.value;
      rWrite(0x4084,(chan[c.chan].modOn<<7)|0x40|chan[c.chan].modDepth);
      break;
    case DIV_CMD_FDS_MOD_HIGH:
      chan[c.chan].modOn=c.value>>4;
      chan[c.chan].modFreq=((c.value&15)<<8)|(chan[c.chan].modFreq&0xff);
      rWrite(0x4084,(chan[c.chan].modOn<<7)|0x40|chan[c.chan].modDepth);
      rWrite(0x4087,chan[c.chan].modFreq>>8);
      break;
    case DIV_CMD_FDS_MOD_LOW:
      chan[c.chan].modFreq=(chan[c.chan].modFreq&0xf00)|c.value;
      rWrite(0x4086,chan[c.chan].modFreq&0xff);
      break;
    case DIV_CMD_FDS_MOD_POS:
      chan[c.chan].modPos=c.value&0x7f;
      rWrite(0x4087,0x80|chan[c.chan].modFreq>>8);
      rWrite(0x4085,chan[c.chan].modPos);
      rWrite(0x4087,chan[c.chan].modFreq>>8);
      break;
    case DIV_CMD_FDS_MOD_WAVE: {
      DivWavetable* wt=parent->getWave(c.value);
      for (int i=0; i<32; i++) {
        if (wt->max<1 || wt->len<1) {
          rWrite(0x4040+i,0);
        } else {
          int data=wt->data[i*MIN(32,wt->len)/32]*7/wt->max;
          if (data<0) data=0;
          if (data>7) data=7;
          chan[c.chan].modTable[i]=data;
        }
      }
      rWrite(0x4087,0x80|chan[c.chan].modFreq>>8);
      for (int i=0; i<32; i++) {
        rWrite(0x4088,chan[c.chan].modTable[i]);
      }
      rWrite(0x4087,chan[c.chan].modFreq>>8);
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
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
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_FDS));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 32;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformFDS::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformFDS::forceIns() {
  for (int i=0; i<1; i++) {
    chan[i].insChanged=true;
    chan[i].prevFreq=65535;
  }
  updateWave();
}

void* DivPlatformFDS::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformFDS::getRegisterPool() {
  return regPool;
}

int DivPlatformFDS::getRegisterPoolSize() {
  return 128;
}

void DivPlatformFDS::reset() {
  for (int i=0; i<1; i++) {
    chan[i]=DivPlatformFDS::Channel();
    chan[i].std.setEngine(parent);
  }
  ws.setEngine(parent);
  ws.init(NULL,64,63,false);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  fds_reset(fds);
  memset(regPool,0,128);

  rWrite(0x4023,0);
  rWrite(0x4023,0x83);
  rWrite(0x4089,0);
}

bool DivPlatformFDS::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformFDS::setFlags(unsigned int flags) {
  if (flags==2) { // Dendy
    rate=COLOR_PAL*2.0/5.0;
  } else if (flags==1) { // PAL
    rate=COLOR_PAL*3.0/8.0;
  } else { // NTSC
    rate=COLOR_NTSC/2.0;
  }
  chipClock=rate;
}

void DivPlatformFDS::notifyInsDeletion(void* ins) {
  for (int i=0; i<1; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformFDS::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformFDS::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformFDS::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  apuType=flags;
  dumpWrites=false;
  skipRegisterWrites=false;
  fds=new struct _fds;
  for (int i=0; i<1; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);

  reset();
  return 5;
}

void DivPlatformFDS::quit() {
  delete fds;
}

DivPlatformFDS::~DivPlatformFDS() {
}
