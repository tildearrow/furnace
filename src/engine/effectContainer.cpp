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

#include "engine.h"
#include "effect/dummy.h"

void DivEffectContainer::preAcquire(size_t count) {
  if (!count) return;

  int inCount=effect->getInputCount();

  if (inLen<count) {
    for (int i=0; i<inCount; i++) {
      if (in[i]!=NULL) {
        delete[] in[i];
        in[i]=new float[count];
      }
    }
    inLen=count;
  }

  for (int i=0; i<inCount; i++) {
    if (in[i]==NULL) {
      in[i]=new float[count];
    }
  }
}

void DivEffectContainer::acquire(size_t count) {
  if (!count) return;

  int outCount=effect->getOutputCount();

  if (outLen<count) {
    for (int i=0; i<outCount; i++) {
      if (out[i]!=NULL) {
        delete[] out[i];
        out[i]=new float[count];
      }
    }
    outLen=count;
  }

  for (int i=0; i<outCount; i++) {
    if (out[i]==NULL) {
      out[i]=new float[count];
    }
  }

  effect->acquire(in,out,count);
}

bool DivEffectContainer::init(DivEffectType effectType, DivEngine* eng, double rate, unsigned short version, const unsigned char* data, size_t len) {
  switch (effectType) {
    case DIV_EFFECT_DUMMY:
    default:
      effect=new DivEffectDummy;
  }
  return effect->init(eng,rate,version,data,len);
}

void DivEffectContainer::quit() {
  effect->quit();
  delete effect;
  effect=NULL;

  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (in[i]) {
      delete[] in[i];
      in[i]=NULL;
    }
    if (out[i]) {
      delete[] out[i];
      out[i]=NULL;
    }
  }
  inLen=0;
  outLen=0;
}