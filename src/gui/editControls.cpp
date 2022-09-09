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
#include "IconsFontAwesome4.h"
#include <imgui.h>

void FurnaceGUI::drawMobileControls() {
  float timeScale=1.0f/(60.0f*ImGui::GetIO().DeltaTime);
  if (mobileMenuOpen) {
    if (mobileMenuPos<0.999f) {
      WAKE_UP;
      mobileMenuPos+=MIN(0.1,(1.0-mobileMenuPos)*0.65)*timeScale;
    } else {
      mobileMenuPos=1.0f;
    }
  } else {
    if (mobileMenuPos>0.001f) {
      WAKE_UP;
      mobileMenuPos-=MIN(0.1,mobileMenuPos*0.65)*timeScale;
    } else {
      mobileMenuPos=0.0f;
    }
  }
  ImGui::SetNextWindowPos(portrait?ImVec2(0.0f,((1.0-mobileMenuPos*0.65)*scrH*dpiScale)-(0.16*scrW*dpiScale)):ImVec2(0.5*scrW*dpiScale*mobileMenuPos,0.0f));
  ImGui::SetNextWindowSize(portrait?ImVec2(scrW*dpiScale,0.16*scrW*dpiScale):ImVec2(0.16*scrH*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Mobile Controls",NULL,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
    float avail=portrait?ImGui::GetContentRegionAvail().y:ImGui::GetContentRegionAvail().x;
    ImVec2 buttonSize=ImVec2(avail,avail);

    const char* mobButtonName=ICON_FA_CHEVRON_RIGHT "##MobileMenu";
    if (portrait) mobButtonName=ICON_FA_CHEVRON_UP "##MobileMenu";
    if (mobileMenuOpen) {
      if (portrait) {
        mobButtonName=ICON_FA_CHEVRON_DOWN "##MobileMenu";
      } else {
        mobButtonName=ICON_FA_CHEVRON_LEFT "##MobileMenu";
      }
    }
    if (ImGui::Button(mobButtonName,buttonSize)) {
      mobileMenuOpen=!mobileMenuOpen;
    }

    if (!portrait) ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(e->isPlaying()));
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLAY "##Play",buttonSize)) {
      play();
    }
    ImGui::PopStyleColor();
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_STOP "##Stop",buttonSize)) {
      stop();
    }
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne",buttonSize)) {
      e->stepOne(cursor.y);
      pendingStepUpdate=true;
    }

    bool repeatPattern=e->getRepeatPattern();
    ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(repeatPattern));
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern",buttonSize)) {
      e->setRepeatPattern(!repeatPattern);
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(edit));
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_CIRCLE "##Edit",buttonSize)) {
      edit=!edit;
    }
    ImGui::PopStyleColor();

    bool metro=e->getMetronome();
    ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(metro));
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BELL_O "##Metronome",buttonSize)) {
      e->setMetronome(!metro);
    }
    ImGui::PopStyleColor();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
  ImGui::End();

  ImGui::SetNextWindowPos(portrait?ImVec2(0.0f,((1.0-mobileMenuPos*0.65)*scrH*dpiScale)):ImVec2(0.5*scrW*dpiScale*(mobileMenuPos-1.0),0.0f));
  ImGui::SetNextWindowSize(portrait?ImVec2(scrW*dpiScale,0.65*scrH*dpiScale):ImVec2(0.5*scrW*dpiScale,scrH*dpiScale));
  if (ImGui::Begin("Mobile Menu",NULL,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
    if (ImGui::BeginTable("SceneSel",5)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,1.0f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,1.0f);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,1.0f);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,1.0f);
      ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,1.0f);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImVec2 buttonSize=ImGui::GetContentRegionAvail();
      buttonSize.y=30.0f*dpiScale;

      if (ImGui::Button("Pattern",buttonSize)) {
        mobScene=GUI_SCENE_PATTERN;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Orders",buttonSize)) {
        mobScene=GUI_SCENE_ORDERS;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Ins",buttonSize)) {
        mobScene=GUI_SCENE_INSTRUMENT;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Wave",buttonSize)) {
        mobScene=GUI_SCENE_WAVETABLE;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Sample",buttonSize)) {
        mobScene=GUI_SCENE_SAMPLE;
      }
      ImGui::EndTable();
    }

    if (ImGui::Button("Create Ins")) {
      doAction(GUI_ACTION_INS_LIST_ADD);
    }

    ImGui::Text("Data list goes here...");

    if (ImGui::Button("New")) {
      mobileMenuOpen=false;
      //doAction(GUI_ACTION_NEW);
      if (modified) {
        showWarning("Unsaved changes! Save changes before creating a new song?",GUI_WARN_NEW);
      } else {
        displayNew=true;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Open")) {
      mobileMenuOpen=false;
      doAction(GUI_ACTION_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      mobileMenuOpen=false;
      doAction(GUI_ACTION_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save as...")) {
      mobileMenuOpen=false;
      doAction(GUI_ACTION_SAVE_AS);
    }

    ImGui::Button("1.1+ .dmf");
    ImGui::SameLine();
    ImGui::Button("Legacy .dmf");
    ImGui::SameLine();
    ImGui::Button("Export Audio");
    ImGui::SameLine();
    ImGui::Button("Export VGM");

    ImGui::Button("CmdStream");
    ImGui::SameLine();
    ImGui::Button("Panic");
    ImGui::SameLine();
    if (ImGui::Button("Settings")) {
      mobileMenuOpen=false;
    }
    ImGui::SameLine();
    if (ImGui::Button("About")) {
      mobileMenuOpen=false;
      mobileMenuPos=0.0f;
      aboutOpen=true;
    }

    ImGui::Separator();

    if (ImGui::Button("Osc")) {
      oscOpen=!oscOpen;
    }
    ImGui::SameLine();
    if (ImGui::Button("ChanOsc")) {
      chanOscOpen=!chanOscOpen;
    }
    ImGui::SameLine();
    if (ImGui::Button("RegView")) {
      regViewOpen=!regViewOpen;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stats")) {
      statsOpen=!statsOpen;
    }
  }
  ImGui::End();
}

void FurnaceGUI::drawEditControls() {
  if (nextWindow==GUI_WINDOW_EDIT_CONTROLS) {
    editControlsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!editControlsOpen) return;
  switch (settings.controlLayout) {
    case 0: // classic
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,globalWinFlags)) {
        if (ImGui::BeginTable("PlayEditAlign",2)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Octave");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
            if (curOctave>7) curOctave=7;
            if (curOctave<-5) curOctave=-5;
            e->autoNoteOffAll();

            if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
              nextWindow=GUI_WINDOW_PATTERN;
            }
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("Edit Step");
          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
            if (editStep>=e->curSubSong->patLen) editStep=e->curSubSong->patLen-1;
            if (editStep<0) editStep=0;

            if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
              nextWindow=GUI_WINDOW_PATTERN;
            }
          }

          ImGui::EndTable();
        }

        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(e->isPlaying()));
        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Edit",&edit);
        ImGui::SameLine();
        bool metro=e->getMetronome();
        if (ImGui::Checkbox("Metronome",&metro)) {
          e->setMetronome(metro);
        }

        ImGui::Text("Follow");
        ImGui::SameLine();
        unimportant(ImGui::Checkbox("Orders",&followOrders));
        ImGui::SameLine();
        unimportant(ImGui::Checkbox("Pattern",&followPattern));

        bool repeatPattern=e->getRepeatPattern();
        if (ImGui::Checkbox("Repeat pattern",&repeatPattern)) {
          e->setRepeatPattern(repeatPattern);
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(noteInputPoly));
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        ImGui::PopStyleColor();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 1: // compact
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(e->isPlaying()));
        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }

        ImGui::SameLine();
        bool repeatPattern=e->getRepeatPattern();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(repeatPattern));
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(edit));
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        bool metro=e->getMetronome();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(metro));
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::Text("Octave");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          e->autoNoteOffAll();

          if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
            nextWindow=GUI_WINDOW_PATTERN;
          }
        }

        ImGui::SameLine();
        ImGui::Text("Edit Step");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(96.0f*dpiScale);
        if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
          if (editStep>=e->curSubSong->patLen) editStep=e->curSubSong->patLen-1;
          if (editStep<0) editStep=0;

          if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
            nextWindow=GUI_WINDOW_PATTERN;
          }
        }

        ImGui::SameLine();
        ImGui::Text("Follow");
        ImGui::SameLine();
        unimportant(ImGui::Checkbox("Orders",&followOrders));
        ImGui::SameLine();
        unimportant(ImGui::Checkbox("Pattern",&followPattern));

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(noteInputPoly));
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        ImGui::PopStyleColor();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 2: // compact vertical
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
        ImVec2 buttonSize=ImVec2(ImGui::GetContentRegionAvail().x,0.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(e->isPlaying()));
        if (ImGui::Button(ICON_FA_PLAY "##Play",buttonSize)) {
          play();
        }
        ImGui::PopStyleColor();
        if (ImGui::Button(ICON_FA_STOP "##Stop",buttonSize)) {
          stop();
        }
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne",buttonSize)) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }

        bool repeatPattern=e->getRepeatPattern();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(repeatPattern));
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern",buttonSize)) {
          e->setRepeatPattern(!repeatPattern);
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(edit));
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit",buttonSize)) {
          edit=!edit;
        }
        ImGui::PopStyleColor();

        bool metro=e->getMetronome();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(metro));
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome",buttonSize)) {
          e->setMetronome(!metro);
        }
        ImGui::PopStyleColor();

        ImGui::Text("Oct.");
        float avail=ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##Octave",&curOctave,0,0)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          e->autoNoteOffAll();

          if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
            nextWindow=GUI_WINDOW_PATTERN;
          }
        }

        ImGui::Text("Step");
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##EditStep",&editStep,0,0)) {
          if (editStep>=e->curSubSong->patLen) editStep=e->curSubSong->patLen-1;
          if (editStep<0) editStep=0;

          if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
            nextWindow=GUI_WINDOW_PATTERN;
          }
        }

        ImGui::Text("Foll.");
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(followOrders));
        if (ImGui::Button("Ord##FollowOrders",buttonSize)) { handleUnimportant
          followOrders=!followOrders;
        }
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(followPattern));
        if (ImGui::Button("Pat##FollowPattern",buttonSize)) { handleUnimportant
          followPattern=!followPattern;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(noteInputPoly));
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        ImGui::PopStyleColor();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 3: // split
      if (ImGui::Begin("Play Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
        if (e->isPlaying()) {
          ImGui::PushStyleColor(ImGuiCol_Button,uiColors[GUI_COLOR_TOGGLE_ON]);
          if (ImGui::Button(ICON_FA_STOP "##Stop")) {
            stop();
          }
          ImGui::PopStyleColor();
        } else {
          if (ImGui::Button(ICON_FA_PLAY "##Play")) {
            play(oldRow);
          }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLAY_CIRCLE "##PlayAgain")) {
          e->setRepeatPattern(false);
          play();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STEP_FORWARD "##PlayRepeat")) {
          e->setRepeatPattern(true);
          play();
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(edit));
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        ImGui::PopStyleColor();

        bool metro=e->getMetronome();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(metro));
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        bool repeatPattern=e->getRepeatPattern();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(repeatPattern));
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,TOGGLE_COLOR(noteInputPoly));
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        ImGui::PopStyleColor();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();

      if (ImGui::Begin("Edit Controls",&editControlsOpen,globalWinFlags)) {
        ImGui::Columns(2);
        ImGui::Text("Octave");
        ImGui::SameLine();
        float cursor=ImGui::GetCursorPosX();
        float avail=ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##Octave",&curOctave,1,1)) {
          if (curOctave>7) curOctave=7;
          if (curOctave<-5) curOctave=-5;
          e->autoNoteOffAll();

          if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
            nextWindow=GUI_WINDOW_PATTERN;
          }
        }

        ImGui::Text("Step");
        ImGui::SameLine();
        ImGui::SetCursorPosX(cursor);
        ImGui::SetNextItemWidth(avail);
        if (ImGui::InputInt("##EditStep",&editStep,1,1)) {
          if (editStep>=e->curSubSong->patLen) editStep=e->curSubSong->patLen-1;
          if (editStep<0) editStep=0;

          if (settings.insFocusesPattern && !ImGui::IsItemActive() && patternOpen) {
            nextWindow=GUI_WINDOW_PATTERN;
          }
        }
        ImGui::NextColumn();

        unimportant(ImGui::Checkbox("Follow orders",&followOrders));
        unimportant(ImGui::Checkbox("Follow pattern",&followPattern));
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
  }
}
