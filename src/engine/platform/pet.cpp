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

#include "pet.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) {regPool[(a)]=(v)&0xff; if((a)==10) {chan.sreg=(v); chan.cnt=2;}}

#define CHIP_DIVIDER 16
#define SAMP_DIVIDER 4

const char* regCheatSheet6522[]={
  "T2L", "08",
  "SR", "0A",
  "ACR", "0B",
  NULL
};

const char** DivPlatformPET::getRegisterSheet() {
  return regCheatSheet6522;
}

const char* DivPlatformPET::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Change waveform";
      break;
  }
  return NULL;
}

void DivPlatformPET::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  // high-level emulation of 6522 shift register for now
  int t2=regPool[8]*2+4;
  if (((regPool[11]>>2)&7)==4) {
    for (size_t h=start; h<start+len; h++) {
      int cycs=SAMP_DIVIDER;
      while (cycs>0) {
        int adv=MIN(cycs,chan.cnt);
        chan.cnt-=adv;
        cycs-=adv;
        if (chan.cnt==0) {
          chan.out=(chan.sreg&1)*32767;
          chan.sreg=(chan.sreg>>1)|((chan.sreg&1)<<7);
          chan.cnt=t2;
        }
      }
      bufL[h]=chan.out;
      bufR[h]=chan.out;
    }
  } else {
    chan.out=0;
    for (size_t h=start; h<start+len; h++) {
      bufL[h]=0;
      bufR[h]=0;
    }
  }
}

void DivPlatformPET::updateWave() {
  DivWavetable* wt=parent->getWave(chan.wave);
  if (wt->max<1 || wt->len<1) {
    rWrite(10,0);
  } else {
    unsigned char sr=0;
    for (int i=0; i<8; i++) {
      sr=(sr<<1)|((wt->data[i*wt->len/8]*2)/(wt->max+1));
    }
    rWrite(10,sr);
  }
}

void DivPlatformPET::writeOutVol() {
  if (chan.active && !isMuted && chan.outVol>0) {
    if (regPool[11]!=16) {
      rWrite(11,16);
      rWrite(10,regPool[10]);
    }
  } else {
    rWrite(11,0);
  }
}

void DivPlatformPET::tick() {
  chan.std.next();
  if (chan.std.hadVol) {
    chan.outVol=chan.std.vol&chan.vol;
    writeOutVol();
  }
  if (chan.std.hadArp) {
    if (!chan.inPorta) {
      if (chan.std.arpMode) {
        chan.baseFreq=NOTE_PERIODIC(chan.std.arp);
      } else {
        chan.baseFreq=NOTE_PERIODIC(chan.note+chan.std.arp);
      }
    }
    chan.freqChanged=true;
  } else {
    if (chan.std.arpMode && chan.std.finishedArp) {
      chan.baseFreq=NOTE_PERIODIC(chan.note);
      chan.freqChanged=true;
    }
  }
  if (chan.std.hadWave) {
    if (chan.wave!=chan.std.wave) {
      chan.wave=chan.std.wave;
      updateWave();
    }
  }
  if (chan.freqChanged || chan.keyOn || chan.keyOff) {
    chan.freq=parent->calcFreq(chan.baseFreq,chan.pitch,true);
    if (chan.freq>257) chan.freq=257;
    if (chan.freq<2) chan.freq=2;
    rWrite(8,chan.freq-2);
    if (chan.keyOn) {
      if (!chan.std.willVol) {
        chan.outVol=chan.vol;
        writeOutVol();
      }
      if (chan.wave<0) {
        chan.wave=0;
        updateWave();
      }
      chan.keyOn=false;
    }
    if (chan.keyOff) {
      rWrite(11,0);
      chan.keyOff=false;
    }
    chan.freqChanged=false;
  }
}

int DivPlatformPET::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan.ins);
      if (c.value!=DIV_NOTE_NULL) {
        chan.baseFreq=NOTE_PERIODIC(c.value);
        chan.freqChanged=true;
        chan.note=c.value;
      }
      chan.active=true;
      chan.keyOn=true;
      chan.std.init(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan.active=false;
      chan.keyOff=true;
      chan.std.init(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan.std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan.ins!=c.value || c.value2==1) {
        chan.ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan.vol!=c.value) {
        chan.vol=c.value;
        if (!chan.std.hadVol) {
          chan.outVol=chan.vol;
          writeOutVol();
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan.vol;
      break;
    case DIV_CMD_PITCH:
      chan.pitch=c.value;
      chan.freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      chan.wave=c.value;
      updateWave();
      chan.keyOn=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
      bool return2=false;
      if (destFreq>chan.baseFreq) {
        chan.baseFreq+=c.value;
        if (chan.baseFreq>=destFreq) {
          chan.baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan.baseFreq-=c.value;
        if (chan.baseFreq<=destFreq) {
          chan.baseFreq=destFreq;
          return2=true;
        }
      }
      chan.freqChanged=true;
      if (return2) {
        chan.inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan.baseFreq=NOTE_PERIODIC(c.value+((chan.std.willArp && !chan.std.arpMode)?(chan.std.arp):(0)));
      chan.freqChanged=true;
      chan.note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan.active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan.std.init(parent->getIns(chan.ins));
      }
      chan.inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 1;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformPET::muteChannel(int ch, bool mute) {
  isMuted=mute;
  writeOutVol();
}

void DivPlatformPET::forceIns() {
  chan.insChanged=true;
  chan.freqChanged=true;
  updateWave();
  writeOutVol();
}

void* DivPlatformPET::getChanState(int ch) {
  return &chan;
}

unsigned char* DivPlatformPET::getRegisterPool() {
  return regPool;
}

int DivPlatformPET::getRegisterPoolSize() {
  return 16;
}

void DivPlatformPET::reset() {
  memset(regPool,0,16);
  chan=Channel();
  chan.vol=1;
}

bool DivPlatformPET::isStereo() {
  return false;
}

void DivPlatformPET::notifyWaveChange(int wave) {
  if (chan.wave==wave) {
    updateWave();
  }
}

void DivPlatformPET::notifyInsDeletion(void* ins) {
  chan.std.notifyInsDeletion((DivInstrument*)ins);
}

void DivPlatformPET::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformPET::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformPET::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  chipClock=1000000;
  rate=chipClock/SAMP_DIVIDER; // = 250000kHz
  isMuted=false;
  reset();
  return 1;
}

DivPlatformPET::~DivPlatformPET() {
}
