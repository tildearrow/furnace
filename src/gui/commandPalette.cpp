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
#include "commandPalette.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <algorithm>
#include <cctype>
#include "../ta-log.h"

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
  bool accepted=false;

  if (paletteFirstFrame)
    ImGui::SetKeyboardFocusHere();

  int width=ImGui::GetContentRegionAvail().x;
  ImGui::SetNextItemWidth(width);

  const char* hint="Search...";
  switch (curPaletteType) {
  case CMDPAL_TYPE_RECENT:
    hint="Search recent files...";
    break;
  case CMDPAL_TYPE_INSTRUMENTS:
    hint="Search instruments...";
    break;
  case CMDPAL_TYPE_SAMPLES:
    hint="Search samples...";
    break;
  }

  if (ImGui::InputTextWithHint("##CommandPaletteSearch",hint,&paletteQuery) || paletteFirstFrame) {
    paletteSearchResults.clear();

    switch (curPaletteType) {
    case CMDPAL_TYPE_MAIN:
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        if (guiActions[i].defaultBind==-1) continue;
        if (matchFuzzy(guiActions[i].friendlyName,paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
        }
      }
      break;

    case CMDPAL_TYPE_RECENT:
      for (int i=0; i<(int)recentFile.size(); i++) {
        if (matchFuzzy(recentFile.at(i).c_str(),paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
        }
      }
      break;

    case CMDPAL_TYPE_INSTRUMENTS:
      if (matchFuzzy("- None -",paletteQuery.c_str())) {
        paletteSearchResults.push_back(0);
      }
      for (int i=0; i<e->song.insLen; i++) {
        if (matchFuzzy(e->song.ins[i]->name.c_str(),paletteQuery.c_str())) {
          paletteSearchResults.push_back(i+1); // because over here ins=0 is 'None'
        }
      }
      break;

    case CMDPAL_TYPE_SAMPLES:
      for (int i=0; i<e->song.sampleLen; i++) {
        logD("ins #%x: %s", i, e->song.sample[i]->name.c_str());
        if (matchFuzzy(e->song.sample[i]->name.c_str(),paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
        }
      }
      break;

    default:
      logE("invalid command palette type");
      ImGui::CloseCurrentPopup();
      break;
    };
  }

  ImVec2 avail=ImGui::GetContentRegionAvail();
  avail.y-=ImGui::GetFrameHeightWithSpacing();

  if (ImGui::BeginChild("CommandPaletteList",avail,false,0)) {
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

      const char* s="???";
      switch (curPaletteType) {
      case CMDPAL_TYPE_MAIN:
        s=guiActions[id].friendlyName;
        break;
      case CMDPAL_TYPE_RECENT:
        s=recentFile.at(id).c_str();
        break;
      case CMDPAL_TYPE_INSTRUMENTS:
        if (id==0) {
          s="- None -";
        } else {
          s=e->song.ins[id-1]->name.c_str();
        }
        break;
      case CMDPAL_TYPE_SAMPLES:
        s=e->song.sample[id]->name.c_str();
        break;
      default:
        logE("invalid command palette type");
        break;
      };

      if (ImGui::Selectable(s,current)) {
        curPaletteChoice=i;
        accepted=true;
      }
      if ((navigated || paletteFirstFrame) && current) ImGui::SetScrollHereY();
    }
  }
  ImGui::EndChild();

  if (!accepted) {
    if (curPaletteChoice>=(int)paletteSearchResults.size()) {
      curPaletteChoice=paletteSearchResults.size()-1;
    }
    accepted=ImGui::IsKeyPressed(ImGuiKey_Enter);
  }

  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::CloseCurrentPopup();
  }

  // do not move this to after the resetPalette() calls!
  // if they are called before and we're jumping from one palette to the next, the paletteFirstFrame won't be true at the start and the setup will not happen.
  paletteFirstFrame=false;

  if (accepted) {
    if (paletteSearchResults.size()>0) {
      int i=paletteSearchResults[curPaletteChoice];
      switch (curPaletteType) {
      case CMDPAL_TYPE_MAIN:
        doAction(i);
        break;

      case CMDPAL_TYPE_RECENT:
        openRecentFile(recentFile.at(i));
        break;

      case CMDPAL_TYPE_INSTRUMENTS:
        curIns=i-1;
        break;

      case CMDPAL_TYPE_SAMPLES:
        curSample=i;
        break;

      default:
        logE("invalid command palette type");
        break;
      };
    }
    ImGui::CloseCurrentPopup();
  }
}
