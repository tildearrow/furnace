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
#include "util.h"

/// INTERNAL

// put this on a scope to expect a specific stack height difference at its end; useful for debugging, may incur overhead
class StackDiffChecker {
  private:
    lua_State* s;
    int expected, startHeight, endHeight;

  public:
    StackDiffChecker(lua_State* s, int expected=0): s(s), expected(expected) {
      startHeight=lua_gettop(s);
    }
    ~StackDiffChecker() {
      endHeight=lua_gettop(s);
      int got=endHeight-startHeight;
      if (got!=expected) {
        logE("wrong stack difference detected! expected %d, got %d",expected,got);
        assert(false);
      }
    }
};

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

String FurnaceGUI::inspectTopValue(lua_State* s, bool indentTables, int indent) {
  int stackTop=lua_gettop(s);
  if (stackTop<=0) return _("<unknown value>");

  // figures out whether a string is a valid identifier ([a-zA-Z_][a-zA-Z0-9_]*)
  const auto isValidIdentifier=[](const char* s) {
    if (*s=='\0') return false;
    if (!isalpha(*s) && *s!='_') return false;
    while (*s) {
      if (!isalnum(*s) && *s!='_') return false;
      s++;
    }
    return true;
  };

  String ret="";
  switch (lua_type(s,stackTop)) {
    case LUA_TNIL:
      ret+="nil";
      break;
    case LUA_TNUMBER: {
      lua_pushvalue(s,lua_gettop(s)); // duplicate because lua_tostring modifies the value on the stack
      const char* str=lua_tostring(s,lua_gettop(s));
      if (str==NULL) {
        ret+="what?";
      } else {
        ret+=str;
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
      const char* str=lua_tostring(s,stackTop);
      if (str==NULL) {
        ret+="what?";
        break;
      }
      ret+="\"";
      for (const char* sp=str; *sp; sp++) {
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

      // TODO: maybe there's a simpler way to do this that doesn't require multiple passes?

      // figure out whether the table has array keys
      int lastArrayKey=0; // if 0, there are no array keys
      for (int i=1;; i++) {
        lua_pushinteger(s,i);
        lua_gettable(s,tablePos);
        if (lua_isnil(s,lua_gettop(s))) {
          lua_pop(s,1);
          break;
        }
        lua_pop(s,1);
        lastArrayKey=i;
      }

      // whether there is a key-value pair on the top of the stack, where the key is one of the array keys
      const auto isTopArrayKvPair=[s,lastArrayKey](){
        if (lua_isinteger(s,-2)) {
          int n=lua_tointeger(s,-2);
          if (n>0 && n<=lastArrayKey) {
            return true;
          }
        }
        return false;
      };

      // figure out whether the table has non-array keys
      bool hasNonArrayKeys=false;
      lua_pushnil(s);
      while (lua_next(s,tablePos)) {
        // skip array keys
        if (isTopArrayKvPair()) {
          lua_pop(s,1);
          continue;
        }

        hasNonArrayKeys=true;
        lua_pop(s,2); // remove key-value pair from stack
        break;
      }

      // at this point we know whether to indent or not
      const bool doIndent=indentTables && hasNonArrayKeys;
      const int nextIndent=doIndent?(indent + 1):indent;

      ret+="{";

      bool first=true;
      const auto maybeAddSep=[doIndent,nextIndent,&first,&ret](){
        if (first) {
          first=false;
        } else {
          ret+=",";
        }
        if (doIndent) {
          ret+="\n";
          for (int i=0; i<nextIndent; i++) ret+="  ";
        }
      };

      // first show the array entries
      for (int i=1; i<=lastArrayKey; i++) {
        lua_pushinteger(s,i);
        lua_gettable(s,tablePos);

        if (lua_isnil(s,lua_gettop(s))) {
          lua_pop(s,1);
          logE("expected array key. why is there not one??? (k=%d)",i);
          continue;
        }

        maybeAddSep();
        ret+=inspectTopValue(s,indentTables,nextIndent);
        lua_pop(s,1);
      }

      // gets the top value as a string pointer if it is a valid identifier; returns NULL otherwise.
      // does not pop.
      const auto getTopIdentStr=[isValidIdentifier,s]() -> const char* {
        if (lua_type(s,lua_gettop(s))!=LUA_TSTRING) return NULL;
        const char* str=lua_tostring(s,lua_gettop(s));
        if (str==NULL) return NULL;
        if (isValidIdentifier(str)) return str;
        return NULL;
      };

      // then show the show the rest as k-v pairs
      lua_pushnil(s);
      while (lua_next(s,tablePos)) {
        // skip "array" keys (already handled)
        if (isTopArrayKvPair()) {
          lua_pop(s,1);
          continue;
        }

        String valueRepr=inspectTopValue(s,indentTables,nextIndent);
        lua_pop(s,1);

        maybeAddSep();
        const char* identStr=getTopIdentStr();
        if (identStr!=NULL) {
          ret+=identStr;
        } else {
          ret+="[";
          ret+=inspectTopValue(s,indentTables,nextIndent);
          ret+="]";
        }
        ret+="=";
        ret+=valueRepr;
      }

      if (doIndent) {
        ret+="\n";
        for (int i=0; i<indent; i++) ret+="  ";
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
      ret+=_("<unknown value>");
      break;
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
  for (size_t i=0; i<scriptCallbacks.pattern.size(); i++) {
    if (scriptCallbacks.pattern[i].first==s) {
      scriptCallbacks.pattern.erase(scriptCallbacks.pattern.begin()+i);
      i--;
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
            // enable, name, auth, path, info, run, edit, remove
            ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.1f);
            ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.1f);
            ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.3f);
            ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c7",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("c8",ImGuiTableColumnFlags_WidthFixed);
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
