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

#include "gui.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <IconsFontAwesome4.h>
#include <fmt/printf.h>
#include "util.h"
#include "scripting/scripting.h"

String FurnaceGUI::inspectTopValue(lua_State* s, bool indentTables, int indent) {
  if (lua_gettop(s)<=0) {
    return _("<unknown value>");
  }

  String ret;
  LuaValueInspector(s,indentTables).inspect(ret);
  return ret;
}

String FurnaceGUI::inspectValues(lua_State* s, bool indentTables) {
  // saving the amount here so we don't get in an infinite loop in case of bad stack manipulation
  int n=lua_gettop(s);
  if (n<0) return _("<no value>");

  String ret="";
  lua_pushnil(s); // slot to be replaced
  for (int i=0, idx=-n-1; i<n; i++, idx++) {
    if (i>0) ret+=", ";
    lua_copy(s,idx,-1);
    ret+=inspectTopValue(s,indentTables,0);
  }
  lua_pop(s,1); // pop the slot

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
    case LUA_ERRRUN:
      ret=_("runtime error: ");
      break;
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

bool FurnaceGUI::runScriptFunction(lua_State* s, luaFunction id) {
  // logD("calling script function %d",id);
  lua_rawgeti(s,LUA_REGISTRYINDEX,id);
  int result=lua_pcall(s,0,LUA_MULTRET,0);
  if (result!=LUA_OK) {
    showError(fmt::sprintf(_("runScriptFunction error!\n%s"),getScriptError(s,result)));
    return false;
  }
  return true;
}

void FurnaceGUI::resetScriptState(lua_State* s) {
  lua_settop(s,0);
  for (auto w=scriptWindows.begin(); w!=scriptWindows.end();) {
    if (w->second.state==s) {
      luaL_unref(s,LUA_REGISTRYINDEX,w->second.function);
      w=scriptWindows.erase(w);
    } else {
      w++;
    }
  }
  for (auto menu=scriptMenus.begin(); menu!=scriptMenus.end();) {
    for (auto entry=menu->second.begin(); entry!=menu->second.end();) {
      if (entry->second.state==s) {
        luaL_unref(s,LUA_REGISTRYINDEX,entry->second.function);
        entry=scriptMenus.at(menu->first).erase(entry);
      } else {
        entry++;
      }
    }
    if (menu->second.empty()) {
      menu=scriptMenus.erase(menu);
    } else {
      menu++;
    }
  }
  scriptDialog.title.clear();
  scriptDialog.items.clear();
  for (auto p=scriptCallbacks.pattern.cbegin(); p!=scriptCallbacks.pattern.cend();) {
    if (p->second.first==s) {
      luaL_unref(s,LUA_REGISTRYINDEX,p->second.second);
      p=scriptCallbacks.pattern.erase(p++);
    } else {
      ++p;
    }
  }
}

void FurnaceGUI::runCallbacks(scriptCallbackList* which) {
  for (auto f=which->begin(); f!=which->end(); f++) {
    runScriptFunction(f->second.first,f->second.second);
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
        LoadedScript::Status runStatus=LoadedScript::Status::Idle;
        if (loadedScripts.empty()) {
          ImGui::TextUnformatted(_("no scripts loaded"));
          if (ImGui::Button(ICON_FA_PLUS "##scriptAdd")) {
            openFileDialog(GUI_FILE_LOAD_SCRIPT);
          }
        } else {
          ImVec2 tableSize=ImGui::GetContentRegionAvail();
          tableSize.y-=ImGui::GetFrameHeightWithSpacing();
          if (ImGui::BeginTable("loadedScriptTable",8,ImGuiTableFlags_Resizable|ImGuiTableFlags_NoBordersInBody|ImGuiTableFlags_RowBg,tableSize)) {
            ImGui::TableSetupColumn("cEnable",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("cName",ImGuiTableColumnFlags_WidthStretch,0.1f);
            ImGui::TableSetupColumn("cAuthor",ImGuiTableColumnFlags_WidthStretch,0.1f);
            ImGui::TableSetupColumn("cPath",ImGuiTableColumnFlags_WidthStretch,0.3f);
            ImGui::TableSetupColumn("cDesc",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("cRun",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("cEdit",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("cRemove",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupScrollFreeze(1,1);
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_REFRESH)) {
              logD("scripting: resetting global state...");
              resetScriptState(globalState.state);
              initScriptEngine();
              readLoadedScripts();
            }
            ImGui::SetItemTooltip(_("Refresh scripts"));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(_("Name"));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(_("Author"));
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(_("Path"));
            for (size_t i=0; i<loadedScripts.size(); i++) {
              ImGui::PushID(i);
              LoadedScript& s=loadedScripts[i];
              LoadedScript::Metadata* meta=&s.metadata;
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              switch (s.status) {
                case LoadedScript::Status::RunSuccess:
                  ImGui::PushStyleColor(ImGuiCol_CheckMark,ImGui::GetColorU32(uiColors[GUI_COLOR_TOGGLE_ON]));
                  if (runStatus==LoadedScript::Status::Idle) runStatus=LoadedScript::Status::RunSuccess;
                  break;
                case LoadedScript::Status::RunFail:
                  ImGui::PushStyleColor(ImGuiCol_CheckMark,ImGui::GetColorU32(uiColors[GUI_COLOR_DESTRUCTIVE]));
                  if (runStatus!=LoadedScript::Status::RunFail) runStatus=LoadedScript::Status::RunFail;
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
                // here however, we *can* use luaL_loadfile
                int ret=luaL_loadfile(globalState.state,s.path.c_str());
                if (ret==LUA_OK) {
                  ret=lua_pcall(globalState.state,0,LUA_MULTRET,0);
                }
                if (ret!=LUA_OK) {
                  globalState.lastError=getScriptError(globalState.state,ret);
                  showError("failed to run "+loadedScripts[i].path+"!\n"+globalState.lastError);
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
            ImGui::TableNextRow();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,0);
            ImGui::TableNextColumn();
            if (ImGui::Button(ICON_FA_PLUS "##scriptAdd")) {
              openFileDialog(GUI_FILE_LOAD_SCRIPT);
            }
            ImGui::EndTable();
          }
          switch (runStatus) {
            case LoadedScript::Status::RunFail:
              ImGui::TextUnformatted(_("there were errors while trying to run the scripts. check##scriptError1"));
              ImGui::SameLine();
              if (ImGui::SmallButton(_("Log Viewer"))) nextWindow=GUI_WINDOW_LOG;
              ImGui::SameLine();
              ImGui::TextUnformatted(_("for details.##scriptError2"));
              break;
            case LoadedScript::Status::RunSuccess:
              ImGui::TextUnformatted(_("scripts were run successfully"));
            default: break;
          }
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
                playgroundRet=inspectValues(playground.state,true);
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
