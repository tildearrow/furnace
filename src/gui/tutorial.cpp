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

#include "gui.h"
#include <imgui.h>

void FurnaceGUI::syncTutorial() {
  tutorial.userComesFrom=e->getConfInt("tutUserComesFrom",0);
  tutorial.introPlayed=e->getConfBool("tutIntroPlayed",false);
  tutorial.protoWelcome=e->getConfBool("tutProtoWelcome",false);
}

void FurnaceGUI::commitTutorial() {
  e->setConf("tutUserComesFrom",tutorial.userComesFrom);
  e->setConf("tutIntroPlayed",tutorial.introPlayed);
  e->setConf("tutProtoWelcome",tutorial.protoWelcome);
}

void FurnaceGUI::drawTutorial() {
  if (!tutorial.protoWelcome) {
    ImGui::OpenPopup("Welcome");
  }
  if (ImGui::BeginPopupModal("Welcome",NULL,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar)) {
    ImGui::PushFont(bigFont);
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Welcome!").x)*0.5);
    ImGui::Text("Welcome!");
    ImGui::PopFont();

    ImGui::Text("welcome to Furnace, the biggest open-source chiptune tracker!");

    ImGui::TextWrapped("this welcome screen is temporary. 0.6pre5 will feature a tutorial.");

    ImGui::Separator();

    ImGui::TextWrapped("here are some tips to get you started:");
    
    ImGui::TextWrapped(
      "- add an instrument by clicking on + in Instruments\n"
      "- click on the pattern view to focus it\n"
      "- channel columns have the following, in this order: note, instrument, volume and effects\n"
      "- hit space bar while on the pattern to toggle Edit Mode\n"
      "- click on the pattern or use arrow keys to move the cursor\n"
      "- values (instrument, volume, effects and effect values) are in hexadecimal\n"
      "- hit enter to play/stop the song\n"
      "- extend the song by adding more orders in the Orders window\n"
      "- click on the Orders matrix to change the patterns of a channel (left click increases; right click decreases)"
    );

    ImGui::TextWrapped(
      "if you need help, you may:\n"
      "- read the (incomplete) manual: https://github.com/tildearrow/furnace/blob/master/papers/doc/README.md\n"
      "- ask for help in Discussions (https://github.com/tildearrow/furnace/discussions) or the Furnace Discord (https://discord.gg/EfrwT2wq7z)"
    );

    ImGui::Separator();

    ImGui::TextWrapped("if you find any issues, be sure to report them! the issue tracker is here: https://github.com/tildearrow/furnace/issues");

    ImGui::Separator();

    ImGui::TextWrapped(
      "there are two interface modes: Basic, and Advanced.\n"
      "the Basic Mode only shows essential features. use it if you are new to trackers or prefer simplicity.\n"
      "Advanced Mode allows you to use all Furnace features, but it may be confusing."
    );

    ImGui::TextWrapped("pick a mode to begin your journey! (you can always switch by going to Settings > Basic Mode)");

    if (ImGui::Button("Start in Basic Mode")) {
      basicMode=true;
      tutorial.protoWelcome=true;
      commitTutorial();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::Button("Start in Advanced Mode")) {
      basicMode=false;
      tutorial.protoWelcome=true;
      commitTutorial();
      ImGui::CloseCurrentPopup();
    }

    ImGui::SetWindowPos(ImVec2(
      (canvasW-ImGui::GetWindowSize().x)*0.5,
      (canvasH-ImGui::GetWindowSize().y)*0.5
    ));
    ImGui::EndPopup();
  }
}
