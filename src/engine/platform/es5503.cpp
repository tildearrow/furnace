/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "es5503.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

const char* regCheatSheetES5503[]={
  "CHx_FreqL", "x",
  "CHx_FreqH", "20+x",
  "CHx_Vol", "40+x",
  "CHx_CurrSample", "60+x",
  "CHx_WaveAddress", "80+x",
  "CHx_Control", "A0+x",
  "CHx_WaveSize", "C0+x",
  "Osc_interrupt", "E0",
  "Osc_enable", "E1",
  "A/D_conversion_result", "E2",
  NULL
};


const char** DivPlatformES5503::getRegisterSheet() {
  return regCheatSheetES5503;
}

void DivPlatformES5503::acquire(short** buf, size_t len) {
  
}

void DivPlatformES5503::setFlags(const DivConfig& flags) {
  if (es5503!=NULL) {
    delete es5503;
    es5503=NULL;
  }
  es5503=new es5503_core();
}

void DivPlatformES5503::updateWave(int ch) {
  
}

void DivPlatformES5503::tick(bool sysTick) {
  
}

int DivPlatformES5503::dispatch(DivCommand c) {
  return 1;
}

void DivPlatformES5503::muteChannel(int ch, bool mute) {
  
}

void DivPlatformES5503::forceIns() {
  
}

void* DivPlatformES5503::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformES5503::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivSamplePos DivPlatformES5503::getSamplePos(int ch) {
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformES5503::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformES5503::getRegisterPool() {
  return regPool;
}

int DivPlatformES5503::getRegisterPoolSize() {
  return 256;
}

void DivPlatformES5503::reset() {
  
}

int DivPlatformES5503::getOutputCount() {
  return 2;
}

bool DivPlatformES5503::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformES5503::notifyWaveChange(int wave) {
  
}

void DivPlatformES5503::notifyInsDeletion(void* ins) {
  
}

void DivPlatformES5503::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformES5503::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformES5503::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  updateLFO=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  es5503=NULL;
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformES5503::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  if (es5503!=NULL) {
    delete es5503;
    es5503=NULL;
  }
}

DivPlatformES5503::~DivPlatformES5503() {
}
