/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include "guiConst.h"
#include <imgui.h>

bool FurnaceGUI::systemPickerOption(DivSystem sys) {
  const DivSysDef* sysDef=e->getSystemDef(sys);
  if (sysDef==NULL) return false;
  bool ret=ImGui::Selectable(sysDef->name);
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(sysDef->description);
    ImGui::EndTooltip();
  }
  return ret;
}

DivSystem FurnaceGUI::systemPicker() {
  DivSystem ret=DIV_SYSTEM_NULL;
  /*
            for (int j=0; availableSystems[j]; j++) {
            if (!settings.hiddenSystems && (availableSystems[j]==DIV_SYSTEM_YMU759 || availableSystems[j]==DIV_SYSTEM_DUMMY)) continue;
            sysAddOption((DivSystem)availableSystems[j]);
          }
  */
  if (ImGui::InputTextWithHint("##SysSearch","Search...",&sysSearchQuery)) {
    String lowerCase=sysSearchQuery;
    for (char& i: lowerCase) {
      if (i>='A' && i<='Z') i+='a'-'A';
    }
    sysSearchResults.clear();
    for (int j=0; availableSystems[j]; j++) {
      String lowerCase1=e->getSystemName((DivSystem)availableSystems[j]);
      for (char& i: lowerCase1) {
        if (i>='A' && i<='Z') i+='a'-'A';
      }
      if (lowerCase1.find(lowerCase)!=String::npos) {
        sysSearchResults.push_back((DivSystem)availableSystems[j]);
      }
    }
  }
  if (sysSearchQuery.empty()) {
    // display chip list
    for (int j=0; availableSystems[j]; j++) {
      if (systemPickerOption((DivSystem)availableSystems[j])) ret=(DivSystem)availableSystems[j];
    }
  } else {
    // display search results
    for (DivSystem i: sysSearchResults) {
      if (systemPickerOption(i)) ret=i;
    }
  }
  return ret;
}