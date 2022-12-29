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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "IconsFontAwesome4.h"
#include <fmt/printf.h>

// 0: all directions
// 1: half
// 2: half
// 3: quarter
const float mobileButtonAngles[4][8]={
  {0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875},
  {0.8333, 0.0, 0.1667, 0.8, 0.9, 0.0, 0.1, 0.2},
  {0.0833, 0.25, 0.4167, 0.45, 0.35, 0.25, 0.15, 0.05},
  {0.25, 0.125, 0.0, 0.25, 0.1875, 0.125, 0.0625, 0.0}
};

const float mobileButtonDistances[4][8]={
  {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
  {0.8, 0.75, 0.8, 1.5, 1.5, 1.5, 1.5, 1.5},
  {0.8, 0.75, 0.8, 1.5, 1.5, 1.5, 1.5, 1.5},
  {0.9, 1.0, 0.9, 1.78, 1.82, 1.95, 1.82, 1.78}
};

const char* mobileButtonLabels[32]={
  // page 1
  "cut",
  "copy",
  "paste",
  "delete",
  "select\nall",
  "piano",
  "undo",
  "redo",

  // page 2
  "paste\nmix",
  "paste\nmix bg",
  "paste\nins",
  "paste\nins bg",
  "paste\nflood",
  "paste\noverflow",
  "transpose\nnotes",
  "transpose\nvalues",

  // page 3
  "change\nins",
  "find/\nreplace",
  "collapse",
  "expand",
  "flip",
  "invert",
  "interpolate",
  "scale",

  // page 4
  "fade",
  "randomize",
  "opmask",
  "scroll\nmode",
  "input\nlatch",
  "set\nlatch",
  "clear\nlatch",
  "clear"
};

const int mobileButtonActions[32]={
  // page 1
  GUI_ACTION_PAT_CUT,
  GUI_ACTION_PAT_COPY,
  GUI_ACTION_PAT_PASTE,
  GUI_ACTION_PAT_DELETE,
  GUI_ACTION_PAT_SELECT_ALL,
  0,
  GUI_ACTION_UNDO,
  GUI_ACTION_REDO,

  // page 2
  GUI_ACTION_PAT_PASTE_MIX,
  GUI_ACTION_PAT_PASTE_MIX_BG,
  0,
  0,
  GUI_ACTION_PAT_PASTE_FLOOD,
  GUI_ACTION_PAT_PASTE_OVERFLOW,
  0,
  0,

  // page 3
  0,
  GUI_ACTION_WINDOW_FIND,
  GUI_ACTION_PAT_COLLAPSE_ROWS,
  GUI_ACTION_PAT_EXPAND_ROWS,
  GUI_ACTION_PAT_FLIP_SELECTION,
  GUI_ACTION_PAT_INVERT_VALUES,
  GUI_ACTION_PAT_INTERPOLATE,
  0,

  // page 4
  GUI_ACTION_PAT_FADE,
  0,
  0,
  GUI_ACTION_PAT_SCROLL_MODE,
  0,
  GUI_ACTION_PAT_LATCH,
  GUI_ACTION_PAT_CLEAR_LATCH,
  GUI_ACTION_CLEAR
};

const bool mobileButtonPersist[32]={
  // page 1
  false,
  false,
  false,
  false,
  true,
  true,
  true,
  true,

  // page 2
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,

  // page 3
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false,

  // page 4
  false,
  false,
  false,
  true,
  false,
  false,
  false,
  false,
};

void FurnaceGUI::drawMobileControls() {
  float timeScale=1.0f/(60.0f*ImGui::GetIO().DeltaTime);
  if (dragMobileMenu) {
    if (portrait) {
      mobileMenuPos=(dragMobileMenuOrigin.y-ImGui::GetMousePos().y)/(canvasH*0.65);
    } else {
      mobileMenuPos=(ImGui::GetMousePos().x-dragMobileMenuOrigin.x)/(canvasW*0.65);
    }
    if (mobileMenuPos<0.0f) mobileMenuPos=0.0f;
    if (mobileMenuPos>1.0f) mobileMenuPos=1.0f;
  } else {
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
  }

  if (dragMobileEditButton) {
    if (ImGui::GetIO().MouseDragMaxDistanceSqr[ImGuiMouseButton_Left]>ImGui::GetIO().ConfigInertialScrollToleranceSqr) {
      mobileEditButtonPos.x=((ImGui::GetMousePos().x/canvasW)-((portrait?0.16*canvasW:0.16*canvasH)/2)/canvasW);
      mobileEditButtonPos.y=((ImGui::GetMousePos().y/canvasH)-((portrait?0.16*canvasW:0.16*canvasH)/2)/canvasH);
    }
  }

  if (mobileEditButtonPos.x<0) mobileEditButtonPos.x=0;
  if (mobileEditButtonPos.x>1) mobileEditButtonPos.x=1;
  if (mobileEditButtonPos.y<0) mobileEditButtonPos.y=0;
  if (mobileEditButtonPos.y>1) mobileEditButtonPos.y=1;

  if (mobileEdit) {
    mobileEditAnim+=ImGui::GetIO().DeltaTime*2.4;
    if (mobileEditAnim>1.0f) {
      mobileEditAnim=1.0f;
    } else {
      WAKE_UP;
    }
  } else {
    mobileEditAnim-=ImGui::GetIO().DeltaTime*2.4;
    if (mobileEditAnim<0.0f) {
      mobileEditAnim=0.0f;
    } else {
      WAKE_UP;
    }
  }

  if (curWindowLast==GUI_WINDOW_PATTERN) {
    if (mobileEditAnim>0.0f) {
      ImGui::SetNextWindowPos(ImVec2(0.0f,0.0f));
      ImGui::SetNextWindowSize(ImVec2(canvasW,canvasH));
    } else {
      ImGui::SetNextWindowPos(ImVec2((mobileEditButtonPos.x+(portrait?0:(mobileMenuPos*0.65)))*canvasW,(mobileEditButtonPos.y-(portrait?(mobileMenuPos*0.65):0))*canvasH));
      ImGui::SetNextWindowSize(portrait?ImVec2(0.16*canvasW,0.16*canvasW):ImVec2(0.16*canvasH,0.16*canvasH));
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0.0f,0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,mobileEditButtonSize.x);
    if (ImGui::Begin("MobileEdit",NULL,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoBackground|ImGuiWindowFlags_NoDecoration)) {
      bool mobileEditWas=mobileEdit;
      if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && mobileEdit) {
        mobileEdit=false;
      }

      if (mobileEditAnim>0.0f) {
        int curButtonPos=0;
        float buttonDir, buttonDist;
        float buttonMirrorX=1.0f;
        float buttonMirrorY=1.0f;

        int buttonLayout=0;

        ImVec2 scaledButtonPos=ImVec2(
          mobileEditButtonPos.x+((mobileEditButtonSize.x*0.5)/(float)canvasW),
          mobileEditButtonPos.y+((mobileEditButtonSize.y*0.5)/(float)canvasH)
        );

        if (scaledButtonPos.x>0.25 &&
            scaledButtonPos.x<0.75 &&
            scaledButtonPos.y>0.2 &&
            scaledButtonPos.y<0.8) {
          buttonLayout=0;
        } else if (scaledButtonPos.x>0.4 && scaledButtonPos.x<0.6) {
          buttonLayout=2;
        } else if (scaledButtonPos.y>0.25 && scaledButtonPos.y<0.75) {
          buttonLayout=1;
        } else {
          buttonLayout=3;
        }

        switch (buttonLayout) {
          case 1:
            if (mobileEditButtonPos.x>0.5) buttonMirrorX=-1.0f;
            break;
          case 2:
            if (mobileEditButtonPos.y>0.5) buttonMirrorY=-1.0f;
            break;
          case 3:
            if (mobileEditButtonPos.x>0.5) buttonMirrorX=-1.0f;
            if (mobileEditButtonPos.y>0.5) buttonMirrorY=-1.0f;
            break;
        }

        for (int i=0; i<8; i++) {
          float anim=(mobileEditAnim*1.5)-(float)i*0.05;
          if (anim<0.0f) anim=0.0f;
          if (anim>1.0f) anim=1.0f;
          anim=5*anim-7*pow(anim,2.0f)+3*pow(anim,3.0f);

          buttonDir=mobileButtonAngles[buttonLayout][curButtonPos];
          buttonDist=mobileButtonDistances[buttonLayout][curButtonPos]*mobileEditButtonSize.x*1.6f;

          ImGui::SetCursorPos(ImVec2(
            (mobileEditButtonPos.x*canvasW)+cos(buttonDir*2.0*M_PI)*buttonDist*buttonMirrorX*anim,
            (mobileEditButtonPos.y*canvasH)+sin(buttonDir*2.0*M_PI)*buttonDist*buttonMirrorY*anim
          ));
          if (ImGui::Button(mobileButtonLabels[i+mobileEditPage*8],mobileEditButtonSize)) {
            if (mobileButtonActions[i+mobileEditPage*8]) {
              doAction(mobileButtonActions[i+mobileEditPage*8]);
            }
            if (mobileButtonPersist[i+mobileEditPage*8]) {
              if (mobileMenuPos<=0.0) mobileEdit=true;
            }
          }

          curButtonPos++;
        }

        ImGui::SetCursorPos(ImVec2(mobileEditButtonPos.x*canvasW,mobileEditButtonPos.y*canvasH));
      } else {
        float avail=portrait?ImGui::GetContentRegionAvail().y:ImGui::GetContentRegionAvail().x;
        mobileEditButtonSize=ImVec2(avail,avail);
      }

      if (ImGui::Button(ICON_FA_PENCIL "##Edit",mobileEditButtonSize)) {
        // click
        if (mobileEditWas) {
          if (++mobileEditPage>3) mobileEditPage=0;
        }
        if (ImGui::GetIO().MouseDragMaxDistanceSqr[ImGuiMouseButton_Left]<=ImGui::GetIO().ConfigInertialScrollToleranceSqr) {
          if (mobileMenuPos<=0.0) mobileEdit=true;
        }
      }
      if (ImGui::IsItemClicked() && !mobileEdit && mobileMenuPos<=0.0) {
        dragMobileEditButton=true;
      }
    }
    ImGui::End();
    ImGui::PopStyleVar(2);
  }
  
  ImGui::SetNextWindowPos(portrait?ImVec2(0.0f,((1.0-mobileMenuPos*0.65)*canvasH)-(0.16*canvasW)):ImVec2(0.5*canvasW*mobileMenuPos,0.0f));
  ImGui::SetNextWindowSize(portrait?ImVec2(canvasW,0.16*canvasW):ImVec2(0.16*canvasH,canvasH));
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
      if (!dragMobileMenu) {
        mobileMenuOpen=!mobileMenuOpen;
      }
    }
    if (ImGui::IsItemActive() && ImGui::GetIO().MouseDragMaxDistanceSqr[ImGuiMouseButton_Left]>ImGui::GetIO().ConfigInertialScrollToleranceSqr*2.0f) {
      if (!dragMobileMenu) {
        dragMobileMenu=true;
        dragMobileMenuOrigin=ImGui::GetMousePos();
        if (portrait) {
          dragMobileMenuOrigin.y+=mobileMenuPos*canvasH*0.65f;
        } else {
          dragMobileMenuOrigin.x-=mobileMenuPos*canvasW*0.65f;
        }
      }
    }

    if (!portrait) ImGui::Separator();

    pushToggleColors(e->isPlaying());
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_PLAY "##Play",buttonSize)) {
      play();
    }
    popToggleColors();
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
    pushToggleColors(repeatPattern);
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern",buttonSize)) {
      e->setRepeatPattern(!repeatPattern);
    }
    popToggleColors();

    pushToggleColors(edit);
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_CIRCLE "##Edit",buttonSize)) {
      edit=!edit;
    }
    popToggleColors();

    bool metro=e->getMetronome();
    pushToggleColors(metro);
    if (portrait) ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BELL_O "##Metronome",buttonSize)) {
      e->setMetronome(!metro);
    }
    popToggleColors();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
  ImGui::End();

  ImGui::SetNextWindowPos(portrait?ImVec2(0.0f,((1.0-mobileMenuPos*0.65)*canvasH)):ImVec2(0.5*canvasW*(mobileMenuPos-1.0),0.0f));
  ImGui::SetNextWindowSize(portrait?ImVec2(canvasW,0.65*canvasH):ImVec2(0.5*canvasW,canvasH));
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
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::Button("Song",buttonSize)) {
        mobScene=GUI_SCENE_SONG;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Channels",buttonSize)) {
        mobScene=GUI_SCENE_CHANNELS;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Chips",buttonSize)) {
        mobScene=GUI_SCENE_CHIPS;
      }
      ImGui::TableNextColumn();
      if (ImGui::Button("Other",buttonSize)) {
        mobScene=GUI_SCENE_OTHER;
      }
      ImGui::EndTable();
    }

    ImGui::Separator();

    if (settings.unifiedDataView) {
      drawInsList(true);
    } else {
      switch (mobScene) {
        case GUI_SCENE_PATTERN:
        case GUI_SCENE_ORDERS:
        case GUI_SCENE_INSTRUMENT:
          drawInsList(true);
          break;
        case GUI_SCENE_WAVETABLE:
          drawWaveList(true);
          break;
        case GUI_SCENE_SAMPLE:
          drawSampleList(true);
          break;
        case GUI_SCENE_SONG: {
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
          if (ImGui::Button("Export VGM")) {
            openFileDialog(GUI_FILE_EXPORT_VGM);
          }

          ImGui::Button("CmdStream");

          ImGui::Separator();

          drawSongInfo(true);
          break;
        }
        case GUI_SCENE_CHANNELS:
          ImGui::Text("Channels here...");
          break;
        case GUI_SCENE_CHIPS:
          ImGui::Text("Chips here...");
          break;
        case GUI_SCENE_OTHER: {
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
          if (ImGui::Button("Compat Flags")) {
            compatFlagsOpen=!compatFlagsOpen;
          }

          ImGui::Separator();

          ImGui::Button("Panic");
          ImGui::SameLine();
          if (ImGui::Button("Settings")) {
            mobileMenuOpen=false;
            settingsOpen=true;
          }
          ImGui::SameLine();
          if (ImGui::Button("Log")) {
            logOpen=!logOpen;
          }
          ImGui::SameLine();
          if (ImGui::Button("Debug")) {
            debugOpen=!debugOpen;
          }
          ImGui::SameLine();
          if (ImGui::Button("About")) {
            mobileMenuOpen=false;
            mobileMenuPos=0.0f;
            aboutOpen=true;
          }
          if (ImGui::Button("Switch to Desktop Mode")) {
            toggleMobileUI(!mobileUI);
          }
          break;
        }
      }
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

        pushToggleColors(e->isPlaying());
        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Play");
        }
        popToggleColors();
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Stop");
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
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Step one row");
        }

        ImGui::SameLine();
        pushToggleColors(noteInputPoly);
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Polyphony");
        }
        popToggleColors();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 1: // compact
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
        if (ImGui::Button(ICON_FA_STOP "##Stop")) {
          stop();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Stop");
        }
        ImGui::SameLine();
        pushToggleColors(e->isPlaying());
        if (ImGui::Button(ICON_FA_PLAY "##Play")) {
          play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Play");
        }
        popToggleColors();
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Step one row");
        }

        ImGui::SameLine();
        bool repeatPattern=e->getRepeatPattern();
        pushToggleColors(repeatPattern);
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Repeat pattern");
        }
        popToggleColors();

        ImGui::SameLine();
        pushToggleColors(edit);
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit");
        }
        popToggleColors();

        ImGui::SameLine();
        bool metro=e->getMetronome();
        pushToggleColors(metro);
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Metronome");
        }
        popToggleColors();

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
        pushToggleColors(noteInputPoly);
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Polyphony");
        }
        popToggleColors();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 2: // compact vertical
      if (ImGui::Begin("Play/Edit Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
        ImVec2 buttonSize=ImVec2(ImGui::GetContentRegionAvail().x,0.0f);
        pushToggleColors(e->isPlaying());
        if (ImGui::Button(ICON_FA_PLAY "##Play",buttonSize)) {
          play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Play");
        }
        popToggleColors();
        if (ImGui::Button(ICON_FA_STOP "##Stop",buttonSize)) {
          stop();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Stop");
        }
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne",buttonSize)) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Step one row");
        }

        bool repeatPattern=e->getRepeatPattern();
        pushToggleColors(repeatPattern);
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern",buttonSize)) {
          e->setRepeatPattern(!repeatPattern);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Repeat pattern");
        }
        popToggleColors();

        pushToggleColors(edit);
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit",buttonSize)) {
          edit=!edit;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit");
        }
        popToggleColors();

        bool metro=e->getMetronome();
        pushToggleColors(metro);
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome",buttonSize)) {
          e->setMetronome(!metro);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Metronome");
        }
        popToggleColors();

        ImGui::Text("Oct.");
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Octave");
        }
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
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Follow");
        }
        pushToggleColors(followOrders);
        if (ImGui::Button("Ord##FollowOrders",buttonSize)) { handleUnimportant
          followOrders=!followOrders;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Orders");
        }
        popToggleColors();
        pushToggleColors(followPattern);
        if (ImGui::Button("Pat##FollowPattern",buttonSize)) { handleUnimportant
          followPattern=!followPattern;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Pattern");
        }
        popToggleColors();

        pushToggleColors(noteInputPoly);
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Polyphony");
        }
        popToggleColors();
      }
      if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_EDIT_CONTROLS;
      ImGui::End();
      break;
    case 3: // split
      if (ImGui::Begin("Play Controls",&editControlsOpen,ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse|globalWinFlags)) {
        if (e->isPlaying()) {
          pushToggleColors(true);
          if (ImGui::Button(ICON_FA_STOP "##Stop")) {
            stop();
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Stop");
          }
          popToggleColors();
        } else {
          if (ImGui::Button(ICON_FA_PLAY "##Play")) {
            play(oldRow);
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Play");
          }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLAY_CIRCLE "##PlayAgain")) {
          e->setRepeatPattern(false);
          play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Play from the beginning of this pattern");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_STEP_FORWARD "##PlayRepeat")) {
          e->setRepeatPattern(true);
          play();
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Repeat from the beginning of this pattern");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_ARROW_DOWN "##StepOne")) {
          e->stepOne(cursor.y);
          pendingStepUpdate=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Step one row");
        }

        ImGui::SameLine();
        pushToggleColors(edit);
        if (ImGui::Button(ICON_FA_CIRCLE "##Edit")) {
          edit=!edit;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Edit");
        }
        popToggleColors();

        bool metro=e->getMetronome();
        ImGui::SameLine();
        pushToggleColors(metro);
        if (ImGui::Button(ICON_FA_BELL_O "##Metronome")) {
          e->setMetronome(!metro);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Metronome");
        }
        popToggleColors();

        ImGui::SameLine();
        bool repeatPattern=e->getRepeatPattern();
        pushToggleColors(repeatPattern);
        if (ImGui::Button(ICON_FA_REPEAT "##RepeatPattern")) {
          e->setRepeatPattern(!repeatPattern);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Repeat pattern");
        }
        popToggleColors();

        ImGui::SameLine();
        pushToggleColors(noteInputPoly);
        if (ImGui::Button(noteInputPoly?("Poly##PolyInput"):("Mono##PolyInput"))) {
          noteInputPoly=!noteInputPoly;
          e->setAutoNotePoly(noteInputPoly);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Polyphony");
        }
        popToggleColors();
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
