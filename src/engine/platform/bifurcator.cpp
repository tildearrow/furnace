/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#define _USE_MATH_DEFINES
#include "bifurcator.h"
#include "../engine.h"
#include "../filter.h"
#include <math.h>

#define CHIP_FREQBASE 65536

#define rWrite(a,v) {if(!skipRegisterWrites) {regPool[a]=v; if(dumpWrites) addWrite(a,v); }}

const char* regCheatSheetBifurcator[]={
  "CHx_State", "x*8+0",
  "CHx_Param", "x*8+2",
  "CHx_Freq", "x*8+4",
  "CHx_LVol", "x*8+6",
  "CHx_RVol", "x*8+7",
  NULL
};

const char** DivPlatformBifurcator::getRegisterSheet() {
  return regCheatSheetBifurcator;
}

void DivPlatformBifurcator::acquire(short** buf, size_t len) {
  for (int i=0; i<4; i++) {
    chan[i].curx=regPool[i*8]|(regPool[i*8+1]<<8);
    chan[i].param=regPool[i*8+2]|(regPool[i*8+3]<<8);
    chan[i].freq=regPool[i*8+4]|(regPool[i*8+5]<<8);
    chan[i].chVolL=regPool[i*8+6];
    chan[i].chVolR=regPool[i*8+7];
  }
  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t h=0; h<len; h++) {
    int l=0;
    int r=0;
    for (int i=0; i<4; i++) {
      chan[i].audSub+=chan[i].freq;
      if (chan[i].audSub>=65536) {
        int64_t newx=(int64_t)chan[i].curx*(chan[i].param+65536)/32768;
        newx*=65536-chan[i].curx;
        chan[i].curx=(int)(newx/65536);
        chan[i].audSub&=65535;
      }
      int out=chan[i].curx-32768;
      int outL=out*chan[i].chVolL/256;
      int outR=out*chan[i].chVolR/256;
      oscBuf[i]->putSample(h,(short)((outL+outR)/2));
      l+=outL/4;
      r+=outR/4;
    }
    buf[0][h]=(short)l;
    buf[1][h]=(short)r;
  }
  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
  for (int i=0; i<4; i++) {
    regPool[i*8]=chan[i].curx&0xff;
    regPool[i*8+1]=chan[i].curx>>8;
  }
}

void DivPlatformBifurcator::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol*MIN(chan[i].std.vol.val,255))/255;
      chan[i].volChangedL=true;
      chan[i].volChangedR=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      rWrite(i*8+2,chan[i].std.duty.val&0xff);
      rWrite(i*8+3,chan[i].std.duty.val>>8);
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
    if (chan[i].std.panL.had) {
      chan[i].chPanL=(255*(chan[i].std.panL.val&255))/255;
      chan[i].volChangedL=true;
    }

    if (chan[i].std.panR.had) {
      chan[i].chPanR=(255*(chan[i].std.panR.val&255))/255;
      chan[i].volChangedR=true;
    }
    if (chan[i].std.phaseReset.had) {
      if ((chan[i].std.phaseReset.val==1) && chan[i].active) {
        rWrite(i*8,1);
        rWrite(i*8+1,0);
      }
    }
    if (chan[i].std.ex1.had) {
      rWrite(i*8,chan[i].std.ex1.val&0xff);
      rWrite(i*8+1,chan[i].std.ex1.val>>8);
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq>65535) chan[i].freq=65535;
      rWrite(i*8+4,chan[i].freq&0xff);
      rWrite(i*8+5,chan[i].freq>>8);
      if (chan[i].keyOn) {
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
        }
        chan[i].volChangedL=true;
        chan[i].volChangedR=true;
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chan[i].volChangedL=true;
        chan[i].volChangedR=true;
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        chan[i].freqChanged=false;
      }
    }
    if (chan[i].volChangedL) {
      int vol=(isMuted[i] || !chan[i].active)?0:(chan[i].outVol*chan[i].chPanL/255);
      rWrite(i*8+6,vol);
      chan[i].volChangedL=false;
    }
    if (chan[i].volChangedR) {
      int vol=(isMuted[i] || !chan[i].active)?0:(chan[i].outVol*chan[i].chPanR/255);
      rWrite(i*8+7,vol);
      chan[i].volChangedR=false;
    }
  }
}

int DivPlatformBifurcator::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_BIFURCATOR);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value));
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
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
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      chan[c.chan].volChangedL=true;
      chan[c.chan].volChangedR=true;
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].chPanL=c.value;
      chan[c.chan].chPanR=c.value2;
      chan[c.chan].volChangedL=true;
      chan[c.chan].volChangedR=true;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
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
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_BIFURCATOR_STATE_LOAD:
      rWrite(c.chan*8+c.value,c.value2);
      break;
    case DIV_CMD_BIFURCATOR_PARAMETER:
      rWrite(c.chan*8+2+c.value,c.value2);
      break;
    case DIV_CMD_GET_VOLMAX:
      return 255;
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

void DivPlatformBifurcator::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volChangedL=true;
  chan[ch].volChangedR=true;
}

void DivPlatformBifurcator::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].volChangedL=true;
    chan[i].volChangedR=true;
    chan[i].active=false;
  }
}

void* DivPlatformBifurcator::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformBifurcator::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformBifurcator::getRegisterPool() {
  return (unsigned char*)regPool;
}

int DivPlatformBifurcator::getRegisterPoolSize() {
  return 8*4;
}

void DivPlatformBifurcator::reset() {
  memset(regPool,0,8*4);
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformBifurcator::Channel();
    chan[i].std.setEngine(parent);
    rWrite(i*8,chan[i].curx&0xff);
    rWrite(i*8+1,chan[i].curx>>8);
    rWrite(i*8+2,chan[i].param&0xff);
    rWrite(i*8+3,chan[i].param>>8);
  }
}

int DivPlatformBifurcator::getOutputCount() {
  return 2;
}

bool DivPlatformBifurcator::hasSoftPan(int ch) {
  return true;
}

DivMacroInt* DivPlatformBifurcator::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformBifurcator::getPan(int ch) {
  return (chan[ch].chPanL<<8)|(chan[ch].chPanR);
}

void DivPlatformBifurcator::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformBifurcator::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformBifurcator::setFlags(const DivConfig& flags) {
  chipClock=1000000;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/16;
  for (int i=0; i<4; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformBifurcator::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformBifurcator::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformBifurcator::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformBifurcator::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
}
