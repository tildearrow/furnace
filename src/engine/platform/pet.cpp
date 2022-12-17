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
        chan[0].sreg=chan[0].wave;
      }
      break;
    case 10:
      chan[0].sreg=val;
      if (hwSROutput) chan[0].cnt=2;
      break;
  }
  regPool[addr]=val;
}

void DivPlatformPET::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  bool hwSROutput=((regPool[11]>>2)&7)==4;
  if (chan[0].enable) {
    int reload=regPool[8]*2+4;
    if (!hwSROutput) {
      reload+=regPool[9]*512;
    }
    for (size_t h=start; h<start+len; h++) {
      if (SAMP_DIVIDER>chan[0].cnt) {
        chan[0].out=(chan[0].sreg&1)*32767;
        chan[0].sreg=(chan[0].sreg>>1)|((chan[0].sreg&1)<<7);
        chan[0].cnt+=reload-SAMP_DIVIDER;
      } else {
        chan[0].cnt-=SAMP_DIVIDER;
      }
      bufL[h]=chan[0].out;
      bufR[h]=chan[0].out;
      oscBuf->data[oscBuf->needle++]=chan[0].out;
    }
    // emulate driver writes to PCR
    if (!hwSROutput) regPool[12]=chan[0].out?0xe0:0xc0;
  } else {
    chan[0].out=0;
    for (size_t h=start; h<start+len; h++) {
      bufL[h]=0;
      bufR[h]=0;
      oscBuf->data[oscBuf->needle++]=0;
    }
  }
}

void DivPlatformPET::writeOutVol() {
  if (chan[0].active && !isMuted && chan[0].outVol>0) {
    chan[0].enable=true;
    rWrite(11,regPool[9]==0?16:0);
  } else {
    chan[0].enable=false;
    rWrite(11,0);
  }
}

void DivPlatformPET::tick(bool sysTick) {
  chan[0].std.next();
  if (chan[0].std.vol.had) {
    chan[0].outVol=chan[0].std.vol.val&chan[0].vol;
    writeOutVol();
  }
  if (NEW_ARP_STRAT) {
    chan[0].handleArp();
  } else if (chan[0].std.arp.had) {
    if (!chan[0].inPorta) {
      chan[0].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[0].note,chan[0].std.arp.val));
    }
    chan[0].freqChanged=true;
  }
  if (chan[0].std.wave.had) {
    if (chan[0].wave!=chan[0].std.wave.val) {
      chan[0].wave=chan[0].std.wave.val;
      rWrite(10,chan[0].wave);
    }
  }
  if (chan[0].std.pitch.had) {
    if (chan[0].std.pitch.mode) {
      chan[0].pitch2+=chan[0].std.pitch.val;
      CLAMP_VAR(chan[0].pitch2,-32768,32767);
    } else {
      chan[0].pitch2=chan[0].std.pitch.val;
    }
    chan[0].freqChanged=true;
  }
  if (chan[0].freqChanged || chan[0].keyOn || chan[0].keyOff) {
    chan[0].freq=parent->calcFreq(chan[0].baseFreq,chan[0].pitch,chan[0].fixedArp?chan[0].baseNoteOverride:chan[0].arpOff,chan[0].fixedArp,true,0,chan[0].pitch2,chipClock,CHIP_DIVIDER)-2;
    if (chan[0].freq>65535) chan[0].freq=65535;
    if (chan[0].freq<0) chan[0].freq=0;
    rWrite(8,chan[0].freq&0xff);
    rWrite(9,chan[0].freq>>8);
    if (chan[0].keyOn) {
      if (!chan[0].std.vol.will) {
        chan[0].outVol=chan[0].vol;
      }
      chan[0].keyOn=false;
    }
    if (chan[0].keyOff) {
      chan[0].keyOff=false;
    }
    // update mode setting and channel enable
    writeOutVol();
    chan[0].freqChanged=false;
  }
}

int DivPlatformPET::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[0].ins,DIV_INS_PET);
      if (c.value!=DIV_NOTE_NULL) {
        chan[0].baseFreq=NOTE_PERIODIC(c.value);
        chan[0].freqChanged=true;
        chan[0].note=c.value;
      }
      chan[0].active=true;
      chan[0].keyOn=true;
      chan[0].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[0].std.vol.will) {
        chan[0].outVol=chan[0].vol;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[0].active=false;
      chan[0].keyOff=true;
      chan[0].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[0].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[0].ins!=c.value || c.value2==1) {
        chan[0].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[0].vol!=c.value) {
        chan[0].vol=c.value;
        if (!chan[0].std.vol.had) {
          chan[0].outVol=chan[0].vol;
          writeOutVol();
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[0].vol;
      break;
    case DIV_CMD_PITCH:
      chan[0].pitch=c.value;
      chan[0].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      chan[0].wave=c.value;
      rWrite(10,chan[0].wave);
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
      bool return2=false;
      if (destFreq>chan[0].baseFreq) {
        chan[0].baseFreq+=c.value;
        if (chan[0].baseFreq>=destFreq) {
          chan[0].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[0].baseFreq-=c.value;
        if (chan[0].baseFreq<=destFreq) {
          chan[0].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[0].freqChanged=true;
      if (return2) {
        chan[0].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan[0].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[0].std.arp.val):(0)));
      chan[0].freqChanged=true;
      chan[0].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[0].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[0].macroInit(parent->getIns(chan[0].ins,DIV_INS_PET));
      }
      if (!chan[0].inPorta && c.value && !parent->song.brokenPortaArp && chan[0].std.arp.will && !NEW_ARP_STRAT) chan[0].baseFreq=NOTE_PERIODIC(chan[0].note);
      chan[0].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 1;
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

void DivPlatformPET::muteChannel(int ch, bool mute) {
  isMuted=mute;
  writeOutVol();
}

void DivPlatformPET::forceIns() {
  chan[0].insChanged=true;
  chan[0].freqChanged=true;
  writeOutVol();
}

void* DivPlatformPET::getChanState(int ch) {
  return &chan;
}

DivMacroInt* DivPlatformPET::getChanMacroInt(int ch) {
  return &chan[0].std;
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
  chan[0]=Channel();
  chan[0].std.setEngine(parent);
}

bool DivPlatformPET::isStereo() {
  return false;
}

void DivPlatformPET::notifyInsDeletion(void* ins) {
  chan[0].std.notifyInsDeletion((DivInstrument*)ins);
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
  CHECK_CUSTOM_CLOCK;
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
