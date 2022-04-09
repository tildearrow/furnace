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
#include <math.h>

void FurnaceGUI::readOsc() {
  int writePos=e->oscWritePos;
  int readPos=e->oscReadPos;
  int avail=0;
  int total=0;
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
  for (int i=0; i<512; i++) {
    int pos=(readPos+(i*total/512))&0x7fff;
    oscValues[i]=(e->oscBuf[0][pos]+e->oscBuf[1][pos])*0.5f;
  }

  float peakDecay=0.05f*60.0f*ImGui::GetIO().DeltaTime;
  for (int i=0; i<2; i++) {
    peak[i]*=1.0-peakDecay;
    if (peak[i]<0.0001) peak[i]=0.0;
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
  ImGui::SetNextWindowSizeConstraints(ImVec2(64.0f*dpiScale,32.0f*dpiScale),ImVec2(scrW*dpiScale,scrH*dpiScale));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,ImVec2(0,0));
  if (ImGui::Begin("Oscilloscope",&oscOpen)) {
    //ImGui::SetCursorPos(ImVec2(0,0));
    ImGui::BeginDisabled();
    ImGui::PlotLines("##SingleOsc",oscValues,512,0,NULL,-1.0f,1.0f,ImGui::GetContentRegionAvail());
    ImGui::EndDisabled();
  }
  ImGui::PopStyleVar(3);
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_OSCILLOSCOPE;
  ImGui::End();
}