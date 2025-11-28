/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#ifndef _SYS_DEF_H
#define _SYS_DEF_H

#include "dispatch.h"
#include "instrument.h"
#include <functional>
#include <initializer_list>

typedef int EffectValConversion(unsigned char,unsigned char);

struct EffectHandler {
  DivDispatchCmds dispatchCmd;
  const char* description;
  EffectValConversion* val;
  EffectValConversion* val2;
  EffectHandler(
    DivDispatchCmds dispatchCmd_,
    const char* description_,
    EffectValConversion val_=NULL,
    EffectValConversion val2_=NULL
  ):
  dispatchCmd(dispatchCmd_),
  description(description_),
  val(val_),
  val2(val2_) {}
};

struct DivDoNotHandleEffect {
};

typedef std::unordered_map<unsigned char,const EffectHandler> EffectHandlerMap;

enum DivChanTypes {
  DIV_CH_FM=0,
  DIV_CH_PULSE=1,
  DIV_CH_NOISE=2,
  DIV_CH_WAVE=3,
  DIV_CH_PCM=4,
  DIV_CH_OP=5
};

struct DivChanDef {
  String name;
  String shortName;
  int type;
  // 0: primary
  // 1: alternate (usually PCM)
  DivInstrumentType insType[2];

  DivChanDef(const String& n, const String& sn, int t, DivInstrumentType insT, DivInstrumentType insT2=DIV_INS_NULL):
    name(n),
    shortName(sn),
    type(t),
    insType{insT,insT2} {}
  DivChanDef():
    name("??"),
    shortName("??"),
    type(DIV_CH_NOISE),
    insType{DIV_INS_NULL,DIV_INS_NULL} {}
};

struct DivChanDefFunc {
  std::vector<DivChanDef> list;
  std::function<DivChanDef(unsigned short)> func;

  DivChanDef operator()(int& ch) const {
    if (ch<0) {
      return DivChanDef("??","??",DIV_CH_NOISE,DIV_INS_NULL);
    }
    if (ch<(int)list.size()) return list[ch];
    if (func==NULL) {
      return DivChanDef("??","??",DIV_CH_NOISE,DIV_INS_NULL);
    }
    return func(ch);
  }

  DivChanDefFunc(std::initializer_list<DivChanDef> l):
    list(l), func(NULL) {}
  DivChanDefFunc(std::function<DivChanDef(unsigned short)> f):
    list(), func(f) {}
  DivChanDefFunc():
    list(), func(NULL) {}
};

struct DivSysDef {
  const char* name;
  const char* nameJ;
  const char* description;
  unsigned char id;
  unsigned char id_DMF;
  int channels, minChans, maxChans;
  bool isFM, isSTD, isCompound;
  // width 0: variable
  // height 0: no wavetable support
  unsigned short waveWidth, waveHeight;
  unsigned int vgmVersion;
  unsigned int sampleFormatMask;

  DivChanDefFunc getChanDef;

  const EffectHandlerMap effectHandlers;
  const EffectHandlerMap postEffectHandlers;
  const EffectHandlerMap preEffectHandlers;
  DivSysDef(
    const char* sysName, const char* sysNameJ, unsigned char fileID, unsigned char fileID_DMF, int chans, int minCh, int maxCh,
    bool isFMChip, bool isSTDChip, unsigned int vgmVer, bool compound, unsigned int formatMask, unsigned short waveWid, unsigned short waveHei,
    const char* desc,
    DivChanDefFunc gcdFunc,
    const EffectHandlerMap fxHandlers_={},
    const EffectHandlerMap postFxHandlers_={},
    const EffectHandlerMap preFxHandlers_={}):
    name(sysName),
    nameJ(sysNameJ),
    description(desc),
    id(fileID),
    id_DMF(fileID_DMF),
    channels(chans),
    minChans(minCh),
    maxChans(maxCh),
    isFM(isFMChip),
    isSTD(isSTDChip),
    isCompound(compound),
    waveWidth(waveWid),
    waveHeight(waveHei),
    vgmVersion(vgmVer),
    sampleFormatMask(formatMask),
    getChanDef(gcdFunc),
    effectHandlers(fxHandlers_),
    postEffectHandlers(postFxHandlers_),
    preEffectHandlers(preFxHandlers_) {
  }
};

enum DivSystem {
  DIV_SYSTEM_NULL=0,
  DIV_SYSTEM_YMU759,
  DIV_SYSTEM_GENESIS, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_GENESIS_EXT, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_SMS,
  DIV_SYSTEM_SMS_OPLL, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_GB,
  DIV_SYSTEM_PCE,
  DIV_SYSTEM_NES,
  DIV_SYSTEM_NES_VRC7, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_NES_FDS, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_C64_6581,
  DIV_SYSTEM_C64_8580,
  DIV_SYSTEM_ARCADE, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_MSX2, // ** COMPOUND SYSTEM - DO NOT USE! **
  DIV_SYSTEM_YM2610_CRAP,
  DIV_SYSTEM_YM2610_CRAP_EXT,
  
  DIV_SYSTEM_AY8910,
  DIV_SYSTEM_AMIGA,
  DIV_SYSTEM_YM2151,
  DIV_SYSTEM_YM2612,
  DIV_SYSTEM_TIA,
  DIV_SYSTEM_SAA1099,
  DIV_SYSTEM_AY8930,
  DIV_SYSTEM_VIC20,
  DIV_SYSTEM_PET,
  DIV_SYSTEM_SNES,
  DIV_SYSTEM_VRC6,
  DIV_SYSTEM_OPLL,
  DIV_SYSTEM_FDS,
  DIV_SYSTEM_MMC5,
  DIV_SYSTEM_N163,
  DIV_SYSTEM_YM2203,
  DIV_SYSTEM_YM2203_EXT,
  DIV_SYSTEM_YM2608,
  DIV_SYSTEM_YM2608_EXT,
  DIV_SYSTEM_OPL,
  DIV_SYSTEM_OPL2,
  DIV_SYSTEM_OPL3,
  DIV_SYSTEM_MULTIPCM,
  DIV_SYSTEM_PCSPKR,
  DIV_SYSTEM_POKEY,
  DIV_SYSTEM_RF5C68,
  DIV_SYSTEM_SWAN,
  DIV_SYSTEM_OPZ,
  DIV_SYSTEM_POKEMINI,
  DIV_SYSTEM_SEGAPCM,
  DIV_SYSTEM_VBOY,
  DIV_SYSTEM_VRC7,
  DIV_SYSTEM_YM2610B,
  DIV_SYSTEM_SFX_BEEPER,
  DIV_SYSTEM_SFX_BEEPER_QUADTONE,
  DIV_SYSTEM_YM2612_EXT,
  DIV_SYSTEM_SCC,
  DIV_SYSTEM_OPL_DRUMS,
  DIV_SYSTEM_OPL2_DRUMS,
  DIV_SYSTEM_OPL3_DRUMS,
  DIV_SYSTEM_YM2610_FULL,
  DIV_SYSTEM_YM2610_FULL_EXT,
  DIV_SYSTEM_OPLL_DRUMS,
  DIV_SYSTEM_LYNX,
  DIV_SYSTEM_QSOUND,
  DIV_SYSTEM_VERA,
  DIV_SYSTEM_YM2610B_EXT,
  DIV_SYSTEM_SEGAPCM_COMPAT,
  DIV_SYSTEM_X1_010,
  DIV_SYSTEM_BUBSYS_WSG,
  DIV_SYSTEM_OPL4,
  DIV_SYSTEM_OPL4_DRUMS,
  DIV_SYSTEM_ES5506,
  DIV_SYSTEM_Y8950,
  DIV_SYSTEM_Y8950_DRUMS,
  DIV_SYSTEM_SCC_PLUS,
  DIV_SYSTEM_SOUND_UNIT,
  DIV_SYSTEM_MSM6295,
  DIV_SYSTEM_MSM6258,
  DIV_SYSTEM_YMZ280B,
  DIV_SYSTEM_NAMCO,
  DIV_SYSTEM_NAMCO_15XX,
  DIV_SYSTEM_NAMCO_CUS30,
  DIV_SYSTEM_YM2612_DUALPCM,
  DIV_SYSTEM_YM2612_DUALPCM_EXT,
  DIV_SYSTEM_MSM5232,
  DIV_SYSTEM_T6W28,
  DIV_SYSTEM_K007232,
  DIV_SYSTEM_GA20,
  DIV_SYSTEM_PCM_DAC,
  DIV_SYSTEM_PONG,
  DIV_SYSTEM_DUMMY,
  DIV_SYSTEM_YM2612_CSM,
  DIV_SYSTEM_YM2610_CSM,
  DIV_SYSTEM_YM2610B_CSM,
  DIV_SYSTEM_YM2203_CSM,
  DIV_SYSTEM_YM2608_CSM,
  DIV_SYSTEM_SM8521,
  DIV_SYSTEM_PV1000,
  DIV_SYSTEM_K053260,
  DIV_SYSTEM_TED,
  DIV_SYSTEM_C140,
  DIV_SYSTEM_C219,
  DIV_SYSTEM_ESFM,
  DIV_SYSTEM_POWERNOISE,
  DIV_SYSTEM_DAVE,
  DIV_SYSTEM_NDS,
  DIV_SYSTEM_GBA_DMA,
  DIV_SYSTEM_GBA_MINMOD,
  DIV_SYSTEM_5E01,
  DIV_SYSTEM_BIFURCATOR,
  DIV_SYSTEM_SID2,
  DIV_SYSTEM_SUPERVISION,
  DIV_SYSTEM_UPD1771C,
  DIV_SYSTEM_SID3,
  DIV_SYSTEM_C64_PCM,

  DIV_SYSTEM_MAX
};

#endif
