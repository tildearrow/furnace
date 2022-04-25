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
#define immRead(a) (!skipRegisterWrites && a >= 0x200 ? chip.readReg(a & 0xff) : 0)

#define CHANNEL_COUNT 24

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
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Channel& ch = chan[i];
    ch.std.next();

    int vol = ch.vol;

    if (ch.std.phaseReset.had) {
      if (ch.std.phaseReset.val == 1) {
        ch.keyOn = true;
      }
    }

    if (ch.std.vol.will) {
      vol = MIN(MAX(vol + (ch.std.vol.val - 127), 0), 127);
    }

    if (ch.std.arp.had) {
      if (!ch.inPorta) {
        if (ch.std.arp.mode) {
          ch.baseFreq = calcBaseFreq(ch, ch.std.arp.val);
        } else {
          ch.baseFreq = calcBaseFreq(ch, ch.note + ch.std.arp.val);
        }
      }
      ch.freqChanged = true;
    } else if (ch.std.arp.mode && ch.std.arp.finished) {
      ch.baseFreq = calcBaseFreq(ch, ch.note);
      ch.freqChanged = true;
    }

    if (ch.std.pitch.had) {
      ch.freqChanged = true;
    }

    if (ch.freqChanged) {
      ch.freq = MIN(MAX(calcFreq(ch.baseFreq, ch.pitch) + ch.std.pitch.val, -0x1C00), 0x1fff);
      ch.freqChanged = false;
    }

    int panL = ch.std.panL.has ? ch.std.panL.val : ch.panL;
    int panR = ch.std.panR.has ? ch.std.panR.val : ch.panR;
    ch.pan = MIN(MAX(panR - panL, -7), 7);

    if (ch.keyOn) {
      immWrite(i+ADDR_KEY_PAN, immRead(i+ADDR_KEY_PAN) & 0x3f | 0x40);
    }

    unsigned char octave = ch.freq >> 10 << 4;
    unsigned char fnumL = (ch.freq & 0x3ff) << 1;
    unsigned char fnumH = (ch.freq & 0x3ff) >> 7;
    unsigned char sus = ch.sus ? 0x08 : 0x00;
    unsigned char insH = ch.ins >> 8 & 1;
    immWrite(i+ADDR_FREQL, fnumL | insH);
    immWrite(i+ADDR_FREQH, octave | sus | fnumH);

    if (ch.insChanged) {
      unsigned char insL = ch.ins & 0xff;
      immWrite(i+ADDR_WT, insL);
      ch.insChanged = false;
    }

    unsigned char tl = ~vol << 1;
    unsigned char tlDirect = ch.keyOn ? 0x01 : 0x00;
    immWrite(i+ADDR_TL, tl | tlDirect);

    unsigned char key = ch.key ? 0x80 : 0x00;
    unsigned char damp = ch.damp ? 0x40 : 0x00;
    unsigned char pan = isMuted[i] ? -8 : ch.pan & 0xf;
    immWrite(i+ADDR_KEY_PAN, key | damp | pan);

    ch.keyOn = false;
  }
}

int DivPlatformOPL4::dispatch(DivCommand c) {
  Channel& ch = chan[c.chan];
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.value != DIV_NOTE_NULL) {
        ch.note = c.value;
        ch.baseFreq = calcBaseFreq(ch, c.value);
        ch.freqChanged = true;
      }
      ch.key = true;
      ch.keyOn = true;
      ch.damp = false;
      ch.std.init(parent->getIns(ch.ins, DIV_INS_MULTIPCM));
      break;
    }
    case DIV_CMD_NOTE_OFF: {
      ch.key = false;
      ch.keyOn = false;
      ch.damp = true;
      ch.std.init(NULL);
      break;
    }
    case DIV_CMD_NOTE_OFF_ENV: {
      ch.key = false;
      ch.keyOn = false;
      ch.std.release();
      break;
    }
    case DIV_CMD_ENV_RELEASE: {
      ch.std.release();
      break;
    }
    case DIV_CMD_VOLUME: {
      ch.vol = c.value;
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return ch.vol;
      break;
    }
    case DIV_CMD_INSTRUMENT: {
      if (ch.ins != c.value || c.value2 == 1) {
        ch.ins = c.value;
        ch.insChanged = true;
      }
      int sample = parent->getIns(ch.ins, DIV_INS_MULTIPCM)->multipcm.initSample;
      DivSample* s = parent->getSample(sample);
      ch.freqOffset = parent->song.tuning / 440.0f;
      if (s->centerRate > 0) {
        ch.freqOffset *= s->centerRate / 44100.0f;
      }
      break;
    }
    case DIV_CMD_PANNING: {
      ch.panL = c.value >> 4;
      ch.panR = c.value & 15;
      break;
    }
    case DIV_CMD_PITCH: {
      ch.pitch = c.value;
      ch.freqChanged = true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq = calcBaseFreq(ch, c.value2);
      int newFreq;
      bool return2 = false;
      if (destFreq > ch.baseFreq) {
        newFreq = ch.baseFreq + c.value;
        if (newFreq >= destFreq) {
          newFreq = destFreq;
          return2 = true;
        }
      } else {
        newFreq = ch.baseFreq - c.value;
        if (newFreq <= destFreq) {
          newFreq = destFreq;
          return2 = true;
        }
      }
      ch.baseFreq = newFreq;
      ch.freqChanged = true;
      if (return2) {
        ch.inPorta = false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      ch.note = c.value;
      ch.baseFreq = calcBaseFreq(ch, c.value);
      ch.freqChanged = true;
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
      ch.inPorta = c.value;
      break;
    }
    default: {
      // printf("WARNING: unimplemented command %d\n", c.cmd);
      break;
    }
  }
  return 1;
}

void DivPlatformOPL4::muteChannel(int ch, bool mute) {
  isMuted[ch] = mute;
}

// void DivPlatformOPL4::forceIns() {
  
// }

void DivPlatformOPL4::notifyInsChange(int ins) {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    if (chan[i].ins == ins) {
      chan[i].insChanged = true;
    }
  }
}

void DivPlatformOPL4::notifyInsDeletion(void* ins) {
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

int bit_width(unsigned value) {
  int x = 0;
  while (value) {
    value >>= 1;
    ++x;
  }
  return x;
}

int DivPlatformOPL4::calcBaseFreq(Channel& ch, int note) {
  int baseFreq = ch.freqOffset * powf(2.0f, note / 12.0f + 14.0f);
  baseFreq = MIN(MAX(baseFreq, 0x400), 0x1ffc000);
  int shift = bit_width(baseFreq) - 11;
  int octave = (shift - 7) << 10;
  int fnumber = (baseFreq >> shift) & 0x3ff;
  return octave | fnumber;
}

int DivPlatformOPL4::calcFreq(int baseFreq, int pitch) {
  int normalizedFreq = (baseFreq & 0x3ff) | 0x400;
  int newFreq = parent->calcFreq(normalizedFreq, pitch, false, 4);
  while (newFreq >= 0x800) {
    newFreq >>= 1;
    baseFreq += 0x400;
  }
  while (newFreq < 0x400) {
    newFreq <<= 1;
    baseFreq -= 0x400;
  }
  return (baseFreq & ~0x3ff) | (newFreq & 0x3ff);
}

const char* DivPlatformOPL4::getEffectName(unsigned char effect) {
  return NULL;
}

bool DivPlatformOPL4::isStereo() {
  return true;
}

bool DivPlatformOPL4::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformOPL4::keyOffAffectsPorta(int ch) {
  return false;
}

unsigned char* DivPlatformOPL4::getRegisterPool() {
  return regPool;
}

int DivPlatformOPL4::getRegisterPoolSize() {
  return 0x300;
}

void DivPlatformOPL4::poke(unsigned int addr, unsigned short val) {
  immWrite(addr, val);
}

void DivPlatformOPL4::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i : wlist) {
    immWrite(i.addr, i.val);
  }
}

void* DivPlatformOPL4::getChanState(int ch) {
  return &chan[ch];
}

int DivPlatformOPL4::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent = p;
  rom.parent = parent;
  dumpWrites = false;
  skipRegisterWrites = false;
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    isMuted[i] = false;
  }
  setFlags(flags);
  reset();
  return CHANNEL_COUNT;
}

void DivPlatformOPL4::setFlags(unsigned int flags) {
  chipClock = 33868800;
  rate = chipClock / 768;  // 44100 Hz
}

void DivPlatformOPL4::reset() {
  memset(getRegisterPool(), 0, getRegisterPoolSize());
  if (dumpWrites) {
    addWrite(0xffffffff, 0);
  }

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    chan[i] = DivPlatformOPL4::Channel();
    chan[i].std.setEngine(parent);
  }

  chip.reset();
  immWrite(0x105, 3);  // enable OPL4 features
  immWrite(0x202, 0);  // set memory config
}

void DivPlatformOPL4::quit() {
}
