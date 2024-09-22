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

#ifndef _FMSHARED_OPM_H
#define _FMSHARED_OPM_H

#include "fmsharedbase.h"

#define NOTE_LINEAR(x) ((x)<<7)
#define OFFSET_LINEAR (baseFreqOff+log2(parent->song.tuning/440.0)*12.0*128.0)


class DivPlatformOPM: public DivPlatformFMBase {
  protected:
    const unsigned short ADDR_MULT_DT=0x40;
    const unsigned short ADDR_TL=0x60;
    const unsigned short ADDR_RS_AR=0x80;
    const unsigned short ADDR_AM_DR=0xa0;
    const unsigned short ADDR_DT2_D2R=0xc0;
    const unsigned short ADDR_SL_RR=0xe0;
    const unsigned short ADDR_NOTE=0x28;
    const unsigned short ADDR_KF=0x30;
    const unsigned short ADDR_FMS_AMS=0x38;
    const unsigned short ADDR_LR_FB_ALG=0x20;

    const unsigned short opOffs[4]={
      0x00, 0x08, 0x10, 0x18
    };

    unsigned char lfoValue, lfoValue2, lfoShape, lfoShape2;
    bool brokenPitch;

    DivPlatformOPM():
      DivPlatformFMBase(),
      lfoValue(0),
      lfoValue2(0),
      lfoShape(0),
      lfoShape2(0),
      brokenPitch(false) {}
};

#endif
