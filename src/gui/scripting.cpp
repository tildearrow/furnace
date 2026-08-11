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

/**
 * Code for the "inspection" procedure.
 *
 * Fits better in a class since it has to pass around a lot of state to its different parts.
 */
class LuaValueInspector {
  lua_State* s;
  bool indentTables;
  int visitedIdx; // index for the "visited tables" table

public:
  LuaValueInspector(lua_State* s, bool indentTables):
    s(s), indentTables(indentTables), visitedIdx(-1) {}

  void inspect(String& dest) {
    // set up the "visited tables" table
    lua_newtable(s);
    visitedIdx=lua_absindex(s,-1);

    // push a copy of the value to inspect so we can interact with it
    lua_pushnil(s);
    lua_copy(s,-3,-1);

    // do the inspecting on the top value
    inspectAny(dest,0);

    // pop the two things we pushed
    lua_pop(s,2);
  }

private:
  void inspectAny(String& dest, int indent) {
    switch (lua_type(s,-1)) {
      case LUA_TNIL:
        dest+="nil";
        break;
      case LUA_TNUMBER: {
        // duplicate because lua_tostring modifies the value on the stack
        lua_pushvalue(s,-1);
        const char* str=lua_tostring(s,lua_gettop(s));
        if (str==NULL) {
          dest+="what?";
        } else {
          dest+=str;
          lua_pop(s,1);
        }
        break;
      }
      case LUA_TBOOLEAN: {
        int v=lua_toboolean(s,-1);
        dest+=(v?"true":"false");
        break;
      }
      case LUA_TSTRING:
        inspectString(dest,indent);
        break;
      case LUA_TTABLE:
        inspectTable(dest,indent);
        break;
      case LUA_TFUNCTION:
        dest+="<function>";
        break;
      case LUA_TUSERDATA:
        dest+="<userdata>";
        break;
      case LUA_TTHREAD:
        dest+="<thread>";
        break;
      case LUA_TLIGHTUSERDATA:
        dest+="<lightuserdata>";
        break;
      default:
        dest+=_("<unknown value>");
        break;
    }
  }

  void inspectString(String& dest, int indent) {
    const char* str=lua_tostring(s,-1);
    if (str==NULL) {
      dest+="<what?>";
      return;
    }

    dest+="\"";
    for (const char* sp=str; *sp; sp++) {
      char c=*sp;
      switch (c) {
        case '\a': dest+="\\a"; break;
        case '\b': dest+="\\b"; break;
        case '\f': dest+="\\f"; break;
        case '\n': dest+="\\n"; break;
        case '\r': dest+="\\r"; break;
        case '\t': dest+="\\t"; break;
        case '\v': dest+="\\v"; break;
        case '\\': dest+="\\\\"; break;
        case '\"': dest+="\\\""; break;
        case '\'': dest+="'"; break;
        default:
          if (isprint(c)) dest+=c;
          else dest+=fmt::sprintf("\\%03d",(unsigned char)c);
          break;
      }
    }
    dest+="\"";
  }

  // figures out whether a string is a valid identifier ([a-zA-Z_][a-zA-Z0-9_]*)
  static bool isValidIdentifier(const char* s) {
    if (*s=='\0') return false;
    if (!isalpha(*s) && *s!='_') return false;
    while (*s) {
      if (!isalnum(*s) && *s!='_') return false;
      s++;
    }
    return true;
  }

  void inspectTable(String& dest, int indent) {
    // check if the table has already been inspected (to avoid infinite loops)
    lua_pushnil(s);
    lua_copy(s,-2,-1);
    lua_gettable(s,visitedIdx);
    if (!lua_isnil(s,-1)) {
      lua_pop(s,1);
      dest+=_("<cycle reached>");
      return;
    }
    lua_pop(s,1);

    // if we're here, it hasn't been inspected yet, so we gotta mark it as such.
    lua_pushnil(s);
    lua_copy(s,-2,-1);
    lua_pushboolean(s,true);
    lua_settable(s,visitedIdx);

    int tableIdx=lua_absindex(s,-1);

    // TODO: maybe there's a simpler way to inspect a table without this many passes?

    // figure out whether the table has array keys
    int lastArrayKey=0; // if 0, there are no array keys
    for (int i=1;; i++) {
      lua_pushinteger(s,i);
      lua_gettable(s,tableIdx);
      if (lua_isnil(s,lua_gettop(s))) {
        lua_pop(s,1);
        break;
      }
      lua_pop(s,1);
      lastArrayKey=i;
    }

    // whether there is a key-value pair on the top of the stack, where the key is one of the array keys
    const auto isTopArrayKvPair=[this,lastArrayKey](){
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
    while (lua_next(s,tableIdx)) {
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

    dest+="{";

    bool first=true;
    const auto maybeAddSep=[doIndent,nextIndent,&first,&dest](){
      if (first) {
        first=false;
      } else {
        dest+=",";
      }
      if (doIndent) {
        dest+="\n";
        for (int i=0; i<nextIndent; i++) dest+="  ";
      }
    };

    // first show the array entries
    for (int i=1; i<=lastArrayKey; i++) {
      lua_pushinteger(s,i);
      lua_gettable(s,tableIdx);

      if (lua_isnil(s,lua_gettop(s))) {
        lua_pop(s,1);
        logE("expected array key. why is there not one??? (k=%d)",i);
        continue;
      }

      maybeAddSep();
      inspectAny(dest,nextIndent);
      lua_pop(s,1);
    }

    // gets the top value as a string pointer if it is a valid identifier; returns NULL otherwise.
    // does not pop.
    const auto getTopIdentStr=[this]() -> const char* {
      if (lua_type(s,lua_gettop(s))!=LUA_TSTRING) return NULL;
      const char* str=lua_tostring(s,lua_gettop(s));
      if (str==NULL) return NULL;
      if (LuaValueInspector::isValidIdentifier(str)) return str;
      return NULL;
    };

    // then show the show the rest as k-v pairs
    lua_pushnil(s);
    while (lua_next(s,tableIdx)) {
      // skip "array" keys (already handled)
      if (isTopArrayKvPair()) {
        lua_pop(s,1);
        continue;
      }

      String valueRepr;
      inspectAny(valueRepr,nextIndent);
      lua_pop(s,1);

      maybeAddSep();
      const char* identStr=getTopIdentStr();
      if (identStr!=NULL) {
        dest+=identStr;
      } else {
        dest+="[";
        inspectAny(dest,nextIndent);
        dest+="]";
      }
      dest+="=";
      dest+=valueRepr;
    }

    if (doIndent) {
      dest+="\n";
      for (int i=0; i<indent; i++) dest+="  ";
    }
    dest+="}";
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
  if (lua_gettop(s)<=0) {
    return _("<unknown value>");
  }

  String ret;
  LuaValueInspector(s,indentTables).inspect(ret);
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
