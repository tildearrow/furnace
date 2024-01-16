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
#include <fmt/printf.h>
#include <imgui.h>

void FurnaceGUI::drawStats() {
  if (nextWindow==GUI_WINDOW_STATS) {
    statsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!statsOpen) return;
  if (ImGui::Begin("Statistics",&statsOpen,globalWinFlags)) {
    size_t lastProcTime=e->processTime;
    double maxGot=1000000000.0*(double)e->getAudioDescGot().bufsize/(double)e->getAudioDescGot().rate;
    String procStr=fmt::sprintf("%.1f%%",100.0*((double)lastProcTime/(double)maxGot));
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Audio load");
    ImGui::SameLine();
    ImGui::ProgressBar((double)lastProcTime/maxGot,ImVec2(-FLT_MIN,0),procStr.c_str());
    ImGui::Separator();
    for (int i=0; i<e->song.systemLen; i++) {
      DivDispatch* dispatch=e->getDispatch(i);
      for (int j=0; dispatch!=NULL && dispatch->getSampleMemCapacity(j)>0; j++) {
        size_t capacity=dispatch->getSampleMemCapacity(j);
        size_t usage=dispatch->getSampleMemUsage(j);
        String usageStr;
        if (settings.memUsageUnit==1) {
          usageStr=fmt::sprintf("%d/%dKB",usage/1024,capacity/1024);
        } else {
          usageStr=fmt::sprintf("%d/%d",usage,capacity);
        }
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s [%d]", e->getSystemName(e->song.system[i]), j);
        ImGui::SameLine();
        ImGui::ProgressBar(((float)usage)/((float)capacity),ImVec2(-FLT_MIN,0),usageStr.c_str());
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_STATS;
  ImGui::End();
}
