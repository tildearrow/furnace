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

void FurnaceGUI::drawChannels() {
  if (nextWindow==GUI_WINDOW_CHANNELS) {
    channelsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!channelsOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)):ImVec2(canvasW-(0.16*canvasH),canvasH));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    //ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Channels",&channelsOpen,globalWinFlags)) {
    if (ImGui::BeginTable("ChannelList",3)) {
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,48.0f*dpiScale);
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::Text("Visible");
      ImGui::TableNextColumn();
      ImGui::Text("Name");
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Checkbox("##Visible",&e->curSubSong->chanShow[i]);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROWS)) {
        }
        if (ImGui::BeginDragDropSource()) {
          chanToMove=i;
          ImGui::SetDragDropPayload("FUR_CHAN",NULL,0,ImGuiCond_Once);
          ImGui::Button(ICON_FA_ARROWS "##ChanDrag");
          ImGui::EndDragDropSource();
        } else if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("%s #%d\n(drag to swap channels)",e->getSystemName(e->sysOfChan[i]),e->dispatchChanOfChan[i]);
        }
        if (ImGui::BeginDragDropTarget()) {
          const ImGuiPayload* dragItem=ImGui::AcceptDragDropPayload("FUR_CHAN");
          if (dragItem!=NULL) {
            if (dragItem->IsDataType("FUR_CHAN")) {
              if (chanToMove!=i && chanToMove>=0) {
                e->swapChannelsP(chanToMove,i);
              }
              chanToMove=-1;
            }
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##ChanName",e->getChannelName(i),&e->curSubSong->chanName[i]);
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##ChanShortName",e->getChannelShortName(i),&e->curSubSong->chanShortName[i]);
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHANNELS;
  ImGui::End();
}
