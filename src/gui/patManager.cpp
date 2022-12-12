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
#include <imgui.h>

void FurnaceGUI::drawPatManager() {
  if (nextWindow==GUI_WINDOW_PAT_MANAGER) {
    patManagerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!patManagerOpen) return;
  char id[1024];
  unsigned char isUsed[DIV_MAX_PATTERNS];
  bool isNull[DIV_MAX_PATTERNS];
  if (ImGui::Begin("Pattern Manager",&patManagerOpen,globalWinFlags)) {
    ImGui::Text("Global Tasks");

    if (ImGui::Button("De-duplicate patterns")) {
      e->lockEngine([this]() {
        e->curSubSong->optimizePatterns();
      });
    }
    ImGui::SameLine();
    if (ImGui::Button("Re-arrange patterns")) {
      e->lockEngine([this]() {
        e->curSubSong->rearrangePatterns();
      });
    }

    if (ImGui::BeginTable("PatManTable",257,ImGuiTableFlags_ScrollX|ImGuiTableFlags_SizingFixedFit)) {
      ImGui::PushFont(patFont);

      for (int i=0; i<e->getTotalChannelCount(); i++) {
        ImGui::TableNextRow();
        memset(isUsed,0,DIV_MAX_PATTERNS);
        memset(isNull,0,DIV_MAX_PATTERNS*sizeof(bool));
        for (int j=0; j<e->curSubSong->ordersLen; j++) {
          isUsed[e->curSubSong->orders.ord[i][j]]++;
        }
        for (int j=0; j<DIV_MAX_PATTERNS; j++) {
          isNull[j]=(e->curSubSong->pat[i].data[j]==NULL);
        }
        ImGui::TableNextColumn();
        ImGui::Text("%s",e->getChannelShortName(i));

        ImGui::PushID(1000+i);
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          ImGui::TableNextColumn();

          snprintf(id,1023,"%.2X",k);
          if (isNull[k]) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PAT_MANAGER_NULL]);
          } else if (isUsed[k]>=e->curSubSong->ordersLen) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PAT_MANAGER_COMBO_BREAKER]);
          } else if (isUsed[k]>=0.7*(double)e->curSubSong->ordersLen) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED]);
          } else if (isUsed[k]>=0.4*(double)e->curSubSong->ordersLen) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PAT_MANAGER_OVERUSED]);
          } else if (isUsed[k]) {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PAT_MANAGER_USED]);
          } else {
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_PAT_MANAGER_UNUSED]);
          }
          ImGui::Selectable(id,isUsed[k]);
          if (ImGui::IsItemHovered()) {
            ImGui::PushFont(mainFont);
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
            if (isNull[k]) {
              ImGui::SetTooltip("Pattern %.2X\n- not allocated",k);
            } else {
              ImGui::SetTooltip("Pattern %.2X\n- use count: %d (%.0f%%)\n\nright-click to erase",k,isUsed[k],100.0*(double)isUsed[k]/(double)e->curSubSong->ordersLen);
            }
            ImGui::PopStyleColor();
            ImGui::PopFont();
          }
          if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            e->lockEngine([this,i,k]() {
              delete e->curSubSong->pat[i].data[k];
              e->curSubSong->pat[i].data[k]=NULL;
            });
          }
          ImGui::PopStyleColor();
        }
        ImGui::PopID();
      }
      ImGui::PopFont();

      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_PAT_MANAGER;
  ImGui::End();
}
