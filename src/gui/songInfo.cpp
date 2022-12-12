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
  bool began=asChild?ImGui::BeginChild("Song Information"):ImGui::Begin("Song Information",&songInfoOpen,globalWinFlags);
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
      unsigned char realTB=e->curSubSong->timeBase+1;
      if (ImGui::InputScalar("##TimeBase",ImGuiDataType_U8,&realTB,&_ONE,&_THREE)) { MARK_MODIFIED
        if (realTB<1) realTB=1;
        if (realTB>16) realTB=16;
        e->curSubSong->timeBase=realTB-1;
      }
      ImGui::TableNextColumn();
      ImGui::Text("%.2f BPM",calcBPM(e->curSubSong->speed1,e->curSubSong->speed2,e->curSubSong->hz,e->curSubSong->virtualTempoN,e->curSubSong->virtualTempoD));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Speed");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed1",ImGuiDataType_U8,&e->curSubSong->speed1,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->speed1<1) e->curSubSong->speed1=1;
        if (e->isPlaying()) play();
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Speed2",ImGuiDataType_U8,&e->curSubSong->speed2,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->speed2<1) e->curSubSong->speed2=1;
        if (e->isPlaying()) play();
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Virtual Tempo");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##VTempoN",ImGuiDataType_S16,&e->curSubSong->virtualTempoN,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->virtualTempoN<1) e->curSubSong->virtualTempoN=1;
        if (e->curSubSong->virtualTempoN>255) e->curSubSong->virtualTempoN=255;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Numerator");
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##VTempoD",ImGuiDataType_S16,&e->curSubSong->virtualTempoD,&_ONE,&_THREE)) { MARK_MODIFIED
        if (e->curSubSong->virtualTempoD<1) e->curSubSong->virtualTempoD=1;
        if (e->curSubSong->virtualTempoD>255) e->curSubSong->virtualTempoD=255;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Denominator (set to base tempo)");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Highlight");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Highlight1",ImGuiDataType_U8,&e->curSubSong->hilightA,&_ONE,&_THREE)) {
        MARK_MODIFIED;
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      if (ImGui::InputScalar("##Highlight2",ImGuiDataType_U8,&e->curSubSong->hilightB,&_ONE,&_THREE)) {
        MARK_MODIFIED;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Pattern Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int patLen=e->curSubSong->patLen;
      if (ImGui::InputInt("##PatLength",&patLen,1,3)) { MARK_MODIFIED
        if (patLen<1) patLen=1;
        if (patLen>DIV_MAX_PATTERNS) patLen=DIV_MAX_PATTERNS;
        e->curSubSong->patLen=patLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("Song Length");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int ordLen=e->curSubSong->ordersLen;
      if (ImGui::InputInt("##OrdLength",&ordLen,1,3)) { MARK_MODIFIED
        if (ordLen<1) ordLen=1;
        if (ordLen>DIV_MAX_PATTERNS) ordLen=DIV_MAX_PATTERNS;
        e->curSubSong->ordersLen=ordLen;
        if (curOrder>=ordLen) {
          setOrder(ordLen-1);
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::Selectable(tempoView?"Base Tempo##TempoOrHz":"Tick Rate##TempoOrHz")) {
        tempoView=!tempoView;
      }
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      float setHz=tempoView?e->curSubSong->hz*2.5:e->curSubSong->hz;
      if (ImGui::InputFloat("##Rate",&setHz,1.0f,1.0f,"%g")) { MARK_MODIFIED
        if (tempoView) setHz/=2.5;
        if (setHz<1) setHz=1;
        if (setHz>999) setHz=999;
        e->setSongRate(setHz,setHz<52);
      }
      if (tempoView) {
        ImGui::TableNextColumn();
        ImGui::Text("= %gHz",e->curSubSong->hz);
      } else {
        if (e->curSubSong->hz>=49.98 && e->curSubSong->hz<=50.02) {
          ImGui::TableNextColumn();
          ImGui::Text("PAL");
        }
        if (e->curSubSong->hz>=59.9 && e->curSubSong->hz<=60.11) {
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
  if (!asChild && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SONG_INFO;
  if (asChild) {
    ImGui::EndChild();
  } else {
    ImGui::End();
  }
}
