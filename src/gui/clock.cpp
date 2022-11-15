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
#include "imgui_internal.h"
#include "imgui.h"

void FurnaceGUI::drawClock() {
  if (nextWindow==GUI_WINDOW_CLOCK) {
    clockOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!clockOpen) return;
  if (ImGui::Begin("Clock",&clockOpen,globalWinFlags)) {
    int row=e->getRow();
    int elapsedBars=e->getElapsedBars();
    int elapsedBeats=e->getElapsedBeats();
    bool playing=e->isPlaying();
    if (clockShowRow) {
      ImGui::PushFont(bigFont);
      ImGui::Text("%.3d:%.3d",e->getOrder(),row);
      ImGui::PopFont();
    }
    if (clockShowBeat) {
      ImGui::PushFont(bigFont);
      ImGui::Text("%.3d:%.1d",elapsedBars,elapsedBeats+1);
      ImGui::PopFont();
    }
    if (clockShowMetro) {
      ImDrawList* dl=ImGui::GetWindowDrawList();
      ImGuiWindow* window=ImGui::GetCurrentWindow();
      ImVec2 size=ImGui::GetContentRegionAvail();
      size.y=12.0f*dpiScale;

      ImVec2 minArea=window->DC.CursorPos;
      ImVec2 maxArea=ImVec2(
        minArea.x+size.x,
        minArea.y+size.y
      );
      ImRect rect=ImRect(minArea,maxArea);
      /*ImRect inRect=rect;
      inRect.Min.x+=dpiScale;
      inRect.Min.y+=dpiScale;
      inRect.Max.x-=dpiScale;
      inRect.Max.y-=dpiScale;*/
      ImGuiStyle& style=ImGui::GetStyle();
      ImGui::ItemSize(size,style.FramePadding.y);
      if (ImGui::ItemAdd(rect,ImGui::GetID("metroQ"))) {
        int h1=e->curSubSong->hilightA;
        int h2=e->curSubSong->hilightB;
        if (h1>0 && h2>0) {
          int beats=(h2+(h1-1))/h1;
          if (beats<0) beats=1;
          if (beats>16) beats=16;
          if (playing) {
            if (elapsedBeats!=oldBeat || elapsedBars!=oldBar) {
              if (elapsedBeats>15) elapsedBeats=15;
              clockMetroTick[elapsedBeats]=1.0f;
              oldBeat=elapsedBeats;
              oldBar=elapsedBars;
            }
          } else {
            oldBeat=-1;
            oldBar=-1;
          }
          for (int i=0; i<beats; i++) {
            ImVec2 minB=ImLerp(minArea,maxArea,ImVec2((float)i/(float)beats,0.0f));
            ImVec2 maxB=ImLerp(minArea,maxArea,ImVec2((float)(i+1)/(float)beats,1.0f));
            ImVec4 col=ImLerp(uiColors[GUI_COLOR_CLOCK_BEAT_LOW],uiColors[GUI_COLOR_CLOCK_BEAT_HIGH],clockMetroTick[i]);
            dl->AddQuadFilled(
              ImLerp(minB,maxB,ImVec2(0.35f,0.0f)),
              ImLerp(minB,maxB,ImVec2(0.9f,0.0f)),
              ImLerp(minB,maxB,ImVec2(0.65f,1.0f)),
              ImLerp(minB,maxB,ImVec2(0.1f,1.0f)),
              ImGui::GetColorU32(col)
            );

            if (elapsedBeats==i && playing) {
              clockMetroTick[i]-=0.1f*ImGui::GetIO().DeltaTime*60.0f;
              if (clockMetroTick[i]<0.3f) clockMetroTick[i]=0.3f;
            } else {
              clockMetroTick[i]-=0.1f*ImGui::GetIO().DeltaTime*60.0f;
              if (clockMetroTick[i]<0.0f) clockMetroTick[i]=0.0f;
            }
          }
        }
      }
    }
    if (clockShowTime) {
      int totalTicks=e->getTotalTicks();
      int totalSeconds=e->getTotalSeconds();
      ImGui::PushFont(bigFont);
      ImGui::Text("%.2d:%.2d.%.2d",(totalSeconds/60),totalSeconds%60,totalTicks/10000);
      ImGui::PopFont();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SPOILER;
  ImGui::End();
}
