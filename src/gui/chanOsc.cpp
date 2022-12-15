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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_internal.h"

#define FURNACE_FFT_SIZE 4096
#define FURNACE_FFT_RATE 80.0
#define FURNACE_FFT_CUTOFF 0.1

const char* chanOscRefs[]={
  "None (0%)",
  "None (50%)",
  "None (100%)",

  "Frequency",
  "Volume",
  "Channel",
  "Brightness",

  "Note Trigger"
};

float FurnaceGUI::computeGradPos(int type, int chan) {
  switch (type) {
    case GUI_OSCREF_NONE:
      return 0.0f;
      break;
    case GUI_OSCREF_CENTER:
      return 0.5f;
      break;
    case GUI_OSCREF_MAX:
      return 1.0f;
      break;
    case GUI_OSCREF_FREQUENCY:
      return chanOscPitch[chan];
      break;
    case GUI_OSCREF_VOLUME:
      return chanOscVol[chan];
      break;
    case GUI_OSCREF_CHANNEL:
      return (float)chan/(float)(e->getTotalChannelCount()-1);
      break;
    case GUI_OSCREF_BRIGHT:
      return chanOscBright[chan];
      break;
    case GUI_OSCREF_NOTE_TRIGGER:
      return keyHit1[chan];
      break;
  }
  return 0.0f;
}

void FurnaceGUI::calcChanOsc() {
  std::vector<DivDispatchOscBuffer*> oscBufs;
  std::vector<ChanOscStatus*> oscFFTs;
  std::vector<int> oscChans;

  int chans=e->getTotalChannelCount();
  
  for (int i=0; i<chans; i++) {
    int tryAgain=i;
    DivDispatchOscBuffer* buf=e->getOscBuffer(i);
    while (buf==NULL) {
      if (--tryAgain<0) break;
      buf=e->getOscBuffer(tryAgain);
    }
    if (buf!=NULL && e->curSubSong->chanShow[i]) {
      // 30ms should be enough
      int displaySize=(float)(buf->rate)*0.03f;
      if (e->isRunning()) {
        float minLevel=1.0f;
        float maxLevel=-1.0f;
        unsigned short needlePos=buf->needle;
        needlePos-=displaySize;
        for (unsigned short i=0; i<512; i++) {
          float y=(float)buf->data[(unsigned short)(needlePos+(i*displaySize/512))]/65536.0f;
          if (minLevel>y) minLevel=y;
          if (maxLevel<y) maxLevel=y;
        }
        float estimate=pow(maxLevel-minLevel,0.5f);
        if (estimate>1.0f) estimate=1.0f;
        chanOscVol[i]=MAX(chanOscVol[i]*0.87f,estimate);
      }
    } else {
      chanOscVol[i]=MAX(chanOscVol[i]*0.87f,0.0f);
    }
    if (chanOscVol[i]<0.00001f) chanOscVol[i]=0.0f;
  }
}

void FurnaceGUI::drawChanOsc() {
  if (nextWindow==GUI_WINDOW_CHAN_OSC) {
    chanOscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!chanOscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Oscilloscope (per-channel)",&chanOscOpen,globalWinFlags|((chanOscOptions)?0:ImGuiWindowFlags_NoTitleBar))) {
    bool centerSettingReset=false;
    ImDrawList* dl=ImGui::GetWindowDrawList();
    if (chanOscOptions) {
      if (ImGui::BeginTable("ChanOscSettings",3)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Columns");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##COSColumns",&chanOscCols,1,1)) {
          if (chanOscCols<1) chanOscCols=1;
          if (chanOscCols>64) chanOscCols=64;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Size (ms)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputFloat("##COSWinSize",&chanOscWindowSize,1.0f,1.0f)) {
          if (chanOscWindowSize<1.0f) chanOscWindowSize=1.0f;
          if (chanOscWindowSize>50.0f) chanOscWindowSize=50.0f;
        }

        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Center waveform",&chanOscWaveCorr)) {
          centerSettingReset=true;
        }

        ImGui::EndTable();
      }

      ImGui::Checkbox("Gradient",&chanOscUseGrad);

      if (chanOscUseGrad) {
        if (chanOscGradTex==NULL) {
          chanOscGradTex=SDL_CreateTexture(sdlRend,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STREAMING,chanOscGrad.width,chanOscGrad.height);

          if (chanOscGradTex==NULL) {
            logE("error while creating gradient texture! %s",SDL_GetError());
          } else {
            updateChanOscGradTex=true;
          }
        }

        if (ImGui::BeginTable("ChanOscGradSet",2)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (chanOscGradTex!=NULL) {
            if (updateChanOscGradTex) {
              chanOscGrad.render();
              if (SDL_UpdateTexture(chanOscGradTex,NULL,chanOscGrad.grad.get(),chanOscGrad.width*4)==0) {
                updateChanOscGradTex=false;
              } else {
                logE("error while updating gradient texture! %s",SDL_GetError());
              }
            }

            ImVec2 gradLeft=ImGui::GetCursorPos();
            ImVec2 gradSize=ImVec2(400.0f*dpiScale,400.0f*dpiScale);
            ImGui::Image(chanOscGradTex,gradSize);
            ImVec2 gradLeftAbs=ImGui::GetItemRectMin();
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
              if (chanOscGrad.points.size()<32) {
                chanOscGrad.points.push_back(Gradient2DPoint(
                  (ImGui::GetMousePos().x-gradLeftAbs.x)/gradSize.x,
                  (ImGui::GetMousePos().y-gradLeftAbs.y)/gradSize.y
                ));
                updateChanOscGradTex=true;
              }
            }

            ImVec2 oldCurPos=ImGui::GetCursorPos();
            int index=0;
            int removePoint=-1;
            for (Gradient2DPoint& i: chanOscGrad.points) {
              ImGui::PushID(index+16);
              ImGui::SetCursorPos(ImVec2(gradLeft.x+i.x*gradSize.x-8.0*dpiScale,gradLeft.y+i.y*gradSize.y-8.0*dpiScale));
              if (ImGui::InvisibleButton("gradPoint",ImVec2(16.0*dpiScale,16.0*dpiScale))) {
                if (!i.grab) {
                  ImGui::OpenPopup("gradPointSettings");
                }
              }
              if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
                ImGui::SetTooltip("(%.1f, %.1f)",i.x*100.0f,(1.0f-i.y)*100.0f);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
                removePoint=index;
              }
              if (ImGui::IsItemActive()) {
                float mX=(ImGui::GetMousePos().x-gradLeftAbs.x)/gradSize.x;
                float mY=(ImGui::GetMousePos().y-gradLeftAbs.y)/gradSize.y;

                if (i.grab || (fabs(i.x-mX)>0.015 || fabs(i.y-mY)>0.015)) {
                  i.x=mX;
                  i.y=mY;
                  i.grab=true;

                  if (i.x<0) i.x=0;
                  if (i.x>1) i.x=1;
                  if (i.y<0) i.y=0;
                  if (i.y>1) i.y=1;
                  updateChanOscGradTex=true;
                }
              } else {
                i.grab=false;
                i.prevX=i.x;
                i.prevY=i.y;
              }
              if (ImGui::BeginPopup("gradPointSettings",ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize)) {
                if (ImGui::ColorPicker4("Color",(float*)&i.color)) {
                  updateChanOscGradTex=true;
                }
                ImGui::Text("Distance");
                ImGui::SameLine();
                float pDist=i.distance*100.0f;
                if (ImGui::SliderFloat("##PDistance",&pDist,0.0f,150.0f,"%.1f%%")) {
                  i.distance=pDist/100.0f;
                  updateChanOscGradTex=true;
                }

                ImGui::Text("Spread");
                ImGui::SameLine();
                float pSpread=i.spread*100.0f;
                if (ImGui::SliderFloat("##PSpread",&pSpread,0.0f,150.0f,"%.1f%%")) {
                  i.spread=pSpread/100.0f;
                  updateChanOscGradTex=true;
                }

                if (ImGui::Button("Remove")) {
                  removePoint=index;
                  ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
              }

              dl->AddCircle(ImVec2(gradLeftAbs.x+i.x*gradSize.x,gradLeftAbs.y+i.y*gradSize.y),8.0*dpiScale,ImGui::ColorConvertFloat4ToU32(ImVec4(0.5,0.5,0.5,1.0)),6,2.0f*dpiScale);
              dl->AddCircle(ImVec2(gradLeftAbs.x+i.x*gradSize.x,gradLeftAbs.y+i.y*gradSize.y),5.0*dpiScale,ImGui::ColorConvertFloat4ToU32(ImVec4(0.1,0.1,0.1,1.0)),6,2.0f*dpiScale);

              ImGui::PopID();
              index++;
            }
            ImGui::SetCursorPos(oldCurPos);

            if (removePoint>=0) {
              chanOscGrad.points.erase(chanOscGrad.points.begin()+removePoint);
              updateChanOscGradTex=true;
            }
          }

          ImGui::TableNextColumn();
          if (ImGui::ColorEdit4("Background",(float*)&chanOscGrad.bgColor)) {
            updateChanOscGradTex=true;
          }
          ImGui::Combo("X Axis##AxisX",&chanOscColorX,chanOscRefs,GUI_OSCREF_MAX);
          ImGui::Combo("Y Axis##AxisY",&chanOscColorY,chanOscRefs,GUI_OSCREF_MAX);

          ImGui::EndTable();
        }
      } else {
        ImGui::SetNextItemWidth(400.0f*dpiScale);
        ImGui::ColorPicker4("Color",(float*)&chanOscColor);
      }

      if (ImGui::Button("OK")) {
        chanOscOptions=false;
      }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    float availY=ImGui::GetContentRegionAvail().y;
    if (ImGui::BeginTable("ChanOsc",chanOscCols,ImGuiTableFlags_Borders)) {
      std::vector<DivDispatchOscBuffer*> oscBufs;
      std::vector<ChanOscStatus*> oscFFTs;
      std::vector<int> oscChans;
      int chans=e->getTotalChannelCount();
      ImGuiWindow* window=ImGui::GetCurrentWindow();
      ImVec2 waveform[512];

      ImGuiStyle& style=ImGui::GetStyle();

      for (int i=0; i<chans; i++) {
        DivDispatchOscBuffer* buf=e->getOscBuffer(i);
        if (buf!=NULL && e->curSubSong->chanShow[i]) {
          oscBufs.push_back(buf);
          oscFFTs.push_back(&chanOscChan[i]);
          oscChans.push_back(i);
        }
      }
      int rows=(oscBufs.size()+(chanOscCols-1))/chanOscCols;

      for (size_t i=0; i<oscBufs.size(); i++) {
        if (i%chanOscCols==0) ImGui::TableNextRow();
        ImGui::TableNextColumn();

        DivDispatchOscBuffer* buf=oscBufs[i];
        ChanOscStatus* fft=oscFFTs[i];
        int ch=oscChans[i];
        if (buf==NULL) {
          ImGui::Text("Error!");
        } else {
          ImVec2 size=ImGui::GetContentRegionAvail();
          size.y=availY/rows;

          if (centerSettingReset) {
            buf->readNeedle=buf->needle;
          }

          // check FFT status existence
          if (fft->plan==NULL) {
            logD("creating FFT plan for channel %d",ch);
            fft->inBuf=(double*)fftw_malloc(FURNACE_FFT_SIZE*sizeof(double));
            fft->outBuf=(fftw_complex*)fftw_malloc(FURNACE_FFT_SIZE*sizeof(fftw_complex));
            fft->plan=fftw_plan_dft_r2c_1d(FURNACE_FFT_SIZE,fft->inBuf,fft->outBuf,FFTW_ESTIMATE);
          }

          int displaySize=(float)(buf->rate)*(chanOscWindowSize/1000.0f);

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
            if (!e->isRunning()) {
              for (unsigned short i=0; i<512; i++) {
                float x=(float)i/512.0f;
                waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f));
              }
            } else {
              float minLevel=1.0f;
              float maxLevel=-1.0f;
              float dcOff=0.0f;
              unsigned short needlePos=buf->needle;
              for (int i=0; i<FURNACE_FFT_SIZE; i++) {
                fft->inBuf[i]=(double)buf->data[(unsigned short)(needlePos-displaySize*2+((i*displaySize*2)/FURNACE_FFT_SIZE))]/32768.0;
              }
              fftw_execute(fft->plan);
              
              // find origin frequency
              int point=1;
              double candAmp=0.0;
              for (unsigned short i=1; i<512; i++) {
                fftw_complex& f=fft->outBuf[i];
                // AMPLITUDE
                double amp=sqrt(pow(f[0],2.0)+pow(f[1],2.0))/pow((double)i,0.8);
                if (amp>candAmp) {
                  point=i;
                  candAmp=amp;
                }
              }

              // PHASE
              fftw_complex& candPoint=fft->outBuf[point];
              double phase=((double)(displaySize*2)/(double)point)*(0.5+(atan2(candPoint[1],candPoint[0])/(M_PI*2)));

              if (chanOscWaveCorr) {
                needlePos-=phase;
              }
              chanOscPitch[ch]=(float)point/32.0f;
              
              /*
              String cPhase=fmt::sprintf("%d cphase: %f vol: %f",point,phase,chanOscVol[ch]);
              dl->AddText(inRect.Min,0xffffffff,cPhase.c_str());
              */

              needlePos-=displaySize;
              for (unsigned short i=0; i<512; i++) {
                float y=(float)buf->data[(unsigned short)(needlePos+(i*displaySize/512))]/65536.0f;
                if (minLevel>y) minLevel=y;
                if (maxLevel<y) maxLevel=y;
              }
              dcOff=(minLevel+maxLevel)*0.5f;
              for (unsigned short i=0; i<512; i++) {
                float x=(float)i/512.0f;
                float y=(float)buf->data[(unsigned short)(needlePos+(i*displaySize/512))]/65536.0f;
                if (y<-0.5f) y=-0.5f;
                if (y>0.5f) y=0.5f;
                waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-(y-dcOff)));
              }
            }
            ImU32 color=ImGui::GetColorU32(chanOscColor);
            if (chanOscUseGrad) {
              float xVal=computeGradPos(chanOscColorX,ch);
              float yVal=computeGradPos(chanOscColorY,ch);

              xVal=CLAMP(xVal,0.0f,1.0f);
              yVal=CLAMP(yVal,0.0f,1.0f);

              color=chanOscGrad.get(xVal,1.0f-yVal);
            }
            dl->AddPolyline(waveform,512,color,ImDrawFlags_None,dpiScale);
          }
        }
      }
      ImGui::EndTable();

      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        chanOscOptions=!chanOscOptions;
      }
    }
    ImGui::PopStyleVar();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHAN_OSC;
  ImGui::End();
}
