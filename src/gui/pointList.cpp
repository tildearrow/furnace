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
#include <imgui.h>

float* PointList::compile(size_t& length) {
  return NULL;
}

void PointList::drawEditor(const ImVec2& size) {
  ImVec2 itemSize=size;
  if (size.x==0.0f) {
    itemSize.x=ImGui::GetContentRegionAvail().x;
  }
  if (size.y==0.0f) {
    itemSize.y=ImGui::GetContentRegionAvail().y;
  }

  
}