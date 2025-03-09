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

#include "sm8521.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 64

const char* regCheatSheetSM8521[]={
  "SGC", "40",
  "SG0L", "42",
  "SG1L", "44",
  "SG0TL", "46",
  "SG0TH", "47",
  "SG1TL", "48",
  "SG1TH", "49",
  "SG2L", "4A",
  "SG2TL", "4C",
  "SG2TH", "4D",
  "SGDA", "4E",
  "SG0Wn", "60+n",
  "SG1Wn", "70+n",
  NULL
};

const char** DivPlatformSM8521::getRegisterSheet() {
  return regCheatSheetSM8521;
}

void DivPlatformSM8521::acquire(short** buf, size_t len) {
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    sm8521_write(&sm8521,w.addr,w.val);
    regPool[w.addr&0xff]=w.val;
    writes.pop();
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    sm8521_sound_tick(&sm8521,coreQuality);
    buf[0][h]=sm8521.out<<6;
    for (int i=0; i<2; i++) {
      oscBuf[i]->putSample(h,sm8521.sg[i].base.out<<7);
    }
    oscBuf[2]->putSample(h,sm8521.noise.base.out<<7);
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSM8521::updateWave(int ch) {
  if (ch<2) {
    const unsigned char temp=regPool[0x40];
    rWrite(0x40,temp&~(1<<ch));
    for (int i=0; i<16; i++) {
      int nibble1=(chan[ch].ws.output[((i<<1)+chan[ch].antiClickWavePos-1)&31]-8)&0xf;
      int nibble2=(chan[ch].ws.output[((1+(i<<1))+chan[ch].antiClickWavePos-1)&31]-8)&0xf;
      rWrite(0x60+i+(ch*16),(nibble2<<4)|nibble1);
    }
    if (chan[ch].active) {
      rWrite(0x40,temp|(1<<ch));
    }
    chan[ch].antiClickWavePos&=31;
  }
}

void DivPlatformSM8521::tick(bool sysTick) {
  unsigned char keyState=0x80;
  for (int i=0; i<3; i++) {
    // anti-click
    /*
    if (antiClickEnabled && sysTick && chan[i].freq>0) {
      chan[i].antiClickPeriodCount+=(chipClock/MAX(parent->getCurHz(),1.0f));
      chan[i].antiClickWavePos+=chan[i].antiClickPeriodCount/chan[i].freq;
      chan[i].antiClickPeriodCount%=chan[i].freq;
    }
    */

    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&31)*MIN(31,chan[i].std.vol.val))>>5;
      if (!isMuted[i]) {
        chan[i].volumeChanged=true;
      }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].volumeChanged) {
      if (isMuted[i]) {
        rWrite(volMap[i],0);
      } else {
        rWrite(volMap[i],chan[i].outVol&0x1f);
      }
      chan[i].volumeChanged=false;
    }
    if (chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
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
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      chan[i].antiClickWavePos=0;
      chan[i].antiClickPeriodCount=0;
    }
    if (chan[i].active) {
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1)) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
      if (chan[i].freq<1) chan[i].freq=1;
      if (chan[i].freq>4095) chan[i].freq=4095;
      rWrite(freqMap[i][0],chan[i].freq>>8);
      rWrite(freqMap[i][1],chan[i].freq&0xff);
      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff) {
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
    if (!isMuted[i] && chan[i].active) {
      keyState|=(1<<i);
    } else {
      keyState&=~(1<<i);
    }
  }
  if (regPool[0x40]!=keyState) {
    rWrite(0x40,keyState);
    regPool[0x40]=keyState;
  }
}

int DivPlatformSM8521::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SM8521);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }
      chan[c.chan].ws.init(ins,32,15,chan[c.chan].insChanged);
      chan[c.chan].insChanged=false;
      if (!isMuted[c.chan]) {
        chan[c.chan].volumeChanged=true;
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          if (!isMuted[c.chan]) {
            chan[c.chan].volumeChanged=true;
          }
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
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value*((parent->song.linearPitch==2)?1:8);
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value*((parent->song.linearPitch==2)?1:8);
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
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SM8521));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 31;
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

void DivPlatformSM8521::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volumeChanged=true;
  if (mute) {
    rWrite(0x40,regPool[0x40]&~(1<<ch));
  } else if (chan[ch].active) {
    rWrite(0x40,regPool[0x40]|0x80|(1<<ch));
  }
}

void DivPlatformSM8521::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
  }
}

void* DivPlatformSM8521::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSM8521::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSM8521::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSM8521::getRegisterPool() {
  return regPool;
}

int DivPlatformSM8521::getRegisterPoolSize() {
  return 256;
}

void DivPlatformSM8521::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,256);
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformSM8521::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,15,false);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  sm8521_reset(&sm8521);
  rWrite(0x40,0x80); // initialize SGC
}

int DivPlatformSM8521::getOutputCount() {
  return 1;
}

bool DivPlatformSM8521::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformSM8521::notifyWaveChange(int wave) {
  for (int i=0; i<2; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformSM8521::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSM8521::setFlags(const DivConfig& flags) {
  chipClock=11059200;
  CHECK_CUSTOM_CLOCK;
  antiClickEnabled=!flags.getBool("noAntiClick",false);
  rate=chipClock/4/coreQuality; // CKIN -> fCLK(/2) -> Function blocks (/2)
  for (int i=0; i<3; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformSM8521::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSM8521::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformSM8521::setCoreQuality(unsigned char q) {
  switch (q) {
    case 0:
      coreQuality=64;
      break;
    case 1:
      coreQuality=32;
      break;
    case 2:
      coreQuality=16;
      break;
    case 3:
      coreQuality=8;
      break;
    case 4:
      coreQuality=4;
      break;
    case 5:
      coreQuality=1;
      break;
    default:
      coreQuality=8;
      break;
  }
}

int DivPlatformSM8521::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformSM8521::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
}

DivPlatformSM8521::~DivPlatformSM8521() {
}
