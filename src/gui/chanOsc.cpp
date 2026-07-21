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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"

#define FURNACE_CHANOSC_FFT_SIZE 4096
#define FURNACE_CHANOSC_FFT_RATE 80.0
#define FURNACE_CHANOSC_FFT_CUTOFF 0.1

const char* chanOscRefs[]={
  _N("None (0%)"),
  _N("None (50%)"),
  _N("None (100%)"),

  _N("Frequency"),
  _N("Volume"),
  _N("Channel"),
  _N("Brightness"),

  _N("Note Trigger")
};

const char* chanOscArrangeModes[]={
  _N("columns"),
  _N("rows")
};

const char* chanOscDCOffsetCorrModes[]={
  _N("Off##_cocs0"),
  _N("Normal##_cocs1"),
  _N("High-pass##_cocs2"),
};

static void _drawOsc(const ImDrawList* drawList, const ImDrawCmd* cmd) {
  if (cmd!=NULL) {
    if (cmd->UserCallbackData!=NULL) {
      ((FurnaceGUI*)(((PendingDrawOsc*)cmd->UserCallbackData)->gui))->runPendingDrawOsc((PendingDrawOsc*)cmd->UserCallbackData);
    }
  }
}

float FurnaceGUI::computeGradPos(ChanOsc::Ref type, int chan, int totalChans) {
  switch (type) {
    case ChanOsc::Ref::None:
      return 0.0f;
      break;
    case ChanOsc::Ref::Center:
      return 0.5f;
      break;
    case ChanOsc::Ref::Full:
      return 1.0f;
      break;
    case ChanOsc::Ref::Frequency:
      return chanOsc.chan[chan].pitch;
      break;
    case ChanOsc::Ref::Volume:
      return chanOsc.volume[chan];
      break;
    case ChanOsc::Ref::Channel:
      return (float)chan/(float)(totalChans-1);
      break;
    // case ChanOsc::Ref::Brightness:
    //   return chanOscBright[chan];
    //   break;
    case ChanOsc::Ref::NoteTrigger:
      return e->isChannelMuted(chan)?0.0f:keyHit1[chan];
      break;
    default: return 0.0f;
  }
  return 0.0f;
}

void FurnaceGUI::ChanOsc::calcChannelMap(int totalChans) {
  memset(displayMap,-1,sizeof(displayMap));
  switch (arrangement) {
    case Arrange::Columns: {
      displayColumns=columnsRows;
      for (int i=0; i<totalChans; i++) {
        displayMap[i]=i;
      }
      break;
    }
    case Arrange::Rows: {
      displayColumns=ceil((float)totalChans/(float)columnsRows);
      int column=0;
      for (int i=0, r=0; i<totalChans; i++, r++) {
        displayMap[r]=columnsRows*(r%displayColumns)+column;
        if (displayMap[r]>=totalChans) {
          displayMap[r]=-1;
          i--;
        }
        if ((r%displayColumns)==displayColumns-1) column++;
      }
      break;
    }
  }
}

void FurnaceGUI::calcChanOsc() {
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
      int displaySize=65536.0f*0.03f;
      if (e->isRunning()) {
        short minLevel=32767;
        short maxLevel=-32768;
        unsigned short needlePos=buf->needle>>16;
        for (unsigned short i=needlePos-displaySize; i!=needlePos; i++) {
          short y=buf->data[i];
          if (y==-1) continue;
          if (minLevel>y) minLevel=y;
          if (maxLevel<y) maxLevel=y;
        }
        float estimate=pow((float)(maxLevel-minLevel)/32768.0f,0.5f);
        if (estimate>1.0f) estimate=1.0f;
        chanOsc.volume[i]=MAX(chanOsc.volume[i]*0.87f,estimate);
      }
    } else {
      chanOsc.volume[i]=MAX(chanOsc.volume[i]*0.87f,0.0f);
    }
    if (chanOsc.volume[i]<0.00001f) chanOsc.volume[i]=0.0f;
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
  if (ImGui::Begin("Oscilloscope (per-channel)",&chanOscOpen,globalWinFlags,_("Oscilloscope (per-channel)"))) {
    bool centerSettingReset=false;
    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,ImVec2(0.0f,0.0f));
    float availY=ImGui::GetContentRegionAvail().y;

    const int chans=e->getTotalChannelCount();
    struct OscData {
      DivDispatchOscBuffer* buf;
      ChanOsc::ChannelStatus* fft;
      int chan;
    };
    std::vector<OscData> oscData;

    // fill buffers
    for (int i=0; i<chans; i++) {
      DivDispatchOscBuffer* buf=e->getOscBuffer(i);
      if (buf!=NULL && e->curSubSong->chanShowChanOsc[i]) {
        oscData.push_back({buf,&chanOsc.chan[i],i});
      }
    }
    if (chanOsc.autoArrange) {
      chanOsc.columnsRows=sqrt(oscData.size());
      if (chanOsc.columnsRows>64) chanOsc.columnsRows=64;
      if (chanOsc.columnsRows<1) chanOsc.columnsRows=1;
    }

    chanOsc.calcChannelMap(oscData.size());
    if (ImGui::BeginTable("ChanOsc",chanOsc.displayColumns,ImGuiTableFlags_Borders|ImGuiTableFlags_NoClip)) {
      ImGuiWindow* window=ImGui::GetCurrentWindow();

      ImGuiStyle& style=ImGui::GetStyle();
      ImVec2 waveform[1024];

      // check work thread
      if (chanOsc.workPool==NULL) {
        logV(_("creating chan osc work pool"));
        chanOsc.workPool=new DivWorkPool(settings.chanOscThreads);
      }

      // process
      for (size_t i=0; i<oscData.size(); i++) {
        ChanOsc::ChannelStatus* fft_=oscData[i].fft;

        fft_->relatedBuf=oscData[i].buf;
        fft_->relatedCh=oscData[i].chan;

        if (fft_->relatedBuf!=NULL) {
          // prepare
          if (centerSettingReset) {
            fft_->relatedBuf->readNeedle=fft_->relatedBuf->needle>>16;
          }

          // check FFT status existence
          if (!fft_->ready) {
            logD(_("creating FFT plan for channel %d"),fft_->relatedCh);
            fft_->inBuf=(double*)fftw_malloc(FURNACE_CHANOSC_FFT_SIZE*sizeof(double));
            fft_->outBuf=(fftw_complex*)fftw_malloc(FURNACE_CHANOSC_FFT_SIZE*sizeof(fftw_complex));
            fft_->corrBuf=(double*)fftw_malloc(FURNACE_CHANOSC_FFT_SIZE*sizeof(double));
            fft_->plan=fftw_plan_dft_r2c_1d(FURNACE_CHANOSC_FFT_SIZE,fft_->inBuf,fft_->outBuf,FFTW_ESTIMATE);
            fft_->planI=fftw_plan_dft_c2r_1d(FURNACE_CHANOSC_FFT_SIZE,fft_->outBuf,fft_->corrBuf,FFTW_ESTIMATE);
            if (fft_->plan==NULL) {
              logE(_("failed to create plan!"));
            } else if (fft_->planI==NULL) {
              logE(_("failed to create inverse plan!"));
            } else if (fft_->inBuf==NULL || fft_->outBuf==NULL || fft_->corrBuf==NULL) {
              logE(_("failed to create FFT buffers"));
            } else {
              fft_->ready=true;
            }
          }

          if (fft_->ready && e->isRunning()) {
            fft_->windowSize=chanOsc.windowSize;
            fft_->waveCorr=chanOsc.waveCorr;
            chanOsc.workPool->push([](void* fft_v) {
              ChanOsc::ChannelStatus* fft=(ChanOsc::ChannelStatus*)fft_v;
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
              int displaySize=65536.0f*(fft->windowSize/1000.0f);
              int displaySize2=65536.0f*(fft->windowSize/500.0f);
              fft->loudEnough=false;
              fft->needle=buf->needle>>16;

              // first FFT
              int k=0;
              short lastSample=0;
              memset(fft->inBuf,0,FURNACE_CHANOSC_FFT_SIZE*sizeof(double));
              if (displaySize2<FURNACE_CHANOSC_FFT_SIZE) {
                for (int j=-FURNACE_CHANOSC_FFT_SIZE; j<FURNACE_CHANOSC_FFT_SIZE; j++) {
                  const short newData=buf->data[(unsigned short)(fft->needle-displaySize2+((j*displaySize2)/(FURNACE_CHANOSC_FFT_SIZE)))];
                  if (newData!=-1) lastSample=newData;
                  if (j<0) continue;
                  fft->inBuf[j]=(double)lastSample/32768.0;
                  if (fft->inBuf[j]>0.001 || fft->inBuf[j]<-0.001) fft->loudEnough=true;
                  fft->inBuf[j]*=0.55-0.45*cos(M_PI*(double)j/(double)(FURNACE_CHANOSC_FFT_SIZE>>1));
                }
              } else {
                for (unsigned short j=fft->needle-displaySize2; j!=fft->needle; j++, k++) {
                  const int kIn=(k*FURNACE_CHANOSC_FFT_SIZE)/displaySize2;
                  if (kIn>=FURNACE_CHANOSC_FFT_SIZE) break;
                  if (buf->data[j]!=-1) lastSample=buf->data[j];
                  fft->inBuf[kIn]=(double)lastSample/32768.0;
                  if (fft->inBuf[kIn]>0.001 || fft->inBuf[kIn]<-0.001) fft->loudEnough=true;
                  fft->inBuf[kIn]*=0.55-0.45*cos(M_PI*(double)kIn/(double)(FURNACE_CHANOSC_FFT_SIZE>>1));
                }
              }

              // only proceed if not quiet
              if (fft->loudEnough) {
                fftw_execute(fft->plan);

                // auto-correlation and second FFT
                for (int j=0; j<FURNACE_CHANOSC_FFT_SIZE; j++) {
                  fft->outBuf[j][0]/=FURNACE_CHANOSC_FFT_SIZE;
                  fft->outBuf[j][1]/=FURNACE_CHANOSC_FFT_SIZE;
                  fft->outBuf[j][0]=fft->outBuf[j][0]*fft->outBuf[j][0]+fft->outBuf[j][1]*fft->outBuf[j][1];
                  fft->outBuf[j][1]=0;
                }
                fft->outBuf[0][0]=0;
                fft->outBuf[0][1]=0;
                fft->outBuf[1][0]=0;
                fft->outBuf[1][1]=0;
                fftw_execute(fft->planI);

                // window
                for (int j=0; j<(FURNACE_CHANOSC_FFT_SIZE>>1); j++) {
                  fft->corrBuf[j]*=1.0-((double)j/(double)(FURNACE_CHANOSC_FFT_SIZE<<1));
                }

                // find size of period
                double waveLenCandL=DBL_MAX;
                double waveLenCandH=DBL_MIN;
                fft->waveLen=FURNACE_CHANOSC_FFT_SIZE-1;
                fft->waveLenBottom=0;
                fft->waveLenTop=0;

                // find lowest point
                for (int j=(FURNACE_CHANOSC_FFT_SIZE>>2); j>2; j--) {
                  if (fft->corrBuf[j]<waveLenCandL) {
                    waveLenCandL=fft->corrBuf[j];
                    fft->waveLenBottom=j;
                  }
                }

                // find highest point
                for (int j=(FURNACE_CHANOSC_FFT_SIZE>>1)-1; j>fft->waveLenBottom; j--) {
                  if (fft->corrBuf[j]>waveLenCandH) {
                    waveLenCandH=fft->corrBuf[j];
                    fft->waveLen=j;
                  }
                }
                fft->waveLenTop=fft->waveLen;

                // did we find the period size?
                if (fft->waveLen<(FURNACE_CHANOSC_FFT_SIZE-32)) {
                  // we got pitch
                  fft->pitch=pow(1.0-(fft->waveLen/(double)(FURNACE_CHANOSC_FFT_SIZE>>1)),4.0);

                  fft->waveLen*=(double)displaySize*2.0/(double)FURNACE_CHANOSC_FFT_SIZE;

                  // DFT of one period (x_1)
                  double dft[2];
                  dft[0]=0.0;
                  dft[1]=0.0;
                  lastSample=0;
                  for (int j=fft->needle-1-displaySize-(int)fft->waveLen, k=-(displaySize>>1); k<fft->waveLen; j++, k++) {
                    if (buf->data[j&0xffff]!=-1) lastSample=buf->data[j&0xffff];
                    if (k<0) continue;
                    double one=((double)lastSample/32768.0);
                    double two=(double)k*(-2.0*M_PI)/fft->waveLen;
                    dft[0]+=one*cos(two);
                    dft[1]+=one*sin(two);
                  }

                  // calculate and lock into phase
                  phase=(0.5+(atan2(dft[1],dft[0])/(2.0*M_PI)));

                  fft->debugPhase=phase;

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
      chanOsc.workPool->wait();

      int rows=(oscData.size()+(chanOsc.displayColumns-1))/chanOsc.displayColumns;
      if (chanOsc.arrangement==ChanOsc::Arrange::Rows) rows=chanOsc.columnsRows;

      // render
      for (int i=0; i<chanOsc.displayColumns*rows; i++) {
        if (i%chanOsc.displayColumns==0) ImGui::TableNextRow();
        ImGui::TableNextColumn();

        int _c=chanOsc.displayMap[i];
        if (_c==-1) continue;
        DivDispatchOscBuffer* buf=oscData[_c].buf;
        ChanOsc::ChannelStatus* fft=oscData[_c].fft;
        int ch=oscData[_c].chan;
        if (buf==NULL) {
          ImGui::Text(_("Error!"));
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
              if (rend->supportsDrawOsc() && settings.shaderOsc) {
                memset(fft->oscTex,0,2048*sizeof(float));
              } else {
                for (unsigned short j=0; j<precision; j++) {
                  float x=(float)j/(float)precision;
                  waveform[j]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f));
                }
              }
            } else {
              int displaySize=65536.0f*(chanOsc.windowSize/1000.0f);
              int displaySize2=65536.0f*(chanOsc.windowSize/500.0f);

              float minLevel=1.0f;
              float maxLevel=-1.0f;

              if (debugFFT) {
                // FFT debug code!
                double maxavg=0.0;
                for (unsigned short j=0; j<(FURNACE_CHANOSC_FFT_SIZE>>1); j++) {
                  if (fabs(fft->corrBuf[j]>maxavg)) {
                    maxavg=fabs(fft->corrBuf[j]);
                  }
                }
                if (maxavg>0.0000001) maxavg=0.5/maxavg;

                if (rend->supportsDrawOsc() && settings.shaderOsc) {
                  for (unsigned short j=0; j<precision && j<2048; j++) {
                    float y;
                    if (j>=precision/2) {
                      y=fft->inBuf[((j-(precision/2))*FURNACE_CHANOSC_FFT_SIZE*2)/(precision)];
                    } else {
                      y=fft->corrBuf[(j*FURNACE_CHANOSC_FFT_SIZE)/precision]*maxavg;
                    }
                    fft->oscTex[j]=y*2.0;
                  }
                } else {
                  for (unsigned short j=0; j<precision; j++) {
                    float x=(float)j/(float)precision;
                    float y;
                    if (j>=precision/2) {
                      y=fft->inBuf[((j-(precision/2))*FURNACE_CHANOSC_FFT_SIZE*2)/(precision)];
                    } else {
                      y=fft->corrBuf[(j*FURNACE_CHANOSC_FFT_SIZE)/precision]*maxavg;
                    }
                    waveform[j]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-y));
                  }
                }
                if (fft->loudEnough) {
                  String cPhase=fmt::sprintf("\n%.1f (b: %d t: %d)\nSIZES: %d, %d, %d\nPHASE %f",fft->waveLen,fft->waveLenBottom,fft->waveLenTop,displaySize,displaySize2,FURNACE_CHANOSC_FFT_SIZE,fft->debugPhase);
                  dl->AddText(inRect.Min,0xffffffff,cPhase.c_str());

                  dl->AddLine(
                    ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenBottom/(double)FURNACE_CHANOSC_FFT_SIZE,0.0)),
                    ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenBottom/(double)FURNACE_CHANOSC_FFT_SIZE,1.0)),
                    0xffffff00
                  );
                  dl->AddLine(
                    ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenTop/(double)FURNACE_CHANOSC_FFT_SIZE,0.0)),
                    ImLerp(inRect.Min,inRect.Max,ImVec2((double)fft->waveLenTop/(double)FURNACE_CHANOSC_FFT_SIZE,1.0)),
                    0xff00ff00
                  );
                  dl->AddLine(
                    ImLerp(inRect.Min,inRect.Max,ImVec2(0.75-(fft->debugPhase*0.25),0.0)),
                    ImLerp(inRect.Min,inRect.Max,ImVec2(0.75-(fft->debugPhase*0.25),1.0)),
                    0xff00ffff
                  );
                } else {
                  if (debugFFT) {
                    dl->AddText(inRect.Min,0xffffffff,"\nquiet");
                  }
                }
              } else {
                // find the first sample
                float y=0;
                for (int j=0; j<32768; j++) {
                  const short y_s=buf->data[(fft->needle-j)&0xffff];
                  if (y_s!=-1) {
                    y=(float)y_s/32768.0f;
                    break;
                  }
                }
                if (chanOsc.centerStrat==ChanOsc::CenterStrategy::Off) { // DC correction off
                  fft->dcOff=0;
                } else if (chanOsc.centerStrat==ChanOsc::CenterStrategy::Normal) { // normal DC correction
                  float y1=y;
                  if (minLevel>y1) minLevel=y1;
                  if (maxLevel<y1) maxLevel=y1;
                  for (unsigned short j=fft->needle; j!=((fft->needle+displaySize)&0xffff); j++) {
                    const short y_s=buf->data[j];
                    if (y_s!=-1) {
                      y1=(float)y_s/32768.0f;
                      if (minLevel>y1) minLevel=y1;
                      if (maxLevel<y1) maxLevel=y1;
                    }
                  }
                  fft->dcOff=(minLevel+maxLevel)*0.5f;
                }
                // render chan osc
                if (displaySize<precision) {
                  for (int j=0; j<precision; j++) {
                    const short y_s=buf->data[(unsigned short)(fft->needle+(j*displaySize/precision))];
                    if (y_s!=-1) {
                      y=(float)y_s/32768.0f;
                      if (j<0) continue;
                      if (minLevel>y) minLevel=y;
                      if (maxLevel<y) maxLevel=y;
                    }
                    if (j<0) continue;
                    float yOut=y-fft->dcOff;
                    if (chanOsc.centerStrat==ChanOsc::CenterStrategy::HighPass) {
                      fft->dcOff+=(y-fft->dcOff)*0.001;
                    }
                    if (yOut<-0.5f) yOut=-0.5f;
                    if (yOut>0.5f) yOut=0.5f;
                    yOut*=chanOsc.amplitude*2.0f;
                    fft->oscTex[j]=yOut;
                  }
                } else {
                  int k=0;
                  for (unsigned short j=fft->needle; j!=((fft->needle+displaySize)&0xffff); j++, k++) {
                    const short y_s=buf->data[j];
                    const int kTex=(k*precision)/displaySize;
                    if (kTex>=precision) break;
                    if (y_s!=-1) {
                      y=(float)y_s/32768.0f;
                      if (k<0) continue;
                      if (minLevel>y) minLevel=y;
                      if (maxLevel<y) maxLevel=y;
                    }
                    if (kTex<0) continue;
                    float yOut=y-fft->dcOff;
                    if (chanOsc.centerStrat==ChanOsc::CenterStrategy::HighPass) {
                      fft->dcOff+=(y-fft->dcOff)*0.001;
                    }
                    if (yOut<-0.5f) yOut=-0.5f;
                    if (yOut>0.5f) yOut=0.5f;
                    yOut*=chanOsc.amplitude*2.0f;
                    fft->oscTex[kTex]=yOut;
                  }
                }

                if (!(rend->supportsDrawOsc() && settings.shaderOsc)) {
                  for (unsigned short j=0; j<precision; j++) {
                    float x=(float)j/(float)precision;
                    waveform[j]=ImLerp(inRect.Min,inRect.Max,ImVec2(x,0.5f-fft->oscTex[j]*0.5f));
                  }
                }
              }
            }
            ImU32 color=0;
            if (e->isChannelMuted(ch)) {
              color=ImGui::GetColorU32(uiColors[GUI_COLOR_CHANNEL_MUTED]);
            } else {
              switch (chanOsc.colorMode) {
                case ChanOsc::ColorMode::Solid:
                  color=ImGui::GetColorU32(chanOsc.color);
                  break;
                case ChanOsc::ColorMode::Channel:
                  color=ImGui::GetColorU32(channelColor(ch));
                  break;
                default: break;
              }
            }
            if (chanOsc.useGradient) {
              float xVal=computeGradPos(chanOsc.colorX,ch,oscData.size());
              float yVal=computeGradPos(chanOsc.colorY,ch,oscData.size());

              xVal=CLAMP(xVal,0.0f,1.0f);
              yVal=CLAMP(yVal,0.0f,1.0f);

              switch (chanOsc.colorMode) {
                case ChanOsc::ColorMode::Solid:
                  color=chanOsc.gradient.get(xVal,1.0f-yVal);
                  break;
                  case ChanOsc::ColorMode::Channel:
                  color=ImAlphaBlendColors(color,chanOsc.gradient.get(xVal,1.0f-yVal));
                  break;
                default: break;
              }
              // gradient xy debug thing
              // char buf[256];
              // snprintf(buf, 256, "%f:%f",xVal,yVal);
              // dl->AddText(inRect.Min,-1,buf);
              // dl->AddCircleFilled(
              //   ImVec2(
              //     ImLerp(inRect.Min.x,inRect.Max.x,xVal),
              //     ImLerp(inRect.Min.y,inRect.Max.y,1.0f-yVal)
              //   ), 2, 0xffff0000);
            }

            if (inRect.Contains(ImGui::GetMousePos())) {
              ImGui::SetCursorPos(inRect.Min-ImGui::GetWindowPos()+ImGui::GetStyle().FramePadding);
              ImGui::Text("%s##chanOscMute", e->isChannelMuted(ch)?ICON_FA_VOLUME_OFF:ICON_FA_VOLUME_UP);
              if (ImGui::IsItemClicked()) {
                e->toggleMute(ch);
              }
              if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                e->toggleSolo(ch);
              }
              if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip(_("toggle channel mute\nright click to solo"));
              }
              ImGui::SameLine();
              ImGui::TextUnformatted(ICON_FA_CHEVRON_RIGHT "##chanOscGoTo");
              if (ImGui::IsItemClicked()) {
                cursor.xCoarse=ch;
                doAction(GUI_ACTION_WINDOW_PATTERN);
              }
              if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                ImGui::SetTooltip(_("go to this channel"));
              }
            }
            if ((int)i==chanOsc.displayColumns-1) { // topright cell
              if (ImGui::IsWindowHovered()) {
                ImGui::SetCursorPosX(inRect.Max.x-ImGui::GetWindowPos().x-ImGui::GetStyle().FramePadding.x-ImGui::CalcTextSize(ICON_FA_BARS).x);
                ImGui::SetCursorPosY(inRect.Min.y-ImGui::GetWindowPos().y+ImGui::GetStyle().FramePadding.y);
                ImGui::TextUnformatted(ICON_FA_BARS "##chanOscSettings");
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                  ImGui::SetTooltip(_("per-channel oscilloscope settings"));
                }
                if (ImGui::IsItemClicked()) {
                  chanOsc.showOptions=true;
                }
              }
            }
            if (!chanOsc.textFormat.empty()) {
              String text;
              bool inFormat=false;

              for (char j: chanOsc.textFormat) {
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
                      text+=e->getSystemName(e->song.sysOfChan[ch]);
                      break;
                    }
                    case 'p': {
                      text+=FurnaceGUI::getSystemPartNumber(e->song.sysOfChan[ch],e->song.systemFlags[e->song.dispatchOfChan[ch]]);
                      break;
                    }
                    case 'S': {
                      text+=fmt::sprintf("%d",e->song.dispatchOfChan[ch]);
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
                      double volMax=chanState->volMax>>8;
                      if (volMax<1) volMax=1;
                      text+=fmt::sprintf("%.1f%%",((double)(chanState->volume>>8)/volMax)*100);
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
                      if (chanState==NULL) {
                        text+="---";
                      } else if (!chanState->keyOn) {
                        text+="---";
                      } else {
                        // no more conversion necessary after the note/octave unification :>
                        text+=fmt::sprintf("%s",noteName(chanState->note));
                      }
                      break;
                    }
                    case 'l':
                      text+='\n';
                      break;
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
              ImGui::PushClipRect(inRect.Min,inRect.Max,false);
              dl->AddText(ImLerp(inRect.Min,inRect.Max,ImVec2(0.0f,0.0f)),ImGui::GetColorU32(chanOsc.textColor),text.c_str());
              ImGui::PopClipRect();
            }

            if (rend->supportsDrawOsc() && settings.shaderOsc) {
              fft->drawOp.gui=this;
              fft->drawOp.data=fft->oscTex;
              fft->drawOp.len=precision;
              fft->drawOp.pos0=inRect.Min;
              fft->drawOp.pos1=inRect.Max;
              fft->drawOp.color=ImGui::ColorConvertU32ToFloat4(color);
              fft->drawOp.lineSize=dpiScale*chanOsc.lineSize;

              dl->AddCallback(_drawOsc,&fft->drawOp);
              dl->AddCallback(ImDrawCallback_ResetRenderState,NULL);
            } else {
              // ImGui::PushClipRect(inRect.Min,inRect.Max,false);
              //ImDrawListFlags prevFlags=dl->Flags;
              //dl->Flags&=~(ImDrawListFlags_AntiAliasedLines|ImDrawListFlags_AntiAliasedLinesUseTex);
              dl->AddPolyline(waveform,precision,color,ImDrawFlags_None,dpiScale*chanOsc.lineSize);
              //dl->Flags=prevFlags;
              // ImGui::PopClipRect();
            }
          }
        }
      }
      ImGui::EndTable();

      ImGui::PopStyleVar();
    }
    if (chanOsc.showOptions) ImGui::OpenPopup("chanOscSettingsPopup");
    if (ImGui::BeginPopup("chanOscSettingsPopup",ImGuiPopupFlags_MouseButtonLeft)) {
      ImDrawList* dl=ImGui::GetWindowDrawList();
      chanOsc.showOptions=false;
      if (ImGui::BeginTabBar("chanOscSettingsTabBar")) {
        if (ImGui::BeginTabItem(_("Scope"))) {
          ImGui::AlignTextToFramePadding();
          ImGui::TextUnformatted(_("Arrange channels by"));
          ImGui::SameLine();
          ImGui::Combo("##chanArrange",(int*)&chanOsc.arrangement,LocalizedComboGetter,chanOscArrangeModes,2);
          ImGui::Checkbox(_("Arange automatically"),&chanOsc.autoArrange);
          ImGui::BeginDisabled(chanOsc.autoArrange);
          if (ImGui::InputInt(chanOsc.arrangement==ChanOsc::Arrange::Rows?_("Rows"):_("Columns"),&chanOsc.columnsRows)) {
            if (chanOsc.columnsRows>64) chanOsc.columnsRows=64;
            if (chanOsc.columnsRows<1) chanOsc.columnsRows=1;
          }
          ImGui::EndDisabled();
          ImGui::Separator();
          if (ImGui::SliderFloat(_("Size"),&chanOsc.windowSize,1.0f,50.0f,"%g ms")) {
            if (chanOsc.windowSize<1.0f) chanOsc.windowSize=1.0f;
            if (chanOsc.windowSize>50.0f) chanOsc.windowSize=50.0f;
          } rightClickable
          if (ImGui::SliderFloat(_("Amplitude"),&chanOsc.amplitude,0.0f,2.0f)) {
            if (chanOsc.amplitude<0.0f) chanOsc.amplitude=0.0f;
            if (chanOsc.amplitude>2.0f) chanOsc.amplitude=2.0f;
          } rightClickable
          ImGui::Separator();
          if (ImGui::Checkbox(_("Center waveform"),&chanOsc.waveCorr)) centerSettingReset=true;
          ImGui::Checkbox(_("Randomize phase on note"),&chanOsc.randomPhase);
          ImGui::AlignTextToFramePadding();
          ImGui::TextUnformatted(_("DC offset correction"));
          ImGui::SameLine();
          ImGui::Combo("##dcOffCorr",(int*)&chanOsc.centerStrat,LocalizedComboGetter,chanOscDCOffsetCorrModes,3);
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(_("Waveform"))) {
          if (ImGui::SliderFloat(_("Line Size"),&chanOsc.lineSize,0.25f,16.0f)) {
            if (chanOsc.lineSize<0.25f) chanOsc.lineSize=0.25f;
            if (chanOsc.lineSize>16.0f) chanOsc.lineSize=16.0f;
          } rightClickable

          ImGui::Checkbox(_("Gradient"),&chanOsc.useGradient);
          if (chanOsc.useGradient) {
            if (chanOsc.gradientTex==NULL) {
              chanOsc.gradientTex=rend->createTexture(true,chanOsc.gradient.width,chanOsc.gradient.height,true,bestTexFormat);
              if (chanOsc.gradientTex==NULL) {
                logE(_("error while creating gradient texture!"));
              } else {
                chanOsc.updateGradientTex=true;
              }
            }

            if (ImGui::BeginTable("ChanOscGradSet",2)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              if (chanOsc.gradientTex!=NULL) {
                if (chanOsc.updateGradientTex) {
                  chanOsc.gradient.render();
                  if (rend->updateTexture(chanOsc.gradientTex,chanOsc.gradient.grad.get(),chanOsc.gradient.width*4)) {
                    chanOsc.updateGradientTex=false;
                  } else {
                    logE(_("error while updating gradient texture!"));
                  }
                }

                ImVec2 gradSize=ImVec2(400.0f*dpiScale,400.0f*dpiScale);
                int index=0;
                int removePoint=-1;
                // this child nonesense fixes a ErrorCheckUsingSetCursorPosToExtendParentBoundaries() assert
                if (ImGui::BeginChild("chanOscGradArea",gradSize,0,ImGuiWindowFlags_NoScrollbar)) {
                  ImVec2 gradLeft=ImGui::GetWindowPos();
                  dl->AddImage(rend->getTextureID(chanOsc.gradientTex),gradLeft,gradLeft+gradSize,ImVec2(0,0),ImVec2(rend->getTextureU(chanOsc.gradientTex),rend->getTextureV(chanOsc.gradientTex)));
                  ImVec2 gradLeftAbs=ImGui::GetItemRectMin();
                  if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                    if (chanOsc.gradient.points.size()<32) {
                      chanOsc.gradient.points.push_back(Gradient2DPoint(
                        (ImGui::GetMousePos().x-gradLeftAbs.x)/gradSize.x,
                        (ImGui::GetMousePos().y-gradLeftAbs.y)/gradSize.y
                      ));
                      chanOsc.updateGradientTex=true;
                    }
                  }

                  for (Gradient2DPoint& i: chanOsc.gradient.points) {
                    ImGui::PushID(index+16);
                    ImGui::SetCursorPos(ImVec2(i.x*gradSize.x-8.0*dpiScale,i.y*gradSize.y-8.0*dpiScale));
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
                        chanOsc.updateGradientTex=true;
                      }
                    } else {
                      i.grab=false;
                      i.prevX=i.x;
                      i.prevY=i.y;
                    }
                    if (ImGui::BeginPopup("gradPointSettings",ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize)) {
                      if (ImGui::ColorPicker4(_("Color"),(float*)&i.color)) {
                        chanOsc.updateGradientTex=true;
                      }
                      ImGui::AlignTextToFramePadding();
                      ImGui::Text(_("Distance"));
                      ImGui::SameLine();
                      float pDist=i.distance*100.0f;
                      if (ImGui::SliderFloat("##PDistance",&pDist,0.0f,150.0f,"%.1f%%")) {
                        i.distance=pDist/100.0f;
                        chanOsc.updateGradientTex=true;
                      }

                      ImGui::AlignTextToFramePadding();
                      ImGui::Text(_("Spread"));
                      ImGui::SameLine();
                      float pSpread=i.spread*100.0f;
                      if (ImGui::SliderFloat("##PSpread",&pSpread,0.0f,150.0f,"%.1f%%")) {
                        i.spread=pSpread/100.0f;
                        chanOsc.updateGradientTex=true;
                      }

                      pushDestColor();
                      if (ImGui::Button(_("Remove"))) {
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
                }
                if (removePoint>=0) {
                  chanOsc.gradient.points.erase(chanOsc.gradient.points.begin()+removePoint);
                  chanOsc.updateGradientTex=true;
                }
                ImGui::EndChild();
              }

              ImGui::TableNextColumn();
              ImGui::Text(_("Background:"));
              ImGui::Indent();
              if (ImGui::RadioButton(_("Solid color"),chanOsc.colorMode==ChanOsc::ColorMode::Solid)) {
                chanOsc.colorMode=ChanOsc::ColorMode::Solid;
                chanOsc.updateGradientTex=true;
              }
              if (chanOsc.colorMode==ChanOsc::ColorMode::Solid) {
                ImGui::Indent();
                if (ImGui::ColorEdit4(_("Color"),(float*)&chanOsc.gradient.bgColor)) {
                  chanOsc.updateGradientTex=true;
                }
                ImGui::Unindent();
              }
              if (ImGui::RadioButton(_("Channel color"),chanOsc.colorMode==ChanOsc::ColorMode::Channel)) {
                chanOsc.colorMode=ChanOsc::ColorMode::Channel;
                chanOsc.gradient.bgColor.x=0.0f;
                chanOsc.gradient.bgColor.y=0.0f;
                chanOsc.gradient.bgColor.z=0.0f;
                chanOsc.gradient.bgColor.w=0.0f;
                chanOsc.updateGradientTex=true;
              }
              // in preparation of image texture
              ImGui::Unindent();

              ImGui::Combo(_("X Axis##AxisX"),(int*)&chanOsc.colorX,LocalizedComboGetter,chanOscRefs,(int)ChanOsc::Ref::Max);
              ImGui::Combo(_("Y Axis##AxisY"),(int*)&chanOsc.colorY,LocalizedComboGetter,chanOscRefs,(int)ChanOsc::Ref::Max);

              ImGui::EndTable();
            }
          }
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(_("Text"))) {
          ImGui::InputText(_("Format"),&chanOsc.textFormat);
          if (ImGui::IsItemHovered()) {
            if (ImGui::BeginTooltip()) {
              ImGui::TextUnformatted(_(
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
                "- %l: new line\n"
                "- %%: percent sign"
              ));
              ImGui::EndTooltip();
            }
          }
          ImGui::ColorEdit4(_("Color"),(float*)&chanOsc.textColor);
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
      ImGui::EndPopup();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHAN_OSC;
  ImGui::End();
}
