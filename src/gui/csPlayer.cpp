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
#include <fmt/printf.h>
#include "imgui.h"

void FurnaceGUI::drawCSPlayer() {
  if (nextWindow==GUI_WINDOW_CS_PLAYER) {
    csPlayerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!csPlayerOpen) return;
  if (ImGui::Begin("Command Stream Player",&csPlayerOpen,globalWinFlags)) {
    if (ImGui::Button("Load")) {
      openFileDialog(GUI_FILE_CMDSTREAM_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button("Kill")) {
      if (!e->killStream()) {
        showError("Kikai wa mou shindeiru!");
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Burn Current Song")) {
      SafeWriter* w=e->saveCommand(true);
      if (w!=NULL) {
        if (!e->playStream(w->getFinalBuf(),w->size())) {
          showError(e->getLastError());
          w->finish();
          delete w;
        } else {
          w->disown();
          delete w;
        }
      }
    }

    DivCSPlayer* cs=e->getStreamPlayer();
    if (cs) {
      if (ImGui::BeginTabBar("CSOptions")) {
        int chans=e->getTotalChannelCount();
        if (ImGui::BeginTabItem("Status")) {
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Hex")) {
          ImGui::PushFont(patFont);
          if (ImGui::BeginTable("CSHexPos",chans,ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableNextRow();
            for (int i=0; i<chans; i++) {
              ImGui::TableNextColumn();
              ImGui::Text("%d",i);
            }
            ImGui::TableNextRow();
            for (int i=0; i<chans; i++) {
              DivCSChannelState* state=cs->getChanState(i);
              ImGui::TableNextColumn();
              ImGui::Text("$%x",state->readPos);
            }
            ImGui::EndTable();
          }

          float oneCharSize=ImGui::CalcTextSize("A").x;
          float threeCharSize=ImGui::CalcTextSize("AA").x;
          float charViewSize=ImGui::CalcTextSize("0123456789ABCDEF").x;

          if (ImGui::BeginTable("CSHexPos",19)) {
            char charView[17];
            ImGui::TableSetupScrollFreeze(1,1);
            ImGui::TableSetupColumn("addr",ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("d0",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d1",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d2",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d3",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d4",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d5",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d6",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d7",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d8",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d9",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d10",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d11",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d12",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d13",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d14",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("d15",ImGuiTableColumnFlags_WidthFixed,threeCharSize);
            ImGui::TableSetupColumn("spacer",ImGuiTableColumnFlags_WidthFixed,oneCharSize);
            ImGui::TableSetupColumn("char",ImGuiTableColumnFlags_WidthFixed,charViewSize);

            // header
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            for (int i=0; i<16; i++) {
              ImGui::TableNextColumn();
              ImGui::Text("%X",i);
            }

            // content
            unsigned char* buf=cs->getData();
            size_t bufSize=cs->getDataLen();
            csClipper.Begin((bufSize+15)>>4,ImGui::GetTextLineHeightWithSpacing());
            while (csClipper.Step()) {
              //std::vector<int> highlightsUnsorted;
              std::vector<int> highlights;
              int nextHighlight=-1;
              int highlightPos=0;

              for (int i=0; i<chans; i++) {
                DivCSChannelState* state=cs->getChanState(i);
                if ((int)state->readPos>=(csClipper.DisplayStart<<4) && (int)state->readPos<=(csClipper.DisplayEnd<<4)) {
                  highlights.push_back(state->readPos);
                }
              }
              if (!highlights.empty()) nextHighlight=highlights[0];


              for (int i=csClipper.DisplayStart; i<csClipper.DisplayEnd; i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%.4X",i<<4);

                for (int j=0; j<16; j++) {
                  int pos=(i<<4)|j;
                  ImGui::TableNextColumn();
                  if (pos>=(int)bufSize) continue;
                  if (pos==nextHighlight) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,ImGui::GetColorU32(ImGuiCol_Header));
                    highlightPos++;
                    if (highlightPos>=(int)highlights.size()) {
                      nextHighlight=-1;
                    } else {
                      nextHighlight=highlights[highlightPos];
                    }
                  }
                  ImGui::Text("%.2X",buf[pos]);
                }

                ImGui::TableNextColumn();
                ImGui::TableNextColumn();
                for (int j=0; j<16; j++) {
                  int pos=(i<<4)|j;
                  if (pos>=(int)bufSize) {
                    charView[j]=' ';
                  } else if (buf[pos]>=0x20 && buf[pos]<=0x7e) {
                    charView[j]=buf[pos];
                  } else {
                    charView[j]='.';
                  }
                }
                charView[16]=0;

                ImGui::TextUnformatted(charView);
              }
            }
            csClipper.End();

            ImGui::EndTable();
          }
          ImGui::PopFont();
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CS_PLAYER;
  ImGui::End();
}
