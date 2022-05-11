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
String DivEngine::getSongSystemName(bool isMultiSystemAcceptable) {
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
  if (isMultiSystemAcceptable) return "multi-system";

  String ret="";
  for (int i=0; i<song.systemLen; i++) {
    if (i>0) ret+=" + ";
    ret+=getSystemName(song.system[i]);
  }

  return ret;
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
  if (sysDefs[which]==NULL) return 0;
  return sysDefs[which]->vgmVersion;
}

#define IS_YM2610 (sysOfChan[ch]==DIV_SYSTEM_YM2610 || sysOfChan[ch]==DIV_SYSTEM_YM2610_EXT || sysOfChan[ch]==DIV_SYSTEM_YM2610_FULL || sysOfChan[ch]==DIV_SYSTEM_YM2610_FULL_EXT || sysOfChan[ch]==DIV_SYSTEM_YM2610B || sysOfChan[ch]==DIV_SYSTEM_YM2610B_EXT)
#define IS_OPM_LIKE (sysOfChan[ch]==DIV_SYSTEM_YM2151 || sysOfChan[ch]==DIV_SYSTEM_OPZ)

#define OP_EFFECT_MULTI(x,c,op,mask) \
  case x: \
    dispatchCmd(DivCommand(c,ch,op,effectVal&mask)); \
    break;

#define OP_EFFECT_SINGLE(x,c,maxOp,mask) \
  case x: \
    if ((effectVal>>4)>=0 && (effectVal>>4)<=maxOp) { \
      dispatchCmd(DivCommand(c,ch,(effectVal>>4)-1,effectVal&mask)); \
    } \
    break;

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

  auto fmPostEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x10: // LFO or noise mode
        if (IS_OPM_LIKE) {
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
        } else {
          dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal));
        }
        break;
      case 0x11: // FB
        dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
        break;
      case 0x12: // TL op1
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x7f));
        break;
      case 0x13: // TL op2
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x7f));
        break;
      case 0x14: // TL op3
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,2,effectVal&0x7f));
        break;
      case 0x15: // TL op4
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,3,effectVal&0x7f));
        break;
      case 0x16: // MULT
        if ((effectVal>>4)>0 && (effectVal>>4)<5) {
          dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
        }
        break;
      case 0x17: // arcade LFO
        if (IS_OPM_LIKE) {
          dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal));
        }
        break;
      case 0x18: // EXT or LFO waveform
        if (IS_OPM_LIKE) {
          dispatchCmd(DivCommand(DIV_CMD_FM_LFO_WAVE,ch,effectVal));
        } else {
          dispatchCmd(DivCommand(DIV_CMD_FM_EXTCH,ch,effectVal));
        }
        break;
      case 0x19: // AR global
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&31));
        break;
      case 0x1a: // AR op1
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&31));
        break;
      case 0x1b: // AR op2
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&31));
        break;
      case 0x1c: // AR op3
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,2,effectVal&31));
        break;
      case 0x1d: // AR op4
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,3,effectVal&31));
        break;
      case 0x1e: // UNOFFICIAL: Arcade AM depth
        dispatchCmd(DivCommand(DIV_CMD_FM_AM_DEPTH,ch,effectVal&127));
        break;
      case 0x1f: // UNOFFICIAL: Arcade PM depth
        dispatchCmd(DivCommand(DIV_CMD_FM_PM_DEPTH,ch,effectVal&127));
        break;
      case 0x20: // Neo Geo PSG mode
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
        }
        break;
      case 0x21: // Neo Geo PSG noise freq
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
        }
        break;
      case 0x22: // UNOFFICIAL: Neo Geo PSG envelope enable
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SET,ch,effectVal));
        }
        break;
      case 0x23: // UNOFFICIAL: Neo Geo PSG envelope period low
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_LOW,ch,effectVal));
        }
        break;
      case 0x24: // UNOFFICIAL: Neo Geo PSG envelope period high
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_HIGH,ch,effectVal));
        }
        break;
      case 0x25: // UNOFFICIAL: Neo Geo PSG envelope slide up
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,-effectVal));
        }
        break;
      case 0x26: // UNOFFICIAL: Neo Geo PSG envelope slide down
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,effectVal));
        }
        break;
      case 0x29: // auto-envelope
        if (IS_YM2610) {
          dispatchCmd(DivCommand(DIV_CMD_AY_AUTO_ENVELOPE,ch,effectVal));
        }
        break;
      // fixed frequency effects on OPZ
      case 0x30: case 0x31: case 0x32: case 0x33:
      case 0x34: case 0x35: case 0x36: case 0x37:
        if (sysOfChan[ch]==DIV_SYSTEM_OPZ) {
          dispatchCmd(DivCommand(DIV_CMD_FM_FIXFREQ,ch,0,((effect&7)<<8)|effectVal));
        }
        break;
      case 0x38: case 0x39: case 0x3a: case 0x3b:
      case 0x3c: case 0x3d: case 0x3e: case 0x3f:
        if (sysOfChan[ch]==DIV_SYSTEM_OPZ) {
          dispatchCmd(DivCommand(DIV_CMD_FM_FIXFREQ,ch,1,((effect&7)<<8)|effectVal));
        }
        break;
      case 0x40: case 0x41: case 0x42: case 0x43:
      case 0x44: case 0x45: case 0x46: case 0x47:
        if (sysOfChan[ch]==DIV_SYSTEM_OPZ) {
          dispatchCmd(DivCommand(DIV_CMD_FM_FIXFREQ,ch,2,((effect&7)<<8)|effectVal));
        }
        break;
      case 0x48: case 0x49: case 0x4a: case 0x4b:
      case 0x4c: case 0x4d: case 0x4e: case 0x4f:
        if (sysOfChan[ch]==DIV_SYSTEM_OPZ) {
          dispatchCmd(DivCommand(DIV_CMD_FM_FIXFREQ,ch,3,((effect&7)<<8)|effectVal));
        }
        break;
      // extra FM effects here
      OP_EFFECT_SINGLE(0x50,DIV_CMD_FM_AM,4,1);
      OP_EFFECT_SINGLE(0x51,DIV_CMD_FM_SL,4,15);
      OP_EFFECT_SINGLE(0x52,DIV_CMD_FM_RR,4,15);
      OP_EFFECT_SINGLE(0x53,DIV_CMD_FM_DT,4,7);
      OP_EFFECT_SINGLE(0x54,DIV_CMD_FM_RS,4,3);
      OP_EFFECT_SINGLE(0x55,DIV_CMD_FM_SSG,4,(IS_OPM_LIKE?3:15));

      OP_EFFECT_MULTI(0x56,DIV_CMD_FM_DR,-1,31);
      OP_EFFECT_MULTI(0x57,DIV_CMD_FM_DR,0,31);
      OP_EFFECT_MULTI(0x58,DIV_CMD_FM_DR,1,31);
      OP_EFFECT_MULTI(0x59,DIV_CMD_FM_DR,2,31);
      OP_EFFECT_MULTI(0x5a,DIV_CMD_FM_DR,3,31);

      OP_EFFECT_MULTI(0x5b,DIV_CMD_FM_D2R,-1,31);
      OP_EFFECT_MULTI(0x5c,DIV_CMD_FM_D2R,0,31);
      OP_EFFECT_MULTI(0x5d,DIV_CMD_FM_D2R,1,31);
      OP_EFFECT_MULTI(0x5e,DIV_CMD_FM_D2R,2,31);
      OP_EFFECT_MULTI(0x5f,DIV_CMD_FM_D2R,3,31);

      OP_EFFECT_SINGLE(0x28,DIV_CMD_FM_REV,4,7);
      OP_EFFECT_SINGLE(0x2a,DIV_CMD_FM_WS,4,7);
      OP_EFFECT_SINGLE(0x2b,DIV_CMD_FM_EG_SHIFT,4,3);
      OP_EFFECT_SINGLE(0x2c,DIV_CMD_FM_FINE,4,15);
      default:
        return false;
    }
    return true;
  };

  auto fmOPLLPostEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x11: // FB
        dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
        break;
      case 0x12: // TL op1
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x3f));
        break;
      case 0x13: // TL op2
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x0f));
        break;
      case 0x16: // MULT
        if ((effectVal>>4)>0 && (effectVal>>4)<3) {
          dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
        }
        break;
      case 0x19: // AR global
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&31));
        break;
      case 0x1a: // AR op1
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&31));
        break;
      case 0x1b: // AR op2
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&31));
        break;

      // extra FM effects here
      OP_EFFECT_SINGLE(0x50,DIV_CMD_FM_AM,2,1);
      OP_EFFECT_SINGLE(0x51,DIV_CMD_FM_SL,2,15);
      OP_EFFECT_SINGLE(0x52,DIV_CMD_FM_RR,2,15);
      OP_EFFECT_SINGLE(0x53,DIV_CMD_FM_VIB,2,1);
      OP_EFFECT_SINGLE(0x54,DIV_CMD_FM_RS,2,3);
      OP_EFFECT_SINGLE(0x55,DIV_CMD_FM_SUS,2,1);

      OP_EFFECT_MULTI(0x56,DIV_CMD_FM_DR,-1,15);
      OP_EFFECT_MULTI(0x57,DIV_CMD_FM_DR,0,15);
      OP_EFFECT_MULTI(0x58,DIV_CMD_FM_DR,1,15);

      OP_EFFECT_SINGLE(0x5b,DIV_CMD_FM_KSR,2,1);
      default:
        return false;
    }
    return true;
  };

  auto fmOPLPostEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x10: // DAM
        dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,effectVal&1));
        break;
      case 0x11: // FB
        dispatchCmd(DivCommand(DIV_CMD_FM_FB,ch,effectVal&7));
        break;
      case 0x12: // TL op1
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,0,effectVal&0x3f));
        break;
      case 0x13: // TL op2
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,1,effectVal&0x3f));
        break;
      case 0x14: // TL op3
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,2,effectVal&0x3f));
        break;
      case 0x15: // TL op4
        dispatchCmd(DivCommand(DIV_CMD_FM_TL,ch,3,effectVal&0x3f));
        break;
      case 0x16: // MULT
        if ((effectVal>>4)>0 && (effectVal>>4)<5) {
          dispatchCmd(DivCommand(DIV_CMD_FM_MULT,ch,(effectVal>>4)-1,effectVal&15));
        }
        break;
      case 0x17: // DVB
        dispatchCmd(DivCommand(DIV_CMD_FM_LFO,ch,2+(effectVal&1)));
        break;
      case 0x19: // AR global
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,-1,effectVal&15));
        break;
      case 0x1a: // AR op1
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,0,effectVal&15));
        break;
      case 0x1b: // AR op2
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,1,effectVal&15));
        break;
      case 0x1c: // AR op3
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,2,effectVal&15));
        break;
      case 0x1d: // AR op4
        dispatchCmd(DivCommand(DIV_CMD_FM_AR,ch,3,effectVal&15));
        break;
      
      // extra FM effects here
      OP_EFFECT_SINGLE(0x50,DIV_CMD_FM_AM,4,1);
      OP_EFFECT_SINGLE(0x51,DIV_CMD_FM_SL,4,15);
      OP_EFFECT_SINGLE(0x52,DIV_CMD_FM_RR,4,15);
      OP_EFFECT_SINGLE(0x53,DIV_CMD_FM_VIB,4,1);
      OP_EFFECT_SINGLE(0x54,DIV_CMD_FM_RS,4,3);
      OP_EFFECT_SINGLE(0x55,DIV_CMD_FM_SUS,4,1);

      OP_EFFECT_MULTI(0x56,DIV_CMD_FM_DR,-1,15);
      OP_EFFECT_MULTI(0x57,DIV_CMD_FM_DR,0,15);
      OP_EFFECT_MULTI(0x58,DIV_CMD_FM_DR,1,15);
      OP_EFFECT_MULTI(0x59,DIV_CMD_FM_DR,2,15);
      OP_EFFECT_MULTI(0x5a,DIV_CMD_FM_DR,3,15);

      OP_EFFECT_SINGLE(0x5b,DIV_CMD_FM_KSR,4,1);
      OP_EFFECT_SINGLE(0x2a,DIV_CMD_FM_WS,4,7);

      default:
        return false;
    }
    return true;
  };

  auto c64PostEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x10: // select waveform
        dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
        break;
      case 0x11: // cutoff
        dispatchCmd(DivCommand(DIV_CMD_C64_CUTOFF,ch,effectVal));
        break;
      case 0x12: // duty
        dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
        break;
      case 0x13: // resonance
        dispatchCmd(DivCommand(DIV_CMD_C64_RESONANCE,ch,effectVal));
        break;
      case 0x14: // filter mode
        dispatchCmd(DivCommand(DIV_CMD_C64_FILTER_MODE,ch,effectVal));
        break;
      case 0x15: // reset time
        dispatchCmd(DivCommand(DIV_CMD_C64_RESET_TIME,ch,effectVal));
        break;
      case 0x1a: // reset mask
        dispatchCmd(DivCommand(DIV_CMD_C64_RESET_MASK,ch,effectVal));
        break;
      case 0x1b: // cutoff reset
        dispatchCmd(DivCommand(DIV_CMD_C64_FILTER_RESET,ch,effectVal));
        break;
      case 0x1c: // duty reset
        dispatchCmd(DivCommand(DIV_CMD_C64_DUTY_RESET,ch,effectVal));
        break;
      case 0x1e: // extended
        dispatchCmd(DivCommand(DIV_CMD_C64_EXTENDED,ch,effectVal));
        break;
      case 0x30: case 0x31: case 0x32: case 0x33:
      case 0x34: case 0x35: case 0x36: case 0x37:
      case 0x38: case 0x39: case 0x3a: case 0x3b:
      case 0x3c: case 0x3d: case 0x3e: case 0x3f: // fine duty
        dispatchCmd(DivCommand(DIV_CMD_C64_FINE_DUTY,ch,((effect&0x0f)<<8)|effectVal));
        break;
      case 0x40: case 0x41: case 0x42: case 0x43:
      case 0x44: case 0x45: case 0x46: case 0x47: // fine cutoff
        dispatchCmd(DivCommand(DIV_CMD_C64_FINE_CUTOFF,ch,((effect&0x07)<<8)|effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  auto ayPostEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x12: // duty on 8930
        dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,0x10+(effectVal&15)));
        break;
      case 0x20: // mode
        dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal&15));
        break;
      case 0x21: // noise freq
        dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
        break;
      case 0x22: // envelope enable
        dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SET,ch,effectVal));
        break;
      case 0x23: // envelope period low
        dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_LOW,ch,effectVal));
        break;
      case 0x24: // envelope period high
        dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_HIGH,ch,effectVal));
        break;
      case 0x25: // envelope slide up
        dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,-effectVal));
        break;
      case 0x26: // envelope slide down
        dispatchCmd(DivCommand(DIV_CMD_AY_ENVELOPE_SLIDE,ch,effectVal));
        break;
      case 0x27: // noise and mask
        dispatchCmd(DivCommand(DIV_CMD_AY_NOISE_MASK_AND,ch,effectVal));
        break;
      case 0x28: // noise or mask
        dispatchCmd(DivCommand(DIV_CMD_AY_NOISE_MASK_OR,ch,effectVal));
        break;
      case 0x29: // auto-envelope
        dispatchCmd(DivCommand(DIV_CMD_AY_AUTO_ENVELOPE,ch,effectVal));
        break;
      case 0x2d: // TEST
        dispatchCmd(DivCommand(DIV_CMD_AY_IO_WRITE,ch,255,effectVal));
        break;
      case 0x2e: // I/O port A
        dispatchCmd(DivCommand(DIV_CMD_AY_IO_WRITE,ch,0,effectVal));
        break;
      case 0x2f: // I/O port B
        dispatchCmd(DivCommand(DIV_CMD_AY_IO_WRITE,ch,1,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  auto segaPCMPostEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x20: // PCM frequency
        dispatchCmd(DivCommand(DIV_CMD_SAMPLE_FREQ,ch,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  sysDefs[DIV_SYSTEM_YMU759]=new DivSysDef(
    "Yamaha YMU759 (MA-2)", NULL, 0x01, 0x01, 17, true, false, 0, false,
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
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x20: // SN noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
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
    {DIV_INS_GB, DIV_INS_GB, DIV_INS_GB, DIV_INS_GB},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // sweep params
          dispatchCmd(DivCommand(DIV_CMD_GB_SWEEP_TIME,ch,effectVal));
          break;
        case 0x14: // sweep direction
          dispatchCmd(DivCommand(DIV_CMD_GB_SWEEP_DIR,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_PCE]=new DivSysDef(
    "PC Engine/TurboGrafx-16", NULL, 0x05, 0x05, 6, false, true, 0x161, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE, DIV_INS_PCE},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x12: // LFO mode
          dispatchCmd(DivCommand(DIV_CMD_PCE_LFO_MODE,ch,effectVal));
          break;
        case 0x13: // LFO speed
          dispatchCmd(DivCommand(DIV_CMD_PCE_LFO_SPEED,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_NES]=new DivSysDef(
    "NES (Ricoh 2A03)", NULL, 0x06, 0x06, 5, false, true, 0x161, false,
    {"Pulse 1", "Pulse 2", "Triangle", "Noise", "PCM"},
    {"S1", "S2", "TR", "NO", "PCM"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_WAVE, DIV_CH_NOISE, DIV_CH_PCM},
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_STD, DIV_INS_AMIGA},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x11: // DMC write
          dispatchCmd(DivCommand(DIV_CMD_NES_DMC,ch,effectVal));
          break;
        case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x13: // sweep up
          dispatchCmd(DivCommand(DIV_CMD_NES_SWEEP,ch,0,effectVal));
          break;
        case 0x14: // sweep down
          dispatchCmd(DivCommand(DIV_CMD_NES_SWEEP,ch,1,effectVal));
          break;
        case 0x18: // DPCM mode
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
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
    {DIV_INS_C64, DIV_INS_C64, DIV_INS_C64},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    c64PostEffectHandler
  );

  sysDefs[DIV_SYSTEM_C64_8580]=new DivSysDef(
    "Commodore 64 (8580)", NULL, 0x07, 0x07, 3, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3"},
    {"CH1", "CH2", "CH3"},
    {DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_C64, DIV_INS_C64, DIV_INS_C64},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    c64PostEffectHandler
  );

  sysDefs[DIV_SYSTEM_ARCADE]=new DivSysDef(
    "DefleCade", NULL, 0x08, 0x08, 13, true, false, 0, true,
    {}, {}, {}, {}
  );

  auto fmHardResetEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x30: // toggle hard-reset
        dispatchCmd(DivCommand(DIV_CMD_FM_HARD_RESET,ch,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  sysDefs[DIV_SYSTEM_YM2610]=new DivSysDef(
    "Neo Geo CD", NULL, 0x09, 0x09, 13, true, true, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6"},
    {"F1", "F2", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_YM2610_EXT]=new DivSysDef(
    "Neo Geo CD Extended Channel 2", NULL, 0x49, 0x49, 16, true, true, 0x151, false,
    {"FM 1", "FM 2 OP1", "FM 2 OP2", "FM 2 OP3", "FM 2 OP4", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6"},
    {"F1", "O1", "O2", "O3", "O4", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6"},
    {DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_AY8910]=new DivSysDef(
    "AY-3-8910", NULL, 0x80, 0, 3, false, true, 0x151, false,
    {"PSG 1", "PSG 2", "PSG 3"},
    {"S1", "S2", "S3"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_AY, DIV_INS_AY, DIV_INS_AY},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    ayPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_AMIGA]=new DivSysDef(
    "Amiga", NULL, 0x81, 0, 4, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // toggle filter
          dispatchCmd(DivCommand(DIV_CMD_AMIGA_FILTER,ch,effectVal));
          break;
        case 0x11: // toggle AM
          dispatchCmd(DivCommand(DIV_CMD_AMIGA_AM,ch,effectVal));
          break;
        case 0x12: // toggle PM
          dispatchCmd(DivCommand(DIV_CMD_AMIGA_PM,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_YM2151]=new DivSysDef(
    "Yamaha YM2151 (OPM)", NULL, 0x82, 0, 8, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  auto opn2EffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x17: // DAC enable
        dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
        break;
      case 0x30: // toggle hard-reset
        dispatchCmd(DivCommand(DIV_CMD_FM_HARD_RESET,ch,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  sysDefs[DIV_SYSTEM_YM2612]=new DivSysDef(
    "Yamaha YM2612 (OPN2)", NULL, 0x83, 0, 6, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6"},
    {"F1", "F2", "F3", "F4", "F5", "F6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM},
    {DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_AMIGA},
    opn2EffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_TIA]=new DivSysDef(
    "Atari 2600", NULL, 0x84, 0, 2, false, true, 0, false,
    {"Channel 1", "Channel 2"},
    {"CH1", "CH2"},
    {DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_TIA, DIV_INS_TIA},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_SAA1099]=new DivSysDef(
    "Philips SAA1099", NULL, 0x97, 0, 6, false, true, 0x171, false,
    {"PSG 1", "PSG 2", "PSG 3", "PSG 4", "PSG 5", "PSG 6"},
    {"S1", "S2", "S3", "S4", "S5", "S6"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099, DIV_INS_SAA1099},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select channel mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x11: // set noise freq
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_FREQ,ch,effectVal));
          break;
        case 0x12: // setup envelope
          dispatchCmd(DivCommand(DIV_CMD_SAA_ENVELOPE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_AY8930]=new DivSysDef(
    "Microchip AY8930", NULL, 0x9a, 0, 3, false, true, 0x151, false,
    {"PSG 1", "PSG 2", "PSG 3"},
    {"S1", "S2", "S3"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_AY8930, DIV_INS_AY8930, DIV_INS_AY8930},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    ayPostEffectHandler
  );

  auto waveOnlyEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x10: // select waveform
        dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  sysDefs[DIV_SYSTEM_VIC20]=new DivSysDef(
    "Commodore VIC-20", NULL, 0x85, 0, 4, false, true, 0, false,
    {"Low", "Mid", "High", "Noise"},
    {"LO", "MID", "HI", "NO"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_NOISE},
    {DIV_INS_VIC, DIV_INS_VIC, DIV_INS_VIC, DIV_INS_VIC},
    {},
    waveOnlyEffectHandler
  );

  sysDefs[DIV_SYSTEM_PET]=new DivSysDef(
    "Commodore PET", NULL, 0x86, 0, 1, false, true, 0, false,
    {"Wave"},
    {"PET"},
    {DIV_CH_PULSE},
    {DIV_INS_PET},
    {},
    waveOnlyEffectHandler
  );

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
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_NULL},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  auto oplEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x30: // toggle hard-reset
        dispatchCmd(DivCommand(DIV_CMD_FM_HARD_RESET,ch,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  sysDefs[DIV_SYSTEM_OPLL]=new DivSysDef(
    "Yamaha YM2413 (OPLL)", NULL, 0x89, 0, 9, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL},
    {},
    oplEffectHandler,
    fmOPLLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_FDS]=new DivSysDef(
    "Famicom Disk System (chip)", NULL, 0x8a, 0, 1, false, true, 0x161, false,
    {"FDS"},
    {"FDS"},
    {DIV_CH_WAVE},
    {DIV_INS_FDS},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // modulation depth
          dispatchCmd(DivCommand(DIV_CMD_FDS_MOD_DEPTH,ch,effectVal));
          break;
        case 0x12: // modulation enable/high
          dispatchCmd(DivCommand(DIV_CMD_FDS_MOD_HIGH,ch,effectVal));
          break;
        case 0x13: // modulation low
          dispatchCmd(DivCommand(DIV_CMD_FDS_MOD_LOW,ch,effectVal));
          break;
        case 0x14: // modulation pos
          dispatchCmd(DivCommand(DIV_CMD_FDS_MOD_POS,ch,effectVal));
          break;
        case 0x15: // modulation wave
          dispatchCmd(DivCommand(DIV_CMD_FDS_MOD_WAVE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_MMC5]=new DivSysDef(
    "MMC5", NULL, 0x8b, 0, 3, false, true, 0, false,
    {"Pulse 1", "Pulse 2", "PCM"},
    {"S1", "S2", "PCM"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM},
    {DIV_INS_STD, DIV_INS_STD, DIV_INS_AMIGA},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x11: // DMC write
          dispatchCmd(DivCommand(DIV_CMD_NES_DMC,ch,effectVal));
          break;
        case 0x12: // duty or noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_N163]=new DivSysDef(
    "Namco 163", NULL, 0x8c, 0, 8, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163, DIV_INS_N163},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select instrument waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // select instrument waveform position in RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_POSITION,ch,effectVal));
          break;
        case 0x12: // select instrument waveform length in RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LENGTH,ch,effectVal));
          break;
        case 0x13: // change instrument waveform update mode
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_MODE,ch,effectVal));
          break;
        case 0x14: // select waveform for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOAD,ch,effectVal));
          break;
        case 0x15: // select waveform position for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOADPOS,ch,effectVal));
          break;
        case 0x16: // select waveform length for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOADLEN,ch,effectVal));
          break;
        case 0x17: // change waveform load mode
          dispatchCmd(DivCommand(DIV_CMD_N163_WAVE_LOADMODE,ch,effectVal));
          break;
        case 0x18: // change channel limits
          dispatchCmd(DivCommand(DIV_CMD_N163_CHANNEL_LIMIT,ch,effectVal));
          break;
        case 0x20: // (global) select waveform for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOAD,ch,effectVal));
          break;
        case 0x21: // (global) select waveform position for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOADPOS,ch,effectVal));
          break;
        case 0x22: // (global) select waveform length for load to RAM
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOADLEN,ch,effectVal));
          break;
        case 0x23: // (global) change waveform load mode
          dispatchCmd(DivCommand(DIV_CMD_N163_GLOBAL_WAVE_LOADMODE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_OPN]=new DivSysDef(
    "Yamaha YM2203 (OPN)", NULL, 0x8d, 0, 6, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "PSG 1", "PSG 2", "PSG 3"},
    {"F1", "F2", "F3", "S1", "S2", "S3"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_PC98]=new DivSysDef(
    "Yamaha YM2608 (OPNA)", NULL, 0x8e, 0, 16, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Square 1", "Square 2", "Square 3", "Kick", "Snare", "Top", "HiHat", "Tom", "Rim", "ADPCM"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "BD", "SD", "TP", "HH", "TM", "RM", "P"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA}
  );

  sysDefs[DIV_SYSTEM_OPL]=new DivSysDef(
    "Yamaha YM3526 (OPL)", NULL, 0x8f, 0, 9, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL},
    {},
    oplEffectHandler,
    fmOPLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_OPL2]=new DivSysDef(
    "Yamaha YM3812 (OPL2)", NULL, 0x90, 0, 9, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8", "FM 9"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL},
    {},
    oplEffectHandler,
    fmOPLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_OPL3]=new DivSysDef(
    "Yamaha YMF262 (OPL3)", NULL, 0x91, 0, 18, true, false, 0x151, false,
    {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "FM 16", "FM 17", "FM 18"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18"},
    {DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL},
    {},
    oplEffectHandler,
    fmOPLPostEffectHandler
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
    {DIV_INS_NULL, DIV_INS_AMIGA, DIV_INS_NULL, DIV_INS_NULL},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // noise mode
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        case 0x12: // sweep period
          dispatchCmd(DivCommand(DIV_CMD_WS_SWEEP_TIME,ch,effectVal));
          break;
        case 0x13: // sweep amount
          dispatchCmd(DivCommand(DIV_CMD_WS_SWEEP_AMOUNT,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_OPZ]=new DivSysDef(
    "Yamaha YM2414 (OPZ)", NULL, 0x98, 0, 8, true, false, 0, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "FM 7", "FM 8"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ, DIV_INS_OPZ},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x2f: // toggle hard-reset
          dispatchCmd(DivCommand(DIV_CMD_FM_HARD_RESET,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    },
    fmPostEffectHandler
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
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    segaPCMPostEffectHandler
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
    {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL},
    {},
    oplEffectHandler,
    fmOPLLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_YM2610B]=new DivSysDef(
    "Yamaha YM2610B (OPNB-B)", NULL, 0x9e, 0, 16, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_SFX_BEEPER]=new DivSysDef(
    "ZX Spectrum Beeper", NULL, 0x9f, 0, 6, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6"},
    {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER, DIV_INS_BEEPER}
  );

  sysDefs[DIV_SYSTEM_YM2612_EXT]=new DivSysDef(
    "Yamaha YM2612 (OPN2) Extended Channel 3", NULL, 0xa0, 0, 9, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6"},
    {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM},
    {DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_NULL, DIV_INS_AMIGA},
    opn2EffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_SCC]=new DivSysDef(
    "Konami SCC", NULL, 0xa1, 0, 5, false, true, 0x161, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5"},
    {"CH1", "CH2", "CH3", "CH4", "CH5"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC},
    {},
    waveOnlyEffectHandler
  );

  auto oplDrumsEffectHandler=[this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
    switch (effect) {
      case 0x18: // drum mode toggle
        dispatchCmd(DivCommand(DIV_CMD_FM_EXTCH,ch,effectVal));
        break;
      case 0x30: // toggle hard-reset
        dispatchCmd(DivCommand(DIV_CMD_FM_HARD_RESET,ch,effectVal));
        break;
      default:
        return false;
    }
    return true;
  };

  sysDefs[DIV_SYSTEM_OPL_DRUMS]=new DivSysDef(
    "Yamaha YM3526 (OPL) with drums", NULL, 0xa2, 0, 11, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL},
    {},
    oplDrumsEffectHandler,
    fmOPLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_OPL2_DRUMS]=new DivSysDef(
    "Yamaha YM3812 (OPL2) with drums", NULL, 0xa3, 0, 11, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL},
    {},
    oplDrumsEffectHandler,
    fmOPLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_OPL3_DRUMS]=new DivSysDef(
    "Yamaha YMF262 (OPL3) with drums", NULL, 0xa4, 0, 20, true, false, 0x151, false,
    {"4OP 1", "FM 2", "4OP 3", "FM 4", "4OP 5", "FM 6", "4OP 7", "FM 8", "4OP 9", "FM 10", "4OP 11", "FM 12", "FM 13", "FM 14", "FM 15", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL, DIV_INS_OPL},
    {},
    oplDrumsEffectHandler,
    fmOPLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_YM2610_FULL]=new DivSysDef(
    "Yamaha YM2610 (OPNB)", NULL, 0xa5, 0, 14, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "F2", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_YM2610_FULL_EXT]=new DivSysDef(
    "Yamaha YM2610 (OPNB) Extended Channel 2", NULL, 0xa6, 0, 17, true, false, 0x151, false,
    {"FM 1", "FM 2 OP1", "FM 2 OP2", "FM 2 OP3", "FM 2 OP4", "FM 3", "FM 4", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "O1", "O2", "O3", "O4", "F3", "F4", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_OPLL_DRUMS]=new DivSysDef(
    "Yamaha YM2413 (OPLL) with drums", NULL, 0xa7, 0, 11, true, false, 0x150, false,
    {"FM 1", "FM 2", "FM 3", "FM 4", "FM 5", "FM 6", "Kick", "Snare", "Tom", "Top", "HiHat"},
    {"F1", "F2", "F3", "F4", "F5", "F6", "BD", "SD", "TM", "TP", "HH"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL, DIV_INS_OPLL},
    {},
    oplDrumsEffectHandler,
    fmOPLLPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_LYNX]=new DivSysDef(
    "Atari Lynx", NULL, 0xa8, 0, 4, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4"},
    {"CH1", "CH2", "CH3", "CH4"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_MIKEY, DIV_INS_MIKEY, DIV_INS_MIKEY, DIV_INS_MIKEY},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      if (effect>=0x30 && effect<0x40) {
        int value=((int)(effect&0x0f)<<8)|effectVal;
        dispatchCmd(DivCommand(DIV_CMD_LYNX_LFSR_LOAD,ch,value));
        return true;
      }
      return false;
    }
  );

  sysDefs[DIV_SYSTEM_QSOUND]=new DivSysDef(
    "Capcom QSound", NULL, 0xe0, 0, 19, false, true, 0x161, false,
    {"PCM 1", "PCM 2", "PCM 3", "PCM 4", "PCM 5", "PCM 6", "PCM 7", "PCM 8", "PCM 9", "PCM 10", "PCM 11", "PCM 12", "PCM 13", "PCM 14", "PCM 15", "PCM 16", "ADPCM 1", "ADPCM 2", "ADPCM 3"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "A1", "A2", "A3"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_NOISE, DIV_CH_NOISE, DIV_CH_NOISE},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // echo feedback
          dispatchCmd(DivCommand(DIV_CMD_QSOUND_ECHO_FEEDBACK,ch,effectVal));
          break;
        case 0x11: // echo level
          dispatchCmd(DivCommand(DIV_CMD_QSOUND_ECHO_LEVEL,ch,effectVal));
          break;
        case 0x12: // surround
          dispatchCmd(DivCommand(DIV_CMD_QSOUND_SURROUND,ch,effectVal));
          break;
        default:
          if ((effect&0xf0)==0x30) {
            dispatchCmd(DivCommand(DIV_CMD_QSOUND_ECHO_DELAY,ch,((effect & 0x0f) << 8) | effectVal));
          } else {
            return false;
          }
          break;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_VERA]=new DivSysDef(
    "VERA", NULL, 0xac, 0, 17, false, true, 0, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16", "PCM"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "PCM"},
    {DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM},
    {DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_VERA, DIV_INS_AMIGA},
    {},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x20: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x22: // duty
          dispatchCmd(DivCommand(DIV_CMD_STD_NOISE_MODE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_YM2610B_EXT]=new DivSysDef(
    "Yamaha YM2610B (OPNB-B) Extended Channel 3", NULL, 0xde, 0, 19, true, false, 0x151, false,
    {"FM 1", "FM 2", "FM 3 OP1", "FM 3 OP2", "FM 3 OP3", "FM 3 OP4", "FM 4", "FM 5", "FM 6", "PSG 1", "PSG 2", "PSG 3", "ADPCM-A 1", "ADPCM-A 2", "ADPCM-A 3", "ADPCM-A 4", "ADPCM-A 5", "ADPCM-A 6", "ADPCM-B"},
    {"F1", "F2", "O1", "O2", "O3", "O4", "F4", "F5", "F6", "S1", "S2", "S3", "P1", "P2", "P3", "P4", "P5", "P6", "B"},
    {DIV_CH_FM, DIV_CH_FM, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_OP, DIV_CH_FM, DIV_CH_FM, DIV_CH_FM, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PULSE, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_FM, DIV_INS_AY, DIV_INS_AY, DIV_INS_AY, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    fmHardResetEffectHandler,
    fmPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_SEGAPCM_COMPAT]=new DivSysDef(
    "SegaPCM (compatible 5-channel mode)", NULL, 0xa9, 0, 5, false, true, 0x151, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5"},
    {"P1", "P2", "P3", "P4", "P5"},
    {DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM, DIV_CH_PCM},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    {},
    [](int,unsigned char,unsigned char) -> bool {return false;},
    segaPCMPostEffectHandler
  );

  sysDefs[DIV_SYSTEM_X1_010]=new DivSysDef(
    "Seta/Allumer X1-010", NULL, 0xb0, 0, 16, false, true, 0x171, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8", "Channel 9", "Channel 10", "Channel 11", "Channel 12", "Channel 13", "Channel 14", "Channel 15", "Channel 16"},
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010, DIV_INS_X1_010},
    {DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA, DIV_INS_AMIGA},
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x10: // select waveform
          dispatchCmd(DivCommand(DIV_CMD_WAVE,ch,effectVal));
          break;
        case 0x11: // select envelope shape
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_SHAPE,ch,effectVal));
          break;
        case 0x17: // PCM enable
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_MODE,ch,(effectVal>0)));
          break;
        default:
          return false;
      }
      return true;
    },
    [this](int ch, unsigned char effect, unsigned char effectVal) -> bool {
      switch (effect) {
        case 0x20: // PCM frequency
          dispatchCmd(DivCommand(DIV_CMD_SAMPLE_FREQ,ch,effectVal));
          break;
        case 0x22: // envelope mode
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_MODE,ch,effectVal));
          break;
        case 0x23: // envelope period
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_PERIOD,ch,effectVal));
          break;
        case 0x25: // envelope slide up
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_SLIDE,ch,effectVal));
          break;
        case 0x26: // envelope slide down
          dispatchCmd(DivCommand(DIV_CMD_X1_010_ENVELOPE_SLIDE,ch,-effectVal));
          break;
        case 0x29: // auto-envelope
          dispatchCmd(DivCommand(DIV_CMD_X1_010_AUTO_ENVELOPE,ch,effectVal));
          break;
        default:
          return false;
      }
      return true;
    }
  );

  sysDefs[DIV_SYSTEM_BUBSYS_WSG]=new DivSysDef(
    "Konami Bubble System WSG", NULL, 0xad, 0, 2, false, true, 0, false,
    {"Channel 1", "Channel 2"},
    {"CH1", "CH2"},
    {DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_SCC, DIV_INS_SCC},
    {},
    waveOnlyEffectHandler
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
    "Konami SCC+", NULL, 0xb4, 0, 5, false, true, 0x161, false,
    {"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5"},
    {"CH1", "CH2", "CH3", "CH4", "CH5"},
    {DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE, DIV_CH_WAVE},
    {DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC, DIV_INS_SCC},
    {},
    waveOnlyEffectHandler
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
