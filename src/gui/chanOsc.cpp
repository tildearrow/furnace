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
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_internal.h"

#define FURNACE_FFT_SIZE 4096
#define FURNACE_FFT_RATE 80.0
#define FURNACE_FFT_CUTOFF 0.1

void FurnaceGUI::drawChanOsc() {
  if (nextWindow==GUI_WINDOW_CHAN_OSC) {
    chanOscOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!chanOscOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Oscilloscope (per-channel)",&chanOscOpen,globalWinFlags)) {
    bool centerSettingReset=false;
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
    

    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    float availY=ImGui::GetContentRegionAvail().y;
    if (ImGui::BeginTable("ChanOsc",chanOscCols,ImGuiTableFlags_Borders)) {
      std::vector<DivDispatchOscBuffer*> oscBufs;
      std::vector<ChanOscStatus*> oscFFTs;
      std::vector<int> oscChans;
      int chans=e->getTotalChannelCount();
      ImDrawList* dl=ImGui::GetWindowDrawList();
      ImGuiWindow* window=ImGui::GetCurrentWindow();
      ImVec2 waveform[512];

      ImGuiStyle& style=ImGui::GetStyle();
      ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_OSC_WAVE]);

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
            if (!e->isPlaying()) {
              for (unsigned short i=0; i<512; i++) {
                float x=(float)i/512.0f;
                waveform[i]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f));
              }
            } else {
              float minLevel=1.0f;
              float maxLevel=-1.0f;
              float dcOff=0.0f;
              unsigned short needlePos=buf->needle;
              if (chanOscWaveCorr) {
                /*
                double fftDataRate=(FURNACE_FFT_SIZE*FURNACE_FFT_RATE)/((double)buf->rate);
                while (buf->readNeedle!=needlePos) {
                  fft->inBufPosFrac+=fftDataRate;
                  while (fft->inBufPosFrac>=1.0) {
                    chanOscLP0[ch]+=FURNACE_FFT_CUTOFF*((float)buf->data[buf->readNeedle]-chanOscLP0[ch]);
                    chanOscLP1[ch]+=FURNACE_FFT_CUTOFF*(chanOscLP0[ch]-chanOscLP1[ch]);
                    fft->inBuf[fft->inBufPos]=(double)chanOscLP1[ch]/32768.0;
                    if (++fft->inBufPos>=FURNACE_FFT_SIZE) {
                      fftw_execute(fft->plan);
                      fft->inBufPos=0;
                      fft->needle=buf->readNeedle;
                    }
                    fft->inBufPosFrac-=1.0;
                  }
                  buf->readNeedle++;
                }*/

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

                //needlePos=fft->needle;
                needlePos-=phase;
                
                /*
                int alignment=0;
                for (unsigned short i=0; i<displaySize; i++) {
                  if (fabs(buf->data[(unsigned short)(needlePos-i)])>fabs(buf->data[(unsigned short)(needlePos-alignment)])) {
                    alignment=i;
                  }
                }
                needlePos-=alignment;
                */
                
                //String cPhase=fmt::sprintf("%d cphase: %f",point,phase);
                //dl->AddText(inRect.Min,0xffffffff,cPhase.c_str());

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
              } else {
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
