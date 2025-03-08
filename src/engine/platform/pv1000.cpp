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

#include "pv1000.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) {regPool[(a)]=(v)&0xff; d65010g031_write(&d65010g031,a,v);}

#define CHIP_DIVIDER 1024

const char* regCheatSheetPV1000[]={
  "CH1_Pitch", "00",
  "CH2_Pitch", "01",
  "CH3_Pitch", "02",
  "Control", "03",
  NULL
};

const char** DivPlatformPV1000::getRegisterSheet() {
  return regCheatSheetPV1000;
}

void DivPlatformPV1000::acquire(short** buf, size_t len) {
  for (int i=0; i<3; i++) {
    oscBuf[i]->begin(len);
  }
  
  for (size_t h=0; h<len; h++) {
    short samp=d65010g031_sound_tick(&d65010g031,1);
    buf[0][h]=samp;
    for (int i=0; i<3; i++) {
      oscBuf[i]->putSample(h,MAX(d65010g031.out[i]<<2,0));
    }
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformPV1000::tick(bool sysTick) {
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol && chan[i].std.vol.val);
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
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
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=0x3f-parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>62) chan[i].freq=62;
      if (isMuted[i]) chan[i].keyOn=false;
      if (chan[i].keyOn) {
        rWrite(i,(isMuted[i] || (chan[i].outVol<=0)) ? 0x3f : chan[i].freq);
        chan[i].keyOn=false;
      } else if (chan[i].freqChanged && chan[i].active && !isMuted[i]) {
        rWrite(i,(isMuted[i] || (chan[i].outVol<=0)) ? 0x3f : chan[i].freq);
      }
      if (chan[i].keyOff) {
        rWrite(i,0x3f);
        chan[i].keyOff=false;
      }
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformPV1000::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_PV1000);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
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
          chan[c.chan].freqChanged=true;
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
    case DIV_CMD_STD_NOISE_MODE: // ring modulation
      if (c.value&1) {
        rWrite(3,3);
      } else {
        rWrite(3,2);
      }
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
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_PV1000));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformPV1000::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (mute) {
    chan[ch].keyOff=true;
  } else if (chan[ch].active) {
    chan[ch].keyOn=true;
    chan[ch].freqChanged=true;
  }
}

void DivPlatformPV1000::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

void* DivPlatformPV1000::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformPV1000::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformPV1000::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformPV1000::getRegisterPool() {
  return regPool;
}

int DivPlatformPV1000::getRegisterPoolSize() {
  return 4;
}

void DivPlatformPV1000::reset() {
  memset(regPool,0,4);
  for (int i=0; i<3; i++) {
    chan[i]=Channel();
    chan[i].std.setEngine(parent);
  }
  d65010g031_reset(&d65010g031);
  // mute
  rWrite(0,0x3f);
  rWrite(1,0x3f);
  rWrite(2,0x3f);
  rWrite(3,2);
}

int DivPlatformPV1000::getOutputCount() {
  return 1;
}

void DivPlatformPV1000::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPV1000::setFlags(const DivConfig& flags) {
  chipClock=COLOR_NTSC*5.0;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/1024;
  for (int i=0; i<3; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformPV1000::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformPV1000::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

bool DivPlatformPV1000::getDCOffRequired() {
  return true;
}

int DivPlatformPV1000::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformPV1000::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
}

DivPlatformPV1000::~DivPlatformPV1000() {
}
