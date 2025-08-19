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
#include "../ta-log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#define FURNACE_FFT_SIZE 16384//4096
#define FURNACE_FFT_RATE 80.0
#define FURNACE_FFT_CUTOFF 0.1

void FurnaceGUI::drawTuner() {
  if (nextWindow == GUI_WINDOW_TUNER) {
    tunerOpen = true;
    ImGui::SetNextWindowFocus();
    nextWindow = GUI_WINDOW_NOTHING;
  }
  if (!tunerOpen) return;
  if (ImGui::Begin("Tuner", &tunerOpen, globalWinFlags, _("Tuner"))) {

    //fft buffer
    static double inBuf[FURNACE_FFT_SIZE];
    static fftw_complex outBuf[FURNACE_FFT_SIZE];
    static fftw_plan plan=nullptr;

    if (!plan) {
      plan=fftw_plan_dft_r2c_1d(FURNACE_FFT_SIZE, inBuf, outBuf, FFTW_ESTIMATE);
    }

    int chans=e->getAudioDescGot().outChans;
    int needle=e->oscWritePos;

    for (int j=0; j<FURNACE_FFT_SIZE; j++) {
      int pos=(needle-j) & 0x7fff;
      double sample=0.0;
      for (int ch=0; ch<chans; ch++) {
        sample+=e->oscBuf[ch][pos];
      }
      sample=sample/chans;

      inBuf[j]=sample * (0.5 * (1.0 - cos(2.0 * M_PI * j / (FURNACE_FFT_SIZE - 1))));
    }

    fftw_execute(plan);

    std::vector<double> mag(FURNACE_FFT_SIZE / 2);
    for (int k=0; k < FURNACE_FFT_SIZE / 2; k++) {
      mag[k]=sqrt(outBuf[k][0]*outBuf[k][0]+outBuf[k][1]*outBuf[k][1]);
    }

    //harmonic product spectrum
    int harmonics = 2;
    for (int h = 2; h <= harmonics; h++) {
      for (int k = 0; k < FURNACE_FFT_SIZE / (2 * h); k++) {
        mag[k] *= mag[k * h];
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
      freq = (peakIndex + p) * (sampleRate / (double)FURNACE_FFT_SIZE);
    }
    else {
      freq = (double)peakIndex * (sampleRate / (double)FURNACE_FFT_SIZE);
    }

    
    //tuning formulas
    if (freq > 0 && freq < 5000.0) {
      double noteExact = log2(freq / 440.0) * 12.0 + 45.0;
      int noteRounded = (int)std::round(noteExact);

      int noteInOct = noteRounded % 12;
      if (noteInOct < 0) noteInOct += 12;

      int octave = noteRounded / 12;
      if (noteRounded < 0 && (noteRounded % 12)) --octave;

      double cents = (noteExact - noteRounded) * 100.0;

      static const char* names[12] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
      };



      ImGui::Text("Note: %s%d", names[noteInOct], octave);
      ImGui::Text("Freq: %.2f Hz", freq);
      ImGui::Text("Cents offset: %+0.1f", cents);
    }
    else {
      ImGui::Text("Note: -");
      ImGui::Text("Freq: 0.00 Hz");
      ImGui::Text("Cents offset: +0.0");
    }
  }

  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
    curWindow = GUI_WINDOW_TUNER;

  ImGui::End();
}

