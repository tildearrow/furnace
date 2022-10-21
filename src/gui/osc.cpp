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
#include <imgui.h>

void FurnaceGUI::readOsc() {
  int writePos=e->oscWritePos;
  int readPos=e->oscReadPos;
  int avail=0;
  int total=0;
  if (firstFrame) {
    readPos=writePos;
  }
  if (writePos>=readPos) {
    avail=writePos-readPos;
  } else {
    avail=writePos-readPos+32768;
  }
  if (oscTotal==0) {
    oscTotal=ImGui::GetIO().DeltaTime*e->getAudioDescGot().rate;
  } else {
    oscTotal=(oscTotal+(int)round(ImGui::GetIO().DeltaTime*e->getAudioDescGot().rate))>>1;
  }
  int bias=avail-oscTotal-e->getAudioDescGot().bufsize;
  if (bias<0) bias=0;
  total=oscTotal+(bias>>6);
  if (total>avail) total=avail;
  //printf("total: %d. avail: %d bias: %d\n",total,avail,bias);

  int winSize=e->getAudioDescGot().rate*(oscWindowSize/1000.0);
  int oscReadPos=(writePos-winSize)&0x7fff;
  for (int i=0; i<512; i++) {
    int pos=(oscReadPos+(i*winSize/512))&0x7fff;
    oscValues[i]=(e->oscBuf[0][pos]+e->oscBuf[1][pos])*0.5f;
    if (oscValues[i]>0.001f || oscValues[i]<-0.001f) {
      WAKE_UP;
    }
  }

  float peakDecay=0.05f*60.0f*ImGui::GetIO().DeltaTime;
  for (int i=0; i<2; i++) {
    peak[i]*=1.0-peakDecay;
    if (peak[i]<0.0001) {
      peak[i]=0.0;
    } else {
      WAKE_UP;
    }
    float newPeak=peak[i];
    for (int j=0; j<total; j++) {
      int pos=(readPos+j)&0x7fff;
      if (fabs(e->oscBuf[i][pos])>newPeak) {
        newPeak=fabs(e->oscBuf[i][pos]);
      }
    }
    peak[i]+=(newPeak-peak[i])*0.9;
  }

  readPos=(readPos+total)&0x7fff;
  e->oscReadPos=readPos;
}

void FurnaceGUI::drawOsc() {
  if (nextWindow==GUI_WINDOW_OSCILLOSCOPE) {
    oscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!oscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (settings.oscTakesEntireWindow) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));
  }
  if (ImGui::Begin("Oscilloscope",&oscOpen,globalWinFlags)) {
    if (oscZoomSlider) {
      if (ImGui::VSliderFloat("##OscZoom",ImVec2(20.0f*dpiScale,ImGui::GetContentRegionAvail().y),&oscZoom,0.5,2.0)) {
        if (oscZoom<0.5) oscZoom=0.5;
        if (oscZoom>2.0) oscZoom=2.0;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("zoom: %.2fx (%.1fdB)",oscZoom,20.0*log10(oscZoom*2.0));
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        oscZoom=0.5;
      }
      ImGui::SameLine();
      if (ImGui::VSliderFloat("##OscWinSize",ImVec2(20.0f*dpiScale,ImGui::GetContentRegionAvail().y),&oscWindowSize,5.0,100.0)) {
        if (oscWindowSize<5.0) oscWindowSize=5.0;
        if (oscWindowSize>100.0) oscWindowSize=100.0;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("window size: %.1fms",oscWindowSize);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        oscWindowSize=20.0;
      }
      ImGui::SameLine();
    }

    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImGuiWindow* window=ImGui::GetCurrentWindow();

    ImVec2 waveform[512];
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
    ImGuiStyle& style=ImGui::GetStyle();
    ImU32 color=ImGui::GetColorU32(isClipping?uiColors[GUI_COLOR_OSC_WAVE_PEAK]:uiColors[GUI_COLOR_OSC_WAVE]);
    ImU32 borderColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BORDER]);
    ImU32 refColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_REF]);
    ImU32 guideColor=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_GUIDE]);
    ImGui::ItemSize(size,style.FramePadding.y);
    if (ImGui::ItemAdd(rect,ImGui::GetID("wsDisplay"))) {
      dl->AddRectFilledMultiColor(
        inRect.Min,
        inRect.Max,
        ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG1]),
        ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG2]),
        ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG4]),
        ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG3]),
        settings.oscRoundedCorners?(8.0f*dpiScale):0.0f
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.0f,0.5f)),
        ImLerp(rect.Min,rect.Max,ImVec2(1.0f,0.5f)),
        refColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.48f,0.125f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.52f,0.125f)),
        guideColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.47f,0.25f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.53f,0.25f)),
        guideColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.45f,0.375f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.55f,0.375f)),
        guideColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.45f,0.625f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.55f,0.625f)),
        guideColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.47f,0.75f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.53f,0.75f)),
        guideColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.48f,0.875f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.52f,0.875f)),
        guideColor,
        dpiScale
      );

      dl->AddLine(
        ImLerp(rect.Min,rect.Max,ImVec2(0.5f,0.08f)),
        ImLerp(rect.Min,rect.Max,ImVec2(0.5f,0.92f)),
        guideColor,
        dpiScale
      );

      for (size_t i=0; i<512; i++) {
        float x=(float)i/512.0f;
        float y=oscValues[i]*oscZoom;
        if (!settings.oscEscapesBoundary) {
          if (y<-0.5f) y=-0.5f;
          if (y>0.5f) y=0.5f;
        }
        waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
      }
      if (settings.oscEscapesBoundary) {
        ImDrawList* dlf=ImGui::GetForegroundDrawList();
        dlf->AddPolyline(waveform,512,color,ImDrawFlags_None,dpiScale);
      } else {
        dl->AddPolyline(waveform,512,color,ImDrawFlags_None,dpiScale);
      }
      if (settings.oscBorder) {
        dl->AddRect(inRect.Min,inRect.Max,borderColor,settings.oscRoundedCorners?(8.0f*dpiScale):0.0f,0,1.5f*dpiScale);
      }
    }
    if (oscZoomSlider && ImGui::IsItemHovered()) {
      float val=20.0*log10(2.0*fabs(0.5-((ImGui::GetMousePos().y-inRect.Min.y)/(inRect.Max.y-inRect.Min.y))));
      if (val>0.0f) val=0.0f;
      if (val<=-INFINITY) {
        ImGui::SetTooltip("(-Infinity)dB");
      } else {
        ImGui::SetTooltip("%.1fdB",val);
      }
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      oscZoomSlider=!oscZoomSlider;
    }
  }
  if (settings.oscTakesEntireWindow) {
    ImGui::PopStyleVar(3);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_OSCILLOSCOPE;
  ImGui::End();
}
