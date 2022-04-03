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

#include "engine.h"
#include "song.h"

DivSystem DivEngine::systemFromFileFur(unsigned char val) {
  switch (val) {
    case 0x01:
      return DIV_SYSTEM_YMU759;
    case 0x02:
      return DIV_SYSTEM_GENESIS;
    case 0x03:
      return DIV_SYSTEM_SMS;
    case 0x04:
      return DIV_SYSTEM_GB;
    case 0x05:
      return DIV_SYSTEM_PCE;
    case 0x06:
      return DIV_SYSTEM_NES;
    case 0x07:
      return DIV_SYSTEM_C64_8580;
    case 0x08:
      return DIV_SYSTEM_ARCADE;
    case 0x09:
      return DIV_SYSTEM_YM2610;
    case 0x42:
      return DIV_SYSTEM_GENESIS_EXT;
    case 0x43:
      return DIV_SYSTEM_SMS_OPLL;
    case 0x46:
      return DIV_SYSTEM_NES_VRC7;
    case 0x47:
      return DIV_SYSTEM_C64_6581;
    case 0x49:
      return DIV_SYSTEM_YM2610_EXT;
    case 0x80:
      return DIV_SYSTEM_AY8910;
    case 0x81:
      return DIV_SYSTEM_AMIGA;
    case 0x82:
      return DIV_SYSTEM_YM2151;
    case 0x83:
      return DIV_SYSTEM_YM2612;
    case 0x84:
      return DIV_SYSTEM_TIA;
    case 0x85:
      return DIV_SYSTEM_VIC20;
    case 0x86:
      return DIV_SYSTEM_PET;
    case 0x87:
      return DIV_SYSTEM_SNES;
    case 0x88:
      return DIV_SYSTEM_VRC6;
    case 0x89:
      return DIV_SYSTEM_OPLL;
    case 0x8a:
      return DIV_SYSTEM_FDS;
    case 0x8b:
      return DIV_SYSTEM_MMC5;
    case 0x8c:
      return DIV_SYSTEM_N163;
    case 0x8d:
      return DIV_SYSTEM_OPN;
    case 0x8e:
      return DIV_SYSTEM_PC98;
    case 0x8f:
      return DIV_SYSTEM_OPL;
    case 0x90:
      return DIV_SYSTEM_OPL2;
    case 0x91:
      return DIV_SYSTEM_OPL3;
    case 0x92:
      return DIV_SYSTEM_MULTIPCM;
    case 0x93:
      return DIV_SYSTEM_PCSPKR;
    case 0x94:
      return DIV_SYSTEM_POKEY;
    case 0x95:
      return DIV_SYSTEM_RF5C68;
    case 0x96:
      return DIV_SYSTEM_SWAN;
    case 0x97:
      return DIV_SYSTEM_SAA1099;
    case 0x98:
      return DIV_SYSTEM_OPZ;
    case 0x99:
      return DIV_SYSTEM_POKEMINI;
    case 0x9a:
      return DIV_SYSTEM_AY8930;
    case 0x9b:
      return DIV_SYSTEM_SEGAPCM;
    case 0x9c:
      return DIV_SYSTEM_VBOY;
    case 0x9d:
      return DIV_SYSTEM_VRC7;
    case 0x9e:
      return DIV_SYSTEM_YM2610B;
    case 0x9f:
      return DIV_SYSTEM_SFX_BEEPER;
    case 0xa0:
      return DIV_SYSTEM_YM2612_EXT;
    case 0xa1:
      return DIV_SYSTEM_SCC;
    case 0xa2:
      return DIV_SYSTEM_OPL_DRUMS;
    case 0xa3:
      return DIV_SYSTEM_OPL2_DRUMS;
    case 0xa4:
      return DIV_SYSTEM_OPL3_DRUMS;
    case 0xa5:
      return DIV_SYSTEM_YM2610_FULL;
    case 0xa6:
      return DIV_SYSTEM_YM2610_FULL_EXT;
    case 0xa7:
      return DIV_SYSTEM_OPLL_DRUMS;
    case 0xa8:
      return DIV_SYSTEM_LYNX;
    case 0xa9:
      return DIV_SYSTEM_SEGAPCM_COMPAT;
    case 0xac:
      return DIV_SYSTEM_VERA;
    case 0xad:
      return DIV_SYSTEM_BUBSYS_WSG;
    case 0xb0:
      return DIV_SYSTEM_X1_010;
    case 0xde:
      return DIV_SYSTEM_YM2610B_EXT;
    case 0xe0:
      return DIV_SYSTEM_QSOUND;
  }
  return DIV_SYSTEM_NULL;
}

unsigned char DivEngine::systemToFileFur(DivSystem val) {
  switch (val) {
    case DIV_SYSTEM_YMU759:
      return 0x01;
    case DIV_SYSTEM_GENESIS:
      return 0x02;
    case DIV_SYSTEM_SMS:
      return 0x03;
    case DIV_SYSTEM_GB:
      return 0x04;
    case DIV_SYSTEM_PCE:
      return 0x05;
    case DIV_SYSTEM_NES:
      return 0x06;
    case DIV_SYSTEM_C64_8580:
      return 0x07;
    case DIV_SYSTEM_ARCADE:
      return 0x08;
    case DIV_SYSTEM_YM2610:
      return 0x09;
    case DIV_SYSTEM_GENESIS_EXT:
      return 0x42;
    case DIV_SYSTEM_SMS_OPLL:
      return 0x43;
    case DIV_SYSTEM_NES_VRC7:
      return 0x46;
    case DIV_SYSTEM_C64_6581:
      return 0x47;
    case DIV_SYSTEM_YM2610_EXT:
      return 0x49;
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return 0x80;
    case DIV_SYSTEM_AMIGA:
      return 0x81;
    case DIV_SYSTEM_YM2151:
      return 0x82;
    case DIV_SYSTEM_YM2612:
      return 0x83;
    case DIV_SYSTEM_TIA:
      return 0x84;
    case DIV_SYSTEM_VIC20:
      return 0x85;
    case DIV_SYSTEM_PET:
      return 0x86;
    case DIV_SYSTEM_SNES:
      return 0x87;
    case DIV_SYSTEM_VRC6:
      return 0x88;
    case DIV_SYSTEM_OPLL:
      return 0x89;
    case DIV_SYSTEM_FDS:
      return 0x8a;
    case DIV_SYSTEM_MMC5:
      return 0x8b;
    case DIV_SYSTEM_N163:
      return 0x8c;
    case DIV_SYSTEM_OPN:
      return 0x8d;
    case DIV_SYSTEM_PC98:
      return 0x8e;
    case DIV_SYSTEM_OPL:
      return 0x8f;
    case DIV_SYSTEM_OPL2:
      return 0x90;
    case DIV_SYSTEM_OPL3:
      return 0x91;
    case DIV_SYSTEM_MULTIPCM:
      return 0x92;
    case DIV_SYSTEM_PCSPKR:
      return 0x93;
    case DIV_SYSTEM_POKEY:
      return 0x94;
    case DIV_SYSTEM_RF5C68:
      return 0x95;
    case DIV_SYSTEM_SWAN:
      return 0x96;
    case DIV_SYSTEM_SAA1099:
      return 0x97;
    case DIV_SYSTEM_OPZ:
      return 0x98;
    case DIV_SYSTEM_POKEMINI:
      return 0x99;
    case DIV_SYSTEM_AY8930:
      return 0x9a;
    case DIV_SYSTEM_SEGAPCM:
      return 0x9b;
    case DIV_SYSTEM_VBOY:
      return 0x9c;
    case DIV_SYSTEM_VRC7:
      return 0x9d;
    case DIV_SYSTEM_YM2610B:
      return 0x9e;
    case DIV_SYSTEM_SFX_BEEPER:
      return 0x9f;
    case DIV_SYSTEM_YM2612_EXT:
      return 0xa0;
    case DIV_SYSTEM_SCC:
      return 0xa1;
    case DIV_SYSTEM_OPL_DRUMS:
      return 0xa2;
    case DIV_SYSTEM_OPL2_DRUMS:
      return 0xa3;
    case DIV_SYSTEM_OPL3_DRUMS:
      return 0xa4;
    case DIV_SYSTEM_YM2610_FULL:
      return 0xa5;
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return 0xa6;
    case DIV_SYSTEM_OPLL_DRUMS:
      return 0xa7;
    case DIV_SYSTEM_LYNX:
      return 0xa8;
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      return 0xa9;
    case DIV_SYSTEM_VERA:
      return 0xac;
    case DIV_SYSTEM_BUBSYS_WSG:
      return 0xad;
    case DIV_SYSTEM_X1_010:
      return 0xb0;
    case DIV_SYSTEM_YM2610B_EXT:
      return 0xde;
    case DIV_SYSTEM_QSOUND:
      return 0xe0;

    case DIV_SYSTEM_NULL:
      return 0;
  }
  return 0;
}

int DivEngine::getChannelCount(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return 0;
    case DIV_SYSTEM_YMU759:
      return 17;
    case DIV_SYSTEM_GENESIS:
      return 10;
    case DIV_SYSTEM_SMS:
    case DIV_SYSTEM_GB:
      return 4;
    case DIV_SYSTEM_PCE:
      return 6;
    case DIV_SYSTEM_NES:
      return 5;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_8580:
      return 3;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_SMS_OPLL:
      return 13;
    case DIV_SYSTEM_NES_VRC7:
      return 11;
    case DIV_SYSTEM_YM2610_EXT:
      return 16;
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      return 3;
    case DIV_SYSTEM_AMIGA:
      return 4;
    case DIV_SYSTEM_YM2151:
      return 8;
    case DIV_SYSTEM_YM2612:
      return 6;
    case DIV_SYSTEM_TIA:
      return 2;
    case DIV_SYSTEM_VIC20:
      return 4;
    case DIV_SYSTEM_PET:
      return 1;
    case DIV_SYSTEM_SNES:
      return 8;
    case DIV_SYSTEM_VRC6:
      return 3;
    case DIV_SYSTEM_OPLL:
      return 9;
    case DIV_SYSTEM_FDS:
      return 1;
    case DIV_SYSTEM_MMC5:
      return 3;
    case DIV_SYSTEM_N163:
      return 8;
    case DIV_SYSTEM_OPN:
      return 6;
    case DIV_SYSTEM_PC98:
      return 16;
    case DIV_SYSTEM_OPL:
      return 9;
    case DIV_SYSTEM_OPL2:
      return 9;
    case DIV_SYSTEM_OPL3:
      return 18;
    case DIV_SYSTEM_MULTIPCM:
      return 28;
    case DIV_SYSTEM_PCSPKR:
      return 1;
    case DIV_SYSTEM_POKEY:
      return 4;
    case DIV_SYSTEM_RF5C68:
      return 8;
    case DIV_SYSTEM_SWAN:
      return 4;
    case DIV_SYSTEM_SAA1099:
      return 6;
    case DIV_SYSTEM_OPZ:
      return 8;
    case DIV_SYSTEM_POKEMINI:
      return 1;
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_X1_010:
      return 16;
    case DIV_SYSTEM_VBOY:
      return 6;
    case DIV_SYSTEM_VRC7:
      return 6;
    case DIV_SYSTEM_YM2610B:
      return 16;
    case DIV_SYSTEM_SFX_BEEPER:
      return 6;
    case DIV_SYSTEM_YM2612_EXT:
      return 9;
    case DIV_SYSTEM_SCC:
      return 5;
    case DIV_SYSTEM_OPL_DRUMS:
      return 11;
    case DIV_SYSTEM_OPL2_DRUMS:
      return 11;
    case DIV_SYSTEM_OPL3_DRUMS:
      return 20;
    case DIV_SYSTEM_YM2610_FULL:
      return 14;      
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return 17;
    case DIV_SYSTEM_OPLL_DRUMS:
      return 11;
    case DIV_SYSTEM_LYNX:
      return 4;
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      return 5;
    case DIV_SYSTEM_YM2610B_EXT:
    case DIV_SYSTEM_QSOUND:
      return 19;
    case DIV_SYSTEM_VERA:
      return 17;
    case DIV_SYSTEM_BUBSYS_WSG:
      return 2;
  }
  return 0;
}

int DivEngine::getTotalChannelCount() {
  return chans;
}

// TODO: rewrite this function (again). it's an unreliable mess.
const char* DivEngine::getSongSystemName() {
  switch (song.systemLen) {
    case 0:
      return "help! what's going on!";
    case 1:
      if (song.system[0]==DIV_SYSTEM_AY8910) {
        switch (song.systemFlags[0]&0x3f) {
          case 0: // AY-3-8910, 1.79MHz
          case 1: // AY-3-8910, 1.77MHz
          case 2: // AY-3-8910, 1.75MHz
            return "ZX Spectrum";
          case 3: // AY-3-8910, 2MHz
            return "Fujitsu Micro-7";
          case 4: // AY-3-8910, 1.5MHz
            return "Vectrex";
          case 5: // AY-3-8910, 1MHz
            return "Amstrad CPC";

          case 0x10: // YM2149, 1.79MHz
            return "MSX";
          case 0x13: // YM2149, 2MHz
            return "Atari ST";
          case 0x26: // 5B NTSC
            return "Sunsoft 5B standalone";
          case 0x28: // 5B PAL
            return "Sunsoft 5B standalone (PAL)";

          case 0x30: // AY-3-8914, 1.79MHz
            return "Intellivision";
          case 0x33: // AY-3-8914, 2MHz
            return "Intellivision (PAL)";

          default:
            if ((song.systemFlags[0]&0x30)==0x00) {
              return "AY-3-8910";
            } else if ((song.systemFlags[0]&0x30)==0x10) {
              return "Yamaha YM2149";
            } else if ((song.systemFlags[0]&0x30)==0x20) {
              return "Overclocked Sunsoft 5B";
            } else if ((song.systemFlags[0]&0x30)==0x30) {
              return "Intellivision";
            }
        }
      } else if (song.system[0]==DIV_SYSTEM_SMS) {
        switch (song.systemFlags[0]&0x0f) {
          case 0: case 1:
            return "Sega Master System";
          case 6:
            return "BBC Micro";
        }
      } else if (song.system[0]==DIV_SYSTEM_YM2612) {
        switch (song.systemFlags[0]&3) {
          case 2:
            return "FM Towns";
        }
      } else if (song.system[0]==DIV_SYSTEM_YM2151) {
        switch (song.systemFlags[0]&3) {
          case 2:
            return "Sharp X68000";
        }
      } else if (song.system[0]==DIV_SYSTEM_SAA1099) {
        switch (song.systemFlags[0]&3) {
          case 0:
            return "SAM Coupé";
        }
      }
      return getSystemName(song.system[0]);
    case 2:
      if (song.system[0]==DIV_SYSTEM_YM2612 && song.system[1]==DIV_SYSTEM_SMS) {
        return "Sega Genesis/Mega Drive";
      }
      if (song.system[0]==DIV_SYSTEM_YM2612_EXT && song.system[1]==DIV_SYSTEM_SMS) {
        return "Sega Genesis Extended Channel 3";
      }

      if (song.system[0]==DIV_SYSTEM_OPLL && song.system[1]==DIV_SYSTEM_SMS) {
        return "NTSC-J Sega Master System";
      }
      if (song.system[0]==DIV_SYSTEM_OPLL_DRUMS && song.system[1]==DIV_SYSTEM_SMS) {
        return "NTSC-J Sega Master System + drums";
      }
      if (song.system[0]==DIV_SYSTEM_OPLL && song.system[1]==DIV_SYSTEM_AY8910) {
        return "MSX-MUSIC";
      }
      if (song.system[0]==DIV_SYSTEM_OPLL_DRUMS && song.system[1]==DIV_SYSTEM_AY8910) {
        return "MSX-MUSIC + drums";
      }
      if (song.system[0]==DIV_SYSTEM_C64_6581 && song.system[1]==DIV_SYSTEM_C64_6581) {
        return "Commodore 64 with dual 6581";
      }
      if (song.system[0]==DIV_SYSTEM_C64_8580 && song.system[1]==DIV_SYSTEM_C64_8580) {
        return "Commodore 64 with dual 8580";
      }

      if (song.system[0]==DIV_SYSTEM_YM2151 && song.system[1]==DIV_SYSTEM_SEGAPCM_COMPAT) {
        return "YM2151 + SegaPCM Arcade (compatibility)";
      }
      if (song.system[0]==DIV_SYSTEM_YM2151 && song.system[1]==DIV_SYSTEM_SEGAPCM) {
        return "YM2151 + SegaPCM Arcade";
      }

      if (song.system[0]==DIV_SYSTEM_SAA1099 && song.system[1]==DIV_SYSTEM_SAA1099) {
        return "Creative Music System";
      }

      if (song.system[0]==DIV_SYSTEM_GB && song.system[1]==DIV_SYSTEM_AY8910) {
        return "Game Boy with AY expansion";
      }

      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_VRC6) {
        return "NES + Konami VRC6";
      }
      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_VRC7) {
        return "NES + Konami VRC7";
      }
      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_OPLL) {
        return "NES + Yamaha OPLL";
      }
      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_FDS) {
        return "Famicom Disk System";
      }
      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_N163) {
        return "NES + Namco 163";
      }
      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_MMC5) {
        return "NES + MMC5";
      }
      if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_AY8910) {
        return "NES + Sunsoft 5B";
      }

      if (song.system[0]==DIV_SYSTEM_AY8910 && song.system[1]==DIV_SYSTEM_AY8910) {
        return "Bally Midway MCR";
      }

      if (song.system[0]==DIV_SYSTEM_YM2151 && song.system[1]==DIV_SYSTEM_VERA) {
        return "Commander X16";
      }
      break;
    case 3:
      if (song.system[0]==DIV_SYSTEM_AY8910 && song.system[1]==DIV_SYSTEM_AY8910 && song.system[2]==DIV_SYSTEM_BUBSYS_WSG) {
        return "Konami Bubble System";
      }
      break;
  }
  return "multi-system";
}

const char* DivEngine::getSystemName(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return "Unknown";
    case DIV_SYSTEM_YMU759:
      return "Yamaha YMU759";
    case DIV_SYSTEM_GENESIS:
      return "Sega Genesis/Mega Drive";
    case DIV_SYSTEM_SMS:
      return "TI SN76489";
    case DIV_SYSTEM_SMS_OPLL:
      return "Sega Master System + FM Expansion";
    case DIV_SYSTEM_GB:
      return "Game Boy";
    case DIV_SYSTEM_PCE:
      return "PC Engine/TurboGrafx-16";
    case DIV_SYSTEM_NES:
      return "NES";
    case DIV_SYSTEM_NES_VRC7:
      return "NES + Konami VRC7";
    case DIV_SYSTEM_C64_6581:
      return "Commodore 64 with 6581";
    case DIV_SYSTEM_C64_8580:
      return "Commodore 64 with 8580";
    case DIV_SYSTEM_ARCADE:
      return "YM2151 + SegaPCM Arcade";
    case DIV_SYSTEM_GENESIS_EXT:
      return "Sega Genesis Extended Channel 3";
    case DIV_SYSTEM_YM2610:
      return "Neo Geo CD";
    case DIV_SYSTEM_YM2610_EXT:
      return "Neo Geo CD Extended Channel 2";
    // Furnace-specific systems
    case DIV_SYSTEM_YM2610_FULL:
      return "Neo Geo";
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return "Neo Geo Extended Channel 2";
    case DIV_SYSTEM_AY8910:
      return "AY-3-8910";
    case DIV_SYSTEM_AMIGA:
      return "Amiga";
    case DIV_SYSTEM_YM2151:
      return "Yamaha YM2151";
    case DIV_SYSTEM_YM2612:
      return "Yamaha YM2612";
    case DIV_SYSTEM_TIA:
      return "Atari 2600";
    case DIV_SYSTEM_VIC20:
      return "Commodore VIC-20";
    case DIV_SYSTEM_PET:
      return "Commodore PET";
    case DIV_SYSTEM_SNES:
      return "SNES";
    case DIV_SYSTEM_VRC6:
      return "Konami VRC6";
    case DIV_SYSTEM_OPLL:
      return "Yamaha OPLL";
    case DIV_SYSTEM_FDS:
      return "Famicom Disk System (chip)";
    case DIV_SYSTEM_MMC5:
      return "MMC5";
    case DIV_SYSTEM_N163:
      return "Namco 163";
    case DIV_SYSTEM_OPN:
      return "NEC PC-9801-26K";
    case DIV_SYSTEM_PC98:
      return "PC-9801-86 + Chibi-oto";
    case DIV_SYSTEM_OPL:
      return "Yamaha OPL";
    case DIV_SYSTEM_OPL2:
      return "Yamaha OPL2";
    case DIV_SYSTEM_OPL3:
      return "Yamaha OPL3";
    case DIV_SYSTEM_MULTIPCM:
      return "MultiPCM";
    case DIV_SYSTEM_PCSPKR:
      return "PC Speaker";
    case DIV_SYSTEM_POKEY:
      return "Atari 400/800";
    case DIV_SYSTEM_RF5C68:
      return "Ricoh RF5C68";
    case DIV_SYSTEM_SWAN:
      return "WonderSwan";
    case DIV_SYSTEM_SAA1099:
      return "Philips SAA1099";
    case DIV_SYSTEM_OPZ:
      return "Yamaha TX81Z/YS200";
    case DIV_SYSTEM_POKEMINI:
      return "Pokémon Mini";
    case DIV_SYSTEM_AY8930:
      return "Microchip AY8930";
    case DIV_SYSTEM_SEGAPCM:
      return "SegaPCM";
    case DIV_SYSTEM_VBOY:
      return "Virtual Boy";
    case DIV_SYSTEM_VRC7:
      return "Konami VRC7";
    case DIV_SYSTEM_YM2610B:
      return "Taito Arcade";
    case DIV_SYSTEM_SFX_BEEPER:
      return "ZX Spectrum Beeper";
    case DIV_SYSTEM_YM2612_EXT:
      return "Yamaha YM2612 Extended Channel 3";
    case DIV_SYSTEM_SCC:
      return "Konami SCC";
    case DIV_SYSTEM_OPL_DRUMS:
      return "Yamaha OPL with drums";
    case DIV_SYSTEM_OPL2_DRUMS:
      return "Yamaha OPL2 with drums";
    case DIV_SYSTEM_OPL3_DRUMS:
      return "Yamaha OPL3 with drums";
    case DIV_SYSTEM_OPLL_DRUMS:
      return "Yamaha OPLL with drums";
    case DIV_SYSTEM_LYNX:
      return "Atari Lynx";
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      return "SegaPCM (compatible 5-channel mode)";
    case DIV_SYSTEM_YM2610B_EXT:
      return "Taito Arcade Extended Channel 3";
    case DIV_SYSTEM_QSOUND:
      return "Capcom QSound";
    case DIV_SYSTEM_VERA:
      return "VERA";
    case DIV_SYSTEM_X1_010:
      return "Seta/Allumer X1-010";
    case DIV_SYSTEM_BUBSYS_WSG:
      return "Konami Bubble System WSG";
  }
  return "Unknown";
}

const char* DivEngine::getSystemChips(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return "Unknown";
    case DIV_SYSTEM_YMU759:
      return "Yamaha YMU759";
    case DIV_SYSTEM_GENESIS:
      return "Yamaha YM2612 + TI SN76489";
    case DIV_SYSTEM_SMS:
      return "TI SN76489";
    case DIV_SYSTEM_SMS_OPLL:
      return "TI SN76489 + Yamaha YM2413";
    case DIV_SYSTEM_GB:
      return "Sharp LR35902";
    case DIV_SYSTEM_PCE:
      return "Hudson Soft HuC6280";
    case DIV_SYSTEM_NES:
      return "Ricoh 2A03";
    case DIV_SYSTEM_NES_VRC7:
      return "Ricoh 2A03 + Konami VRC7";
    case DIV_SYSTEM_C64_6581:
      return "SID 6581";
    case DIV_SYSTEM_C64_8580:
      return "SID 8580";
    case DIV_SYSTEM_ARCADE:
      return "Yamaha YM2151 + SegaPCM";
    case DIV_SYSTEM_GENESIS_EXT:
      return "Yamaha YM2612 (extended channel 3) + TI SN76489";
    case DIV_SYSTEM_YM2610:
      return "Yamaha YM2610 no ADPCM-B";
    case DIV_SYSTEM_YM2610_EXT:
      return "Yamaha YM2610 no ADPCM-B (extended channel 2)";
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return "AY-3-8910";
    case DIV_SYSTEM_AMIGA:
      return "MOS 8364 Paula";
    case DIV_SYSTEM_YM2151:
      return "Yamaha YM2151";
    case DIV_SYSTEM_YM2612:
      return "Yamaha YM2612";
    case DIV_SYSTEM_TIA:
      return "Atari TIA";
    case DIV_SYSTEM_VIC20:
      return "VIC";
    case DIV_SYSTEM_PET:
      return "Commodore PET";
    case DIV_SYSTEM_SNES:
      return "SPC700";
    case DIV_SYSTEM_VRC6:
      return "Konami VRC6";
    case DIV_SYSTEM_OPLL:
      return "Yamaha YM2413";
    case DIV_SYSTEM_FDS:
      return "Famicom Disk System";
    case DIV_SYSTEM_MMC5:
      return "MMC5";
    case DIV_SYSTEM_N163:
      return "Namco 163";
    case DIV_SYSTEM_OPN:
      return "Yamaha YM2203";
    case DIV_SYSTEM_PC98:
      return "Yamaha YM2608";
    case DIV_SYSTEM_OPL:
      return "Yamaha YM3526";
    case DIV_SYSTEM_OPL2:
      return "Yamaha YM3812";
    case DIV_SYSTEM_OPL3:
      return "Yamaha YMF262";
    case DIV_SYSTEM_MULTIPCM:
      return "Yamaha YMW258";
    case DIV_SYSTEM_PCSPKR:
      return "Intel 8253";
    case DIV_SYSTEM_POKEY:
      return "POKEY";
    case DIV_SYSTEM_RF5C68:
      return "Ricoh RF5C68";
    case DIV_SYSTEM_SWAN:
      return "WonderSwan";
    case DIV_SYSTEM_SAA1099:
      return "Philips SAA1099";
    case DIV_SYSTEM_OPZ:
      return "Yamaha YM2414";
    case DIV_SYSTEM_POKEMINI:
      return "Pokémon Mini";
    case DIV_SYSTEM_AY8930:
      return "Microchip AY8930";
    case DIV_SYSTEM_SEGAPCM:
      return "SegaPCM";
    case DIV_SYSTEM_VBOY:
      return "VSU";
    case DIV_SYSTEM_VRC7:
      return "Konami VRC7";
    case DIV_SYSTEM_YM2610B:
      return "Yamaha YM2610B";
    case DIV_SYSTEM_SFX_BEEPER:
      return "ZX Spectrum Beeper";
    case DIV_SYSTEM_YM2612_EXT:
      return "Yamaha YM2612 Extended Channel 3";
    case DIV_SYSTEM_SCC:
      return "Konami K051649";
    case DIV_SYSTEM_OPL_DRUMS:
      return "Yamaha YM3526 with drums";
    case DIV_SYSTEM_OPL2_DRUMS:
      return "Yamaha YM3812 with drums";
    case DIV_SYSTEM_OPL3_DRUMS:
      return "Yamaha YMF262 with drums";
    case DIV_SYSTEM_YM2610_FULL:
      return "Yamaha YM2610";
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return "Yamaha YM2610 (extended channel 2)";
    case DIV_SYSTEM_OPLL_DRUMS:
      return "Yamaha YM2413 with drums";
    case DIV_SYSTEM_LYNX:
      return "Mikey";
    case DIV_SYSTEM_SEGAPCM_COMPAT:
      return "SegaPCM (compatible 5-channel mode)";
    case DIV_SYSTEM_YM2610B_EXT:
      return "Yamaha YM2610B Extended Channel 3";
    case DIV_SYSTEM_QSOUND:
      return "Capcom DL-1425";
    case DIV_SYSTEM_VERA:
      return "VERA";
    case DIV_SYSTEM_X1_010:
      return "Seta/Allumer X1-010";
    case DIV_SYSTEM_BUBSYS_WSG:
      return "Konami Bubble System WSG";
  }
  return "Unknown";
}

const char* DivEngine::getSystemNameJ(DivSystem sys) {
  switch (sys) {
    case DIV_SYSTEM_NULL:
      return "不明";
    case DIV_SYSTEM_YMU759:
      return "";
    case DIV_SYSTEM_GENESIS:
      return "セガメガドライブ";
    case DIV_SYSTEM_SMS:
    case DIV_SYSTEM_SMS_OPLL:
      return "セガマスターシステム";
    case DIV_SYSTEM_GB:
      return "ゲームボーイ";
    case DIV_SYSTEM_PCE:
      return "PCエンジン";
    case DIV_SYSTEM_NES:
      return "ファミリーコンピュータ";
    case DIV_SYSTEM_C64_6581:
      return "コモドール64 (6581)";
    case DIV_SYSTEM_C64_8580:
      return "コモドール64 (8580)";
    case DIV_SYSTEM_ARCADE:
      return "Arcade";
    case DIV_SYSTEM_GENESIS_EXT:
      return "";
    case DIV_SYSTEM_YM2610:
      return "業務用ネオジオ";
    case DIV_SYSTEM_YM2610_EXT:
      return "業務用ネオジオ";
    case DIV_SYSTEM_YM2610_FULL:
      return "業務用ネオジオ";
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return "業務用ネオジオ";
    // Furnace-specific systems
    case DIV_SYSTEM_AY8910:
      return "";
    case DIV_SYSTEM_AMIGA:
      return "";
    case DIV_SYSTEM_YM2151:
      return "";
    case DIV_SYSTEM_YM2612:
      return "";
    case DIV_SYSTEM_TIA:
      return "";
    case DIV_SYSTEM_SAA1099:
      return "";
    case DIV_SYSTEM_AY8930:
      return "";
    default: // TODO
      return "";
  }
  return "不明";
}

bool DivEngine::isFMSystem(DivSystem sys) {
  return (sys==DIV_SYSTEM_GENESIS ||
          sys==DIV_SYSTEM_GENESIS_EXT ||
          sys==DIV_SYSTEM_SMS_OPLL ||
          sys==DIV_SYSTEM_NES_VRC7 ||
          sys==DIV_SYSTEM_ARCADE ||
          sys==DIV_SYSTEM_YM2610 ||
          sys==DIV_SYSTEM_YM2610_EXT ||
          sys==DIV_SYSTEM_YM2610_FULL ||
          sys==DIV_SYSTEM_YM2610_FULL_EXT ||
          sys==DIV_SYSTEM_YM2610B ||
          sys==DIV_SYSTEM_YM2610B_EXT ||
          sys==DIV_SYSTEM_YMU759 ||
          sys==DIV_SYSTEM_YM2151 ||
          sys==DIV_SYSTEM_YM2612);
}

bool DivEngine::isSTDSystem(DivSystem sys) {
  return (sys!=DIV_SYSTEM_ARCADE &&
          sys!=DIV_SYSTEM_YMU759 &&
          sys!=DIV_SYSTEM_YM2612 &&
          sys!=DIV_SYSTEM_YM2151);
}

const char* chanNames[40][32]={
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "PCM"}, // YMU759/SegaPCM
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Noise"}, // Genesis
  {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Noise"}, // Genesis (extended channel 3)
  {"Square 1", "Square 2", "Square 3", "Noise"}, // SMS
  {"Pulse 1", "Pulse 2", "Wavetable", "Noise"}, // GB
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6"}, // PCE/ZX Beeper
  {"Pulse 1", "Pulse 2", "Triangle", "Noise", "PCM"}, // NES
  {"Channel 1", "Channel 2", "Channel 3"}, // C64
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5"}, // Arcade
  {"FM 1", "FM 2", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"}, // YM2610
  {"FM 1", "FM 2 OP1", "FM 2 OP2", "FM 2 OP3", "FM 2 OP4", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"}, // YM2610 (extended channel 2)
  {"PSG 1", "PSG 2", "PSG 3"},  // AY-3-8910
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},  // Amiga/POKEY/Lynx
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8"}, // YM2151/YM2414
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6"}, // YM2612
  {"Channel 1", "Channel 2"}, // TIA
  {"PSG 1", "PSG 2", "PSG 3", "PSG 4", "PSG 5", "PSG 6"}, // SAA1099
  {"PSG 1", "PSG 2", "PSG 3"},  // AY8930
  {"Low", "Mid", "High", "Noise"}, // VIC-20
  {"Wave"}, // PET
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"}, // SNES/N163/RF5C68
  {"VRC6 1", "VRC6 2", "VRC6 Saw"}, // VRC6
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"}, // OPLL/OPL/OPL2/VRC7
  {"FDS"}, // FDS
  {"Pulse 1", "Pulse 2", "PCM"}, // MMC5
  {"FM 1", "FM 2", "FM 3", "PSG 1", "PSG 2", "PSG 3"}, // OPN
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Kick", "Snare", "Top", "HiHat", "Tom", "Rim", "ADPCM"}, // PC-98
  {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "FM 16", "FM 17", "FM 18"}, // OPL3
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "Channel 17", "Channel 18", "Channel 19", "Channel 20", "Channel 21", "Channel 22", "Channel 23", "Channel 24", "Channel 25", "Channel 26", "Channel 27", "Channel 28"}, // MultiPCM
  {"Square"}, // PC Speaker/Pokémon Mini
  {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Noise"}, // Virtual Boy/SCC
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"}, // YM2610B
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"}, // OPLL/OPL/OPL2 drums
  {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "Kick", "Snare", "Tom", "Top", "HiHat"}, // OPL3 drums
  {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "4OP 1", "4OP 2", "4OP 3", "4OP 4", "4OP 5", "4OP 6"}, // OPL3 4-op (UNUSED)
  {"FM 1", "FM 2", "FM 3", "4OP 1", "4OP 2", "4OP 3", "4OP 4", "4OP 5", "4OP 6", "Kick", "Snare", "Tom", "Top", "HiHat"}, // OPL3 4-op + drums (UNUSED)
  {"PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5", "PCM 6", "PCM 7", "PCM 8", "PCM 9", "PCM 10", "PCM 11", "PCM 12", "PCM 13", "PCM 14", "PCM 15", "PCM 16", "ADPCM 1", "ADPCM 2", "ADPCM 3"}, // QSound
  {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"}, // YM2610B (extended channel 3)
  {"Wave", "Wave/PCM", "Wave", "Wave/Noise"}, // Swan
  {"PSG 1", "PSG 2", "PSG 3", "PSG 4", "PSG 5", "PSG 6", "PSG 7", "PSG 8", "PSG 9", "PSG 10", "PSG 11", "PSG 12", "PSG 13", "PSG 14", "PSG 15", "PSG 16", "PCM"}, // VERA
};

const char* chanShortNames[38][32]={
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "PCM"}, // YMU759
  {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "NO"}, // Genesis
  {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6", "S1", "S2", "S3", "S4"}, // Genesis (extended channel 3)
  {"S1", "S2", "S3", "NO"}, // SMS
  {"S1", "S2", "WA", "NO"}, // GB
  {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"}, // PCE
  {"S1", "S2", "TR", "NO", "PCM"}, // NES
  {"CH1", "CH2", "CH3"}, // C64
  {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "P1", "P2", "P3", "P4", "P5"}, // Arcade
  {"F1", "F2", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"}, // YM2610
  {"F1", "O1", "O2", "O3", "O4", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"}, // YM2610 (extended channel 2)
  {"S1", "S2", "S3"},  // AY-3-8910
  {"CH1", "CH2", "CH3", "CH4"},  // Amiga/Lynx
  {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"}, // YM2151
  {"F1", "F2", "F3", "F4", "F5", "F6"}, // YM2612
  {"CH1", "CH2"}, // TIA
  {"S1", "S2", "S3", "S4", "S5", "S6"}, // SAA1099
  {"S1", "S2", "S3"},  // AY8930
  {"LO", "MID", "HI", "NO"}, // VIC-20
  {"PET"}, // PET
  {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"}, // SNES/N163/RF5C68
  {"V1", "V2", "VS"}, // VRC6
  {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"}, // OPLL/OPL/OPL2/VRC7
  {"FDS"}, // FDS
  {"S1", "S2", "PCM"}, // MMC5
  {"F1", "F2", "F3", "S1", "S2", "S3"}, // OPN
  {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "BD", "SD", "TP", "HH", "TM", "RM", "P"}, // PC-98
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18"}, // OPL3
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28"}, // MultiPCM
  {"SQ"}, // PC Speaker/Pokémon Mini
  {"CH1", "CH2", "CH3", "CH4", "CH5", "NO"}, // Virtual Boy/SCC
  {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"}, // YM2610B
  {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"}, // OPLL/OPL/OPL2 drums
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "BD", "SD", "TM", "TP", "HH"}, // OPL3 drums
  {"F1", "F2", "F3", "F4", "F5", "F6", "Q1", "Q2", "Q3", "Q4", "Q5", "Q6"}, // OPL3 4-op (UNUSED)
  {"F1", "F2", "F3", "Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "BD", "SD", "TM", "TP", "HH"}, // OPL3 4-op + drums (UNUSED)
  {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "A1", "A2", "A3"}, // QSound
  {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"}, // YM2610B (extended channel 3)
};

const int chanTypes[41][32]={
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4}, // YMU759
  {0, 0, 0, 0, 0, 0, 1, 1, 1, 2}, // Genesis
  {0, 0, 5, 5, 5, 5, 0, 0, 0, 1, 1, 1, 2}, // Genesis (extended channel 3)
  {1, 1, 1, 2}, // SMS
  {1, 1, 3, 2}, // GB
  {3, 3, 3, 3, 3, 3}, // PCE
  {1, 1, 3, 2, 4}, // NES
  {2, 2, 2}, // C64
  {0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4}, // Arcade
  {0, 0, 0, 0, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4}, // YM2610
  {0, 5, 5, 5, 5, 0, 0, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4}, // YM2610 (extended channel 2)
  {1, 1, 1},  // AY-3-8910
  {4, 4, 4, 4},  // Amiga
  {0, 0, 0, 0, 0, 0, 0, 0}, // YM2151
  {0, 0, 0, 0, 0, 0}, // YM2612
  {3, 3}, // TIA
  {1, 1, 1, 1, 1, 1}, // SAA1099
  {1, 1, 1},  // AY8930
  {1, 1, 1, 2}, // VIC-20
  {1}, // PET
  {4, 4, 4, 4, 4, 4, 4, 4}, // SNES/N163/RF5C68
  {1, 1, 3}, // VRC6
  {0, 0, 0, 0, 0, 0, 0, 0, 0}, // OPLL/OPL/OPL2/VRC7
  {3}, // FDS
  {1, 1, 4}, // MMC5
  {0, 0, 0, 1, 1, 1}, // OPN
  {0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4}, // PC-98
  {5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 0, 0, 0, 0, 0, 0}, // OPL3
  {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}, // MultiPCM/QSound
  {1}, // PC Speaker/Pokémon Mini
  {3, 3, 3, 3, 3, 2}, // Virtual Boy/SCC
  {0, 0, 0, 0, 0, 0, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4}, // YM2610B
  {0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2}, // OPLL/OPL/OPL2 drums
  {5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 0, 0, 0, 2, 2, 2, 2, 2}, // OPL3 drums
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // OPL3 4-op (UNUSED)
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2}, // OPL3 4-op + drums (UNUSED)
  {3, 3, 3, 3}, // Lynx
  {0, 0, 5, 5, 5, 5, 0, 0, 0, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4}, // YM2610B (extended channel 3)
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4}, // VERA
  {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}, // X1-010
  {3, 4, 3, 2}, // Swan
};

const DivInstrumentType chanPrefType[47][28]={
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YMU759
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // Genesis
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // Genesis (extended channel 3)
  {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // SMS
  {DIV_INS_GB, DIV_INS_GB, DIV_INS_GB, DIV_INS_GB}, // GB
  {DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE}, // PCE
  {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}, // NES
  {DIV_INS_C64, DIV_INS_C64, DIV_INS_C64}, // C64
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // Arcade
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // YM2610
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // YM2610 (extended channel 2)
  {DIV_INS_AY, DIV_INS_AY, DIV_INS_AY},  // AY-3-8910
  {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},  // Amiga
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YM2151
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}, // YM2612
  {DIV_INS_TIA, DIV_INS_TIA}, // TIA
  {DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099}, // SAA1099
  {DIV_INS_AY8930, DIV_INS_AY8930, DIV_INS_AY8930},  // AY8930
  {DIV_INS_VIC, DIV_INS_VIC, DIV_INS_VIC, DIV_INS_VIC}, // VIC-20
  {DIV_INS_PET}, // PET
  {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // SNES/N163/RF5C68
  {DIV_INS_VRC6, DIV_INS_VRC6, DIV_INS_VRC6}, // VRC6
  {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL}, // OPLL/VRC7
  {DIV_INS_FDS}, // FDS
  {DIV_INS_STD, DIV_INS_STD, DIV_INS_AMIGA}, // MMC5
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY}, // OPN
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // PC-98
  {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}, // OPL/OPL2/OPL3
  {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // MultiPCM/QSound
  {DIV_INS_STD}, // PC Speaker/Pokémon Mini
  {DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY}, // Virtual Boy
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // YM2610B
  {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL}, // OPLL drums
  {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}, // OPL3 drums
  {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ}, // OPL3 4-op
  {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}, // OPL3 4-op + drums
  {DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC}, // SCC
  {DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163}, // N163
  {DIV_INS_POKEY, DIV_INS_POKEY, DIV_INS_POKEY, DIV_INS_POKEY}, // POKEY
  {DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER}, // ZX beeper
  {DIV_INS_SWAN, DIV_INS_SWAN, DIV_INS_SWAN, DIV_INS_SWAN}, // Swan
  {DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ}, // Z
  {DIV_INS_MIKEY, DIV_INS_MIKEY, DIV_INS_MIKEY, DIV_INS_MIKEY}, // Lynx
  {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}, // YM2610B (extended channel 3)
  {DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_AMIGA}, // VERA
  {DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010}, // X1-010
  {DIV_INS_SCC, DIV_INS_SCC}, // Bubble System WSG
};

const char* DivEngine::getChannelName(int chan) {
  if (chan<0 || chan>chans) return "??";
  if (!song.chanName[chan].empty()) return song.chanName[chan].c_str();
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanNames[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanNames[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2612_EXT:
      return chanNames[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanNames[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS_OPLL: // this is flattened to SMS + OPLL.
    case DIV_SYSTEM_NES_VRC7: // this is flattened to NES + VRC7.
      return "??";
      break;
    case DIV_SYSTEM_GB:
      return chanNames[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
    case DIV_SYSTEM_SFX_BEEPER:
    case DIV_SYSTEM_BUBSYS_WSG:
      return chanNames[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanNames[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanNames[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_OPZ:
      return chanNames[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
      return chanNames[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return chanNames[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanNames[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
    case DIV_SYSTEM_POKEY:
    case DIV_SYSTEM_LYNX:
      return chanNames[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SWAN:
      return chanNames[38][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanNames[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanNames[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanNames[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VIC20:
      return chanNames[18][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PET:
      return chanNames[19][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SNES:
    case DIV_SYSTEM_N163:
    case DIV_SYSTEM_RF5C68:
      return chanNames[20][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VRC6:
      return chanNames[21][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_VRC7:
      return chanNames[22][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_FDS:
      return chanNames[23][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MMC5:
      return chanNames[24][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPN:
      return chanNames[25][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PC98:
      return chanNames[26][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL3:
      return chanNames[27][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MULTIPCM:
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
    case DIV_SYSTEM_X1_010:
      return chanNames[28][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCSPKR:
    case DIV_SYSTEM_POKEMINI:
      return chanNames[29][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VBOY:
    case DIV_SYSTEM_SCC:
      return chanNames[30][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B:
      return chanNames[31][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B_EXT:
      return chanNames[37][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
      return chanNames[32][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL3_DRUMS:
      return chanNames[33][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanNames[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanNames[17][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_QSOUND:
      return chanNames[36][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VERA:
      return chanNames[39][dispatchChanOfChan[chan]];
      break;
  }
  return "??";
}

const char* DivEngine::getChannelShortName(int chan) {
  if (chan<0 || chan>chans) return "??";
  if (!song.chanShortName[chan].empty()) return song.chanShortName[chan].c_str();
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanShortNames[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanShortNames[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2612_EXT:
      return chanShortNames[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanShortNames[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS_OPLL: // this is flattened to SMS + OPLL.
    case DIV_SYSTEM_NES_VRC7: // this is flattened to NES + VRC7.
      return "??";
      break;
    case DIV_SYSTEM_GB:
      return chanShortNames[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
    case DIV_SYSTEM_SFX_BEEPER:
    case DIV_SYSTEM_BUBSYS_WSG:
      return chanShortNames[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanShortNames[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanShortNames[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_OPZ:
      return chanShortNames[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
      return chanShortNames[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return chanShortNames[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanShortNames[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
    case DIV_SYSTEM_POKEY:
    case DIV_SYSTEM_SWAN:
    case DIV_SYSTEM_LYNX:
      return chanShortNames[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanShortNames[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanShortNames[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanShortNames[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VIC20:
      return chanShortNames[18][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PET:
      return chanShortNames[19][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SNES:
    case DIV_SYSTEM_N163:
    case DIV_SYSTEM_RF5C68:
      return chanShortNames[20][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VRC6:
      return chanShortNames[21][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_VRC7:
      return chanShortNames[22][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_FDS:
      return chanShortNames[23][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MMC5:
      return chanShortNames[24][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPN:
      return chanShortNames[25][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PC98:
      return chanShortNames[26][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL3:
      return chanShortNames[27][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MULTIPCM:
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
    case DIV_SYSTEM_X1_010:
      return chanShortNames[28][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCSPKR:
    case DIV_SYSTEM_POKEMINI:
      return chanShortNames[29][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VBOY:
    case DIV_SYSTEM_SCC:
      return chanShortNames[30][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B:
      return chanShortNames[31][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B_EXT:
      return chanShortNames[37][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
      return chanShortNames[32][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL3_DRUMS:
      return chanShortNames[33][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanShortNames[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanShortNames[17][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_QSOUND:
      return chanShortNames[36][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VERA:
      return chanShortNames[0][dispatchChanOfChan[chan]];
      break;
  }
  return "??";
}

int DivEngine::getChannelType(int chan) {
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanTypes[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanTypes[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2612_EXT:
      return chanTypes[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanTypes[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS_OPLL: // this is flattened to SMS + OPLL.
    case DIV_SYSTEM_NES_VRC7: // this is flattened to NES + VRC7.
      return 0;
      break;
    case DIV_SYSTEM_GB:
      return chanTypes[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
    case DIV_SYSTEM_SFX_BEEPER:
    case DIV_SYSTEM_BUBSYS_WSG:
      return chanTypes[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanTypes[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanTypes[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
    case DIV_SYSTEM_OPZ:
      return chanTypes[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
      return chanTypes[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return chanTypes[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanTypes[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
    case DIV_SYSTEM_POKEY:
      return chanTypes[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanTypes[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanTypes[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanTypes[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VIC20:
      return chanTypes[18][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PET:
      return chanTypes[19][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SNES:
    case DIV_SYSTEM_N163:
    case DIV_SYSTEM_RF5C68:
      return chanTypes[20][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VRC6:
      return chanTypes[21][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_VRC7:
      return chanTypes[22][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_FDS:
      return chanTypes[23][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MMC5:
      return chanTypes[24][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPN:
      return chanTypes[25][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PC98:
      return chanTypes[26][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL3:
      return chanTypes[27][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MULTIPCM:
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
    case DIV_SYSTEM_QSOUND:
      return chanTypes[28][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCSPKR:
    case DIV_SYSTEM_POKEMINI:
      return chanTypes[29][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VBOY:
    case DIV_SYSTEM_SCC:
      return chanTypes[30][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B:
      return chanTypes[31][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B_EXT:
      return chanTypes[37][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
      return chanTypes[32][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL3_DRUMS:
      return chanTypes[33][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanTypes[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanTypes[17][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_LYNX:
      return chanTypes[36][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VERA:
      return chanTypes[38][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_X1_010:
      return chanTypes[39][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SWAN:
      return chanTypes[40][dispatchChanOfChan[chan]];
      break;
  }
  return 1;
}

DivInstrumentType DivEngine::getPreferInsType(int chan) {
  switch (sysOfChan[chan]) {
    case DIV_SYSTEM_NULL: case DIV_SYSTEM_YMU759:
      return chanPrefType[0][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS:
      return chanPrefType[1][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_GENESIS_EXT:
    case DIV_SYSTEM_YM2612_EXT:
      return chanPrefType[2][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS:
      return chanPrefType[3][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SMS_OPLL: // this is flattened to SMS + OPLL.
    case DIV_SYSTEM_NES_VRC7: // this is flattened to NES + VRC7.
      return DIV_INS_OPLL;
      break;
    case DIV_SYSTEM_GB:
      return chanPrefType[4][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCE:
      return chanPrefType[5][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_NES:
      return chanPrefType[6][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_C64_6581: case DIV_SYSTEM_C64_8580:
      return chanPrefType[7][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_ARCADE:
      return chanPrefType[8][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_FULL:
      return chanPrefType[9][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL_EXT:
      return chanPrefType[10][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8910:
      return chanPrefType[11][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AMIGA:
      return chanPrefType[12][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2151:
      return chanPrefType[13][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2612:
      return chanPrefType[14][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_TIA:
      return chanPrefType[15][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VIC20:
      return chanPrefType[18][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PET:
      return chanPrefType[19][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SNES:
    case DIV_SYSTEM_RF5C68:
      return chanPrefType[20][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VRC6:
      return chanPrefType[21][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_VRC7:
      return chanPrefType[22][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_FDS:
      return chanPrefType[23][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MMC5:
      return chanPrefType[24][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPN:
      return chanPrefType[25][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PC98:
      return chanPrefType[26][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL3:
      return chanPrefType[27][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_MULTIPCM:
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
    case DIV_SYSTEM_QSOUND:
      return chanPrefType[28][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_PCSPKR:
    case DIV_SYSTEM_POKEMINI:
      return chanPrefType[29][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VBOY:
      return chanPrefType[30][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SCC:
      return chanPrefType[36][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_N163:
      return chanPrefType[37][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B:
      return chanPrefType[31][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_YM2610B_EXT:
      return chanPrefType[43][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPLL_DRUMS:
      return chanPrefType[32][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_OPL3_DRUMS:
      return chanPrefType[33][dispatchChanOfChan[chan]];
      return chanPrefType[33][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SAA1099:
      return chanPrefType[16][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_AY8930:
      return chanPrefType[17][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_POKEY:
      return chanPrefType[38][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SFX_BEEPER:
      return chanPrefType[39][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_SWAN:
      return chanPrefType[40][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_OPZ:
      return chanPrefType[41][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_LYNX:
      return chanPrefType[42][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_VERA:
      return chanPrefType[44][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_X1_010:
      return chanPrefType[45][dispatchChanOfChan[chan]];
      break;
    case DIV_SYSTEM_BUBSYS_WSG:
      return chanPrefType[46][dispatchChanOfChan[chan]];
      break;
  }
  return DIV_INS_FM;
}

int DivEngine::minVGMVersion(DivSystem which) {
  switch (which) {
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_SMS:
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
    case DIV_SYSTEM_VRC7:
    case DIV_SYSTEM_YM2151:
      return 0x150; // due to usage of data blocks
    case DIV_SYSTEM_SEGAPCM:
    case DIV_SYSTEM_SEGAPCM_COMPAT:
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      return 0x151;
    case DIV_SYSTEM_GB:
    case DIV_SYSTEM_PCE:
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_QSOUND:
      return 0x161;
    case DIV_SYSTEM_SAA1099:
    case DIV_SYSTEM_X1_010:
    case DIV_SYSTEM_SWAN:
      return 0x171;
    default:
      return 0;
  }
  return 0;
}
