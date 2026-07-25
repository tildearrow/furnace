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

#include "c352.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <algorithm>
#include <math.h>
#include <cstring>
#include <cstdint>

namespace {
template<typename T>
inline T clamp_val(T v, T lo, T hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}
}

#define CHIP_FREQBASE 74448896

const char* regCheatSheetC352[] = {
  "CHx_RVol", "00+x*10",
  "CHx_LVol", "01+x*10",
  "CHx_FreqH", "02+x*10",
  "CHx_FreqL", "03+x*10",
  "CHx_Ctrl", "05+x*10",
  "CHx_StartH", "06+x*10",
  "CHx_StartL", "07+x*10",
  "CHx_EndH", "08+x*10",
  "CHx_EndL", "09+x*10",
  "CHx_LoopH", "0A+x*10",
  "CHx_LoopL", "0B+x*10",
  NULL
};



const char** DivPlatformC352::getRegisterSheet() {
  return regCheatSheetC352;
}

void DivPlatformC352::acquire_352(short** buf, size_t len) {
  for (int i = 0; i < totalChans; i++) {
    oscBuf[i]->begin(len);
  }
  const size_t memCapacity = getSampleMemCapacity(0);
  for (size_t h = 0; h < len; h++) {
    while (!writes.empty()) {
      QueuedWrite w = writes.front();
      c352_write;
      regPool[w.addr & 0x1ff] = w.val;
      writes.pop();
    }

    c352_tick;

    // use locals and scale to 16-bit range (do not mutate c352.lout/rout)
    int lout = c352.lout >> 10;
    int rout = c352.rout >> 10;

    if (lout > 32767) lout = 32767; else if (lout < -32768) lout = -32768;
    if (rout > 32767) rout = 32767; else if (rout < -32768) rout = -32768;

    buf[0][h] = static_cast<short>(lout);
    buf[1][h] = static_cast<short>(rout);

    for (int i = 0; i < totalChans; i++) {
      if (c352.voice[i].inv_lout) {
        int v = (c352.voice[i].lout - c352.voice[i].rout) >> 10;
        if (v > 32767) v = 32767; else if (v < -32768) v = -32768;
        oscBuf[i]->putSample(h, static_cast<short>(v));
      }
    }
  }
  for (int i = 0; i < totalChans; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformC352::acquire(short** buf, size_t len) {
  // both variants currently use the same path; call the dedicated function
  acquire_352(buf, len);
}

void DivPlatformC352::tick(bool sysTick) {
  for (int i = 0; i < totalChans; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol = (chan[i].vol * MIN(chan[i].macroVolMul, chan[i].std.vol.val)) / chan[i].macroVolMul;
      chan[i].volChangedL = true;
      chan[i].volChangedR = true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    }
    else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq = NOTE_FREQUENCY(parent->calcArp(chan[i].note, chan[i].std.arp.val));
      }
      chan[i].freqChanged = true;
    }
    if (is352){
      if (chan[i].std.duty.had) {
        unsigned char singleByte = (
          (chan[i].noise ? 1 : 0) |
          (chan[i].invert ? 2 : 0) |
          (chan[i].surround ? 4 : 0)
          );
        if (singleByte != (chan[i].std.duty.val & 7)) {
          chan[i].noise = chan[i].std.duty.val & 1;
          chan[i].invert = chan[i].std.duty.val & 2;
          chan[i].surround = chan[i].std.duty.val & 4;
          chan[i].freqChanged = true;
          chan[i].writeCtrl = true;
        }
      }
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2 += chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2, -32768, 32767);
      }
      else {
        chan[i].pitch2 = chan[i].std.pitch.val;
      }
      chan[i].freqChanged = true;
    }
    if (chan[i].std.panL.had) {
      chan[i].chPanL = (255 * (chan[i].std.panL.val & 255)) / chan[i].macroPanMul;
      chan[i].volChangedL = true;
    }

    if (chan[i].std.panR.had) {
      chan[i].chPanR = (255 * (chan[i].std.panR.val & 255)) / chan[i].macroPanMul;
      chan[i].volChangedR = true;
    }

    if (chan[i].std.phaseReset.had) {
      if ((chan[i].std.phaseReset.val == 1) && chan[i].active) {
        chan[i].audPos = 0;
        chan[i].setPos = true;
      }
    }
    if (chan[i].volChangedL) {
      chan[i].chVolL = (chan[i].outVol * chan[i].chPanL) / 255;
      rWrite(1+(i<<4),chan[i].chVolL);
      chan[i].volChangedL = false;
    }
    if (chan[i].volChangedR) {
      chan[i].chVolR = (chan[i].outVol * chan[i].chPanR) / 255;
      rWrite(0+(i<<4),chan[i].chVolR);
      chan[i].volChangedR = false;
    }
    if (chan[i].setPos) {
      // force keyon
      chan[i].keyOn = true;
      chan[i].setPos = false;
    }
    else {
      chan[i].audPos = 0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivSample* s = parent->getSample(chan[i].sample);
      unsigned char ctrl = 0;
      double off = (s->centerRate >= 1) ? ((double)s->centerRate / parent->getCenterRate()) : 1.0;
      chan[i].freq = (int)(off * parent->calcFreq(chan[i].baseFreq, chan[i].pitch, chan[i].fixedArp ? chan[i].baseNoteOverride : chan[i].arpOff, chan[i].fixedArp, false, 2, chan[i].pitch2, chipClock, CHIP_FREQBASE));
      if (chan[i].freq < 0) chan[i].freq = 0;
      if (chan[i].freq > 65535) chan[i].freq = 65535;
      if (is352) {
        ctrl |= (chan[i].active ? 0x80 : 0) | ((s->isLoopable() || chan[i].noise) ? 0x10 : 0) | ((s->depth == DIV_SAMPLE_DEPTH_C352) ? 1 : 0) | (chan[i].invert ? 0x40 : 0) | (chan[i].surround ? 8 : 0) | (chan[i].noise ? 4 : 0);
      }
      else {
        ctrl |= (chan[i].active ? 0x80 : 0) | ((s->isLoopable()) ? 0x10 : 0) | ((s->depth == DIV_SAMPLE_DEPTH_MULAW) ? 0x08 : 0);
      }
      if (chan[i].keyOn) {
        unsigned int bank = 0;
        unsigned int start = 0;
        unsigned int loop = 0;
        unsigned int end = 0;
        if (chan[i].sample >= 0 && chan[i].sample < parent->song.sampleLen) {
          if (is352) {
            bank = (sampleOff[chan[i].sample] >> 16) & 3;
            start = sampleOff[chan[i].sample] & 0xffff;
            end = MIN(start + (s->length8 >> 1) - 1, 65535);
          }
          else {
            bank = (sampleOff[chan[i].sample] >> 16) & 0xff;
            start = sampleOff[chan[i].sample] & 0xffff;
            end = MIN(start + s->length8 - 1, 65535);
          }
        }
        else if (chan[i].noise&&is352) {
          bank = groupBank[i >> 2];
          start = 0;
          end = 1;
        }
        if (chan[i].audPos > 0) {
          start = MIN(start + (MIN(chan[i].audPos, s->length8) >> 1), 65535);
        }
        if (chan[i].sample >= 0 && chan[i].sample < parent->song.sampleLen && s->isLoopable()) {
          if (is352) {
            loop = MIN(start + (s->loopStart >> 1), 65535);
            end = MIN(start + (s->loopEnd >> 1), 65535);
          }
          else {
            loop = MIN(start + s->loopStart + 1, 65535);
            end = MIN(start + s->loopEnd + 1, 65535);
          }
        }
        else if (chan[i].noise&&is352) {
          loop = 0;
        }
        rWrite(0x05 + (i << 4), 0); // force keyoff first
        if (is352) {
          if (groupBank[i >> 2] != bank) {
            groupBank[i >> 2] = bank;
            rWrite(0x1f1 + (((3 + (i >> 2)) & 3) << 1), groupBank[i >> 2]);
            // shut everyone else up
            for (int j = 0; j < 4; j++) {
              int ch = (i & (~3)) | j;
              if (chan[ch].active && !chan[ch].keyOn && (i & 3) != j) {
                chan[ch].sample = -1;
                chan[ch].active = false;
                chan[ch].keyOff = true;
                chan[ch].macroInit(NULL);
                rWrite(0x05+(ch<<4),ctrl);
              }
            }
          }
        }
        else {
          switch (bankType) {
          case 0:
            bank = ((bank & 8) << 2) | (bank & 7);
            break;
          case 1:
            bank = ((bank & 0x18) << 1) | (bank & 7);
            break;
          }
          rWrite(0x04+(i<<4),bank);
        }
        rWrite(0x06+(i<<4),(start>>8)&0xff);
        rWrite(0x07+(i<<4),start&0xff);
        rWrite(0x08+(i<<4),(end >> 8)&0xff);
        rWrite(0x09+(i<<4),end & 0xff);
        rWrite(0x0a+(i<<4),(loop>>8)&0xff);
        rWrite(0x0b+(i<<4),loop & 0xff);
        if (!chan[i].std.vol.had) {
          chan[i].outVol = chan[i].vol;
          chan[i].volChangedL = true;
          chan[i].volChangedR = true;
        }
        chan[i].writeCtrl = true;
        chan[i].keyOn = false;
      }
      if (chan[i].keyOff) {
        chan[i].writeCtrl = true;
        chan[i].keyOff = false;
      }
      if (chan[i].freqChanged) {
        rWrite(0x02 + (i << 4), chan[i].freq >> 8);
        rWrite(0x03 + (i << 4), chan[i].freq & 0xff);
        chan[i].freqChanged = false;
      }
      if (chan[i].writeCtrl) {
        rWrite(0x05 + (i << 4), ctrl);
        chan[i].writeCtrl = false;
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    bankLabel[i][0] = '0' + groupBank[i];
  }
}

int DivPlatformC352::dispatch(DivCommand c) {
  switch (c.cmd) {
  case DIV_CMD_NOTE_ON: {
    DivInstrument* ins = parent->getIns(chan[c.chan].ins, DIV_INS_AMIGA);
    chan[c.chan].macroVolMul = ins->type == DIV_INS_AMIGA ? 64 : 255;
    chan[c.chan].macroPanMul = ins->type == DIV_INS_AMIGA ? 127 : 255;
    if (c.value != DIV_NOTE_NULL) {
      chan[c.chan].sample = ins->amiga.getSample(c.value);
      chan[c.chan].sampleNote = c.value;
      c.value = ins->amiga.getFreq(c.value);
      chan[c.chan].sampleNoteDelta = c.value - chan[c.chan].sampleNote;
    }
    if (c.value != DIV_NOTE_NULL) {
      chan[c.chan].baseFreq = NOTE_FREQUENCY(c.value);
    }
    if (chan[c.chan].sample < 0 || chan[c.chan].sample >= parent->song.sampleLen) {
      chan[c.chan].sample = -1;
    }
    if (c.value != DIV_NOTE_NULL) {
      chan[c.chan].freqChanged = true;
      chan[c.chan].note = c.value;
    }
    chan[c.chan].active = true;
    chan[c.chan].keyOn = true;
    chan[c.chan].macroInit(ins);
    if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
      chan[c.chan].outVol = chan[c.chan].vol;
      chan[c.chan].volChangedL = true;
      chan[c.chan].volChangedR = true;
    }
    break;
  }
  case DIV_CMD_NOTE_OFF:
    chan[c.chan].sample = -1;
    chan[c.chan].active = false;
    chan[c.chan].keyOff = true;
    chan[c.chan].macroInit(NULL);
    break;
  case DIV_CMD_NOTE_OFF_ENV:
  case DIV_CMD_ENV_RELEASE:
    chan[c.chan].std.release();
    break;
  case DIV_CMD_INSTRUMENT:
    if (chan[c.chan].ins != c.value || c.value2 == 1) {
      chan[c.chan].ins = c.value;
    }
    break;
  case DIV_CMD_VOLUME:
    chan[c.chan].vol = c.value;
    if (!chan[c.chan].std.vol.has) {
      chan[c.chan].outVol = c.value;
    }
    chan[c.chan].volChangedL = true;
    chan[c.chan].volChangedR = true;
    break;
  case DIV_CMD_GET_VOLUME:
    if (chan[c.chan].std.vol.has) {
      return chan[c.chan].vol;
    }
    return chan[c.chan].outVol;
    break;
  case DIV_CMD_STD_NOISE_MODE:
    if (!is352) break;
    chan[c.chan].noise = c.value;
    chan[c.chan].writeCtrl = true;
    break;
  case DIV_CMD_SNES_INVERT:
    if (!is352) break;
    chan[c.chan].invert = c.value & 15;
    chan[c.chan].surround = c.value >> 4;
    chan[c.chan].writeCtrl = true;
    break;
  case DIV_CMD_PANNING:
    chan[c.chan].chPanL = c.value;
    chan[c.chan].chPanR = c.value2;
    chan[c.chan].volChangedL = true;
    chan[c.chan].volChangedR = true;
    break;
  case DIV_CMD_PITCH:
    chan[c.chan].pitch = c.value;
    chan[c.chan].freqChanged = true;
    break;
  case DIV_CMD_NOTE_PORTA: {
    int destFreq = NOTE_FREQUENCY(c.value2 + chan[c.chan].sampleNoteDelta);
    bool return2 = false;
    if (destFreq > chan[c.chan].baseFreq) {
      chan[c.chan].baseFreq += c.value;
      if (chan[c.chan].baseFreq >= destFreq) {
        chan[c.chan].baseFreq = destFreq;
        return2 = true;
      }
    }
    else {
      chan[c.chan].baseFreq -= c.value;
      if (chan[c.chan].baseFreq <= destFreq) {
        chan[c.chan].baseFreq = destFreq;
        return2 = true;
      }
    }
    chan[c.chan].freqChanged = true;
    if (return2) {
      chan[c.chan].inPorta = false;
      return 2;
    }
    break;
  }
  case DIV_CMD_LEGATO: {
    chan[c.chan].baseFreq = NOTE_FREQUENCY(c.value + chan[c.chan].sampleNoteDelta + ((HACKY_LEGATO_MESS) ? (chan[c.chan].std.arp.val - 12) : (0)));
    chan[c.chan].freqChanged = true;
    chan[c.chan].note = c.value;
    break;
  }
  case DIV_CMD_PRE_PORTA:
    if (chan[c.chan].active && c.value2) {
      if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins, DIV_INS_AMIGA));
    }
    if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq = NOTE_FREQUENCY(chan[c.chan].note);
    chan[c.chan].inPorta = c.value;
    break;
  case DIV_CMD_SAMPLE_POS:
    chan[c.chan].audPos = c.value;
    chan[c.chan].setPos = true;
    break;
  case DIV_CMD_GET_VOLMAX:
    return 255;
    break;
  case DIV_CMD_MACRO_OFF:
    chan[c.chan].std.mask(c.value, true);
    break;
  case DIV_CMD_MACRO_ON:
    chan[c.chan].std.mask(c.value, false);
    break;
  case DIV_CMD_MACRO_RESTART:
    chan[c.chan].std.restart(c.value);
    break;
  default:
    break;
  }
  return 1;
}

void DivPlatformC352::muteChannel(int ch, bool mute) {
  if (ch < 0 || ch >= totalChans) {
    logW("DivPlatformC352::muteChannel(): invalid channel %d", ch);
    return;
  }

  // no-op if state already matches
  if (isMuted[ch] == mute) return;

  isMuted[ch] = mute;

  // underlying core uses the same field for both variants; set it unconditionally.
  c352.voice[ch] .muted = mute;
}

void DivPlatformC352::forceIns() {
  for (int i = 0; i < totalChans; i++) {
    chan[i].insChanged = true;
    chan[i].freqChanged = true;
    chan[i].volChangedL = true;
    chan[i].volChangedR = true;
    chan[i].sample = -1;
  }
  if (is352) {
    // restore banks
    for (int i = 0; i < 4; i++) {
      rWrite(0x1f1 + (((3 + i) & 3) << 1), groupBank[i]);
    }
  }
}

void* DivPlatformC352::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformC352::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformC352::getPan(int ch) {
  return (chan[ch].chPanL << 8) | (chan[ch].chPanR);
}

DivDispatchOscBuffer* DivPlatformC352::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformC352::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool, 0, 512);
  c352_reset;
  for (int i = 0; i < totalChans; i++) {
    chan[i] = DivPlatformC352::Channel();
    chan[i].std.setEngine(parent);
    rWrite(0x05 + (i << 4), 0);
  }
  for (int i = 0; i < 4; i++) {
    groupBank[i] = 0;
  }
}

void DivPlatformC352::rWrite(unsigned short addr, unsigned short val) {
  if (!skipRegisterWrites) {
    // queue a byte write for the C352 core
    writes.push(QueuedWrite(addr, (unsigned char)(val & 0xff)));
    // keep a shadow of register state (C352 uses 9-bit register space)
    regPool[addr & 0x1ff] = (unsigned char)(val & 0xff);
    if (dumpWrites) {
      addWrite(addr, val);
    }
  }
}

int DivPlatformC352::getOutputCount() {
  return 2;
}

void DivPlatformC352::notifyInsChange(int ins) {
  for (int i = 0; i < totalChans; i++) {
    if (chan[i].ins == ins) {
      chan[i].insChanged = true;
    }
  }
}

void DivPlatformC352::notifyWaveChange(int wave) {

}

void DivPlatformC352::notifyInsDeletion(void* ins) {
  for (int i = 0; i < totalChans; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformC352::poke(unsigned int addr, unsigned short val) {
  rWrite(addr, val);
}

void DivPlatformC352::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i : wlist) rWrite(i.addr, i.val);
}

unsigned char* DivPlatformC352::getRegisterPool() {
  return regPool;
}

int DivPlatformC352::getRegisterPoolSize() {
  return 512;
}

float DivPlatformC352::getPostAmp() {
  return 3.0f;
}

void DivPlatformC352::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (!is352) return;
  if ((ch & 3) == 0) {
    ret.push_back(DivChannelPair(bankLabel[ch >> 2], ch + 1, ch + 2, ch + 3, -1, -1, -1, -1, -1));
  }
}

const void* DivPlatformC352::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformC352::getSampleMemCapacity(int index) {
  if (index != 0) return 0;
  if (is352) return 524288;
  switch (bankType) {
  case 0:
    return 2097152;
  case 1:
    return 4194304;
  }
  return 16777216;
}

size_t DivPlatformC352::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformC352::isSampleLoaded(int index, int sample) {
  if (index != 0) return false;
  if (sample < 0 || sample>32767) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformC352::getMemCompo(int index) {
  if (index != 0) return NULL;
  return &memCompo;
}

void DivPlatformC352::renderSamples(int sysID) {
  size_t capacity = getSampleMemCapacity(0);
  memset(sampleMem, 0, capacity);
  memset(sampleOff, 0, 32768 * sizeof(unsigned int));
  memset(sampleLoaded, 0, 32768 * sizeof(bool));

  memCompo = DivMemoryComposition();
  memCompo.name = "Sample ROM";

  size_t memPos = 0;
  for (int sidx = 0; sidx < parent->song.sampleLen; sidx++) {
    DivSample* s = parent->song.sample[sidx];
    if (!s->renderOn[0][sysID]) {
      sampleOff[sidx] = 0;
      continue;
    }

    if (is352) { // C352 (8-bit)
      unsigned int length = s->length8 + 4;
      // fit sample size to single bank size
      if (length > 131072) {
        length = 131072;
      }
      if (length & 1) length++;
      if ((memPos & 0xfe0000) != ((memPos + length) & 0xfe0000)) {
        memPos = ((memPos + 0x1ffff) & 0xfe0000);
      }
      logV("%d", length);
      if (memPos >= capacity) {
        logW("out of C352 memory for sample %d!", sidx);
        break;
      }
      if (memPos + length >= capacity) {
        length = static_cast<unsigned int>(capacity - memPos);
        logW("out of C352 memory for sample %d!", sidx);
      }
      if (s->depth == DIV_SAMPLE_DEPTH_C352) {
        unsigned char next = 0;
        unsigned int sPos = 0;
        for (unsigned int j = 0; j < length; j++) {
          if (sPos < s->lengthC352) {
            next = s->dataC352[sPos++];
            if (s->isLoopable()) {
              if ((int)sPos >= s->loopEnd) {
                sPos = s->loopStart;
              }
            }
          }
          sampleMem[(memPos + j) ^ 1] = next;
        }
      }
      else {
        signed char next = 0;
        unsigned int sPos = 0;
        for (unsigned int j = 0; j < length; j++) {
          if (sPos < s->length8) {
            next = s->data8[sPos++];
            if (s->isLoopable()) {
              if ((int)sPos >= s->loopEnd) {
                sPos = s->loopStart;
              }
            }
          }
          sampleMem[(memPos + j) ^ 1] = next;
        }
      }
      sampleOff[sidx] = memPos >> 1;
      sampleLoaded[sidx] = true;
      memCompo.entries.push_back(DivMemoryEntry((DivMemoryEntryType)(DIV_MEMORY_BANK0 + ((memPos >> 17) & 3)), "Sample", sidx, memPos, memPos + length));
      memPos += length;
    }
    else { // C352 (16-bit)
      unsigned int length = s->length16 + 4;
      // fit sample size to single bank size
      if (length > (131072)) {
        length = 131072;
      }
      if ((memPos & 0xfe0000) != ((memPos + length) & 0xfe0000)) {
        memPos = ((memPos + 0x1ffff) & 0xfe0000);
      }
      if (memPos >= capacity) {
        logW("out of C352 memory for sample %d!", sidx);
        break;
      }
      if (memPos + length >= capacity) {
        length = static_cast<unsigned int>(capacity - memPos);
        logW("out of C352 memory for sample %d!", sidx);
      }
      if (s->depth == DIV_SAMPLE_DEPTH_MULAW) {
        for (unsigned int j = 0; j < length; j += 2) {
          if ((j >> 1) >= s->lengthMuLaw) break;
          unsigned char x = s->dataMuLaw[j >> 1] ^ 0xff;
          if (x & 0x80) x ^= 15;
          unsigned char c352Mu = (x & 0x80) | ((x & 15) << 3) | ((x & 0x70) >> 4);
          sampleMem[j + memPos] = 0;
          sampleMem[1 + j + memPos] = c352Mu;
        }
      }
      else {
        short next = 0;
        unsigned int sPos = 0;
        for (unsigned int j = 0; j < length; j += 2) {
          if (sPos < s->samples) {
            next = s->data16[sPos++];
            if (s->isLoopable()) {
              if ((int)sPos >= s->loopEnd) {
                sPos = s->loopStart;
              }
            }
          }
          sampleMem[memPos + j] = ((unsigned short)next);
          sampleMem[memPos + j + 1] = ((unsigned short)next) >> 8;
        }
      }
      sampleOff[sidx] = memPos >> 1;
      sampleLoaded[sidx] = true;
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE, "Sample", sidx, memPos, memPos + length));
      memPos += length;
    }
  }
  sampleMemLen = memPos + 256;

  memCompo.used = sampleMemLen;
  memCompo.capacity = capacity;
}

void DivPlatformC352::set352(bool is_352) {
  is352 = is_352;
  totalChans = is352 ? 16 : 24;
}

int DivPlatformC352::getClockRangeMin() {
  if (is352) return 1000000;
  return MIN_CUSTOM_CLOCK;
}

int DivPlatformC352::getClockRangeMax() {
  if (is352) return 100000000;
  return MAX_CUSTOM_CLOCK;
}

void DivPlatformC352::setFlags(const DivConfig& flags) {
  if (is352) {
    chipClock = 50113000; // 50.113MHz clock input in Namco NA-1/NA-2 PCB
    CHECK_CUSTOM_CLOCK;
    rate = chipClock / 1136; // assumed as ~44100hz
  }
  else {
    chipClock = 32000 * 256; // 8.192MHz and 12.288MHz input, verified from Assault Schematics
    CHECK_CUSTOM_CLOCK;
    rate = chipClock / 192;
  }
  bankType = flags.getInt("bankType", 0);
  if (!is352) {
    c352_bank_type;
  }
  for (int i = 0; i < totalChans; i++) {
    oscBuf[i]->setRate(rate);
  }
}

int DivPlatformC352::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent = p;
  dumpWrites = false;
  skipRegisterWrites = false;
  bankType = 0;

  // ensure the platform uses the requested number of channels
  totalChans = channels;

  memset(bankLabel, 0, 16);

  for (int i = 0; i < totalChans; i++) {
    isMuted[i] = false;
    oscBuf[i] = new DivDispatchOscBuffer;
  }

  // set flags early so getSampleMemCapacity() returns correct size
  setFlags(flags);

  // allocate sample memory using the capacity helper
  size_t capacity = getSampleMemCapacity(0);
  sampleMem = new unsigned char[capacity];
  sampleMemLen = 0;

  c352_init;
  if (is352) {
    c352.sample_mem = reinterpret_cast<signed char*>(sampleMem);
  }
  else {
    c352.sample_mem = reinterpret_cast<signed char*>(sampleMem);
  }

  reset();

  return totalChans;
}

void DivPlatformC352::quit() {
  delete[] sampleMem;
  for (int i = 0; i < totalChans; i++) {
    delete oscBuf[i];
  }
}

// initialization of important arrays
DivPlatformC352::DivPlatformC352() {
  sampleOff = new unsigned int[32768];
  sampleLoaded = new bool[32768];
  // sensible defaults
  totalChans = 0;
  is352 = true;
}

DivPlatformC352::~DivPlatformC352() {
  delete[] sampleOff;
  delete[] sampleLoaded;
}
