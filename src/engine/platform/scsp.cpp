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

#include "scsp.h"
#include "../engine.h"

#define CHIP_FREQBASE 4096

void DivPlatformSCSP::acquire(short** buf, size_t len) {
  for (int i=0; i<8; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t i=0; i<len; i++) {
    buf[0][i]=0;
    buf[1][i]=0;
    for (int j=0; j<8; j++) {
      oscBuf[j]->putSample(i,0);
    }
  }
  for (int i=0; i<8; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSCSP::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformSCSP::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged) {
      chan[i].freqChanged=false;
      chan[i].freq=chan[i].calcFreq();
    }
  }
}

SharedChannel* DivPlatformSCSP::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformSCSP::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformSCSP::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value);
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].active=true;
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      if (chan[c.chan].vol>127) chan[c.chan].vol=127;
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].pan=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSCSP::notifyInsDeletion(void* ins) {
}

void DivPlatformSCSP::notifyPitchTable(int sample) {
  pitchTable.init(parent->song.tuning,chipClock,CHIP_FREQBASE,0xffff,false,parent->song.compatFlags.linearPitch);
}

void DivPlatformSCSP::reset() {
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformSCSP::Channel(parent->song.compatFlags.linearPitch);
    chan[i].pitchTable=&pitchTable;
    chan[i].vol=0x7f;
  }
  for (int i=0; i<32; i++) {
    slotInUse[i]=false;
  }
}

int DivPlatformSCSP::getOutputCount() {
  return 2;
}

bool DivPlatformSCSP::hasSoftPan(int ch) {
  return true;
}

int DivPlatformSCSP::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->setRate(44100);
  }
  chipClock=22579200;
  rate=44100;
  notifyPitchTable();
  reset();
  return 8;
}

void DivPlatformSCSP::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}

DivPlatformSCSP::~DivPlatformSCSP() {
}
