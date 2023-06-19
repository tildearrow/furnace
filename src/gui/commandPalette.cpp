/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
#include "guiConst.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <algorithm>
#include <cctype>
#include "../ta-log.h"

enum CommandPaletteType {
  CMDPAL_TYPE_MAIN = 0,
  CMDPAL_TYPE_RECENT,
  // TODO: are there more?

  CMDPAL_TYPE_MAX,
};

enum CommandPaletteExtraAction {
  CMDPAL_EXTRA_RECENT = 0,

  CMDPAL_EXTRA_MAX,
};

struct CommandPaletteExtraDef {
  const char* name;
  const char* friendlyName;
  CommandPaletteExtraDef(const char* n, const char* fn):
    name(n), friendlyName(fn) {}
};

#define D CommandPaletteExtraDef
const CommandPaletteExtraDef commandPaletteExtras[CMDPAL_EXTRA_MAX] = {
  D("CMDPAL_EXTRA_RECENT", "Recent files"),
};
#undef D

static inline bool matchFuzzy(const char* haystack,const char* needle) {
  size_t h_i=0; // haystack idx
  size_t n_i=0; // needle idx
  while (needle[n_i]!='\0') {
    for (; std::tolower(haystack[h_i])!=std::tolower(needle[n_i]); h_i++) {
      if (haystack[h_i]=='\0')
        return false;
    }
    n_i+=1;
  }
  return true;
}

void FurnaceGUI::drawPalette() {
  auto resetPalette = [](FurnaceGUI* g){
    g->paletteFirstFrame=true;
    g->paletteQuery="";
    g->curPaletteChoice=0;
  };

  bool accepted=false;

  if (paletteFirstFrame)
    ImGui::SetKeyboardFocusHere();

  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

  if (ImGui::InputTextWithHint("##CommandPaletteSearch","Search...",&paletteQuery) || paletteFirstFrame) {
    paletteSearchResults.clear();

    switch (curPaletteType) {
    case CMDPAL_TYPE_MAIN:
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        if (guiActions[i].defaultBind==-1) continue;
        if (matchFuzzy(guiActions[i].friendlyName,paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
        }
      }
      for (int i=0; i<CMDPAL_EXTRA_MAX; i++) {
        if (matchFuzzy(commandPaletteExtras[i].friendlyName,paletteQuery.c_str())) {
          paletteSearchResults.push_back(GUI_ACTION_MAX+i);
        }
      }
      break;

    case CMDPAL_TYPE_RECENT:
      for (int i=0; i<(int)recentFile.size(); i++) {
        if (matchFuzzy(recentFile[i].c_str(),paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
        }
      }
      break;

    default:
      // TODO: PANIC! DIE! PERISH!
      break;
    };
  }

  if (ImGui::BeginChild("CommandPaletteList",ImVec2(312,216),false,0)) {
    bool navigated=false;
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && curPaletteChoice>0) {
      curPaletteChoice-=1;
      navigated=true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
      curPaletteChoice+=1;
      navigated=true;
    }

    for (int i=0; i<(int)paletteSearchResults.size(); i++) {
      bool current=(i==curPaletteChoice);
      int id=paletteSearchResults[i];

      const char *s="???";
      switch (curPaletteType) {
      case CMDPAL_TYPE_MAIN:
        s=(id<GUI_ACTION_MAX)?(guiActions[id].friendlyName):(commandPaletteExtras[id-GUI_ACTION_MAX].friendlyName);
        break;
      case CMDPAL_TYPE_RECENT:
        s=recentFile[id].c_str();
        break;
      default:
        // TODO: DIE
        break;
      };

      if (ImGui::Selectable(s, current)) {
        curPaletteChoice=i;
        accepted=true;
      }
      if (navigated && current) ImGui::SetScrollHereY();
    }
  }
  ImGui::EndChild();

  if (!accepted) {
    if (curPaletteChoice>=(int)paletteSearchResults.size()) {
      curPaletteChoice=paletteSearchResults.size()-1;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
      // TODO: properly test this - what happens when enter is pressed and the list is empty?
      accepted=true;
    }
  }

  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::CloseCurrentPopup();
  }

  if (accepted) {
    int i=paletteSearchResults[curPaletteChoice];

    switch (curPaletteType) {
    case CMDPAL_TYPE_MAIN:
      if (i<GUI_ACTION_MAX) {
        doAction(i);
        ImGui::CloseCurrentPopup();
      } else {
        switch (i-GUI_ACTION_MAX) {
        case CMDPAL_EXTRA_RECENT:
          resetPalette(this);
          curPaletteType=CMDPAL_TYPE_RECENT;
          break;
        default:
          // TODO: PANIC! DIE! PERISH!
          break;
        };
      }
      break;

    case CMDPAL_TYPE_RECENT:
      openRecentFile(recentFile[i]);
      ImGui::CloseCurrentPopup();
      break;

    default:
      // TODO: PANIC! DIE! PERISH!
      break;
    };
  } else {
    paletteFirstFrame=false;
  }
}
