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

#include "sample.h"
#include "../ta-log.h"
#include <string.h>
#include <sndfile.h>
#include <math.h>

bool DivSample::save(const char* path) {
  SNDFILE* f;
  SF_INFO si;
  memset(&si,0,sizeof(SF_INFO));

  if (length<1) return false;

  si.channels=1;
  si.samplerate=rate;
  if (depth==16) {
    si.format=SF_FORMAT_PCM_16|SF_FORMAT_WAV;
  } else {
    si.format=SF_FORMAT_PCM_U8|SF_FORMAT_WAV;
  }

  f=sf_open(path,SFM_WRITE,&si);

  if (f==NULL) {
    logE("could not open wave file for saving! %s\n",sf_error_number(sf_error(f)));
    return false;
  }

  SF_INSTRUMENT inst;
  memset(&inst, 0, sizeof(inst));
  inst.gain = 1;
  short pitch = (0x3c * 100) + 50 - (log2((double)centerRate/rate) * 12.0 * 100.0);
  inst.basenote = pitch / 100;
  inst.detune = 50 - (pitch % 100);
  inst.velocity_hi = 0x7f;
  inst.key_hi = 0x7f;
  if(loopStart != -1)
  {
    inst.loop_count = 1;
    inst.loops[0].mode = SF_LOOP_FORWARD;
    inst.loops[0].start = loopStart;
    inst.loops[0].end = length;
  }
  sf_command(f, SFC_SET_INSTRUMENT, &inst, sizeof(inst));

  if (depth==16) {
    sf_writef_short(f,data,length);
  } else {
    short* cbuf=new short[length];
    for (int i=0; i<length; i++) {
      cbuf[i]=(data[i]<<8)^0x8000;
    }
    sf_writef_short(f,cbuf,length);
    delete[] cbuf;
  }

  sf_close(f);

  return true;
}

DivSample::~DivSample() {
  if (data) delete[] data;
  if (rendData) delete[] rendData;
  if (adpcmRendData) delete[] adpcmRendData;
}
