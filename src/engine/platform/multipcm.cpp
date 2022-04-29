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

    if (ch.key.changed || (ch.std.phaseReset.had && ch.std.phaseReset.val == 1)) {
      ch.state.key.set(ch.key.value > 0, ch.key.value > 0);
      ch.state.damp.set(ch.key.value < 0);
      ch.key.changed = false;
    }

    if (ch.ins.changed) {
      ch.state.ins.set(ch.ins.value);
      ch.ins.changed = false;
    }

    if (ch.note.changed || ch.pitch.changed || ch.porta.value || ch.pitchOffset.changed ||
        ch.std.pitch.had || ch.std.arp.had || (ch.std.arp.mode && ch.std.arp.finished)) {
      int basePitch = ch.note.value << 7;
      if (ch.std.arp.has || (ch.std.arp.will && !ch.std.arp.mode)) {
        if (ch.std.arp.mode) {
          basePitch = ch.std.arp.val << 7;
        } else {
          basePitch += ch.std.arp.val << 7;
        }
      }
      basePitch += ch.pitch.value + ch.std.pitch.val + ch.porta.value + ch.pitchOffset.value;
      ch.state.freq.set(calcFreq(basePitch));
      ch.note.changed = false;
      ch.pitch.changed = false;
      ch.porta.changed = false;
      ch.pitchOffset.changed = false;
    }

    if (ch.vol.changed || ch.std.vol.will) {
      int macroTL = ch.std.vol.will ? 0x7f - ch.std.vol.val : 0;
      ch.state.tl.set(MIN(MAX((0x7f - ch.vol.value) + macroTL, 0), 0x7f));
      ch.state.tlDirect.set(ch.state.key.changed && ch.state.key.value);
      ch.vol.changed = false;
    }

    if (ch.pan.changed || ch.muted.changed || ch.std.panL.had || ch.std.panR.had) {
      int panL = ch.std.panL.has ? ch.std.panL.val : ch.pan.value >> 4;
      int panR = ch.std.panR.has ? ch.std.panR.val : ch.pan.value & 15;
      ch.state.pan.set(ch.muted.value ? -8 : MIN(MAX(panR - panL, -7), 7));
      ch.pan.changed = false;
      ch.muted.changed = false;
    }

    writeChannelState(i, ch.state);
  }
}

int DivPlatformYMF278::dispatch(DivCommand c) {
  if (c.chan>=channelCount) return 0;
  Channel& ch = chan[c.chan];
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.value != DIV_NOTE_NULL) {
        ch.note.set(c.value);
        ch.porta.set(0);
      }
      ch.key.set(1, true);
      ch.std.init(parent->getIns(ch.ins.value, DIV_INS_MULTIPCM));
      break;
    }
    case DIV_CMD_NOTE_OFF: {
      ch.key.set(-1);
      ch.std.init(NULL);
      break;
    }
    case DIV_CMD_NOTE_OFF_ENV: {
      ch.key.set(0);
      ch.std.release();
      break;
    }
    case DIV_CMD_ENV_RELEASE: {
      ch.std.release();
      break;
    }
    case DIV_CMD_VOLUME: {
      ch.vol.set(c.value);
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return ch.vol.value;
      break;
    }
    case DIV_CMD_INSTRUMENT: {
      ch.ins.set(c.value, c.value2 == 1);
      if (ch.ins.changed) {
        DivInstrument* ins = parent->getIns(ch.ins.value, DIV_INS_MULTIPCM);
        DivSample* s = parent->getSample(ins->multipcm.initSample);
        float octaveOffset = log2f(parent->song.tuning) - log2f(440.0f);
        if (s->centerRate > 0) {
          octaveOffset += log2f(s->centerRate) - log2f(44100.0f);
        }
        ch.pitchOffset.set(roundf((octaveOffset - 3.0f) * (12.0f * 128.0f)));
      }
      break;
    }
    case DIV_CMD_PANNING: {
      ch.pan.set(c.value);
      break;
    }
    case DIV_CMD_PITCH: {
      ch.pitch.set(c.value);
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int curPitch = ch.porta.value;
      int destPitch = (c.value2 - ch.note.value) << 7;
      int newPitch;
      bool return2 = false;
      if (destPitch > curPitch) {
        newPitch = curPitch + c.value;
        if (newPitch >= destPitch) {
          newPitch = destPitch;
          return2 = true;
        }
      } else {
        newPitch = curPitch - c.value;
        if (newPitch <= destPitch) {
          newPitch = destPitch;
          return2 = true;
        }
      }
      ch.porta.set(newPitch);
      if (return2) {
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      ch.note.set(c.value);
      ch.porta.set(0);
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
  chan[ch].muted.set(mute);
}

// void DivPlatformYMF278::forceIns() {
  
// }

void DivPlatformYMF278::notifyInsChange(int ins) {
  for (int i = 0; i < channelCount; i++) {
    if (chan[i].state.ins.value == ins) {
      chan[i].state.ins.changed = true;
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

void DivPlatformYMF278::reset() {
  memset(getRegisterPool(), 0, getRegisterPoolSize());
  if (dumpWrites) {
    addWrite(0xffffffff, 0);
  }

  for (int i = 0; i < channelCount; i++) {
    int muted = chan[i].muted.value;
    chan[i] = DivPlatformYMF278::Channel();
    chan[i].std.setEngine(parent);
    if (muted) {
      chan[i].muted.set(muted);
    }
  }
}

void DivPlatformYMF278::quit() {
}

void DivPlatformMultiPCM::reset() {
  DivPlatformYMF278::reset();
  memory.parent = parent;
  chip.reset();
}

void DivPlatformMultiPCM::setFlags(unsigned int flags) {
  chipClock = 9878400;
  rate = chipClock / 224;
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

void DivPlatformMultiPCM::writeChannelState(int i, Channel::State& ch) {
  if (ch.key.changed && ch.key.value) {
    immWrite(i, ADDR_MPCM_KEY, 0x00);
  }

  if (ch.ins.changed || ch.freq.changed) {
    unsigned char fnumL = (ch.freq.value & 0x3ff) << 2;
    unsigned char insH = ch.ins.value >> 8 & 0x1;
    immWrite(i, ADDR_MPCM_FREQL, fnumL | insH);
  }

  if (ch.ins.changed) {
    unsigned char insL = ch.ins.value & 0xff;
    immWrite(i, ADDR_MPCM_WT, insL);
    ch.ins.changed = false;
  }

  if (ch.freq.changed) {
    unsigned char octave = ch.freq.value >> 10 << 4;
    unsigned char fnumH = (ch.freq.value & 0x3ff) >> 6;
    immWrite(i, ADDR_MPCM_FREQH, octave | fnumH);
    ch.freq.changed = false;
  }

  if (ch.tl.changed) {
    unsigned char tl = ch.tl.value << 1;
    unsigned char tlDirect = ch.tlDirect.value & 0x1;
    immWrite(i, ADDR_MPCM_TL, tl | tlDirect);
    ch.tl.changed = false;
    ch.tlDirect.changed = false;
  }

  if (ch.pan.changed) {
    unsigned char pan = ch.pan.value << 4;
    immWrite(i, ADDR_MPCM_PAN, pan);
    ch.pan.changed = false;
  }

  if (ch.key.changed) {
    unsigned char key = ch.key.value << 7;
    immWrite(i, ADDR_MPCM_KEY, key);
    ch.key.changed = false;
  }
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

void DivPlatformOPL4PCM::setFlags(unsigned int flags) {
  chipClock = 33868800;
  rate = chipClock / 768;  // 44100 Hz
}

YMF278& DivPlatformOPL4PCM::getChip() {
  return chip;
}

void DivPlatformOPL4PCM::writeChannelState(int i, Channel::State& ch) {
  if (ch.key.changed && ch.key.value) {
    immWrite(i+ADDR_OPL4_KEY_PAN, (immRead(i+ADDR_OPL4_KEY_PAN) & 0x3f) | 0x40);
  }

  if (ch.ins.changed || ch.freq.changed) {
    unsigned char fnumL = (ch.freq.value & 0x3ff) << 1;
    unsigned char insH = ch.ins.value >> 8 & 0x1;
    immWrite(i+ADDR_OPL4_FREQL, fnumL | insH);
  }

  if (ch.ins.changed) {
    unsigned char insL = ch.ins.value & 0xff;
    immWrite(i+ADDR_OPL4_WT, insL);
    ch.ins.changed = false;
  }

  if (ch.freq.changed || ch.sus.changed) {
    unsigned char octave = ch.freq.value >> 10 << 4;
    unsigned char fnumH = (ch.freq.value & 0x3ff) >> 7;
    unsigned char sus = ch.sus.value ? 0x08 : 0x00;
    immWrite(i+ADDR_OPL4_FREQH, octave | sus | fnumH);
    ch.freq.changed = false;
    ch.sus.changed = false;
  }

  if (ch.tl.changed || ch.tlDirect.changed) {
    unsigned char tl = ch.tl.value << 1;
    unsigned char tlDirect = ch.tlDirect.value & 0x1;
    immWrite(i+ADDR_OPL4_TL, tl | tlDirect);
    ch.tl.changed = false;
    ch.tlDirect.changed = false;
  }

  if (ch.key.changed || ch.damp.changed || ch.lfoReset.changed || ch.pan.changed) {
    unsigned char key = ch.key.value << 7;
    unsigned char damp = ch.damp.value << 6;
    unsigned char lfoReset = ch.lfoReset.value << 5;
    unsigned char pan = ch.pan.value & 0xf;
    immWrite(i+ADDR_OPL4_KEY_PAN, key | damp | lfoReset | pan);
    ch.key.changed = false;
    ch.damp.changed = false;
    ch.lfoReset.changed = false;
    ch.pan.changed = false;
  }
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
