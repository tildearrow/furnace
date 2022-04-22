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

#include "tia.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {tia.set(a,v); regPool[((a)-0x15)&0x0f]=v; if (dumpWrites) {addWrite(a,v);} }

const char* regCheatSheetTIA[]={
  "AUDC0", "15",
  "AUDC1", "16",
  "AUDF0", "17",
  "AUDF1", "18",
  "AUDV0", "19",
  "AUDV1", "1A",
  NULL
};

const char* DivPlatformTIA::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Select shape (0 to F)";
      break;
  }
  return NULL;
}

const char** DivPlatformTIA::getRegisterSheet() {
  return regCheatSheetTIA;
}

void DivPlatformTIA::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  tia.process(bufL+start,len);
}

unsigned char DivPlatformTIA::dealWithFreq(unsigned char shape, int base, int pitch) {
  if (base&0x80000000 && ((base&0x7fffffff)<32)) {
    return base&0x1f;
  }
  int bp=base+pitch;
  double mult=0.25*(parent->song.tuning*0.0625)*pow(2.0,double(768+bp)/(256.0*12.0));
  if (mult<0.5) mult=0.5;
  switch (shape) {
    case 1: // buzzy
      return ceil(31400/(30.6*mult))-1;
      break;
    case 2: // low buzzy
      return ceil(31400/(480*mult))-1;
      break;
    case 3: // flangy
      return ceil(31400/(60*mult))-1;
      break;
    case 4: case 5: // square
      return ceil(31400/(4.05*mult))-1;
      break;
    case 6: case 7: case 8: case 9: case 10: // pure buzzy/reedy/noise
      return ceil(31400/(63*mult))-1;
      break;
    case 12: case 13: // low square
      return round(31400/(4.0*3*mult))-1;
      break;
    case 14: case 15: // low pure buzzy/reedy
      return ceil(31400/(3*63*mult))-1;
      break;
  }
  return 0;
}

void DivPlatformTIA::tick(bool sysTick) {
  for (int i=0; i<2; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MIN(15,chan[i].std.vol.val)-(15-(chan[i].vol&15));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(0x19+i,0);
      } else {
        rWrite(0x19+i,chan[i].outVol&15);
      }
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=0x80000000|chan[i].std.arp.val;
        } else {
          chan[i].baseFreq=(chan[i].note+chan[i].std.arp.val)<<8;
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=chan[i].note<<8;
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.wave.had) {
      chan[i].shape=chan[i].std.wave.val&15;
      rWrite(0x15+i,chan[i].shape);
      chan[i].freqChanged=true;
    }
    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (chan[i].insChanged) {
        if (!chan[i].std.wave.will) {
          chan[i].shape=4;
          rWrite(0x15+i,chan[i].shape);
        }
        chan[i].insChanged=false;
      }
      chan[i].freq=dealWithFreq(chan[i].shape,chan[i].baseFreq,chan[i].pitch)+chan[i].std.pitch.val;
      if ((chan[i].shape==4 || chan[i].shape==5) && !(chan[i].baseFreq&0x80000000 && ((chan[i].baseFreq&0x7fffffff)<32))) {
        if (chan[i].baseFreq<39*256) {
          rWrite(0x15+i,6);
          chan[i].freq=dealWithFreq(6,chan[i].baseFreq,chan[i].pitch)+chan[i].std.pitch.val;
        } else if (chan[i].baseFreq<59*256) {
          rWrite(0x15+i,12);
          chan[i].freq=dealWithFreq(12,chan[i].baseFreq,chan[i].pitch)+chan[i].std.pitch.val;
        } else {
          rWrite(0x15+i,chan[i].shape);
        }
      }
      if (chan[i].freq>31) chan[i].freq=31;
      if (chan[i].keyOff) {
        rWrite(0x19+i,0);
      }
      rWrite(0x17+i,chan[i].freq);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformTIA::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_TIA);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=c.value<<8;
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      rWrite(0x15+c.chan,chan[c.chan].shape);
      chan[c.chan].std.init(ins);
      if (isMuted[c.chan]) {
        rWrite(0x19+c.chan,0);
      } else {
        rWrite(0x19+c.chan,chan[c.chan].vol&15);
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (isMuted[c.chan]) {
        rWrite(0x19+c.chan,0);
      } else {
        rWrite(0x19+c.chan,chan[c.chan].vol&15);
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
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=c.value2<<8;
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
      chan[c.chan].baseFreq=c.value<<8;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_WAVE:
      chan[c.chan].shape=c.value&15;
      rWrite(0x15+c.chan,chan[c.chan].shape);
      chan[c.chan].freqChanged=true;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_TIA));
      }
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

void DivPlatformTIA::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(0x19+ch,0);
  } else {
    if (chan[ch].active) rWrite(0x19+ch,chan[ch].outVol&15);
  }
}

void DivPlatformTIA::forceIns() {
  for (int i=0; i<2; i++) {
    chan[i].insChanged=true;
    if (chan[i].active) {
      chan[i].freqChanged=true;
      if (!isMuted[i]) rWrite(0x19+i,chan[i].outVol&15);
    }
  }
}

void* DivPlatformTIA::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformTIA::getRegisterPool() {
  return regPool;
}

int DivPlatformTIA::getRegisterPoolSize() {
  return 6;
}

void DivPlatformTIA::reset() {
  tia.reset();
  memset(regPool,0,16);
  for (int i=0; i<2; i++) {
    chan[i]=DivPlatformTIA::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x0f;
  }
}

bool DivPlatformTIA::isStereo() {
  return false;
}

bool DivPlatformTIA::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformTIA::notifyInsDeletion(void* ins) {
  for (int i=0; i<2; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformTIA::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformTIA::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformTIA::setFlags(unsigned int flags) {
  if (flags&1) {
    rate=31250;
  } else {
    rate=31468;
  }
  chipClock=rate;
}

int DivPlatformTIA::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<2; i++) {
    isMuted[i]=false;
  }
  tia.channels(1,false);
  setFlags(flags);
  reset();
  return 2;
}

void DivPlatformTIA::quit() {
}
