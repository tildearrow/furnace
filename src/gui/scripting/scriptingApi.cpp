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

#include "scripting.h"
#include "../gui.h"

static const char* macroTypeValueNames[][2]={
  {"macroVolume","volume"},
  {"macroArp","arp"},
  {"macroDuty","duty"},
  {"macroWave","wave"},
  {"macroPitch","pitch"},
  {"macroEx1","ex1"},
  {"macroEx2","ex2"},
  {"macroEx3","ex3"},
  {"macroAlg","alg"},
  {"macroFeedback","feedback"},
  {"macroFMS","fms"},
  {"macroAMS","ams"},
  {"macroPanLeft","panLeft"},
  {"macroPanRight","panRight"},
  {"macroPhaseReset","phaseReset"},
  {"macroEx4","ex4"},
  {"macroEx5","ex5"},
  {"macroEx6","ex6"},
  {"macroEx7","ex7"},
  {"macroEx8","ex8"},
  {"macroEx9","ex9"},
  {"macroEx10","ex10"},
  {NULL,NULL}
};

typedef void (*InsFeatureWriteFunc)(DivInstrument*,lua_State*);
typedef void (*InsFeatureReadFunc)(DivInstrument*,lua_State*,int);

struct InsFeatureDef {
  const char* valueName;
  InsFeatureWriteFunc featureWriteFunc;
  InsFeatureReadFunc featureReadFunc;
};

static const InsFeatureDef insFeatures[]={
  {"featureFM",writeFeatureFM,readFeatureFM},
  {"featureGB",writeFeatureGB,readFeatureGB},
  {"featureC64",writeFeatureC64,readFeatureC64},
  {"featureAmiga",writeFeatureAmiga,readFeatureAmiga},
  {"featureX1",writeFeatureX1,readFeatureX1},
  {"feature163",writeFeatureN163,readFeatureN163},
  {"featureFDS",writeFeatureFDS,readFeatureFDS},
  {"featureMultiPCM",writeFeatureMultiPCM,readFeatureMultiPCM},
  {"featureWaveSynth",writeFeatureWaveSynth,readFeatureWaveSynth},
  {"featureSoundUnit",writeFeatureSoundUnit,readFeatureSoundUnit},
  {"featureES5506",writeFeatureES5506,readFeatureES5506},
  {"featureSNES",writeFeatureSNES,readFeatureSNES},
  {"featureESFM",writeFeatureESFM,readFeatureESFM},
  {"featurePowerNoise",writeFeaturePowerNoise,readFeaturePowerNoise},
  {"featureSID2",writeFeatureSID2,readFeatureSID2},
  {"featureSID3",writeFeatureSID3,readFeatureSID3},
  {"featureKlattsch",writeFeatureKlattsch,readFeatureKlattsch},
  {NULL,NULL,NULL}
};

// NULLs are unsupported chips
static const char* chipIdNames[]={
  NULL,
  NULL,
  NULL,
  NULL,
  "chipSN76489",
  NULL,
  "chipGB",
  "chipPCE",
  "chipNES",
  NULL,
  NULL,
  "chip6581",
  "chip8580",
  NULL,
  NULL,
  NULL,
  NULL,
  "chipAY",
  "chipAmiga",
  "chipYM2151",
  "chipYM2612",
  "chipTIA",
  "chipSAA1099",
  "chipAY8930",
  "chipVIC20",
  "chipPET",
  "chipSNES",
  "chipVRC6",
  "chipYM2413",
  "chipFDS",
  "chipMMC5",
  "chip163",
  "chipYM2203",
  "chipYM2203Ext",
  "chipYM2608",
  "chipYM2608Ext",
  "chipYM3526",
  "chipYM3812",
  "chipYMF262",
  "chipMultiPCM",
  "chipPCSpeaker",
  "chipPOKEY",
  "chipRF5C68",
  "chipSwan",
  "chipYM2414",
  "chipPokeMini",
  "chipSegaPCM",
  "chipVB",
  "chipVRC7",
  "chipYM2610B",
  "chipZXSFX",
  "chipZXQuadtone",
  "chipYM2612Ext",
  "chipSCC",
  "chipYM3526Drums",
  "chipYM3812Drums",
  "chipYMF262Drums",
  "chipYM2610",
  "chipYM2610Ext",
  "chipYM2413Drums",
  "chipLynx",
  "chipQSound",
  "chipVERA",
  "chipYM2610BExt",
  NULL,
  "chipX1",
  "chipBubSysWSG",
  "chipYMF278B",
  "chipYMF278BDrums",
  "chipES5506",
  "chipY8950",
  "chipY8950Drums",
  "chipSCCPlus",
  "chipSoundUnit",
  "chipMSM6295",
  "chipMSM6258",
  "chipYMZ280B",
  "chipNamcoWSG",
  "chipNamco15xx",
  "chipNamcoCUS30",
  "chipYM2612DualPCM",
  "chipYM2612DualPCMExt",
  "chipMSM5232",
  "chipT6W28",
  "chipK007232",
  "chipGA20",
  "chipPCMDAC",
  "chipPong",
  "chipDummy",
  "chipYM2612CSM",
  "chipYM2610CSM",
  "chipYM2610BCSM",
  "chipYM2203CSM",
  "chipYM2608CSM",
  "chipSM8521",
  "chipPV1000",
  "chipK053260",
  "chipTED",
  "chipC140",
  "chipC219",
  "chipESFM",
  "chipPowerNoise",
  "chipDave",
  "chipNDS",
  "chipGBADMA",
  "chipGBAMinmod",
  "chip5E01",
  "chipBifurcator",
  "chipSID2",
  "chipSupervision",
  /*"chipuPD1771C"*/ NULL,
  "chipSID3",
  "chip6581PCM",
  "chipNamcoPolePos",
  "chipKlattsch"
};

static_assert((sizeof(chipIdNames)/sizeof(const char*))==DIV_SYSTEM_MAX,"chipIdNames: missing chip!");

static FurnaceGUI* externGUI;

/// FUNCTIONS

_CF(version) {
  lua_pushinteger(s,DIV_ENGINE_VERSION);
  return 1;
}

_CF(versionStr) {
  lua_pushliteral(s,DIV_VERSION);
  return 1;
}

_CF(showError) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  showError(lua_tostring(s,1));
  return 0;
}

_CF(getCursor) {
  lua_pushinteger(s,cursor.xCoarse);
  lua_pushinteger(s,cursor.xFine);
  lua_pushinteger(s,cursor.y);
  return 3;
}

_CF(setCursor) {
  CHECK_ARGS(3);

  CHECK_TYPE_INTEGER(1);
  CHECK_TYPE_INTEGER(2);
  CHECK_TYPE_INTEGER(3);

  cursor.xCoarse=lua_tointeger(s,1);
  cursor.xFine=lua_tointeger(s,2);
  cursor.y=lua_tointeger(s,3);

  return 0;
}

_CF(getSelStart) {
  lua_pushinteger(s,selStart.xCoarse);
  lua_pushinteger(s,selStart.xFine);
  lua_pushinteger(s,selStart.y);
  return 3;
}

_CF(setSelStart) {
  CHECK_ARGS(3);

  CHECK_TYPE_INTEGER(1);
  CHECK_TYPE_INTEGER(2);
  CHECK_TYPE_INTEGER(3);

  selStart.xCoarse=lua_tointeger(s,1);
  selStart.xFine=lua_tointeger(s,2);
  selStart.y=lua_tointeger(s,3);

  return 0;
}

_CF(getSelEnd) {
  lua_pushinteger(s,selEnd.xCoarse);
  lua_pushinteger(s,selEnd.xFine);
  lua_pushinteger(s,selEnd.y);
  return 3;
}

_CF(setSelEnd) {
  CHECK_ARGS(3);

  CHECK_TYPE_INTEGER(1);
  CHECK_TYPE_INTEGER(2);
  CHECK_TYPE_INTEGER(3);

  selEnd.xCoarse=lua_tointeger(s,1);
  selEnd.xFine=lua_tointeger(s,2);
  selEnd.y=lua_tointeger(s,3);

  return 0;
}

_CF(getCurOrder) {
  lua_pushinteger(s,e->getOrder());
  return 1;
}

_CF(getCurRow) {
  lua_pushinteger(s,e->getRow());
  return 1;
}

_CF(getPlayTimeSec) {
  lua_pushinteger(s,e->getCurTime().seconds);
  return 1;
}

_CF(getPlayTimeMicro) {
  lua_pushinteger(s,e->getCurTime().micros);
  return 1;
}

_CF(getPlayTimeTicks) {
  lua_pushinteger(s,e->getTotalTicksR());
  return 1;
}

_CF(isPlaying) {
  lua_pushboolean(s,e->isPlaying());
  return 1;
}

_CF(isRunning) {
  lua_pushboolean(s,e->isRunning());
  return 1;
}

_CF(isFreelance) {
  lua_pushboolean(s,e->isFreelance());
  return 1;
}

_CF(getChanCount) {
  lua_pushinteger(s,e->getTotalChannelCount());
  return 1;
}

_CF(getCurSubSong) {
  lua_pushinteger(s,e->getCurrentSubSong());
  return 1;
}

_CF(getEditOrder) {
  lua_pushinteger(s,curOrder);
  return 1;
}

_CF(registerMenuEntry) {
  CHECK_ARGS(3);
  CHECK_TYPE_STRING(1);
  CHECK_TYPE_STRING(2);
  CHECK_TYPE_FUNCTION(3);

  const char* menuName=lua_tostring(s,1);
  const char* menuEntry=lua_tostring(s,2);
  luaFunction funcID=luaL_ref(s,LUA_REGISTRYINDEX);

  FurnaceGUIScriptAction action;
  action.function=funcID;
  action.state=s;
  scriptMenus[menuName][menuEntry]=action;

  return 0;
}

_CF(getCurIns) {
  lua_pushinteger(s,curIns);
  return 1;
}

_CF(getCurWave) {
  lua_pushinteger(s,curWave);
  return 1;
}

_CF(getCurSample) {
  lua_pushinteger(s,curSample);
  return 1;
}

_CF(setCurIns) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  curIns=lua_tointeger(s,1);
  wavePreviewInit=true;
  updateFMPreview=true;
  return 0;
}

_CF(setCurWave) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  curWave=lua_tointeger(s,1);
  return 0;
}

_CF(setCurSample) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  curSample=lua_tointeger(s,1);
  samplePos=0;
  updateSampleTex=true;
  return 0;
}

_CF(getOctave) {
  lua_pushinteger(s,curOctave);
  return 1;
}

_CF(getEditStep) {
  lua_pushinteger(s,editStep);
  return 1;
}

_CF(getEditStepCoarse) {
  lua_pushinteger(s,editStepCoarse);
  return 1;
}

_CF(getOrderEditMode) {
  lua_pushinteger(s,orderEditMode);
  return 1;
}

_CF(getOrderCursor) {
  lua_pushinteger(s,orderCursor);
  return 1;
}

_CF(setOctave) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  curOctave=lua_tointeger(s,1);
  return 0;
}

_CF(setEditStep) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  editStep=lua_tointeger(s,1);
  return 0;
}

_CF(setEditStepCoarse) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  editStepCoarse=lua_tointeger(s,1);
  return 0;
}

_CF(setOrderEditMode) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  orderEditMode=lua_tointeger(s,1);
  return 0;
}

_CF(setOrderCursor) {
  CHECK_ARGS(1);
  CHECK_TYPE_INTEGER(1);

  orderCursor=lua_tointeger(s,1);
  return 0;
}

_CF(getSongName) {
  lua_pushstring(s,e->song.name.c_str());
  return 1;
}

_CF(setSongName) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.name=lua_tostring(s,1);
  return 0;
}

_CF(getSongAuthor) {
  lua_pushstring(s,e->song.author.c_str());
  return 1;
}

_CF(setSongAuthor) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.author=lua_tostring(s,1);
  return 0;
}

_CF(getSongAlbum) {
  lua_pushstring(s,e->song.category.c_str());
  return 1;
}

_CF(setSongAlbum) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.category=lua_tostring(s,1);
  return 0;
}

_CF(getSongSysName) {
  lua_pushstring(s,e->song.systemName.c_str());
  return 1;
}

_CF(setSongSysName) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.systemName=lua_tostring(s,1);
  return 0;
}

_CF(getSongTuning) {
  lua_pushnumber(s,e->song.tuning);
  return 1;
}

_CF(setSongTuning) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

  e->song.tuning=lua_tonumber(s,1);
  return 0;
}

_CF(getSongComments) {
  lua_pushstring(s,e->song.notes.c_str());
  return 1;
}

_CF(setSongComments) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.notes=lua_tostring(s,1);
  return 0;
}

_CF(getSubSongName) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  lua_pushstring(s,sub->name.c_str());
  return 1;
}

_CF(setSubSongName) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_STRING(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2)
    int index=lua_tointeger(s,-2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->name=lua_tostring(s,-1);
  return 0;
}

_CF(getSubSongComments) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  lua_pushstring(s,sub->notes.c_str());
  return 1;
}

_CF(setSubSongComments) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_STRING(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2)
    int index=lua_tointeger(s,-2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->notes=lua_tostring(s,-1);
  return 0;
}

_CF(getSongRate) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  lua_pushnumber(s,sub->hz);
  return 1;
}

_CF(setSongRate) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_NUMBER(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2)
    int index=lua_tointeger(s,-2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->hz=lua_tonumber(s,-1);
  return 0;
}

_CF(getSongVirtualTempo) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
    lua_pop(s,1);
  }

  lua_pushnumber(s,sub->virtualTempoN);
  lua_pushnumber(s,sub->virtualTempoD);

  return 2;
}

_CF(setSongVirtualTempo) {
  CHECK_ARGS_RANGE(2,3);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    int index=lua_tointeger(s,-3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->virtualTempoN=lua_tointeger(s,-2);
  sub->virtualTempoD=lua_tointeger(s,-1);
  return 0;
}

_CF(getSongHighlights) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
    lua_pop(s,1);
  }

  lua_pushnumber(s,sub->hilightA);
  lua_pushnumber(s,sub->hilightB);

  return 2;
}

_CF(setSongHighlights) {
  CHECK_ARGS_RANGE(2,3);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_INTEGER(-3)
    int index=lua_tointeger(s,-3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->hilightA=lua_tointeger(s,-2);
  sub->hilightB=lua_tointeger(s,-1);
  return 0;
}

_CF(getSongSpeeds) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
    lua_pop(s,1);
  }

  lua_createtable(s,sub->speeds.len,0);
  for (int i=0; i<sub->speeds.len; i++) {
    lua_pushinteger(s,sub->speeds.val[i]);
    lua_seti(s,1,i+1);
  }

  return 1;
}

_CF(setSongSpeeds) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_TABLE(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2)
    int index=lua_tointeger(s,-2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->speeds.len=1;
  memset(sub->speeds.val,6,sizeof(sub->speeds.val));

  int tablePos=lua_gettop(s);
  lua_pushnil(s);
  while (lua_next(s,tablePos)) {
    if (!lua_isinteger(s,-2)) {
      // ignore other keys
      lua_pop(s,1);
      continue;
    }
    CHECK_TYPE_INTEGER(-1);
    int index=lua_tointeger(s,-2)-1;
    int speed=lua_tointeger(s,-1);

    if (index<0 || index>=16) {
      // ignore invalid index
      lua_pop(s,1);
      continue;
    }
    sub->speeds.val[index]=speed;
    if (sub->speeds.len<=index) sub->speeds.len=index+1;

    lua_pop(s,1);
  }

  return 0;
}

_CF(getSongLength) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  lua_pushinteger(s,sub->ordersLen);
  return 1;
}

_CF(setSongLength) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2)
    int index=lua_tointeger(s,-2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->ordersLen=lua_tointeger(s,-1);
  return 0;
}

_CF(getPatLength) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  lua_pushinteger(s,sub->patLen);
  return 1;
}

_CF(setPatLength) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2)
    int index=lua_tointeger(s,-2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->patLen=lua_tointeger(s,-1);
  return 0;
}

_CF(createIns) {
  int ret=e->addInstrument();
  if (ret>=0) {
    lua_pushinteger(s,ret);
  } else {
    lua_pushnil(s);
  }
  return 1;
}

_CF(deleteIns) {
  CHECK_ARGS_RANGE(0,1);

  int index=curIns;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
    if (index<0 || index>=e->song.insLen) {
      SC_ERROR("invalid instrument index");
    }
  }

  e->delInstrument(index);

  return 0;
}

_CF(setInsData) {
  CHECK_ARGS_RANGE(2,3)

  int index=curIns;
  int tableIdx;
  int featureCode;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_INTEGER(1);
    CHECK_TYPE_INTEGER(2);
    CHECK_TYPE_TABLE(3)
    index=lua_tointeger(s,1);
    featureCode=lua_tointeger(s,2);
    tableIdx=3;
    if (index<0 || index>=e->song.insLen) {
      SC_ERROR("invalid instrument index");
    }
  } else {
    CHECK_TYPE_INTEGER(1);
    CHECK_TYPE_TABLE(2)
    featureCode=lua_tointeger(s,1);
    tableIdx=2;
  }
  DivInstrument* ins=e->getIns(index);
  insFeatures[featureCode].featureReadFunc(ins,s,tableIdx);
  return 0;
}

_CF(setInsMacroData) {
  CHECK_ARGS_RANGE(2,3)

  int index=curIns;
  int tableIdx;
  int macroType;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_INTEGER(1);
    CHECK_TYPE_INTEGER(2);
    CHECK_TYPE_TABLE(3)
    index=lua_tointeger(s,1);
    macroType=lua_tointeger(s,2);
    tableIdx=3;
    if (index<0 || index>=e->song.insLen) {
      SC_ERROR("invalid instrument index");
    }
  } else {
    CHECK_TYPE_INTEGER(1);
    CHECK_TYPE_TABLE(2)
    macroType=lua_tointeger(s,1);
    tableIdx=2;
  }
  DivInstrumentMacro* macro=e->getIns(index)->std.macroByType((DivMacroType)macroType);
  lua_pushnil(s);
  while (lua_next(s,tableIdx)) {
    if (lua_isstring(s, -2)) {
      const char* key=lua_tostring(s,-2);
      int value;
      switch (lua_type(s,-1)) {
        case LUA_TBOOLEAN:
          value=lua_toboolean(s,-1);
          writeMacro(macro,key,NULL,value);
          break;
        case LUA_TNUMBER:
          value=lua_tointeger(s,-1);
          writeMacro(macro,key,NULL,value);
          break;
        case LUA_TTABLE: { // subtable
          if (strcmp(key,"values")==0) {
            lua_pushnil(s);
            int count=0;
            while (lua_next(s,-2)) {
              if (lua_isnumber(s,-2)) {
                if (lua_isnumber(s,-1) && count<255) {
                  int idx=lua_tointeger(s,-2);
                  macro->val[idx-1]=lua_tointeger(s,-1); // TODO: limit to 255
                  count++;
                }
              }
              lua_pop(s,1);
            }
            macro->open=macro->open&(~6);
            macro->len=count;
          } else {
            lua_pushnil(s);
            while (lua_next(s,-2)) {
              if (lua_isstring(s,-2)) {
                if (lua_isinteger(s,-1)) {
                  const char* subkey=lua_tostring(s,-2);
                  int value=lua_tointeger(s,-1);
                  writeMacro(macro,key,subkey,value);
                }
              }
              lua_pop(s,1);
            }
          }
        }
        default: break;
      }
    }
    lua_pop(s,1);
  }
  return 0;
}

_CF(getInsData) {
  CHECK_ARGS_RANGE(0,1)
  int index=curIns;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1)
    index=lua_tointeger(s,1);
    if (index<0 || index>=e->song.insLen) {
      SC_ERROR("invalid instrument index");
    }
  }
  if (e->song.insLen==0) {
    lua_pushnil(s);
    return 1;
  }
  DivInstrument* ins=e->getIns(index);
  lua_newtable(s);
  for (int i=0; insFeatures[i].valueName; i++) {
    insFeatures[i].featureWriteFunc(ins,s);
  }
  return 1;
}

_CF(getInsMacroData) {
  CHECK_ARGS_RANGE(0,1)
  int index=curIns;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1)
    index=lua_tointeger(s,1);
    if (index<0 || index>=e->song.insLen) {
      SC_ERROR("invalid instrument index");
    }
  }
  if (e->song.insLen==0) {
    lua_pushnil(s);
    return 1;
  }
  DivInstrument* ins=e->getIns(index);
  lua_newtable(s);
  for (int i=DIV_MACRO_VOL; i<=DIV_MACRO_EX10; i++) {
    DivInstrumentMacro* macro=ins->std.macroByType(DivMacroType(i));
    lua_pushstring(s,macroTypeValueNames[i][1]);
    lua_newtable(s);
    API_ADD_VALUE("delay",macro->delay,integer)
    API_ADD_VALUE("speed",macro->speed,integer)
    API_ADD_VALUE("loop",macro->loop,integer)
    API_ADD_VALUE("release",macro->rel,integer)
    API_ADD_VALUE("mode",macro->mode,integer)
    API_ADD_VALUE("open",macro->open&1,boolean)
    API_ADD_VALUE("instantRelease",macro->open&(1<<3),boolean)
    int type=(macro->open>>1)&3;
    switch (type) {
      case 0: {
        lua_pushstring(s,"values");
        lua_newtable(s);
        for (int j=0; j<macro->len; j++) {
          lua_pushinteger(s,j+1);
          lua_pushinteger(s,macro->val[j]);
          lua_settable(s,-3);
        }
        lua_settable(s,-3);
        break;
      }
      case 1: {
        lua_pushstring(s,"envelope");
        lua_newtable(s);
        API_ADD_VALUE("bottom",macro->val[0],integer)
        API_ADD_VALUE("top",macro->val[1],integer)
        API_ADD_VALUE("attack",macro->val[2],integer)
        API_ADD_VALUE("hold",macro->val[3],integer)
        API_ADD_VALUE("decay",macro->val[4],integer)
        API_ADD_VALUE("sustain",macro->val[5],integer)
        API_ADD_VALUE("susTime",macro->val[6],integer)
        API_ADD_VALUE("susDecay",macro->val[7],integer)
        API_ADD_VALUE("release",macro->val[8],integer)
        lua_settable(s,-3);
        break;
      }
      case 2: {
        lua_pushstring(s,"envelope");
        lua_newtable(s);
        API_ADD_VALUE("bottom",macro->val[0],integer)
        API_ADD_VALUE("top",macro->val[1],integer)
        API_ADD_VALUE("speed",macro->val[11],integer)
        API_ADD_VALUE("waveform",macro->val[12],integer)
        API_ADD_VALUE("phase",macro->val[13],integer)
        lua_settable(s,-3);
        break;
      }
    }
    lua_settable(s,-3);
  }
  return 1;
}

_CF(createWave) {
  int ret=e->addWave();
  if (ret>=0) {
    lua_pushinteger(s,ret);
  } else {
    lua_pushnil(s);
  }
  return 1;
}

_CF(deleteWave) {
  CHECK_ARGS_RANGE(0,1);

  int index=curWave;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  e->delWave(index);

  return 0;
}

_CF(getWaveWidth) {
  CHECK_ARGS_RANGE(0,1);

  int index=curWave;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.waveLen) {
    lua_pushnil(s);
  } else {
    DivWavetable* wave=e->song.wave[index];
    lua_pushinteger(s,wave->len);
  }

  return 1;
}

_CF(setWaveWidth) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.waveLen) {
    SC_ERROR("invalid wavetable index");
  } else {
    DivWavetable* wave=e->song.wave[index];
    int val=lua_tointeger(s,-1);
    if (val<1 || val>256) {
      SC_ERROR("value out of range");
    }
    wave->len=val;
  }

  return 0;
}

_CF(getWaveHeight) {
  CHECK_ARGS_RANGE(0,1);

  int index=curWave;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.waveLen) {
    lua_pushnil(s);
  } else {
    DivWavetable* wave=e->song.wave[index];
    lua_pushinteger(s,wave->max+1);
  }

  return 1;
}

_CF(setWaveHeight) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.waveLen) {
    SC_ERROR("invalid wavetable index");
  } else {
    DivWavetable* wave=e->song.wave[index];
    int val=lua_tointeger(s,-1);
    if (val<1 || val>256) {
      SC_ERROR("value out of range");
    }
    wave->max=val-1;
  }

  return 0;
}

_CF(getWaveData) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.waveLen) {
    lua_pushnil(s);
  } else {
    DivWavetable* wave=e->song.wave[index];
    int pos=lua_tointeger(s,-1);

    if (pos<0 || pos>=wave->len) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,wave->data[pos]);
    }
  }

  return 1;
}

_CF(setWaveData) {
  CHECK_ARGS_RANGE(2,3);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);

  int index=curWave;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_INTEGER(-3);
    index=lua_tointeger(s,-3);
  }

  if (index<0 || index>=e->song.waveLen) {
    SC_ERROR("invalid wavetable index");
  } else {
    DivWavetable* wave=e->song.wave[index];
    int pos=lua_tointeger(s,-2);
    int val=lua_tointeger(s,-1);
    if (pos<0 || pos>=wave->len) {
      SC_ERROR("position out of range");
    }
    if (val<0 || val>255) {
      SC_ERROR("value out of range");
    }
    wave->data[pos]=val;
  }

  return 0;
}

_CF(createSample) {
  int ret=e->addSample();
  if (ret>=0) {
    lua_pushinteger(s,ret);
  } else {
    lua_pushnil(s);
  }
  return 1;
}

_CF(deleteSample) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  e->delSample(index);

  return 0;
}

_CF(getSampleLength) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    lua_pushinteger(s,sample->samples);
  }

  return 1;
}

_CF(setSampleLength) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    int val=lua_tointeger(s,-1);
    if (val<0 || val>16777215) {
      SC_ERROR("value out of range");
    }
    bool errored=false;
    e->lockEngine([this,sample,&errored,index,val]() {
      if (!sample->resize(val)) {
        errored=true;
      } else {
        e->renderSamples(index);
      }
    });
    if (errored) {
      SC_ERROR("sample is not editable");
    }
    updateSampleTex=true;
    sampleSelStart=-1;
    sampleSelEnd=-1;
  }

  return 0;
}

_CF(getSampleSize) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    lua_pushinteger(s,sample->getCurBufLen());
  }

  return 1;
}

_CF(getSampleType) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    lua_pushinteger(s,sample->depth);
  }

  return 1;
}

_CF(setSampleType) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    int val=lua_tointeger(s,-1);
    if (val<0 || val>=DIV_SAMPLE_DEPTH_MAX) {
      SC_ERROR("value out of range");
    }
    e->lockEngine([this,sample,val,index]() {
      sample->convert((DivSampleDepth)val,e->getSampleFormatMask());
      e->renderSamples(index);
    });
    updateSampleTex=true;
  }

  return 0;
}

_CF(getSampleLoop) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
    lua_pushnil(s);
    lua_pushnil(s);
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    lua_pushboolean(s,sample->loop);
    lua_pushinteger(s,sample->loopStart);
    lua_pushinteger(s,sample->loopEnd);
    lua_pushinteger(s,sample->loopMode);
  }

  return 4;
}

_CF(setSampleLoop) {
  CHECK_ARGS_RANGE(4,5);
  CHECK_TYPE_BOOLEAN(-4);
  CHECK_TYPE_INTEGER(-3);
  CHECK_TYPE_INTEGER(-2);
  CHECK_TYPE_INTEGER(-1);

  int index=curSample;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_INTEGER(-5);
    index=lua_tointeger(s,-5);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    sample->loop=lua_toboolean(s,-4);
    sample->loopStart=lua_tointeger(s,-3);
    sample->loopEnd=lua_tointeger(s,-2);
    sample->loopMode=(DivSampleLoopMode)lua_tointeger(s,-1);
    e->renderSamplesP(index);
    updateSampleTex=true;
  }

  return 0;
}

_CF(getSampleRate) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    lua_pushinteger(s,sample->centerRate);
  }

  return 1;
}

_CF(setSampleRate) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    sample->centerRate=lua_tointeger(s,-1);
  }

  return 0;
}

_CF(getSampleData) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_INTEGER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(-2);
    index=lua_tointeger(s,-2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    int pos=lua_tointeger(s,-1);

    if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
      if (pos<0 || pos>=(int)sample->samples) {
        SC_ERROR("position out of range");
      }
      lua_pushinteger(s,sample->data16[pos]);
    } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
      if (pos<0 || pos>=(int)sample->samples) {
        SC_ERROR("position out of range");
      }
      lua_pushinteger(s,sample->data8[pos]);
    } else {
      if (pos<0 || pos>=(int)sample->getCurBufLen()) {
        SC_ERROR("position out of range");
      }
      lua_pushinteger(s,((unsigned char*)sample->getCurBuf())[pos]);
    }
  }

  return 1;
}

_CF(setSampleData) {
  CHECK_ARGS_RANGE(2,3);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);

  int index=curSample;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_INTEGER(-3);
    index=lua_tointeger(s,-3);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    int pos=lua_tointeger(s,-2);
    int val=lua_tointeger(s,-1);

    if (sample->depth==DIV_SAMPLE_DEPTH_16BIT) {
      if (pos<0 || pos>=(int)sample->samples) {
        SC_ERROR("position out of range");
      }
      sample->data16[pos]=val;
    } else if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
      if (pos<0 || pos>=(int)sample->samples) {
        SC_ERROR("position out of range");
      }
      sample->data8[pos]=val;
    } else {
      if (pos<0 || pos>=(int)sample->getCurBufLen()) {
        SC_ERROR("position out of range");
      }
      ((unsigned char*)sample->getCurBuf())[pos]=val;
    }
  }

  return 0;
}

_CF(isSampleEditable) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    lua_pushboolean(s,(sample->depth==DIV_SAMPLE_DEPTH_8BIT || sample->depth==DIV_SAMPLE_DEPTH_16BIT)?1:0);
  }

  return 1;
}

_CF(renderSamples) {
  CHECK_ARGS_RANGE(0,1);

  int index=-1;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_INTEGER(1);
    index=lua_tointeger(s,1);
  }

  e->renderSamplesP(index);
  return 0;
}

_CF(getOrder) {
  CHECK_ARGS_RANGE(2,3);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_INTEGER(-3)
    int index=lua_tointeger(s,-3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,-2);
  int order=lua_tointeger(s,-1);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (order<0 || order>=DIV_MAX_PATTERNS) {
    SC_ERROR("order out of range");
  }

  lua_pushinteger(s,sub->orders.ord[chan][order]);

  return 1;
}

_CF(setOrder) {
  CHECK_ARGS_RANGE(3,4);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);
  CHECK_TYPE_INTEGER(-3);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>3) {
    CHECK_TYPE_INTEGER(-4)
    int index=lua_tointeger(s,-4);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,-3);
  int order=lua_tointeger(s,-2);
  int val=lua_tointeger(s,-1);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (order<0 || order>=DIV_MAX_PATTERNS) {
    SC_ERROR("order out of range");
  }
  if (val<0 || val>=DIV_MAX_PATTERNS) {
    SC_ERROR("value out of range");
  }

  sub->orders.ord[chan][order]=val;

  return 0;
}

_CF(getPattern) {
  CHECK_ARGS_RANGE(3,5);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);
  CHECK_TYPE_INTEGER(-3);

  int order=curOrder;
  if (lua_gettop(s)>3) {
    CHECK_TYPE_INTEGER(-4);
    order=lua_tointeger(s,-4);
    if (order<0 || order>=DIV_MAX_PATTERNS) {
      SC_ERROR("invalid order");
    }
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_INTEGER(-5)
    int index=lua_tointeger(s,-5);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,-3);
  int row=lua_tointeger(s,-2);
  int pos=lua_tointeger(s,-1);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>DIV_PAT_RAW) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(sub->orders.ord[chan][order],false);

  if (pos==DIV_PAT_RAW) {
    if (p->newData[row][DIV_PAT_NOTE]!=DIV_NOTE_RAW) {
      lua_pushnil(s);
    } else {
      unsigned int rawFreq=
         p->newData[row][DIV_PAT_RAW0]|
        (p->newData[row][DIV_PAT_RAW1]<<8)|
        (p->newData[row][DIV_PAT_RAW2]<<16)|
        (p->newData[row][DIV_PAT_RAW3]<<24);
      lua_pushinteger(s,rawFreq);
    }
  } else {
    if (p->newData[row][pos]==-1) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,p->newData[row][pos]);
    }
  }

  return 1;
}

_CF(setPattern) {
  CHECK_ARGS_RANGE(4,6);
  CHECK_TYPE_INTEGER(-4);
  CHECK_TYPE_INTEGER(-3);
  CHECK_TYPE_INTEGER(-2);

  int order=curOrder;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_INTEGER(-5);
    order=lua_tointeger(s,-5);
    if (order<0 || order>=DIV_MAX_PATTERNS) {
      SC_ERROR("invalid order");
    }
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>5) {
    CHECK_TYPE_INTEGER(-6)
    int index=lua_tointeger(s,-6);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,-4);
  int row=lua_tointeger(s,-3);
  int pos=lua_tointeger(s,-2);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>DIV_PAT_RAW) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(sub->orders.ord[chan][order],true);

  if (lua_isnil(s,-1)) {
    // clear out
    if (row==DIV_PAT_RAW) {
      p->newData[row][DIV_PAT_RAW0]=-1;
      p->newData[row][DIV_PAT_RAW1]=-1;
      p->newData[row][DIV_PAT_RAW2]=-1;
      p->newData[row][DIV_PAT_RAW3]=-1;
      p->newData[row][DIV_PAT_NOTE]=-1;
    } else {
      p->newData[row][pos]=-1;
    }
  } else if (lua_isinteger(s,-1)) {
    int val=lua_tointeger(s,-1);

    if (pos==DIV_PAT_RAW) {
      p->newData[row][DIV_PAT_NOTE]=DIV_NOTE_RAW;
      p->newData[row][DIV_PAT_RAW0]=val&0xff;
      p->newData[row][DIV_PAT_RAW1]=(val>>8)&0xff;
      p->newData[row][DIV_PAT_RAW2]=(val>>16)&0xff;
      p->newData[row][DIV_PAT_RAW3]=(val>>24)&0xff;
    } else if (pos==DIV_PAT_NOTE) {
      if (val==DIV_NOTE_OFF || val==DIV_NOTE_REL || val==DIV_MACRO_REL) {
        p->newData[row][DIV_PAT_NOTE]=val;
      } else if (val>=0 && val<=180) {
        p->newData[row][DIV_PAT_NOTE]=val;
      } else {
        SC_ERROR("value out of range (0 to 180)");
      }
    } else {
      if (val<0 || val>255) {
        SC_ERROR("value out of range (0-255)");
      }
      p->newData[row][pos]=val;
    }
  } else {
    SC_ERROR("value is not a number or nil");
  }

  return 0;
}

_CF(getPatternDirect) {
  CHECK_ARGS_RANGE(4,5);
  CHECK_TYPE_INTEGER(-1);
  CHECK_TYPE_INTEGER(-2);
  CHECK_TYPE_INTEGER(-3);
  CHECK_TYPE_INTEGER(-4);

  int pat=lua_tointeger(s,-4);
  if (pat<0 || pat>=DIV_MAX_PATTERNS) {
    SC_ERROR("invalid pattern");
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_INTEGER(-5)
    int index=lua_tointeger(s,-5);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,-3);
  int row=lua_tointeger(s,-2);
  int pos=lua_tointeger(s,-1);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>2+DIV_MAX_EFFECTS*2) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(pat,false);

  if (pos==DIV_PAT_RAW) {
    if (p->newData[row][DIV_PAT_NOTE]!=DIV_NOTE_RAW) {
      lua_pushnil(s);
    } else {
      unsigned int rawFreq=
         p->newData[row][DIV_PAT_RAW0]|
        (p->newData[row][DIV_PAT_RAW1]<<8)|
        (p->newData[row][DIV_PAT_RAW2]<<16)|
        (p->newData[row][DIV_PAT_RAW3]<<24);
      lua_pushinteger(s,rawFreq);
    }
  } else {
    if (p->newData[row][pos]==-1) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,p->newData[row][pos]);
    }
  }

  return 1;
}

_CF(setPatternDirect) {
  CHECK_ARGS_RANGE(5,6);
  CHECK_TYPE_INTEGER(-5);
  CHECK_TYPE_INTEGER(-4);
  CHECK_TYPE_INTEGER(-3);
  CHECK_TYPE_INTEGER(-2);

  int pat=lua_tointeger(s,-5);
  if (pat<0 || pat>=DIV_MAX_PATTERNS) {
    SC_ERROR("invalid pattern");
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>5) {
    CHECK_TYPE_INTEGER(-6)
    int index=lua_tointeger(s,-6);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,-4);
  int row=lua_tointeger(s,-3);
  int pos=lua_tointeger(s,-2);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>2+DIV_MAX_EFFECTS*2) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(pat,true);

  if (lua_isnil(s,-1)) {
    // clear out
    if (row==DIV_PAT_RAW) {
      p->newData[row][DIV_PAT_RAW0]=-1;
      p->newData[row][DIV_PAT_RAW1]=-1;
      p->newData[row][DIV_PAT_RAW2]=-1;
      p->newData[row][DIV_PAT_RAW3]=-1;
      p->newData[row][DIV_PAT_NOTE]=-1;
    } else {
      p->newData[row][pos]=-1;
    }
  } else if (lua_isinteger(s,-1)) {
    int val=lua_tointeger(s,-1);

    if (pos==DIV_PAT_RAW) {
      p->newData[row][DIV_PAT_NOTE]=DIV_NOTE_RAW;
      p->newData[row][DIV_PAT_RAW0]=val&0xff;
      p->newData[row][DIV_PAT_RAW1]=(val>>8)&0xff;
      p->newData[row][DIV_PAT_RAW2]=(val>>16)&0xff;
      p->newData[row][DIV_PAT_RAW3]=(val>>24)&0xff;
    } else if (pos==DIV_PAT_NOTE) {
      if (val==DIV_NOTE_OFF || val==DIV_NOTE_REL || val==DIV_MACRO_REL) {
        p->newData[row][DIV_PAT_NOTE]=val;
      } else if (val>=0 && val<=180) {
        p->newData[row][DIV_PAT_NOTE]=val;
      } else {
        SC_ERROR("value out of range (0 to 180)");
      }
    }
  } else {
    SC_ERROR("value is not a number or nil");
  }

  return 0;
}

_CF(addPatternInputCallback) {
  CHECK_ARGS(1)
  CHECK_TYPE_FUNCTION(1)
  luaFunction funcID=luaL_ref(s,LUA_REGISTRYINDEX);
  scriptCallbacks.pattern.push_back({s,funcID});
  return 0;
}

_CF(addChip) {
  CHECK_ARGS(1)
  CHECK_TYPE_INTEGER(1)
  int id=lua_tointeger(s,1);
  if (id<0 || id>DIV_SYSTEM_MAX || chipIdNames[id]==NULL) {
    SC_ERROR("invalid chip id!");
  }
  bool success=e->addSystem((DivSystem)id);
  lua_pushboolean(s,success);
  return 1;
}

_CF(setChipConf) {
  CHECK_ARGS(2)
  CHECK_TYPE_INTEGER(1)
  CHECK_TYPE_TABLE(2)
  int which=lua_tointeger(s,1);
  if (which<0 || which>=DIV_MAX_CHIPS) {
    SC_ERROR("invalid chip index!");
  }
  if (which>=e->song.systemLen) {
    return 0;
  }
  DivConfig* conf=&e->song.systemFlags[which];
  lua_pushnil(s);
  while (lua_next(s,2)) {
    if (lua_isstring(s,-2)) {
      const char* key=lua_tostring(s,-2);
      String value;
      switch (lua_type(s,-1)) {
        case LUA_TBOOLEAN:
          value=lua_toboolean(s,-1)?"1":"0";
          break;
        case LUA_TNUMBER:
          value=fmt::sprintf("%d",lua_tointeger(s,-1));
          break;
        case LUA_TSTRING:
          value=lua_tostring(s,-1);
          break;
        default:
          lua_pop(s,1);
          continue;
      }
      conf->set(key,value);
    }
    lua_pop(s,1);
  }
  e->updateSysFlags(which,true,true); // i cant be bothered to read the keys to know when to mustRender
  return 0;
}

_CF(getChipCount) {
  lua_pushinteger(s,e->song.systemLen);
  return 1;
}

_CF(getChipConf) {
  CHECK_ARGS(1)
  CHECK_TYPE_INTEGER(1)
  int which=lua_tointeger(s,1);
  if (which<0 || which>=DIV_MAX_CHIPS) {
    SC_ERROR("invalid chip index!");
  }
  if (which>=e->song.systemLen) {
    lua_pushnil(s);
    return 1;
  }
  DivConfig* conf=&e->song.systemFlags[which];
  lua_newtable(s);
  for (auto& i:conf->configMap()) {
    API_ADD_VALUE(i.first.c_str(),i.second.c_str(),string);
  }
  return 1;
}

_CF(removeChip) {
  CHECK_ARGS_RANGE(1,2)
  int which;
  bool preserveOrder=true;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_INTEGER(1)
    CHECK_TYPE_BOOLEAN(2)
    which=lua_tointeger(s,1);
    preserveOrder=lua_toboolean(s,2);
  } else {
    CHECK_TYPE_INTEGER(1)
    which=lua_tointeger(s,1);
  }
  bool success=e->removeSystem(which,preserveOrder);
  lua_pushboolean(s,success);
  return 1;
}

_CF(dialogNew) {
  CHECK_ARGS(1)
  CHECK_TYPE_STRING(1)
  const char* title=lua_tostring(s,1);
  scriptDialog.title=title;
  scriptDialog.items.clear();
  return 0;
}

_CF(dialogItemInt) {
  CHECK_ARGS_RANGE(1,4)
  CHECK_TYPE_STRING(-1)
  ScriptDialog::Item item;
  item.type=ScriptDialog::Item::Type::Int;
  item.valueInt.i=0;
  item.min.i=0;
  item.max.i=100;
  if (lua_gettop(s)>3) {
    CHECK_TYPE_STRING(-4)
    CHECK_TYPE_INTEGER(-3)
    CHECK_TYPE_INTEGER(-2)
    CHECK_TYPE_INTEGER(-1)
    item.max.i=lua_tointeger(s,-1);
    item.min.i=lua_tointeger(s,-2);
    item.valueInt.i=lua_tointeger(s,-3);
    item.label=lua_tostring(s,-4);
  } else if (lua_gettop(s)>1) {
    CHECK_TYPE_STRING(-2)
    CHECK_TYPE_INTEGER(-1)
    item.valueInt.i=lua_tointeger(s,-1);
    item.label=lua_tostring(s,-2);
  } else {
    CHECK_TYPE_STRING(-1)
    item.label=lua_tostring(s,-1);
  }
  scriptDialog.items.push_back(item);
  return 0;
}

_CF(dialogItemFloat) {
  CHECK_ARGS_RANGE(1,4)
  ScriptDialog::Item item;
  item.type=ScriptDialog::Item::Type::Float;
  item.valueInt.f=0;
  item.min.f=0;
  item.max.f=100;
  if (lua_gettop(s)>3) {
    CHECK_TYPE_STRING(-4)
    CHECK_TYPE_NUMBER(-3)
    CHECK_TYPE_NUMBER(-2)
    CHECK_TYPE_NUMBER(-1)
    item.max.f=lua_tonumber(s,-1);
    item.min.f=lua_tonumber(s,-2);
    item.valueInt.f=lua_tonumber(s,-3);
    item.label=lua_tostring(s,-4);
  } else if (lua_gettop(s)>1) {
    CHECK_TYPE_STRING(-2)
    CHECK_TYPE_NUMBER(-1)
    item.valueInt.f=lua_tonumber(s,-1);
    item.label=lua_tostring(s,-2);
  } else {
    CHECK_TYPE_STRING(-1)
    item.label=lua_tostring(s,-1);
  }
  scriptDialog.items.push_back(item);
  return 0;
}

_CF(dialogItemString) {
  CHECK_ARGS_RANGE(1,2)
  ScriptDialog::Item item;
  item.type=ScriptDialog::Item::Type::String;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_STRING(-2)
    CHECK_TYPE_STRING(-1)
    const char* defaultValue=lua_tostring(s,-1);
    item.valueS=defaultValue;
    item.label=lua_tostring(s,-2);
  } else {
    CHECK_TYPE_STRING(-1)
    item.label=lua_tostring(s,-1);
  }
  scriptDialog.items.push_back(item);
  return 0;
}

_CF(dialogItemCheckbox) {
  CHECK_ARGS_RANGE(1,2)
  ScriptDialog::Item item;
  item.type=ScriptDialog::Item::Type::Checkbox;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_STRING(-2)
    CHECK_TYPE_BOOLEAN(-1)
    bool defaultValue=lua_toboolean(s,-1);
    item.valueInt.b=defaultValue;
    item.label=lua_tostring(s,-2);
  } else {
    CHECK_TYPE_STRING(-1)
    item.label=lua_tostring(s,-1);
  }
  scriptDialog.items.push_back(item);
  return 0;
}

_CF(dialogShow) {
  CHECK_ARGS(1)
  CHECK_TYPE_FUNCTION(1)
  luaFunction funcID=luaL_ref(s,LUA_REGISTRYINDEX);
  scriptDialog.callbackFunction=funcID;
  scriptDialog.state=s;
  scriptDialog.dialogOpen=true;
  return 0;
}

_CF(dialogGetItems) {
  for (ScriptDialog::Item& i:scriptDialog.items) {
    switch (i.type) {
      case ScriptDialog::Item::Type::Checkbox:
        lua_pushboolean(s,i.valueInt.b);
        break;
      case ScriptDialog::Item::Type::Int:
        lua_pushinteger(s,i.valueInt.i);
        break;
      case ScriptDialog::Item::Type::Float:
        lua_pushnumber(s,i.valueInt.f);
        break;
      case ScriptDialog::Item::Type::String:
        lua_pushstring(s,i.valueS.c_str());
        break;
    }
  }
  return scriptDialog.items.size();
}

_CF(inspect) {
  CHECK_ARGS(1);
  String repr=inspectTopValue(s,false);
  lua_pushstring(s,repr.c_str());
  return 1;
}

// LOGGING
// these functions dont need FurnaceGUI so theyre not members of it

static int _logV(lua_State *s) {
  CHECK_ARGS(1)
  CHECK_TYPE_STRING(1)
  logV(lua_tostring(s,1));
  return 0;
}

static int _logD(lua_State *s) {
  CHECK_ARGS(1)
  CHECK_TYPE_STRING(1)
  logD(lua_tostring(s,1));
  return 0;
}

static int _logI(lua_State *s) {
  CHECK_ARGS(1)
  CHECK_TYPE_STRING(1)
  logI(lua_tostring(s,1));
  return 0;
}

static int _logW(lua_State *s) {
  CHECK_ARGS(1)
  CHECK_TYPE_STRING(1)
  logW(lua_tostring(s,1));
  return 0;
}

static int _logE(lua_State *s) {
  CHECK_ARGS(1)
  CHECK_TYPE_STRING(1)
  logE(lua_tostring(s,1));
  return 0;
}

// effect/value column helper functions
// also not members of FurnaceGUI

static int _patternColumnEffect(lua_State* s) {
  CHECK_ARGS(1)
  CHECK_TYPE_INTEGER(1);
  int col=lua_tointeger(s,1);
  if (col<0 || col>=DIV_MAX_EFFECTS) {
    SC_ERROR("invalid column number!");
  }
  lua_pushinteger(s,DIV_PAT_FX(col));
  return 1;
}

static int _patternColumnEffectValue(lua_State* s) {
  CHECK_ARGS(1)
  CHECK_TYPE_INTEGER(1);
  int col=lua_tointeger(s,1);
  if (col<0 || col>=DIV_MAX_EFFECTS) {
    SC_ERROR("invalid column number!");
  }
  lua_pushinteger(s,DIV_PAT_FXVAL(col));
  return 1;
}

void FurnaceGUI::bindScriptFunctions(lua_State* s) {
  // make "fur" table
  lua_newtable(s);
  lua_setglobal(s,"fur");

  API_GLOBAL_FUNC(version);
  API_GLOBAL_FUNC(versionStr);
  API_GLOBAL_FUNC(showError);
  API_GLOBAL_FUNC(inspect);
  API_GLOBAL_FUNC(logV);
  API_GLOBAL_FUNC(logD);
  API_GLOBAL_FUNC(logI);
  API_GLOBAL_FUNC(logW);
  API_GLOBAL_FUNC(logE);
  API_USE_CATG("cursor");
    API_ADD_FUNC("getPos",getCursor);
    API_ADD_FUNC("setPos",setCursor);
    API_ADD_FUNC("getSelStart",getSelStart);
    API_ADD_FUNC("setSelStart",setSelStart);
    API_ADD_FUNC("getSelEnd",getSelEnd);
    API_ADD_FUNC("setSelEnd",setSelEnd);
  API_CATG_END;
  API_USE_CATG("engine");
    API_ADD_FUNC("getCurOrder",getCurOrder);
    API_ADD_FUNC("getCurRow",getCurRow);
    API_ADD_FUNC("getPlayTimeSec",getPlayTimeSec);
    API_ADD_FUNC("getPlayTimeMicro",getPlayTimeMicro);
    API_ADD_FUNC("getPlayTimeTicks",getPlayTimeTicks);
    API_ADD_FUNC("isPlaying",isPlaying);
    API_ADD_FUNC("isRunning",isRunning);
    API_ADD_FUNC("isFreelance",isFreelance);
    API_ADD_FUNC("getChanCount",getChanCount);
    API_ADD_FUNC("getCurSubSong",getCurSubSong);
  API_CATG_END;
  API_USE_CATG("interface");
    API_ADD_FUNC("getEditOrder",getEditOrder);
    API_ADD_FUNC("getCurIns",getCurIns);
    API_ADD_FUNC("getCurWave",getCurWave);
    API_ADD_FUNC("getCurSample",getCurSample);
    API_ADD_FUNC("setCurIns",setCurIns);
    API_ADD_FUNC("setCurWave",setCurWave);
    API_ADD_FUNC("setCurSample",setCurSample);
    API_ADD_FUNC("getOctave",getOctave);
    API_ADD_FUNC("getEditStep",getEditStep);
    API_ADD_FUNC("getEditStepCoarse",getEditStepCoarse);
    API_ADD_FUNC("getOrderEditMode",getOrderEditMode);
    API_ADD_FUNC("getOrderCursor",getOrderCursor);
    API_ADD_FUNC("setOctave",setOctave);
    API_ADD_FUNC("setEditStep",setEditStep);
    API_ADD_FUNC("setEditStepCoarse",setEditStepCoarse);
    API_ADD_FUNC("setOrderEditMode",setOrderEditMode);
    API_ADD_FUNC("setOrderCursor",setOrderCursor);
    API_ADD_FUNC("registerMenuEntry",registerMenuEntry);
  API_CATG_END;
  API_USE_CATG("song");
    API_ADD_FUNC("getName",getSongName);
    API_ADD_FUNC("setName",setSongName);
    API_ADD_FUNC("getAuthor",getSongAuthor);
    API_ADD_FUNC("setAuthor",setSongAuthor);
    API_ADD_FUNC("getAlbum",getSongAlbum);
    API_ADD_FUNC("setAlbum",setSongAlbum);
    API_ADD_FUNC("getSysName",getSongSysName);
    API_ADD_FUNC("setSysName",setSongSysName);
    API_ADD_FUNC("getTuning",getSongTuning);
    API_ADD_FUNC("setTuning",setSongTuning);
    API_ADD_FUNC("getComments",getSongComments);
    API_ADD_FUNC("setComments",setSongComments);
  API_CATG_END;
  API_USE_CATG("subsong");
    API_ADD_FUNC("getName",getSubSongName);
    API_ADD_FUNC("setName",setSubSongName);
    API_ADD_FUNC("getComments",getSubSongComments);
    API_ADD_FUNC("setComments",setSubSongComments);
    API_ADD_FUNC("getRate",getSongRate);
    API_ADD_FUNC("setRate",setSongRate);
    API_ADD_FUNC("getVirtualTempo",getSongVirtualTempo);
    API_ADD_FUNC("setVirtualTempo",setSongVirtualTempo);
    API_ADD_FUNC("getHighlights",getSongHighlights);
    API_ADD_FUNC("setHighlights",setSongHighlights);
    API_ADD_FUNC("getSpeeds",getSongSpeeds);
    API_ADD_FUNC("setSpeeds",setSongSpeeds);
    API_ADD_FUNC("getLength",getSongLength);
    API_ADD_FUNC("setLength",setSongLength);
    API_ADD_FUNC("getPatLength",getPatLength);
    API_ADD_FUNC("setPatLength",setPatLength);
  API_CATG_END;
  API_USE_CATG("instrument");
    API_ADD_FUNC("create",createIns);
    API_ADD_FUNC("delete",deleteIns);
    API_ADD_FUNC("setData",setInsData);
    API_ADD_FUNC("getData",getInsData)
    API_ADD_FUNC("setMacroData",setInsMacroData);
    API_ADD_FUNC("getMacroData",getInsMacroData)
    // instrument types
    for (int i=0; insFeatures[i].valueName; i++) {
      API_ADD_VALUE(insFeatures[i].valueName,i,integer);
    }
    // macro types
    for (int i=0; macroTypeValueNames[i][0]; i++) {
      API_ADD_VALUE(macroTypeValueNames[i][0],i,integer);
    }
  API_CATG_END;
  API_USE_CATG("wave");
    API_ADD_FUNC("create",createWave);
    API_ADD_FUNC("delete",deleteWave);
    API_ADD_FUNC("getWidth",getWaveWidth);
    API_ADD_FUNC("setWidth",setWaveWidth);
    API_ADD_FUNC("getHeight",getWaveHeight);
    API_ADD_FUNC("setHeight",setWaveHeight);
    API_ADD_FUNC("getData",getWaveData);
    API_ADD_FUNC("setData",setWaveData);
  API_CATG_END;
  API_USE_CATG("sample");
    API_ADD_FUNC("create",createSample);
    API_ADD_FUNC("delete",deleteSample);
    API_ADD_FUNC("getLength",getSampleLength);
    API_ADD_FUNC("setLength",setSampleLength);
    API_ADD_FUNC("getSize",getSampleSize);
    API_ADD_FUNC("getType",getSampleType);
    API_ADD_FUNC("setType",setSampleType);
    API_ADD_FUNC("getLoop",getSampleLoop);
    API_ADD_FUNC("setLoop",setSampleLoop);
    API_ADD_FUNC("getRate",getSampleRate);
    API_ADD_FUNC("setRate",setSampleRate);
    API_ADD_FUNC("getData",getSampleData);
    API_ADD_FUNC("setData",setSampleData);
    API_ADD_FUNC("isEditable",isSampleEditable);
    API_ADD_FUNC("render",renderSamples);
  API_CATG_END;
  API_USE_CATG("order");
    API_ADD_FUNC("get",getOrder);
    API_ADD_FUNC("set",setOrder);
  API_CATG_END;
  API_USE_CATG("pattern");
    API_ADD_FUNC("get",getPattern);
    API_ADD_FUNC("set",setPattern);
    API_ADD_FUNC("getDirect",getPatternDirect);
    API_ADD_FUNC("setDirect",setPatternDirect);
    API_ADD_FUNC("addInputCallback",addPatternInputCallback);
    // helpers
    API_ADD_VALUE("columnNote",DIV_PAT_NOTE,integer);
    API_ADD_VALUE("columnIns",DIV_PAT_INS,integer);
    API_ADD_VALUE("columnVol",DIV_PAT_VOL,integer);
    API_ADD_FUNC("columnEffect",patternColumnEffect);
    API_ADD_FUNC("columnEffectVal",patternColumnEffectValue);
    API_ADD_VALUE("columnNoteRaw",DIV_PAT_RAW,integer);
    // note values
    API_ADD_VALUE("noteOff",DIV_NOTE_OFF,integer);
    API_ADD_VALUE("noteRel",DIV_NOTE_REL,integer);
    API_ADD_VALUE("macroRel",DIV_MACRO_REL,integer);
    API_ADD_VALUE("noteRaw",DIV_NOTE_RAW,integer);
  API_CATG_END;
  API_USE_CATG("system");
    API_ADD_FUNC("add",addChip);
    API_ADD_FUNC("remove",removeChip);
    API_ADD_FUNC("getFlags",getChipConf);
    API_ADD_FUNC("setFlags",setChipConf);
    API_ADD_FUNC("getCount",getChipCount);
    // chip ids
    for (int i=0; i<DIV_SYSTEM_MAX; i++) {
      if (chipIdNames[i]) {
        API_ADD_VALUE(chipIdNames[i],i,integer);
      }
    }
  API_CATG_END;
  API_USE_CATG("dialog");
    API_ADD_FUNC("new",dialogNew);
    API_ADD_FUNC("itemInt",dialogItemInt);
    API_ADD_FUNC("itemFloat",dialogItemFloat);
    API_ADD_FUNC("itemString",dialogItemString);
    API_ADD_FUNC("itemCheckbox",dialogItemCheckbox);
    API_ADD_FUNC("show",dialogShow);
    API_ADD_FUNC("getItems",dialogGetItems);
  API_CATG_END;

  // API_USE_CATG("talvez");
  // API_ADD_VALUE("a",200,integer);
  // API_ADD_VALUE("b",400,integer);
  // API_ADD_VALUE("c",150,integer);
  // API_ADD_FUNC("logE",logE);
  lua_pop(s,1);
}

#define LOAD_LIB(_s,_n,_f) luaL_requiref(_s,_n,_f,1); lua_pop(_s,1);

void FurnaceGUI::initScriptEngine(bool initGlobal) {
  externGUI=this;

  if (playground.state) {
    lua_close(playground.state);
    playground.state=NULL;
  }
  playground.state=luaL_newstate();
  if (playground.state==NULL) {
    logE("could not create script playground state!");
  } else {
    LOAD_LIB(playground.state,LUA_GNAME,luaopen_base)
    if (settings.scriptingAllowPackage) {LOAD_LIB(playground.state,LUA_LOADLIBNAME,luaopen_package)}
    LOAD_LIB(playground.state,LUA_COLIBNAME,luaopen_coroutine)
    LOAD_LIB(playground.state,LUA_TABLIBNAME,luaopen_table)
    if (settings.scriptingAllowIO) {LOAD_LIB(playground.state,LUA_IOLIBNAME,luaopen_io)}
    if (settings.scriptingAllowOS) {LOAD_LIB(playground.state,LUA_OSLIBNAME,luaopen_os)}
    LOAD_LIB(playground.state,LUA_STRLIBNAME,luaopen_string)
    LOAD_LIB(playground.state,LUA_MATHLIBNAME,luaopen_math)
    LOAD_LIB(playground.state,LUA_UTF8LIBNAME,luaopen_utf8)
    LOAD_LIB(playground.state,LUA_DBLIBNAME,luaopen_debug)
    bindScriptFunctions(playground.state);
  }
  if (initGlobal) {
    if (globalState.state) {
      lua_close(globalState.state);
      globalState.state=NULL;
    }
    globalState.state=luaL_newstate();
    if (globalState.state==NULL) {
      logE("could not create script state!");
    } else {
      LOAD_LIB(globalState.state,LUA_GNAME,luaopen_base)
      if (settings.scriptingAllowPackage) {LOAD_LIB(globalState.state,LUA_LOADLIBNAME,luaopen_package)}
      LOAD_LIB(globalState.state,LUA_COLIBNAME,luaopen_coroutine)
      LOAD_LIB(globalState.state,LUA_TABLIBNAME,luaopen_table)
      if (settings.scriptingAllowIO) {LOAD_LIB(globalState.state,LUA_IOLIBNAME,luaopen_io)}
      if (settings.scriptingAllowOS) {LOAD_LIB(globalState.state,LUA_OSLIBNAME,luaopen_os)}
      LOAD_LIB(globalState.state,LUA_STRLIBNAME,luaopen_string)
      LOAD_LIB(globalState.state,LUA_MATHLIBNAME,luaopen_math)
      LOAD_LIB(globalState.state,LUA_UTF8LIBNAME,luaopen_utf8)
      // LOAD_LIB(globalState.state,LUA_DBLIBNAME,luaopen_debug)
      bindScriptFunctions(globalState.state);
    }
  }
}
