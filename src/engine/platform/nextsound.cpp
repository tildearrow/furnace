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

#include "nextsound.h"
#include "../engine.h"
#include "sound/nextsound.h"
#include <stdio.h>
#include <math.h>

#define rWrite(a,v) regPool[a]=v; next_writereg(chip,a,v);
#define CHIP_DIVIDER 8

const char* regCheatSheetNS[] = {
  "FreqLx", "x*4+0",
  "FreqHx", "x*4+1",
  "CTRLx", "x*4+2",
  "FDivN", "x*4+3",
  NULL
};

void DivPlatformNextSound::acquire(short** buf, size_t len) {
  int chanOut;
  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t i=0; i<len; i++) {
    next_render(chip);
    oscBuf[0]->putSample(i, (short)chip->chan[0].real_output<<8);
    oscBuf[1]->putSample(i, (short)chip->chan[1].real_output<<8);
    oscBuf[2]->putSample(i, (short)chip->chan[2].real_output<<8);
    oscBuf[3]->putSample(i, (short)chip->chan[3].real_output<<8);
    buf[0][i]=chip->buffer*256;
  }
  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformNextSound::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(ch*4+2,0);
  } else {
    rWrite(ch*4+2, chan[ch].outVol|(chan[ch].noise<<6));
  }
}

void DivPlatformNextSound::tick(bool sysTick) {
  for (unsigned char i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MAX(chan[i].vol+chan[i].std.vol.val-31,0);
      if (!isMuted[i]) { rWrite(i*4+2, chan[i].outVol|(chan[i].noise<<6)); }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    }
    else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note, chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      }
      else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].noisePeriod=chan[i].std.duty.val;
      rWrite(i*4+3, chan[i].noisePeriod);
    }
    if (chan[i].std.phaseReset.had) {
      rWrite(i*4+3, chan[i].noisePeriod|(chan[i].std.phaseReset.val<<7));
    }
    if (chan[i].std.wave.had) {
      chan[i].noise=chan[i].std.wave.val&3;
      if (!isMuted[i]) { rWrite(i*4+2, chan[i].outVol|(chan[i].noise<<6)); }
    }
    if (chan[i].freqChanged) {
      chan[i].freqChanged=false;
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      rWrite(i*4, chan[i].freq & 0x00ff);
      rWrite(i*4+1, (chan[i].freq & 0xff00) >> 8);
    }
  }
}

void* DivPlatformNextSound::getChanState(int ch) {
  return &chan[ch];
}

int DivPlatformNextSound::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_NEXTSOUND);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      chan[c.chan].insChanged=false;
      if (!isMuted[c.chan]) { rWrite(c.chan*4+2, chan[c.chan].outVol); }
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].macroInit(NULL);
      rWrite(c.chan*4+2, 0);
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
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (!isMuted[c.chan]) { rWrite(c.chan*4+2, chan[c.chan].outVol|(chan[c.chan].noise<<6)); }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
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
      if (return2) return 2;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 31;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformNextSound::notifyInsDeletion(void* ins) {
  // nothing
}

float DivPlatformNextSound::getGain(int ch, int vol) {
  if (vol == 0) return 0;
  return (pow(10, (vol * 0.307f) / 20) - 1) / (pow(10, (31 * 0.307f) / 20) - 1);
}

void DivPlatformNextSound::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformNextSound::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x1f;
  }
  next_reset(chip);
}

DivDispatchOscBuffer* DivPlatformNextSound::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformNextSound::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  chipClock=2000000;
  rate=chipClock/8;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->setRate(rate);
  }
  chip=new struct NEXTChip;
  reset();
  return channels;
}

const char** DivPlatformNextSound::getRegisterSheet() {
  return regCheatSheetNS;
}

unsigned char* DivPlatformNextSound::getRegisterPool() {
  return regPool;
}

int DivPlatformNextSound::getRegisterPoolSize() {
  return 16;
}

void DivPlatformNextSound::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  delete chip;
}

DivPlatformNextSound::~DivPlatformNextSound() {
}
