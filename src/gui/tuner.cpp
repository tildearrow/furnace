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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#define FURNACE_TUNER_FFT_SIZE 8192

void FurnaceGUI::drawTuner() {
  if (nextWindow == GUI_WINDOW_TUNER) {
    tunerOpen = true;
    ImGui::SetNextWindowFocus();
    nextWindow = GUI_WINDOW_NOTHING;
  }
  if (!tunerOpen) return;
  if (ImGui::Begin("Tuner", &tunerOpen, globalWinFlags, _("Tuner"))) {

    //fft buffer
    if (!tunerFFTInBuf) tunerFFTInBuf=new double[FURNACE_TUNER_FFT_SIZE];
    if (!tunerFFTOutBuf) tunerFFTOutBuf=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*FURNACE_TUNER_FFT_SIZE);

    if (!tunerPlan) {
      tunerPlan=fftw_plan_dft_r2c_1d(FURNACE_TUNER_FFT_SIZE, tunerFFTInBuf, tunerFFTOutBuf, FFTW_ESTIMATE);
    }

    int chans=e->getAudioDescGot().outChans;
    int needle=e->oscReadPos;

    for (int j=0; j<FURNACE_TUNER_FFT_SIZE; j++) {
      int pos=(needle-FURNACE_TUNER_FFT_SIZE+j) & 0x7fff;
      double sample=0.0;
      for (int ch=0; ch<chans; ch++) {
        sample+=e->oscBuf[ch][pos];
      }
      sample=sample/chans;

      tunerFFTInBuf[j]=sample*(0.5*(1.0-cos(2.0*M_PI*j/(FURNACE_TUNER_FFT_SIZE-1))));
    }

    fftw_execute(tunerPlan);

    std::vector<double> mag(FURNACE_TUNER_FFT_SIZE/2);
    for (int k=0; k < FURNACE_TUNER_FFT_SIZE / 2; k++) {
      mag[k]=sqrt(tunerFFTOutBuf[k][0]*tunerFFTOutBuf[k][0]+tunerFFTOutBuf[k][1]*tunerFFTOutBuf[k][1]);
    }

    //harmonic product spectrum
    int harmonics=2;
    for (int h=2; h<=harmonics; h++) {
      for (int k=0; k<FURNACE_TUNER_FFT_SIZE/(2*h); k++) {
        mag[k]*=mag[k*h];
      }
    }

    //peak with interpolation
    int peakIndex = std::distance(mag.begin(), std::max_element(mag.begin(), mag.end()));
    double sampleRate = e->getAudioDescGot().rate;

    double freq = 0.0;
    if (peakIndex > 0 && peakIndex < (int)mag.size() - 1) {
      double alpha = mag[peakIndex - 1];
      double beta = mag[peakIndex];
      double gamma = mag[peakIndex + 1];
      double p = 0.5 * (alpha - gamma) / (alpha - 2.0 * beta + gamma);
      freq = (peakIndex + p) * (sampleRate / (double)FURNACE_TUNER_FFT_SIZE);
    }
    else {
      freq = (double)peakIndex * (sampleRate / (double)FURNACE_TUNER_FFT_SIZE);
    }

    
    //tuning formulas
    double noteExact=0;
    int noteRounded=0;
    double cents=0.0f;
    if (freq > 0 && freq < 5000.0) {
      noteExact=CLAMP(log2(freq / e->song.tuning) * 12.0 + 117.0,0,180);
      noteRounded=round(noteExact);
      cents=noteExact-noteRounded;
    }
    String noteText=fmt::sprintf("%s",(freq > 0 && freq < 5000.0)?noteName(noteExact):"---");
    ImGui::Text("Note: %s", noteText.c_str());
    ImGui::Text("Freq: %f Hz", freq);
    ImGui::Text("Cents: %f ", cents*100.0f);

    {
      ImDrawList* dl=ImGui::GetWindowDrawList();
      ImVec2 origin=ImGui::GetWindowPos();
      ImVec2 size=ImGui::GetWindowSize();
      float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
      origin.y+=titleBar;
      size.y-=titleBar;
      // debug stuff
      ImVec2* plot=new ImVec2[FURNACE_TUNER_FFT_SIZE];
      for (size_t i=0; i<FURNACE_TUNER_FFT_SIZE; i++) {
        plot[i].x=origin.x+size.x*((float)i/FURNACE_TUNER_FFT_SIZE);
        plot[i].y=origin.y+size.y-size.y*(tunerFFTInBuf[i]+1.0f)/2.0f;
      }
      dl->AddPolyline(plot, FURNACE_TUNER_FFT_SIZE, 0xff00ff00, 0, 1.0f);
      for (size_t i=0; i<mag.size(); i++) {
        plot[i].x=origin.x+size.x*((float)i/mag.size());
        plot[i].y=origin.y+size.y-size.y*mag[i]/FURNACE_TUNER_FFT_SIZE;
      }
      dl->AddPolyline(plot, mag.size(), 0xffffffff, 0, 1.0f);
      delete[] plot;
      ImVec2 needleCenter=origin+ImVec2(size.x/2,size.y/4*3);
      float needleLength = (size.x/2)*0.9f;
      const float halfSpan=0.7; // radians
      const float trim=.2;
      float angle=cents*halfSpan;
      ImVec2 needleTip = needleCenter + ImVec2(
        needleLength*sin(angle),
        -needleLength*cos(angle)
      );
      // text
      ImGui::PushFont(bigFont);
      ImVec2 textSize=ImGui::CalcTextSize(noteText.c_str());
      dl->AddText(needleCenter-ImVec2(textSize.x/2.0f,textSize.y/2.0f),ImGui::ColorConvertFloat4ToU32(uiColors[GUI_COLOR_TEXT]),noteText.c_str());
      ImGui::PopFont();
      // needle
      needleCenter = needleCenter+ImVec2(
        needleLength*sin(angle)*trim,
        -needleLength*cos(angle)*trim
      );

      dl->AddLine(needleCenter, needleTip, 0xff00ffff, 2.0f);
    }
  }

  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
    curWindow = GUI_WINDOW_TUNER;

  ImGui::End();
}

