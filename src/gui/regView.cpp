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
#include <imgui.h>

void FurnaceGUI::drawRegView() {
  if (nextWindow==GUI_WINDOW_REGISTER_VIEW) {
    channelsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!regViewOpen) return;
  if (ImGui::Begin("Register View",&regViewOpen,globalWinFlags,_("Register View"))) {
    for (int i=0; i<e->song.systemLen; i++) {
      ImGui::Text("%d. %s",i+1,getSystemName(e->song.system[i]));
      int size=0;
      int depth=8;
      unsigned char* regPool=e->getRegisterPool(i,size,depth);
      unsigned short* regPoolW=(unsigned short*)regPool;
      if (regPool==NULL) {
        ImGui::Text(_("- no register pool available"));
      } else {
        ImGui::PushFont(patFont);
        if (ImGui::BeginTable("Memory",17)) {
          float widthOne=ImGui::CalcTextSize("0").x;
          if (size>0xfff) { // no im got gonna put some clamped log formula instead
            ImGui::TableSetupColumn("addr",ImGuiTableColumnFlags_WidthFixed, widthOne*4.0f);
          } else if (size>0xff) {
            ImGui::TableSetupColumn("addr",ImGuiTableColumnFlags_WidthFixed, widthOne*3.0f);
          } else {
            ImGui::TableSetupColumn("addr",ImGuiTableColumnFlags_WidthFixed, widthOne*2.0f);
          }
          
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          for (int i=0; i<16; i++) {
            ImGui::TableNextColumn();
            ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX]," %X",i);
          }
          for (int i=0; i<=((size-1)>>4); i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX],"%.2X",i*16);
            for (int j=0; j<16; j++) {
              ImGui::TableNextColumn();
              if (i*16+j>=size) continue;
              if (depth == 8) {
                ImGui::Text("%.2x",regPool[i*16+j]);
              } else if (depth == 16) {
                ImGui::Text("%.4x",regPoolW[i*16+j]);
              } else {
                ImGui::Text("??");
              }
            }
          }
          ImGui::EndTable();
        }
        ImGui::PopFont();
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_REGISTER_VIEW;
  ImGui::End();
}
