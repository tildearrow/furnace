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

#include "fmshared_OPN.h"

unsigned int DivPlatformOPN::getMaxFreq(int ch) {
  switch (chTypes[ch]) {
    case OPN_CH_FM:
    case OPN_CH_EXTCH:
      return 0x3fff;
    case OPN_CH_PSG:
      return 0xfff;
    case OPN_CH_ADPCM_A:
      return 0;
    case OPN_CH_ADPCM_B:
      return 0xffff;
    case OPN_CH_FM_DAC:
      return 0x3ffff;
    case OPN_CH_CSM:
      return 0x3ff;
  }
  // fallback
  return 0xffff;
}
