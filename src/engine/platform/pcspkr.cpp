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

#include "pcspkr.h"
#include "../engine.h"
#include <math.h>

#define PCSPKR_DIVIDER 4
#define CHIP_DIVIDER 1

const char* regCheatSheetPCSpeaker[]={
  "Period", "0",
  NULL
};

const char** DivPlatformPCSpeaker::getRegisterSheet() {
  return regCheatSheetPCSpeaker;
}

const char* DivPlatformPCSpeaker::getEffectName(unsigned char effect) {
  return NULL;
}

void DivPlatformPCSpeaker::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    if (on) {
      pos-=PCSPKR_DIVIDER;
      while (pos<0) {
        if (freq<1) {
          pos=1;
        } else {
          pos+=flip?(freq>>1):((freq+1)>>1);
        }
        flip=!flip;
      }
      bufL[i]=(flip && !isMuted[0])?32767:0;
    } else {
      bufL[i]=0;
    }
  }
}

void DivPlatformPCSpeaker::tick() {
  for (int i=0; i<1; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      // ok, why are the volumes like that?
      chan[i].outVol=chan[i].vol;
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp);
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true)-1;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].keyOn) {
        on=true;
      }
      if (chan[i].keyOff) {
        on=false;
      }
      freq=chan[i].freq;
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformPCSpeaker::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
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
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) {
          on=chan[c.chan].vol;
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
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp):(0)));
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

void DivPlatformPCSpeaker::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformPCSpeaker::forceIns() {
  for (int i=0; i<1; i++) {
    chan[i].insChanged=true;
  }
}

void* DivPlatformPCSpeaker::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformPCSpeaker::getRegisterPool() {
  return regPool;
}

int DivPlatformPCSpeaker::getRegisterPoolSize() {
  return 2;
}

void DivPlatformPCSpeaker::reset() {
  for (int i=0; i<1; i++) {
    chan[i]=DivPlatformPCSpeaker::Channel();
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  on=false;
  freq=0;
  pos=0;
  flip=false;

  memset(regPool,0,2);
}

bool DivPlatformPCSpeaker::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformPCSpeaker::setFlags(unsigned int flags) {
  chipClock=COLOR_NTSC/3.0;
  rate=chipClock/PCSPKR_DIVIDER;
}

void DivPlatformPCSpeaker::notifyInsDeletion(void* ins) {
  for (int i=0; i<1; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPCSpeaker::poke(unsigned int addr, unsigned short val) {
  // ???
}

void DivPlatformPCSpeaker::poke(std::vector<DivRegWrite>& wlist) {
  // ???
}

int DivPlatformPCSpeaker::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<1; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);

  reset();
  return 5;
}

void DivPlatformPCSpeaker::quit() {
}

DivPlatformPCSpeaker::~DivPlatformPCSpeaker() {
}
