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
#include <imgui.h>

void FurnaceGUI::drawMemory() {
  if (nextWindow==GUI_WINDOW_MEMORY) {
    memoryOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!memoryOpen) return;
  if (ImGui::Begin("Memory Composition",&memoryOpen,globalWinFlags)) {
    for (int i=0; i<e->song.systemLen; i++) {
      DivDispatch* dispatch=e->getDispatch(i);
      for (int j=0; j<4; j++) {
        const DivMemoryComposition* mc=dispatch->getMemCompo(j);
        if (mc==NULL) break;

        ImGui::Text("%s: %s",e->getSystemName(e->song.system[i]),mc->name.c_str());
      }
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MEMORY;
  ImGui::End();
}
