/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

void FurnaceGUI::drawXYOsc() {
  if (nextWindow==GUI_WINDOW_XY_OSC) {
    xyOscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!xyOscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(128.0f*dpiScale,128.0f*dpiScale),ImVec2(canvasW,canvasH));
  bool noPadding=settings.oscTakesEntireWindow && !xyOscOptions;
  if (noPadding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));
  }
  if (ImGui::Begin("Oscilloscope (X-Y)",&xyOscOpen,globalWinFlags,_("Oscilloscope (X-Y)"))) {
    if (xyOscOptions) {
      int xyOscXChannelP1=xyOscXChannel+1;
      int xyOscYChannelP1=xyOscYChannel+1;

      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("X Channel"));
      ImGui::SameLine();
      if (ImGui::InputInt("##XChannel",&xyOscXChannelP1)) {
        xyOscXChannel=MIN(MAX(xyOscXChannelP1,1),DIV_MAX_OUTPUTS)-1;
      } rightClickable
      ImGui::SameLine();
      ImGui::Checkbox(_("Invert##X"),&xyOscXInvert);
      ImGui::AlignTextToFramePadding();
      ImGui::Text(_("Y Channel"));
      ImGui::SameLine();
      if (ImGui::InputInt("##YChannel",&xyOscYChannelP1)) {
        xyOscYChannel=MIN(MAX(xyOscYChannelP1,1),DIV_MAX_OUTPUTS)-1;
      } rightClickable
      ImGui::SameLine();
      ImGui::Checkbox(_("Invert##Y"),&xyOscYInvert);
      if (ImGui::SliderFloat(_("Zoom"),&xyOscZoom,0.5f,4.0f,"%.2fx")) {
        xyOscZoom=MAX(xyOscZoom,0.0f);
      } rightClickable
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%.1fdB",20.0f*log10f(xyOscZoom));
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        xyOscZoom=1.0f;
      }
      if (ImGui::SliderInt(_("Samples"),&xyOscSamples,2,32768)) {
        xyOscSamples=MIN(MAX(xyOscSamples,2),32768);
      } rightClickable
      if (ImGui::SliderFloat(_("Decay Time (ms)"),&xyOscDecayTime,1.0f,1000.0f,"%.1f",ImGuiSliderFlags_Logarithmic)) {
        xyOscDecayTime=MAX(xyOscDecayTime,0.0f);
      } rightClickable
      if (ImGui::SliderFloat(_("Intensity"),&xyOscIntensity,0.0f,5.0f,"%.2f")) {
        xyOscIntensity=MAX(xyOscIntensity,0.0f);
      } rightClickable
      if (ImGui::SliderFloat(_("Line Thickness"),&xyOscThickness,0.0f,10.0f,"%.2f")) {
        xyOscThickness=MAX(xyOscThickness,0.0f);
      } rightClickable
      if (ImGui::Button(_("OK"))) {
        xyOscOptions=false;
      }
    } else {
      ImDrawList* dl=ImGui::GetWindowDrawList();
      ImGuiWindow* window=ImGui::GetCurrentWindow();
      ImVec2 size=ImGui::GetContentRegionAvail();

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
      ImVec2 inSqrCenter=rect.GetCenter();
      float inSqrLength;
      ImRect inSqr=inRect;
      if (rect.GetWidth() > rect.GetHeight()) {
        inSqrLength=inSqr.GetHeight()/2;
        inSqr.Min.x=inSqrCenter.x-inSqrLength;
        inSqr.Max.x=inSqrCenter.x+inSqrLength;
      } else {
        inSqrLength=inSqr.GetWidth()/2;
        inSqr.Min.y=inSqrCenter.y-inSqrLength;
        inSqr.Max.y=inSqrCenter.y+inSqrLength;
      }
      float scaleX=xyOscZoom*inSqrLength*(xyOscXInvert?-1:1);
      float scaleY=xyOscZoom*inSqrLength*(xyOscYInvert?1:-1);
      const ImGuiStyle& style=ImGui::GetStyle();
      ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_WAVE]);
      color&=~IM_COL32_A_MASK;
      ImU32 borderColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BORDER]);
      ImU32 refColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_REF]);
      ImU32 guideColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_GUIDE]);
      ImGui::ItemSize(size,style.FramePadding.y);
      if (ImGui::ItemAdd(rect,ImGui::GetID("wsDisplay"))) {
        // background
        dl->AddRectFilledMultiColor(
          inRect.Min,
          inRect.Max,
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG1]),
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG2]),
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG4]),
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG3]),
          settings.oscRoundedCorners?(8.0f*dpiScale):0.0f
        );

        // axis guides
        dl->AddLine(
          ImLerp(rect.Min,rect.Max,ImVec2(0.0f,0.5f)),
          ImLerp(rect.Min,rect.Max,ImVec2(1.0f,0.5f)),
          refColor,
          dpiScale
        );

        dl->AddLine(
          ImLerp(rect.Min,rect.Max,ImVec2(0.5f,0.0f)),
          ImLerp(rect.Min,rect.Max,ImVec2(0.5f,1.0f)),
          refColor,
          dpiScale
        );

        bool reflect=xyOscXInvert!=xyOscYInvert;
        dl->AddLine(
          ImLerp(inSqr.Min,inSqr.Max,ImVec2(0.0f,reflect?0.0f:1.0f)),
          ImLerp(inSqr.Min,inSqr.Max,ImVec2(1.0f,reflect?1.0f:0.0f)),
          refColor,
          dpiScale
        );

        for (int i=1; i<5; i++) {
          float ip = (float)i/8.f;
          dl->AddRect(
            ImLerp(inSqr.Min,inSqr.Max,ImVec2(0.5f-ip,0.5f-ip)),
            ImLerp(inSqr.Min,inSqr.Max,ImVec2(0.5f+ip,0.5f+ip)),
            guideColor,
            0.0f,0,dpiScale
          );
        }

        // line
        const float* oscBufX=e->oscBuf[xyOscXChannel];
        const float* oscBufY=e->oscBuf[xyOscYChannel];
        if (oscBufX!=NULL && oscBufY!=NULL) {
          int pos=e->oscWritePos;
          float lx=inSqrCenter.x;
          float ly=inSqrCenter.y;
          float maxA=xyOscIntensity*256.f;
          float decay=exp2f(-1e3f/e->getAudioDescGot().rate/xyOscDecayTime);
          ImDrawListFlags prevFlags=dl->Flags;
          dl->Flags|=ImDrawFlags_RoundCornersNone;
          if (safeMode) {
            dl->Flags&=~(ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex);
          }
          if (settings.oscEscapesBoundary) {
            dl->PushClipRectFullScreen();
          }
          for (int i=0; i<xyOscSamples; i++) {
            pos=(pos-1)&32767;
            float x=oscBufX[pos]*scaleX+inSqrCenter.x;
            float y=oscBufY[pos]*scaleY+inSqrCenter.y;
            if (i != 0) {
              float a=maxA/sqrtf((x-lx)*(x-lx)+(y-ly)*(y-ly));
              if (a>=1) {
                a=MIN(a,255);
                dl->AddLine(ImVec2(lx,ly),ImVec2(x,y),(color|((ImU32)a<<IM_COL32_A_SHIFT)),xyOscThickness*dpiScale);
              }
              maxA*=decay;
              if (maxA<1) {
                break;
              }
            }
            lx=x;
            ly=y;
          }
          if (settings.oscEscapesBoundary) {
            dl->PopClipRect();
          }
          dl->Flags=prevFlags;
        }
        if (settings.oscBorder) {
          dl->AddRect(inRect.Min,inRect.Max,borderColor,settings.oscRoundedCorners?(8.0f*dpiScale):0.0f,0,1.5f*dpiScale);
        }
      }
      if (ImGui::IsItemHovered()) {
        float valX=20.0f*log10f(fabsf((ImGui::GetMousePos().x-inSqrCenter.x)/scaleX));
        float valY=20.0f*log10f(fabsf((ImGui::GetMousePos().y-inSqrCenter.y)/scaleY));
        if (valX<=-INFINITY && valY<=-INFINITY) {
          ImGui::SetTooltip(_("(-Infinity)dB,(-Infinity)dB"));
        } else if (valX<=-INFINITY) {
          ImGui::SetTooltip(_("(-Infinity)dB,%.1fdB"),valY);
        } else if (valY<=-INFINITY) {
          ImGui::SetTooltip(_("%.1fdB,(-Infinity)dB"),valY);
        } else {
          ImGui::SetTooltip(_("%.1fdB,%.1fdB"),valX,valY);
        }
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        xyOscOptions=true;
      }
      if (ImGui::IsItemHovered() && CHECK_LONG_HOLD) {
        NOTIFY_LONG_HOLD;
        xyOscOptions=true;
      }
    }
  }
  if (noPadding) {
    ImGui::PopStyleVar(3);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_XY_OSC;
  ImGui::End();
}
