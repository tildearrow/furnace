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

#include "dummy.h"

void DivEffectDummy::acquire(float** in, float** out, size_t len) {
  memcpy(out[0],in[0],len*sizeof(float));
}

void DivEffectDummy::reset() {
}

int DivEffectDummy::getInputCount() {
  return 1;
}

int DivEffectDummy::getOutputCount() {
  return 1;
}

const char* DivEffectDummy::getParams() {
  return NULL;
}

size_t DivEffectDummy::getParamCount() {
  return 0;
}

bool DivEffectDummy::load(unsigned short version, const unsigned char* data, size_t len) {
  return true;
}

unsigned char* DivEffectDummy::save(unsigned short* version, size_t* len) {
  *len=0;
  *version=0;
  return NULL;
}

bool DivEffectDummy::init(DivEngine* parent, double rate, unsigned short version, const unsigned char* data, size_t len) {
  return false;
}

void DivEffectDummy::quit() {
}