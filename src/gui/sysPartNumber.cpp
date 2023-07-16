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
 
#include "gui.h"

const char* FurnaceGUI::getSystemPartNumber(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_YMU759:
      return "YMU759";
      break;
    case DIV_SYSTEM_SMS:
      return "SN76489";
      break;
    case DIV_SYSTEM_NES:
      return "2A03";
      break;
    case DIV_SYSTEM_C64_6581:
      return "MOS 6581";
      break;
    case DIV_SYSTEM_C64_8580:
      return "MOS 8580";
      break;
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_Y8950_DRUMS:
      return "Y8950";
      break;
    case DIV_SYSTEM_AY8910:
      return "AY8910";
      break;
    case DIV_SYSTEM_AMIGA:
      return "Amiga";
      break;
    case DIV_SYSTEM_YM2151:
      return "YM2151";
      break;
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_CSM:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
    case DIV_SYSTEM_YM2612_EXT:
      return "YM2612";
      break;
    case DIV_SYSTEM_TIA:
      return "TIA";
      break;
    case DIV_SYSTEM_SAA1099:
      return "SAA1099";
      break;
    case DIV_SYSTEM_AY8930:
      return "AY8930";
      break;
    case DIV_SYSTEM_VIC20:
      return "VIC";
      break;
    case DIV_SYSTEM_PET:
      return "PET";
      break;
    case DIV_SYSTEM_VRC6:
      return "VRC6";
      break;
    case DIV_SYSTEM_FDS:
      return "FDS";
      break;
    case DIV_SYSTEM_MMC5:
      return "MMC5";
      break;
    case DIV_SYSTEM_N163:
      return "N163";
      break;
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
    case DIV_SYSTEM_YM2203_CSM:
      return "YM2203";
      break;
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_CSM:
    case DIV_SYSTEM_YM2608_EXT:
      return "YM2608";
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
      return "YM3526";
      break;
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
      return "YM3812";
      break;
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
      return "YMF262";
      break;
    case DIV_SYSTEM_OPL4:
    case DIV_SYSTEM_OPL4_DRUMS:
      return "OPL4";
      break;
    case DIV_SYSTEM_MULTIPCM:
      return "MultiPCM";
      break;
    case DIV_SYSTEM_RF5C68:
      return "RF5C68";
      break;
    case DIV_SYSTEM_OPZ:
      return "YM2414";
      break;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      return "SegaPCM";
      break;
    case DIV_SYSTEM_VRC7:
      return "VRC7";
      break;
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_CSM:
    case DIV_SYSTEM_YM2610B_EXT:
      return "YM2610B";
      break;
    case DIV_SYSTEM_SFX_BEEPER:
    case DIV_SYSTEM_SFX_BEEPER_QUADTONE:
      return "ZXS Beeper";
      break;
    case DIV_SYSTEM_SCC:
      return "SCC";
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_CSM:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return "YM2610";
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
      return "YM2413";
      break;
    case DIV_SYSTEM_QSOUND:
      return "QSound";
      break;
    case DIV_SYSTEM_VERA:
      return "VERA";
      break;
    case DIV_SYSTEM_X1_010:
      return "X1-010";
      break;
    case DIV_SYSTEM_BUBSYS_WSG:
      return "Konami WSG";
      break;
    case DIV_SYSTEM_ES5506:
      return "ES5506";
      break;
    case DIV_SYSTEM_SCC_PLUS:
      return "SCC+";
      break;
    case DIV_SYSTEM_SOUND_UNIT:
      return "TSU";
      break;
    case DIV_SYSTEM_MSM6295:
      return "MSM6295";
      break;
    case DIV_SYSTEM_MSM6258:
      return "MSM6258";
      break;
    case DIV_SYSTEM_YMZ280B:
      return "YMZ280B";
      break;
    case DIV_SYSTEM_NAMCO:
      return "Namco WSG";
      break;
    case DIV_SYSTEM_NAMCO_15XX:
      return "C15";
      break;
    case DIV_SYSTEM_NAMCO_CUS30:
      return "C30";
      break;
    case DIV_SYSTEM_MSM5232:
      return "MSM5232";
      break;
    case DIV_SYSTEM_T6W28:
      return "T6W28";
      break;
    case DIV_SYSTEM_K007232:
      return "K007232";
      break;
    case DIV_SYSTEM_GA20:
      return "GA20";
      break;
    case DIV_SYSTEM_PCM_DAC:
      return "DAC";
      break;
    case DIV_SYSTEM_SM8521:
      return "SM8521";
      break;
    case DIV_SYSTEM_PV1000:
      return "PV-1000";
      break;
    default:
      return FurnaceGUI::getSystemName(sys);
      break;
  }
}
