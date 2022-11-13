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
#include <fmt/printf.h>
#include <algorithm>

void FurnaceGUI::drawNewSong() {
  bool accepted=false;

  ImGui::PushFont(bigFont);
  ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Choose a System!").x)*0.5);
  ImGui::Text("Choose a System!");
  ImGui::PopFont();

  ImVec2 avail=ImGui::GetContentRegionAvail();
  avail.y-=ImGui::GetFrameHeightWithSpacing();

  if (ImGui::BeginChild("sysPickerC",avail,false,ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoScrollbar)) {
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::InputTextWithHint("##SysSearch","Search...",&newSongQuery)) {
      String lowerCase=newSongQuery;
      for (char& i: lowerCase) {
        if (i>='A' && i<='Z') i+='a'-'A';
      }
      auto lastItem=std::remove_if(lowerCase.begin(),lowerCase.end(),[](char c) {
        return (c==' ' || c=='_' || c=='-');
      });
      lowerCase.erase(lastItem,lowerCase.end());
      newSongSearchResults.clear();
      for (FurnaceGUISysCategory& i: sysCategories) {
        for (FurnaceGUISysDef& j: i.systems) {
          String lowerCase1=j.name;
          for (char& i: lowerCase1) {
            if (i>='A' && i<='Z') i+='a'-'A';
          }
          auto lastItem=std::remove_if(lowerCase1.begin(),lowerCase1.end(),[](char c) {
            return (c==' ' || c=='_' || c=='-');
          });
          lowerCase1.erase(lastItem,lowerCase1.end());
          if (lowerCase1.find(lowerCase)!=String::npos) {
            newSongSearchResults.push_back(j);
          }
        }
        std::sort(newSongSearchResults.begin(),newSongSearchResults.end(),[](const FurnaceGUISysDef& a, const FurnaceGUISysDef& b) {
          return strcmp(a.name,b.name)<0;
        });
        auto lastItem=std::unique(newSongSearchResults.begin(),newSongSearchResults.end(),[](const FurnaceGUISysDef& a, const FurnaceGUISysDef& b) {
          return strcmp(a.name,b.name)==0;
        });
        newSongSearchResults.erase(lastItem,newSongSearchResults.end());
      }
    }
    if (ImGui::BeginTable("sysPicker",newSongQuery.empty()?2:1,ImGuiTableFlags_BordersInnerV)) {
      if (newSongQuery.empty()) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0f);
      }
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0f);

      if (newSongQuery.empty()) {
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text("Categories");
        ImGui::TableNextColumn();
        ImGui::Text("Systems");
      }

      ImGui::TableNextRow();

      // CATEGORIES
      if (newSongQuery.empty()) {
        ImGui::TableNextColumn();
        int index=0;
        for (FurnaceGUISysCategory& i: sysCategories) {
          if (ImGui::Selectable(i.name,newSongCategory==index,ImGuiSelectableFlags_DontClosePopups)) { \
            newSongCategory=index;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s",i.description);
          }
          index++;
        }
      }

      // SYSTEMS
      ImGui::TableNextColumn();
      if (ImGui::BeginTable("Systems",1,ImGuiTableFlags_BordersInnerV|ImGuiTableFlags_ScrollY)) {
        std::vector<FurnaceGUISysDef>& category=(newSongQuery.empty())?(sysCategories[newSongCategory].systems):(newSongSearchResults);
        for (FurnaceGUISysDef& i: category) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (ImGui::Selectable(i.name,false,ImGuiSelectableFlags_DontClosePopups)) {
            nextDesc=i.definition;
            nextDescName=i.name;
            accepted=true;
          }
        }
        ImGui::EndTable();
      }

      ImGui::EndTable();
    }
  }
  ImGui::EndChild();

  if (ImGui::Button("I'm feeling lucky")) {
    if (sysCategories.size()==0) {
      ImGui::CloseCurrentPopup();
    } else {
      FurnaceGUISysCategory* newSystemCat=&sysCategories[rand()%sysCategories.size()];
      if (newSystemCat->systems.size()==0) {
        ImGui::CloseCurrentPopup();
      } else {
        unsigned int selection=rand()%newSystemCat->systems.size();
        nextDesc=newSystemCat->systems[selection].definition;
        nextDescName=newSystemCat->systems[selection].name;
        accepted=true;
      }
    }
  }

  ImGui::SameLine();

  if (ImGui::Button("Cancel")) {
    ImGui::CloseCurrentPopup();
  }

  if (accepted) {
    e->createNew(nextDesc.c_str(),nextDescName,false);
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
