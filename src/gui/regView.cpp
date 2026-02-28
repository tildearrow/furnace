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

#include "../engine/bsr.h"
#include "gui.h"
#include <imgui.h>

#define ENABLE_REGVIEW_OPTIONS \
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) { \
    regViewOptions=!regViewOptions; \
  } \
  if (ImGui::IsItemHovered() && CHECK_LONG_HOLD) { \
    NOTIFY_LONG_HOLD; \
    regViewOptions=!regViewOptions; \
  }

void FurnaceGUI::drawRegView() {
  if (nextWindow==GUI_WINDOW_REGISTER_VIEW) {
    channelsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!regViewOpen) return;
  if (ImGui::Begin("Register View",&regViewOpen,globalWinFlags,_("Register View"))) {
    if (regViewOptions) {
      if (ImGui::BeginTable("regViewSettings",1)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text(_("Bytes per columns"));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##RegViewColumns",&regViewColumns,1,4)) {
          if (regViewColumns<1) regViewColumns=1;
          if (regViewColumns>64) regViewColumns=64;
        }
      }
      if (ImGui::Button(_("OK"))) {
        regViewOptions=false;
      }
    } else {
      for (int i=0; i<e->song.systemLen; i++) {
        ImGui::Text("%d. %s",i+1,getSystemName(e->song.system[i]));
        ENABLE_REGVIEW_OPTIONS
        int size=0;
        int depth=8;
        unsigned char* regPool=e->getRegisterPool(i,size,depth);
        unsigned short* regPoolW=(unsigned short*)regPool;
        if (regPool==NULL) {
          ImGui::Text(_("- no register pool available"));
          ENABLE_REGVIEW_OPTIONS
        } else {
          ImGui::PushFont(patFont);
          int rows=MAX(1,regViewColumns/MAX(1,(1<<(bsr(depth)-4))));
          if (ImGui::BeginTable("Memory",1+rows)) {
            int calcSize=size/rows;
            int calcSizeMul=(bsr32(calcSize)+3)>>2;
            float widthOne=ImGui::CalcTextSize("0").x;
            float widthMul=1.0f+((float)calcSizeMul);
            ImGui::TableSetupColumn("addr",ImGuiTableColumnFlags_WidthFixed, widthOne*widthMul);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            for (int i=0; i<rows; i++) {
              ImGui::TableNextColumn();
              ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX]," %X",i);
              ENABLE_REGVIEW_OPTIONS
            }
            for (int i=0; i<size; i+=rows) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::TextColored(uiColors[GUI_COLOR_PATTERN_ROW_INDEX],"%.2X",i);
              ENABLE_REGVIEW_OPTIONS
              for (int j=0; j<rows; j++) {
                ImGui::TableNextColumn();
                if (i+j>=size) continue;
                switch (depth) {
                  case 8: ImGui::Text("%.2x",regPool[i+j]); break;
                  case 16: ImGui::Text("%.4x",regPoolW[i+j]); break;
                  default: ImGui::Text("??"); break;
                }
                ENABLE_REGVIEW_OPTIONS
              }
            }
            ImGui::EndTable();
          }
          ImGui::PopFont();
        }
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_REGISTER_VIEW;
  ImGui::End();
}
