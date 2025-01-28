/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
#include "IconsFontAwesome4.h"
#include <fmt/printf.h>
#include "intConst.h"

void FurnaceGUI::drawGrooves() {
  if (nextWindow==GUI_WINDOW_GROOVES) {
    groovesOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!groovesOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Grooves",&groovesOpen,globalWinFlags,_("Grooves"))) {
    int delGroove=-1;

    ImGui::Text(_("use effect 09xx to select a groove pattern."));
    if (!e->song.grooves.empty()) if (ImGui::BeginTable("GrooveList",3,ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);

      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::Text("#");
      ImGui::TableNextColumn();
      ImGui::Text(_("pattern"));
      ImGui::TableNextColumn();
      // ImGui::Text("remove"); removed because the text clips from the fixed width

      int index=0;
      for (DivGroovePattern& i: e->song.grooves) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushFont(patFont);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%.2X",index);
        ImGui::PopFont();

        ImGui::TableNextColumn();

        String grooveStr;

        if (curGroove==index) {
          int intVersion[256];
          unsigned char intVersionLen=i.len;
          unsigned char ignoredLoop=0;
          unsigned char ignoredRel=0;
          memset(intVersion,0,sizeof(int));
          for (int j=0; j<16; j++) {
            intVersion[j]=i.val[j];
          }
          if (intVersionLen>16) intVersionLen=16;
          grooveStr=fmt::sprintf("##_GRI%d",index);
          bool wantedFocus=wantGrooveListFocus;
          if (wantGrooveListFocus) {
            wantGrooveListFocus=false;
            ImGui::SetItemDefaultFocus();
            ImGui::SetKeyboardFocusHere();
          }
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::AlignTextToFramePadding();
          if (ImGui::InputText(grooveStr.c_str(),&grooveListString)) {
            decodeMMLStr(grooveListString,intVersion,intVersionLen,ignoredLoop,1,255,ignoredRel);
            if (intVersionLen<1) {
              intVersionLen=1;
              intVersion[0]=6;
            }
            if (intVersionLen>16) intVersionLen=16;
            e->lockEngine([&i,intVersion,intVersionLen]() {
              i.len=intVersionLen;
              for (int j=0; j<16; j++) {
                i.val[j]=intVersion[j];
              }
            });
            MARK_MODIFIED;
          }
          if (!ImGui::IsItemActive() && !wantedFocus) {
            curGroove=-1;
            //encodeMMLStr(grooveListString,intVersion,intVersionLen,-1,-1,false);
          }
        } else {
          String grooveStr;

          for (int j=0; j<i.len; j++) {
            if (j>0) {
              grooveStr+=' ';
            }
            grooveStr+=fmt::sprintf("%d",(int)i.val[j]);
          }

          size_t groovePrevLen=grooveStr.size();

          grooveStr+=fmt::sprintf("##_GR%d",index);

          if (ImGui::Selectable(grooveStr.c_str(),false)) {
            curGroove=index;
            grooveListString=grooveStr.substr(0,groovePrevLen);
            wantGrooveListFocus=true;
          }
        }

        ImGui::TableNextColumn();
        pushDestColor();
        String grooveID=fmt::sprintf(ICON_FA_TIMES "##GRR%d",index);
        if (ImGui::Button(grooveID.c_str())) {
          delGroove=index;
        }
        popDestColor();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("remove"));
        }

        index++;
      }

      ImGui::EndTable();
    }

    if (delGroove>=0) {
      e->lockEngine([this,delGroove]() {
        e->song.grooves.erase(e->song.grooves.begin()+delGroove);
      });
      MARK_MODIFIED;
    }

    if (ImGui::Button(ICON_FA_PLUS "##AddGroove")) {
      e->lockEngine([this]() {
        e->song.grooves.push_back(DivGroovePattern());
      });
      MARK_MODIFIED;
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
    curWindow=GUI_WINDOW_GROOVES;
  } else {
    curGroove=-1;
  }
  ImGui::End();
}
