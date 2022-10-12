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
  sysCategories.clear();

  FurnaceGUISysCategory cat;

  cat=FurnaceGUISysCategory("FM","chips which use frequency modulation (FM) to generate sound.\nsome of these also pack more (like square and sample channels).");
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2151 (OPM)", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2203 (OPN)", {
      DIV_SYSTEM_OPN, 64, 0, 3,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2203 (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 3,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2608 (OPNA)", {
      DIV_SYSTEM_PC98, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2608 (extended channel 3)", {
      DIV_SYSTEM_PC98_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2610 (OPNB)", {
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
    "Yamaha YM2610B (OPNB2)", {
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
    "Yamaha YM2612 (OPN2)", {
      DIV_SYSTEM_YM2612, 64, 0, (int)0x80000000,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2612 (extended channel 3)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, (int)0x80000000,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2612 (OPN2) with DualPCM", {
      DIV_SYSTEM_YM2612_FRAC, 64, 0, (int)0x80000000,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2612 (extended channel 3) with DualPCM", {
      DIV_SYSTEM_YM2612_FRAC_EXT, 64, 0, (int)0x80000000,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2413 (OPLL)", {
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
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2414 (OPZ)", {
      DIV_SYSTEM_OPZ, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3438 (OPN2C)", {
      DIV_SYSTEM_YM2612, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3438 (extended channel 3)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3438 (OPN2C) with DualPCM", {
      DIV_SYSTEM_YM2612_FRAC, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3438 (extended channel 3) with DualPCM", {
      DIV_SYSTEM_YM2612_FRAC_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3526 (OPL)", {
      DIV_SYSTEM_OPL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3526 (drums mode)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha Y8950", {
      DIV_SYSTEM_Y8950, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha Y8950 (drums mode)", {
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3812 (OPL2)", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM3812 (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YMF262 (OPL3)", {
      DIV_SYSTEM_OPL3, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YMF262 (drums mode)", {
      DIV_SYSTEM_OPL3_DRUMS, 64, 0, 0,
      0
    }
  ));
  if (settings.hiddenSystems) {
    cat.systems.push_back(FurnaceGUISysDef(
      "Yamaha YMU759 (MA-2)", {
        DIV_SYSTEM_YMU759, 64, 0, 0,
        0
      }
    ));
  }
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Square","these chips generate square/pulse tones only (but may include noise).");
  cat.systems.push_back(FurnaceGUISysDef(
    "TI SN76489", {
      DIV_SYSTEM_SMS, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "TI SN76489A", {
      DIV_SYSTEM_SMS, 64, 0, 0x40,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "TI SN76496", {
      DIV_SYSTEM_SMS, 64, 0, 0x44,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NCR 8496", {
      DIV_SYSTEM_SMS, 64, 0, 0x48,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tandy PSSJ 3-voice sound", {
      DIV_SYSTEM_SMS, 64, 0, 0x4c,
      // 8 bit DAC
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega PSG (SN76489-like)", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega PSG (SN76489-like, Stereo)", {
      DIV_SYSTEM_SMS, 64, 0, 0xc,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "TI SN94624", {
      DIV_SYSTEM_SMS, 64, 0, 0x182,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "TI SN76494", {
      DIV_SYSTEM_SMS, 64, 0, 0x186,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Toshiba T6W28", {
      DIV_SYSTEM_T6W28, 64, 0, 0,
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
    "AY-3-8914", {
      DIV_SYSTEM_AY8910, 64, 0, 48,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YM2149(F)", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Philips SAA1099", {
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC Speaker", {
      DIV_SYSTEM_PCSPKR, 32, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore VIC", {
      DIV_SYSTEM_VIC20, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "OKI MSM5232", {
      DIV_SYSTEM_MSM5232, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Sample","chips/systems which use PCM or ADPCM samples for sound synthesis.");
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
  cat.systems.push_back(FurnaceGUISysDef(
    "Yamaha YMZ280B (PCMD8)", {
      DIV_SYSTEM_YMZ280B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Ricoh RF5C68", {
      DIV_SYSTEM_RF5C68, 64, 0, 0,
      0
    }
  )); 
  cat.systems.push_back(FurnaceGUISysDef(
    "OKI MSM6258", {
      DIV_SYSTEM_MSM6258, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "OKI MSM6295", {
      DIV_SYSTEM_MSM6295, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNES", {
      DIV_SYSTEM_SNES, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Wavetable","chips which use user-specified waveforms to generate sound.");
  cat.systems.push_back(FurnaceGUISysDef(
    "PC Engine", {
      DIV_SYSTEM_PCE, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore PET (pseudo-wavetable)", {
      DIV_SYSTEM_PET, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Bubble System WSG", {
      DIV_SYSTEM_BUBSYS_WSG, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami SCC", {
      DIV_SYSTEM_SCC, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami SCC+", {
      DIV_SYSTEM_SCC_PLUS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco WSG", {
      DIV_SYSTEM_NAMCO, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco C15 (8-channel mono)", {
      DIV_SYSTEM_NAMCO_15XX, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco C30 (8-channel stereo)", {
      DIV_SYSTEM_NAMCO_CUS30, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco 163", {
      DIV_SYSTEM_N163, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom Disk System (chip)", {
      DIV_SYSTEM_FDS, 64, 0, 0,
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
    "Virtual Boy", {
      DIV_SYSTEM_VBOY, 64, 0, 0,
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

  cat=FurnaceGUISysCategory("Specialized","chips/systems with unique sound synthesis methods.");
  cat.systems.push_back(FurnaceGUISysDef(
    "MOS Technology SID (6581)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MOS Technology SID (8580)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Microchip AY8930", {
      DIV_SYSTEM_AY8930, 64, 0, 0,
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
    "Atari Lynx", {
      DIV_SYSTEM_LYNX, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari TIA", {
      DIV_SYSTEM_TIA, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commander X16 (VERA only)", {
      DIV_SYSTEM_VERA, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (beeper only)", {
      DIV_SYSTEM_SFX_BEEPER, 64, 0, 0,
      0
    }
  ));
  if (settings.hiddenSystems) {
    cat.systems.push_back(FurnaceGUISysDef(
      "Dummy System", {
        DIV_SYSTEM_DUMMY, 64, 0, 0,
        0
      }
    ));
  }
  cat.systems.push_back(FurnaceGUISysDef(
    "tildearrow Sound Unit", {
      DIV_SYSTEM_SOUND_UNIT, 64, 0, 0,
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Game consoles","let's play some chiptune making games!");
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
    "Sega Genesis (Fractal Sound template)", {
      DIV_SYSTEM_YM2612_FRAC, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis (Fractal Sound template, extended channel 3)", {
      DIV_SYSTEM_YM2612_FRAC_EXT, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis (with Sega CD)", {
      DIV_SYSTEM_YM2612, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      DIV_SYSTEM_RF5C68, 64, 0, 18,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Genesis (extended channel 3 with Sega CD)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 0,
      DIV_SYSTEM_SMS, 24, 0, 0,
      DIV_SYSTEM_RF5C68, 64, 0, 18,
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
    "Sega Game Gear", {
      DIV_SYSTEM_SMS, 64, 0, 0xc,
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
    "Famicom with Konami VRC6", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_VRC6, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom with Konami VRC7", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_VRC7, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom with MMC5", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_MMC5, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom with Sunsoft 5B", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 32,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom with Namco 163", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_N163, 64, 0, 112,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Comboy with Family Noraebang", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_OPLL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Comboy with Family Noraebang (drums mode)", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom Disk System", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_FDS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNES", {
      DIV_SYSTEM_SNES, 64, 0, 0,
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
      DIV_SYSTEM_YM2610_FULL, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Neo Geo AES (extended channel 2)", {
      DIV_SYSTEM_YM2610_FULL_EXT, 64, 0, 1,
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
    "Virtual Boy", {
      DIV_SYSTEM_VBOY, 64, 0, 0,
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

  cat=FurnaceGUISysCategory("Computers","let's get to work on chiptune today.");
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
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID + Sound Expander)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      DIV_SYSTEM_OPL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID + Sound Expander in drums mode)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID + Sound Expander)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      DIV_SYSTEM_OPL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID + Sound Expander in drums mode)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 0,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID + FM-YAM)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      DIV_SYSTEM_OPL2, 64, 0, 0,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (6581 SID + FM-YAM in drums mode)", {
      DIV_SYSTEM_C64_6581, 64, 0, 1,
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID + FM-YAM)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      DIV_SYSTEM_OPL2, 64, 0, 0,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "Commodore 64 (8580 SID + FM-YAM in drums mode)", {
      DIV_SYSTEM_C64_8580, 64, 0, 1,
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      0
    }
  ));
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
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_YM2151, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + MSX-AUDIO", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_Y8950, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + MSX-AUDIO (drums mode)", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + MSX-MUSIC", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_OPLL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + MSX-MUSIC (drums mode)", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + Darky", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_AY8930, 64, 0, 139, // 3.58MHz
      DIV_SYSTEM_AY8930, 64, 0, 139, // 3.58MHz or 3.6MHz selectable via register
      // per-channel mixer (soft panning, post processing) isn't emulated at all
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "MSX + Playsoniq", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_SMS, 64, 0, 0, // Sega VDP
      DIV_SYSTEM_C64_8580, 64, 0, 0,
      DIV_SYSTEM_SCC_PLUS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + SCC", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_SCC, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + SCC+", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_SCC_PLUS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + Neotron", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_YM2610_FULL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + Neotron (extended channel 2)", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_YM2610_FULL_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + Neotron (with YM2610B)", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_YM2610B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + Neotron (with YM2610B; extended channel 3)", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_YM2610B_EXT, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "MSX + SIMPL", {
      DIV_SYSTEM_AY8910, 64, 0, 16,
      DIV_SYSTEM_PCM_DAC, 64, 0, 55929|(7<<16), // variable rate, Mono DAC
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with PC-9801-26/K)", {
      DIV_SYSTEM_OPN, 64, 0, 4, // 3.9936MHz but some compatible card has 4MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with PC-9801-26/K; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 4, // 3.9936MHz but some compatible card has 4MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra)", {
      DIV_SYSTEM_OPN, 64, 0, 4,
      DIV_SYSTEM_OPL2, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 4,
      DIV_SYSTEM_OPL2, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra in drums mode)", {
      DIV_SYSTEM_OPN, 64, 0, 4,
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra in drums mode; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 4,
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra V)", {
      DIV_SYSTEM_OPN, 64, 0, 4,
      DIV_SYSTEM_Y8950, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra V; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 4,
      DIV_SYSTEM_Y8950, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra V in drums mode)", {
      DIV_SYSTEM_OPN, 64, 0, 4,
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Orchestra V in drums mode; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 4,
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 4,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with PC-9801-86)", { // -73 also has OPNA
      DIV_SYSTEM_PC98, 64, 0, 1,
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16), // 2x 16-bit Burr Brown DAC
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16),
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with PC-9801-86; extended channel 3)", { // -73 also has OPNA
      DIV_SYSTEM_PC98_EXT, 64, 0, 1,
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16),
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16),
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with PC-9801-73)", {
      DIV_SYSTEM_PC98, 64, 0, 1,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with PC-9801-73; extended channel 3)", {
      DIV_SYSTEM_PC98_EXT, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible)", {
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16)|(1<<20),
      DIV_SYSTEM_OPL3, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16)|(1<<20),
      DIV_SYSTEM_OPL3, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode)", {
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16)|(1<<20),
      DIV_SYSTEM_OPL3_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NEC PC-98 (with Sound Blaster 16 for PC-9800 w/PC-9801-26/K compatible in drums mode; extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16)|(1<<20),
      DIV_SYSTEM_OPL3_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (48K)", {
      DIV_SYSTEM_AY8910, 64, 0, 2,
      DIV_SYSTEM_SFX_BEEPER, 64, 0, 0,
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
    "ZX Spectrum (128K) with TurboSound FM", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM (extended channel 3 on first OPN)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM (extended channel 3 on second OPN)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM (extended channel 3 on both OPNs)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM + SAA", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM + SAA (extended channel 3 on first OPN)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM + SAA (extended channel 3 on second OPN)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound FM + SAA (extended channel 3 on both OPNs)", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      DIV_SYSTEM_OPN_EXT, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ZX Spectrum (128K) with TurboSound", {
      DIV_SYSTEM_AY8910, 64, 0, 1,
      DIV_SYSTEM_AY8910, 64, 0, 1, // or YM2149
      DIV_SYSTEM_AY8910, 64, 0, 1, // or YM2149
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
    "Atari ST", {
      DIV_SYSTEM_AY8910, 64, 0, 19,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "Atari STE", {
      DIV_SYSTEM_AY8910, 64, 0, 19,
      DIV_SYSTEM_PCM_DAC, 64, 0, 50667|(7<<16),
      DIV_SYSTEM_PCM_DAC, 64, 0, 50667|(7<<16),
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
      DIV_SYSTEM_SMS, 64, 0, 0x42, // SN76489A 4MHz
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
    "IBM PCjr", {
      // it can be enable sound output at once
      DIV_SYSTEM_SMS, 64, 0, 0x44, // SN76496
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tandy 1000", {
      DIV_SYSTEM_SMS, 64, 0, 0x44, // NCR 8496 or SN76496 or Tandy PSSJ(with 8 bit DAC)
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
      DIV_SYSTEM_SAA1099, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 1,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
    cat.systems.push_back(FurnaceGUISysDef(
    "PC + AdLib", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
      cat.systems.push_back(FurnaceGUISysDef(
    "PC + AdLib (drums mode)", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 22049|(7<<16),
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + AdLib/Sound Blaster (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 22049|(7<<16),
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster w/Game Blaster Compatible", {
      DIV_SYSTEM_OPL2, 64, 0, 0,
      DIV_SYSTEM_SAA1099, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 1,
      DIV_SYSTEM_PCM_DAC, 64, 0, 22049|(7<<16),
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster w/Game Blaster Compatible (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0,
      DIV_SYSTEM_SAA1099, 64, 0, 1,
      DIV_SYSTEM_SAA1099, 64, 0, 1,
      DIV_SYSTEM_PCM_DAC, 64, 0, 22049|(7<<16),
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster Pro", {
      DIV_SYSTEM_OPL2, 64, -127, 0,
      DIV_SYSTEM_OPL2, 64, 127, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 22049|(7<<16)|(1<<20), //alternatively 44.1 khz mono
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster Pro (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, -127, 0,
      DIV_SYSTEM_OPL2_DRUMS, 64, 127, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 22049|(7<<16)|(1<<20), //alternatively 44.1 khz mono
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster Pro 2", {
      DIV_SYSTEM_OPL3, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16)|(1<<20),
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + Sound Blaster Pro 2 (drums mode)", {
      DIV_SYSTEM_OPL3_DRUMS, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 44099|(15<<16)|(1<<20),
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + PC-FXGA", {
      DIV_SYSTEM_PCE, 64, 0, 0, // HuC6230 (WSG from HuC6280 but with built in 2 OKI ADPCM playback engines)
      DIV_SYSTEM_PCSPKR, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "PC + SAAYM", {
      DIV_SYSTEM_YM2151, 64, 0, 0, // 3.58MHz or 4MHz selectable via jumper
      DIV_SYSTEM_SAA1099, 64, 0, 1, // 7.16MHz or 8MHz selectable via jumper
      DIV_SYSTEM_SAA1099, 64, 0, 1, // ""
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
  cat.systems.push_back(FurnaceGUISysDef(
    "Sharp X68000", {
      DIV_SYSTEM_YM2151, 64, 0, 2,
      DIV_SYSTEM_MSM6258, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "FM Towns", {
      DIV_SYSTEM_YM2612, 64, 0, 2, // YM3438
      DIV_SYSTEM_RF5C68, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Commander X16", {
      DIV_SYSTEM_VERA, 64, 0, 0,
      DIV_SYSTEM_YM2151, 32, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "TI-99/4A", {
      DIV_SYSTEM_SMS, 64, 0, 0x182, // SN94624 447KHz
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("Arcade systems","INSERT COIN");
  cat.systems.push_back(FurnaceGUISysDef(
    "Bally Midway MCR", {
      // SSIO sound board
      DIV_SYSTEM_AY8910, 64, 0, 3, // 2MHz
      DIV_SYSTEM_AY8910, 64, 0, 3, // 2MHz
      // additional sound boards, mostly software controlled DAC
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Williams/Midway Y/T unit w/ADPCM sound board", {
      // ADPCM sound board
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 15624|(7<<16), // variable via OPM timer?
      DIV_SYSTEM_MSM6295, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Gyruss", {
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 0,
      DIV_SYSTEM_AY8910, 64, 0, 0,
      // additional discrete sound logics
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
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Battlantis", {
      DIV_SYSTEM_OPL2, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPL2, 64, 0, 3, // ""
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Battlantis (drums mode on first OPL2)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPL2, 64, 0, 3, // ""
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Battlantis (drums mode on second OPL2)", {
      DIV_SYSTEM_OPL2, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 3, // ""
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Battlantis (drums mode on both OPL2s)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 3, // ""
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Konami Hexion", {
      DIV_SYSTEM_SCC, 64, 0, 2, // 1.5MHz (3MHz input)
      DIV_SYSTEM_MSM6295, 64, 0, 1,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Kyugo", {
      DIV_SYSTEM_AY8910, 64, 0, 14,
      DIV_SYSTEM_AY8910, 64, 0, 14,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 1", {
      DIV_SYSTEM_SMS, 64, 0, 0x42, // SN76489A 4MHz
      DIV_SYSTEM_SMS, 64, 0, 0x0141, // SN76489A 2MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System E", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_SMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System E (with FM expansion)", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_OPLL, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System E (with FM expansion in drums mode)", {
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_SMS, 64, 0, 0,
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Hang-On", {
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_SEGAPCM, 64, 0, 0, // discrete logics, 62.5KHz output rate
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega Hang-On (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_SEGAPCM, 64, 0, 0, // discrete logics, 62.5KHz output rate
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega OutRun/X Board", {
      DIV_SYSTEM_YM2151, 64, 0, 2, // 4MHz
      DIV_SYSTEM_SEGAPCM, 64, 0, 0, // ASIC, 31.25KHz output rate
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 24", {
      DIV_SYSTEM_YM2151, 64, 0, 2, // 4MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 61499|(7<<16), // software controlled, variable rate via configurable timers
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 18", {
      DIV_SYSTEM_YM2612, 64, 0, 2, // discrete 8MHz YM3438
      DIV_SYSTEM_YM2612, 64, 0, 2, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 1, // 10MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 18 (extended channel 3 on first OPN2C)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 2, // discrete 8MHz YM3438
      DIV_SYSTEM_YM2612, 64, 0, 2, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 1, // 10MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 18 (extended channel 3 on second OPN2C)", {
      DIV_SYSTEM_YM2612, 64, 0, 2, // discrete 8MHz YM3438
      DIV_SYSTEM_YM2612_EXT, 64, 0, 2, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 1, // 10MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 18 (extended channel 3 on both OPN2Cs)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 2, // discrete 8MHz YM3438
      DIV_SYSTEM_YM2612_EXT, 64, 0, 2, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 1, // 10MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 32", {
      DIV_SYSTEM_YM2612, 64, 0, 4, // discrete 8.05MHz YM3438
      DIV_SYSTEM_YM2612, 64, 0, 4, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 2, // 12.5MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 32 (extended channel 3 on first OPN2C)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 4, // discrete 8.05MHz YM3438
      DIV_SYSTEM_YM2612, 64, 0, 4, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 2, // 12.5MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 32 (extended channel 3 on second OPN2C)", {
      DIV_SYSTEM_YM2612, 64, 0, 4, // discrete 8.05MHz YM3438
      DIV_SYSTEM_YM2612_EXT, 64, 0, 4, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 2, // 12.5MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sega System 32 (extended channel 3 on both OPN2Cs)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 4, // discrete 8.05MHz YM3438
      DIV_SYSTEM_YM2612_EXT, 64, 0, 4, // ^^
      DIV_SYSTEM_RF5C68, 64, 0, 2, // 12.5MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom Exed Eyes", {
      DIV_SYSTEM_AY8910, 64, 0, 4, // 1.5MHz
      DIV_SYSTEM_SMS, 64, 0, 0x0104, // SN76489 3MHz
      DIV_SYSTEM_SMS, 64, 0, 0x0104, // SN76489 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom Arcade", { // 1943, Side arms, etc
      DIV_SYSTEM_OPN, 64, 0, 5, // 4 or 1.5MHz; various per games
      DIV_SYSTEM_OPN, 64, 0, 5,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom Arcade (extended channel 3 on first OPN)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 5,
      DIV_SYSTEM_OPN, 64, 0, 5,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom Arcade (extended channel 3 on second OPN)", {
      DIV_SYSTEM_OPN, 64, 0, 5,
      DIV_SYSTEM_OPN_EXT, 64, 0, 5,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom Arcade (extended channel 3 on both OPNs)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 5,
      DIV_SYSTEM_OPN_EXT, 64, 0, 5,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Capcom CPS-1", { 
      DIV_SYSTEM_YM2151, 64, 0, 0, // 3.58MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0,
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
    "Jaleco Ginga NinkyouDen", {
      DIV_SYSTEM_AY8910, 64, 0, 16, // 1.79MHz
      DIV_SYSTEM_Y8950, 64, 0, 0, // 3.58MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Jaleco Ginga NinkyouDen (drums mode)", {
      DIV_SYSTEM_AY8910, 64, 0, 16, // 1.79MHz
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 0, // 3.58MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Jaleco Mega System 1", {
      DIV_SYSTEM_YM2151, 64, 0, 1, // 3.5MHz (7MHz / 2)
      DIV_SYSTEM_MSM6295, 64, 0, 2, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 2, // 4MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NMK 16-bit Arcade", {
      DIV_SYSTEM_OPN, 64, 0, 5, // 1.5MHz; optional
      DIV_SYSTEM_MSM6295, 64, 0, 130, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 130, // ^^
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "NMK 16-bit Arcade (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 5, // 1.5MHz; optional
      DIV_SYSTEM_MSM6295, 64, 0, 130, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 130, // ^^
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Kaneko DJ Boy", {
      DIV_SYSTEM_OPN, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, -127, 12, // 1.5MHz, Left output
      DIV_SYSTEM_MSM6295, 64, 127, 12, // 1.5MHz, Right output
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Kaneko DJ Boy (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, -127, 12, // 1.5MHz, Left output
      DIV_SYSTEM_MSM6295, 64, 127, 12, // 1.5MHz, Right output
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Kaneko Air Buster", {
      DIV_SYSTEM_OPN, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, 0, 141, // 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Kaneko Air Buster (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, 0, 141, // 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Kaneko Toybox System", {
      DIV_SYSTEM_AY8910, 64, 0, 19, // YM2149 2MHz
      DIV_SYSTEM_AY8910, 64, 0, 19, // ^^
      DIV_SYSTEM_MSM6295, 64, 0, 8, // 2MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Kaneko Jackie Chan", {
      DIV_SYSTEM_YMZ280B, 64, 0, 3, // 16MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Super Kaneko Nova System", {
      DIV_SYSTEM_YMZ280B, 64, 0, 4, // 16.67MHz (33.33MHz / 2)
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tecmo Ninja Gaiden", { // Ninja Gaiden, Raiga, etc
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tecmo Ninja Gaiden (extended channel 3 on first OPN)", { // Ninja Gaiden, Raiga, etc
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tecmo Ninja Gaiden (extended channel 3 on second OPN)", { // Ninja Gaiden, Raiga, etc
      DIV_SYSTEM_OPN, 64, 0, 2, // 4MHz
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tecmo Ninja Gaiden (extended channel 3 on both OPNs)", { // Ninja Gaiden, Raiga, etc
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tecmo System", {
      DIV_SYSTEM_OPL3, 64, 0, 0,
      DIV_SYSTEM_YMZ280B, 64, 0, 0,
      DIV_SYSTEM_MSM6295, 64, 0, 8, // 2MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Tecmo System (drums mode)", {
      DIV_SYSTEM_OPL3_DRUMS, 64, 0, 0,
      DIV_SYSTEM_YMZ280B, 64, 0, 0,
      DIV_SYSTEM_MSM6295, 64, 0, 8, // 2MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seibu Kaihatsu Raiden", { // Raiden, Seibu cup soccer, Zero team, etc
      DIV_SYSTEM_OPL2, 64, 0, 0, // YM3812 3.58MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 or 1.023MHz (28.636363MHz / 28); various per games
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seibu Kaihatsu Raiden (drums mode)", { // Raiden, Seibu cup soccer, Zero team, etc
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 0, // YM3812 3.58MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 or 1.023MHz (28.636363MHz / 28); various per games
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sunsoft Shanghai 3", {
      DIV_SYSTEM_AY8910, 64, 0, 20, // YM2149 1.5MHz
      DIV_SYSTEM_MSM6295, 64, 0, 1, // 1.056MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sunsoft Arcade", {
      DIV_SYSTEM_YM2612, 64, 0, 2, // discrete YM3438 8MHz
      DIV_SYSTEM_MSM6295, 64, 0, 1, // 1.056MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Sunsoft Arcade (extended channel 3)", {
      DIV_SYSTEM_YM2612_EXT, 64, 0, 2, // discrete YM3438 8MHz
      DIV_SYSTEM_MSM6295, 64, 0, 1, // 1.056MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari Klax", { 
      DIV_SYSTEM_MSM6295, 64, 0, 7, // 0.895MHz (3.579545MHz / 4)
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari Rampart", { 
      DIV_SYSTEM_OPLL, 64, 0, 0, // 3.579545MHz
      DIV_SYSTEM_MSM6295, 64, 0, 14, // 1.193MHz (3.579545MHz / 3)
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari Rampart (drums mode)", { 
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0, // 3.579545MHz
      DIV_SYSTEM_MSM6295, 64, 0, 14, // 1.193MHz (3.579545MHz / 3)
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Atari JSA IIIs", { 
      DIV_SYSTEM_YM2151, 64, 0, 0, // 3.579545MHz
      DIV_SYSTEM_MSM6295, 64, -127, 14, // 1.193MHz (3.579545MHz / 3), Left output
      DIV_SYSTEM_MSM6295, 64, 127, 14, // 1.193MHz (3.579545MHz / 3), Right output
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Karnov", {
      DIV_SYSTEM_OPN, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL, 64, 0, 3, // 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Karnov (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL, 64, 0, 3, // 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Karnov (drums mode)", {
      DIV_SYSTEM_OPN, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 3, // 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Karnov (extended channel 3; drums mode)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 3, // 3MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Arcade", { // Bad dudes, Robocop, etc
      DIV_SYSTEM_OPN, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL2, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 to 1.056MHz; various per games or optional
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Arcade (extended channel 3)", { // Bad dudes, Robocop, etc
      DIV_SYSTEM_OPN_EXT, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL2, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 to 1.056MHz; various per games or optional
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Arcade (drums mode)", { // Bad dudes, Robocop, etc
      DIV_SYSTEM_OPN, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 to 1.056MHz; various per games or optional
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Arcade (extended channel 3; drums mode)", { // Bad dudes, Robocop, etc
      DIV_SYSTEM_OPN_EXT, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 3, // 3MHz
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 to 1.056MHz; various per games or optional
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East PCX", {
      DIV_SYSTEM_OPN, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_PCE, 64, 0, 0,
      // software controlled MSM5205
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East PCX (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 5, // 1.5MHz
      DIV_SYSTEM_PCE, 64, 0, 0,
      // software controlled MSM5205
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Dark Seal", { // Dark Seal, Crude Buster, Vapor Trail, etc
      DIV_SYSTEM_YM2151, 64, 0, 0, // 3.580MHz (32.22MHz / 9)
      DIV_SYSTEM_OPN, 64, 0, 2, // 4.0275MHz (32.22MHz / 8); optional
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1.007MHz (32.22MHz / 32)
      DIV_SYSTEM_MSM6295, 64, 0, 8, // 2.014MHz (32.22MHz / 16); optional
      // HuC6280 is for control them, internal sound isn't used
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Dark Seal (extended channel 3)", { // Dark Seal, Crude Buster, Vapor Trail, etc
      DIV_SYSTEM_YM2151, 64, 0, 0, // 3.580MHz (32.22MHz / 9)
      DIV_SYSTEM_OPN_EXT, 64, 0, 2, // 4.0275MHz (32.22MHz / 8); optional
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1.007MHz (32.22MHz / 32)
      DIV_SYSTEM_MSM6295, 64, 0, 8, // 2.014MHz (32.22MHz / 16); optional
      // HuC6280 is for control them, internal sound isn't used
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East Deco 156", {
      DIV_SYSTEM_MSM6295, 64, 0, 0, // 1 or 1.007MHz (32.22MHz / 32); various per games
      DIV_SYSTEM_MSM6295, 64, 0, 8, // 1 or 2 or 2.014MHz (32.22MHz / 16); various per games
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Data East MLC", {
      DIV_SYSTEM_YMZ280B, 64, 0, 5, // 14MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Ikari Warriors", {
      DIV_SYSTEM_OPL, 64, 0, 2,
      DIV_SYSTEM_OPL, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Ikari Warriors (drums mode on first OPL)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      DIV_SYSTEM_OPL, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Ikari Warriors (drums mode on second OPL)", {
      DIV_SYSTEM_OPL, 64, 0, 2,
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Ikari Warriors (drums mode on both OPLs)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Triple Z80", {
      DIV_SYSTEM_Y8950, 64, 0, 2,
      DIV_SYSTEM_OPL, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Triple Z80 (drums mode on Y8950)", {
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 2,
      DIV_SYSTEM_OPL, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Triple Z80 (drums mode on OPL)", {
      DIV_SYSTEM_Y8950, 64, 0, 2,
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Triple Z80 (drums mode on Y8950 and OPL)", {
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 2,
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Chopper I", {
      DIV_SYSTEM_Y8950, 64, 0, 2,
      DIV_SYSTEM_OPL2, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Chopper I (drums mode on Y8950)", {
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 2,
      DIV_SYSTEM_OPL2, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Chopper I (drums mode on OPL2)", {
      DIV_SYSTEM_Y8950, 64, 0, 2,
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Chopper I (drums mode on Y8950 and OPL2)", {
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 2,
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Touchdown Fever", {
      DIV_SYSTEM_OPL, 64, 0, 2,
      DIV_SYSTEM_Y8950, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Touchdown Fever (drums mode on OPL)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      DIV_SYSTEM_Y8950, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Touchdown Fever (drums mode on Y8950)", {
      DIV_SYSTEM_OPL, 64, 0, 2,
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "SNK Touchdown Fever (drums mode on OPL and Y8950)", {
      DIV_SYSTEM_OPL_DRUMS, 64, 0, 2,
      DIV_SYSTEM_Y8950_DRUMS, 64, 0, 2,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Alpha denshi Alpha-68K", {
      DIV_SYSTEM_OPN, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPLL, 64, 0, 0, // 3.58MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 7613|(7<<16), // software controlled 8 bit DAC
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Alpha denshi Alpha-68K (extended channel 3)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPLL, 64, 0, 0, // 3.58MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 7613|(7<<16), // software controlled 8 bit DAC
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Alpha denshi Alpha-68K (drums mode)", {
      DIV_SYSTEM_OPN, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0, // 3.58MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 7613|(7<<16), // software controlled 8 bit DAC
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Alpha denshi Alpha-68K (extended channel 3; drums mode)", {
      DIV_SYSTEM_OPN_EXT, 64, 0, 3, // 3MHz
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0, // 3.58MHz
      DIV_SYSTEM_PCM_DAC, 64, 0, 7613|(7<<16), // software controlled 8 bit DAC
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
    "Nichibutsu Mag Max", {
      DIV_SYSTEM_AY8910, 64, 0, 13,
      DIV_SYSTEM_AY8910, 64, 0, 13,
      DIV_SYSTEM_AY8910, 64, 0, 13,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco (3-channel WSG)", { // Pac-Man, Galaga, Xevious, etc
      DIV_SYSTEM_NAMCO, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco Mappy", { // Mappy, Super Pac-Man, Libble Rabble, etc
      DIV_SYSTEM_NAMCO_15XX, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco Pac-Land", { // Pac-Land, Baraduke, Sky kid, etc
      DIV_SYSTEM_NAMCO_CUS30, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco System 86", { // without expansion board case; Hopping Mappy, etc
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_NAMCO_CUS30, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco Thunder Ceptor", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_NAMCO_CUS30, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 7999|(7<<16), // M65C02 software driven, correct sample rate?
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Namco System 1", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_NAMCO_CUS30, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 5999|(7<<16), // sample rate verified from https://github.com/mamedev/mame/blob/master/src/devices/sound/n63701x.cpp
      DIV_SYSTEM_PCM_DAC, 64, 0, 5999|(7<<16), // ""
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
    "Seta 1", {
      DIV_SYSTEM_X1_010, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seta 1 + FM addon", {
      DIV_SYSTEM_X1_010, 64, 0, 0,
      DIV_SYSTEM_YM2612, 64, 0, 2, // Discrete YM3438
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Seta 1 + FM addon (extended channel 3)", {
      DIV_SYSTEM_X1_010, 64, 0, 0,
      DIV_SYSTEM_YM2612_EXT, 64, 0, 2, // Discrete YM3438
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
    "Cave 68000", {
      DIV_SYSTEM_YMZ280B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Coreland Cyber Tank", {
      DIV_SYSTEM_Y8950, 64, -127, 0, // 3.58MHz, Left output
      DIV_SYSTEM_Y8950, 64, 127, 0, // 3.58MHz, Right output
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Coreland Cyber Tank (drums mode)", {
      DIV_SYSTEM_Y8950, 64, -127, 0, // 3.58MHz, Left output
      DIV_SYSTEM_Y8950, 64, 127, 0, // 3.58MHz, Right output
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "ICE Skimaxx", {
      DIV_SYSTEM_MSM6295, 64, -127, 130, // 4MHz, Left output
      DIV_SYSTEM_MSM6295, 64, 127, 130, // 4MHz, Right output
      DIV_SYSTEM_MSM6295, 64, -127, 8, // 2MHz, Left output
      DIV_SYSTEM_MSM6295, 64, 127, 8, // 2MHz, Right output
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Toaplan 1", {
      DIV_SYSTEM_OPL2, 64, 0, 5, // 3.5MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Toaplan 1 (drums mode)", {
      DIV_SYSTEM_OPL2_DRUMS, 64, 0, 5, // 3.5MHz
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Dynax/Nakanihon 3rd generation hardware", {
      DIV_SYSTEM_AY8910, 64, 0, 0, // AY or YM, optional - 1.79MHz or 3.58MHz; various per game
      DIV_SYSTEM_OPLL, 64, 0, 0,
      DIV_SYSTEM_MSM6295, 64, 0, 6, // 1.023MHz mostly
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Dynax/Nakanihon 3rd generation hardware (drums mode)", {
      DIV_SYSTEM_AY8910, 64, 0, 0, // AY or YM, optional - 1.79MHz or 3.58MHz; various per game
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      DIV_SYSTEM_MSM6295, 64, 0, 6, // 1.023MHz mostly
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Dynax/Nakanihon Real Break", {
      DIV_SYSTEM_OPLL, 64, 0, 0,
      DIV_SYSTEM_YMZ280B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Dynax/Nakanihon Real Break (drums mode)", {
      DIV_SYSTEM_OPLL_DRUMS, 64, 0, 0,
      DIV_SYSTEM_YMZ280B, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Irem M72", {
      DIV_SYSTEM_YM2151, 64, 0, 0,
      DIV_SYSTEM_PCM_DAC, 64, 0, 7811|(7<<16),
      0
    }
  ));
  sysCategories.push_back(cat);

  cat=FurnaceGUISysCategory("DefleMask-compatible","these configurations are compatible with DefleMask.\nselect this if you need to save as .dmf or work with that program.");
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
    "Famicom with Konami VRC7", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_VRC7, 64, 0, 0,
      0
    }
  ));
  cat.systems.push_back(FurnaceGUISysDef(
    "Famicom Disk System", {
      DIV_SYSTEM_NES, 64, 0, 0,
      DIV_SYSTEM_FDS, 64, 0, 0,
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
