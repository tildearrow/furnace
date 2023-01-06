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
#include "intConst.h"

void FurnaceGUI::drawMixer() {
  if (nextWindow==GUI_WINDOW_MIXER) {
    mixerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!mixerOpen) return;
  ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f*dpiScale,200.0f*dpiScale),ImVec2(canvasW,canvasH));
  if (ImGui::Begin("Mixer",&mixerOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    char id[32];
    if (ImGui::SliderFloat("Master Volume",&e->song.masterVol,0,3,"%.2fx")) {
      if (e->song.masterVol<0) e->song.masterVol=0;
      if (e->song.masterVol>3) e->song.masterVol=3;
      MARK_MODIFIED;
    } rightClickable
    for (int i=0; i<e->song.systemLen; i++) {
      snprintf(id,31,"MixS%d",i);
      bool doInvert=e->song.systemVol[i]<0;
      float vol=fabs(e->song.systemVol[i]);
      ImGui::PushID(id);
      ImGui::Text("%d. %s",i+1,getSystemName(e->song.system[i]));
      ImGui::SameLine(ImGui::GetWindowWidth()-(82.0f*dpiScale));
      if (ImGui::Checkbox("Invert",&doInvert)) {
        e->song.systemVol[i]=-e->song.systemVol[i];
        MARK_MODIFIED;
      }
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (CWSliderFloat("Volume",&vol,0,2)) {
        if (doInvert) {
          if (vol<0.0001) vol=0.0001;
        }
        if (vol<0) vol=0;
        if (vol>10) vol=10;
        e->song.systemVol[i]=(doInvert)?-vol:vol;
        MARK_MODIFIED;
      } rightClickable
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (CWSliderFloat("Panning",&e->song.systemPan[i],-1.0f,1.0f)) {
        if (e->song.systemPan[i]<-1.0f) e->song.systemPan[i]=-1.0f;
        if (e->song.systemPan[i]>1.0f) e->song.systemPan[i]=1.0f;
        MARK_MODIFIED;
      } rightClickable
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
      if (CWSliderFloat("Front/Rear",&e->song.systemPanFR[i],-1.0f,1.0f)) {
        if (e->song.systemPanFR[i]<-1.0f) e->song.systemPanFR[i]=-1.0f;
        if (e->song.systemPanFR[i]>1.0f) e->song.systemPanFR[i]=1.0f;
        MARK_MODIFIED;
      } rightClickable

      ImGui::PopID();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MIXER;
  ImGui::End();
}
