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

// NOTE: please don't ask me to enable text wrap.
//       Dear ImGui doesn't have that feature. D:
void FurnaceGUI::drawNotes() {
  if (nextWindow==GUI_WINDOW_NOTES) {
    notesOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!notesOpen) return;
  if (ImGui::Begin("Song Comments",&notesOpen,globalWinFlags)) {
    if (ImGui::InputTextMultiline("##SongNotes",&e->song.notes,ImGui::GetContentRegionAvail(),ImGuiInputTextFlags_UndoRedo)) {
      MARK_MODIFIED;
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_NOTES;
  ImGui::End();
}
