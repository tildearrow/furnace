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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

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

const char* autoColsTypes[]={
  "Off",
  "Mode 1",
  "Mode 2",
  "Mode 3"
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
      return chanOscChan[chan].pitch;
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
    if (buf!=NULL && e->curSubSong->chanShowChanOsc[i]) {
      // 30ms should be enough
      int displaySize=(float)(buf->rate)*0.03f;
      if (e->isRunning()) {
        short minLevel=32767;
        short maxLevel=-32768;
        unsigned short needlePos=buf->needle;
        needlePos-=displaySize;
        for (unsigned short i=0; i<512; i++) {
          short y=buf->data[(unsigned short)(needlePos+(i*displaySize/512))];
          if (minLevel>y) minLevel=y;
          if (maxLevel<y) maxLevel=y;
        }
        float estimate=pow((float)(maxLevel-minLevel)/32768.0f,0.5f);
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
  if (ImGui::Begin("Oscilloscope (per-channel)",&chanOscOpen,globalWinFlags)) {
    bool centerSettingReset=false;
    ImDrawList* dl=ImGui::GetWindowDrawList();
    if (chanOscOptions) {
      if (ImGui::BeginTable("ChanOscSettings",2)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Columns");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##COSColumns",&chanOscCols,1,3)) {
          if (chanOscCols<1) chanOscCols=1;
          if (chanOscCols>64) chanOscCols=64;
        }

        ImGui::TableNextColumn();
        ImGui::Text("Size (ms)");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputFloat("##COSWinSize",&chanOscWindowSize,1.0f,10.0f)) {
          if (chanOscWindowSize<1.0f) chanOscWindowSize=1.0f;
          if (chanOscWindowSize>50.0f) chanOscWindowSize=50.0f;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Automatic columns");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        const char* previewColType=autoColsTypes[chanOscAutoColsType&3];
        if (ImGui::BeginCombo("##AutoCols",previewColType)) {
          for (int j=0; j<4; j++) {
            const bool isSelected=(chanOscAutoColsType==j);
            if (ImGui::Selectable(autoColsTypes[j],isSelected)) chanOscAutoColsType=j;
            if (isSelected) ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }

        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Center waveform",&chanOscWaveCorr)) {
          centerSettingReset=true;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("Randomize phase on note",&chanOscRandomPhase)) {
        }
        ImGui::EndTable();
      }

      ImGui::AlignTextToFramePadding();
      ImGui::Text("Amplitude");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (CWSliderFloat("##COSAmp",&chanOscAmplify,0.0f,2.0f)) {
        if (chanOscAmplify<0.0f) chanOscAmplify=0.0f;
        if (chanOscAmplify>2.0f) chanOscAmplify=2.0f;
      }

      ImGui::Checkbox("Gradient",&chanOscUseGrad);

      if (chanOscUseGrad) {
        if (chanOscGradTex==NULL) {
          chanOscGradTex=rend->createTexture(true,chanOscGrad.width,chanOscGrad.height);

          if (chanOscGradTex==NULL) {
            logE("error while creating gradient texture!");
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
              if (rend->updateTexture(chanOscGradTex,chanOscGrad.grad.get(),chanOscGrad.width*4)) {
                updateChanOscGradTex=false;
              } else {
                logE("error while updating gradient texture!");
              }
            }

            ImVec2 gradLeft=ImGui::GetCursorPos();
            ImVec2 gradSize=ImVec2(400.0f*dpiScale,400.0f*dpiScale);
            ImGui::Image(rend->getTextureID(chanOscGradTex),gradSize);
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
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Distance");
                ImGui::SameLine();
                float pDist=i.distance*100.0f;
                if (ImGui::SliderFloat("##PDistance",&pDist,0.0f,150.0f,"%.1f%%")) {
                  i.distance=pDist/100.0f;
                  updateChanOscGradTex=true;
                }

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Spread");
                ImGui::SameLine();
                float pSpread=i.spread*100.0f;
                if (ImGui::SliderFloat("##PSpread",&pSpread,0.0f,150.0f,"%.1f%%")) {
                  i.spread=pSpread/100.0f;
                  updateChanOscGradTex=true;
                }

                pushDestColor();
                if (ImGui::Button("Remove")) {
                  removePoint=index;
                  ImGui::CloseCurrentPopup();
                }
                popDestColor();

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

      ImGui::AlignTextToFramePadding();
      ImGui::Text("Text format:");
      ImGui::SameLine();
      ImGui::InputText("##TextFormat",&chanOscTextFormat);
      if (ImGui::IsItemHovered()) {
        if (ImGui::BeginTooltip()) {
          ImGui::TextUnformatted(
            "format guide:\n"
            "- %c: channel name\n"
            "- %C: channel short name\n"
            "- %d: channel number (starting from 0)\n"
            "- %D: channel number (starting from 1)\n"
            "- %n: channel note\n"
            "- %i: instrument name\n"
            "- %I: instrument number (decimal)\n"
            "- %x: instrument number (hex)\n"
            "- %s: chip name\n"
            "- %p: chip part number\n"
            "- %S: chip ID\n"
            "- %v: volume (decimal)\n"
            "- %V: volume (percentage)\n"
            "- %b: volume (hex)\n"
            "- %%: percent sign"
          );
          ImGui::EndTooltip();
        }
      }

      ImGui::ColorEdit4("Text color",(float*)&chanOscTextColor);

      if (ImGui::Button("OK")) {
        chanOscOptions=false;
      }
    } else {
      ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
      float availY=ImGui::GetContentRegionAvail().y;
      if (ImGui::BeginTable("ChanOsc",chanOscCols,ImGuiTableFlags_Borders|ImGuiTableFlags_NoClip)) {
        std::vector<DivDispatchOscBuffer*> oscBufs;
        std::vector<ChanOscStatus*> oscFFTs;
        std::vector<int> oscChans;
        int chans=e->getTotalChannelCount();
        ImGuiWindow* window=ImGui::GetCurrentWindow();

        ImGuiStyle& style=ImGui::GetStyle();
        ImVec2 waveform[1024];

        // check work thread
        if (chanOscWorkPool==NULL) {
          logV("creating chan osc work pool");
          chanOscWorkPool=new DivWorkPool(settings.chanOscThreads);
        }

        // fill buffers
        for (int i=0; i<chans; i++) {
          DivDispatchOscBuffer* buf=e->getOscBuffer(i);
          if (buf!=NULL && e->curSubSong->chanShowChanOsc[i]) {
            oscBufs.push_back(buf);
            oscFFTs.push_back(&chanOscChan[i]);
            oscChans.push_back(i);
          }
        }

        // process
        for (size_t i=0; i<oscBufs.size(); i++) {
          ChanOscStatus* fft_=oscFFTs[i];

          fft_->relatedBuf=oscBufs[i];
          fft_->relatedCh=oscChans[i];

          if (fft_->relatedBuf!=NULL) {
            // prepare
            if (centerSettingReset) {
              fft_->relatedBuf->readNeedle=fft_->relatedBuf->needle;
            }

            // check FFT status existence
            if (!fft_->ready) {
              logD("creating FFT plan for channel %d",fft_->relatedCh);
              fft_->inBuf=(double*)fftw_malloc(FURNACE_FFT_SIZE*sizeof(double));
              fft_->outBuf=(fftw_complex*)fftw_malloc(FURNACE_FFT_SIZE*sizeof(fftw_complex));
              fft_->corrBuf=(double*)fftw_malloc(FURNACE_FFT_SIZE*sizeof(double));
              fft_->plan=fftw_plan_dft_r2c_1d(FURNACE_FFT_SIZE,fft_->inBuf,fft_->outBuf,FFTW_ESTIMATE);
              fft_->planI=fftw_plan_dft_c2r_1d(FURNACE_FFT_SIZE,fft_->outBuf,fft_->corrBuf,FFTW_ESTIMATE);
              if (fft_->plan==NULL) {
                logE("failed to create plan!");
              } else if (fft_->planI==NULL) {
                logE("failed to create inverse plan!");
              } else if (fft_->inBuf==NULL || fft_->outBuf==NULL || fft_->corrBuf==NULL) {
                logE("failed to create FFT buffers");
              } else {
                fft_->ready=true;
              }
            }

            if (fft_->ready && e->isRunning()) {
              fft_->windowSize=chanOscWindowSize;
              fft_->waveCorr=chanOscWaveCorr;
              chanOscWorkPool->push([](void* fft_v) {
                ChanOscStatus* fft=(ChanOscStatus*)fft_v;
                DivDispatchOscBuffer* buf=fft->relatedBuf;

                // the STRATEGY
                // 1. FFT of windowed signal
                // 2. inverse FFT of auto-correlation
                // 3. find size of one period
                // 4. DFT of the fundamental of ONE PERIOD
                // 5. now we can get phase information
                //
                // I have a feeling this could be simplified to two FFTs or even one...
                // if you know how, please tell me

                // initialization
                double phase=0.0;
                int displaySize=(float)(buf->rate)*(fft->windowSize/1000.0f);
                fft->loudEnough=false;
                fft->needle=buf->needle;

                // first FFT
                for (int j=0; j<FURNACE_FFT_SIZE; j++) {
                  fft->inBuf[j]=(double)buf->data[(unsigned short)(fft->needle-displaySize*2+((j*displaySize*2)/(FURNACE_FFT_SIZE)))]/32768.0;
                  if (fft->inBuf[j]>0.001 || fft->inBuf[j]<-0.001) fft->loudEnough=true;
                  fft->inBuf[j]*=0.55-0.45*cos(M_PI*(double)j/(double)(FURNACE_FFT_SIZE>>1));
                }

                // only proceed if not quiet
                if (fft->loudEnough) {
                  fftw_execute(fft->plan);

                  // auto-correlation and second FFT
                  for (int j=0; j<FURNACE_FFT_SIZE; j++) {
                    fft->outBuf[j][0]/=FURNACE_FFT_SIZE;
                    fft->outBuf[j][1]/=FURNACE_FFT_SIZE;
                    fft->outBuf[j][0]=fft->outBuf[j][0]*fft->outBuf[j][0]+fft->outBuf[j][1]*fft->outBuf[j][1];
                    fft->outBuf[j][1]=0;
                  }
                  fft->outBuf[0][0]=0;
                  fft->outBuf[0][1]=0;
                  fft->outBuf[1][0]=0;
                  fft->outBuf[1][1]=0;
                  fftw_execute(fft->planI);

                  // window
                  for (int j=0; j<(FURNACE_FFT_SIZE>>1); j++) {
                    fft->corrBuf[j]*=1.0-((double)j/(double)(FURNACE_FFT_SIZE<<1));
                  }

                  // find size of period
                  double waveLenCandL=DBL_MAX;
                  double waveLenCandH=DBL_MIN;
                  fft->waveLen=FURNACE_FFT_SIZE-1;
                  fft->waveLenBottom=0;
                  fft->waveLenTop=0;

                  // find lowest point
                  for (int j=(FURNACE_FFT_SIZE>>2); j>2; j--) {
                    if (fft->corrBuf[j]<waveLenCandL) {
                      waveLenCandL=fft->corrBuf[j];
                      fft->waveLenBottom=j;
                    }
                  }
                  
                  // find highest point
                  for (int j=(FURNACE_FFT_SIZE>>1)-1; j>fft->waveLenBottom; j--) {
                    if (fft->corrBuf[j]>waveLenCandH) {
                      waveLenCandH=fft->corrBuf[j];
                      fft->waveLen=j;
                    }
                  }
                  fft->waveLenTop=fft->waveLen;

                  // did we find the period size?
                  if (fft->waveLen<(FURNACE_FFT_SIZE-32)) {
                    // we got pitch
                    fft->pitch=pow(1.0-(fft->waveLen/(double)(FURNACE_FFT_SIZE>>1)),4.0);
                    
                    fft->waveLen*=(double)displaySize*2.0/(double)FURNACE_FFT_SIZE;

                    // DFT of one period (x_1)
                    double dft[2];
                    dft[0]=0.0;
                    dft[1]=0.0;
                    for (int j=fft->needle-1-(displaySize>>1)-(int)fft->waveLen, k=0; k<fft->waveLen; j++, k++) {
                      double one=((double)buf->data[j&0xffff]/32768.0);
                      double two=(double)k*(-2.0*M_PI)/fft->waveLen;
                      dft[0]+=one*cos(two);
                      dft[1]+=one*sin(two);
                    }

                    // calculate and lock into phase
                    phase=(0.5+(atan2(dft[1],dft[0])/(2.0*M_PI)));

                    if (fft->waveCorr) {
                      fft->needle-=(phase+(fft->phaseOff*2))*fft->waveLen;
                    }
                  }
                }

                fft->needle-=displaySize;
              },fft_);
            }
          }
        }
        chanOscWorkPool->wait();

        // 0: none
        // 1: sqrt(chans)
        // 2: sqrt(chans+1)
        // 3: sqrt(chans)+1
        switch (chanOscAutoColsType) {
          case 1:
            chanOscCols=sqrt(oscChans.size());
            break;
          case 2:
            chanOscCols=sqrt(oscChans.size()+1);
            break;
          case 3:
            chanOscCols=sqrt(oscChans.size())+1;
            break;
        }
        if (chanOscCols<1) chanOscCols=1;
        if (chanOscCols>64) chanOscCols=64;
        
        int rows=(oscBufs.size()+(chanOscCols-1))/chanOscCols;

        // render
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

            ImVec2 minArea=window->DC.CursorPos;
            ImVec2 maxArea=ImVec2(
              minArea.x+size.x,
              minArea.y+size.y
            );
            ImRect rect=ImRect(minArea,maxArea);
            ImRect inRect=rect;
            inRect.Min.x+=dpiScale;
            inRect.Min.y+=2.0*dpiScale;
            inRect.Max.x-=dpiScale;
            inRect.Max.y-=2.0*dpiScale;

            int precision=inRect.Max.x-inRect.Min.x;
            if (precision<1) precision=1;
            if (precision>1024) precision=1024;

            ImGui::ItemSize(size,style.FramePadding.y);
            if (ImGui::ItemAdd(rect,ImGui::GetID("chOscDisplay"))) {
              if (!e->isRunning()) {
                for (unsigned short j=0; j<precision; j++) {
                  float x=(float)j/(float)precision;
                  waveform[j]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f));
                }
              } else {
                int displaySize=(float)(buf->rate)*(chanOscWindowSize/1000.0f);

                float minLevel=1.0f;
                float maxLevel=-1.0f;
                float dcOff=0.0f;

                if (debugFFT) {
                  // FFT debug code!
                  double maxavg=0.0;
                  for (unsigned short j=0; j<(FURNACE_FFT_SIZE>>1); j++) {
                    if (fabs(fft->corrBuf[j]>maxavg)) {
                      maxavg=fabs(fft->corrBuf[j]);
                    }
                  }
                  if (maxavg>0.0000001) maxavg=0.5/maxavg;

                  for (unsigned short j=0; j<precision; j++) {
                    float x=(float)j/(float)precision;
                    float y;
                    if (j>=precision/2) {
                      y=fft->inBuf[((j-(precision/2))*FURNACE_FFT_SIZE*2)/(precision)];
                    } else {
                      y=fft->corrBuf[(j*FURNACE_FFT_SIZE)/precision]*maxavg;
                    }
                    waveform[j]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
                  }
                  if (fft->loudEnough) {
                    String cPhase=fmt::sprintf("\n%.1f (b: %d t: %d)",fft->waveLen,fft->waveLenBottom,fft->waveLenTop);
                    dl->AddText(inRect.Min,0xffffffff,cPhase.c_str());

                    dl->AddLine(
                      ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenBottom/(double)FURNACE_FFT_SIZE,0.0)),
                      ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenBottom/(double)FURNACE_FFT_SIZE,1.0)),
                      0xffffff00
                    );
                    dl->AddLine(
                      ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenTop/(double)FURNACE_FFT_SIZE,0.0)),
                      ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenTop/(double)FURNACE_FFT_SIZE,1.0)),
                      0xff00ff00
                    );
                  } else {
                    if (debugFFT) {
                      dl->AddText(inRect.Min,0xffffffff,"\nquiet");
                    }
                  }
                } else {
                  for (unsigned short j=0; j<precision; j++) {
                    float y=(float)buf->data[(unsigned short)(fft->needle+(j*displaySize/precision))]/32768.0f;
                    if (minLevel>y) minLevel=y;
                    if (maxLevel<y) maxLevel=y;
                  }
                  dcOff=(minLevel+maxLevel)*0.5f;
                  for (unsigned short j=0; j<precision; j++) {
                    float x=(float)j/(float)precision;
                    float y=(float)buf->data[(unsigned short)(fft->needle+(j*displaySize/precision))]/32768.0f;
                    y-=dcOff;
                    if (y<-0.5f) y=-0.5f;
                    if (y>0.5f) y=0.5f;
                    y*=chanOscAmplify;
                    waveform[j]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
                  }
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
              ImGui::PushClipRect(inRect.Min,inRect.Max,false);

              //ImDrawListFlags prevFlags=dl->Flags;
              //dl->Flags&=~(ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex);
              dl->AddPolyline(waveform,precision,color,ImDrawFlags_None,dpiScale);
              //dl->Flags=prevFlags;

              if (!chanOscTextFormat.empty()) {
                String text;
                bool inFormat=false;

                for (char j: chanOscTextFormat) {
                  if (inFormat) {
                    switch (j) {
                      case 'c':
                        text+=e->getChannelName(ch);
                        break;
                      case 'C':
                        text+=e->getChannelShortName(ch);
                        break;
                      case 'd':
                        text+=fmt::sprintf("%d",ch);
                        break;
                      case 'D':
                        text+=fmt::sprintf("%d",ch+1);
                        break;
                      case 'i': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL) break;
                        DivInstrument* ins=e->getIns(chanState->lastIns);
                        text+=ins->name;
                        break;
                      }
                      case 'I': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL) break;
                        text+=fmt::sprintf("%d",chanState->lastIns);
                        break;
                      }
                      case 'x': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL) break;
                        if (chanState->lastIns<0) {
                          text+="??";
                        } else {
                          text+=fmt::sprintf("%.2X",chanState->lastIns);
                        }
                        break;
                      }
                      case 's': {
                        text+=e->getSystemName(e->sysOfChan[ch]);
                        break;
                      }
                      case 'p': {
                        text+=FurnaceGUI::getSystemPartNumber(e->sysOfChan[ch],e->song.systemFlags[e->dispatchOfChan[ch]]);
                        break;
                      }
                      case 'S': {
                        text+=fmt::sprintf("%d",e->dispatchOfChan[ch]);
                        break;
                      }
                      case 'v': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL) break;
                        text+=fmt::sprintf("%d",chanState->volume>>8);
                        break;
                      }
                      case 'V': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL) break;
                        int volMax=chanState->volMax>>8;
                        if (volMax<1) volMax=1;
                        text+=fmt::sprintf("%d%%",(chanState->volume>>8)/volMax);
                        break;
                      }
                      case 'b': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL) break;
                        text+=fmt::sprintf("%.2X",chanState->volume>>8);
                        break;
                      }
                      case 'n': {
                        DivChannelState* chanState=e->getChanState(ch);
                        if (chanState==NULL || !(chanState->keyOn)) break;
                        short tempNote=chanState->note; //all of this conversion is necessary because notes 100-102 are special chars
                        short noteMod=tempNote%12+12; //also note 0 is a BUG, hence +12 on the note and -1 on the octave
                        short oct=tempNote/12-1; 
                        text+=fmt::sprintf("%s",noteName(noteMod,oct));
                        break;
                      }
                      case '%':
                        text+='%';
                        break;
                      default:
                        text+='%';
                        text+=j;
                        break;
                    }
                    inFormat=false;
                  } else {
                    if (j=='%') {
                      inFormat=true;
                    } else {
                      text+=j;
                    }
                  }
                }
                dl->AddText(ImLerp(inRect.Min,inRect.Max,ImVec2(0.0f,0.0f)),ImGui::GetColorU32(chanOscTextColor),text.c_str());
              }

              ImGui::PopClipRect();
            }
          }
        }
        ImGui::EndTable();

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          chanOscOptions=!chanOscOptions;
        }
        if (ImGui::IsItemHovered() && CHECK_LONG_HOLD) {
          NOTIFY_LONG_HOLD;
          chanOscOptions=!chanOscOptions;
        }
      }
      ImGui::PopStyleVar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHAN_OSC;
  ImGui::End();
}
