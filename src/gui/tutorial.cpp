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
#include "../ta-log.h"
#include "IconsFontAwesome4.h"

#define TS FurnaceGUITutorialStep

void FurnaceGUI::initTutorial() {
  tutorials[GUI_TUTORIAL_OVERVIEW]=FurnaceGUITutorialDef("Overview",{
    TS(
      "hello! this is the Furnace tutorial!\n"
      "I will teach you how to use Furnace.\n"
      "if you wish to skip these tutorials, click on one of the Skip buttons. otherwise, click the " ICON_FA_CHEVRON_RIGHT " icon to continue."
    ),
    TS(
      "Furnace is a chiptune tracker!\n"
      "in a tracker, the song is written down in patterns, which are lists of notes, instruments and effects to play.\n"
      "a playhead scrolls down through these patterns, and plays the notes that are written in them as it moves.\n"
      "think of it as a piano roll, but replace the dots in the position of notes with note names (e.g. A-4)."
    ),
    TS(
      "these are called \"patterns\" because they may appear more than once in the song.\n"
      "this is useful for (in example) percussion, avoiding duplication.\n"
      "the order in which these patterns appear is determined by an order list which also scrolls down as the playhead moves between patterns."
    ),
    TS(
      "this is the Pattern window. it displays a pattern (which contains the notes and stuff)."
    ),
    TS(
      "this is the Orders window. it displays which patterns are going to play."
    ),
    TS(
      "this is the Instruments window. it shows a list of instruments (sounds) in the song."
    ),
    TS(
      "this is the Song Information window, which allows you to change some song properties."
    ),
    TS(
      "this is the Speed window, which contains speed parameters."
    ),
    TS(
      "and here are the Play Controls. you can use these to play the song and do some other things.\n"
      "now that I am done explaining the interface, let's make a song!"
    ),
    TS(
      "we'll start by creating an instrument.\n"
      "while it's possible to work without instruments, we would be stuck with the default sound!\n"
      "so let's create one... for that you click on the " ICON_FA_PLUS " button in the Instruments window."
    ),
    TS(
      ""
    ),
    TS(
      "now let's type in some notes!\n"
      "first, enable Edit Mode with the Space key.\n"
      "after that, click on the pattern editor to focus it (the first empty column to be specific).\n"
    ),
    TS(
      "the keyboard layout for inserting notes resembles that of a piano:\n"
      "- Z-M: low octave\n"
      "- Q-U: mid octave\n"
      "- I-P and beyond: high octave\n"
      "the keys one row above the mentioned octaves are the upper keys of a piano.\n"
      "let's press some of these keys to insert notes!"
    ),
    TS(
      ""
    ),
    TS(
      "and now, let's play the song! hit Enter to play it (and press it while playing to stop)."
    ),
    TS(
      ""
    ),
    TS(
      "great!\n"
      "by the way, you can move around the pattern view using the arrow keys or by clicking in the pattern."
    ),
    TS(
      "now let me teach you about these columns in the pattern editor.\n"
      "each big column represents a channel.\n"
      "a channel consists of five (or more) smaller columns:\n"
      "- the first column contains notes.\n"
      "- the second one represents the instruments to play (these will be filled in automatically when you have an instrument selected).\n"
      "- the third one has volume changes (will explain these later).\n"
      "- the last two ones are effects and effect values (you can have multiple effect columns, but that's something I will be covering later as well.)\n"
      "the instrument, volume and effect columns are continuous. this means that if nothing is specified, the last value will be used."
    ),
    TS(
      "let's add some volume changes to show you what I mean."
    ),
    TS(
      ""
    ),
    TS(
      "as you can hear, volume changes affects all notes which are placed next to, or after it.\n"
      "let's place some bass notes."
    ),
    TS(
      ""
    ),
    TS(
      "oh wait! the keyboard layout only contains 2 octaves!\n"
      "however, we can change the octave range by using the Octave setting in the play/edit controls."
    ),
    TS(
      ""
    ),
    TS(
      "now let's type in the notes..."
    ),
    TS(
      ""
    ),
    TS(
      "...and play this?"
    ),
    TS(
      ""
    ),
    TS(
      "cool, huh? but we're running out of space here.\n"
      "let's expand the song."
    ),
    TS(
      "for that we go in the Orders window, which contains the order these patterns will appear in.\n"
      "the first column is position while the other columns contain pattern numbers of every channel."
    ),
    TS(
      "clicking on the " ICON_FA_PLUS " button adds one row to the orders..."
    )
    TS(
      ""
    ),
    TS(
      "and then clicking on a cell of the first column takes us there."
    ),
    TS(
      ""
    ),
    TS(
      "now let's add more to the song."
    ),
    TS(
      ""
    ),
    TS(
      "and after that, let's hear it!\n"
      "note that playing the song by pressing Enter will play from the current position in the Orders view - that is, order 1!\n"
      "we want to play from order 0 (the first order), so we go back to it first and then hit Enter."
    ),
    TS(
      ""
    ),
    TS(
      "congratulations! you made it\n"
      "what else?"
    )
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
    if (ImGui::Begin("Tutorial",NULL,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar)) {
      FurnaceGUITutorialStep& step=tutorials[curTutorial].steps[curTutorialStep];
      ImGui::Dummy(ImVec2(400.0*dpiScale,1.0));
      ImGui::TextWrapped("%s",step.text);

      if (ImGui::Button(ICON_FA_CHEVRON_RIGHT)) {
        curTutorialStep++;
        if (step.runAfter!=NULL) step.runAfter();
        if (curTutorialStep>=(int)tutorials[curTutorial].steps.size()) {
          tutorial.taken[curTutorial]=true;
          curTutorial=-1;
          curTutorialStep=0;
        }
      }
    }
    ImGui::End();
  }
}

FurnaceGUITutorialDef::FurnaceGUITutorialDef(const char* n, std::initializer_list<FurnaceGUITutorialStep> step):
  name(n) {
  steps=step;
}
