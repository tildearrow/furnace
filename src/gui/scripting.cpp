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
  }

#define CHECK_ARGS(x) \
  if (lua_gettop(s)!=x) { \
    lua_pushliteral(s,"invalid argument count!"); \
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

/// FUNCTIONS (C++)

int FurnaceGUI::sc_getCursor(lua_State* s) {
  lua_pushinteger(s,cursor.xCoarse);
  lua_pushinteger(s,cursor.xFine);
  lua_pushinteger(s,cursor.y);
  return 3;
}
_CF(getCursor)

int FurnaceGUI::sc_setCursor(lua_State* s) {
  CHECK_ARGS(3);

  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

  cursor.xCoarse=lua_tointeger(s,1);
  cursor.xFine=lua_tointeger(s,2);
  cursor.y=lua_tointeger(s,3);

  return 0;
}
_CF(setCursor)

int FurnaceGUI::sc_getSelStart(lua_State* s) {
  lua_pushinteger(s,selStart.xCoarse);
  lua_pushinteger(s,selStart.xFine);
  lua_pushinteger(s,selStart.y);
  return 3;
}
_CF(getSelStart)

int FurnaceGUI::sc_setSelStart(lua_State* s) {
  CHECK_ARGS(3);

  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

  selStart.xCoarse=lua_tointeger(s,1);
  selStart.xFine=lua_tointeger(s,2);
  selStart.y=lua_tointeger(s,3);

  return 0;
}
_CF(setSelStart)

int FurnaceGUI::sc_getSelEnd(lua_State* s) {
  lua_pushinteger(s,selEnd.xCoarse);
  lua_pushinteger(s,selEnd.xFine);
  lua_pushinteger(s,selEnd.y);
  return 3;
}
_CF(getSelEnd)

int FurnaceGUI::sc_setSelEnd(lua_State* s) {
  CHECK_ARGS(3);

  CHECK_TYPE_NUMBER(1);
  CHECK_TYPE_NUMBER(2);
  CHECK_TYPE_NUMBER(3);

  selEnd.xCoarse=lua_tointeger(s,1);
  selEnd.xFine=lua_tointeger(s,2);
  selEnd.y=lua_tointeger(s,3);

  return 0;
}
_CF(setSelEnd)

int FurnaceGUI::sc_getCurOrder(lua_State* s) {
  lua_pushinteger(s,e->getOrder());
  return 1;
}
_CF(getCurOrder)

int FurnaceGUI::sc_getCurRow(lua_State* s) {
  lua_pushinteger(s,e->getRow());
  return 1;
}
_CF(getCurRow)

int FurnaceGUI::sc_getPlayTimeSec(lua_State* s) {
  lua_pushinteger(s,e->getTotalSeconds());
  return 1;
}
_CF(getPlayTimeSec)

int FurnaceGUI::sc_getPlayTimeMicro(lua_State* s) {
  lua_pushinteger(s,e->getTotalTicks());
  return 1;
}
_CF(getPlayTimeMicro)

int FurnaceGUI::sc_getPlayTimeTicks(lua_State* s) {
  lua_pushinteger(s,e->getTotalTicksR());
  return 1;
}
_CF(getPlayTimeTicks)

int FurnaceGUI::sc_isPlaying(lua_State* s) {
  lua_pushboolean(s,e->isPlaying());
  return 1;
}
_CF(isPlaying)

int FurnaceGUI::sc_getChanCount(lua_State* s) {
  lua_pushinteger(s,e->getTotalChannelCount());
  return 1;
}
_CF(getChanCount)

int FurnaceGUI::sc_getSongName(lua_State* s) {
  lua_pushstring(s,e->song.name.c_str());
  return 1;
}
_CF(getSongName)

int FurnaceGUI::sc_setSongName(lua_State* s) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.name=lua_tostring(s,1);
  return 0;
}
_CF(setSongName)

int FurnaceGUI::sc_getSongAuthor(lua_State* s) {
  lua_pushstring(s,e->song.author.c_str());
  return 1;
}
_CF(getSongAuthor)

int FurnaceGUI::sc_setSongAuthor(lua_State* s) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.author=lua_tostring(s,1);
  return 0;
}
_CF(setSongAuthor)

int FurnaceGUI::sc_getSongAlbum(lua_State* s) {
  lua_pushstring(s,e->song.category.c_str());
  return 1;
}
_CF(getSongAlbum)

int FurnaceGUI::sc_setSongAlbum(lua_State* s) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.category=lua_tostring(s,1);
  return 0;
}
_CF(setSongAlbum)

int FurnaceGUI::sc_getSongSysName(lua_State* s) {
  lua_pushstring(s,e->song.systemName.c_str());
  return 1;
}
_CF(getSongSysName)

int FurnaceGUI::sc_setSongSysName(lua_State* s) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.systemName=lua_tostring(s,1);
  return 0;
}
_CF(setSongSysName)

int FurnaceGUI::sc_getSongTuning(lua_State* s) {
  lua_pushnumber(s,e->song.tuning);
  return 1;
}
_CF(getSongTuning)

int FurnaceGUI::sc_setSongTuning(lua_State* s) {
  CHECK_ARGS(1);
  CHECK_TYPE_NUMBER(1);

  e->song.tuning=lua_tonumber(s,1);
  return 0;
}
_CF(setSongTuning)

int FurnaceGUI::sc_getSongComments(lua_State* s) {
  lua_pushstring(s,e->song.notes.c_str());
  return 1;
}
_CF(getSongComments)

int FurnaceGUI::sc_setSongComments(lua_State* s) {
  CHECK_ARGS(1);
  CHECK_TYPE_STRING(1);

  e->song.notes=lua_tostring(s,1);
  return 0;
}
_CF(setSongComments)

/// INTERNAL

void FurnaceGUI::initScriptEngine() {
  externGUI=this;

  playgroundState=luaL_newstate();
  if (playgroundState==NULL) {
    logE("could not create script playground state!");
  } else {
    luaL_openlibs(playgroundState);
    lua_register(playgroundState,"getCursor",_getCursor);
    lua_register(playgroundState,"setCursor",_setCursor);
    lua_register(playgroundState,"getSelStart",_getSelStart);
    lua_register(playgroundState,"setSelStart",_setSelStart);
    lua_register(playgroundState,"getSelEnd",_getSelEnd);
    lua_register(playgroundState,"setSelEnd",_setSelEnd);
    lua_register(playgroundState,"getCurOrder",_getCurOrder);
    lua_register(playgroundState,"getCurRow",_getCurRow);
    lua_register(playgroundState,"getPlayTimeSec",_getPlayTimeSec);
    lua_register(playgroundState,"getPlayTimeMicro",_getPlayTimeMicro);
    lua_register(playgroundState,"getPlayTimeTicks",_getPlayTimeTicks);
    lua_register(playgroundState,"isPlaying",_isPlaying);
    lua_register(playgroundState,"getChanCount",_getChanCount);
    lua_register(playgroundState,"getSongName",_getSongName);
    lua_register(playgroundState,"setSongName",_setSongName);
    lua_register(playgroundState,"getSongAuthor",_getSongAuthor);
    lua_register(playgroundState,"setSongAuthor",_setSongAuthor);
    lua_register(playgroundState,"getSongAlbum",_getSongAlbum);
    lua_register(playgroundState,"setSongAlbum",_setSongAlbum);
    lua_register(playgroundState,"getSongSysName",_getSongSysName);
    lua_register(playgroundState,"setSongSysName",_setSongSysName);
    lua_register(playgroundState,"getSongTuning",_getSongTuning);
    lua_register(playgroundState,"setSongTuning",_setSongTuning);
    lua_register(playgroundState,"getSongComments",_getSongComments);
    lua_register(playgroundState,"setSongComments",_setSongComments);
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
          lua_settop(playgroundState,0);
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
