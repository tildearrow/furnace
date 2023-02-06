/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
#include "intConst.h"

void FurnaceGUI::drawSongInfo(bool asChild) {
  if (nextWindow==GUI_WINDOW_SONG_INFO) {
    songInfoOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!songInfoOpen && !asChild) return;
  bool began=asChild?ImGui::BeginChild("Song Info##Song Information"):ImGui::Begin("Song Info##Song Information",&songInfoOpen,globalWinFlags);
  if (began) {
    if (ImGui::BeginTable("NameAuthor",2,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Name");
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputText("##Name",&e->song.name,ImGuiInputTextFlags_UndoRedo)) { MARK_MODIFIED
        updateWindowTitle();
      }
      if (e->song.insLen==1) {
        unsigned int checker=0x11111111;
        unsigned int checker1=0;
        DivInstrument* ins=e->getIns(0);
        if (ins->name.size()==15 && e->curSubSong->ordersLen==8) {
          for (int i=0; i<15; i++) {
            checker^=ins->name[i]<<i;
            checker1+=ins->name[i];
            checker=(checker>>1|(((checker)^(checker>>2)^(checker>>3)^(checker>>5))&1)<<31);
            checker1<<=1;
          }
          if (checker==0x5ec4497d && checker1==0x6347ee) nonLatchNibble=true;
        }
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Author");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputText("##Author",&e->song.author,ImGuiInputTextFlags_UndoRedo)) {
        MARK_MODIFIED;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Album");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputText("##Category",&e->song.category,ImGuiInputTextFlags_UndoRedo)) {
        MARK_MODIFIED;
      }
      if (!basicMode) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("System");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(MAX(16.0f*dpiScale,avail-autoButtonSize-ImGui::GetStyle().ItemSpacing.x));
        if (ImGui::InputText("##SystemName",&e->song.systemName,ImGuiInputTextFlags_UndoRedo)) {
          MARK_MODIFIED;
          updateWindowTitle();
          e->song.autoSystem=false;
        }
        ImGui::SameLine();
        pushToggleColors(e->song.autoSystem);
        if (ImGui::Button("Auto")) {
          e->song.autoSystem=!e->song.autoSystem;
          if (e->song.autoSystem) {
            autoDetectSystem();
            updateWindowTitle();
          }
          MARK_MODIFIED;
        }
        popToggleColors();
        autoButtonSize=ImGui::GetItemRectSize().x;
      }

      ImGui::EndTable();
    }

    if (basicMode) {
      if (e->song.tuning<435.8 || e->song.tuning>444) {
        ImGui::TextWrapped("Tuning changed - disable Basic Mode to edit.");
      }
    } else if (ImGui::BeginTable("OtherProps",2,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Tuning (A-4)");
      ImGui::TableNextColumn();
      float tune=e->song.tuning;
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputFloat("##Tuning",&tune,1.0f,3.0f,"%g")) { MARK_MODIFIED
        if (tune<220.0f) tune=220.0f;
        if (tune>880.0f) tune=880.0f;
        e->song.tuning=tune;
      }
      ImGui::EndTable();
    }
  }
  if (!asChild && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SONG_INFO;
  if (asChild) {
    ImGui::EndChild();
  } else {
    ImGui::End();
  }
}
