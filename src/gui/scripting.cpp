/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <IconsFontAwesome4.h>
#include <fmt/printf.h>
#include <lua.h>
#include "util.h"

static FurnaceGUI* externGUI;

#define DIV_PAT_RAW (DIV_PAT_FXVAL(DIV_MAX_EFFECTS)+1)

#define _CF(x) \
  static int _ ## x(lua_State* s) { \
    return externGUI->sc_ ## x(s); \
  } \
  int FurnaceGUI::sc_ ## x(lua_State* s)

#define CHECK_ARGS(x) \
  if (lua_gettop(s)!=x) { \
    lua_pushliteral(s,"invalid argument count!"); \
    lua_error(s); \
    return 0; \
  }

#define CHECK_ARGS_RANGE(x,y) \
  if (lua_gettop(s)<x || lua_gettop(s)>y) { \
    lua_pushliteral(s,"invalid argument count!"); \
    lua_error(s); \
    return 0; \
  }

#define SC_ERROR(x) \
  lua_pushliteral(s,x); \
  lua_error(s); \
  return 0;

#define CHECK_TYPE_BOOLEAN(x) \
  if (!lua_isboolean(s,x)) { \
    lua_pushliteral(s,"invalid argument type"); \
    lua_error(s); \
    return 0; \
  }

#define CHECK_TYPE_FUNCTION(x) \
  if (!lua_isfunction(s,x)) { \
    lua_pushliteral(s,"invalid argument type"); \
    lua_error(s); \
    return 0; \
  }

#define CHECK_TYPE_NUMBER(x) \
  if (!lua_isnumber(s,x)) { \
    lua_pushliteral(s,"invalid argument type"); \
    lua_error(s); \
    return 0; \
  }

#define CHECK_TYPE_STRING(x) \
  if (!lua_isstring(s,x)) { \
    lua_pushliteral(s,"invalid argument type"); \
    lua_error(s); \
    return 0; \
  }

#define CHECK_TYPE_TABLE(x) \
  if (!lua_istable(s,x)) { \
    lua_pushliteral(s,"invalid argument type"); \
    lua_error(s); \
    return 0; \
  }

#define REG_FUNC(x) \
  lua_getglobal(s,"fur"); \
  lua_pushcfunction(s,_ ## x); \
  lua_setfield(s,-2,#x); \
  lua_pop(s,1);

#define REG_ENUM(x,l) \
  lua_getglobal(s,"fur"); \
  lua_pushinteger(s,x); \
  lua_setfield(s,-2,#l); \
  lua_pop(s,1);

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

  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

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

  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

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

  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

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

  FurnaceGUIScriptMenu* menu=NULL;
  for (FurnaceGUIScriptMenu& i: scriptMenus) {
    if (i.name==menuName) {
      menu=&i;
      break;
    }
  }
  // TODO: check for duplicates
  if (menu==NULL) {
    scriptMenus.push_back(FurnaceGUIScriptMenu());
    menu=&scriptMenus[scriptMenus.size()-1];
    menu->name=menuName;
  }
  FurnaceGUIScriptAction action;
  action.name=menuEntry;
  action.function=funcID;
  action.state=s;
  menu->entries.push_back(action);

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
  CHECK_TYPE_NUMBER(1);

  curIns=lua_tointeger(s,1);
  wavePreviewInit=true;
  updateFMPreview=true;
  return 0;
}

_CF(setCurWave) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

  curWave=lua_tointeger(s,1);
  return 0;
}

_CF(setCurSample) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

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
  CHECK_TYPE_NUMBER(1);

  curOctave=lua_tointeger(s,1);
  return 0;
}

_CF(setEditStep) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

  editStep=lua_tointeger(s,1);
  return 0;
}

_CF(setEditStepCoarse) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

  editStepCoarse=lua_tointeger(s,1);
  return 0;
}

_CF(setOrderEditMode) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

  orderEditMode=lua_tointeger(s,1);
  return 0;
}

_CF(setOrderCursor) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);

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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
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
    CHECK_TYPE_NUMBER(-1);
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
  CHECK_TYPE_NUMBER(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
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
  CHECK_TYPE_NUMBER(-1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
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
    CHECK_TYPE_NUMBER(1);
    index=lua_tointeger(s,1);
  }

  e->delInstrument(index);

  return 0;
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
    CHECK_TYPE_NUMBER(1);
    index=lua_tointeger(s,1);
  }

  e->delWave(index);

  return 0;
}

_CF(getWaveWidth) {
  CHECK_ARGS_RANGE(0,1);

  int index=curWave;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_NUMBER(1);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
    CHECK_TYPE_NUMBER(1);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);

  int index=curWave;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_NUMBER(-3);
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
    CHECK_TYPE_NUMBER(1);
    index=lua_tointeger(s,1);
  }

  e->delSample(index);

  return 0;
}

_CF(getSampleLength) {
  CHECK_ARGS_RANGE(0,1);

  int index=curSample;
  if (lua_gettop(s)>0) {
    CHECK_TYPE_NUMBER(1);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
    CHECK_TYPE_NUMBER(1);
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
    CHECK_TYPE_NUMBER(1);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
    CHECK_TYPE_NUMBER(1);
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
  CHECK_TYPE_NUMBER(-3);
  CHECK_TYPE_NUMBER(-2);
  CHECK_TYPE_NUMBER(-1);

  int index=curSample;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_NUMBER(-5);
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
    CHECK_TYPE_NUMBER(1);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
  CHECK_TYPE_NUMBER(-1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(-2);
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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);

  int index=curSample;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_NUMBER(-3);
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
    CHECK_TYPE_NUMBER(1);
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
    CHECK_TYPE_NUMBER(1);
    index=lua_tointeger(s,1);
  }

  e->renderSamplesP(index);
  return 0;
}

_CF(getOrder) {
  CHECK_ARGS_RANGE(2,3);
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);
  CHECK_TYPE_NUMBER(-3);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>3) {
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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);
  CHECK_TYPE_NUMBER(-3);

  int order=curOrder;
  if (lua_gettop(s)>3) {
    CHECK_TYPE_NUMBER(-4);
    order=lua_tointeger(s,-4);
    if (order<0 || order>=DIV_MAX_PATTERNS) {
      SC_ERROR("invalid order");
    }
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>4) {
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

_CF(setPattern) { // TODO: raw freq
  CHECK_ARGS_RANGE(4,6);
  CHECK_TYPE_NUMBER(-4);
  CHECK_TYPE_NUMBER(-3);
  CHECK_TYPE_NUMBER(-2);

  int order=curOrder;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_NUMBER(-5);
    order=lua_tointeger(s,-5);
    if (order<0 || order>=DIV_MAX_PATTERNS) {
      SC_ERROR("invalid order");
    }
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>5) {
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
  CHECK_TYPE_NUMBER(-1);
  CHECK_TYPE_NUMBER(-2);
  CHECK_TYPE_NUMBER(-3);
  CHECK_TYPE_NUMBER(-4);

  int pat=lua_tointeger(s,-4);
  if (pat<0 || pat>=DIV_MAX_PATTERNS) {
    SC_ERROR("invalid pattern");
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>4) {
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
  CHECK_TYPE_NUMBER(-5);
  CHECK_TYPE_NUMBER(-4);
  CHECK_TYPE_NUMBER(-3);
  CHECK_TYPE_NUMBER(-2);

  int pat=lua_tointeger(s,-5);
  if (pat<0 || pat>=DIV_MAX_PATTERNS) {
    SC_ERROR("invalid pattern");
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>5) {
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
    CHECK_TYPE_NUMBER(-3)
    CHECK_TYPE_NUMBER(-2)
    CHECK_TYPE_NUMBER(-1)
    item.max.i=lua_tointeger(s,-1);
    item.min.i=lua_tointeger(s,-2);
    item.valueInt.i=lua_tointeger(s,-3);
    item.label=lua_tostring(s,-4);
  } else if (lua_gettop(s)>1) {
    CHECK_TYPE_STRING(-2)
    CHECK_TYPE_NUMBER(-1)
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

/// INTERNAL

String FurnaceGUI::inspectTopValue(lua_State* s) {
  String ret="";
  int stackTop=lua_gettop(s);
  if (stackTop<=0) return _("unknown value");
  switch (lua_type(s,stackTop)) {
    case LUA_TNIL:
      ret+="nil";
      break;
    case LUA_TNUMBER: {
      lua_pushvalue(s,lua_gettop(s)); // duplicate because lua_tostring modifies the value on the stack
      const char* tostr=lua_tostring(s,lua_gettop(s));
      if (tostr==NULL) {
        ret+="what?";
      } else {
        ret+=tostr;
        lua_pop(s,1);
      }
      break;
    }
    case LUA_TBOOLEAN: {
      int v=lua_toboolean(s,stackTop);
      ret+=(v?"true":"false");
      break;
    }
    case LUA_TSTRING: {
      const char* tostr=lua_tostring(s,stackTop);
      if (tostr==NULL) {
        ret+="what?";
        break;
      }
      ret+="\"";
      for (const char* sp=tostr; *sp; sp++) {
        char c=*sp;
        switch (c) {
          case '\a': ret+="\\a"; break;
          case '\b': ret+="\\b"; break;
          case '\f': ret+="\\f"; break;
          case '\n': ret+="\\n"; break;
          case '\r': ret+="\\r"; break;
          case '\t': ret+="\\t"; break;
          case '\v': ret+="\\v"; break;
          case '\\': ret+="\\\\"; break;
          case '\"': ret+="\\\""; break;
          case '\'': ret+="'"; break;
          default:
            if (isprint(c)) ret+=c;
            else ret+=fmt::sprintf("\\%03d",(unsigned char)c);
            break;
        }
      }
      ret+="\"";
      break;
    }
    case LUA_TTABLE: {
      int tablePos=lua_absindex(s,stackTop);
      bool first=true;
      ret+="{";
      lua_pushnil(s);

      int lastArrayKey=0;
      for (int i=1;; i++) {
        lua_pushinteger(s,i);
        lua_gettable(s,tablePos);
        if (lua_isnil(s,lua_gettop(s))) {
          lua_pop(s,1);
          break;
        }
        lastArrayKey=i;

        if (first) {
          first=false;
        } else {
          ret+=",";
        }
        ret+=inspectTopValue(s);
        lua_pop(s,1);
      }

      while (lua_next(s,tablePos)) {
        // skip "array" keys (already handled)
        if (lua_isinteger(s,-2)) {
          int n=lua_tointeger(s,-2);
          if (n>0 && n<=lastArrayKey) {
            lua_pop(s,1);
            continue;
          }
        }

        if (first) {
          first=false;
        } else {
          ret+=",";
        }

        String valueRepr=inspectTopValue(s);
        lua_pop(s,1);

        const auto isValidIdentifier=[](const char* s) {
          if (*s=='\0') return false;
          if (!isalpha(*s) && *s!='_') return false;
          while (*s) {
            if (!isalnum(*s) && *s!='_') return false;
            s++;
          }
          return true;
        };

        // format table key field
        if (lua_type(s,lua_gettop(s))==LUA_TSTRING) {
          const char *tostr=lua_tostring(s,lua_gettop(s));
          if (tostr!=NULL && isValidIdentifier(tostr)) {
            ret+=tostr;
          } else {
            ret+="[";
            ret+=inspectTopValue(s);
            ret+="]";
          }
        } else {
          ret+="[";
          ret+=inspectTopValue(s);
          ret+="]";
        }
        ret+="=";
        ret+=valueRepr;
      }
      ret+="}";
      break;
    }
    case LUA_TFUNCTION:
      ret+="<function>";
      break;
    case LUA_TUSERDATA:
      ret+="<userdata>";
      break;
    case LUA_TTHREAD:
      ret+="<thread>";
      break;
    case LUA_TLIGHTUSERDATA:
      ret+="<lightuserdata>";
      break;
    default:
      ret+=_("unknown value");
  }
  return ret;
}

String FurnaceGUI::getScriptError(lua_State* s, int result) {
  String ret="";
  switch (result) {
    case LUA_OK:
      return "OK";
    case LUA_YIELD:
      return _("yeild");
    case LUA_ERRMEM:
      ret=_("memory error: ");
      break;
    case LUA_ERRSYNTAX:
      ret=_("syntax error: ");
      break;
    case LUA_ERRERR:
      ret=_("error calling error handler: ");
      break;
    case LUA_ERRFILE:
      ret=_("file error: ");
      break;
    case LUA_ERRRUN: {
      ret=_("runtime error: ");
      break;
    }
    default:
      return "what?";
  }
  const char* error=lua_tostring(s,lua_gettop(s));
  if (error==NULL) {
    ret+="NULL!";
  } else {
    luaL_traceback(s,s,error,16);
    ret+=error;
  }
  return ret;
}

int FurnaceGUI::runScript(lua_State* s, const char* script) {
  int ret=luaL_loadstring(s,script);
  if (ret==LUA_OK) {
    return lua_pcall(s,0,LUA_MULTRET,0);
  } else {
    return ret;
  }
}

void FurnaceGUI::runScriptFunction(lua_State* s, luaFunction id) {
  logD("calling script function %d",id);
  lua_rawgeti(s,LUA_REGISTRYINDEX,id);
  int result=lua_pcall(s,0,LUA_MULTRET,0);
  if (result!=LUA_OK) {
    showError(fmt::sprintf(_("runScriptFunction error!\n%s"),getScriptError(s,result)));
  }
}

void FurnaceGUI::resetScriptState(lua_State* s) {
  lua_settop(s,0);
  for (size_t i=0; i<scriptMenus.size(); i++) {
    FurnaceGUIScriptMenu& menu=scriptMenus[i];
    for (size_t j=0; j<menu.entries.size(); j++) {
      FurnaceGUIScriptAction& entry=menu.entries[j];
      if (entry.state==s) {
        luaL_unref(s,LUA_REGISTRYINDEX,entry.function);
        menu.entries.erase(menu.entries.begin()+j);
        j--;
      }
    }
    if (menu.entries.empty()) {
      scriptMenus.erase(scriptMenus.begin()+i);
      i--;
    }
  }
  scriptDialog.title.clear();
  scriptDialog.items.clear();
  for (size_t i=0; i<scriptCallbacks.pattern.size(); i++) {
    if (scriptCallbacks.pattern[i].first==s) {
      scriptCallbacks.pattern.erase(scriptCallbacks.pattern.begin()+i);
      i--;
    }
  }
}

void FurnaceGUI::bindScriptFunctions(lua_State* s) {
  lua_newtable(s);
  lua_setglobal(s,"fur");
  REG_FUNC(version);
  REG_FUNC(versionStr);
  REG_FUNC(showError);
  REG_FUNC(getCursor);
  REG_FUNC(setCursor);
  REG_FUNC(getSelStart);
  REG_FUNC(setSelStart);
  REG_FUNC(getSelEnd);
  REG_FUNC(setSelEnd);
  REG_FUNC(getCurOrder);
  REG_FUNC(getCurRow);
  REG_FUNC(getPlayTimeSec);
  REG_FUNC(getPlayTimeMicro);
  REG_FUNC(getPlayTimeTicks);
  REG_FUNC(isPlaying);
  REG_FUNC(isRunning);
  REG_FUNC(isFreelance);
  REG_FUNC(getChanCount);
  REG_FUNC(getCurSubSong);
  REG_FUNC(getEditOrder);
  REG_FUNC(registerMenuEntry);
  REG_FUNC(getCurIns);
  REG_FUNC(getCurWave);
  REG_FUNC(getCurSample);
  REG_FUNC(setCurIns);
  REG_FUNC(setCurWave);
  REG_FUNC(setCurSample);
  REG_FUNC(getOctave);
  REG_FUNC(getEditStep);
  REG_FUNC(getEditStepCoarse);
  REG_FUNC(getOrderEditMode);
  REG_FUNC(getOrderCursor);
  REG_FUNC(setOctave);
  REG_FUNC(setEditStep);
  REG_FUNC(setEditStepCoarse);
  REG_FUNC(setOrderEditMode);
  REG_FUNC(setOrderCursor);
  REG_FUNC(getSongName);
  REG_FUNC(setSongName);
  REG_FUNC(getSongAuthor);
  REG_FUNC(setSongAuthor);
  REG_FUNC(getSongAlbum);
  REG_FUNC(setSongAlbum);
  REG_FUNC(getSongSysName);
  REG_FUNC(setSongSysName);
  REG_FUNC(getSongTuning);
  REG_FUNC(setSongTuning);
  REG_FUNC(getSongComments);
  REG_FUNC(setSongComments);
  REG_FUNC(getSubSongName);
  REG_FUNC(setSubSongName);
  REG_FUNC(getSubSongComments);
  REG_FUNC(setSubSongComments);
  REG_FUNC(getSongRate);
  REG_FUNC(setSongRate);
  REG_FUNC(getSongVirtualTempo);
  REG_FUNC(setSongVirtualTempo);
  REG_FUNC(getSongHighlights);
  REG_FUNC(setSongHighlights);
  REG_FUNC(getSongSpeeds);
  REG_FUNC(setSongSpeeds);
  REG_FUNC(getSongLength);
  REG_FUNC(setSongLength);
  REG_FUNC(getPatLength);
  REG_FUNC(setPatLength);
  REG_FUNC(createIns);
  REG_FUNC(deleteIns);
  REG_FUNC(createWave);
  REG_FUNC(deleteWave);
  REG_FUNC(getWaveWidth);
  REG_FUNC(setWaveWidth);
  REG_FUNC(getWaveHeight);
  REG_FUNC(setWaveHeight);
  REG_FUNC(getWaveData);
  REG_FUNC(setWaveData);
  REG_FUNC(createSample);
  REG_FUNC(deleteSample);
  REG_FUNC(getSampleLength);
  REG_FUNC(setSampleLength);
  REG_FUNC(getSampleSize);
  REG_FUNC(getSampleType);
  REG_FUNC(setSampleType);
  REG_FUNC(getSampleLoop);
  REG_FUNC(setSampleLoop);
  REG_FUNC(getSampleRate);
  REG_FUNC(setSampleRate);
  REG_FUNC(getSampleData);
  REG_FUNC(setSampleData);
  REG_FUNC(isSampleEditable);
  REG_FUNC(renderSamples);
  REG_FUNC(getOrder);
  REG_FUNC(setOrder);
  REG_FUNC(getPattern);
  REG_FUNC(setPattern);
  REG_FUNC(getPatternDirect);
  REG_FUNC(setPatternDirect);
  REG_FUNC(addPatternInputCallback);
  REG_FUNC(dialogNew);
  REG_FUNC(dialogItemInt);
  REG_FUNC(dialogItemFloat);
  REG_FUNC(dialogItemString);
  REG_FUNC(dialogItemCheckbox);
  REG_FUNC(dialogShow);
  REG_FUNC(dialogGetItems);
  REG_FUNC(logV);
  REG_FUNC(logD);
  REG_FUNC(logI);
  REG_FUNC(logW);
  REG_FUNC(logE);
  // constants/enums
  REG_ENUM(DIV_NOTE_OFF,NOTE_OFF)
  REG_ENUM(DIV_NOTE_REL,NOTE_REL)
  REG_ENUM(DIV_MACRO_REL,MACRO_REL)
  REG_ENUM(DIV_NOTE_RAW,NOTE_RAW)

  REG_ENUM(DIV_PAT_NOTE,PAT_NOTE)
  REG_ENUM(DIV_PAT_INS,PAT_INS)
  REG_ENUM(DIV_PAT_VOL,PAT_VOL)
  REG_ENUM(DIV_PAT_RAW,PAT_RAW)
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

void FurnaceGUI::runCallbacks(scriptCallbackList* which) {
  for (auto f=which->begin(); f<which->end(); f++) {
    if (f->second!=-1) runScriptFunction(f->first,f->second);
  }
}

/// WINDOW

void FurnaceGUI::drawScripting() {
  if (nextWindow==GUI_WINDOW_SCRIPTING) {
    scriptingOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!scriptingOpen) return;
  if (ImGui::Begin("Scripts",&scriptingOpen,globalWinFlags,_("Scripts"))) {
    if (ImGui::BeginTabBar("ScriptsTab")) {
      if (ImGui::BeginTabItem("Loaded Scripts")) {
        bool hadErrors=false;
        if (loadedScripts.empty()) {
          ImGui::TextUnformatted(_("no scripts loaded"));
        } else if (ImGui::BeginTable("loadedScriptTable",8,ImGuiTableFlags_Resizable)) {
          // enable, name, auth, desc, path, run, edit, remove
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.1f);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.1f);
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.3f);
          ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupScrollFreeze(7,1);
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          if (ImGui::Button(ICON_FA_REFRESH)) {
            logD("scripting: resetting global state...");
            resetScriptState(globalState.state);
            initScriptEngine();
            readLoadedScripts();
          }
          ImGui::SetItemTooltip(_("Refresh scripts"));
          // ImGui::TextUnformatted(_("Enable"));
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(_("Name"));
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(_("Author"));
          ImGui::TableNextColumn();
          ImGui::TextUnformatted(_("Path"));
          // ImGui::TableNextColumn();
          // ImGui::TextUnformatted(_("Edit"));
          // ImGui::TableNextColumn();
          // ImGui::TextUnformatted(_("Remove"));
          for (size_t i=0; i<loadedScripts.size(); i++) {
            ImGui::PushID(i);
            LoadedScript& s=loadedScripts[i];
            LoadedScript::Metadata* meta=&s.metadata;
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            switch (s.status) {
              case LoadedScript::Status::RunSuccess:
                ImGui::PushStyleColor(ImGuiCol_CheckMark,ImGui::GetColorU32(uiColors[GUI_COLOR_TOGGLE_ON]));
                break;
              case LoadedScript::Status::RunFail:
                ImGui::PushStyleColor(ImGuiCol_CheckMark,ImGui::GetColorU32(uiColors[GUI_COLOR_DESTRUCTIVE]));
                hadErrors=true;
                break;
              default: break;
            }
            ImGui::Checkbox("##scriptEnable",&loadedScripts[i].enabled);
            if (s.status!=LoadedScript::Status::Idle) ImGui::PopStyleColor();
            ImGui::SetItemTooltip(_("Run on startup"));
            ImGui::TableNextColumn();
            if (!meta->title.empty()) ImGui::TextUnformatted(meta->title.c_str());
            ImGui::TableNextColumn();
            if (!meta->author.empty()) ImGui::TextUnformatted(meta->author.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(s.path.c_str());
            ImGui::TableNextColumn();
            if (!meta->description.empty()) {
              ImGui::TextUnformatted(ICON_FA_INFO_CIRCLE);
              ImGui::SetItemTooltip("%s",meta->description.c_str());
            }
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_PLAY "##scriptRun")) {
              String script;
              if (!readTextFile(s.path.c_str(),script)) {
                showError("failed to read script file!");
              } else {
                globalState.lastRet=runScript(globalState.state,script.c_str());
                if (globalState.lastRet!=LUA_OK) {
                  globalState.lastError=getScriptError(globalState.state,globalState.lastRet);
                  showError("failed to run loaded script!\n"+globalState.lastError);
                  loadedScripts[i].status=LoadedScript::Status::RunFail;
                } else {
                  loadedScripts[i].status=LoadedScript::Status::RunSuccess;
                }
              }
            }
            ImGui::SetItemTooltip(_("Run"));
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_PENCIL "##scriptEdit")) SDL_OpenURL(s.path.c_str());
            ImGui::SetItemTooltip(_("Edit"));
            ImGui::TableNextColumn();
            pushDestColor();
            if (ImGui::Button(ICON_FA_TIMES "##scriptRemove")) {
              loadedScripts.erase(i+loadedScripts.begin());
            }
            popDestColor();
            ImGui::SetItemTooltip(_("Remove"));
            ImGui::PopID();
          }
          ImGui::EndTable();
        }
        if (ImGui::Button(ICON_FA_PLUS "##scriptAdd")) {
          openFileDialog(GUI_FILE_LOAD_SCRIPT);
        }
        if (hadErrors) {
          ImGui::SetCursorPosY(ImGui::GetWindowHeight()-ImGui::GetFrameHeightWithSpacing());
          ImGui::TextUnformatted(_("there were errors while trying to run the scripts. check##scriptError1"));
          ImGui::SameLine();
          if (ImGui::SmallButton(_("Log Viewer"))) nextWindow=GUI_WINDOW_LOG;
          ImGui::SameLine();
          ImGui::TextUnformatted(_("for details.##scriptError2"));
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("Playground"))) {
        if (ImGui::BeginTable("playgroundTable",2,ImGuiTableFlags_Resizable)) {
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.6f);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.3f);
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (ImGui::Button(ICON_FA_PLAY "##palygroundRun")) {
            resetScriptState(playground.state);
            playgroundRet.clear();
            playground.lastRet=runScript(playground.state,playgroundData.c_str());
            playground.lastError=getScriptError(playground.state,playground.lastRet);
            if (playground.lastRet==LUA_OK) {
              int stackTop=lua_gettop(playground.state);
              if (stackTop>0) {
                playgroundRet=inspectTopValue(playground.state);
              }
            }
          }
          ImGui::SetItemTooltip(_("Reset and run"));
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_REFRESH "##playgroundReset")) {
            resetScriptState(playground.state);
            playground.lastError.clear();
            playgroundRet.clear();
          }
          ImGui::SetItemTooltip(_("Reset state"));
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER_OPEN "##playgroundLoad")) {
            openFileDialog(GUI_FILE_LOAD_SCRIPT_PLAYGROUND);
          }
          ImGui::SetItemTooltip(_("Open"));
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FLOPPY_O "##playgroundSave")) {
            openFileDialog(GUI_FILE_SAVE_SCRIPT_PLAYGROUND);
          }
          ImGui::SetItemTooltip(_("Save"));

          ImGui::PushFont(patFont);
          ImGui::InputTextMultiline("##ScriptPlayground",&playgroundData,ImGui::GetContentRegionAvail(),ImGuiInputTextFlags_AllowTabInput);
          ImGui::PopFont();
          ImGui::TableNextColumn();
          ImVec2 size=ImGui::GetContentRegionAvail();
          size.y*=4.f/5.f;
          if (ImGui::BeginChild("playgroundOut",size,ImGuiChildFlags_ResizeY)) {
            ImGui::SeparatorText(_("Output##scriptOutput"));
            ImGui::PushFont(patFont);
            ImGui::InputTextMultiline("##ScriptPlaygroundOutput",&playgroundRet,ImGui::GetContentRegionAvail(),ImGuiInputTextFlags_ReadOnly|ImGuiInputTextFlags_WordWrap);
            ImGui::PopFont();
          }
          ImGui::EndChild();
          if (ImGui::BeginChild("playgroundRet",ImGui::GetContentRegionAvail())) {
            ImGui::SeparatorText(_("Status##scriptError"));
            ImGui::PushFont(patFont);
            ImGui::InputTextMultiline("##ScriptPlaygroundRet",&playground.lastError,ImGui::GetContentRegionAvail(),ImGuiInputTextFlags_ReadOnly|ImGuiInputTextFlags_WordWrap);
            ImGui::PopFont();
          }
          ImGui::EndChild();
          ImGui::EndTable();
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SCRIPTING;
  ImGui::End();
}
