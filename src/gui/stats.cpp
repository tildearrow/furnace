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

#include "gui.h"
#include <fmt/printf.h>
#include <imgui.h>

void FurnaceGUI::drawStats() {
  if (nextWindow==GUI_WINDOW_STATS) {
    statsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!statsOpen) return;
  if (ImGui::Begin("Statistics",&statsOpen,globalWinFlags,_("Statistics"))) {
    size_t lastProcTime=e->processTime;
    double maxGot=1000000000.0*(double)e->getAudioDescGot().bufsize/(double)e->getAudioDescGot().rate;
    ImGui::AlignTextToFramePadding();
    ImGui::Text(_("Audio load"));
    ImGui::SameLine();
    ImGui::ProgressBar((double)lastProcTime/maxGot,ImVec2(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("100.0%").x,0),"");
    ImGui::SameLine();
    ImGui::Text("%.1f%%",100.0*((double)lastProcTime/(double)maxGot));
    if (ImGui::GetContentRegionAvail().y>8.0f*dpiScale) {
      // draw a chart
      lastAudioLoads[lastAudioLoadsPos]=(double)lastProcTime/maxGot;
      if (++lastAudioLoadsPos>=120) lastAudioLoadsPos=0;

      ImGui::PushStyleColor(ImGuiCol_FrameBg,ImVec4(0,0,0,0));
      ImGui::PlotLines("##ALChart",lastAudioLoads,120,lastAudioLoadsPos,NULL,0.0f,1.0f,ImGui::GetContentRegionAvail());
      ImGui::PopStyleColor();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_STATS;
  ImGui::End();
}
