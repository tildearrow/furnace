/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

// NOTE: please don't ask me to enable text wrap.
//       Dear ImGui doesn't have that feature. D:
void FurnaceGUI::drawTuner(bool asChild) {
  if (nextWindow==GUI_WINDOW_TUNER) {
    tunerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!tunerOpen&&!asChild) return;
  bool began=asChild?ImGui::BeginChild("Song Info##Song Information"):ImGui::Begin("Song Comments",&tunerOpen,globalWinFlags,_("Tuner"));
}
