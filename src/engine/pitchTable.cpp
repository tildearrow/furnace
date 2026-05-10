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

#include "dispatch.h"
#include "engine.h"

// DivPitchTableManager

DivPitchTable* DivPitchTableManager::get(int sample) {
  if (e==NULL) return NULL;
  if (!samplePitchTable) return &defaultPitchTable;
  if (sample<0 || sample>=(int)e->song.sample.size()) return &defaultPitchTable;
  return &samplePitchTable[sample];
}

size_t DivPitchTableManager::eSongSampleSize() {
  if (!e) return 0;
  return e->song.sample.size();
}

void DivPitchTableManager::updateSub(float tuning, double clock, double divider, int maximum, bool period, bool linear, int sample) {
  // should we recalculate the tables for all samples, or only one sample?
  if (sample==-1) {
    // calculate the default table
    defaultPitchTable.init(tuning,clock,divider,maximum,period,linear);

    for (size_t i=0; i<MIN(e->song.sample.size(),samplePitchTableLen); i++) {
      DivSample* s=e->song.sample[i];
      double off=(s->centerRate>=1)?((double)s->centerRate/e->getCenterRate()):1.0;
      samplePitchTable[i].init(tuning,clock,divider*off,maximum,period,linear);
    }
  } else {
    if (sample>=0 && sample<(int)e->song.sample.size() && sample<(int)samplePitchTableLen) {
      DivSample* s=e->song.sample[sample];
      double off=(s->centerRate>=1)?((double)s->centerRate/e->getCenterRate()):1.0;
      samplePitchTable[sample].init(tuning,clock,divider*off,maximum,period,linear);
    }
  }
}

void DivPitchTableManager::init(DivEngine* eng) {
  e=eng;
}

DivPitchTableManager::~DivPitchTableManager() {
  if (samplePitchTable) {
    delete[] samplePitchTable;
    samplePitchTable=NULL;
    samplePitchTableLen=0;
  }
}

// DivPitchTable

int DivPitchTable::get(int base, int pitch1, int pitch2) {
  int offset=base+pitch1+pitch2;

  if (!linearity) {
    return offset;
  }

  int coarse=offset>>7;
  if (coarse<0) {
    coarse=0;
  }
  int fine=offset&127;
  int index=coarse%12;
  int octave=period?
    ((coarse/12)-shift):
    (shift-(coarse/12));

  int root=pitch[index];
  int diff=pitchDiff[index];

  // shift the root pitch and delta by the "octave" and then round
  if (octave>0) {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)) && !defined(ANDROID)
    asm(
      "movb %[octave], %%cl\n"
      "movl %[i_root], %%eax\n"
      "sarl %%cl, %%eax\n"
      "jnc rootShiftEnd%=\n"
      "inc %%eax\n"
      "rootShiftEnd%=:\n"
      "movl %%eax, %[root]\n"
      "\n"
      "movl %[i_diff], %%eax\n"
      "sarl %%cl, %%eax\n"
      "jnc diffShiftEnd%=\n"
      "inc %%eax\n"
      "diffShiftEnd%=:\n"
      "movl %%eax, %[diff]\n"
      "\n"
    : [root] "+m" (root),
      [diff] "+m" (diff)
    : [i_root] "m" (root),
      [i_diff] "m" (diff),
      [octave] "m" (octave)
    : "eax", "cl"
    );
#else
    bool carry=root&(1<<(octave-1));
    root>>=octave;
    if (carry) root++;

    carry=diff&(1<<(octave-1));
    diff>>=octave;
    if (carry) diff++;
#endif
  } else if (octave<0) {
    // if we're going to perform a negative shift, return the highest frequency/period.
    return maxFreq;
  }

  return root+((diff*fine)>>7);
}

int DivPitchTable::getBase(int note) {
  // non-linear pitch
  if (!linearity) {
    int index=note%12;
    int octave=period?
      ((note/12)-shift):
      (shift-(note/12));

    int root=pitch[index];

    // shift the root pitch and delta by the "octave" and then round
    if (octave>0) {
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)) && !defined(ANDROID)
      asm(
        "movb %[octave], %%cl\n"
        "movl %[i_root], %%eax\n"
        "sarl %%cl, %%eax\n"
        "jnc rootShiftEnd%=\n"
        "inc %%eax\n"
        "rootShiftEnd%=:\n"
        "movl %%eax, %[root]\n"
      : [root] "+m" (root)
      : [i_root] "m" (root),
        [octave] "m" (octave)
      : "eax", "cl"
      );
#else
      bool carry=root&(1<<(octave-1));
      root>>=octave;
      if (carry) root++;
#endif
    }

    return root;
  }

  // linear pitch - convert note to fixed point
  return note<<7;
}

void DivPitchTable::init(float tuning, double clock, double divider, int maximum, bool isPeriod, bool isLinear) {
  period=isPeriod;
  linearity=isLinear;
  maxFreq=maximum;
  shift=period?0:14;

  // adjust the shift value so that the highest (or lowest in period mode) note has the highest period/freq
  if (period) {
    while (shift<14) {
      int nbase=(shift-4)*12;
      double fbase=(tuning*0.0625)*pow(2.0,(float)(nbase+3)/(12.0));
      int bf=round((clock/fbase)/divider);
      if (bf<=maximum) break;
      shift++;
    }
  } else {
    while (shift>0) {
      int nbase=(shift-5)*12;
      double fbase=tuning*pow(2.0,(float)(nbase+3)/(12.0));
      int bf=round(fbase*(divider/clock));
      if (bf<=maximum) break;
      shift--;
    }
  }

  logV("DivPitchTable init(%f,%f,%f,%x,%s)",tuning,clock,divider,maximum,isPeriod?"period":"freq");
  logV("(shift: %d)",shift);

  for (int i=0; i<=12; i++) {
    int nbase=i+(shift-5)*12;
    double fbase=(period?(tuning*0.0625):tuning)*pow(2.0,(float)(nbase+3)/(12.0));
    int bf=period?
           round((clock/fbase)/divider):
           round(fbase*(divider/clock));
    pitch[i]=bf;
  }

  for (int i=0; i<12; i++) {
    pitchDiff[i]=pitch[i+1]-pitch[i];
    logV("- %d: %x (%x)",i,pitch[i],pitchDiff[i]);
  }
}
