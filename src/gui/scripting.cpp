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

static FurnaceGUI* externGUI;

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
  lua_getglobal(playgroundState,"fur"); \
  lua_pushcfunction(playgroundState,_ ## x); \
  lua_setfield(playgroundState,-2,#x);

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
  lua_pushinteger(s,e->getTotalSeconds());
  return 1;
}

_CF(getPlayTimeMicro) {
  lua_pushinteger(s,e->getTotalTicks());
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
  int funcID=luaL_ref(s,LUA_REGISTRYINDEX);

  FurnaceGUIScriptMenu* menu=NULL;
  for (FurnaceGUIScriptMenu& i: scriptMenus) {
    if (i.name==menuName) {
      menu=&i;
      break;
    }
  }
  if (menu==NULL) {
    scriptMenus.push_back(FurnaceGUIScriptMenu());
    menu=&scriptMenus[scriptMenus.size()-1];
  }
  FurnaceGUIScriptAction action;
  action.name=menuEntry;
  action.function=funcID;
  action.state=s;
  menu->entries.push_back(action);

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
  CHECK_TYPE_STRING(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    int index=lua_tointeger(s,2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->name=lua_tostring(s,1);
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
  CHECK_TYPE_STRING(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    int index=lua_tointeger(s,2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->notes=lua_tostring(s,1);
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
  CHECK_TYPE_NUMBER(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    int index=lua_tointeger(s,2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->hz=lua_tonumber(s,1);
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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    int index=lua_tointeger(s,3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
    lua_pop(s,1);
  }

  sub->virtualTempoN=lua_tointeger(s,1);
  sub->virtualTempoD=lua_tointeger(s,2);
  return 0;
}

_CF(getSongDivider) {
  CHECK_ARGS_RANGE(0,1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>0) {
    int index=lua_tointeger(s,1);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  lua_pushinteger(s,sub->timeBase);
  return 1;
}

_CF(setSongDivider) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_NUMBER(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    int index=lua_tointeger(s,2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->timeBase=lua_tointeger(s,1);
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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    int index=lua_tointeger(s,3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
    lua_pop(s,1);
  }

  sub->hilightA=lua_tointeger(s,1);
  sub->hilightB=lua_tointeger(s,2);
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
  CHECK_TYPE_TABLE(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    int index=lua_tointeger(s,3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
    lua_pop(s,1);
  }

  sub->speeds.len=1;
  memset(sub->speeds.val,6,sizeof(sub->speeds.val));
  
  lua_pushnil(s);
  while (lua_next(s,1)) {
    if (!lua_isinteger(s,2)) {
      // ignore other keys
      lua_pop(s,1);
      continue;
    }
    CHECK_TYPE_NUMBER(3);
    int index=lua_tointeger(s,2)-1;
    int speed=lua_tointeger(s,3);

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
  CHECK_TYPE_NUMBER(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    int index=lua_tointeger(s,2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->ordersLen=lua_tointeger(s,1);
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
  CHECK_TYPE_NUMBER(1);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>1) {
    int index=lua_tointeger(s,2);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  sub->patLen=lua_tointeger(s,1);
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
  CHECK_TYPE_NUMBER(1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.waveLen) {
    SC_ERROR("invalid wavetable index");
  } else {
    DivWavetable* wave=e->song.wave[index];
    int val=lua_tointeger(s,1);
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
  CHECK_TYPE_NUMBER(1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.waveLen) {
    SC_ERROR("invalid wavetable index");
  } else {
    DivWavetable* wave=e->song.wave[index];
    int val=lua_tointeger(s,1);
    if (val<1 || val>256) {
      SC_ERROR("value out of range");
    }
    wave->max=val-1;
  }

  return 0;
}

_CF(getWaveData) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_NUMBER(1);

  int index=curWave;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.waveLen) {
    lua_pushnil(s);
  } else {
    DivWavetable* wave=e->song.wave[index];
    int pos=lua_tointeger(s,1);

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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);

  int index=curWave;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_NUMBER(3);
    index=lua_tointeger(s,3);
  }

  if (index<0 || index>=e->song.waveLen) {
    SC_ERROR("invalid wavetable index");
  } else {
    DivWavetable* wave=e->song.wave[index];
    int pos=lua_tointeger(s,1);
    int val=lua_tointeger(s,2);
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
  CHECK_TYPE_NUMBER(1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    int val=lua_tointeger(s,1);
    if (val<0 || val>16777215) {
      SC_ERROR("value out of range");
    }
    bool errored=false;
    e->lockEngine([this,sample,&errored,index]() {
      if (!sample->resize(resizeSize)) {
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
  CHECK_TYPE_NUMBER(1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    int val=lua_tointeger(s,1);
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
  CHECK_TYPE_BOOLEAN(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);
  CHECK_TYPE_NUMBER(4);

  int index=curSample;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_NUMBER(5);
    index=lua_tointeger(s,5);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    sample->loop=lua_toboolean(s,1);
    sample->loopStart=lua_tointeger(s,2);
    sample->loopEnd=lua_tointeger(s,3);
    sample->loopMode=(DivSampleLoopMode)lua_tointeger(s,4);
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
  CHECK_TYPE_NUMBER(1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    sample->centerRate=lua_tointeger(s,1);
  }

  return 0;
}

_CF(getSampleCompatRate) {
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
    lua_pushinteger(s,sample->rate);
  }

  return 1;
}

_CF(setSampleCompatRate) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_NUMBER(1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    sample->rate=lua_tointeger(s,1);
  }

  return 0;
}

_CF(getSampleData) {
  CHECK_ARGS_RANGE(1,2);
  CHECK_TYPE_NUMBER(1);

  int index=curSample;
  if (lua_gettop(s)>1) {
    CHECK_TYPE_NUMBER(2);
    index=lua_tointeger(s,2);
  }

  if (index<0 || index>=e->song.sampleLen) {
    lua_pushnil(s);
  } else {
    DivSample* sample=e->song.sample[index];
    int pos=lua_tointeger(s,1);

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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);

  int index=curSample;
  if (lua_gettop(s)>2) {
    CHECK_TYPE_NUMBER(3);
    index=lua_tointeger(s,3);
  }

  if (index<0 || index>=e->song.sampleLen) {
    SC_ERROR("invalid sample index");
  } else {
    DivSample* sample=e->song.sample[index];
    int pos=lua_tointeger(s,1);
    int val=lua_tointeger(s,2);

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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>2) {
    int index=lua_tointeger(s,3);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,1);
  int order=lua_tointeger(s,2);

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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>3) {
    int index=lua_tointeger(s,4);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,1);
  int order=lua_tointeger(s,2);
  int val=lua_tointeger(s,3);

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
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

  int order=curOrder;
  if (lua_gettop(s)>3) {
    CHECK_TYPE_NUMBER(4);
    order=lua_tointeger(s,4);
    if (order<0 || order>=DIV_MAX_PATTERNS) {
      SC_ERROR("invalid order");
    }
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>4) {
    int index=lua_tointeger(s,5);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,1);
  int row=lua_tointeger(s,2);
  int pos=lua_tointeger(s,3);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>=2+DIV_MAX_EFFECTS*2) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(sub->orders.ord[chan][order],false);

  if (pos==0) { // map note/octave to a single value
    if (p->data[row][0]==100) {
      lua_pushinteger(s,120);
    } else if (p->data[row][0]==101) {
      lua_pushinteger(s,121);
    } else if (p->data[row][0]==102) {
      lua_pushinteger(s,122);
    } else if (p->data[row][0]==0 && p->data[row][1]==0) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,p->data[row][0]+(((signed char)p->data[row][1])*12));
    }
  } else {
    if (p->data[row][pos+1]==-1) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,p->data[row][pos+1]);
    }
  }

  return 1;
}

_CF(setPattern) {
  CHECK_ARGS_RANGE(4,6);
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

  int order=curOrder;
  if (lua_gettop(s)>4) {
    CHECK_TYPE_NUMBER(5);
    order=lua_tointeger(s,5);
    if (order<0 || order>=DIV_MAX_PATTERNS) {
      SC_ERROR("invalid order");
    }
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>5) {
    int index=lua_tointeger(s,6);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,1);
  int row=lua_tointeger(s,2);
  int pos=lua_tointeger(s,3);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>=2+DIV_MAX_EFFECTS*2) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(sub->orders.ord[chan][order],true);

  if (lua_isnil(s,4)) {
    // clear out
    if (pos==0) {
      p->data[row][0]=0;
      p->data[row][1]=0;
    } else {
      p->data[row][pos+1]=-1;
    }
  } else if (lua_isinteger(s,4)) {
    int val=lua_tointeger(s,4);

    if (pos==0) { // map single value to a note/octave
      if (val<-60 || val>122) {
        SC_ERROR("value out of range (-60 to 122)");
      }
      val+=60;
      if (val>=180) {
        p->data[row][0]=val-80;
        p->data[row][1]=0;
      } else {
        if ((val%12)==0) {
          p->data[row][0]=12;
          p->data[row][1]=(unsigned char)((val/12)-6);
        } else {
          p->data[row][0]=(val%12);
          p->data[row][1]=(unsigned char)((val/12)-5);
        }
      }
    } else {
      if (val<0 || val>255) {
        SC_ERROR("value out of range (0-255)");
      }
      p->data[row][pos+1]=val;
    }
  } else {
    SC_ERROR("value is not a number or nil");
  }

  return 0;
}

_CF(getPatternDirect) {
  CHECK_ARGS_RANGE(4,5);
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);
  CHECK_TYPE_NUMBER(4);

  int pat=lua_tointeger(s,4);
  if (pat<0 || pat>=DIV_MAX_PATTERNS) {
    SC_ERROR("invalid pattern");
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>4) {
    int index=lua_tointeger(s,5);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,1);
  int row=lua_tointeger(s,2);
  int pos=lua_tointeger(s,3);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>=2+DIV_MAX_EFFECTS*2) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(pat,false);

  if (pos==0) { // map note/octave to a single value
    if (p->data[row][0]==100) {
      lua_pushinteger(s,120);
    } else if (p->data[row][0]==101) {
      lua_pushinteger(s,121);
    } else if (p->data[row][0]==102) {
      lua_pushinteger(s,122);
    } else if (p->data[row][0]==0 && p->data[row][1]==0) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,p->data[row][0]+(((signed char)p->data[row][1])*12));
    }
  } else {
    if (p->data[row][pos+1]==-1) {
      lua_pushnil(s);
    } else {
      lua_pushinteger(s,p->data[row][pos+1]);
    }
  }

  return 1;
}

_CF(setPatternDirect) {
  CHECK_ARGS_RANGE(5,6);
  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);
  CHECK_TYPE_NUMBER(5);

  int pat=lua_tointeger(s,5);
  if (pat<0 || pat>=DIV_MAX_PATTERNS) {
    SC_ERROR("invalid pattern");
  }

  DivSubSong* sub=e->curSubSong;
  if (lua_gettop(s)>5) {
    int index=lua_tointeger(s,6);
    if (index<0 || index>=(int)e->song.subsong.size()) {
      SC_ERROR("invalid subsong index");
    }
    sub=e->song.subsong[index];
  }

  int chan=lua_tointeger(s,1);
  int row=lua_tointeger(s,2);
  int pos=lua_tointeger(s,3);

  if (chan<0 || chan>=DIV_MAX_CHANS) {
    SC_ERROR("channel out of range");
  }
  if (row<0 || row>=DIV_MAX_ROWS) {
    SC_ERROR("row out of range");
  }
  if (pos<0 || pos>=2+DIV_MAX_EFFECTS*2) {
    SC_ERROR("position out of range");
  }

  DivPattern* p=sub->pat[chan].getPattern(pat,true);

  if (lua_isnil(s,4)) {
    // clear out
    if (pos==0) {
      p->data[row][0]=0;
      p->data[row][1]=0;
    } else {
      p->data[row][pos+1]=-1;
    }
  } else if (lua_isinteger(s,4)) {
    int val=lua_tointeger(s,4);

    if (pos==0) { // map single value to a note/octave
      if (val<-60 || val>122) {
        SC_ERROR("value out of range (-60 to 122)");
      }
      val+=60;
      if (val>=180) {
        p->data[row][0]=val-80;
        p->data[row][1]=0;
      } else {
        if ((val%12)==0) {
          p->data[row][0]=12;
          p->data[row][1]=(unsigned char)((val/12)-6);
        } else {
          p->data[row][0]=(val%12);
          p->data[row][1]=(unsigned char)((val/12)-5);
        }
      }
    } else {
      if (val<0 || val>255) {
        SC_ERROR("value out of range (0-255)");
      }
      p->data[row][pos+1]=val;
    }
  } else {
    SC_ERROR("value is not a number or nil");
  }

  return 0;
}

/// INTERNAL

void FurnaceGUI::runScriptFunction(lua_State* s, int id) {
  lua_rawgeti(s,LUA_REGISTRYINDEX,id);
  lua_pcall(s,0,0,0);
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
}

void FurnaceGUI::initScriptEngine() {
  externGUI=this;

  playgroundState=luaL_newstate();
  if (playgroundState==NULL) {
    logE("could not create script playground state!");
  } else {
    luaL_openlibs(playgroundState);
    lua_newtable(playgroundState);
    lua_setglobal(playgroundState,"fur");
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
    REG_FUNC(getSongDivider);
    REG_FUNC(setSongDivider);
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
    REG_FUNC(getSampleCompatRate);
    REG_FUNC(setSampleCompatRate);
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
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Playground")) {
        if (ImGui::Button("Run")) {
          resetScriptState(playgroundState);
          playgroundRet=luaL_loadstring(playgroundState,playgroundData.c_str());
          if (playgroundRet==LUA_OK) {
            playgroundRet=lua_pcall(playgroundState,0,LUA_MULTRET,0);
            switch (playgroundRet) {
              case LUA_OK: {
                playgroundStatus="OK";
                int stackTop=lua_gettop(playgroundState);
                if (stackTop>0) {
                  const char* val=lua_tostring(playgroundState,stackTop);
                  playgroundStatus+=" (";
                  if (val==NULL) {
                    playgroundStatus+="unknown value";
                  } else {
                    playgroundStatus+=val;
                  }
                  playgroundStatus+=")";
                }
                break;
              }
              case LUA_ERRMEM:
                playgroundStatus="memory error";
                break;
              case LUA_ERRSYNTAX:
                playgroundStatus="syntax error";
                break;
              case LUA_ERRERR:
                playgroundStatus="error calling error handler";
                break;
              case LUA_ERRFILE:
                playgroundStatus="file error";
                break;
              case LUA_ERRRUN: {
                playgroundStatus="runtime error: ";
                const char* error=lua_tostring(playgroundState,lua_gettop(playgroundState));
                if (error==NULL) {
                  playgroundStatus+="NULL!";
                } else {
                  playgroundStatus+=error;
                }
                break;
              }
              default:
                playgroundStatus="what?";
                break;
            }
          } else {
            switch (playgroundRet) {
              case LUA_ERRSYNTAX:
                playgroundStatus="compile: syntax error";
                break;
              case LUA_ERRMEM:
                playgroundStatus="compile: out of memory";
                break;
              default:
                playgroundStatus="compile: unknown error";
                break;
            }
          }
        }
        ImGui::PushFont(patFont);
        ImGui::InputTextMultiline("##ScriptPlayground",&playgroundData,ImVec2(ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()));
        ImGui::PopFont();

        ImGui::TextUnformatted(playgroundStatus.c_str());

        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SCRIPTING;
  ImGui::End();
}
