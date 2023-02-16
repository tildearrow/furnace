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
#include "imgui_internal.h"

void FurnaceGUI::drawIntro() {
  if (introPos<7.0) {
    WAKE_UP;
    nextWindow=GUI_WINDOW_NOTHING;
    introPos+=ImGui::GetIO().DeltaTime;
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(canvasW,canvasH));
    if (ImGui::Begin("Intro",NULL,ImGuiWindowFlags_Modal)) {
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      ImVec2 top=ImVec2(0.0f,0.0f);
      ImVec2 bottom=ImVec2(canvasW,canvasH);

      ImU32 bgColor=ImGui::GetColorU32(ImVec4(0.0f,0.0f,0.0f,1.0f));

      dl->AddRectFilled(top,bottom,bgColor);
      dl->AddText(top,0xffffffff,"Furnace intro - work in progress");

      SDL_Texture* icon=getTexture(GUI_IMAGE_ICON);
      if (icon!=NULL) {
        dl->AddImage(icon,ImVec2(introPos*100,40),ImVec2(256+introPos*100,40+256));
      } else {

      }
    }
    ImGui::End();
  }
}
