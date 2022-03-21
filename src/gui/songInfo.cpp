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
#include "intConst.h"

void FurnaceGUI::drawSongInfo() {
  if (nextWindow==GUI_WINDOW_SONG_INFO) {
    songInfoOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!songInfoOpen) return;
  if (ImGui::Begin("Song Information",&songInfoOpen)) {
    if (ImGui::BeginTable("NameAuthor",2,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Name");
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputText("##Name",&e->song.name)) { MARK_MODIFIED
        updateWindowTitle();
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Author");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputText("##Author",&e->song.author)) {
        MARK_MODIFIED;
      }
      ImGui::EndTable();
    }

    if (ImGui::BeginTable("OtherProps",3,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("TimeBase");
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      unsigned char realTB=e->song.timeBase+1;
      if (ImGui::InputScalar("##TimeBase",ImGuiDataType_U8,&realTB,&_ONE,&_THREE)) { MARK_MODIFIED
        if (realTB<1) realTB=1;
        if (realTB>16) realTB=16;
        e->song.timeBase=realTB-1;
      }
      ImGui::TableNextColumn();
      ImGui::Text("%.2f BPM",calcBPM(e->song.speed1,e->song.speed2,e->song.hz));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Speed");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed1",ImGuiDataType_U8,&e->song.speed1,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->song.speed1<1) e->song.speed1=1;
        if (e->isPlaying()) play();
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed2",ImGuiDataType_U8,&e->song.speed2,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->song.speed2<1) e->song.speed2=1;
        if (e->isPlaying()) play();
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Highlight");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Highlight1",ImGuiDataType_U8,&e->song.hilightA,&_ONE,&_THREE)) {
        MARK_MODIFIED;
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Highlight2",ImGuiDataType_U8,&e->song.hilightB,&_ONE,&_THREE)) {
        MARK_MODIFIED;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Pattern Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int patLen=e->song.patLen;
      if (ImGui::InputInt("##PatLength",&patLen,1,3)) { MARK_MODIFIED
        if (patLen<1) patLen=1;
        if (patLen>256) patLen=256;
        e->song.patLen=patLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Song Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int ordLen=e->song.ordersLen;
      if (ImGui::InputInt("##OrdLength",&ordLen,1,3)) { MARK_MODIFIED
        if (ordLen<1) ordLen=1;
        if (ordLen>127) ordLen=127;
        e->song.ordersLen=ordLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::Selectable(tempoView?"Base Tempo##TempoOrHz":"Tick Rate##TempoOrHz")) {
        tempoView=!tempoView;
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      float setHz=tempoView?e->song.hz*2.5:e->song.hz;
      if (ImGui::InputFloat("##Rate",&setHz,1.0f,1.0f,"%g")) { MARK_MODIFIED
        if (tempoView) setHz/=2.5;
        if (setHz<10) setHz=10;
        if (setHz>999) setHz=999;
        e->setSongRate(setHz,setHz<52);
      }
      if (tempoView) {
        ImGui::TableNextColumn();
        ImGui::Text("= %gHz",e->song.hz);
      } else {
        if (e->song.hz>=49.98 && e->song.hz<=50.02) {
          ImGui::TableNextColumn();
          ImGui::Text("PAL");
        }
        if (e->song.hz>=59.9 && e->song.hz<=60.11) {
          ImGui::TableNextColumn();
          ImGui::Text("NTSC");
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Tuning (A-4)");
      ImGui::TableNextColumn();
      float tune=e->song.tuning;
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputFloat("##Tuning",&tune,1.0f,3.0f,"%g")) { MARK_MODIFIED
        if (tune<220.0f) tune=220.0f;
        if (tune>880.0f) tune=880.0f;
        e->song.tuning=tune;
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SONG_INFO;
  ImGui::End();
}