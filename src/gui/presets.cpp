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

#include "gui.h"

// add system configurations here.
// every entry is written in the following format:
//   cat.systems.push_back(FurnaceGUISysDef(
//     "System Name", {
//      DIV_SYSTEM_???, Volume, Panning, Flags,
//      DIV_SYSTEM_???, Volume, Panning, Flags,
//      ...
//      0
//    }
//  ));

void FurnaceGUI::initSystemPresets() {
  FurnaceGUISysCategory cat;

  cat=FurnaceGUISysCategory("FM");
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2612", {
      DIV_SYSTEM_YM2612, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2612 (extended channel 3)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2151", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2610", {
      DIV_SYSTEM_YM2610_FULL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2610 (extended channel 2)", {
      DIV_SYSTEM_YM2610_FULL_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2610B", {
      DIV_SYSTEM_YM2610B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2610B (extended channel 3)", {
      DIV_SYSTEM_YM2610B_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2413", {
      DIV_SYSTEM_OPLL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2413 (drums mode)", {
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Square");
  cat.systems.push_back(FurnaceGUISysDef(
    "TI SN76489", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "AY-3-8910", {
      DIV_SYSTEM_AY8910, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Philips SAA1099", {
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Sample");
  cat.systems.push_back(FurnaceGUISysDef(
    "Amiga", {
      DIV_SYSTEM_AMIGA, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SegaPCM", {
      DIV_SYSTEM_SEGAPCM, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom QSound", {
      DIV_SYSTEM_QSOUND, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seta/Allumer X1-010", {
      DIV_SYSTEM_X1_010, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Game consoles");
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis", {
      DIV_SYSTEM_YM2612, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis (extended channel 3)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Master System", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Master System (with FM expansion)", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_OPLL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Master System (with FM expansion in drums mode)", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Game Boy", {
      DIV_SYSTEM_GB, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC Engine/TurboGrafx-16", {
      DIV_SYSTEM_PCE, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NES", {
      DIV_SYSTEM_NES, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NES with Konami VRC7", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_VRC7, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NES with Sunsoft 5B", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 38,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Mattel Intellivision", {
      DIV_SYSTEM_AY8910, 64, 0, 48,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Vectrex", {
      DIV_SYSTEM_AY8910, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo AES", {
      DIV_SYSTEM_YM2610_FULL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo AES (extended channel 2)", {
      DIV_SYSTEM_YM2610_FULL_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari 2600/7800", {
      DIV_SYSTEM_TIA, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari Lynx", {
      DIV_SYSTEM_LYNX, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "WonderSwan", {
      DIV_SYSTEM_SWAN, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Gamate", {
      DIV_SYSTEM_AY8910, 64, 0, 73,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Computers");
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore PET", {
      DIV_SYSTEM_PET, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore VIC-20", {
      DIV_SYSTEM_VIC20, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      0
    }
  ));
  /*
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID + Sound Expander)", {
      DIV_SYSTEM_OPL, 64, 0, 0,
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID + Sound Expander with drums mode)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 0,
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID + Sound Expander)", {
      DIV_SYSTEM_OPL, 64, 0, 0,
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID + Sound Expander with drums mode)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 0,
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      0
    }
  ));*/
  cat.systems.push_back(FurnaceGUISysDef(
    "Amiga", {
      DIV_SYSTEM_AMIGA, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + SFG-01", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 16,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + MSX-MUSIC", {
      DIV_SYSTEM_OPLL, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 16,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + MSX-MUSIC (drums mode)", {
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 16,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (48K)", {
      DIV_SYSTEM_AY8910, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Amstrad CPC", {
      DIV_SYSTEM_AY8910, 64, 0, 5,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SAM Coupé", {
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "BBC Micro", {
      DIV_SYSTEM_SMS, 64, 0, 6,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC (barebones)", {
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Covox Sound Master", {
      DIV_SYSTEM_AY8930, 64, 0, 3,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + SSI 2001", {
      DIV_SYSTEM_C64_6581, 64, 0, 2,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Game Blaster", {
      DIV_SYSTEM_SAA1099, 64, -127, 1,
      DIV_SYSTEM_SAA1099, 64, 127, 1,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + AdLib/Sound Blaster", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + AdLib/Sound Blaster (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster w/Game Blaster Compatible", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      DIV_SYSTEM_SAA1099, 64, -127, 1,
      DIV_SYSTEM_SAA1099, 64, 127, 1,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster w/Game Blaster Compatible (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      DIV_SYSTEM_SAA1099, 64, -127, 1,
      DIV_SYSTEM_SAA1099, 64, 127, 1,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster Pro 2", {
      DIV_SYSTEM_OPL3, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster Pro 2 (drums mode)", {
      DIV_SYSTEM_OPL3_DRUMS, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sharp X1", {
      DIV_SYSTEM_AY8910, 64, 0, 3,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sharp X1 + FM Addon", {
      DIV_SYSTEM_AY8910, 64, 0, 3,
      DIV_SYSTEM_YM2151, 64, 0, 2,
      0
    }
  ));
  /*
  cat.systems.push_back(FurnaceGUISysDef(
    "Sharp X68000", {
      DIV_SYSTEM_YM2151, 64, 0, 2,
      DIV_SYSTEM_MSM6258, 64, 0, 0,
      0
    }
  ));*/
  cat.systems.push_back(FurnaceGUISysDef(
    "Commander X16", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_VERA, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Arcade systems");
  cat.systems.push_back(FurnaceGUISysDef(
    "Bally Midway MCR", {
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Kyugo", {
      DIV_SYSTEM_AY8910, 64, 0, 4,
      DIV_SYSTEM_AY8910, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega OutRun/X Board", {
      DIV_SYSTEM_YM2151, 64, 0, 2,
      DIV_SYSTEM_SEGAPCM, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo MVS", {
      DIV_SYSTEM_YM2610_FULL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo MVS (extended channel 2)", {
      DIV_SYSTEM_YM2610_FULL_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Taito Arcade", {
      DIV_SYSTEM_YM2610B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Taito Arcade (extended channel 3)", {
      DIV_SYSTEM_YM2610B_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom CPS-2 (QSound)", {
      DIV_SYSTEM_QSOUND, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seta 1", {
      DIV_SYSTEM_X1_010, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seta 2", {
      DIV_SYSTEM_X1_010, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Bubble System", {
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_BUBSYS_WSG, 64, 0, 0,
      // VLM5030 exists but not used for music at all
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("DefleMask-compatible");
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis", {
      DIV_SYSTEM_YM2612, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis (extended channel 3)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Master System", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Master System (with FM expansion)", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_OPLL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Game Boy", {
      DIV_SYSTEM_GB, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC Engine/TurboGrafx-16", {
      DIV_SYSTEM_PCE, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NES", {
      DIV_SYSTEM_NES, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NES with Konami VRC7", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_VRC7, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Arcade (YM2151 and SegaPCM)", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_SEGAPCM_COMPAT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo CD", {
      DIV_SYSTEM_YM2610, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo CD (extended channel 2)", {
      DIV_SYSTEM_YM2610_EXT, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);
}
