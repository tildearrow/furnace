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

  float squareSize=MAX(canvasW,canvasH);
  float uDiff=uvMax.x-uvMin.x;
  float vDiff=uvMax.y-uvMin.y;

  ImVec2 rectMin=ImVec2(
    -imgI->width*0.5*scale.x*(squareSize/2048.0)*uDiff,
    -imgI->height*0.5*scale.y*(squareSize/2048.0)*vDiff
  );
  ImVec2 rectMax=ImVec2(
    -rectMin.x,
    -rectMin.y
  );

  ImVec2 posAbs=ImLerp(
    ImVec2((canvasW-squareSize)*0.5,(canvasH-squareSize)*0.5),
    ImVec2((canvasW+squareSize)*0.5,(canvasH+squareSize)*0.5),
    pos
  );

  ImVec2 quad0=ImVec2(
    posAbs.x+rectMin.x*cos(rotate)-rectMin.y*sin(rotate),
    posAbs.y+rectMin.x*sin(rotate)+rectMin.y*cos(rotate)
  );
  ImVec2 quad1=ImVec2(
    posAbs.x+rectMax.x*cos(rotate)-rectMin.y*sin(rotate),
    posAbs.y+rectMax.x*sin(rotate)+rectMin.y*cos(rotate)
  );
  ImVec2 quad2=ImVec2(
    posAbs.x+rectMax.x*cos(rotate)-rectMax.y*sin(rotate),
    posAbs.y+rectMax.x*sin(rotate)+rectMax.y*cos(rotate)
  );
  ImVec2 quad3=ImVec2(
    posAbs.x+rectMin.x*cos(rotate)-rectMax.y*sin(rotate),
    posAbs.y+rectMin.x*sin(rotate)+rectMax.y*cos(rotate)
  );

  ImVec2 uv0=ImVec2(uvMin.x,uvMin.y);
  ImVec2 uv1=ImVec2(uvMax.x,uvMin.y);
  ImVec2 uv2=ImVec2(uvMax.x,uvMax.y);
  ImVec2 uv3=ImVec2(uvMin.x,uvMax.y);

  ImU32 colorConverted=ImGui::GetColorU32(imgColor);

  dl->AddImageQuad(img,quad0,quad1,quad2,quad3,uv0,uv1,uv2,uv3,colorConverted);
}

void FurnaceGUI::drawIntro() {
  if (introPos<9.0) {
    WAKE_UP;
    nextWindow=GUI_WINDOW_NOTHING;
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(canvasW,canvasH));
    ImGui::SetNextWindowFocus();
    if (ImGui::Begin("Intro",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoDocking|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground)) {
      ImDrawList* dl=ImGui::GetForegroundDrawList();
      ImVec2 top=ImVec2(0.0f,0.0f);
      ImVec2 bottom=ImVec2(canvasW,canvasH);

      // background
      float bgAlpha=CLAMP(9.0-introPos,0.0,1.0);
      bgAlpha=3.0*pow(bgAlpha,2.0)-2.0*pow(bgAlpha,3.0);
      ImU32 bgColor=ImGui::GetColorU32(ImVec4(0.0f,0.0f,0.0f,bgAlpha));

      dl->AddRectFilled(top,bottom,bgColor);

      // part 1 - talogo
      if (introPos<2.3) {
        drawImage(dl,GUI_IMAGE_TALOGO,ImVec2(0.5,0.5),ImVec2(0.7,0.7),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(1.0,1.0,1.0,MAX(0.01,1.0-pow(MAX(0.0,1.0-introPos*2.0),3.0))));

        for (int i=0; i<16; i++) {
          double chipCenter=0.25+pow(MAX(0.0,1.5-introPos*0.8-((double)i/36.0)),2.0)+pow(sin(-introPos*2.2-(double)i*0.44),24)*0.05;
          ImVec2 chipPos=ImVec2(
            0.5+chipCenter*cos(2.0*M_PI*(double)i/16.0-pow(introPos,2.2)),
            0.5+chipCenter*sin(2.0*M_PI*(double)i/16.0-pow(introPos,2.2))
          );
          drawImage(dl,GUI_IMAGE_TACHIP,chipPos,ImVec2(0.25,0.25),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,0.5),ImVec4(1.0,1.0,1.0,1.0));
        }
      }

      // transition
      float transitionPos=CLAMP(introPos*4.0-8,-1.5,3.5);
      dl->AddQuadFilled(
        ImVec2(
          (transitionPos-1.5)*canvasW,
          0.0
        ),
        ImVec2(
          (transitionPos)*canvasW,
          0.0
        ),
        ImVec2(
          (transitionPos-0.2)*canvasW,
          canvasH
        ),
        ImVec2(
          (transitionPos-1.7)*canvasW,
          canvasH
        ),
        ImGui::GetColorU32(ImVec4(0.35,0.4,0.5,1.0))
      );

      // part 2 - falling patterns
      if (introPos>2.3) {
        
      }

      // part 3 - falling chips
      if (introPos>3.0) {
        for (int i=0; i<40; i++) {
          float blah=(introPos-4.25)*1.3;
          ImVec2 chipPos=ImVec2(
            0.5+sin(i)*0.4,
            0.1-(1.1*pow(blah,2.0)-1.3*pow(blah,2.0)+pow(blah,5.0))+i*0.02+((introPos-3.75)*1.3*(fabs(sin(i*1.3))*0.28))
          );
          drawImage(dl,GUI_IMAGE_TACHIP,chipPos,ImVec2(0.33,0.33),0.5*M_PI,ImVec2(0.0,0.0),ImVec2(1.0,0.5),ImVec4(1.0,1.0,1.0,1.0));
        }
      }

      // part 4 - logo end
      if (introPos>5.0) {
        drawImage(dl,GUI_IMAGE_LOGO,ImVec2(0.5-0.25*(1.0-pow(1.0-CLAMP(introPos-6.0,0.0,1.0),6.0)),0.5+pow(1.0-CLAMP(introPos-5.0,0.0,1.0),4.0)),ImVec2(0.67,0.67),0.0f,ImVec2(0.0,0.0),ImVec2(1.0,1.0),ImVec4(1.0,1.0,1.0,bgAlpha));
      }

      dl->AddText(top,ImGui::GetColorU32(ImVec4(1.0f,1.0f,1.0f,bgAlpha)),"Furnace intro - work in progress");
    }
    ImGui::End();

    if (mustClear<=0) {
      introPos+=ImGui::GetIO().DeltaTime;
    }
  }
}
