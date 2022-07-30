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

#include "../engine.h"
#include "../instrument.h"
#include "../sample.h"
#include "../../ta-utils.h"

class DivPlatformSample {
  protected:
    int sampleBank;

    inline void setSampleBank(DivEngine* parent, int bank) {
      sampleBank=parent->setSampleBank(bank);
    }

    inline void setSampleBank(DivEngine* parent, int& prev, int bank) {
      prev=parent->setSampleBank(bank);
    }

    inline int getCompatibleSample(DivEngine* parent, int note) {
      return parent->getCompatibleSample(sampleBank,note);
    }

    inline int getCompatibleSample(DivEngine* parent, int bank, int note) {
      return parent->getCompatibleSample(bank,note);
    }

    inline bool getSampleVaild(DivEngine* parent, int index) {
      return parent->getSampleVaild(index);
    }

    inline double getCenterRate(DivInstrument* ins, DivSample* s, int note, bool period) {
      return ins->amiga.getCenterRate(s,note,period);
    }

    DivPlatformSample():
      sampleBank(0) {}
};

#endif
