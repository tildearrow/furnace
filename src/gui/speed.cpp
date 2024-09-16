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
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "intConst.h"

void FurnaceGUI::drawSpeed(bool asChild) {
  if (nextWindow==GUI_WINDOW_SPEED) {
    speedOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!speedOpen && !asChild) return;
  bool began=asChild?ImGui::BeginChild("Speed"):ImGui::Begin("Speed",&speedOpen,globalWinFlags,_("Speed"));
  if (began) {
    if (ImGui::BeginTable("Props",2,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      if (ImGui::SmallButton(tempoView?_("Base Tempo##TempoOrHz"):_("Tick Rate##TempoOrHz"))) {
        tempoView=!tempoView;
      }
      if (ImGui::IsItemHovered()) {
        if (tempoView) {
          ImGui::SetTooltip(_("click to display tick rate"));
        } else {
          ImGui::SetTooltip(_("click to display base tempo"));
        }
      }
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      float halfAvail=(avail-ImGui::GetStyle().ItemSpacing.x)*0.5;
      ImGui::SetNextItemWidth(halfAvail);
      float setHz=tempoView?e->curSubSong->hz*2.5:e->curSubSong->hz;
      if (ImGui::InputFloat("##Rate",&setHz,1.0f,10.0f,"%g")) { MARK_MODIFIED
        if (tempoView) setHz/=2.5;
        if (setHz<1) setHz=1;
        if (setHz>2048) setHz=2048;
        e->setSongRate(setHz);
      }
      if (tempoView) {
        ImGui::SameLine();
        ImGui::Text("= %gHz",e->curSubSong->hz);
      } else {
        if (e->curSubSong->hz>=49.98 && e->curSubSong->hz<=50.02) {
          ImGui::SameLine();
          ImGui::Text("PAL");
        }
        if (e->curSubSong->hz>=59.9 && e->curSubSong->hz<=60.11) {
          ImGui::SameLine();
          ImGui::Text("NTSC");
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      if (keepGrooveAlive || e->curSubSong->speeds.len>2) {
        if (ImGui::SmallButton(_("Groove"))) {
          e->lockEngine([this]() {
            e->curSubSong->speeds.len=1;
          });
          if (e->isPlaying()) play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("click for one speed"));
        }
      } else if (e->curSubSong->speeds.len>1) {
        if (ImGui::SmallButton(_("Speeds"))) {
          e->lockEngine([this]() {
            e->curSubSong->speeds.len=4;
            e->curSubSong->speeds.val[2]=e->curSubSong->speeds.val[0];
            e->curSubSong->speeds.val[3]=e->curSubSong->speeds.val[1];
          });
          if (e->isPlaying()) play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("click for groove pattern"));
        }
      } else {
        if (ImGui::SmallButton(_("Speed"))) {
          e->lockEngine([this]() {
            e->curSubSong->speeds.len=2;
            e->curSubSong->speeds.val[1]=e->curSubSong->speeds.val[0];
          });
          if (e->isPlaying()) play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("click for two (alternating) speeds"));
        }
      }
      ImGui::TableNextColumn();
      if (keepGrooveAlive || e->curSubSong->speeds.len>2) {
        int intVersion[256];
        unsigned char intVersionLen=e->curSubSong->speeds.len;
        unsigned char ignoredLoop=0;
        unsigned char ignoredRel=0;
        memset(intVersion,0,sizeof(int));
        for (int i=0; i<16; i++) {
          intVersion[i]=e->curSubSong->speeds.val[i];
        }
        if (intVersionLen>16) intVersionLen=16;

        keepGrooveAlive=false;

        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputText("##SpeedG",&grooveString)) {
          decodeMMLStr(grooveString,intVersion,intVersionLen,ignoredLoop,1,255,ignoredRel);
          if (intVersionLen<1) {
            intVersionLen=1;
            intVersion[0]=6;
          }
          if (intVersionLen>16) intVersionLen=16;
          e->lockEngine([this,intVersion,intVersionLen]() {
            e->curSubSong->speeds.len=intVersionLen;
            for (int i=0; i<16; i++) {
              e->curSubSong->speeds.val[i]=intVersion[i];
            }
          });
          if (e->isPlaying()) play();
          MARK_MODIFIED;
        }
        if (!ImGui::IsItemActive()) {
          encodeMMLStr(grooveString,intVersion,intVersionLen,-1,-1,false);
        } else {
          keepGrooveAlive=true;
        }
      } else {
        ImGui::SetNextItemWidth(halfAvail);
        if (ImGui::InputScalar("##Speed1",ImGuiDataType_U8,&e->curSubSong->speeds.val[0],&_ONE,&_THREE)) { MARK_MODIFIED
          if (e->curSubSong->speeds.val[0]<1) e->curSubSong->speeds.val[0]=1;
          if (e->isPlaying()) play();
        }
        if (e->curSubSong->speeds.len>1) {
          ImGui::SameLine();
          ImGui::SetNextItemWidth(halfAvail);
          if (ImGui::InputScalar("##Speed2",ImGuiDataType_U8,&e->curSubSong->speeds.val[1],&_ONE,&_THREE)) { MARK_MODIFIED
            if (e->curSubSong->speeds.val[1]<1) e->curSubSong->speeds.val[1]=1;
            if (e->isPlaying()) play();
          }
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Virtual Tempo"));
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(halfAvail);
      if (ImGui::InputScalar("##VTempoN",ImGuiDataType_S16,&e->curSubSong->virtualTempoN,&_ONE,&_TEN)) { MARK_MODIFIED
        if (e->curSubSong->virtualTempoN<1) e->curSubSong->virtualTempoN=1;
        if (e->curSubSong->virtualTempoN>255) e->curSubSong->virtualTempoN=255;
        e->virtualTempoChanged();
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Numerator"));
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(halfAvail);
      if (ImGui::InputScalar("##VTempoD",ImGuiDataType_S16,&e->curSubSong->virtualTempoD,&_ONE,&_TEN)) { MARK_MODIFIED
        if (e->curSubSong->virtualTempoD<1) e->curSubSong->virtualTempoD=1;
        if (e->curSubSong->virtualTempoD>255) e->curSubSong->virtualTempoD=255;
        e->virtualTempoChanged();
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("Denominator (set to base tempo)"));
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Divider"));
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(halfAvail);
      unsigned char realTB=e->curSubSong->timeBase+1;
      if (ImGui::InputScalar("##TimeBase",ImGuiDataType_U8,&realTB,&_ONE,&_THREE)) { MARK_MODIFIED
        if (realTB<1) realTB=1;
        if (realTB>16) realTB=16;
        e->curSubSong->timeBase=realTB-1;
      }
      ImGui::SameLine();
      ImGui::Text("%.2f BPM",calcBPM(e->curSubSong->speeds,e->curSubSong->hz,e->curSubSong->virtualTempoN,e->curSubSong->virtualTempoD));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Highlight"));
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(halfAvail);
      if (ImGui::InputScalar("##Highlight1",ImGuiDataType_U8,&e->curSubSong->hilightA,&_ONE,&_FOUR)) {
        MARK_MODIFIED;
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(halfAvail);
      if (ImGui::InputScalar("##Highlight2",ImGuiDataType_U8,&e->curSubSong->hilightB,&_ONE,&_FOUR)) {
        MARK_MODIFIED;
      }
      ImGui::EndTable();
    }

    ImGui::Separator();

    if (ImGui::BeginTable("Props2",3,ImGuiTableFlags_SizingStretchProp)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Pattern Length"));
      ImGui::TableNextColumn();
      float avail=ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);
      int patLen=e->curSubSong->patLen;
      if (ImGui::InputInt("##PatLength",&patLen,1,16)) { MARK_MODIFIED
        if (patLen<1) patLen=1;
        if (patLen>DIV_MAX_PATTERNS) patLen=DIV_MAX_PATTERNS;
        e->curSubSong->patLen=patLen;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Song Length"));
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(avail);
      int ordLen=e->curSubSong->ordersLen;
      if (ImGui::InputInt("##OrdLength",&ordLen,1,4)) { MARK_MODIFIED
        if (ordLen<1) ordLen=1;
        if (ordLen>DIV_MAX_PATTERNS) ordLen=DIV_MAX_PATTERNS;
        e->curSubSong->ordersLen=ordLen;
        if (curOrder>=ordLen) {
          setOrder(ordLen-1);
        }
      }

      ImGui::EndTable();
    }
  }
  if (!asChild && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SPEED;
  if (asChild) {
    ImGui::EndChild();
  } else {
    ImGui::End();
  }
}
