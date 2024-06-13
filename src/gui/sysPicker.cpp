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
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include "guiConst.h"
#include <imgui.h>

DivSystem FurnaceGUI::systemPicker() {
  DivSystem ret=DIV_SYSTEM_NULL;
  DivSystem hoveredSys=DIV_SYSTEM_NULL;
  bool reissueSearch=false;
  if (curSysSection==NULL) {
    curSysSection=availableSystems;
  }

  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  if (ImGui::InputTextWithHint("##SysSearch",_("Search..."),&sysSearchQuery)) reissueSearch=true;
  if (ImGui::BeginTabBar("SysCats")) {
    for (int i=0; chipCategories[i]; i++) {
      if (ImGui::BeginTabItem(_(chipCategoryNames[i]))) {
        if (ImGui::IsItemActive()) {
          reissueSearch=true;
        }
        curSysSection=chipCategories[i];
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  if (reissueSearch) {
    String lowerCase=sysSearchQuery;
    for (char& i: lowerCase) {
      if (i>='A' && i<='Z') i+='a'-'A';
    }
    sysSearchResults.clear();
    for (int j=0; curSysSection[j]; j++) {
      String lowerCase1=e->getSystemName((DivSystem)curSysSection[j]);
      for (char& i: lowerCase1) {
        if (i>='A' && i<='Z') i+='a'-'A';
      }
      if (lowerCase1.find(lowerCase)!=String::npos) {
        sysSearchResults.push_back((DivSystem)curSysSection[j]);
      }
    }
  }
  if (ImGui::BeginTable("SysList",1,ImGuiTableFlags_ScrollY,ImVec2(500.0f*dpiScale,200.0*dpiScale))) {
    if (sysSearchQuery.empty()) {
      // display chip list
      for (int j=0; curSysSection[j]; j++) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(e->getSystemName((DivSystem)curSysSection[j]),false,0,ImVec2(500.0f*dpiScale,0.0f))) ret=(DivSystem)curSysSection[j];
        if (ImGui::IsItemHovered()) {
          hoveredSys=(DivSystem)curSysSection[j];
        }
      }
    } else {
      // display search results
      for (DivSystem i: sysSearchResults) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(e->getSystemName(i),false,0,ImVec2(500.0f*dpiScale,0.0f))) ret=i;
        if (ImGui::IsItemHovered()) {
          hoveredSys=i;
        }
      }
    }
    ImGui::EndTable();
  }
  ImGui::Separator();
  if (ImGui::BeginChild("SysDesc",ImVec2(0.0f,150.0f*dpiScale),false,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse)) {
    if (hoveredSys!=DIV_SYSTEM_NULL) {
      const DivSysDef* sysDef=e->getSystemDef(hoveredSys);
      ImGui::TextWrapped("%s",sysDef->description);
    }
  }
  ImGui::EndChild();
  return ret;
}
