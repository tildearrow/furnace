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
#include "plot_nolerp.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include <imgui.h>

void FurnaceGUI::drawWaveEdit() {
  if (nextWindow==GUI_WINDOW_WAVE_EDIT) {
    waveEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!waveEditOpen) return;
  float wavePreview[257];
  ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f*dpiScale,300.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Wavetable Editor",&waveEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curWave<0 || curWave>=(int)e->song.wave.size()) {
      ImGui::Text("no wavetable selected");
    } else {
      DivWavetable* wave=e->song.wave[curWave];

      if (ImGui::BeginTable("WEProps",2)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(80.0f*dpiScale);
        if (ImGui::InputInt("##CurWave",&curWave,1,1)) {
          if (curWave<0) curWave=0;
          if (curWave>=(int)e->song.wave.size()) curWave=e->song.wave.size()-1;
        }
        ImGui::SameLine();
        // TODO: load replace
        if (ImGui::Button(ICON_FA_FOLDER_OPEN "##WELoad")) {
          doAction(GUI_ACTION_WAVE_LIST_OPEN);
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_O "##WESave")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE);
        }
        ImGui::SameLine();

        if (ImGui::RadioButton("Steps",waveEditStyle==0)) {
          waveEditStyle=0;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Lines",waveEditStyle==1)) {
          waveEditStyle=1;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Width");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("use a width of:\n- any on Amiga/N163\n- 32 on Game Boy, PC Engine and WonderSwan\n- 64 on FDS\n- 128 on X1-010\nany other widths will be scaled during playback.");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##_WTW",&wave->len,1,2)) {
          if (wave->len>256) wave->len=256;
          if (wave->len<1) wave->len=1;
          e->notifyWaveChange(curWave);
          if (wavePreviewOn) e->previewWave(curWave,wavePreviewNote);
          MARK_MODIFIED;
        }
        ImGui::SameLine();
        ImGui::Text("Height");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("use a height of:\n- 15 for Game Boy, WonderSwan, X1-010 Envelope shape and N163\n- 31 for PC Engine\n- 63 for FDS\n- 255 for X1-010\nany other heights will be scaled during playback.");
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##_WTH",&wave->max,1,2)) {
          if (wave->max>255) wave->max=255;
          if (wave->max<1) wave->max=1;
          e->notifyWaveChange(curWave);
          MARK_MODIFIED;
        }

        ImGui::SameLine();
        if (ImGui::Button(waveGenVisible?(ICON_FA_CHEVRON_RIGHT "##WEWaveGen"):(ICON_FA_CHEVRON_LEFT "##WEWaveGen"))) {
          waveGenVisible=!waveGenVisible;
        }

        ImGui::EndTable();
      }

      for (int i=0; i<wave->len; i++) {
        if (wave->data[i]>wave->max) wave->data[i]=wave->max;
        wavePreview[i]=wave->data[i];
      }
      if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];

      if (ImGui::BeginTable("WEWaveSection",waveGenVisible?2:1)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        if (waveGenVisible) ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,250.0f*dpiScale);
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0.0f,0.0f));

        ImVec2 contentRegion=ImGui::GetContentRegionAvail(); // wavetable graph size determined here
        contentRegion.y-=ImGui::GetFrameHeightWithSpacing()+ImGui::GetStyle().WindowPadding.y;
        if (waveEditStyle) {
          PlotNoLerp("##Waveform",wavePreview,wave->len+1,0,NULL,0,wave->max,contentRegion);
        } else {
          PlotCustom("##Waveform",wavePreview,wave->len,0,NULL,0,wave->max,contentRegion,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,true);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
          waveDragStart=ImGui::GetItemRectMin();
          waveDragAreaSize=contentRegion;
          waveDragMin=0;
          waveDragMax=wave->max;
          waveDragLen=wave->len;
          waveDragActive=true;
          waveDragTarget=wave->data;
          processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
          e->notifyWaveChange(curWave);
          modified=true;
        }
        ImGui::PopStyleVar();

        if (waveGenVisible) {
          ImGui::TableNextColumn();

          if (ImGui::BeginTabBar("WaveGenOpt")) {
            if (ImGui::BeginTabItem("Shapes")) {
              ImGui::Button("Square");
              ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("FM")) {
              ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Mangle")) {
              ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
          }
        }
        ImGui::EndTable();
      }

      if (ImGui::RadioButton("Dec",!waveHex)) {
        waveHex=false;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Hex",waveHex)) {
        waveHex=true;
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
      if (ImGui::InputText("##MMLWave",&mmlStringW)) {
        decodeMMLStrW(mmlStringW,wave->data,wave->len,wave->max,waveHex);
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStringW,wave->data,wave->len,-1,-1,waveHex);
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_EDIT;
  ImGui::End();
}
