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

void FurnaceGUI::drawNewSong() {
  bool accepted=false;

  ImGui::PushFont(bigFont);
  ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Choose a System!").x)*0.5);
  ImGui::Text("Choose a System!");
  ImGui::PopFont();

  if (ImGui::BeginTable("sysPicker",2)) {
    ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0f);
    ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
    ImGui::TableNextColumn();
    ImGui::Text("Categories");
    ImGui::TableNextColumn();
    ImGui::Text("Systems");

    ImGui::TableNextRow();

    // CATEGORIES
    ImGui::TableNextColumn();
    int index=0;
    for (FurnaceGUISysCategory& i: sysCategories) {
      if (ImGui::Selectable(i.name,newSongCategory==index,ImGuiSelectableFlags_DontClosePopups)) { \
        newSongCategory=index;
      }
      index++;
    }

    // SYSTEMS
    ImGui::TableNextColumn();
    if (ImGui::BeginTable("Systems",1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollY)) {
      for (FurnaceGUISysDef& i: sysCategories[newSongCategory].systems) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(i.name,false,ImGuiSelectableFlags_DontClosePopups)) {
          nextDesc=i.definition.data();
          accepted=true;
        }
      }
      ImGui::EndTable();
    }

    ImGui::EndTable();
  }

  if (ImGui::Button("I'm feeling lucky")) {
    if (sysCategories.size()==0) {
      ImGui::CloseCurrentPopup();
    } else {
      FurnaceGUISysCategory* newSystemCat=&sysCategories[rand()%sysCategories.size()];
      if (newSystemCat->systems.size()==0) {
        ImGui::CloseCurrentPopup();
      } else {
        nextDesc=newSystemCat->systems[rand()%newSystemCat->systems.size()].definition.data();
        accepted=true;
      }
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }

  if (accepted) {
    e->createNew(nextDesc);
    undoHist.clear();
    redoHist.clear();
    curFileName="";
    modified=false;
    curNibble=false;
    orderNibble=false;
    orderCursor=-1;
    samplePos=0;
    updateSampleTex=true;
    selStart=SelectionPoint();
    selEnd=SelectionPoint();
    cursor=SelectionPoint();
    updateWindowTitle();
    ImGui::CloseCurrentPopup();
  }
}
