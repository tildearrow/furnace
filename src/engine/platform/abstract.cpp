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

#include "../dispatch.h"

void DivDispatch::acquire(short* bufL, short* bufR, size_t start, size_t len) {
}

void DivDispatch::tick() {
}

void* DivDispatch::getChanState(int chan) {
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

bool DivDispatch::isStereo() {
  return false;
}

bool DivDispatch::keyOffAffectsArp(int ch) {
  return false;
}

bool DivDispatch::keyOffAffectsPorta(int ch) {
  return false;
}

int DivDispatch::getPortaFloor(int ch) {
  return 0x00;
}

const char* DivDispatch::getEffectName(unsigned char effect) {
  return NULL;
}

void DivDispatch::setFlags(unsigned int flags) {
}

void DivDispatch::setSkipRegisterWrites(bool value) {
  skipRegisterWrites=value;
}

void DivDispatch::notifyInsChange(int ins) {

}

void DivDispatch::notifyWaveChange(int ins) {

}

void DivDispatch::notifyInsDeletion(void* ins) {

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

int DivDispatch::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  return 0;
}

void DivDispatch::quit() {
}

DivDispatch::~DivDispatch() {
}
