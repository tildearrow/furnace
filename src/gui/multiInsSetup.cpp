/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
#include "IconsFontAwesome4.h"

void FurnaceGUI::drawMultiInsSetup() {
  if (nextWindow==GUI_WINDOW_MULTI_INS_SETUP) {
    multiInsSetupOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!multiInsSetupOpen && !isMultiInsActive()) return;
  if (ImGui::Begin("Multi-Ins Setup",isMultiInsActive()?NULL:&multiInsSetupOpen,globalWinFlags,_("Multi-Ins Setup"))) {
    if (ImGui::BeginTable("MultiInsSlots",8,ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableNextRow();
      for (int i=0; i<8; i++) {
        ImGui::TableNextColumn();
        ImGui::Text("%d",i+1);
      }
      ImGui::TableNextRow();
      for (int i=0; i<8; i++) {
        String id;
        int tr=(i==0)?0:multiInsTranspose[i-1];
        bool thisInsOn=(i==0)?true:(multiIns[i-1]!=-1);
        if (tr==0) {
          id=fmt::sprintf("±%d###TrAmount",tr);
        } else if (tr>0) {
          id=fmt::sprintf("+%d###TrAmount",tr);
        } else {
          id=fmt::sprintf("%d###TrAmount",tr);
        }
        ImGui::TableNextColumn();
        ImGui::PushID(i);

        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat,true);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
        if (ImGui::Button(ICON_FA_CHEVRON_UP "##Up",ImVec2(ImGui::GetContentRegionAvail().x,0))) {
          if (i>0) {
            multiInsTranspose[i-1]++;
            if (multiInsTranspose[i-1]>60) multiInsTranspose[i-1]=60;
          }
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          if (i>0) {
            multiInsTranspose[i-1]+=12;
            if (multiInsTranspose[i-1]>60) multiInsTranspose[i-1]=60;
          }
        }
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();

        if (i>0) {
          ImVec4 colorActive=uiColors[GUI_COLOR_MULTI_INS_1+i-1];
          ImVec4 colorHover=ImVec4(colorActive.x,colorActive.y,colorActive.z,colorActive.w*0.5);
          ImVec4 color=ImVec4(colorActive.x,colorActive.y,colorActive.z,colorActive.w*0.25);
          ImGui::PushStyleColor(ImGuiCol_Header,color);
          ImGui::PushStyleColor(ImGuiCol_HeaderHovered,colorHover);
          ImGui::PushStyleColor(ImGuiCol_HeaderActive,colorActive);
        }
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,ImVec2(0.5f,0.5f));
        if (ImGui::Selectable(id.c_str(),thisInsOn,0,ImVec2(
            ImGui::GetContentRegionAvail().x,
            ImGui::GetContentRegionAvail().y-ImGui::GetTextLineHeightWithSpacing()
          ))) {
          if (i>0) multiInsTranspose[i-1]=0;
        }
        ImGui::PopStyleVar();
        if (i>0) {
          ImGui::PopStyleColor(3);
        }

        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat,true);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));
        if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##Down",ImVec2(ImGui::GetContentRegionAvail().x,0))) {
          if (i>0) {
            multiInsTranspose[i-1]--;
            if (multiInsTranspose[i-1]<-60) multiInsTranspose[i-1]=-60;
          }
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          if (i>0) {
            multiInsTranspose[i-1]-=12;
            if (multiInsTranspose[i-1]<-60) multiInsTranspose[i-1]=-60;
          }
        }
        ImGui::PopStyleVar();
        ImGui::PopItemFlag();
        
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MULTI_INS_SETUP;
  ImGui::End();
}
