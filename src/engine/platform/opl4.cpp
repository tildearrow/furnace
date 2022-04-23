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

#include "opl4.h"
#include "../engine.h"
#include <math.h>

#define immWrite(a,v) if (!skipRegisterWrites) { if (a >= 0x200) { chip.writeReg(a & 0xff, v); regPool[a] = v; } if (dumpWrites) { addWrite(a, v); } }

#define ADDR_WT 0x208
#define ADDR_FREQL 0x220
#define ADDR_FREQH 0x238
#define ADDR_TL 0x250
#define ADDR_KEY_PAN 0x268
#define ADDR_LFO_VIB 0x280
#define ADDR_AR_D1R 0x298
#define ADDR_DL_D2R 0x2B0
#define ADDR_RC_RR 0x2C8
#define ADDR_AM 0x2E0

byte DivOPL4MemoryInterface::operator[](unsigned address) const {
  if (parent && parent->opl4WaveMem && address < parent->opl4WaveMemLen) {
    return parent->opl4WaveMem[address];
  }
  return 0;
}

void DivPlatformOPL4::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  chip.generate(bufL + start, bufR + start, len);
}

void DivPlatformOPL4::tick(bool sysTick) {
  for (int i=0; i<24; i++) {
    if (chan[i].keyOff) {
      immWrite(i+ADDR_KEY_PAN,0x00);
      chan[i].keyOff=false;
    }
  }
  for (int i=0; i<24; i++) {
    if (chan[i].freqChanged) {
      const unsigned freq = parent->calcFreq(chan[i].baseFreq,
        chan[i].pitch, false, 1 << (toOctave(chan[i].baseFreq) + 8));
      // const unsigned freq = chan[i].baseFreq;
      const int octave = toOctave(freq);
      const int fnumber = toFreq(freq);
      chan[i].freqH = fnumber << 1;
      chan[i].freqL = octave << 4 | fnumber >> 7;
      immWrite(i+ADDR_FREQL,chan[i].freqH);
      immWrite(i+ADDR_FREQH,chan[i].freqL);
    }
    if (chan[i].keyOn) {
      if (chan[i].insChanged) {
        immWrite(i+ADDR_WT,chan[i].ins & 0xff);
      }
      immWrite(i+ADDR_TL,~chan[i].vol << 1 & 0xff | 0x01);
      immWrite(i+ADDR_KEY_PAN,0x80);
      chan[i].keyOn=false;
    } else {
      immWrite(i+ADDR_TL,~chan[i].vol << 1 & 0xff);
    }
    chan[i].freqChanged=false;
  }
}

int clampFreq(unsigned freq) {
  return MIN(MAX(freq, 0x400), 0x1ffffff);
}

int bit_width(unsigned value) {
  int x = 0;
  while (value) {
    value >>= 1;
    ++x;
  }
  return x;
}

int DivPlatformOPL4::toOctave(int freq) {
  return bit_width(clampFreq(freq)) - 18;
}

int DivPlatformOPL4::toFreq(int freq) {
  freq = clampFreq(freq);
  const int shift = bit_width(freq) - 11;
  return freq >> shift & 0x3ff;  // todo: round
}

// void* DivPlatformOPL4::getChanState(int chan) {
//   return NULL;
// }

unsigned char* DivPlatformOPL4::getRegisterPool() {
  return regPool;
}

int DivPlatformOPL4::getRegisterPoolSize() {
  return 0x300;
}

// void DivPlatformOPL4::muteChannel(int ch, bool mute) {
// }

double DivPlatformOPL4::calcBaseFreq(int ch, int note) {
  double off = 1.0;
  const int sample = parent->getIns(chan[ch].ins,DIV_INS_MULTIPCM)->multipcm.initSample;
  DivSample* s = parent->getSample(sample);
  if (s->centerRate > 0) {
    off = s->centerRate / 44100.0;
  }
  return off * (parent->song.tuning / 440.0) * pow(2.0, note / 12.0 + 14.0);
  // return parent->calcBaseFreq(44100 * 440, s->centerRate * 0x4000, note - 3, false);
}

int DivPlatformOPL4::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=calcBaseFreq(c.chan, c.value);
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF: {
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      break;
    }
    case DIV_CMD_NOTE_OFF_ENV: {
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    }
    case DIV_CMD_ENV_RELEASE: {
      chan[c.chan].std.release();
      break;
    }
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT: {
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    }
    case DIV_CMD_PANNING: {
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=calcBaseFreq(c.chan, c.value);
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      break;
    }
    case DIV_CMD_FM_FB: {
      break;
    }
    case DIV_CMD_FM_MULT: {
      break;
    }
    case DIV_CMD_FM_TL: {
      break;
    }
    case DIV_CMD_FM_AR: {
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      break;
    }
    case DIV_ALWAYS_SET_VOLUME: {
      return 0;
      break;
    }
    case DIV_CMD_GET_VOLMAX: {
      return 127;
      break;
    }
    case DIV_CMD_PRE_PORTA: {
      break;
    }
    case DIV_CMD_PRE_NOTE: {
      break;
    }
    default: {
      printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
    }
  }
  return 1;
}

void DivPlatformOPL4::reset() {
  memset(getRegisterPool(),0,getRegisterPoolSize());
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  for (int i=0; i<24; i++) {
    chan[i]=DivPlatformOPL4::Channel();
    chan[i].std.setEngine(parent);
  }

  immWrite(0x105,3);  // enable OPL4 features
  immWrite(0x202,0);  // set memory config
}

bool DivPlatformOPL4::isStereo() {
  return true;
}

// bool DivPlatformOPL4::keyOffAffectsArp(int ch) {
//   return false;
// }

// bool DivPlatformOPL4::keyOffAffectsPorta(int ch) {
//   return false;
// }

// int DivPlatformOPL4::getPortaFloor(int ch) {
//   return 0x00;
// }

// float DivPlatformOPL4::getPostAmp() {
//   return 1.0f;
// }

// const char* DivPlatformOPL4::getEffectName(unsigned char effect) {
//   return NULL;
// }

void DivPlatformOPL4::setFlags(unsigned int flags) {
  chipClock = 33868800;
  rate = chipClock / 768;  // 44100 Hz
}

// void DivPlatformOPL4::setSkipRegisterWrites(bool value) {
//   skipRegisterWrites=value;
// }

void DivPlatformOPL4::notifyInsChange(int ins) {
}

// void DivPlatformOPL4::notifyWaveChange(int ins) {

// }

void DivPlatformOPL4::notifyInsDeletion(void* ins) {
}

// void DivPlatformOPL4::notifyPlaybackStop() {

// }

// void DivPlatformOPL4::forceIns() {
  
// }

void DivPlatformOPL4::toggleRegisterDump(bool enable) {
  dumpWrites=enable;
}

void DivPlatformOPL4::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformOPL4::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

// const char** DivPlatformOPL4::getRegisterSheet() {
//   return NULL;
// }

int DivPlatformOPL4::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  rom.parent=parent;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<24; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();
  return 24;
}

// void DivPlatformOPL4::quit() {
// }

DivPlatformOPL4::DivPlatformOPL4() : rom(0x200000), ram(0x200000), chip(rom, ram) {
}

DivPlatformOPL4::~DivPlatformOPL4() {
}
