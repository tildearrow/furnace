/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "../dispatch.h"
#include "../../ta-log.h"

void DivDispatch::acquire(short** buf, size_t len) {
}

void DivDispatch::fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
}

void DivDispatch::tick(bool sysTick) {
}

void* DivDispatch::getChanState(int chan) {
  return NULL;
}

unsigned short DivDispatch::getPan(int chan) {
  return 0;
}

void DivDispatch::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  ret.push_back(DivChannelPair());
}

DivChannelModeHints DivDispatch::getModeHints(int chan) {
  return DivChannelModeHints();
}

DivMacroInt* DivDispatch::getChanMacroInt(int chan) {
  return NULL;
}

DivSamplePos DivDispatch::getSamplePos(int chan) {
  return DivSamplePos();
}

DivDispatchOscBuffer* DivDispatch::getOscBuffer(int chan) {
  return NULL;
}

unsigned char* DivDispatch::getRegisterPool() {
  return NULL;
}

int DivDispatch::getRegisterPoolSize() {
  return 0;
}

int DivDispatch::getRegisterPoolDepth() {
  return 8;
}

void* DivDispatch::getState() {
  return NULL;
}

void DivDispatch::setState(void* state) {
}

void DivDispatch::muteChannel(int ch, bool mute) {
}

int DivDispatch::dispatch(DivCommand c) {
  return 1;
}

void DivDispatch::reset() {
}

int DivDispatch::getOutputCount() {
  return 1;
}

bool DivDispatch::keyOffAffectsArp(int ch) {
  return false;
}

bool DivDispatch::keyOffAffectsPorta(int ch) {
  return false;
}

bool DivDispatch::isVolGlobal() {
  return false;
}

int DivDispatch::mapVelocity(int ch, float vel) {
  const int volMax=MAX(1,dispatch(DivCommand(DIV_CMD_GET_VOLMAX,MAX(ch,0))));
  return round(vel*volMax);
}

float DivDispatch::getGain(int ch, int vol) {
  const float volMax=MAX(1,dispatch(DivCommand(DIV_CMD_GET_VOLMAX,MAX(ch,0))));
  return (float)vol/volMax;
}

int DivDispatch::getPortaFloor(int ch) {
  return 0x00;
}

bool DivDispatch::getLegacyAlwaysSetVolume() {
  return true;
}

float DivDispatch::getPostAmp() {
  return 1.0f;
}

bool DivDispatch::getDCOffRequired() {
  return false;
}

bool DivDispatch::getWantPreNote() {
  return false;
}

int DivDispatch::getClockRangeMin() {
  return MIN_CUSTOM_CLOCK;
}

int DivDispatch::getClockRangeMax() {
  return MAX_CUSTOM_CLOCK;
}

void DivDispatch::setFlags(const DivConfig& flags) {
}

void DivDispatch::setSkipRegisterWrites(bool value) {
  skipRegisterWrites=value;
}

void DivDispatch::notifyInsChange(int ins) {

}

void DivDispatch::notifyWaveChange(int ins) {

}

void DivDispatch::notifyInsDeletion(void* ins) {
  logE("notifyInsDeletion NOT implemented!");
  abort();
}

void DivDispatch::notifyPlaybackStop() {

}

void DivDispatch::forceIns() {
  
}

void DivDispatch::toggleRegisterDump(bool enable) {
  dumpWrites=enable;
}

std::vector<DivRegWrite>& DivDispatch::getRegisterWrites() {
  return regWrites;
}

void DivDispatch::poke(unsigned int addr, unsigned short val) {

}

void DivDispatch::poke(std::vector<DivRegWrite>& wlist) {
  
}

const char** DivDispatch::getRegisterSheet() {
  return NULL;
}

const void* DivDispatch::getSampleMem(int index) {
  return NULL;
}

size_t DivDispatch::getSampleMemCapacity(int index) {
  return 0;
}

const char* DivDispatch::getSampleMemName(int index) {
  return NULL;
}

size_t DivDispatch::getSampleMemUsage(int index) {
  return 0;
}

const DivMemoryComposition* DivDispatch::getMemCompo(int index) {
  return NULL;
}

bool DivDispatch::isSampleLoaded(int index, int sample) {
  printf("you are calling.\n");
  return false;
}

void DivDispatch::renderSamples(int sysID) {
  
}

void DivDispatch::notifyPitchTable() {
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  return 0;
}

void DivDispatch::quit() {
}

DivDispatch::~DivDispatch() {
}
