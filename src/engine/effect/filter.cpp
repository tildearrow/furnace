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

#include "filter.h"
#include "../engine.h"

void DivEffectFilter::acquire(float** in, float** out, size_t len) {
  for (size_t i=0; i<len; i++) {
    last[0]+=cut*(in[0][i]-last[0]);
    out[0][i]=last[0];

    if (stereo) {
      last[1]+=cut*(in[1][i]-last[1]);
      out[1][i]=last[1];
    }
  }
}

void DivEffectFilter::reset() {
  last[0]=0;
  last[1]=0;
}

int DivEffectFilter::getInputCount() {
  return stereo?2:1;
}

int DivEffectFilter::getOutputCount() {
  return stereo?2:1;
}

void DivEffectFilter::rateChanged(double rate) {
  cut=MIN(1.0,cutoff/rate);
}

DivEffectParam DivEffectFilter::getParam(size_t param) {
  switch (param) {
    case 0:
      return DivEffectParam((int)stereo);
      break;
    case 1:
      return DivEffectParam(type);
      break;
    case 2:
      return DivEffectParam(cutoff);
      break;
    default:
      throw std::out_of_range("param");
  }
  return "";
}

bool DivEffectFilter::setParam(size_t param, DivEffectParam value) {
  switch (param) {
    case 0:
      if (value.type!=DIV_PARAM_TYPE_S32) return false;
      stereo=value.val.s32;
      break;
    case 1:
      if (value.type!=DIV_PARAM_TYPE_S32) return false;
      type=value.val.s32;
      break;
    case 2:
      if (value.type!=DIV_PARAM_TYPE_DOUBLE) return false;
      cutoff=value.val.d;
      cut=MIN(1.0,cutoff/parent->getAudioDescGot().rate);
      break;
    default:
      return false;
  }
  return true;
}

const char* DivEffectFilter::getParams() {
  return
    "0:R:Channels::Mono:Stereo\n"
    "1:R:Mode::RC low-pass:RC high-pass\n"
    "2:D:Cutoff:(Hz):0:96000:2000\n";
}

size_t DivEffectFilter::getParamCount() {
  return 3;
}

bool DivEffectFilter::load(unsigned short version, const unsigned char* data, size_t len) {
  return true;
}

unsigned char* DivEffectFilter::save(unsigned short* version, size_t* len) {
  *len=0;
  *version=0;
  return NULL;
}

bool DivEffectFilter::init(DivEngine* p, double rate, unsigned short version, const unsigned char* data, size_t len) {
  parent=p;

  cutoff=2000.0;
  cut=MIN(1.0,cutoff/rate);
  stereo=true;
  type=0;
  last[0]=0;
  last[1]=0;

  return load(version,data,len);
}

void DivEffectFilter::quit() {
}
