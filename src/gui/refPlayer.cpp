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
#include "misc/cpp/imgui_stdlib.h"

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
        fp->stop();
        fp->setPosSeconds(e->getFilePlayerCue());
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        fp->stop();
        fp->setPos(0);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        fp->setPosSeconds(e->getFilePlayerCue());
      }
      ImGui::SetItemTooltip(
        _("left click: go to cue position\n"
        "middle click: go to beginning\n"
        "right click: go to cue position (but don't stop)")
      );
    } else {
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        // try setting cue pos
        TimeMicros curPos=fp->getPosSeconds();
        TimeMicros rowTS=e->curSubSong->ts.getTimes(curOrder,0);
        if (rowTS.seconds==-1) {
          showError(_("the first row of this order isn't going to play."));
        } else {
          e->setFilePlayerCue(curPos-rowTS);
        }
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
        fp->setPos(0);
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        TimeMicros cueTime=e->getFilePlayerCue();
        fpCueInput=cueTime.toString(-1,TA_TIME_FORMAT_AUTO);
      }
      if (ImGui::BeginPopupContextItem("Edit Cue Position",ImGuiPopupFlags_MouseButtonRight)) {
        ImGui::TextUnformatted(_("Set cue position at first order:"));
        TimeMicros cueTime=e->getFilePlayerCue();
        bool altered=false;
        pushWarningColor(false,fpCueInputFailed);
        if (ImGui::InputText("##CuePos",&fpCueInput)) {
          fpCueInputFailed=false;
          try {
            cueTime=TimeMicros::fromString(fpCueInput);
            altered=true;
          } catch (std::invalid_argument& e) {
            fpCueInputFailed=true;
            fpCueInputFailReason=e.what();
          }
        }
        if (!ImGui::IsItemActive()) {
          fpCueInputFailed=false;
        }
        if (ImGui::IsItemHovered() && fpCueInputFailed) {
          ImGui::SetTooltip("%s",fpCueInputFailReason.c_str());
        }
        popWarningColor();
        if (altered) {
          e->setFilePlayerCue(cueTime);
        }
        if (ImGui::Button(_("OK"))) {
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
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        // try setting cue pos
        TimeMicros curPos=fp->getPosSeconds();
        TimeMicros rowTS=e->curSubSong->ts.getTimes(curOrder,0);
        if (rowTS.seconds==-1) {
          showError(_("the first row of this order isn't going to play."));
        } else {
          e->setFilePlayerCue(curPos-rowTS);
          fp->stop();
        }
      }
      ImGui::SetItemTooltip(_("pause\n(right click to set cue position and pause)"));
      popToggleColors();
    } else {
      if (ImGui::Button(ICON_FA_PLAY "##Play")) {
        fp->play();
      }
      ImGui::SetItemTooltip(_("play"));
    }
    ImGui::SameLine();

    if (ImGui::Button(ICON_FA_STEP_FORWARD "##PlayPos")) {
      // handled outside
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      TimeMicros rowTS;
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        rowTS=e->curSubSong->ts.getTimes(cursor.order,cursor.y);
      } else {
        rowTS=e->curSubSong->ts.getTimes(curOrder,0);
      }
      TimeMicros cueTime=e->getFilePlayerCue();
      if (rowTS.seconds==-1) {
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
          showError(_("the row that the pattern cursor is at isn't going to play. try moving the cursor."));
        } else {
          showError(_("the first row of this order isn't going to play. try another order."));
        }
      } else {
        fp->setPosSeconds(rowTS+cueTime);
        fp->play();
      }
    }
    if (ImGui::IsItemHovered() && (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right))) {
      fp->stop();
    }
    ImGui::SetItemTooltip(_(
      "hold left click to play from current order\n"
      "hold right click to play from pattern cursor position\n"
      "release mouse button to stop"
    ));
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
    ImGui::SetItemTooltip(_("right click to reset"));

    //ImGui::Text("Memory usage: %" PRIu64 "K",fp->getMemUsage()>>10);
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_REF_PLAYER;
  ImGui::End();


  if (!refPlayerOpen) {
    fp->stop();
    e->setFilePlayerSync(false);
  }
}
