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
  DivFilePlayer* fp=e->getFilePlayer();
  if (nextWindow==GUI_WINDOW_REF_PLAYER) {
    refPlayerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  fp->setActive(refPlayerOpen);
  if (!refPlayerOpen) return;

  if (ImGui::Begin("Music Player",&refPlayerOpen,globalWinFlags,_("Music Player"))) {
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
    if (fp->isLoaded()) {
      if (playPosNegative) {
        ImGui::Text("-%d:%02d:%02d.%03d",posHours,posMinutes,posSeconds,posMillis);
      } else {
        ImGui::Text("%d:%02d:%02d.%03d",posHours,posMinutes,posSeconds,posMillis);
      }
    } else {
      ImGui::TextUnformatted(_("no file loaded"));
    }

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderScalar("##Position",ImGuiDataType_U64,&playPos,&minPos,&maxPos,"")) {
      fp->setPos(playPos);
    }

    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##Open")) {
      openFileDialog(GUI_FILE_MUSIC_OPEN);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      e->synchronizedSoft([this,fp]() {
        if (!fp->closeFile()) {
          showError(_("you haven't loaded a file!"));
        }
      });
    }
    ImGui::SetItemTooltip(_("open file\n(right click to unload current file)"));
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_STEP_BACKWARD)) {
      // handled outside
    }
    if (fp->isPlaying()) {
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        int cueSeconds=0;
        int cueMicros=0;
        fp->stop();
        e->getFilePlayerCue(cueSeconds,cueMicros);
        fp->setPosSeconds(cueSeconds,cueMicros);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        fp->stop();
        fp->setPos(0);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        int cueSeconds=0;
        int cueMicros=0;
        e->getFilePlayerCue(cueSeconds,cueMicros);
        fp->setPosSeconds(cueSeconds,cueMicros);
      }
      ImGui::SetItemTooltip(
        _("left click: go to cue position\n"
        "middle click: go to beginning\n"
        "right click: go to cue position (but don't stop)")
      );
    } else {
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        // try setting cue pos
        ssize_t curSeconds=0;
        unsigned int curMicros=0;
        fp->getPosSeconds(curSeconds,curMicros);
        DivSongTimestamps::Timestamp rowTS=e->curSubSong->ts.getTimes(curOrder,0);
        if (rowTS.seconds==-1) {
          showError("the first row of this order isn't going to play.");
        } else {
          // calculate difference and set cue pos
          curSeconds-=rowTS.seconds;
          int curMicrosI=curMicros-rowTS.micros;
          while (curMicrosI<0) {
            curMicrosI+=1000000;
            curSeconds--;
          }
          e->setFilePlayerCue(curSeconds,curMicrosI);
        }
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        fp->setPos(0);
      }
      if (ImGui::BeginPopupContextItem("Edit Cue Position",ImGuiPopupFlags_MouseButtonRight)) {
        ImGui::Text("Edit me");
        if (ImGui::Button("OK")) {
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      ImGui::SetItemTooltip(
        _("left click: set cue position here\n"
        " - current playback time becomes position at first row of current order\n"
        "middle click: go to beginning\n"
        "right click: fine edit cue position")
      );
    }
    ImGui::SameLine();
    if (fp->isPlaying()) {
      pushToggleColors(true);
      if (ImGui::Button(ICON_FA_PAUSE "##Pause")) {
        fp->stop();
      }
      ImGui::SetItemTooltip(_("pause"));
      popToggleColors();
    } else {
      if (ImGui::Button(ICON_FA_PLAY "##Play")) {
        fp->play();
      }
      ImGui::SetItemTooltip(_("play"));
    }
    ImGui::SameLine();

    pushToggleColors(filePlayerSync);
    if (ImGui::Button(_("Sync"))) {
      filePlayerSync=!filePlayerSync;
    }
    ImGui::SetItemTooltip(_("synchronize playback with tracker playback"));
    popToggleColors();
    e->setFilePlayerSync(filePlayerSync);

    ImGui::SameLine();
    ImGui::Text(_("Mix:"));
    
    float vol=fp->getVolume();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::SliderFloat("##Volume",&vol,-1.0f,1.0f,_("<-- Tracker / Reference -->"))) {
      if (vol<-1.0f) vol=-1.0f;
      if (vol>1.0f) vol=1.0f;
      fp->setVolume(vol);
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      fp->setVolume(0.0f);
    }

    //ImGui::Text("Memory usage: %" PRIu64 "K",fp->getMemUsage()>>10);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_REF_PLAYER;
  ImGui::End();


  if (!refPlayerOpen) {
    fp->stop();
    e->setFilePlayerSync(false);
  }
}
