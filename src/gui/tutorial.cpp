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
#include "../ta-log.h"

#define TS FurnaceGUITutorialStep

void FurnaceGUI::initTutorial() {
  tutorials[GUI_TUTORIAL_OVERVIEW]=FurnaceGUITutorialDef("Overview",{
    TS("step 1"),
    TS("step 2"),
    TS("step 3")
  });
}

void FurnaceGUI::syncTutorial() {
//  tutorial.userComesFrom=e->getConfInt("tutUserComesFrom",0);
  tutorial.introPlayed=e->getConfBool("tutIntroPlayed",false);
//  tutorial.welcome=e->getConfBool("tutWelcome",false);
}

void FurnaceGUI::commitTutorial() {
//  e->setConf("tutUserComesFrom",tutorial.userComesFrom);
  e->setConf("tutIntroPlayed",tutorial.introPlayed);
//  e->setConf("tutWelcome",tutorial.welcome);
}

void FurnaceGUI::activateTutorial(FurnaceGUITutorials which) {
  if (tutorial.welcome && !tutorial.taken[which] && !ImGui::IsPopupOpen(NULL,ImGuiPopupFlags_AnyPopupId|ImGuiPopupFlags_AnyPopupLevel) && curTutorial==-1 && introPos>=10.0) {
    logV("activating tutorial %d.",which);
    curTutorial=which;
    curTutorialStep=0;
  }
}

void FurnaceGUI::drawTutorial() {
  // welcome
  if (!tutorial.welcome) {
    ImGui::OpenPopup("Welcome");
  }
  if (ImGui::BeginPopupModal("Welcome",NULL,ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar)) {
    ImGui::PushFont(bigFont);
    ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Welcome!").x)*0.5);
    ImGui::Text("Welcome!");
    ImGui::PopFont();

    ImGui::Text("welcome to Furnace, the biggest open-source chiptune tracker!");

    ImGui::TextWrapped("get ready for the tutorial, which will teach you how to use it.");

    ImGui::TextWrapped(
      "there are two interface modes: Basic, and Advanced.\n"
      "the Basic Mode only shows essential features. use it if you are new to trackers or prefer simplicity.\n"
      "Advanced Mode allows you to use all Furnace features, but it may be confusing."
    );

    ImGui::TextWrapped("pick a mode to begin your journey! (you can always switch by going to Settings > Basic Mode)");

    if (ImGui::Button("Start in Basic Mode")) {
      basicMode=true;
      tutorial.welcome=true;
      commitTutorial();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::Button("Start in Advanced Mode")) {
      basicMode=false;
      tutorial.welcome=true;
      commitTutorial();
      ImGui::CloseCurrentPopup();
    }

    ImGui::TextWrapped("if you find any issues, be sure to report them! the issue tracker is here: https://github.com/tildearrow/furnace/issues");


    ImGui::SetWindowPos(ImVec2(
      (canvasW-ImGui::GetWindowSize().x)*0.5,
      (canvasH-ImGui::GetWindowSize().y)*0.5
    ));
    ImGui::EndPopup();
  }

  // tutorial
  if (curTutorial>=0 && curTutorial<GUI_TUTORIAL_MAX) {
    if (ImGui::Begin("Tutorial",NULL,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar)) {
      ImGui::Text("Tutorial...");
    }
    ImGui::End();
  }
}

FurnaceGUITutorialDef::FurnaceGUITutorialDef(const char* n, std::initializer_list<FurnaceGUITutorialStep> step):
  name(n) {
  steps=step;
}
