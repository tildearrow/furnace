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
#include "imgui_internal.h"
#include <imgui.h>
#include "../ta-log.h"
#include "../engine/filter.h"

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

  for (int ch=0; ch<e->getAudioDescGot().outChans; ch++) {
    if (oscValues[ch]==NULL) {
      oscValues[ch]=new float[2048];
    }
    memset(oscValues[ch],0,2048*sizeof(float));
    float* sincITable=DivFilterTables::getSincIntegralSmallTable();

    float posFrac=0.0;
    float factor=(float)oscWidth/(float)winSize;
    int posInt=oscReadPos-(8.0f/factor);
    for (int i=7; i<oscWidth-9; i++) {
      oscValues[ch][i]+=e->oscBuf[ch][posInt&0x7fff];

      posFrac+=1.0;
      while (posFrac>=1.0) {
        unsigned int n=((unsigned int)(posFrac*64.0))&63;
        posFrac-=factor;
        posInt++;

        float* t1=&sincITable[(63-n)<<3];
        float* t2=&sincITable[n<<3];
        float delta=e->oscBuf[ch][posInt&0x7fff]-e->oscBuf[ch][(posInt-1)&0x7fff];

        oscValues[ch][i-7]+=t1[7]*-delta;
        oscValues[ch][i-6]+=t1[6]*-delta;
        oscValues[ch][i-5]+=t1[5]*-delta;
        oscValues[ch][i-4]+=t1[4]*-delta;
        oscValues[ch][i-3]+=t1[3]*-delta;
        oscValues[ch][i-2]+=t1[2]*-delta;
        oscValues[ch][i-1]+=t1[1]*-delta;
        oscValues[ch][i]  +=t1[0]*-delta;

        oscValues[ch][i+1]+=t2[0]*delta;
        oscValues[ch][i+2]+=t2[1]*delta;
        oscValues[ch][i+3]+=t2[2]*delta;
        oscValues[ch][i+4]+=t2[3]*delta;
        oscValues[ch][i+5]+=t2[4]*delta;
        oscValues[ch][i+6]+=t2[5]*delta;
        oscValues[ch][i+7]+=t2[6]*delta;
        oscValues[ch][i+8]+=t2[7]*delta;
      }
    }

    for (int i=0; i<oscWidth; i++) {
      if (oscValues[ch][i]>0.001f || oscValues[ch][i]<-0.001f) {
        WAKE_UP;
      }
    }
  }

  if (oscValuesAverage==NULL) {
    oscValuesAverage=new float[2048];
  }
  memset(oscValuesAverage,0,2048*sizeof(float));
  for (int i=0; i<oscWidth; i++) {
    float avg=0;
    for (int j=0; j<e->getAudioDescGot().outChans; j++) {
      avg+=oscValues[j][i];
    }
    avg/=e->getAudioDescGot().outChans;
    oscValuesAverage[i]=avg*oscZoom*2.0f;
  }

  /*for (int i=0; i<oscWidth; i++) {
    oscValues[i]=(i&1)?0.3:0;
  }*/

  float peakDecay=0.05f*60.0f*ImGui::GetIO().DeltaTime;
  for (int i=0; i<e->getAudioDescGot().outChans; i++) {
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

PendingDrawOsc _do;

static void _drawOsc(const ImDrawList* drawList, const ImDrawCmd* cmd) {
  if (cmd!=NULL) {
    if (cmd->UserCallbackData!=NULL) {
      ((FurnaceGUI*)(((PendingDrawOsc*)cmd->UserCallbackData)->gui))->runPendingDrawOsc((PendingDrawOsc*)cmd->UserCallbackData);
    }
  }
}

void FurnaceGUI::runPendingDrawOsc(PendingDrawOsc* which) {
  rend->drawOsc(which->data,which->len,which->pos0,which->pos1,which->color,ImVec2(canvasW,canvasH),which->lineSize);
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
  if (ImGui::Begin("Oscilloscope",&oscOpen,globalWinFlags,_("Oscilloscope"))) {
    if (oscZoomSlider) {
      if (ImGui::VSliderFloat("##OscZoom",ImVec2(20.0f*dpiScale,ImGui::GetContentRegionAvail().y),&oscZoom,0.5,2.0)) {
        if (oscZoom<0.5) oscZoom=0.5;
        if (oscZoom>2.0) oscZoom=2.0;
      } rightClickable
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("zoom: %.2fx (%.1fdB)"),oscZoom,20.0*log10(oscZoom*2.0));
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        oscZoom=0.5;
      }
      ImGui::SameLine();
      if (ImGui::VSliderFloat("##OscWinSize",ImVec2(20.0f*dpiScale,ImGui::GetContentRegionAvail().y),&oscWindowSize,5.0,100.0)) {
        if (oscWindowSize<5.0) oscWindowSize=5.0;
        if (oscWindowSize>100.0) oscWindowSize=100.0;
      } rightClickable
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("window size: %.1fms"),oscWindowSize);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        oscWindowSize=20.0;
      }
      ImGui::SameLine();
    }

    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImGuiWindow* window=ImGui::GetCurrentWindow();

    static ImVec2 waveform[2048];
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
      if (safeMode || renderBackend==GUI_BACKEND_SOFTWARE) {
        dl->AddRectFilled(
          inRect.Min,
          inRect.Max,
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG4])
        );
      } else {
        dl->AddRectFilledMultiColor(
          inRect.Min,
          inRect.Max,
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG1]),
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG2]),
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG4]),
          ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_BG3]),
          settings.oscRoundedCorners?(8.0f*dpiScale):0.0f
        );
      }

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

      oscWidth=round(inRect.Max.x-inRect.Min.x)+24;
      if (oscWidth<17) oscWidth=17;
      if (oscWidth>2048) oscWidth=2048;

      ImDrawListFlags prevFlags=dl->Flags;
      if (!settings.oscAntiAlias || safeMode) {
        dl->Flags&=~(ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex);
      }

      if ((oscWidth-24)>0) {
        if (settings.oscMono) {
          if (rend->supportsDrawOsc() && settings.shaderOsc) {
            _do.gui=this;
            _do.data=&oscValuesAverage[12];
            _do.len=oscWidth-24;
            _do.pos0=inRect.Min;
            _do.pos1=inRect.Max;
            _do.color=isClipping?uiColors[GUI_COLOR_OSC_WAVE_PEAK]:uiColors[GUI_COLOR_OSC_WAVE];
            _do.lineSize=dpiScale*settings.oscLineSize;

            dl->AddCallback(_drawOsc,&_do);
            dl->AddCallback(ImDrawCallback_ResetRenderState,NULL);
          } else {
            for (int i=0; i<oscWidth-24; i++) {
              float x=(float)i/(float)(oscWidth-24);
              float y=oscValuesAverage[i+12]*0.5f;
              if (!settings.oscEscapesBoundary) {
                if (y<-0.5f) y=-0.5f;
                if (y>0.5f) y=0.5f;
              }
              waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
            }

            if (settings.oscEscapesBoundary) {
              dl->PushClipRectFullScreen();
              dl->AddPolyline(waveform,oscWidth-24,color,ImDrawFlags_None,dpiScale*settings.oscLineSize);
              dl->PopClipRect();
            } else {
              dl->AddPolyline(waveform,oscWidth-24,color,ImDrawFlags_None,dpiScale*settings.oscLineSize);
            }
          }
        } else {
          for (int ch=0; ch<e->getAudioDescGot().outChans; ch++) {
            if (!isClipping) {
              color=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_WAVE_CH0+ch]);
            }

            if (rend->supportsDrawOsc() && settings.shaderOsc) {
              _do.gui=this;
              _do.data=&oscValues[ch][12];
              _do.len=oscWidth-24;
              _do.pos0=inRect.Min;
              _do.pos1=inRect.Max;
              _do.color=isClipping?uiColors[GUI_COLOR_OSC_WAVE_PEAK]:uiColors[GUI_COLOR_OSC_WAVE_CH0+ch];
              _do.lineSize=dpiScale*settings.oscLineSize;

              dl->AddCallback(_drawOsc,&_do);
              dl->AddCallback(ImDrawCallback_ResetRenderState,NULL);
            } else {
              for (int i=0; i<oscWidth-24; i++) {
                float x=(float)i/(float)(oscWidth-24);
                float y=oscValues[ch][i+12]*oscZoom;
                if (!settings.oscEscapesBoundary) {
                  if (y<-0.5f) y=-0.5f;
                  if (y>0.5f) y=0.5f;
                }
                waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
              }

              
              if (settings.oscEscapesBoundary) {
                dl->PushClipRectFullScreen();
                dl->AddPolyline(waveform,oscWidth-24,color,ImDrawFlags_None,dpiScale*settings.oscLineSize);
                dl->PopClipRect();
              } else {
                dl->AddPolyline(waveform,oscWidth-24,color,ImDrawFlags_None,dpiScale*settings.oscLineSize);
              }
            }
          }
        }
      }

      dl->Flags=prevFlags;
      
      if (settings.oscBorder) {
        dl->AddRect(inRect.Min,inRect.Max,borderColor,(settings.oscRoundedCorners && !(safeMode || renderBackend==GUI_BACKEND_SOFTWARE))?(8.0f*dpiScale):0.0f,0,1.5f*dpiScale);
      }
    }
    if (oscZoomSlider && ImGui::IsItemHovered()) {
      float val=20.0*log10(2.0*fabs(0.5-((ImGui::GetMousePos().y-inRect.Min.y)/(inRect.Max.y-inRect.Min.y))));
      if (val>0.0f) val=0.0f;
      if (val<=-INFINITY) {
        ImGui::SetTooltip(_("(-Infinity)dB"));
      } else {
        ImGui::SetTooltip("%.1fdB",val);
      }
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      oscZoomSlider=!oscZoomSlider;
    }
    if (mobileUI && ImGui::IsItemHovered() && CHECK_LONG_HOLD) {
      oscZoomSlider=!oscZoomSlider;
      NOTIFY_LONG_HOLD;
    }
  }
  if (settings.oscTakesEntireWindow) {
    ImGui::PopStyleVar(3);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_OSCILLOSCOPE;
  ImGui::End();
}
