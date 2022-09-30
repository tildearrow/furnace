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

#define CHIP_DIVIDER 16
#define SAMP_DIVIDER 4

const char* regCheatSheet6522[]={
  "T2L", "08",
  "T2H", "09",
  "SR", "0A",
  "ACR", "0B",
  "PCR", "0C",
  NULL
};

const char** DivPlatformPET::getRegisterSheet() {
  return regCheatSheet6522;
}

// high-level emulation of 6522 shift register and driver software for now
void DivPlatformPET::rWrite(unsigned int addr, unsigned char val) {
  bool hwSROutput=((regPool[11]>>2)&7)==4;
  switch (addr) {
    case 9:
      // simulate phase reset from switching between hw/sw shift registers
      if ((regPool[9]==0)^(val==0)) {
        chan.sreg=chan.wave;
      }
      break;
    case 10:
      chan.sreg=val;
      if (hwSROutput) chan.cnt=2;
      break;
  }
  regPool[addr]=val;
}

void DivPlatformPET::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  bool hwSROutput=((regPool[11]>>2)&7)==4;
  if (chan.enable) {
    int reload=regPool[8]*2+4;
    if (!hwSROutput) {
      reload+=regPool[9]*512;
    }
    for (size_t h=start; h<start+len; h++) {
      if (SAMP_DIVIDER>chan.cnt) {
        chan.out=(chan.sreg&1)*32767;
        chan.sreg=(chan.sreg>>1)|((chan.sreg&1)<<7);
        chan.cnt+=reload-SAMP_DIVIDER;
      } else {
        chan.cnt-=SAMP_DIVIDER;
      }
      bufL[h]=chan.out;
      bufR[h]=chan.out;
      oscBuf->data[oscBuf->needle++]=chan.out;
    }
    // emulate driver writes to PCR
    if (!hwSROutput) regPool[12]=chan.out?0xe0:0xc0;
  } else {
    chan.out=0;
    for (size_t h=start; h<start+len; h++) {
      bufL[h]=0;
      bufR[h]=0;
      oscBuf->data[oscBuf->needle++]=0;
    }
  }
}

void DivPlatformPET::writeOutVol() {
  if (chan.active && !isMuted && chan.outVol>0) {
    chan.enable=true;
    rWrite(11,regPool[9]==0?16:0);
  } else {
    chan.enable=false;
    rWrite(11,0);
  }
}

void DivPlatformPET::tick(bool sysTick) {
  chan.std.next();
  if (chan.std.vol.had) {
    chan.outVol=chan.std.vol.val&chan.vol;
    writeOutVol();
  }
  if (chan.std.arp.had) {
    if (!chan.inPorta) {
      chan.baseFreq=NOTE_PERIODIC(parent->calcArp(chan.note,chan.std.arp.val));
    }
    chan.freqChanged=true;
  }
  if (chan.std.wave.had) {
    if (chan.wave!=chan.std.wave.val) {
      chan.wave=chan.std.wave.val;
      rWrite(10,chan.wave);
    }
  }
  if (chan.std.pitch.had) {
    if (chan.std.pitch.mode) {
      chan.pitch2+=chan.std.pitch.val;
      CLAMP_VAR(chan.pitch2,-32768,32767);
    } else {
      chan.pitch2=chan.std.pitch.val;
    }
    chan.freqChanged=true;
  }
  if (chan.freqChanged || chan.keyOn || chan.keyOff) {
    chan.freq=parent->calcFreq(chan.baseFreq,chan.pitch,true,0,chan.pitch2,chipClock,CHIP_DIVIDER)-2;
    if (chan.freq>65535) chan.freq=65535;
    if (chan.freq<0) chan.freq=0;
    rWrite(8,chan.freq&0xff);
    rWrite(9,chan.freq>>8);
    if (chan.keyOn) {
      if (!chan.std.vol.will) {
        chan.outVol=chan.vol;
      }
      chan.keyOn=false;
    }
    if (chan.keyOff) {
      chan.keyOff=false;
    }
    // update mode setting and channel enable
    writeOutVol();
    chan.freqChanged=false;
  }
}

int DivPlatformPET::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan.ins,DIV_INS_PET);
      if (c.value!=DIV_NOTE_NULL) {
        chan.baseFreq=NOTE_PERIODIC(c.value);
        chan.freqChanged=true;
        chan.note=c.value;
      }
      chan.active=true;
      chan.keyOn=true;
      chan.macroInit(ins);
      if (!parent->song.brokenOutVol && !chan.std.vol.will) {
        chan.outVol=chan.vol;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan.active=false;
      chan.keyOff=true;
      chan.macroInit(NULL);
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
        if (!chan.std.vol.had) {
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
      rWrite(10,chan.wave);
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
      chan.baseFreq=NOTE_PERIODIC(c.value+((chan.std.arp.will && !chan.std.arp.mode)?(chan.std.arp.val):(0)));
      chan.freqChanged=true;
      chan.note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan.active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan.macroInit(parent->getIns(chan.ins,DIV_INS_PET));
      }
      if (!chan.inPorta && c.value && !parent->song.brokenPortaArp && chan.std.arp.will) chan.baseFreq=NOTE_PERIODIC(chan.note);
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
  writeOutVol();
}

void* DivPlatformPET::getChanState(int ch) {
  return &chan;
}

DivMacroInt* DivPlatformPET::getChanMacroInt(int ch) {
  return &chan.std;
}

DivDispatchOscBuffer* DivPlatformPET::getOscBuffer(int ch) {
  return oscBuf;
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
  chan.std.setEngine(parent);
}

bool DivPlatformPET::isStereo() {
  return false;
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

int DivPlatformPET::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  chipClock=1000000;
  rate=chipClock/SAMP_DIVIDER; // = 250000kHz
  isMuted=false;
  oscBuf=new DivDispatchOscBuffer;
  oscBuf->rate=rate;
  reset();
  return 1;
}

void DivPlatformPET::quit() {
  delete oscBuf;
}

DivPlatformPET::~DivPlatformPET() {
}
