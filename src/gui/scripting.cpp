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

/// FUNCTIONS (C++)

int FurnaceGUI::sc_getRow(lua_State* s) {
  lua_pushinteger(s,e->getRow());
  return 1;
}

/// FUNCTIONS (C)

static int _getRow(lua_State* s) {
  return externGUI->sc_getRow(s);
}

/// INTERNAL

void FurnaceGUI::initScriptEngine() {
  externGUI=this;

  playgroundState=luaL_newstate();
  if (playgroundState==NULL) {
    logE("could not create script playground state!");
  } else {
    luaL_openlibs(playgroundState);
    lua_register(playgroundState,"getRow",_getRow);
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
