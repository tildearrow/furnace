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
#include <fmt/printf.h>

void FurnaceGUI::drawStats() {
  if (nextWindow==GUI_WINDOW_STATS) {
    statsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!statsOpen) return;
  if (ImGui::Begin("Statistics",&statsOpen)) {
    if (curKStage==10) ImGui::SetWindowPos(ImVec2(ImGui::GetWindowPos().x+(rand()%256)-128,ImGui::GetWindowPos().y+(rand()%256)-128));
    String adpcmAUsage=fmt::sprintf("%d/16384KB",e->adpcmAMemLen/1024);
    String adpcmBUsage=fmt::sprintf("%d/16384KB",e->adpcmBMemLen/1024);
    String qsoundUsage=fmt::sprintf("%d/16384KB",e->qsoundMemLen/1024);
    String x1_010Usage=fmt::sprintf("%d/1024KB",e->x1_010MemLen/1024);
    ImGui::Text("ADPCM-A");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->adpcmAMemLen)/16777216.0f,ImVec2(-FLT_MIN,0),adpcmAUsage.c_str());
    ImGui::Text("ADPCM-B");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->adpcmBMemLen)/16777216.0f,ImVec2(-FLT_MIN,0),adpcmBUsage.c_str());
    ImGui::Text("QSound");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->qsoundMemLen)/16777216.0f,ImVec2(-FLT_MIN,0),qsoundUsage.c_str());
    ImGui::Text("X1-010");
    ImGui::SameLine();
    ImGui::ProgressBar(((float)e->x1_010MemLen)/1048576.0f,ImVec2(-FLT_MIN,0),x1_010Usage.c_str());
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_STATS;
  ImGui::End();
}