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

#ifndef _SAMPLESHARED_H
#define _SAMPLESHARED_H

#define _USE_MATH_DEFINES
#include <cmath>
#include "../engine.h"
#include "../instrument.h"
#include "../sample.h"

class DivPlatformSample {
  protected:
    int sampleBank;

    inline int getCompatibleSample(int note) {
      return 12*sampleBank+note%12;
    }

    inline int getCompatibleSample(int bank, int note) {
      return 12*bank+note%12;
    }

    inline bool getSampleVaild(DivEngine* parent, int index) {
      return (index>=0 && index<parent->song.sampleLen);
    }

    inline double getCenterRate(DivInstrument* ins, DivSample* s, int note, bool period) {
      double off=1.0;
      double center=(double)s->centerRate;
      if (center<1) {
        off=1.0;
      } else {
        off=period?(8363.0/(double)center):((double)center/8363.0);
      }
      if (ins->amiga.useNoteMap) {
        if (period) {
          off/=(double)ins->amiga.getFreq(note)/((double)MAX(1,center)*pow(2.0,((double)note-48.0)/12.0));
        } else {
          off*=(double)ins->amiga.getFreq(note)/((double)MAX(1,center)*pow(2.0,((double)note-48.0)/12.0));
        }
      }
      return off;
    }

    DivPlatformSample():
      sampleBank(0) {}
};

#endif
