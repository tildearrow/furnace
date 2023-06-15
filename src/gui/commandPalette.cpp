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

static std::vector<int> paletteItems;

static inline bool matchFuzzy(const char* haystack, const char* needle) {
  // TODO: case insensitivity
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

  if (paletteFirstFrame) {
    paletteItems.clear();
    for (int i=0; i<GUI_ACTION_MAX; i++) {
      paletteItems.push_back(i);
    }
  }

  if (paletteFirstFrame)
    ImGui::SetKeyboardFocusHere();

  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

  if (ImGui::InputTextWithHint("##CommandPaletteSearch","Search...",&paletteQuery) || paletteFirstFrame) {
    paletteSearchResults.clear();
    paletteSearchResults.reserve(paletteItems.size());
    for (int entry: paletteItems) {
      if (guiActions[entry].defaultBind==-1) continue;
      if (matchFuzzy(guiActions[entry].friendlyName, paletteQuery.c_str())) {
        paletteSearchResults.push_back(entry);
      }
    }
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

    for (size_t i=0; i<paletteSearchResults.size(); i++) {
      bool current=(i==curPaletteChoice);
      int id=paletteSearchResults[i];

      if (ImGui::Selectable(guiActions[id].friendlyName, current)) {
        curPaletteChoice=i;
        accepted=true;
      }
      if (navigated && current) ImGui::SetScrollHereY();
    }
  }
  ImGui::EndChild();

  if (!accepted) {
    if (curPaletteChoice>=paletteSearchResults.size()) {
      curPaletteChoice=paletteSearchResults.size()-1;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
      accepted=true;
    }
  }

  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::CloseCurrentPopup();
  }

  if (accepted) {
    int action=paletteSearchResults[curPaletteChoice];
    logD("Chose: %s", guiActions[action].friendlyName);
    doAction(action);
    ImGui::CloseCurrentPopup();
  }

  paletteFirstFrame=false;
}
