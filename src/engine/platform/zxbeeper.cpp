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

#include "zxbeeper.h"
#include "../engine.h"
#include <math.h>

#define CHIP_FREQBASE 8192*6

const char** DivPlatformZXBeeper::getRegisterSheet() {
  return NULL;
}

void DivPlatformZXBeeper::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  bool o=false;
  for (size_t h=start; h<start+len; h++) {
    // clock here
    if (curSample>=0 && curSample<parent->song.sampleLen) {
      if (--curSamplePeriod<0) {
        DivSample* s=parent->getSample(curSample);
        if (s->samples>0) {
          sampleOut=(s->data8[curSamplePos++]>0);
          if (curSamplePos>=s->samples) curSample=-1;
          // 256 bits
          if (curSamplePos>2047) curSample=-1;
          
          curSamplePeriod=15;
        } else {
          curSample=-1;
        }
      }
      o=sampleOut;
      bufL[h]=o?16384:0;
      oscBuf[0]->data[oscBuf[0]->needle++]=o?16384:-16384;
      continue;
    }

    unsigned short oldPos=chan[curChan].sPosition;
    o=false;

    if (sOffTimer) {
      sOffTimer--;
      o=true;
    }
    chan[curChan].sPosition+=chan[curChan].freq;
    if (oldPos>chan[curChan].sPosition) {
      if (!isMuted[curChan] && chan[curChan].outVol) sOffTimer+=chan[curChan].duty;
    }
    if (++curChan>=6) curChan=0;
    
    bufL[h]=o?16384:0;
    oscBuf[0]->data[oscBuf[0]->needle++]=o?16384:-16384;
  }
}

void DivPlatformZXBeeper::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&1)*MIN(1,chan[i].std.vol.val));
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
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
      if (chan[i].active) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
        if (chan[i].freq>65535) chan[i].freq=65535;
      }
      if (chan[i].keyOn) {
        //rWrite(16+i*5,0x80);
        //chWrite(i,0x04,0x80|chan[i].vol);
      }
      if (chan[i].keyOff) {
        chan[i].freq=0;
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformZXBeeper::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_BEEPER);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
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
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=c.value;
      break;
    case DIV_CMD_SAMPLE_MODE:
      if (isMuted[c.chan]) break;
      curSample=c.value;
      curSamplePos=0;
      curSamplePeriod=0;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BEEPER));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
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
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformZXBeeper::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformZXBeeper::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

void* DivPlatformZXBeeper::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformZXBeeper::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformZXBeeper::getOscBuffer(int ch) {
  return (ch<1)?oscBuf[ch]:NULL;
}

unsigned char* DivPlatformZXBeeper::getRegisterPool() {
  ulaOut=sOffTimer?0x10:0x08;
  return &ulaOut;
}

int DivPlatformZXBeeper::getRegisterPoolSize() {
  return 1;
}

void DivPlatformZXBeeper::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,128);
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformZXBeeper::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  lastPan=0xff;
  memset(tempL,0,32*sizeof(int));
  memset(tempR,0,32*sizeof(int));
  cycles=0;
  curChan=0;
  sOffTimer=0;
  ulaOut=0;
  curSample=-1;
  curSamplePos=0;
  curSamplePeriod=0;
  sampleOut=false;
}

bool DivPlatformZXBeeper::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformZXBeeper::notifyWaveChange(int wave) {
}

void DivPlatformZXBeeper::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformZXBeeper::setFlags(const DivConfig& flags) {
  // TODO: where's ZX Spectrum 48K?!
  if (flags.getInt("clockSel",0)) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/4;
  for (int i=0; i<6; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformZXBeeper::poke(unsigned int addr, unsigned short val) {

}

void DivPlatformZXBeeper::poke(std::vector<DivRegWrite>& wlist) {

}

int DivPlatformZXBeeper::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformZXBeeper::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
}

DivPlatformZXBeeper::~DivPlatformZXBeeper() {
}
