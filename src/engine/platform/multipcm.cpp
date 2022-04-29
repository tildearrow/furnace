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

#include "multipcm.h"
#include "../engine.h"
#include <math.h>

#define ADDR_MPCM_PAN 0
#define ADDR_MPCM_WT 1
#define ADDR_MPCM_FREQL 2
#define ADDR_MPCM_FREQH 3
#define ADDR_MPCM_KEY 4
#define ADDR_MPCM_TL 5
#define ADDR_MPCM_LFO_VIB 6
#define ADDR_MPCM_AM 7

#define ADDR_OPL4_WT 0x208
#define ADDR_OPL4_FREQL 0x220
#define ADDR_OPL4_FREQH 0x238
#define ADDR_OPL4_TL 0x250
#define ADDR_OPL4_KEY_PAN 0x268
#define ADDR_OPL4_LFO_VIB 0x280
#define ADDR_OPL4_AR_D1R 0x298
#define ADDR_OPL4_DL_D2R 0x2B0
#define ADDR_OPL4_RC_RR 0x2C8
#define ADDR_OPL4_AM 0x2E0

byte DivYMF278MemoryInterface::operator[](unsigned address) const {
  if (parent && parent->opl4PCMMem && address < parent->opl4PCMMemLen) {
    return parent->opl4PCMMem[address];
  }
  return 0;
}

void DivPlatformYMF278::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    generate(bufL[h], bufR[h]);
  }
}

void DivPlatformYMF278::tick(bool sysTick) {
  for (int i = 0; i < channelCount; i++) {
    Channel& ch = chan[i];
    ch.std.next();

    int basePitch = ch.basePitch;
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
      if (ch.std.arp.mode) {
        basePitch += (ch.std.arp.val << 7) - (ch.note << 7);
      } else {
        basePitch += ch.std.arp.val << 7;
      }
      ch.freqChanged = true;
    } else if (ch.std.arp.mode && ch.std.arp.finished) {
      ch.freqChanged = true;
    }

    if (ch.std.pitch.had) {
      ch.freqChanged = true;
    }

    if (ch.freqChanged) {
      ch.freq = calcFreq(basePitch + ch.pitch + ch.std.pitch.val + ch.pitchOffset);
      ch.freqChanged = false;
    }

    int panL = ch.std.panL.has ? ch.std.panL.val : ch.panL;
    int panR = ch.std.panR.has ? ch.std.panR.val : ch.panR;
    ch.pan = MIN(MAX(panR - panL, -7), 7);

    tickWrite(i, ch, vol);
  }
}

int DivPlatformYMF278::dispatch(DivCommand c) {
  if (c.chan>=channelCount) return 0;
  Channel& ch = chan[c.chan];
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.value != DIV_NOTE_NULL) {
        ch.note = c.value;
        ch.basePitch = c.value << 7;
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
      float octaveOffset = log2f(parent->song.tuning) - log2f(440.0f);
      if (s->centerRate > 0) {
        octaveOffset += log2f(s->centerRate) - log2f(44100.0f);
      }
      ch.pitchOffset = roundf((octaveOffset - 3.0f) * (12.0f * 128.0f));
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
      int destPitch = c.value2 << 7;
      int newPitch;
      bool return2 = false;
      if (destPitch > ch.basePitch) {
        newPitch = ch.basePitch + c.value;
        if (newPitch >= destPitch) {
          newPitch = destPitch;
          return2 = true;
        }
      } else {
        newPitch = ch.basePitch - c.value;
        if (newPitch <= destPitch) {
          newPitch = destPitch;
          return2 = true;
        }
      }
      ch.basePitch = newPitch;
      ch.freqChanged = true;
      if (return2) {
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      ch.note = c.value;
      ch.basePitch = c.value << 7;
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
    default: {
      // printf("WARNING: unimplemented command %d\n", c.cmd);
      break;
    }
  }
  return 1;
}

void DivPlatformYMF278::muteChannel(int ch, bool mute) {
  chan[ch].isMuted = mute;
}

// void DivPlatformYMF278::forceIns() {
  
// }

void DivPlatformYMF278::notifyInsChange(int ins) {
  for (int i = 0; i < channelCount; i++) {
    if (chan[i].ins == ins) {
      chan[i].insChanged = true;
    }
  }
}

void DivPlatformYMF278::notifyInsDeletion(void* ins) {
  for (int i = 0; i < channelCount; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

int DivPlatformYMF278::calcFreq(int basePitch) {
  float baseOctave = basePitch / (12.0f * 128.0f);
  float octave = floorf(baseOctave);
  float fnumber = roundf((powf(2.0f, baseOctave - octave) - 1.0f) * 1024.0f);
  return MIN(MAX((int)octave << 10 | (int)fnumber, -0x1C00), 0x1fff);
}

const char* DivPlatformYMF278::getEffectName(unsigned char effect) {
  return NULL;
}

bool DivPlatformYMF278::isStereo() {
  return true;
}

bool DivPlatformYMF278::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformYMF278::keyOffAffectsPorta(int ch) {
  return false;
}

void* DivPlatformYMF278::getChanState(int ch) {
  return &chan[ch];
}

int DivPlatformYMF278::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent = p;
  dumpWrites = false;
  skipRegisterWrites = false;
  setFlags(flags);
  reset();
  return channelCount;
}

void DivPlatformYMF278::setFlags(unsigned int flags) {
  chipClock = 33868800;
  rate = chipClock / 768;  // 44100 Hz
}

void DivPlatformYMF278::reset() {
  memset(getRegisterPool(), 0, getRegisterPoolSize());
  if (dumpWrites) {
    addWrite(0xffffffff, 0);
  }

  for (int i = 0; i < channelCount; i++) {
    bool muted = chan[i].isMuted;
    chan[i] = DivPlatformYMF278::Channel();
    chan[i].std.setEngine(parent);
    chan[i].isMuted = muted;
  }
}

void DivPlatformYMF278::quit() {
}

void DivPlatformMultiPCM::reset() {
  DivPlatformYMF278::reset();
  memory.parent = parent;
  chip.reset();
}

unsigned char* DivPlatformMultiPCM::getRegisterPool() {
  return regPool;
}

int DivPlatformMultiPCM::getRegisterPoolSize() {
  return 0x100;
}

void DivPlatformMultiPCM::poke(unsigned int addr, unsigned short val) {
  immWrite(addr & 0x1f, addr >> 3, val);
}

void DivPlatformMultiPCM::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i : wlist) {
    immWrite(i.addr & 0x1f, i.addr >> 3, i.val);
  }
}

void DivPlatformMultiPCM::tickWrite(int i, DivPlatformYMF278::Channel& ch, int vol) {
  if (ch.keyOn) {
    immWrite(i, ADDR_MPCM_KEY, 0x00);
  }

  unsigned char octave = ch.freq >> 10 << 4;
  unsigned char fnumL = (ch.freq & 0x3ff) << 2;
  unsigned char fnumH = (ch.freq & 0x3ff) >> 6;
  unsigned char insH = ch.ins >> 8 & 1;
  immWrite(i, ADDR_MPCM_FREQL, fnumL | insH);
  immWrite(i, ADDR_MPCM_FREQH, octave | fnumH);

  if (ch.insChanged) {
    unsigned char insL = ch.ins & 0xff;
    immWrite(i, ADDR_MPCM_WT, insL);
    ch.insChanged = false;
  }

  unsigned char tl = ~vol << 1;
  unsigned char tlDirect = ch.keyOn ? 0x01 : 0x00;
  immWrite(i, ADDR_MPCM_TL, tl | tlDirect);

  unsigned char pan = ch.isMuted ? 0x80 : ch.pan << 4;
  immWrite(i, ADDR_MPCM_PAN, pan);

  unsigned char key = ch.key ? 0x80 : 0x00;
  immWrite(i, ADDR_MPCM_KEY, key);

  ch.keyOn = false;
}

void DivPlatformMultiPCM::immWrite(int ch, int reg, unsigned char v) {
  if (!skipRegisterWrites) {
    ch = ch * 8 / 7;
    int a = reg << 5 | ch;
    chip.writeReg(ch, reg, v);
    regPool[a] = v;
    if (dumpWrites) {
      addWrite(a, v);
    }
  }
}

void DivPlatformOPL4PCM::reset() {
  DivPlatformYMF278::reset();
  memory.parent = parent;
  chip.reset();
  immWrite(0x202, 0);  // set memory config
}

YMF278& DivPlatformOPL4PCM::getChip() {
  return chip;
}

void DivPlatformOPL4PCM::tickWrite(int i, DivPlatformYMF278::Channel& ch, int vol) {
  if (ch.keyOn) {
    immWrite(i+ADDR_OPL4_KEY_PAN, (immRead(i+ADDR_OPL4_KEY_PAN) & 0x3f) | 0x40);
  }

  unsigned char octave = ch.freq >> 10 << 4;
  unsigned char fnumL = (ch.freq & 0x3ff) << 1;
  unsigned char fnumH = (ch.freq & 0x3ff) >> 7;
  unsigned char sus = ch.sus ? 0x08 : 0x00;
  unsigned char insH = ch.ins >> 8 & 1;
  immWrite(i+ADDR_OPL4_FREQL, fnumL | insH);
  immWrite(i+ADDR_OPL4_FREQH, octave | sus | fnumH);

  if (ch.insChanged) {
    unsigned char insL = ch.ins & 0xff;
    immWrite(i+ADDR_OPL4_WT, insL);
    ch.insChanged = false;
  }

  unsigned char tl = ~vol << 1;
  unsigned char tlDirect = ch.keyOn ? 0x01 : 0x00;
  immWrite(i+ADDR_OPL4_TL, tl | tlDirect);

  unsigned char key = ch.key ? 0x80 : 0x00;
  unsigned char damp = ch.damp ? 0x40 : 0x00;
  unsigned char pan = ch.isMuted ? 0x08 : ch.pan & 0x0f;
  immWrite(i+ADDR_OPL4_KEY_PAN, key | damp | pan);

  ch.keyOn = false;
}

void DivPlatformOPL4PCM::immWrite(int a, unsigned char v) {
  if (!skipRegisterWrites) {
    if (a >= 0x200) {
      chip.writeReg(a & 0xff, v);
    }
    if (dumpWrites) {
      addWrite(a, v);
    }
  }
}

unsigned char DivPlatformOPL4PCM::immRead(int a) {
  return !skipRegisterWrites && a >= 0x200 ? chip.readReg(a & 0xff) : 0;
}
