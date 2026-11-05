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

#include "sound/ymfm/ymfm.h"
#include "ym2608.h"
#include "../engine.h"

#include "sound/rss.h"

uint8_t DivYM2608Interface::ymfm_external_read(ymfm::access_class type, uint32_t address) {
  switch (type) {
    case ymfm::ACCESS_ADPCM_A:
      return YM2608_ADPCM_ROM[address&0x1fff];
    case ymfm::ACCESS_ADPCM_B:
      if (adpcmBMem==NULL) {
        return 0;
      }
      return adpcmBMem[address&0x3ffff];
    default:
      return 0;
  }
  return 0;
}

void DivYM2608Interface::ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) {
}
