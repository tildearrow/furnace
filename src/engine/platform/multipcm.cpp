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
#include "../../ta-log.h"
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
  if (memory && address < size) {
    return memory[address];
  }
  return 0;
}

void DivPlatformYMF278::tick(bool sysTick) {
  if (!skipRegisterWrites) {
    writeGlobalState();
  }
  for (int i = 0; i < channelCount; i++) {
    Channel& ch = chan[i];
    ch.std.next();

    if (ch.key.changed || (ch.std.phaseReset.had && ch.std.phaseReset.val == 1)) {
      ch.state.key.set(ch.key.value > 0, ch.key.value > 0);
      ch.state.damp.set(ch.key.value < 0);
      ch.key.changed = false;
    }

    if (ch.ins.changed) {
      const InsMapping& insMapping = getInsMapping(ch.ins.value);
      if (insMapping.valid) {
        ch.state.ins.set(insMapping.ins);

        if (ch.state.ins.changed) {
          unsigned char* insTable = (unsigned char*)getSampleMem() + insMapping.ins * 12 +
            (insMapping.ins >= 0x180 && getSampleMemCapacity(1) ? getSampleMemCapacity(0) - 0x180 * 12: 0);
          ch.state.lfoRate.init(insTable[7] >> 3 & 7);
          ch.state.pm.init(insTable[7] & 7);
          ch.state.am.init(insTable[11] & 7);
          ch.state.ar.init(insTable[8] >> 4);
          ch.state.d1r.init(insTable[8] & 15);
          ch.state.dl.init(insTable[9] >> 4);
          ch.state.d2r.init(insTable[9] & 15);
          ch.state.rc.init(insTable[10] >> 4);
          ch.state.rr.init(insTable[10] & 15);
        }
        DivInstrument* ins = parent->getIns(ch.ins.value, DIV_INS_MULTIPCM);
        ch.state.lfoRate.set(ins->multipcm.lfo);
        ch.state.pm.set(ins->multipcm.vib);
        ch.state.am.set(ins->multipcm.am);
        ch.state.ar.set(ins->multipcm.ar);
        ch.state.d1r.set(ins->multipcm.d1r);
        ch.state.dl.set(ins->multipcm.dl);
        ch.state.d2r.set(ins->multipcm.d2r);
        ch.state.rc.set(ins->multipcm.rc);
        ch.state.rr.set(ins->multipcm.rr);
      }
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
      if (ch.std.pitch.will && !ch.std.pitch.mode) {
        basePitch += ch.std.pitch.val;
      } else {
        basePitch += ch.pitch.value + ch.std.pitch.val;
      }
      basePitch += ch.porta.value + ch.pitchOffset.value;
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

    if (ch.pan.changed || ch.muted.changed || ch.std.panL.had) {
      int pan = ch.pan.value + (ch.std.panL.has ? ch.std.panL.val : 0);
      ch.state.pan.set(ch.muted.value ? -8 : MIN(MAX(pan, -7), 7));
      ch.pan.changed = false;
      ch.muted.changed = false;
    }

    if (ch.lfoRate.changed || ch.std.ex1.had) {
      ch.state.lfoRate.set(MIN(MAX(ch.std.ex1.will ? ch.std.ex1.val : ch.lfoRate.value, 0), 7));
      ch.lfoRate.changed = false;
    }

    if (ch.pm.changed || ch.std.ex2.had) {
      ch.state.pm.set(MIN(MAX(ch.std.ex2.will ? ch.std.ex2.val : ch.pm.value, 0), 7));
      ch.pm.changed = false;
    }

    if (ch.am.changed || ch.std.ex3.had) {
      ch.state.am.set(MIN(MAX(ch.std.ex3.will ? ch.std.ex3.val : ch.am.value, 0), 7));
      ch.am.changed = false;
    }

    if (ch.ar.changed) {
      ch.state.ar.set(MIN(MAX(ch.ar.value, 0), 15));
      ch.ar.changed = false;
    }

    if (ch.d1r.changed) {
      ch.state.d1r.set(MIN(MAX(ch.d1r.value, 0), 15));
      ch.d1r.changed = false;
    }

    if (ch.dl.changed) {
      ch.state.dl.set(MIN(MAX(ch.dl.value, 0), 15));
      ch.dl.changed = false;
    }

    if (ch.d2r.changed) {
      ch.state.d2r.set(MIN(MAX(ch.d2r.value, 0), 15));
      ch.d2r.changed = false;
    }

    if (ch.rr.changed) {
      ch.state.rr.set(MIN(MAX(ch.rr.value, 0), 15));
      ch.rr.changed = false;
    }

    if (ch.rc.changed) {
      ch.state.rc.set(MIN(MAX(ch.rc.value, 0), 15));
      ch.rc.changed = false;
    }

    if (!skipRegisterWrites) {
      writeChannelState(i, ch.state);
    }
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
        ch.pitchOffset.set(getInsMapping(ch.ins.value).pitch);
      }
      break;
    }
    case DIV_CMD_PANNING: {
      int pan = parent->convertPanSplitToLinearLR(c.value, c.value2, 255) - 128;
      ch.pan.set(MIN(MAX((pan + 4) >> 3, -7), 7)); // match TL scale
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
    case DIV_CMD_OPL4_GLOBAL_LEVEL: {
      int level = MIN(MAX(0x07 - (c.value & 0x0f), 0x00), 0x07) |
                  MIN(MAX(0x70 - (c.value & 0xf0), 0x00), 0x70);
      if (c.value2) {
        fmLevel.set(level);
      } else {
        pcmLevel.set(level);
      }
      break;
    }
    case DIV_CMD_MULTIPCM_LFO_RATE: {
      ch.lfoRate.set(c.value);
      break;
    }
    case DIV_CMD_MULTIPCM_LFO_PM_DEPTH: {
      ch.pm.set(c.value);
      break;
    }
    case DIV_CMD_MULTIPCM_LFO_AM_DEPTH: {
      ch.am.set(c.value);
      break;
    }
    case DIV_CMD_OPL4_PCM_AR: {
      ch.ar.set(c.value);
      break;
    }
    case DIV_CMD_OPL4_PCM_D1R: {
      ch.d1r.set(c.value);
      break;
    }
    case DIV_CMD_OPL4_PCM_DL: {
      ch.dl.set(c.value);
      break;
    }
    case DIV_CMD_OPL4_PCM_D2R: {
      ch.d2r.set(c.value);
      break;
    }
    case DIV_CMD_OPL4_PCM_RR: {
      ch.rr.set(c.value);
      break;
    }
    case DIV_CMD_OPL4_PCM_RC: {
      ch.rc.set(c.value);
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

void DivPlatformYMF278::forceIns() {
  // nothing to do, handled by skipRegisterWrites check in tick.
}

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

const DivPlatformYMF278::InsMapping& DivPlatformYMF278::getInsMapping(int ins) {
  return ins >= 0 && (size_t)ins < insMap.size() ? insMap[ins] : nullInsMapping;
}

int DivPlatformYMF278::calcFreq(int basePitch) {
  float baseOctave = basePitch / (12.0f * 128.0f) - 3.0f +
    log2f(parent->song.tuning) - log2f(440.0f) + log2f(baseClock) - log2f(chipClock);
  float octave = floorf(baseOctave);
  float fnumber = roundf((powf(2.0f, baseOctave - octave) - 1.0f) * 1024.0f);
  return MIN(MAX((int)octave << 10 | (int)fnumber, -0x1C00), 0x1fff);
}

const char* DivPlatformYMF278::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x20:
      return "20xx: PCM LFO Rate (0 to 7)";
      break;
    case 0x21:
      return "21xx: PCM LFO PM Depth (0 to 7)";
      break;
    case 0x22:
      return "22xx: PCM LFO AM Depth (0 to 7)";
      break;
  }
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
  fmLevel=Param(0x33);
  pcmLevel=Param(0x00);
}

void DivPlatformYMF278::quit() {
}

void DivPlatformMultiPCM::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  short pcmBuf[28];
  for (size_t h=start; h<start+len; h++) {
    chip.generate(bufL[h], bufR[h], pcmBuf);
    for (int i = 0; i < 28; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++] = pcmBuf[i];
    }
  }
}

void DivPlatformMultiPCM::reset() {
  DivPlatformYMF278::reset();
  chip.reset();
}

void DivPlatformMultiPCM::setFlags(unsigned int flags) {
  useTG100 = (flags & 0x30) == 0x10;
  useMU5 = (flags & 0x30) == 0x20;
  switch (flags & 15) {
    case 1:
      chipClock = 9400000;
      break;
    default:
      chipClock = 10000000;
      break;
  }
  chip.setClockFrequency(chipClock);
  rate = chip.getSampleRate();
  for (int i = 0; i < 28; i++) {
    oscBuf[i]->rate = rate;
  }
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

DivDispatchOscBuffer* DivPlatformMultiPCM::getOscBuffer(int ch) {
  return oscBuf[ch];
}

const void* DivPlatformMultiPCM::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformMultiPCM::getSampleMemCapacity(int index) {
  return index == 0 ? 0x200000 : 0;
}

size_t DivPlatformMultiPCM::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

void DivPlatformMultiPCM::renderSamples() {
  if (useTG100 && parent->tg100ROM != NULL) {
    memcpy(sampleMem, parent->tg100ROM, getSampleMemCapacity());
  } else if (useMU5 && parent->mu5ROM != NULL) {
    memcpy(sampleMem, parent->mu5ROM, getSampleMemCapacity());
  } else {
    memset(sampleMem, 0, getSampleMemCapacity());
  }
  sampleMap.clear();

  size_t memPos = useTG100 || useMU5 ? getSampleMemCapacity() : 0x1800;
  for (DivSample* s : parent->song.sample) {
    void* data;
    unsigned int length;
    if (s->depth <= 8) {
      data = s->data8;
      length = s->length8;
    } else {
      data = s->data12;
      length = s->length12;
    }
    if (memPos + length > getSampleMemCapacity()) {
      logW("out of MultiPCM memory for sample %s!", s->name);
      break;
    }
    memcpy(sampleMem + memPos, data, length);
    sampleMap.push_back(memPos);
    memPos += length;
  }
  sampleMemLen = memPos;
}

void DivPlatformMultiPCM::renderInstruments() {
  insMap.clear();
  int insNumber = 0, insAddress = 0;
  for (DivInstrument* ins : parent->song.ins) {
    InsMapping insMapping;
    if ((useTG100 || useMU5) && ins->type == DIV_INS_MULTIPCM && ins->multipcm.memType >= 2 &&
      ins->multipcm.romIns >= 0 && ins->multipcm.romIns < 0x200) {
      insMapping = {ins->multipcm.romIns, -1 * 12 * 128};
    } else if (!useTG100 && !useMU5 && insNumber < 0x200 && ins->type == DIV_INS_MULTIPCM && ins->multipcm.memType == 0 &&
      ins->amiga.initSample >= 0 && (size_t)ins->amiga.initSample < sampleMap.size()) {
      DivSample* s = parent->getSample(ins->amiga.initSample);
      int memPos = sampleMap[ins->amiga.initSample];
      int start = 0;
      int length = s->samples;
      int loop = s->loopStart >= 0 ? s->loopStart : length - 4;
      if (ins->multipcm.customPos) {
        start = MIN(MAX(ins->multipcm.start, 0), length - 1);
        length = MIN(MAX(ins->multipcm.end >= 1 ? ins->multipcm.end : length - start + ins->multipcm.end, 1), length - start);
        loop = MIN(MAX(ins->multipcm.loop >= 0 ? ins->multipcm.loop : length + ins->multipcm.loop, 0), length - 1);
      }
      int bitDepth = s->depth <= 8 ? 0 : 3;
      length = MIN(MAX(length, 1), 0x10000);
      loop = MIN(MAX(loop, 0), length - 1);
      start = memPos + (s->depth == 16 ? start * 2 : s->depth == 12 ? start * 3 / 2 : start);
      sampleMem[insAddress + 0] = start >> 16 | bitDepth << 6;
      sampleMem[insAddress + 1] = start >> 8;
      sampleMem[insAddress + 2] = start >> 0;
      sampleMem[insAddress + 3] = loop >> 8;
      sampleMem[insAddress + 4] = loop >> 0;
      sampleMem[insAddress + 5] = ~(length - 1) >> 8;
      sampleMem[insAddress + 6] = ~(length - 1) >> 0;
      sampleMem[insAddress + 7] = ins->multipcm.lfo << 3 | ins->multipcm.vib;
      sampleMem[insAddress + 8] = ins->multipcm.ar << 4 | ins->multipcm.d1r;
      sampleMem[insAddress + 9] = ins->multipcm.dl << 4 | ins->multipcm.d2r;
      sampleMem[insAddress + 10] = ins->multipcm.rc << 4 | ins->multipcm.rr;
      sampleMem[insAddress + 11] = ins->multipcm.am;

      int pitch = s->centerRate <= 0 ? 0 :
        roundf((log2f(s->centerRate) - log2f(44100.0f)) * (12.0f * 128.0f));
      insMapping = {insNumber++, pitch};
      insAddress += 12;
    }
    insMap.push_back(insMapping);
  }
}

int DivPlatformMultiPCM::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  for (int i = 0; i < 28; i++) {
    oscBuf[i] = new DivDispatchOscBuffer;
  }
  sampleMem = new unsigned char[memory.getSize()];
  sampleMemLen = 0;
  memory.memory = sampleMem;
  return DivPlatformYMF278::init(p, channels, sugRate, flags);
}

void DivPlatformMultiPCM::quit() {
  memory.memory = NULL;
  delete[] sampleMem;
  for (int i = 0; i < 28; i++) {
    delete oscBuf[i];
  }
  DivPlatformYMF278::quit();
}

void DivPlatformMultiPCM::writeGlobalState() {
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

  if (ch.lfoRate.changed || ch.pm.changed) {
    unsigned char lfoRate = (ch.lfoRate.value & 7) << 3;
    unsigned char pm = ch.pm.value & 7;
    immWrite(i, ADDR_MPCM_LFO_VIB, lfoRate | pm);
    ch.lfoRate.changed = false;
    ch.pm.changed = false;
  }

  if (ch.am.changed) {
    unsigned char am = ch.am.value & 7;
    immWrite(i, ADDR_MPCM_AM, am);
    ch.am.changed = false;
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
  chip.reset();
  if (useYRW801) {
    immWrite(0x202, 0x10);  // set memory config
  } else {
    immWrite(0x202, 0x00);  // set memory config
  }
}

void DivPlatformOPL4PCM::setFlags(unsigned int flags) {
  useYRW801 = (flags & 0x30) == 0x10;
  switch (flags & 15) {
    case 1:
      chipClock = COLOR_NTSC*8.0;
      break;
    default:
      chipClock = 33868800;
      break;
  }
  chip.setClockFrequency(chipClock);
  rate = chip.getSampleRate();
}

const char* DivPlatformOPL4PCM::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x1f:
      return "1Fxy: FM/PCM global level (left, right; 0 to 7)";
      break;
    case 0x23:
      return "23xx: PCM Attack Rate (0 to 15)";
      break;
    case 0x24:
      return "24xx: PCM Decay 1 Rate (0 to 15)";
      break;
    case 0x25:
      return "25xx: PCM Decay Level (0 to 15)";
      break;
    case 0x26:
      return "26xx: PCM Decay 2 Rate (0 to 15)";
      break;
    case 0x27:
      return "27xx: PCM Release Rate (0 to 15)";
      break;
    case 0x28:
      return "28xx: PCM Rate Correction (0 to 15)";
      break;
  }
  return DivPlatformYMF278::getEffectName(effect);
}

const void* DivPlatformOPL4PCM::getSampleMem(int index) {
  if (useYRW801) {
    return index == 0 ? sampleMem : index == 1 ? sampleMem + getSampleMemCapacity(0) : NULL;
  } else {
    return index == 0 ? sampleMem : NULL;
  }
}

size_t DivPlatformOPL4PCM::getSampleMemCapacity(int index) {
  if (useYRW801) {
    return index == 0 ? 0x200000 : index == 1 ? 0x200000 : 0;
  } else {
    return index == 0 ? 0x400000 : 0;
  }
}

size_t DivPlatformOPL4PCM::getSampleMemUsage(int index) {
  if (useYRW801) {
    return index == 0 ? getSampleMemCapacity(0) : index == 1 ? sampleMemLen - getSampleMemCapacity(0) : 0;
  } else {
    return index == 0 ? sampleMemLen : 0;
  }
}

void DivPlatformOPL4PCM::renderSamples() {
  if (useYRW801) {
    if (parent->yrw801ROM != NULL) {
      memcpy(sampleMem, parent->yrw801ROM, getSampleMemCapacity(0));
    } else {
      memset(sampleMem, 0, getSampleMemCapacity(0));
    }
    memset(sampleMem + getSampleMemCapacity(0), 0, getSampleMemCapacity(1));
  } else {
    memset(sampleMem, 0, getSampleMemCapacity(0));
  }
  sampleMap.clear();

  size_t memPos = useYRW801 ? getSampleMemCapacity(0) + 0x800 : 0x1800;
  for (DivSample* s : parent->song.sample) {
    void* data;
    unsigned int length;
    if (s->depth <= 8) {
      data = s->data8;
      length = s->length8;
    } else if (s->depth <= 12) {
      data = s->data12;
      length = s->length12;
    } else {
      data = s->data16be;
      length = s->length16be;
    }
    if (memPos + length > getSampleMemCapacity(0) + getSampleMemCapacity(1)) {
      logW("out of OPL4 Wave memory for sample %s!", s->name);
      break;
    }
    memcpy(sampleMem + memPos, data, length);
    sampleMap.push_back(memPos);
    memPos += length;
  }
  sampleMemLen = memPos;
}

void DivPlatformOPL4PCM::renderInstruments() {
  insMap.clear();
  int insNumber = useYRW801 ? 0x180 : 0;
  int insAddress = useYRW801 ? getSampleMemCapacity(0) : 0;
  for (DivInstrument* ins : parent->song.ins) {
    InsMapping insMapping;
    if (useYRW801 && ins->type == DIV_INS_MULTIPCM && ins->multipcm.memType == 1 &&
      ins->multipcm.romIns >= 0 && ins->multipcm.romIns < 0x180) {
      insMapping = {ins->multipcm.romIns, -1 * (12 * 128)};
    } else if (insNumber < 0x200 && ins->type == DIV_INS_MULTIPCM && ins->multipcm.memType == 0 &&
      ins->amiga.initSample >= 0 && (size_t)ins->amiga.initSample < sampleMap.size()) {
      DivSample* s = parent->getSample(ins->amiga.initSample);
      int memPos = sampleMap[ins->amiga.initSample];
      int start = 0;
      int length = s->samples;
      int loop = s->loopStart >= 0 ? s->loopStart : length - 4;
      if (ins->multipcm.customPos) {
        start = MIN(MAX(ins->multipcm.start, 0), length - 1);
        length = MIN(MAX(ins->multipcm.end >= 1 ? ins->multipcm.end : length - start + ins->multipcm.end, 1), length - start);
        loop = MIN(MAX(ins->multipcm.loop >= 0 ? ins->multipcm.loop : length + ins->multipcm.loop, 0), length - 1);
      }
      int bitDepth = s->depth <= 8 ? 0 : s->depth <= 12 ? 1 : 2;
      length = MIN(MAX(length, 1), 0x10000);
      loop = MIN(MAX(loop, 0), length - 1);
      start = memPos + (s->depth == 16 ? start * 2 : s->depth == 12 ? start * 3 / 2 : start);
      sampleMem[insAddress + 0] = start >> 16 | bitDepth << 6;
      sampleMem[insAddress + 1] = start >> 8;
      sampleMem[insAddress + 2] = start >> 0;
      sampleMem[insAddress + 3] = loop >> 8;
      sampleMem[insAddress + 4] = loop >> 0;
      sampleMem[insAddress + 5] = ~(length - 1) >> 8;
      sampleMem[insAddress + 6] = ~(length - 1) >> 0;
      sampleMem[insAddress + 7] = ins->multipcm.lfo << 3 | ins->multipcm.vib;
      sampleMem[insAddress + 8] = ins->multipcm.ar << 4 | ins->multipcm.d1r;
      sampleMem[insAddress + 9] = ins->multipcm.dl << 4 | ins->multipcm.d2r;
      sampleMem[insAddress + 10] = ins->multipcm.rc << 4 | ins->multipcm.rr;
      sampleMem[insAddress + 11] = ins->multipcm.am;

      int pitch = s->centerRate <= 0 ? 0 :
        roundf((log2f(s->centerRate) - log2f(44100.0f)) * (12.0f * 128.0f));
      insMapping = {insNumber++, pitch};
      insAddress += 12;
    }
    insMap.push_back(insMapping);
  }
}

int DivPlatformOPL4PCM::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  sampleMem = new unsigned char[memory.getSize()];
  sampleMemLen = 0;
  memory.memory = sampleMem;
  return DivPlatformYMF278::init(p, channels, sugRate, flags);
}

void DivPlatformOPL4PCM::quit() {
  memory.memory = NULL;
  delete[] sampleMem;
  DivPlatformYMF278::quit();
}

YMF278& DivPlatformOPL4PCM::getChip() {
  return chip;
}

void DivPlatformOPL4PCM::writeGlobalState() {
  if (fmLevel.changed) {
    unsigned char right = (fmLevel.value & 0x7) << 3;
    unsigned char left = (fmLevel.value >> 4) & 0x7;
    immWrite(0x2f8, right | left);
    fmLevel.changed = false;
  }
  if (pcmLevel.changed) {
    unsigned char right = (pcmLevel.value & 0x7) << 3;
    unsigned char left = (pcmLevel.value >> 4) & 0x7;
    immWrite(0x2f9, right | left);
    pcmLevel.changed = false;
  }
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

  if (ch.lfoRate.changed || ch.pm.changed) {
    unsigned char lfoRate = (ch.lfoRate.value & 7) << 3;
    unsigned char pm = ch.pm.value & 7;
    immWrite(i+ADDR_OPL4_LFO_VIB, lfoRate | pm);
    ch.lfoRate.changed = false;
    ch.pm.changed = false;
  }

  if (ch.ar.changed || ch.d1r.changed) {
    unsigned char ar = ch.ar.value << 4;
    unsigned char d1r = ch.d1r.value & 0xf;
    immWrite(i+ADDR_OPL4_AR_D1R, ar | d1r);
    ch.ar.changed = false;
    ch.d1r.changed = false;
  }

  if (ch.dl.changed || ch.d2r.changed) {
    unsigned char dl = ch.dl.value << 4;
    unsigned char d2r = ch.d2r.value & 0xf;
    immWrite(i+ADDR_OPL4_DL_D2R, dl | d2r);
    ch.dl.changed = false;
    ch.d2r.changed = false;
  }

  if (ch.rc.changed || ch.rr.changed) {
    unsigned char rc = ch.rc.value << 4;
    unsigned char rr = ch.rr.value & 0xf;
    immWrite(i+ADDR_OPL4_RC_RR, rc | rr);
    ch.rc.changed = false;
    ch.rr.changed = false;
  }

  if (ch.am.changed) {
    unsigned char am = ch.am.value & 7;
    immWrite(i+ADDR_OPL4_AM, am);
    ch.am.changed = false;
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
