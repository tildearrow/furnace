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

void FurnaceGUI::drawImage(ImDrawList* dl, FurnaceGUIImages image, const ImVec2& pos, const ImVec2& scale, double rotate, const ImVec2& uvMin, const ImVec2& uvMax, const ImVec4& imgColor) {
  FurnaceGUIImage* imgI=getImage(image);
  SDL_Texture* img=getTexture(image);

  ImVec2 rectMin=ImVec2(
    -imgI->width*0.5*scale.x,
    -imgI->height*0.5*scale.y
  );
  ImVec2 rectMax=ImVec2(
    -rectMin.x,
    -rectMin.y
  );

  ImVec2 quad0=ImVec2(
    pos.x+rectMin.x*cos(rotate)-rectMin.y*sin(rotate),
    pos.y+rectMin.x*sin(rotate)+rectMin.y*cos(rotate)
  );
  ImVec2 quad1=ImVec2(
    pos.x+rectMax.x*cos(rotate)-rectMin.y*sin(rotate),
    pos.y+rectMax.x*sin(rotate)+rectMin.y*cos(rotate)
  );
  ImVec2 quad2=ImVec2(
    pos.x+rectMax.x*cos(rotate)-rectMax.y*sin(rotate),
    pos.y+rectMax.x*sin(rotate)+rectMax.y*cos(rotate)
  );
  ImVec2 quad3=ImVec2(
    pos.x+rectMin.x*cos(rotate)-rectMax.y*sin(rotate),
    pos.y+rectMin.x*sin(rotate)+rectMax.y*cos(rotate)
  );

  ImVec2 uv0=ImVec2(uvMin.x,uvMin.y);
  ImVec2 uv1=ImVec2(uvMax.x,uvMin.y);
  ImVec2 uv2=ImVec2(uvMax.x,uvMax.y);
  ImVec2 uv3=ImVec2(uvMin.x,uvMax.y);

  ImU32 colorConverted=ImGui::GetColorU32(imgColor);

  dl->AddImageQuad(img,quad0,quad1,quad2,quad3,uv0,uv1,uv2,uv3,colorConverted);
}

void FurnaceGUI::drawIntro() {
  if (introPos<7.0) {
    WAKE_UP;
    nextWindow=GUI_WINDOW_NOTHING;
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(canvasW,canvasH));
    if (ImGui::Begin("Intro",NULL,ImGuiWindowFlags_Modal)) {
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      ImVec2 top=ImVec2(0.0f,0.0f);
      ImVec2 bottom=ImVec2(canvasW,canvasH);

      ImU32 bgColor=ImGui::GetColorU32(ImVec4(0.0f,0.0f,0.0f,1.0f));

      dl->AddRectFilled(top,bottom,bgColor);
      dl->AddText(top,0xffffffff,"Furnace intro - work in progress");

      drawImage(dl,GUI_IMAGE_TALOGO,ImVec2(0.5*canvasW,0.5*canvasH),ImVec2(1.0,1.0),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(1.0,1.0,1.0,pow(MIN(1.0,introPos*2.0),0.8)));
    }
    ImGui::End();

    if (mustClear<=0) {
      introPos+=ImGui::GetIO().DeltaTime;
    }
  }
}
