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
#include "../ta-log.h"

DivSystem DivEngine::systemFromFileFur(unsigned char val) {
  return sysFileMapFur[val];
}

unsigned char DivEngine::systemToFileFur(DivSystem val) {
  if (sysDefs[val]==NULL) return 0;
  return sysDefs[val]->id;
}

DivSystem DivEngine::systemFromFileDMF(unsigned char val) {
  return sysFileMapDMF[val];
}

unsigned char DivEngine::systemToFileDMF(DivSystem val) {
  if (sysDefs[val]==NULL) return 0;
  return sysDefs[val]->id_DMF;
}

int DivEngine::getChannelCount(DivSystem sys) {
  if (sysDefs[sys]==NULL) return 0;
  return sysDefs[sys]->channels;
}

int DivEngine::getTotalChannelCount() {
  return chans;
}

std::vector<DivInstrumentType>& DivEngine::getPossibleInsTypes() {
  return possibleInsTypes;
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
  if (sysDefs[sys]==NULL) return "Unknown";
  return sysDefs[sys]->name;
}

const char* DivEngine::getSystemNameJ(DivSystem sys) {
  if (sysDefs[sys]==NULL) return "不明";
  if (sysDefs[sys]->nameJ==NULL) return "";
  return sysDefs[sys]->nameJ;
  /*
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
  */
}

bool DivEngine::isFMSystem(DivSystem sys) {
  if (sysDefs[sys]==NULL) return false;
  return sysDefs[sys]->isFM;
}

bool DivEngine::isSTDSystem(DivSystem sys) {
  if (sysDefs[sys]==NULL) return false;
  return sysDefs[sys]->isSTD;
}

const char* DivEngine::getChannelName(int chan) {
  if (chan<0 || chan>chans) return "??";
  if (!song.chanName[chan].empty()) return song.chanName[chan].c_str();
  if (sysDefs[sysOfChan[chan]]==NULL) return "??";
  
  const char* ret=sysDefs[sysOfChan[chan]]->chanNames[dispatchChanOfChan[chan]];
  if (ret==NULL) return "??";
  return ret;
}

const char* DivEngine::getChannelShortName(int chan) {
  if (chan<0 || chan>chans) return "??";
  if (!song.chanShortName[chan].empty()) return song.chanShortName[chan].c_str();
  if (sysDefs[sysOfChan[chan]]==NULL) return "??";
  
  const char* ret=sysDefs[sysOfChan[chan]]->chanShortNames[dispatchChanOfChan[chan]];
  if (ret==NULL) return "??";
  return ret;
}

int DivEngine::getChannelType(int chan) {
  if (chan<0 || chan>chans) return DIV_CH_NOISE;
  if (sysDefs[sysOfChan[chan]]==NULL) return DIV_CH_NOISE;
  return sysDefs[sysOfChan[chan]]->chanTypes[dispatchChanOfChan[chan]];
}

DivInstrumentType DivEngine::getPreferInsType(int chan) {
  if (chan<0 || chan>chans) return DIV_INS_STD;
  if (sysDefs[sysOfChan[chan]]==NULL) return DIV_INS_STD;
  return sysDefs[sysOfChan[chan]]->chanInsType[dispatchChanOfChan[chan]][0];
}

DivInstrumentType DivEngine::getPreferInsSecondType(int chan) {
  if (chan<0 || chan>chans) return DIV_INS_NULL;
  if (sysDefs[sysOfChan[chan]]==NULL) return DIV_INS_NULL;
  return sysDefs[sysOfChan[chan]]->chanInsType[dispatchChanOfChan[chan]][1];
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
    case DIV_SYSTEM_FDS:
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

// define systems like:
// sysDefs[DIV_SYSTEM_ID]=new DivSysDef(
//   "Name", "Name (japanese, optional)", fileID, fileID_DMF, channels, isFM, isSTD, vgmVersion,
//   {"Channel Names", ...},
//   {"Channel Short Names", ...},
//   {chanTypes, ...},
//   {chanPreferInsType, ...},
//   {chanPreferInsType2, ...}, (optional)
//   [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {}, (effect handler, optional)
//   [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {} (post effect handler, optional)
// );

void DivEngine::registerSystems() {
  logD("registering systems...");

  sysDefs[DIV_SYSTEM_YMU759]=new DivSysDef(
    "Yamaha YMU759", NULL, 0x01, 0x01, 17, true, false, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "PCM"        }, // name
    {"1",         "2",         "3",         "4",         "5",         "6",         "7",         "8",         "9",         "10",         "11",         "12",         "13",         "14",         "15",         "16",         "PCM"        }, // short
    {DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,   DIV_CH_FM,    DIV_CH_FM,    DIV_CH_FM,    DIV_CH_FM,    DIV_CH_FM,    DIV_CH_FM,    DIV_CH_FM,    DIV_CH_PCM   }, // type
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL,  DIV_INS_OPL,  DIV_INS_OPL,  DIV_INS_OPL,  DIV_INS_OPL,  DIV_INS_OPL,  DIV_INS_OPL,  DIV_INS_AMIGA}  // ins
  );

  sysDefs[DIV_SYSTEM_GENESIS]=new DivSysDef(
    "Sega Genesis/Mega Drive", "セガメガドライブ", 0x02, 0x02, 10, true, true, 0, true,
    {}, {}, {}, {}
  );

  sysDefs[DIV_SYSTEM_GENESIS_EXT]=new DivSysDef(
    "Sega Genesis Extended Channel 3", NULL, 0x42, 0x42, 13, true, true, 0, true,
    {}, {}, {}, {}
  );

  sysDefs[DIV_SYSTEM_SMS]=new DivSysDef(
    "TI SN76489", NULL, 0x03, 0x03, 4, false, true, 0x150, false,
    {"Square 1", "Square 2", "Square 3", "Noise"},
    {"S1", "S2", "S3", "NO"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_NOISE},
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}
  );

  sysDefs[DIV_SYSTEM_SMS_OPLL]=new DivSysDef(
    "Sega Master System + FM Expansion", NULL, 0x43, 0x43, 13, true, true, 0, true,
    {}, {}, {}, {}
  );

  sysDefs[DIV_SYSTEM_GB]=new DivSysDef(
    "Game Boy", NULL, 0x04, 0x04, 4, false, true, 0x161, false,
    {"Pulse 1", "Pulse 2", "Wavetable", "Noise"},
    {"S1", "S2", "WA", "NO"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_WAVE, DIV_CH_NOISE},
    {DIV_INS_GB, DIV_INS_GB, DIV_INS_GB, DIV_INS_GB}
  );

  sysDefs[DIV_SYSTEM_PCE]=new DivSysDef(
    "PC Engine/TurboGrafx-16", NULL, 0x05, 0x05, 6, false, true, 0x161, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE}
  );

  sysDefs[DIV_SYSTEM_NES]=new DivSysDef(
    "NES (Ricoh 2A03)", NULL, 0x06, 0x06, 5, false, true, 0x161, false,
    {"Pulse 1", "Pulse 2", "Triangle", "Noise", "PCM"},
    {"S1", "S2", "TR", "NO", "PCM"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_WAVE, DIV_CH_NOISE, DIV_CH_PCM},
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_NES_VRC7]=new DivSysDef(
    "NES + Konami VRC7", NULL, 0x46, 0x46, 11, true, true, 0, true,
    {}, {}, {}, {}
  );

  sysDefs[DIV_SYSTEM_NES_FDS]=new DivSysDef(
    "Famicom Disk System", NULL, 0, 0x86, 6, false, true, 0, true,
    {}, {}, {}, {}
  );

  sysDefs[DIV_SYSTEM_C64_6581]=new DivSysDef(
    "Commodore 64 (6581)", NULL, 0x47, 0x47, 3, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3"},
    {"CH1", "CH2", "CH3"},
    {DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_C64, DIV_INS_C64, DIV_INS_C64}
  );

  sysDefs[DIV_SYSTEM_C64_8580]=new DivSysDef(
    "Commodore 64 (8580)", NULL, 0x07, 0x07, 3, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3"},
    {"CH1", "CH2", "CH3"},
    {DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_C64, DIV_INS_C64, DIV_INS_C64}
  );

  sysDefs[DIV_SYSTEM_ARCADE]=new DivSysDef(
    "DefleCade", NULL, 0x08, 0x08, 13, true, false, 0, true,
    {}, {}, {}, {}
  );

  sysDefs[DIV_SYSTEM_YM2610]=new DivSysDef(
    "Neo Geo CD", NULL, 0x09, 0x09, 13, true, true, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6"},
    {"F1", "F2", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_YM2610_EXT]=new DivSysDef(
    "Neo Geo CD Extended Channel 2", NULL, 0x49, 0x49, 16, true, true, 0x151, false,
    {"FM 1", "FM 2 OP1", "FM 2 OP2", "FM 2 OP3", "FM 2 OP4", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6"},
    {"F1", "O1", "O2", "O3", "O4", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6"},
    {DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_AY8910]=new DivSysDef(
    "AY-3-8910", NULL, 0x80, 0, 3, false, true, 0x151, false,
    {"PSG 1", "PSG 2", "PSG 3"},
    {"S1", "S2", "S3"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_AY, DIV_INS_AY, DIV_INS_AY}
  );

  sysDefs[DIV_SYSTEM_AMIGA]=new DivSysDef(
    "Amiga", NULL, 0x81, 0, 4, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_YM2151]=new DivSysDef(
    "Yamaha YM2151", NULL, 0x82, 0, 8, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM}
  );

  sysDefs[DIV_SYSTEM_YM2612]=new DivSysDef(
    "Yamaha YM2612", NULL, 0x83, 0, 6, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6"},
    {"F1", "F2", "F3", "F4", "F5", "F6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM},
    {DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_TIA]=new DivSysDef(
    "Atari 2600", NULL, 0x84, 0, 2, false, true, 0, false,
    {"Channel 1", "Channel 2"},
    {"CH1", "CH2"},
    {DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_TIA, DIV_INS_TIA}
  );

  sysDefs[DIV_SYSTEM_SAA1099]=new DivSysDef(
    "Philips SAA1099", NULL, 0x97, 0, 6, false, true, 0x171, false,
    {"PSG 1", "PSG 2", "PSG 3", "PSG 4", "PSG 5", "PSG 6"},
    {"S1", "S2", "S3", "S4", "S5", "S6"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099}
  );

  sysDefs[DIV_SYSTEM_AY8930]=new DivSysDef(
    "Microchip AY8930", NULL, 0x9a, 0, 3, false, true, 0x151, false,
    {"PSG 1", "PSG 2", "PSG 3"},
    {"S1", "S2", "S3"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_AY8930, DIV_INS_AY8930, DIV_INS_AY8930}
  );

  sysDefs[DIV_SYSTEM_VIC20]=new DivSysDef(
    "Commodore VIC-20", NULL, 0x85, 0, 4, false, true, 0, false,
    {"Low", "Mid", "High", "Noise"},
    {"LO", "MID", "HI", "NO"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_NOISE},
    {DIV_INS_VIC, DIV_INS_VIC, DIV_INS_VIC, DIV_INS_VIC}
  );

  sysDefs[DIV_SYSTEM_PET]=new DivSysDef(
    "Commodore PET", NULL, 0x86, 0, 1, false, true, 0, false,
    {"Wave"},
    {"PET"},
    {DIV_CH_PULSE},
    {DIV_INS_PET}
  );

  // TODO: DIV_INS_SNES
  sysDefs[DIV_SYSTEM_SNES]=new DivSysDef(
    "SNES", NULL, 0x87, 0, 8, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_SNES, DIV_INS_SNES, DIV_INS_SNES, DIV_INS_SNES, DIV_INS_SNES, DIV_INS_SNES, DIV_INS_SNES, DIV_INS_SNES}
  );

  sysDefs[DIV_SYSTEM_VRC6]=new DivSysDef(
    "Konami VRC6", NULL, 0x88, 0, 3, false, true, 0, false,
    {"VRC6 1", "VRC6 2", "VRC6 Saw"},
    {"V1", "V2", "VS"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_WAVE},
    {DIV_INS_VRC6, DIV_INS_VRC6, DIV_INS_VRC6_SAW},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_NULL}
  );

  sysDefs[DIV_SYSTEM_OPLL]=new DivSysDef(
    "Yamaha OPLL", NULL, 0x89, 0, 9, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL}
  );

  sysDefs[DIV_SYSTEM_FDS]=new DivSysDef(
    "Famicom Disk System (chip)", NULL, 0x8a, 0, 1, false, true, 0x161, false,
    {"FDS"},
    {"FDS"},
    {DIV_CH_WAVE},
    {DIV_INS_FDS}
  );

  sysDefs[DIV_SYSTEM_MMC5]=new DivSysDef(
    "MMC5", NULL, 0x8b, 0, 3, false, true, 0, false,
    {"Pulse 1", "Pulse 2", "PCM"},
    {"S1", "S2", "PCM"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM},
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_N163]=new DivSysDef(
    "Namco 163", NULL, 0x8c, 0, 8, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163}
  );

  sysDefs[DIV_SYSTEM_OPN]=new DivSysDef(
    "Yamaha YM2203", NULL, 0x8d, 0, 6, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "PSG 1", "PSG 2", "PSG 3"},
    {"F1", "F2", "F3", "S1", "S2", "S3"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY}
  );

  sysDefs[DIV_SYSTEM_PC98]=new DivSysDef(
    "Yamaha YM2608", NULL, 0x8e, 0, 16, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Kick", "Snare", "Top", "HiHat", "Tom", "Rim", "ADPCM"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "BD", "SD", "TP", "HH", "TM", "RM", "P"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_OPL]=new DivSysDef(
    "Yamaha OPL", NULL, 0x8f, 0, 9, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}
  );

  sysDefs[DIV_SYSTEM_OPL2]=new DivSysDef(
    "Yamaha OPL2", NULL, 0x90, 0, 9, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}
  );

  sysDefs[DIV_SYSTEM_OPL3]=new DivSysDef(
    "Yamaha OPL3", NULL, 0x91, 0, 18, true, false, 0x151, false,
    {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "FM 16", "FM 17", "FM 18"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18"},
    {DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}
  );

  sysDefs[DIV_SYSTEM_MULTIPCM]=new DivSysDef(
    "MultiPCM", NULL, 0x92, 0, 28, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "Channel 17", "Channel 18", "Channel 19", "Channel 20", "Channel 21", "Channel 22", "Channel 23", "Channel 24", "Channel 25", "Channel 26", "Channel 27", "Channel 28"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM}
  );

  sysDefs[DIV_SYSTEM_PCSPKR]=new DivSysDef(
    "PC Speaker", NULL, 0x93, 0, 1, false, true, 0, false,
    {"Square"},
    {"SQ"},
    {DIV_CH_PULSE},
    {DIV_INS_STD}
  );

  sysDefs[DIV_SYSTEM_POKEY]=new DivSysDef(
    "POKEY", NULL, 0x94, 0, 4, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_POKEY, DIV_INS_POKEY, DIV_INS_POKEY, DIV_INS_POKEY}
  );

  sysDefs[DIV_SYSTEM_RF5C68]=new DivSysDef(
    "Ricoh RF5C68", NULL, 0x95, 0, 8, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_SWAN]=new DivSysDef(
    "WonderSwan", NULL, 0x96, 0, 4, false, true, 0x171, false,
    {"Wave", "Wave/PCM", "Wave", "Wave/Noise"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_WAVE, DIV_CH_PCM, DIV_CH_WAVE, DIV_CH_NOISE},
    {DIV_INS_SWAN, DIV_INS_SWAN, DIV_INS_SWAN, DIV_INS_SWAN},
    {DIV_INS_NULL, DIV_INS_AMIGA, DIV_INS_NULL, DIV_INS_NULL}
  );

  sysDefs[DIV_SYSTEM_OPZ]=new DivSysDef(
    "Yamaha TX81Z/YS200", NULL, 0x98, 0, 8, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ}
  );

  sysDefs[DIV_SYSTEM_POKEMINI]=new DivSysDef(
    "Pokémon Mini", NULL, 0x99, 0, 1, false, true, 0, false,
    {"Square"},
    {"SQ"},
    {DIV_CH_PULSE},
    {DIV_INS_STD}
  );

  sysDefs[DIV_SYSTEM_SEGAPCM]=new DivSysDef(
    "SegaPCM", NULL, 0x9b, 0, 16, false, true, 0x151, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_VBOY]=new DivSysDef(
    "Virtual Boy", NULL, 0x9c, 0, 6, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Noise"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "NO"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_NOISE},
    {DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY, DIV_INS_VBOY}
  );

  sysDefs[DIV_SYSTEM_VRC7]=new DivSysDef(
    "Konami VRC7", NULL, 0x9d, 0, 6, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6"},
    {"F1", "F2", "F3", "F4", "F5", "F6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL}
  );

  sysDefs[DIV_SYSTEM_YM2610B]=new DivSysDef(
    "Yamaha YM2610B", NULL, 0x9e, 0, 16, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_SFX_BEEPER]=new DivSysDef(
    "ZX Spectrum Beeper", NULL, 0x9f, 0, 6, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER}
  );

  sysDefs[DIV_SYSTEM_YM2612_EXT]=new DivSysDef(
    "Yamaha YM2612 Extended Channel 3", NULL, 0xa0, 0, 9, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6"},
    {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM},
    {DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_SCC]=new DivSysDef(
    "Konami SCC", NULL, 0xa1, 0, 5, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5"},
    {"CH1", "CH2", "CH3", "CH4", "CH5"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC}
  );

  sysDefs[DIV_SYSTEM_OPL_DRUMS]=new DivSysDef(
    "Yamaha OPL with drums", NULL, 0xa2, 0, 11, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}
  );

  sysDefs[DIV_SYSTEM_OPL2_DRUMS]=new DivSysDef(
    "Yamaha OPL2 with drums", NULL, 0xa3, 0, 11, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}
  );

  sysDefs[DIV_SYSTEM_OPL3_DRUMS]=new DivSysDef(
    "Yamaha OPL3 with drums", NULL, 0xa4, 0, 20, true, false, 0x151, false,
    {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL}
  );

  sysDefs[DIV_SYSTEM_YM2610_FULL]=new DivSysDef(
    "Yamaha YM2610", NULL, 0xa5, 0, 14, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "F2", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_YM2610_FULL_EXT]=new DivSysDef(
    "Yamaha YM2610 Extended Channel 2", NULL, 0xa6, 0, 17, true, false, 0x151, false,
    {"FM 1", "FM 2 OP1", "FM 2 OP2", "FM 2 OP3", "FM 2 OP4", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "O1", "O2", "O3", "O4", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_OPLL_DRUMS]=new DivSysDef(
    "Yamaha OPLL with drums", NULL, 0xa7, 0, 11, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL}
  );

  sysDefs[DIV_SYSTEM_LYNX]=new DivSysDef(
    "Atari Lynx", NULL, 0xa8, 0, 4, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_MIKEY, DIV_INS_MIKEY, DIV_INS_MIKEY, DIV_INS_MIKEY}
  );

  sysDefs[DIV_SYSTEM_QSOUND]=new DivSysDef(
    "Capcom QSound", NULL, 0xe0, 0, 19, false, true, 0x161, false,
    {"PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5", "PCM 6", "PCM 7", "PCM 8", "PCM 9", "PCM 10", "PCM 11", "PCM 12", "PCM 13", "PCM 14", "PCM 15", "PCM 16", "ADPCM 1", "ADPCM 2", "ADPCM 3"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "A1", "A2", "A3"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_VERA]=new DivSysDef(
    "VERA", NULL, 0xac, 0, 17, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "PCM"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "PCM"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM},
    {DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_YM2610B_EXT]=new DivSysDef(
    "Yamaha YM2610B Extended Channel 3", NULL, 0xde, 0, 19, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_SEGAPCM_COMPAT]=new DivSysDef(
    "SegaPCM (compatible 5-channel mode)", NULL, 0xa9, 0, 5, false, true, 0x151, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5"},
    {"P1", "P2", "P3", "P4", "P5"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_X1_010]=new DivSysDef(
    "Seta/Allumer X1-010", NULL, 0xb0, 0, 16, false, true, 0x171, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_BUBSYS_WSG]=new DivSysDef(
    "Konami Bubble System WSG", NULL, 0xad, 0, 2, false, true, 0, false,
    {"Channel 1", "Channel 2"},
    {"CH1", "CH2"},
    {DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_SCC, DIV_INS_SCC}
  );

  // to Grauw: feel free to change this to 24 during development of OPL4's PCM part.
  sysDefs[DIV_SYSTEM_OPL4]=new DivSysDef(
    "Yamaha OPL4", NULL, 0xae, 0, 42, true, true, 0, false,
    {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "FM 16", "FM 17", "FM 18", "PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5", "PCM 6", "PCM 7", "PCM 8", "PCM 9", "PCM 10", "PCM 11", "PCM 12", "PCM 13", "PCM 14", "PCM 15", "PCM 16", "PCM 17", "PCM 18", "PCM 19", "PCM 20", "PCM 21", "PCM 22", "PCM 23", "PCM 24"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P8", "P10", "P11", "P12", "P13", "P14", "P15", "P16", "P17", "P18", "P19", "P20", "P21", "P22", "P23", "P24"},
    {DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM}
  );

  sysDefs[DIV_SYSTEM_OPL4_DRUMS]=new DivSysDef(
    "Yamaha OPL4 with drums", NULL, 0xaf, 0, 44, true, true, 0, false,
    {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "Kick", "Snare", "Tom", "Top", "HiHat", "PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5", "PCM 6", "PCM 7", "PCM 8", "PCM 9", "PCM 10", "PCM 11", "PCM 12", "PCM 13", "PCM 14", "PCM 15", "PCM 16", "PCM 17", "PCM 18", "PCM 19", "PCM 20", "PCM 21", "PCM 22", "PCM 23", "PCM 24"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "BD", "SD", "TM", "TP", "HH", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "P8", "P10", "P11", "P12", "P13", "P14", "P15", "P16", "P17", "P18", "P19", "P20", "P21", "P22", "P23", "P24"},
    {DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM, DIV_INS_MULTIPCM}
  );

  sysDefs[DIV_SYSTEM_ES5506]=new DivSysDef(
    "Ensoniq ES5506", NULL, 0xb1, 0, 32, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "Channel 17", "Channel 18", "Channel 19", "Channel 20", "Channel 21", "Channel 22", "Channel 23", "Channel 24", "Channel 25", "Channel 26", "Channel 27", "Channel 28", "Channel 29", "Channel 30", "Channel 31", "Channel 32"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506, DIV_INS_ES5506}
  );

  sysDefs[DIV_SYSTEM_Y8950]=new DivSysDef(
    "Yamaha Y8950", NULL, 0xb2, 0, 10, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9", "PCM"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "PCM"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PCM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_Y8950_DRUMS]=new DivSysDef(
    "Yamaha Y8950 with drums", NULL, 0xb3, 0, 12, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat", "PCM"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH", "PCM"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_PCM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_SCC_PLUS]=new DivSysDef(
    "Konami SCC+", NULL, 0xb4, 0, 5, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5"},
    {"CH1", "CH2", "CH3", "CH4", "CH5"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC}
  );

  sysDefs[DIV_SYSTEM_SOUND_UNIT]=new DivSysDef(
    "tildearrow Sound Unit", NULL, 0xb5, 0, 8, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"},
    {DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_SU, DIV_INS_SU, DIV_INS_SU, DIV_INS_SU, DIV_INS_SU, DIV_INS_SU, DIV_INS_SU, DIV_INS_SU},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_MSM6295]=new DivSysDef(
    "OKI MSM6295", NULL, 0xaa, 0, 4, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_MSM6258]=new DivSysDef(
    "OKI MSM6258", NULL, 0xab, 0, 1, false, true, 0, false,
    {"Sample"},
    {"PCM"},
    {DIV_CH_PCM},
    {DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_DUMMY]=new DivSysDef(
    "Dummy System", NULL, 0xfd, 0, 8, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"},
    {DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD}
  );

  for (int i=0; i<256; i++) {
    if (sysDefs[i]==NULL) continue;
    if (sysDefs[i]->id!=0) {
      sysFileMapFur[sysDefs[i]->id]=(DivSystem)i;
    }
    if (sysDefs[i]->id_DMF!=0) {
      sysFileMapDMF[sysDefs[i]->id_DMF]=(DivSystem)i;
    }
  }

  systemsRegistered=true;
}
