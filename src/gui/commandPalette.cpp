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
#include "guiConst.h"
#include "commandPalette.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <algorithm>
#include <ctype.h>
#include "../ta-log.h"

static inline bool matchFuzzy(const char* haystack, const char* needle) {
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

  const char* hint=_("Search...");
  switch (curPaletteType) {
  case CMDPAL_TYPE_RECENT:
    hint=_("Search recent files...");
    break;
  case CMDPAL_TYPE_INSTRUMENTS:
    hint=_("Search instruments...");
    break;
  case CMDPAL_TYPE_SAMPLES:
    hint=_("Search samples...");
    break;
  case CMDPAL_TYPE_INSTRUMENT_CHANGE:
    hint=_("Search instruments (to change to)...");
    break;
  case CMDPAL_TYPE_ADD_CHIP:
    hint=_("Search chip (to add)...");
    break;
  }

  if (ImGui::InputTextWithHint("##CommandPaletteSearch",hint,&paletteQuery) || paletteFirstFrame) {
    paletteSearchResults.clear();

    switch (curPaletteType) {
    case CMDPAL_TYPE_MAIN:
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        if (guiActions[i].isNotABind()) continue;
        if (matchFuzzy(guiActions[i].friendlyName,paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
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

    case CMDPAL_TYPE_INSTRUMENTS:
    case CMDPAL_TYPE_INSTRUMENT_CHANGE:
      if (matchFuzzy(_("- None -"),paletteQuery.c_str())) {
        paletteSearchResults.push_back(0);
      }
      for (int i=0; i<e->song.insLen; i++) {
        String s=fmt::sprintf("%02X: %s", i, e->song.ins[i]->name.c_str());
        if (matchFuzzy(s.c_str(),paletteQuery.c_str())) {
          paletteSearchResults.push_back(i+1); // because over here ins=0 is 'None'
        }
      }
      break;

    case CMDPAL_TYPE_SAMPLES:
      for (int i=0; i<e->song.sampleLen; i++) {
        if (matchFuzzy(e->song.sample[i]->name.c_str(),paletteQuery.c_str())) {
          paletteSearchResults.push_back(i);
        }
      }
      break;

    case CMDPAL_TYPE_ADD_CHIP:
      for (int i=0; availableSystems[i]; i++) {
        int ds=availableSystems[i];
        const char* sysname=getSystemName((DivSystem)ds);
        if (matchFuzzy(sysname,paletteQuery.c_str())) {
          paletteSearchResults.push_back(ds);
        }
      }
      break;

    default:
      logE(_("invalid command palette type"));
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

    if (paletteSearchResults.size()>0 && curPaletteChoice<0) {
      curPaletteChoice=0;
      navigated=true;
    }
    if (curPaletteChoice>=(int)paletteSearchResults.size()) {
      curPaletteChoice=paletteSearchResults.size()-1;
      navigated=true;
    }

    for (int i=0; i<(int)paletteSearchResults.size(); i++) {
      bool current=(i==curPaletteChoice);
      int id=paletteSearchResults[i];

      String s="???";
      switch (curPaletteType) {
      case CMDPAL_TYPE_MAIN:
        s=guiActions[id].friendlyName;
        break;
      case CMDPAL_TYPE_RECENT:
        s=recentFile[id].c_str();
        break;
      case CMDPAL_TYPE_INSTRUMENTS:
      case CMDPAL_TYPE_INSTRUMENT_CHANGE:
        if (id==0) {
          s=_("- None -");
        } else {
          s=fmt::sprintf("%02X: %s", id-1, e->song.ins[id-1]->name.c_str());
        }
        break;
      case CMDPAL_TYPE_SAMPLES:
        s=e->song.sample[id]->name.c_str();
        break;
      case CMDPAL_TYPE_ADD_CHIP:
        s=getSystemName((DivSystem)id);
        break;
      default:
        logE(_("invalid command palette type"));
        break;
      };

      if (ImGui::Selectable(s.c_str(),current)) {
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

  if (ImGui::Button(_("Cancel")) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
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
          openRecentFile(recentFile[i]);
          break;
        case CMDPAL_TYPE_INSTRUMENTS:
          curIns=i-1;
          break;
        case CMDPAL_TYPE_SAMPLES:
          curSample=i;
          break;
        case CMDPAL_TYPE_INSTRUMENT_CHANGE:
          doChangeIns(i-1);
          break;
        case CMDPAL_TYPE_ADD_CHIP:
          if (i!=DIV_SYSTEM_NULL) {
            if (!e->addSystem((DivSystem)i)) {
              showError("cannot add chip! ("+e->getLastError()+")");
            } else {
              MARK_MODIFIED;
            }
            ImGui::CloseCurrentPopup();
            if (e->song.autoSystem) {
              autoDetectSystem();
            }
            updateWindowTitle();
          }
          break;
        default:
          logE(_("invalid command palette type"));
          break;
      };
    }
    ImGui::CloseCurrentPopup();
  }
}
