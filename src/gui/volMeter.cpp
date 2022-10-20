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

void FurnaceGUI::drawVolMeter() {
  if (nextWindow==GUI_WINDOW_VOL_METER) {
    volMeterOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!volMeterOpen) return;
  if (--isClipping<0) isClipping=0;
  ImGui::SetNextWindowSizeConstraints(ImVec2(6.0f*dpiScale,6.0f*dpiScale),ImVec2(canvasW,canvasH));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));
  if (ImGui::Begin("Volume Meter",&volMeterOpen,globalWinFlags)) {
    ImDrawList* dl=ImGui::GetWindowDrawList();
    bool aspectRatio=(ImGui::GetWindowSize().x/ImGui::GetWindowSize().y)>1.0;

    ImVec2 minArea=ImVec2(
      ImGui::GetWindowPos().x+ImGui::GetCursorPos().x,
      ImGui::GetWindowPos().y+ImGui::GetCursorPos().y
    );
    ImVec2 maxArea=ImVec2(
      ImGui::GetWindowPos().x+ImGui::GetCursorPos().x+ImGui::GetContentRegionAvail().x,
      ImGui::GetWindowPos().y+ImGui::GetCursorPos().y+ImGui::GetContentRegionAvail().y
    );
    ImRect rect=ImRect(minArea,maxArea);
    ImGuiStyle& style=ImGui::GetStyle();
    ImGui::ItemSize(ImVec2(4.0f,4.0f),style.FramePadding.y);
    ImU32 lowColor=ImGui::GetColorU32(uiColors[GUI_COLOR_VOLMETER_LOW]);
    if (ImGui::ItemAdd(rect,ImGui::GetID("volMeter"))) {
      ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
      for (int i=0; i<2; i++) {
        float logPeak=(20*log10(peak[i])/36.0);
        if (logPeak==NAN) logPeak=0.0;
        if (logPeak<-1.0) logPeak=-1.0;
        if (logPeak>0.0) {
          isClipping=8;
          logPeak=0.0;
        }
        logPeak+=1.0;
        ImU32 highColor=ImGui::GetColorU32(
          ImLerp(uiColors[GUI_COLOR_VOLMETER_LOW],uiColors[GUI_COLOR_VOLMETER_HIGH],logPeak)
        );
        ImRect s;
        if (aspectRatio) {
          s=ImRect(
            ImLerp(rect.Min,rect.Max,ImVec2(0,float(i)*0.5)),
            ImLerp(rect.Min,rect.Max,ImVec2(logPeak,float(i+1)*0.5))
          );
          if (i==0) s.Max.y-=dpiScale;
          if (isClipping) {
            dl->AddRectFilled(s.Min,s.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_VOLMETER_PEAK]));
          } else {
            dl->AddRectFilledMultiColor(s.Min,s.Max,lowColor,highColor,highColor,lowColor);
          }
        } else {
          s=ImRect(
            ImLerp(rect.Min,rect.Max,ImVec2(float(i)*0.5,1.0-logPeak)),
            ImLerp(rect.Min,rect.Max,ImVec2(float(i+1)*0.5,1.0))
          );
          if (i==0) s.Max.x-=dpiScale;
          if (isClipping) {
            dl->AddRectFilled(s.Min,s.Max,ImGui::GetColorU32(uiColors[GUI_COLOR_VOLMETER_PEAK]));
          } else {
            dl->AddRectFilledMultiColor(s.Min,s.Max,highColor,highColor,lowColor,lowColor);
          }
        }
      }
      if (ImGui::IsItemHovered()) {
        if (aspectRatio) {
          ImGui::SetTooltip("%.1fdB",36*((ImGui::GetMousePos().x-ImGui::GetItemRectMin().x)/(rect.Max.x-rect.Min.x)-1.0));
        } else {
          ImGui::SetTooltip("%.1fdB",-(36+36*((ImGui::GetMousePos().y-ImGui::GetItemRectMin().y)/(rect.Max.y-rect.Min.y)-1.0)));
        }
      }
    }
  }
  ImGui::PopStyleVar(4);
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_VOL_METER;
  ImGui::End();
}
