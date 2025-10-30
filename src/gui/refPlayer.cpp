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
#include <fmt/printf.h>
#include "imgui.h"
#include "IconsFontAwesome4.h"

void FurnaceGUI::drawRefPlayer() {
  if (nextWindow==GUI_WINDOW_REF_PLAYER) {
    refPlayerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!refPlayerOpen) return;
  if (ImGui::Begin("Music Player",&refPlayerOpen,globalWinFlags,_("Music Player"))) {
    DivFilePlayer* fp=e->getFilePlayer();

    bool playPosNegative=false;
    ssize_t playPos=fp->getPos();
    if (playPos<0) {
      playPos=-playPos;
      playPosNegative=true;
    }
    size_t minPos=0;
    size_t maxPos=fp->getFileInfo().frames;
    int fileRate=fp->getFileInfo().samplerate;
    if (fileRate<1) fileRate=1;
    int posHours=(playPos/fileRate)/3600;
    int posMinutes=((playPos/fileRate)/60)%60;
    int posSeconds=(playPos/fileRate)%60;
    int posMillis=(1000*(playPos%fileRate))/fileRate;
    if (playPosNegative) {
      ImGui::Text("-%d:%02d:%02d.%03d",posHours,posMinutes,posSeconds,posMillis);
    } else {
      ImGui::Text("%d:%02d:%02d.%03d",posHours,posMinutes,posSeconds,posMillis);
    }

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderScalar("##Position",ImGuiDataType_U64,&playPos,&minPos,&maxPos,"")) {
      fp->setPos(playPos);
    }

    if (ImGui::Button("Open")) {
      openFileDialog(GUI_FILE_MUSIC_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FAST_BACKWARD)) {
      fp->stop();
      fp->setPos(0);
    }
    ImGui::SameLine();
    if (fp->isPlaying()) {
      pushToggleColors(true);
      if (ImGui::Button(ICON_FA_PAUSE "##Pause")) {
        fp->stop();
      }
      popToggleColors();
    } else {
      if (ImGui::Button(ICON_FA_PLAY "##Play")) {
        fp->play();
      }
    }
    ImGui::SameLine();

    pushToggleColors(e->getFilePlayerSync());
    if (ImGui::Button(_("Sync"))) {
      e->setFilePlayerSync(!e->getFilePlayerSync());
    }
    popToggleColors();
    
    float vol=fp->getVolume();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderFloat("##Volume",&vol,0.0f,1.0f)) {
      if (vol<0.0f) vol=0.0f;
      if (vol>1.0f) vol=1.0f;
      fp->setVolume(vol);
    }

    //ImGui::Text("Memory usage: %" PRIu64 "K",fp->getMemUsage()>>10);

    if (!refPlayerOpen) {
      fp->stop();
      e->setFilePlayerSync(false);
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_REF_PLAYER;
  ImGui::End();
}
