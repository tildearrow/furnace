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
#include "imgui.h"
#include "imgui_internal.h"

void FurnaceGUI::drawChanOsc() {
  if (nextWindow==GUI_WINDOW_CHAN_OSC) {
    chanOscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!chanOscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Oscilloscope (per-channel)",&chanOscOpen)) {
    if (ImGui::InputInt("Columns",&chanOscCols,1,1)) {
      if (chanOscCols<1) chanOscCols=1;
      if (chanOscCols>64) chanOscCols=64;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    float availY=ImGui::GetContentRegionAvail().y;
    if (ImGui::BeginTable("ChanOsc",chanOscCols,ImGuiTableFlags_Borders)) {
      int chans=e->getTotalChannelCount();
      int rows=(chans+(chanOscCols-1))/chanOscCols;
      ImDrawList* dl=ImGui::GetWindowDrawList();
      ImGuiWindow* window=ImGui::GetCurrentWindow();
      ImVec2 waveform[512];

      ImGuiStyle& style=ImGui::GetStyle();
      ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_WAVE]);

      for (int i=0; i<chans; i++) {
        if (i%chanOscCols==0) ImGui::TableNextRow();
        ImGui::TableNextColumn();

        DivDispatchOscBuffer* buf=e->getOscBuffer(i);
        if (buf==NULL) {
          ImGui::Text("Not Available");
        } else {
          ImVec2 size=ImGui::GetContentRegionAvail();
          size.y=availY/rows;

          int displaySize=(buf->rate)/30;

          ImVec2 minArea=window->DC.CursorPos;
          ImVec2 maxArea=ImVec2(
            minArea.x+size.x,
            minArea.y+size.y
          );
          ImRect rect=ImRect(minArea,maxArea);
          ImRect inRect=rect;
          inRect.Min.x+=dpiScale;
          inRect.Min.y+=dpiScale;
          inRect.Max.x-=dpiScale;
          inRect.Max.y-=dpiScale;
          ImGui::ItemSize(size,style.FramePadding.y);
          if (ImGui::ItemAdd(rect,ImGui::GetID("chOscDisplay"))) {
            if (!e->isPlaying()) {
              for (unsigned short i=0; i<512; i++) {
                float x=(float)i/512.0f;
                waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f));
              }
            } else {
              unsigned short needlePos=buf->needle-displaySize;
              for (unsigned short i=0; i<512; i++) {
                float x=(float)i/512.0f;
                float y=(float)buf->data[(unsigned short)(needlePos+(i*displaySize/512))]/65536.0f;
                if (y<-0.5f) y=-0.5f;
                if (y>0.5f) y=0.5f;
                waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
              }
            }
            dl->AddPolyline(waveform,512,color,ImDrawFlags_None,dpiScale);
          }
        }
      }
      ImGui::EndTable();
    }
    ImGui::PopStyleVar();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHAN_OSC;
  ImGui::End();
}