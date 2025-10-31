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

// bulk of the code is from https://github.com/Eknous-P/unscope/blob/spectrum/src/gui/wins/spectrum.cpp

#include "gui.h"
#include "imgui_internal.h"
#include <IconsFontAwesome4.h>
#include <fftw3.h>
#include <math.h> // fmod

inline float scaleFuncLog(float x) {
  constexpr float base=100;
  return log((base-1)*x+1.0f)/log(base);
}

inline float scaleFuncDb(float y) {
  return log10(y)*20.0f/70.0f+1;
}

void FurnaceGUI::drawSpectrum() {
  if (nextWindow==GUI_WINDOW_SPECTRUM) {
    spectrumOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!spectrumOpen) return;
  if (ImGui::Begin("Spectrum",&spectrumOpen,globalWinFlags,_("Spectrum"))) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0.0f,0.0f));
    ImDrawList* dl=ImGui::GetWindowDrawList();
    ImVec2 origin=ImGui::GetWindowPos(), size=ImGui::GetWindowSize();
    float titleBar=ImGui::GetCurrentWindow()->TitleBarHeight;
    origin.y+=titleBar;
    size.y-=titleBar;
    // x scale labels (precalc)
    if (spectrum.showXScale) size.y-=ImGui::GetFontSize();
    // v scale labels
    if (spectrum.showYScale) {
      float padding=ImGui::GetStyle().FramePadding.x;
      ImVec2 p1, textSize;
      char buf[16];
      textSize=ImGui::CalcTextSize("-100");
      textSize.x+=5.f;
      int lineOffset=spectrum.yOffset*8;
      for (int z=lineOffset; z<=7+lineOffset; z++) {
        p1.y=origin.y+size.y*((z+1)/8.f-spectrum.yOffset)-textSize.y/2.0f;
        p1.x=origin.x+padding;
        snprintf(buf,16,"-%2d",(z+1)*10);
        dl->AddText(p1,ImGui::GetColorU32(ImGuiCol_Text),buf);
      }
      origin.x+=textSize.x+padding;
      size.x-=textSize.x+padding;
    }
    // y grid
    if (spectrum.showYGrid) {
      int lines=((size.y>450)?16:8);
      float offset=fmod(spectrum.yOffset*lines,1.0f)/lines;
      for (unsigned char z=1; z<=lines; z++) {
        dl->AddLine(
          origin+ImVec2(0,size.y*((float)z/lines-offset)),
          origin+ImVec2(size.x,size.y*((float)z/lines-offset)),
          0x55ffffff);
      }
    }
    // x grid
    if (spectrum.showXGrid || spectrum.showXScale) {
      char buf[16];
      float pos=0, prevPos=0, i;
      for (size_t j=0; j<spectrum.frequencies.size(); j++) {
        i=spectrum.frequencies[j];
        ImU32 color=0x22ffffff;
        pos=spectrum.xZoom*size.x*(scaleFuncLog(i/(e->getAudioDescGot().rate/2.0f))-spectrum.xOffset);
        if (pos>size.x) break;
        if (j%9==0) color=0x55ffffff;
        if (spectrum.showXScale) {
          if (i>=1000) {
            snprintf(buf,16,"%dk",(int)i/1000);
          } else {
            snprintf(buf,16,"%d",(int)i);
          }
          float x=ImGui::CalcTextSize(buf).x;
          if (pos-prevPos>x || j%9==0) dl->AddText(
            origin+ImVec2(pos-x/2,size.y),
            ImGui::GetColorU32(ImGuiCol_Text),
            buf
          );
        }
        if (spectrum.showXGrid) dl->AddLine(
          origin+ImVec2(pos,0),
          origin+ImVec2(pos,size.y),
          color);
        prevPos=pos;
      }
    }
    int chans=e->getAudioDescGot().outChans;
    if (spectrum.update) {
      spectrum.update=false;
      spectrum.running=true;
      if (spectrum.buffer) {
        fftw_free(spectrum.buffer);
        spectrum.buffer=NULL;
      }
      if (spectrum.in) {
        delete[] spectrum.in;
        spectrum.in=NULL;
      }
      if (spectrum.plot) {
        delete[] spectrum.plot;
        spectrum.plot=NULL;
      }
      spectrum.buffer=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*spectrum.bins);
      if (!spectrum.buffer) spectrum.running=false;
      spectrum.in=new double[spectrum.bins];
      if (!spectrum.in) spectrum.running=false;
      spectrum.plot=new ImVec2[spectrum.bins/2];
      if (!spectrum.plot) spectrum.running=false;
      spectrum.plan=fftw_plan_dft_r2c_1d(spectrum.bins,spectrum.in,spectrum.buffer,FFTW_ESTIMATE);
      spectrum.frequencies.clear();
      float freq;
      float maxRate=e->getAudioDescGot().rate/2;
      for (int j=10; j<maxRate; j*=10) {
        for (int i=1; i<10; i++) {
          freq = i*j;
          if (freq>maxRate) break;
          spectrum.frequencies.push_back((int)freq);
        }
        if (freq>maxRate) break;
      }
      if (spectrum.running) logV("spectrum ready!");
    }
    if (spectrum.running) {
      for (int z=spectrum.mono?0:(chans-1); z>=0; z--) {
        // get buffer
        memset(spectrum.in, 0, sizeof(double)*spectrum.bins);
        int needle=e->oscReadPos-spectrum.bins;

        for (int j=0; j<spectrum.bins; j++) {
          int pos=(needle+j)&0x7fff;
          double sample=0.0;
          if (spectrum.mono) {
            for (int i=0; i<chans; i++) {
              sample+=e->oscBuf[i][pos];
            }
            sample/=chans;
          } else {
            sample+=e->oscBuf[z][pos];
          }
          spectrum.in[j]=sample*(0.5*(1.0-cos(2.0*M_PI*j/(spectrum.bins-1))));
        }
        fftw_execute(spectrum.plan);
        unsigned int count=0;
        float mag=0.0f, x=0.0f, y=0.0f;
        count=spectrum.bins/2;
        for (unsigned int i=0; i<count; i++) {
          x=spectrum.xZoom*size.x*(scaleFuncLog((float)i/count)-spectrum.xOffset);
          mag=2.0f*sqrt(spectrum.buffer[i][0]*spectrum.buffer[i][0]+spectrum.buffer[i][1]*spectrum.buffer[i][1])/count;
          y=1.0-scaleFuncDb(mag);
          spectrum.plot[i].x=origin.x+x;
          spectrum.plot[i].y=origin.y+size.y*(y-spectrum.yOffset);
        }
        ImGui::PushClipRect(origin, origin+size, true);
        dl->AddPolyline(spectrum.plot, count, ImGui::GetColorU32(uiColors[spectrum.mono?GUI_COLOR_OSC_WAVE:GUI_COLOR_OSC_WAVE_CH0+z]), 0, 1.0f);
        dl->PathFillConcave(ImGui::GetColorU32(uiColors[spectrum.mono?GUI_COLOR_OSC_WAVE:GUI_COLOR_OSC_WAVE_CH0+z]));
        ImGui::PopClipRect();
      }
    }
    ImGui::PopStyleVar();
    if (ImGui::IsWindowHovered()) {
      ImGui::SetCursorPosX(ImGui::GetWindowSize().x-ImGui::GetStyle().ItemSpacing.x-ImGui::CalcTextSize(ICON_FA_BARS).x);
      ImGui::TextUnformatted(ICON_FA_BARS "##spectrumSettings");
    }
    if (ImGui::BeginPopupContextItem("spectrumSettingsPopup",ImGuiPopupFlags_MouseButtonLeft)) {
      ImGui::Checkbox(_("Mono##spec"), &spectrum.mono);
      if (ImGui::InputScalar("Bins",ImGuiDataType_U32,&spectrum.bins)) {
        if (spectrum.bins<32) spectrum.bins=32;
        if (spectrum.bins>32768) spectrum.bins=32768;
        spectrum.update=true;
      }
      ImGui::Separator();
      if (ImGui::SliderFloat("X Zoom", &spectrum.xZoom, 1.0f, 10.0f)) {
        if (spectrum.xZoom<1.0f) spectrum.xZoom=1.0f;
        if (spectrum.xZoom>10.0f) spectrum.xZoom=10.0f;
      }
      if (ImGui::SliderFloat("X Offset", &spectrum.xOffset, 0.0f, 1.0f)) {
        if (spectrum.xOffset<0.0f) spectrum.xOffset=0.0f;
        if (spectrum.xOffset>1.0f) spectrum.xOffset=1.0f;
      }
      ImGui::Separator();
      if (ImGui::SliderFloat("Y Offset", &spectrum.yOffset, 0.0f, 1.0f)) {
        if (spectrum.yOffset<0.0f) spectrum.yOffset=0.0f;
        if (spectrum.yOffset>1.0f) spectrum.yOffset=1.0f;
      }
      ImGui::Separator();
      ImGui::Checkbox(_("Show X Grid"), &spectrum.showXGrid);
      ImGui::Checkbox(_("Show Y Grid"), &spectrum.showYGrid);
      ImGui::Checkbox(_("Show X Scale"), &spectrum.showXScale);
      ImGui::Checkbox(_("Show Y Scale"), &spectrum.showYScale);
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SPECTRUM;
  ImGui::End();
}
