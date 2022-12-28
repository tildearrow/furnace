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
#include "fonts.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "../utfutils.h"
#include "util.h"
#include "guiConst.h"
#include "intConst.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include <fmt/printf.h>
#include <imgui.h>

#define DEFAULT_NOTE_KEYS "5:7;6:4;7:3;8:16;10:6;11:8;12:24;13:10;16:11;17:9;18:26;19:28;20:12;21:17;22:1;23:19;24:23;25:5;26:14;27:2;28:21;29:0;30:100;31:13;32:15;34:18;35:20;36:22;38:25;39:27;43:100;46:101;47:29;48:31;53:102;"

#if defined(_WIN32) || defined(__APPLE__) || defined(IS_MOBILE)
#define POWER_SAVE_DEFAULT 1
#else
// currently off on Linux/other due to Mesa catch-up behavior.
#define POWER_SAVE_DEFAULT 0
#endif

#if defined(__HAIKU__) || defined(IS_MOBILE) || (defined(_WIN32) && !defined(_WIN64))
// NFD doesn't support Haiku
// NFD doesn't support Windows XP either
#define SYS_FILE_DIALOG_DEFAULT 0
#else
#define SYS_FILE_DIALOG_DEFAULT 1
#endif

const char* mainFonts[]={
  "IBM Plex Sans",
  "Liberation Sans",
  "Exo",
  "Proggy Clean",
  "GNU Unifont",
  "<Use system font>",
  "<Custom...>"
};

const char* patFonts[]={
  "IBM Plex Mono",
  "Mononoki",
  "PT Mono",
  "Proggy Clean",
  "GNU Unifont",
  "<Use system font>",
  "<Custom...>"
};

const char* audioBackends[]={
  "JACK",
  "SDL"
};

const char* audioQualities[]={
  "High",
  "Low"
};

const char* arcadeCores[]={
  "ymfm",
  "Nuked-OPM"
};

const char* ym2612Cores[]={
  "Nuked-OPN2",
  "ymfm"
};

const char* snCores[]={
  "MAME",
  "Nuked-PSG Mod"
};

const char* nesCores[]={
  "puNES",
  "NSFplay"
};

const char* c64Cores[]={
  "reSID",
  "reSIDfp"
};

const char* pokeyCores[]={
  "Atari800 (mzpokeysnd)",
  "ASAP (C++ port)"
};

const char* opnCores[]={
  "ymfm only",
  "Nuked-OPN2 (FM) + ymfm (SSG/ADPCM)"
};

const char* pcspkrOutMethods[]={
  "evdev SND_TONE",
  "KIOCSOUND on /dev/tty1",
  "/dev/port",
  "KIOCSOUND on standard output",
  "outb()"
};

const char* valueInputStyles[]={
  "Disabled/custom",
  "Two octaves (0 is C-4, F is D#5)",
  "Raw (note number is value)",
  "Two octaves alternate (lower keys are 0-9, upper keys are A-F)",
  "Use dual control change (one for each nibble)",
  "Use 14-bit control change",
  "Use single control change (imprecise)"
};

const char* valueSInputStyles[]={
  "Disabled/custom",
  "Use dual control change (one for each nibble)",
  "Use 14-bit control change",
  "Use single control change (imprecise)"
};

const char* messageTypes[]={
  "--select--",
  "???",
  "???",
  "???",
  "???",
  "???",
  "???",
  "???",
  "Note Off",
  "Note On",
  "Aftertouch",
  "Control",
  "Program",
  "ChanPressure",
  "Pitch Bend",
  "SysEx"
};

const char* messageChannels[]={
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "Any"
};

const char* specificControls[18]={
  "Instrument",
  "Volume",
  "Effect 1 type",
  "Effect 1 value",
  "Effect 2 type",
  "Effect 2 value",
  "Effect 3 type",
  "Effect 3 value",
  "Effect 4 type",
  "Effect 4 value",
  "Effect 5 type",
  "Effect 5 value",
  "Effect 6 type",
  "Effect 6 value",
  "Effect 7 type",
  "Effect 7 value",
  "Effect 8 type",
  "Effect 8 value"
};

#define SAMPLE_RATE_SELECTABLE(x) \
  if (ImGui::Selectable(#x,settings.audioRate==x)) { \
    settings.audioRate=x; \
  }

#define BUFFER_SIZE_SELECTABLE(x) \
  if (ImGui::Selectable(#x,settings.audioBufSize==x)) { \
    settings.audioBufSize=x; \
  }

#define UI_COLOR_CONFIG(what,label) \
  if (ImGui::ColorEdit4(label "##CC_" #what,(float*)&uiColors[what])) { \
    applyUISettings(false); \
  }

#define KEYBIND_CONFIG_BEGIN(id) \
  if (ImGui::BeginTable(id,2)) {

#define KEYBIND_CONFIG_END \
    ImGui::EndTable(); \
  }

#define UI_KEYBIND_CONFIG(what) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::TextUnformatted(guiActions[what].friendlyName); \
  ImGui::TableNextColumn(); \
  if (ImGui::Button(fmt::sprintf("%s##KC_" #what,(bindSetPending && bindSetTarget==what)?"Press key...":getKeyName(actionKeys[what])).c_str())) { \
    promptKey(what); \
  } \
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) actionKeys[what]=0;

String stripName(String what) {
  String ret;
  for (char& i: what) {
    if ((i>='A' && i<='Z') || (i>='a' && i<='z') || (i>='0' && i<='9')) {
      ret+=i;
    } else {
      ret+='-';
    }
  }
  return ret;
}

void FurnaceGUI::promptKey(int which) {
  bindSetTarget=which;
  bindSetActive=true;
  bindSetPending=true;
  bindSetPrevValue=actionKeys[which];
  actionKeys[which]=0;
}

struct MappedInput {
  int scan;
  int val;
  MappedInput():
    scan(SDL_SCANCODE_UNKNOWN), val(0) {}
  MappedInput(int s, int v):
    scan(s), val(v) {}
};

void FurnaceGUI::drawSettings() {
  if (nextWindow==GUI_WINDOW_SETTINGS) {
    settingsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!settingsOpen) return;
  if (mobileUI) {
    ImVec2 setWindowPos=ImVec2(0,0);
    ImVec2 setWindowSize=ImVec2(canvasW,canvasH);
    ImGui::SetNextWindowPos(setWindowPos);
    ImGui::SetNextWindowSize(setWindowSize);
  }
  if (ImGui::Begin("Settings",&settingsOpen,ImGuiWindowFlags_NoDocking|globalWinFlags)) {
    if (!settingsOpen) {
      settingsOpen=true;
      showWarning("Do you want to save your settings?",GUI_WARN_CLOSE_SETTINGS);
    }
    if (ImGui::BeginTabBar("settingsTab")) {
      if (ImGui::BeginTabItem("General")) {
        ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
        settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
        if (ImGui::BeginChild("SettingsView",settingsViewSize)) {
          ImGui::Text("Workspace layout:");
          ImGui::SameLine();
          if (ImGui::Button("Import")) {
            openFileDialog(GUI_FILE_IMPORT_LAYOUT);
          }
          ImGui::SameLine();
          if (ImGui::Button("Export")) {
            openFileDialog(GUI_FILE_EXPORT_LAYOUT);
          }
          ImGui::SameLine();
          if (ImGui::Button("Reset")) {
            showWarning("Are you sure you want to reset the workspace layout?",GUI_WARN_RESET_LAYOUT);
          }

          ImGui::Separator();

          ImGui::Text("Initial system:");
          ImGui::SameLine();
          if (ImGui::Button("Current system")) {
            settings.initialSys.clear();
            for (int i=0; i<e->song.systemLen; i++) {
              settings.initialSys.set(fmt::sprintf("id%d",i),e->systemToFileFur(e->song.system[i]));
              settings.initialSys.set(fmt::sprintf("vol%d",i),(int)e->song.systemVol[i]);
              settings.initialSys.set(fmt::sprintf("pan%d",i),(int)e->song.systemPan[i]);
              settings.initialSys.set(fmt::sprintf("flags%d",i),e->song.systemFlags[i].toBase64());
            }
            settings.initialSysName=e->song.systemName;
          }
          ImGui::SameLine();
          if (ImGui::Button("Randomize")) {
            settings.initialSys.clear();
            int howMany=1+rand()%3;
            int totalAvailSys=0;
            for (totalAvailSys=0; availableSystems[totalAvailSys]; totalAvailSys++);
            if (totalAvailSys>0) {
              for (int i=0; i<howMany; i++) {
                settings.initialSys.set(fmt::sprintf("id%d",i),e->systemToFileFur((DivSystem)availableSystems[rand()%totalAvailSys]));
                settings.initialSys.set(fmt::sprintf("vol%d",i),64);
                settings.initialSys.set(fmt::sprintf("pan%d",i),0);
                settings.initialSys.set(fmt::sprintf("flags%d",i),"");
              }
            } else {
              settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_DUMMY));
              settings.initialSys.set("vol0",64);
              settings.initialSys.set("pan0",0);
              settings.initialSys.set("flags0","");
              howMany=1;
            }
            // randomize system name
            std::vector<String> wordPool[6];
            for (int i=0; i<howMany; i++) {
              int wpPos=0;
              DivSystem sysID=e->systemFromFileFur(settings.initialSys.getInt(fmt::sprintf("id%d",i),0));
              String sName=e->getSystemName(sysID);
              String nameWord;
              sName+=" ";
              for (char& i: sName) {
                if (i==' ') {
                  if (nameWord!="") {
                    wordPool[wpPos++].push_back(nameWord);
                    if (wpPos>=6) break;
                    nameWord="";
                  }
                } else {
                  nameWord+=i;
                }
              }
            }
            settings.initialSysName="";
            for (int i=0; i<6; i++) {
              if (wordPool[i].empty()) continue;
              settings.initialSysName+=wordPool[i][rand()%wordPool[i].size()];
              settings.initialSysName+=" ";
            }
          }
          ImGui::SameLine();
          if (ImGui::Button("Reset to defaults")) {
            settings.initialSys.clear();
            settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_YM2612));
            settings.initialSys.set("vol0",64);
            settings.initialSys.set("pan0",0);
            settings.initialSys.set("flags0","");
            settings.initialSys.set("id1",e->systemToFileFur(DIV_SYSTEM_SMS));
            settings.initialSys.set("vol1",32);
            settings.initialSys.set("pan1",0);
            settings.initialSys.set("flags1","");
            settings.initialSysName="Sega Genesis/Mega Drive";
          }

          ImGui::Text("Name");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::InputText("##InitSysName",&settings.initialSysName);

          int sysCount=0;
          int doRemove=-1;
          for (size_t i=0; settings.initialSys.getInt(fmt::sprintf("id%d",i),0); i++) {
            DivSystem sysID=e->systemFromFileFur(settings.initialSys.getInt(fmt::sprintf("id%d",i),0));
            signed char sysVol=settings.initialSys.getInt(fmt::sprintf("vol%d",i),0);
            signed char sysPan=settings.initialSys.getInt(fmt::sprintf("pan%d",i),0);

            sysCount=i+1;

            //bool doRemove=false;
            bool doInvert=sysVol&128;
            signed char vol=sysVol&127;
            ImGui::PushID(i);

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize("Invert").x-ImGui::GetFrameHeightWithSpacing()*2.0-ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::BeginCombo("##System",getSystemName(sysID))) {
              for (int j=0; availableSystems[j]; j++) {
                if (ImGui::Selectable(getSystemName((DivSystem)availableSystems[j]),sysID==availableSystems[j])) {
                  sysID=(DivSystem)availableSystems[j];
                  settings.initialSys.set(fmt::sprintf("id%d",i),(int)e->systemToFileFur(sysID));
                  settings.initialSys.set(fmt::sprintf("flags%d",i),"");
                }
              }
              ImGui::EndCombo();
            }

            ImGui::SameLine();
            if (ImGui::Checkbox("Invert",&doInvert)) {
              sysVol^=128;
              settings.initialSys.set(fmt::sprintf("vol%d",i),(int)sysVol);
            }
            ImGui::SameLine();
            //ImGui::BeginDisabled(settings.initialSys.size()<=4);
            if (ImGui::Button(ICON_FA_MINUS "##InitSysRemove")) {
              doRemove=i;
            }
            //ImGui::EndDisabled();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-(50.0f*dpiScale));
            if (CWSliderScalar("Volume",ImGuiDataType_S8,&vol,&_ZERO,&_ONE_HUNDRED_TWENTY_SEVEN)) {
              sysVol=(sysVol&128)|vol;
              settings.initialSys.set(fmt::sprintf("vol%d",i),(int)sysVol);
            } rightClickable
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x-(50.0f*dpiScale));
            if (CWSliderScalar("Panning",ImGuiDataType_S8,&sysPan,&_MINUS_ONE_HUNDRED_TWENTY_SEVEN,&_ONE_HUNDRED_TWENTY_SEVEN)) {
              settings.initialSys.set(fmt::sprintf("pan%d",i),(int)sysPan);
            } rightClickable

            // oh please MSVC don't cry
            if (ImGui::TreeNode("Configure")) {
              String sysFlagsS=settings.initialSys.getString(fmt::sprintf("flags%d",i),"");
              DivConfig sysFlags;
              sysFlags.loadFromBase64(sysFlagsS.c_str());
              if (drawSysConf(-1,sysID,sysFlags,false)) {
                settings.initialSys.set(fmt::sprintf("flags%d",i),sysFlags.toBase64());
              }
              ImGui::TreePop();
            }

            ImGui::PopID();
          }

          if (doRemove>=0 && sysCount>1) {
            for (int i=doRemove; i<sysCount-1; i++) {
              int sysID=settings.initialSys.getInt(fmt::sprintf("id%d",i+1),0);
              int sysVol=settings.initialSys.getInt(fmt::sprintf("vol%d",i+1),0);
              int sysPan=settings.initialSys.getInt(fmt::sprintf("pan%d",i+1),0);
              String sysFlags=settings.initialSys.getString(fmt::sprintf("flags%d",i+1),"");
              settings.initialSys.set(fmt::sprintf("id%d",i),sysID);
              settings.initialSys.set(fmt::sprintf("vol%d",i),sysVol);
              settings.initialSys.set(fmt::sprintf("pan%d",i),sysPan);
              settings.initialSys.set(fmt::sprintf("flags%d",i),sysFlags);
            }

            settings.initialSys.remove(fmt::sprintf("id%d",sysCount-1));
            settings.initialSys.remove(fmt::sprintf("vol%d",sysCount-1));
            settings.initialSys.remove(fmt::sprintf("pan%d",sysCount-1));
            settings.initialSys.remove(fmt::sprintf("flags%d",sysCount-1));
          }

          if (sysCount<32) if (ImGui::Button(ICON_FA_PLUS "##InitSysAdd")) {
            settings.initialSys.set(fmt::sprintf("id%d",sysCount),(int)e->systemToFileFur(DIV_SYSTEM_YM2612));
            settings.initialSys.set(fmt::sprintf("vol%d",sysCount),64);
            settings.initialSys.set(fmt::sprintf("pan%d",sysCount),0);
            settings.initialSys.set(fmt::sprintf("flags%d",sysCount),"");
          }

          ImGui::Separator();

          ImGui::Text("Toggle channel solo on:");
          if (ImGui::RadioButton("Right-click or double-click##soloA",settings.soloAction==0)) {
            settings.soloAction=0;
          }
          if (ImGui::RadioButton("Right-click##soloR",settings.soloAction==1)) {
            settings.soloAction=1;
          }
          if (ImGui::RadioButton("Double-click##soloD",settings.soloAction==2)) {
            settings.soloAction=2;
          }

          bool pushNibbleB=settings.pushNibble;
          if (ImGui::Checkbox("Push value when overwriting instead of clearing it",&pushNibbleB)) {
            settings.pushNibble=pushNibbleB;
          }

          bool pullDeleteBehaviorB=settings.pullDeleteBehavior;
          if (ImGui::Checkbox("Move cursor up on backspace-delete",&pullDeleteBehaviorB)) {
            settings.pullDeleteBehavior=pullDeleteBehaviorB;
          }

          bool stepOnDeleteB=settings.stepOnDelete;
          if (ImGui::Checkbox("Move cursor by edit step on delete",&stepOnDeleteB)) {
            settings.stepOnDelete=stepOnDeleteB;
          }

          bool absorbInsInputB=settings.absorbInsInput;
          if (ImGui::Checkbox("Change current instrument when changing instrument column (absorb)",&absorbInsInputB)) {
            settings.absorbInsInput=absorbInsInputB;
          }

          bool effectDeletionAltersValueB=settings.effectDeletionAltersValue;
          if (ImGui::Checkbox("Delete effect value when deleting effect",&effectDeletionAltersValueB)) {
            settings.effectDeletionAltersValue=effectDeletionAltersValueB;
          }

          bool scrollChangesOrderB=settings.scrollChangesOrder;
          if (ImGui::Checkbox("Change order when scrolling outside of pattern bounds",&scrollChangesOrderB)) {
            settings.scrollChangesOrder=scrollChangesOrderB;
          }

          bool stepOnInsertB=settings.stepOnInsert;
          if (ImGui::Checkbox("Move cursor by edit step on insert (push)",&stepOnInsertB)) {
            settings.stepOnInsert=stepOnInsertB;
          }

          bool cursorPastePosB=settings.cursorPastePos;
          if (ImGui::Checkbox("Move cursor to end of clipboard content when pasting",&cursorPastePosB)) {
            settings.cursorPastePos=cursorPastePosB;
          }

          bool cursorMoveNoScrollB=settings.cursorMoveNoScroll;
          if (ImGui::Checkbox("Don't scroll when moving cursor",&cursorMoveNoScrollB)) {
            settings.cursorMoveNoScroll=cursorMoveNoScrollB;
          }

          bool doubleClickColumnB=settings.doubleClickColumn;
          if (ImGui::Checkbox("Double click selects entire column",&doubleClickColumnB)) {
            settings.doubleClickColumn=doubleClickColumnB;
          }

          bool allowEditDockingB=settings.allowEditDocking;
          if (ImGui::Checkbox("Allow docking editors",&allowEditDockingB)) {
            settings.allowEditDocking=allowEditDockingB;
          }

          bool avoidRaisingPatternB=settings.avoidRaisingPattern;
          if (ImGui::Checkbox("Don't raise pattern editor on click",&avoidRaisingPatternB)) {
            settings.avoidRaisingPattern=avoidRaisingPatternB;
          }

          bool insFocusesPatternB=settings.insFocusesPattern;
          if (ImGui::Checkbox("Focus pattern editor when selecting instrument",&insFocusesPatternB)) {
            settings.insFocusesPattern=insFocusesPatternB;
          }

          bool restartOnFlagChangeB=settings.restartOnFlagChange;
          if (ImGui::Checkbox("Restart song when changing chip properties",&restartOnFlagChangeB)) {
            settings.restartOnFlagChange=restartOnFlagChangeB;
          }

          bool sysFileDialogB=settings.sysFileDialog;
          if (ImGui::Checkbox("Use system file picker",&sysFileDialogB)) {
            settings.sysFileDialog=sysFileDialogB;
          }

          bool moveWindowTitleB=settings.moveWindowTitle;
          if (ImGui::Checkbox("Only allow window movement when clicking on title bar",&moveWindowTitleB)) {
            settings.moveWindowTitle=moveWindowTitleB;
            applyUISettings(false);
          }

          bool eventDelayB=settings.eventDelay;
          if (ImGui::Checkbox("Enable event delay",&eventDelayB)) {
            settings.eventDelay=eventDelayB;
            applyUISettings(false);
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("may cause issues with high-polling-rate mice when previewing notes.");
          }

          bool powerSaveB=settings.powerSave;
          if (ImGui::Checkbox("Power-saving mode",&powerSaveB)) {
            settings.powerSave=powerSaveB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("saves power by lowering the frame rate to 2fps when idle.\nmay cause issues under Mesa drivers!");
          }

#ifndef IS_MOBILE
          bool noThreadedInputB=settings.noThreadedInput;
          if (ImGui::Checkbox("Disable threaded input (restart after changing!)",&noThreadedInputB)) {
            settings.noThreadedInput=noThreadedInputB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("threaded input processes key presses for note preview on a separate thread (on supported platforms), which reduces latency.\nhowever, crashes have been reported when threaded input is on. enable this option if that is the case.");
          }

          bool saveWindowPosB=settings.saveWindowPos;
          if (ImGui::Checkbox("Remember window position",&saveWindowPosB)) {
            settings.saveWindowPos=saveWindowPosB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("remembers the window's last position on startup.");
          }
#endif

          bool blankInsB=settings.blankIns;
          if (ImGui::Checkbox("New instruments are blank",&blankInsB)) {
            settings.blankIns=blankInsB;
          }

          bool saveUnusedPatternsB=settings.saveUnusedPatterns;
          if (ImGui::Checkbox("Save unused patterns",&saveUnusedPatternsB)) {
            settings.saveUnusedPatterns=saveUnusedPatternsB;
          }

          ImGui::Text("Audio export loop/fade out time:");
          if (ImGui::RadioButton("Set to these values on start-up:##fot0",settings.persistFadeOut==0)) {
            settings.persistFadeOut=0;
          }
          ImGui::BeginDisabled(settings.persistFadeOut);
          if (ImGui::InputInt("Loops",&settings.exportLoops,1,2)) {
            if (exportLoops<0) exportLoops=0;
            exportLoops=settings.exportLoops;
          }
          if (ImGui::InputDouble("Fade out (seconds)",&settings.exportFadeOut,1.0,2.0,"%.1f")) {
            if (exportFadeOut<0.0) exportFadeOut=0.0;
            exportFadeOut=settings.exportFadeOut;
          }
          ImGui::EndDisabled();
          if (ImGui::RadioButton("Remember last values##fot1",settings.persistFadeOut==1)) {
            settings.persistFadeOut=1;
          }

          ImGui::Text("Note preview behavior:");
          if (ImGui::RadioButton("Never##npb0",settings.notePreviewBehavior==0)) {
            settings.notePreviewBehavior=0;
          }
          if (ImGui::RadioButton("When cursor is in Note column##npb1",settings.notePreviewBehavior==1)) {
            settings.notePreviewBehavior=1;
          }
          if (ImGui::RadioButton("When cursor is in Note column or not in edit mode##npb2",settings.notePreviewBehavior==2)) {
            settings.notePreviewBehavior=2;
          }
          if (ImGui::RadioButton("Always##npb3",settings.notePreviewBehavior==3)) {
            settings.notePreviewBehavior=3;
          }

          ImGui::Text("Wrap pattern cursor horizontally:");
          if (ImGui::RadioButton("No##wrapH0",settings.wrapHorizontal==0)) {
            settings.wrapHorizontal=0;
          }
          if (ImGui::RadioButton("Yes##wrapH1",settings.wrapHorizontal==1)) {
            settings.wrapHorizontal=1;
          }
          if (ImGui::RadioButton("Yes, and move to next/prev row##wrapH2",settings.wrapHorizontal==2)) {
            settings.wrapHorizontal=2;
          }

          ImGui::Text("Wrap pattern cursor vertically:");
          if (ImGui::RadioButton("No##wrapV0",settings.wrapVertical==0)) {
            settings.wrapVertical=0;
          }
          if (ImGui::RadioButton("Yes##wrapV1",settings.wrapVertical==1)) {
            settings.wrapVertical=1;
          }
          if (ImGui::RadioButton("Yes, and move to next/prev pattern##wrapV2",settings.wrapVertical==2)) {
            settings.wrapVertical=2;
          }

          ImGui::Text("Cursor movement keys behavior:");
          if (ImGui::RadioButton("Move by one##cmk0",settings.scrollStep==0)) {
            settings.scrollStep=0;
          }
          if (ImGui::RadioButton("Move by Edit Step##cmk1",settings.scrollStep==1)) {
            settings.scrollStep=1;
          }

          ImGui::Text("Effect input cursor behavior:");
          if (ImGui::RadioButton("Move down##eicb0",settings.effectCursorDir==0)) {
            settings.effectCursorDir=0;
          }
          if (ImGui::RadioButton("Move to effect value (otherwise move down)##eicb1",settings.effectCursorDir==1)) {
            settings.effectCursorDir=1;
          }
          if (ImGui::RadioButton("Move to effect value/next effect and wrap around##eicb2",settings.effectCursorDir==2)) {
            settings.effectCursorDir=2;
          }

          ImGui::Text("Allow dragging selection:");
          if (ImGui::RadioButton("No##dms0",settings.dragMovesSelection==0)) {
            settings.dragMovesSelection=0;
          }
          if (ImGui::RadioButton("Yes##dms1",settings.dragMovesSelection==1)) {
            settings.dragMovesSelection=1;
          }
          if (ImGui::RadioButton("Yes (while holding Ctrl only)##dms2",settings.dragMovesSelection==2)) {
            settings.dragMovesSelection=2;
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Audio/MIDI")) {
        ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
        settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
        if (ImGui::BeginChild("SettingsView",settingsViewSize)) {
          ImGui::Text("Backend");
          ImGui::SameLine();
          ImGui::Combo("##Backend",&settings.audioEngine,audioBackends,2);

          ImGui::Text("Device");
          ImGui::SameLine();
          String audioDevName=settings.audioDevice.empty()?"<System default>":settings.audioDevice;
          if (ImGui::BeginCombo("##AudioDevice",audioDevName.c_str())) {
            if (ImGui::Selectable("<System default>",settings.audioDevice.empty())) {
              settings.audioDevice="";
            }
            for (String& i: e->getAudioDevices()) {
              if (ImGui::Selectable(i.c_str(),i==settings.audioDevice)) {
                settings.audioDevice=i;
              }
            }
            ImGui::EndCombo();
          }

          ImGui::Text("Sample rate");
          ImGui::SameLine();
          String sr=fmt::sprintf("%d",settings.audioRate);
          if (ImGui::BeginCombo("##SampleRate",sr.c_str())) {
            SAMPLE_RATE_SELECTABLE(8000);
            SAMPLE_RATE_SELECTABLE(16000);
            SAMPLE_RATE_SELECTABLE(22050);
            SAMPLE_RATE_SELECTABLE(32000);
            SAMPLE_RATE_SELECTABLE(44100);
            SAMPLE_RATE_SELECTABLE(48000);
            SAMPLE_RATE_SELECTABLE(88200);
            SAMPLE_RATE_SELECTABLE(96000);
            SAMPLE_RATE_SELECTABLE(192000);
            ImGui::EndCombo();
          }

          ImGui::Text("Buffer size");
          ImGui::SameLine();
          String bs=fmt::sprintf("%d (latency: ~%.1fms)",settings.audioBufSize,2000.0*(double)settings.audioBufSize/(double)MAX(1,settings.audioRate));
          if (ImGui::BeginCombo("##BufferSize",bs.c_str())) {
            BUFFER_SIZE_SELECTABLE(64);
            BUFFER_SIZE_SELECTABLE(128);
            BUFFER_SIZE_SELECTABLE(256);
            BUFFER_SIZE_SELECTABLE(512);
            BUFFER_SIZE_SELECTABLE(1024);
            BUFFER_SIZE_SELECTABLE(2048);
            ImGui::EndCombo();
          }

          ImGui::Text("Quality");
          ImGui::SameLine();
          ImGui::Combo("##Quality",&settings.audioQuality,audioQualities,2);

          ImGui::Text("Metronome volume");
          ImGui::SameLine();
          if (ImGui::SliderInt("##MetroVol",&settings.metroVol,0,200,"%d%%")) {
            if (settings.metroVol<0) settings.metroVol=0;
            if (settings.metroVol>200) settings.metroVol=200;
            e->setMetronomeVol(((float)settings.metroVol)/100.0f);
          }

          bool lowLatencyB=settings.lowLatency;
          if (ImGui::Checkbox("Low-latency mode (experimental!)",&lowLatencyB)) {
            settings.lowLatency=lowLatencyB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("reduces latency by running the engine faster than the tick rate.\nuseful for live playback/jam mode.\n\nwarning: experimental! may produce glitches.\nonly enable if your buffer size is small (10ms or less).");
          }

          bool forceMonoB=settings.forceMono;
          if (ImGui::Checkbox("Force mono audio",&forceMonoB)) {
            settings.forceMono=forceMonoB;
          }

          bool clampSamplesB=settings.clampSamples;
          if (ImGui::Checkbox("Software clipping",&clampSamplesB)) {
            settings.clampSamples=clampSamplesB;
          }

          TAAudioDesc& audioWant=e->getAudioDescWant();
          TAAudioDesc& audioGot=e->getAudioDescGot();

          ImGui::Text("want: %d samples @ %.0fHz",audioWant.bufsize,audioWant.rate);
          ImGui::Text("got: %d samples @ %.0fHz",audioGot.bufsize,audioGot.rate);

          ImGui::Separator();

          ImGui::Text("MIDI input");
          ImGui::SameLine();
          String midiInName=settings.midiInDevice.empty()?"<disabled>":settings.midiInDevice;
          bool hasToReloadMidi=false;
          if (ImGui::BeginCombo("##MidiInDevice",midiInName.c_str())) {
            if (ImGui::Selectable("<disabled>",settings.midiInDevice.empty())) {
              settings.midiInDevice="";
              hasToReloadMidi=true;
            }
            for (String& i: e->getMidiIns()) {
              if (ImGui::Selectable(i.c_str(),i==settings.midiInDevice)) {
                settings.midiInDevice=i;
                hasToReloadMidi=true;
              }
            }
            ImGui::EndCombo();
          }

          if (hasToReloadMidi) {
            midiMap.read(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");
            midiMap.compile();
          }

          ImGui::Text("MIDI output");
          ImGui::SameLine();
          String midiOutName=settings.midiOutDevice.empty()?"<disabled>":settings.midiOutDevice;
          if (ImGui::BeginCombo("##MidiOutDevice",midiOutName.c_str())) {
            if (ImGui::Selectable("<disabled>",settings.midiOutDevice.empty())) {
              settings.midiOutDevice="";
            }
            for (String& i: e->getMidiIns()) {
              if (ImGui::Selectable(i.c_str(),i==settings.midiOutDevice)) {
                settings.midiOutDevice=i;
              }
            }
            ImGui::EndCombo();
          }

          if (ImGui::TreeNode("MIDI input settings")) {
            ImGui::Checkbox("Note input",&midiMap.noteInput);
            ImGui::Checkbox("Velocity input",&midiMap.volInput);
            // TODO
            //ImGui::Checkbox("Use raw velocity value (don't map from linear to log)",&midiMap.rawVolume);
            //ImGui::Checkbox("Polyphonic/chord input",&midiMap.polyInput);
            ImGui::Checkbox("Map MIDI channels to direct channels",&midiMap.directChannel);
            ImGui::Checkbox("Map Yamaha FM voice data to instruments",&midiMap.yamahaFMResponse);
            ImGui::Checkbox("Program change is instrument selection",&midiMap.programChange);
            //ImGui::Checkbox("Listen to MIDI clock",&midiMap.midiClock);
            //ImGui::Checkbox("Listen to MIDI time code",&midiMap.midiTimeCode);
            ImGui::Combo("Value input style",&midiMap.valueInputStyle,valueInputStyles,7);
            if (midiMap.valueInputStyle>3) {
              if (midiMap.valueInputStyle==6) {
                if (ImGui::InputInt("Control##valueCCS",&midiMap.valueInputControlSingle,1,16)) {
                  if (midiMap.valueInputControlSingle<0) midiMap.valueInputControlSingle=0;
                  if (midiMap.valueInputControlSingle>127) midiMap.valueInputControlSingle=127;
                }
              } else {
                if (ImGui::InputInt((midiMap.valueInputStyle==4)?"CC of upper nibble##valueCC1":"MSB CC##valueCC1",&midiMap.valueInputControlMSB,1,16)) {
                  if (midiMap.valueInputControlMSB<0) midiMap.valueInputControlMSB=0;
                  if (midiMap.valueInputControlMSB>127) midiMap.valueInputControlMSB=127;
                }
                if (ImGui::InputInt((midiMap.valueInputStyle==4)?"CC of lower nibble##valueCC2":"LSB CC##valueCC2",&midiMap.valueInputControlLSB,1,16)) {
                  if (midiMap.valueInputControlLSB<0) midiMap.valueInputControlLSB=0;
                  if (midiMap.valueInputControlLSB>127) midiMap.valueInputControlLSB=127;
                }
              }
            }
            if (ImGui::TreeNode("Per-column control change")) {
              for (int i=0; i<18; i++) {
                ImGui::PushID(i);
                ImGui::Combo(specificControls[i],&midiMap.valueInputSpecificStyle[i],valueSInputStyles,4);
                if (midiMap.valueInputSpecificStyle[i]>0) {
                  ImGui::Indent();
                  if (midiMap.valueInputSpecificStyle[i]==3) {
                    if (ImGui::InputInt("Control##valueCCS",&midiMap.valueInputSpecificSingle[i],1,16)) {
                      if (midiMap.valueInputSpecificSingle[i]<0) midiMap.valueInputSpecificSingle[i]=0;
                      if (midiMap.valueInputSpecificSingle[i]>127) midiMap.valueInputSpecificSingle[i]=127;
                    }
                  } else {
                    if (ImGui::InputInt((midiMap.valueInputSpecificStyle[i]==4)?"CC of upper nibble##valueCC1":"MSB CC##valueCC1",&midiMap.valueInputSpecificMSB[i],1,16)) {
                      if (midiMap.valueInputSpecificMSB[i]<0) midiMap.valueInputSpecificMSB[i]=0;
                      if (midiMap.valueInputSpecificMSB[i]>127) midiMap.valueInputSpecificMSB[i]=127;
                    }
                    if (ImGui::InputInt((midiMap.valueInputSpecificStyle[i]==4)?"CC of lower nibble##valueCC2":"LSB CC##valueCC2",&midiMap.valueInputSpecificLSB[i],1,16)) {
                      if (midiMap.valueInputSpecificLSB[i]<0) midiMap.valueInputSpecificLSB[i]=0;
                      if (midiMap.valueInputSpecificLSB[i]>127) midiMap.valueInputSpecificLSB[i]=127;
                    }
                  }
                  ImGui::Unindent();
                }
                ImGui::PopID();
              }
              ImGui::TreePop();
            }
            if (ImGui::SliderFloat("Volume curve",&midiMap.volExp,0.01,8.0,"%.2f")) {
              if (midiMap.volExp<0.01) midiMap.volExp=0.01;
              if (midiMap.volExp>8.0) midiMap.volExp=8.0;
            } rightClickable
            float curve[128];
            for (int i=0; i<128; i++) {
              curve[i]=(int)(pow((double)i/127.0,midiMap.volExp)*127.0);
            }
            ImGui::PlotLines("##VolCurveDisplay",curve,128,0,"Volume curve",0.0,127.0,ImVec2(200.0f*dpiScale,200.0f*dpiScale));

            ImGui::Text("Actions:");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PLUS "##AddAction")) {
              midiMap.binds.push_back(MIDIBind());
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_EXTERNAL_LINK "##AddLearnAction")) {
              midiMap.binds.push_back(MIDIBind());
              learning=midiMap.binds.size()-1;
            }
            if (learning!=-1) {
              ImGui::SameLine();
              ImGui::Text("(learning! press a button or move a slider/knob/something on your device.)");
            }

            if (ImGui::BeginTable("MIDIActions",7)) {
              ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.2);
              ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.1);
              ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.3);
              ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch,0.2);
              ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.5);
              ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed);
              ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed);

              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("Type");
              ImGui::TableNextColumn();
              ImGui::Text("Channel");
              ImGui::TableNextColumn();
              ImGui::Text("Note/Control");
              ImGui::TableNextColumn();
              ImGui::Text("Velocity/Value");
              ImGui::TableNextColumn();
              ImGui::Text("Action");
              ImGui::TableNextColumn();
              ImGui::Text("Learn");
              ImGui::TableNextColumn();
              ImGui::Text("Remove");

              for (size_t i=0; i<midiMap.binds.size(); i++) {
                MIDIBind& bind=midiMap.binds[i];
                char bindID[1024];
                ImGui::PushID(i);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##BType",messageTypes[bind.type])) {
                  for (int j=8; j<15; j++) {
                    if (ImGui::Selectable(messageTypes[j],bind.type==j)) {
                      bind.type=j;
                    }
                  }
                  ImGui::EndCombo();
                }

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##BChannel",messageChannels[bind.channel])) {
                  if (ImGui::Selectable(messageChannels[16],bind.channel==16)) {
                    bind.channel=16;
                  }
                  for (int j=0; j<16; j++) {
                    if (ImGui::Selectable(messageChannels[j],bind.channel==j)) {
                      bind.channel=j;
                    }
                  }
                  ImGui::EndCombo();
                }

                ImGui::TableNextColumn();
                if (bind.data1==128) {
                  snprintf(bindID,1024,"Any");
                } else {
                  const char* nName="???";
                  if ((bind.data1+60)>0 && (bind.data1+60)<180) {
                    nName=noteNames[bind.data1+60];
                  }
                  snprintf(bindID,1024,"%d (0x%.2X, %s)",bind.data1,bind.data1,nName);
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##BValue1",bindID)) {
                  if (ImGui::Selectable("Any",bind.data1==128)) {
                    bind.data1=128;
                  }
                  for (int j=0; j<128; j++) {
                    const char* nName="???";
                    if ((j+60)>0 && (j+60)<180) {
                      nName=noteNames[j+60];
                    }
                    snprintf(bindID,1024,"%d (0x%.2X, %s)##BV1_%d",j,j,nName,j);
                    if (ImGui::Selectable(bindID,bind.data1==j)) {
                      bind.data1=j;
                    }
                  }
                  ImGui::EndCombo();
                }

                ImGui::TableNextColumn();
                if (bind.data2==128) {
                  snprintf(bindID,1024,"Any");
                } else {
                  snprintf(bindID,1024,"%d (0x%.2X)",bind.data2,bind.data2);
                }
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##BValue2",bindID)) {
                  if (ImGui::Selectable("Any",bind.data2==128)) {
                    bind.data2=128;
                  }
                  for (int j=0; j<128; j++) {
                    snprintf(bindID,1024,"%d (0x%.2X)##BV2_%d",j,j,j);
                    if (ImGui::Selectable(bindID,bind.data2==j)) {
                      bind.data2=j;
                    }
                  }
                  ImGui::EndCombo();
                }

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                if (ImGui::BeginCombo("##BAction",(bind.action==0)?"--none--":guiActions[bind.action].friendlyName)) {
                  if (ImGui::Selectable("--none--",bind.action==0)) {
                    bind.action=0;
                  }
                  for (int j=0; j<GUI_ACTION_MAX; j++) {
                    if (strcmp(guiActions[j].friendlyName,"")==0) continue;
                    if (strstr(guiActions[j].friendlyName,"---")==guiActions[j].friendlyName) {
                      ImGui::TextUnformatted(guiActions[j].friendlyName);
                    } else {
                      snprintf(bindID,1024,"%s##BA_%d",guiActions[j].friendlyName,j);
                      if (ImGui::Selectable(bindID,bind.action==j)) {
                        bind.action=j;
                      }
                    }
                  }
                  ImGui::EndCombo();
                }

                ImGui::TableNextColumn();
                if (ImGui::Button((learning==(int)i)?("waiting...##BLearn"):(ICON_FA_SQUARE_O "##BLearn"))) {
                  if (learning==(int)i) {
                    learning=-1;
                  } else {
                    learning=i;
                  }
                }

                ImGui::TableNextColumn();
                if (ImGui::Button(ICON_FA_TIMES "##BRemove")) {
                  midiMap.binds.erase(midiMap.binds.begin()+i);
                  if (learning==(int)i) learning=-1;
                  i--;
                }

                ImGui::PopID();
              }
              ImGui::EndTable();
            }

            ImGui::TreePop();
          }
          if (ImGui::TreeNode("MIDI output settings")) {
            ImGui::Text("Output mode:");
            if (ImGui::RadioButton("Off (use for TX81Z)",settings.midiOutMode==0)) {
              settings.midiOutMode=0;
            }
            if (ImGui::RadioButton("Melodic",settings.midiOutMode==1)) {
              settings.midiOutMode=1;
            }
            /*
            if (ImGui::RadioButton("Light Show (use for Launchpad)",settings.midiOutMode==2)) {
              settings.midiOutMode=2;
            }*/

            bool midiOutClockB=settings.midiOutClock;
            if (ImGui::Checkbox("Send MIDI clock",&midiOutClockB)) {
              settings.midiOutClock=midiOutClockB;
            }

            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Emulation")) {
        ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
        settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
        if (ImGui::BeginChild("SettingsView",settingsViewSize)) {
          ImGui::Text("Arcade/YM2151 core");
          ImGui::SameLine();
          ImGui::Combo("##ArcadeCore",&settings.arcadeCore,arcadeCores,2);

          ImGui::Text("Genesis/YM2612 core");
          ImGui::SameLine();
          ImGui::Combo("##YM2612Core",&settings.ym2612Core,ym2612Cores,2);

          ImGui::Text("SN76489 core");
          ImGui::SameLine();
          ImGui::Combo("##SNCore",&settings.snCore,snCores,2);

          ImGui::Text("NES core");
          ImGui::SameLine();
          ImGui::Combo("##NESCore",&settings.nesCore,nesCores,2);

          ImGui::Text("FDS core");
          ImGui::SameLine();
          ImGui::Combo("##FDSCore",&settings.fdsCore,nesCores,2);

          ImGui::Text("SID core");
          ImGui::SameLine();
          ImGui::Combo("##C64Core",&settings.c64Core,c64Cores,2);

          ImGui::Text("POKEY core");
          ImGui::SameLine();
          ImGui::Combo("##POKEYCore",&settings.pokeyCore,pokeyCores,2);

          ImGui::Text("OPN/OPNA/OPNB cores");
          ImGui::SameLine();
          ImGui::Combo("##OPNCore",&settings.opnCore,opnCores,2);

          ImGui::Separator();

          ImGui::Text("PC Speaker strategy");
          ImGui::SameLine();
          ImGui::Combo("##PCSOutMethod",&settings.pcSpeakerOutMethod,pcspkrOutMethods,5);

          ImGui::Separator();
          ImGui::Text("Sample ROMs:");

          ImGui::Text("OPL4 YRW801 path");
          ImGui::SameLine();
          ImGui::InputText("##YRW801Path",&settings.yrw801Path);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##YRW801Load")) {
            openFileDialog(GUI_FILE_YRW801_ROM_OPEN);
          }

          ImGui::Text("MultiPCM TG100 path");
          ImGui::SameLine();
          ImGui::InputText("##TG100Path",&settings.tg100Path);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##TG100Load")) {
            openFileDialog(GUI_FILE_TG100_ROM_OPEN);
          }

          ImGui::Text("MultiPCM MU5 path");
          ImGui::SameLine();
          ImGui::InputText("##MU5Path",&settings.mu5Path);
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_FOLDER "##MU5Load")) {
            openFileDialog(GUI_FILE_MU5_ROM_OPEN);
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Appearance")) {
        ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
        settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
        if (ImGui::BeginChild("SettingsView",settingsViewSize)) {
          bool dpiScaleAuto=(settings.dpiScale<0.5f);
          if (ImGui::Checkbox("Automatic UI scaling factor",&dpiScaleAuto)) {
            if (dpiScaleAuto) {
              settings.dpiScale=0.0f;
            } else {
              settings.dpiScale=1.0f;
            }
          }
          if (!dpiScaleAuto) {
            if (ImGui::SliderFloat("UI scaling factor",&settings.dpiScale,1.0f,3.0f,"%.2fx")) {
              if (settings.dpiScale<0.5f) settings.dpiScale=0.5f;
              if (settings.dpiScale>3.0f) settings.dpiScale=3.0f;
            } rightClickable
          }
          ImGui::Text("Main font");
          ImGui::SameLine();
          ImGui::Combo("##MainFont",&settings.mainFont,mainFonts,7);
          if (settings.mainFont==6) {
            ImGui::InputText("##MainFontPath",&settings.mainFontPath);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER "##MainFontLoad")) {
              openFileDialog(GUI_FILE_LOAD_MAIN_FONT);
            }
          }
          if (ImGui::InputInt("Size##MainFontSize",&settings.mainFontSize)) {
            if (settings.mainFontSize<3) settings.mainFontSize=3;
            if (settings.mainFontSize>96) settings.mainFontSize=96;
          }
          ImGui::Text("Pattern font");
          ImGui::SameLine();
          ImGui::Combo("##PatFont",&settings.patFont,patFonts,7);
          if (settings.patFont==6) {
            ImGui::InputText("##PatFontPath",&settings.patFontPath);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_FOLDER "##PatFontLoad")) {
              openFileDialog(GUI_FILE_LOAD_PAT_FONT);
            }
          }
          if (ImGui::InputInt("Size##PatFontSize",&settings.patFontSize)) {
            if (settings.patFontSize<3) settings.patFontSize=3;
            if (settings.patFontSize>96) settings.patFontSize=96;
          }
          if (ImGui::InputInt("Icon size",&settings.iconSize)) {
            if (settings.iconSize<3) settings.iconSize=3;
            if (settings.iconSize>48) settings.iconSize=48;
          }

          bool loadJapaneseB=settings.loadJapanese;
          if (ImGui::Checkbox("Display Japanese characters",&loadJapaneseB)) {
            settings.loadJapanese=loadJapaneseB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "このオプションは、十分なグラフィックメモリがある場合にのみ切り替えてください。\n"
              "これは、Dear ImGuiにダイナミックフォントアトラスが実装されるまでの一時的な解決策です。"
            );
          }

          bool loadChineseB=settings.loadChinese;
          if (ImGui::Checkbox("Display Chinese (Simplified) characters",&loadChineseB)) {
            settings.loadChinese=loadChineseB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "请在确保你有足够的显存后再启动此设定\n"
              "这是一个在ImGui实现动态字体加载之前的临时解决方案"
            );
          }

          bool loadChineseTraditionalB=settings.loadChineseTraditional;
          if (ImGui::Checkbox("Display Chinese (Traditional) characters",&loadChineseTraditionalB)) {
            settings.loadChineseTraditional=loadChineseTraditionalB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "請在確保你有足夠的顯存后再啟動此設定\n"
              "這是一個在ImGui實現動態字體加載之前的臨時解決方案"
            );
          }

          bool loadKoreanB=settings.loadKorean;
          if (ImGui::Checkbox("Display Korean characters",&loadKoreanB)) {
            settings.loadKorean=loadKoreanB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(
              "Only toggle this option if you have enough graphics memory.\n"
              "This is a temporary solution until dynamic font atlas is implemented in Dear ImGui.\n\n"
              "그래픽 메모리가 충분한 경우에만 이 옵션을 선택하십시오.\n"
              "이 옵션은 Dear ImGui에 동적 글꼴 아틀라스가 구현될 때까지 임시 솔루션입니다."
            );
          }

          ImGui::Separator();

          if (ImGui::InputInt("Number of recent files",&settings.maxRecentFile)) {
            if (settings.maxRecentFile<0) settings.maxRecentFile=0;
            if (settings.maxRecentFile>30) settings.maxRecentFile=30;
          }

          ImGui::Separator();

          ImGui::Text("Pattern view labels:");
          ImGui::InputTextWithHint("Note off (3-char)","OFF",&settings.noteOffLabel);
          ImGui::InputTextWithHint("Note release (3-char)","===",&settings.noteRelLabel);
          ImGui::InputTextWithHint("Macro release (3-char)","REL",&settings.macroRelLabel);
          ImGui::InputTextWithHint("Empty field (3-char)","...",&settings.emptyLabel);
          ImGui::InputTextWithHint("Empty field (2-char)","..",&settings.emptyLabel2);

          ImGui::Separator();

          ImGui::Text("Orders row number format:");
          if (ImGui::RadioButton("Decimal##orbD",settings.orderRowsBase==0)) {
            settings.orderRowsBase=0;
          }
          if (ImGui::RadioButton("Hexadecimal##orbH",settings.orderRowsBase==1)) {
            settings.orderRowsBase=1;
          }

          ImGui::Text("Pattern row number format:");
          if (ImGui::RadioButton("Decimal##prbD",settings.patRowsBase==0)) {
            settings.patRowsBase=0;
          }
          if (ImGui::RadioButton("Hexadecimal##prbH",settings.patRowsBase==1)) {
            settings.patRowsBase=1;
          }

          ImGui::Text("FM parameter names:");
          if (ImGui::RadioButton("Friendly##fmn0",settings.fmNames==0)) {
            settings.fmNames=0;
          }
          if (ImGui::RadioButton("Technical##fmn1",settings.fmNames==1)) {
            settings.fmNames=1;
          }
          if (ImGui::RadioButton("Technical (alternate)##fmn2",settings.fmNames==2)) {
            settings.fmNames=2;
          }

          ImGui::Separator();

          ImGui::Text("Title bar:");
          if (ImGui::RadioButton("Furnace##tbar0",settings.titleBarInfo==0)) {
            settings.titleBarInfo=0;
            updateWindowTitle();
          }
          if (ImGui::RadioButton("Song Name - Furnace##tbar1",settings.titleBarInfo==1)) {
            settings.titleBarInfo=1;
            updateWindowTitle();
          }
          if (ImGui::RadioButton("file_name.fur - Furnace##tbar2",settings.titleBarInfo==2)) {
            settings.titleBarInfo=2;
            updateWindowTitle();
          }
          if (ImGui::RadioButton("/path/to/file.fur - Furnace##tbar3",settings.titleBarInfo==3)) {
            settings.titleBarInfo=3;
            updateWindowTitle();
          }

          bool titleBarSysB=settings.titleBarSys;
          if (ImGui::Checkbox("Display system name on title bar",&titleBarSysB)) {
            settings.titleBarSys=titleBarSysB;
            updateWindowTitle();
          }

          bool noMultiSystemB=settings.noMultiSystem;
          if (ImGui::Checkbox("Display chip names instead of \"multi-system\" in title bar",&noMultiSystemB)) {
            settings.noMultiSystem=noMultiSystemB;
            updateWindowTitle();
          }

          ImGui::Text("Status bar:");
          if (ImGui::RadioButton("Cursor details##sbar0",settings.statusDisplay==0)) {
            settings.statusDisplay=0;
          }
          if (ImGui::RadioButton("File path##sbar1",settings.statusDisplay==1)) {
            settings.statusDisplay=1;
          }
          if (ImGui::RadioButton("Cursor details or file path##sbar2",settings.statusDisplay==2)) {
            settings.statusDisplay=2;
          }
          if (ImGui::RadioButton("Nothing##sbar3",settings.statusDisplay==3)) {
            settings.statusDisplay=3;
          }

          ImGui::Text("Play/edit controls layout:");
          if (ImGui::RadioButton("Classic##ecl0",settings.controlLayout==0)) {
            settings.controlLayout=0;
          }
          if (ImGui::RadioButton("Compact##ecl1",settings.controlLayout==1)) {
            settings.controlLayout=1;
          }
          if (ImGui::RadioButton("Compact (vertical)##ecl2",settings.controlLayout==2)) {
            settings.controlLayout=2;
          }
          if (ImGui::RadioButton("Split##ecl3",settings.controlLayout==3)) {
            settings.controlLayout=3;
          }

          ImGui::Text("FM parameter editor layout:");
          if (ImGui::RadioButton("Modern##fml0",settings.fmLayout==0)) {
            settings.fmLayout=0;
          }
          if (ImGui::RadioButton("Compact (2x2, classic)##fml1",settings.fmLayout==1)) {
            settings.fmLayout=1;
          }
          if (ImGui::RadioButton("Compact (1x4)##fml2",settings.fmLayout==2)) {
            settings.fmLayout=2;
          }
          if (ImGui::RadioButton("Compact (4x1)##fml3",settings.fmLayout==3)) {
            settings.fmLayout=3;
          }
          if (ImGui::RadioButton("Alternate (2x2)##fml4",settings.fmLayout==4)) {
            settings.fmLayout=4;
          }
          if (ImGui::RadioButton("Alternate (1x4)##fml5",settings.fmLayout==5)) {
            settings.fmLayout=5;
          }
          if (ImGui::RadioButton("Alternate (4x1)##fml5",settings.fmLayout==6)) {
            settings.fmLayout=6;
          }

          ImGui::Text("Position of Sustain in FM editor:");
          if (ImGui::RadioButton("Between Decay and Sustain Rate##susp0",settings.susPosition==0)) {
            settings.susPosition=0;
          }
          if (ImGui::RadioButton("After Release Rate##susp1",settings.susPosition==1)) {
            settings.susPosition=1;
          }

          ImGui::Text("Macro editor layout:");
          if (ImGui::RadioButton("Unified##mel0",settings.macroLayout==0)) {
            settings.macroLayout=0;
          }
          if (ImGui::RadioButton("Mobile##mel1",settings.macroLayout==1)) {
            settings.macroLayout=1;
          }
          if (ImGui::RadioButton("Grid##mel2",settings.macroLayout==2)) {
            settings.macroLayout=2;
          }
          if (ImGui::RadioButton("Single (with list)##mel3",settings.macroLayout==3)) {
            settings.macroLayout=3;
          }
          if (ImGui::RadioButton("Single (combo box)##mel4",settings.macroLayout==4)) {
            settings.macroLayout=4;
          }

          ImGui::Separator();

          ImGui::Text("Namco 163 chip name");
          ImGui::SameLine();
          ImGui::InputTextWithHint("##C163Name",DIV_C163_DEFAULT_NAME,&settings.c163Name);

          ImGui::Separator();

          ImGui::Text("Channel colors:");
          if (ImGui::RadioButton("Single##CHC0",settings.channelColors==0)) {
            settings.channelColors=0;
          }
          if (ImGui::RadioButton("Channel type##CHC1",settings.channelColors==1)) {
            settings.channelColors=1;
          }
          if (ImGui::RadioButton("Instrument type##CHC2",settings.channelColors==2)) {
            settings.channelColors=2;
          }

          ImGui::Text("Channel name colors:");
          if (ImGui::RadioButton("Single##CTC0",settings.channelTextColors==0)) {
            settings.channelTextColors=0;
          }
          if (ImGui::RadioButton("Channel type##CTC1",settings.channelTextColors==1)) {
            settings.channelTextColors=1;
          }
          if (ImGui::RadioButton("Instrument type##CTC2",settings.channelTextColors==2)) {
            settings.channelTextColors=2;
          }

          ImGui::Text("Channel style:");
          if (ImGui::RadioButton("Classic##CHS0",settings.channelStyle==0)) {
            settings.channelStyle=0;
          }
          if (ImGui::RadioButton("Line##CHS1",settings.channelStyle==1)) {
            settings.channelStyle=1;
          }
          if (ImGui::RadioButton("Round##CHS2",settings.channelStyle==2)) {
            settings.channelStyle=2;
          }
          if (ImGui::RadioButton("Split button##CHS3",settings.channelStyle==3)) {
            settings.channelStyle=3;
          }
          if (ImGui::RadioButton("Square border##CH42",settings.channelStyle==4)) {
            settings.channelStyle=4;
          }
          if (ImGui::RadioButton("Round border##CHS5",settings.channelStyle==5)) {
            settings.channelStyle=5;
          }

          ImGui::Text("Channel volume bar:");
          if (ImGui::RadioButton("None##CHV0",settings.channelVolStyle==0)) {
            settings.channelVolStyle=0;
          }
          if (ImGui::RadioButton("Simple##CHV1",settings.channelVolStyle==1)) {
            settings.channelVolStyle=1;
          }
          if (ImGui::RadioButton("Stereo##CHV2",settings.channelVolStyle==2)) {
            settings.channelVolStyle=2;
          }
          if (ImGui::RadioButton("Real##CHV3",settings.channelVolStyle==3)) {
            settings.channelVolStyle=3;
          }
          if (ImGui::RadioButton("Real (stereo)##CHV4",settings.channelVolStyle==4)) {
            settings.channelVolStyle=4;
          }

          ImGui::Text("Channel feedback style:");

          if (ImGui::RadioButton("Off##CHF0",settings.channelFeedbackStyle==0)) {
            settings.channelFeedbackStyle=0;
          }
          if (ImGui::RadioButton("Note##CHF1",settings.channelFeedbackStyle==1)) {
            settings.channelFeedbackStyle=1;
          }
          if (ImGui::RadioButton("Volume##CHF2",settings.channelFeedbackStyle==2)) {
            settings.channelFeedbackStyle=2;
          }
          if (ImGui::RadioButton("Active##CHF3",settings.channelFeedbackStyle==3)) {
            settings.channelFeedbackStyle=3;
          }

          ImGui::Text("Channel font:");

          if (ImGui::RadioButton("Regular##CHFont0",settings.channelFont==0)) {
            settings.channelFont=0;
          }
          if (ImGui::RadioButton("Monospace##CHFont1",settings.channelFont==1)) {
            settings.channelFont=1;
          }

          bool channelTextCenterB=settings.channelTextCenter;
          if (ImGui::Checkbox("Center channel name",&channelTextCenterB)) {
            settings.channelTextCenter=channelTextCenterB;
          }

          ImGui::Separator();

          bool insEditColorizeB=settings.insEditColorize;
          if (ImGui::Checkbox("Colorize instrument editor using instrument type",&insEditColorizeB)) {
            settings.insEditColorize=insEditColorizeB;
          }

          bool separateFMColorsB=settings.separateFMColors;
          if (ImGui::Checkbox("Use separate colors for carriers/modulators in FM editor",&separateFMColorsB)) {
            settings.separateFMColors=separateFMColorsB;
          }

          bool unifiedDataViewB=settings.unifiedDataView;
          if (ImGui::Checkbox("Unified instrument/wavetable/sample list",&unifiedDataViewB)) {
            settings.unifiedDataView=unifiedDataViewB;
          }
          if (settings.unifiedDataView) {
            settings.horizontalDataView=0;
          }

          ImGui::BeginDisabled(settings.unifiedDataView);
          bool horizontalDataViewB=settings.horizontalDataView;
          if (ImGui::Checkbox("Horizontal instrument list",&horizontalDataViewB)) {
            settings.horizontalDataView=horizontalDataViewB;
          }
          ImGui::EndDisabled();

          bool oplStandardWaveNamesB=settings.oplStandardWaveNames;
          if (ImGui::Checkbox("Use standard OPL waveform names",&oplStandardWaveNamesB)) {
            settings.oplStandardWaveNames=oplStandardWaveNamesB;
          }

          bool overflowHighlightB=settings.overflowHighlight;
          if (ImGui::Checkbox("Overflow pattern highlights",&overflowHighlightB)) {
            settings.overflowHighlight=overflowHighlightB;
          }

          bool viewPrevPatternB=settings.viewPrevPattern;
          if (ImGui::Checkbox("Display previous/next pattern",&viewPrevPatternB)) {
            settings.viewPrevPattern=viewPrevPatternB;
          }

          bool germanNotationB=settings.germanNotation;
          if (ImGui::Checkbox("Use German notation",&germanNotationB)) {
            settings.germanNotation=germanNotationB;
          }

          bool centerPatternB=settings.centerPattern;
          if (ImGui::Checkbox("Center pattern view",&centerPatternB)) {
            settings.centerPattern=centerPatternB;
          }

          bool unsignedDetuneB=settings.unsignedDetune;
          if (ImGui::Checkbox("Unsigned FM detune values",&unsignedDetuneB)) {
            settings.unsignedDetune=unsignedDetuneB;
          }

          // sorry. temporarily disabled until ImGui has a way to add separators in tables arbitrarily.
          /*bool sysSeparatorsB=settings.sysSeparators;
          if (ImGui::Checkbox("Add separators between systems in Orders",&sysSeparatorsB)) {
            settings.sysSeparators=sysSeparatorsB;
          }*/

          bool ordersCursorB=settings.ordersCursor;
          if (ImGui::Checkbox("Highlight channel at cursor in Orders",&ordersCursorB)) {
            settings.ordersCursor=ordersCursorB;
          }

          bool partyTimeB=settings.partyTime;
          if (ImGui::Checkbox("About screen party time",&partyTimeB)) {
            settings.partyTime=partyTimeB;
          }
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Warning: may cause epileptic seizures.");
          }

          ImGui::Separator();

          bool waveLayoutB=settings.waveLayout;
          if (ImGui::Checkbox("Use compact wave editor",&waveLayoutB)) {
            settings.waveLayout=waveLayoutB;
          }

          bool oldMacroVSliderB=settings.oldMacroVSlider;
          if (ImGui::Checkbox("Use classic macro editor vertical slider",&oldMacroVSliderB)) {
            settings.oldMacroVSlider=oldMacroVSliderB;
          }

          bool roundedWindowsB=settings.roundedWindows;
          if (ImGui::Checkbox("Rounded window corners",&roundedWindowsB)) {
            settings.roundedWindows=roundedWindowsB;
          }

          bool roundedButtonsB=settings.roundedButtons;
          if (ImGui::Checkbox("Rounded buttons",&roundedButtonsB)) {
            settings.roundedButtons=roundedButtonsB;
          }

          bool roundedMenusB=settings.roundedMenus;
          if (ImGui::Checkbox("Rounded menu corners",&roundedMenusB)) {
            settings.roundedMenus=roundedMenusB;
          }

          bool frameBordersB=settings.frameBorders;
          if (ImGui::Checkbox("Borders around widgets",&frameBordersB)) {
            settings.frameBorders=frameBordersB;
          }

          ImGui::Separator();

          ImGui::Text("Oscilloscope settings:");

          bool oscRoundedCornersB=settings.oscRoundedCorners;
          if (ImGui::Checkbox("Rounded corners",&oscRoundedCornersB)) {
            settings.oscRoundedCorners=oscRoundedCornersB;
          }

          bool oscTakesEntireWindowB=settings.oscTakesEntireWindow;
          if (ImGui::Checkbox("Fill entire window",&oscTakesEntireWindowB)) {
            settings.oscTakesEntireWindow=oscTakesEntireWindowB;
          }

          bool oscEscapesBoundaryB=settings.oscEscapesBoundary;
          if (ImGui::Checkbox("Waveform goes out of bounds",&oscEscapesBoundaryB)) {
            settings.oscEscapesBoundary=oscEscapesBoundaryB;
          }

          bool oscBorderB=settings.oscBorder;
          if (ImGui::Checkbox("Border",&oscBorderB)) {
            settings.oscBorder=oscBorderB;
          }

          ImGui::Separator();

          ImGui::Text("Pattern view spacing after:");

          if (CWSliderInt("Note",&settings.noteCellSpacing,0,32)) {
            if (settings.noteCellSpacing<0) settings.noteCellSpacing=0;
            if (settings.noteCellSpacing>32) settings.noteCellSpacing=32;
          }

          if (CWSliderInt("Instrument",&settings.insCellSpacing,0,32)) {
            if (settings.insCellSpacing<0) settings.insCellSpacing=0;
            if (settings.insCellSpacing>32) settings.insCellSpacing=32;
          }

          if (CWSliderInt("Volume",&settings.volCellSpacing,0,32)) {
            if (settings.volCellSpacing<0) settings.volCellSpacing=0;
            if (settings.volCellSpacing>32) settings.volCellSpacing=32;
          }

          if (CWSliderInt("Effect",&settings.effectCellSpacing,0,32)) {
            if (settings.effectCellSpacing<0) settings.effectCellSpacing=0;
            if (settings.effectCellSpacing>32) settings.effectCellSpacing=32;
          }

          if (CWSliderInt("Effect value",&settings.effectValCellSpacing,0,32)) {
            if (settings.effectValCellSpacing<0) settings.effectValCellSpacing=0;
            if (settings.effectValCellSpacing>32) settings.effectValCellSpacing=32;
          }

          ImGui::Separator();

          if (ImGui::TreeNode("Color scheme")) {
            if (ImGui::Button("Import")) {
              openFileDialog(GUI_FILE_IMPORT_COLORS);
            }
            ImGui::SameLine();
            if (ImGui::Button("Export")) {
              openFileDialog(GUI_FILE_EXPORT_COLORS);
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset defaults")) {
              showWarning("Are you sure you want to reset the color scheme?",GUI_WARN_RESET_COLORS);
            }
            if (ImGui::TreeNode("General")) {
              ImGui::Text("Color scheme type:");
              if (ImGui::RadioButton("Dark##gcb0",settings.guiColorsBase==0)) {
                settings.guiColorsBase=0;
                applyUISettings(false);
              }
              if (ImGui::RadioButton("Light##gcb1",settings.guiColorsBase==1)) {
                settings.guiColorsBase=1;
                applyUISettings(false);
              }
              if (ImGui::SliderInt("Frame shading",&settings.guiColorsShading,0,100,"%d%%")) {
                if (settings.guiColorsShading<0) settings.guiColorsShading=0;
                if (settings.guiColorsShading>100) settings.guiColorsShading=100;
                applyUISettings(false);
              }
              UI_COLOR_CONFIG(GUI_COLOR_BACKGROUND,"Background");
              UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND,"Window background");
              UI_COLOR_CONFIG(GUI_COLOR_MODAL_BACKDROP,"Modal backdrop");
              UI_COLOR_CONFIG(GUI_COLOR_HEADER,"Header");
              UI_COLOR_CONFIG(GUI_COLOR_TEXT,"Text");
              UI_COLOR_CONFIG(GUI_COLOR_ACCENT_PRIMARY,"Primary");
              UI_COLOR_CONFIG(GUI_COLOR_ACCENT_SECONDARY,"Secondary");
              UI_COLOR_CONFIG(GUI_COLOR_BORDER,"Border");
              UI_COLOR_CONFIG(GUI_COLOR_BORDER_SHADOW,"Border shadow");
              UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_ON,"Toggle on");
              UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_OFF,"Toggle off");
              UI_COLOR_CONFIG(GUI_COLOR_EDITING,"Editing");
              UI_COLOR_CONFIG(GUI_COLOR_SONG_LOOP,"Song loop");
              UI_COLOR_CONFIG(GUI_COLOR_PLAYBACK_STAT,"Playback status");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("File Picker (built-in)")) {
              UI_COLOR_CONFIG(GUI_COLOR_FILE_DIR,"Directory");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_SONG_NATIVE,"Song (native)");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_SONG_IMPORT,"Song (import)");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_INSTR,"Instrument");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_AUDIO,"Audio");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_WAVE,"Wavetable");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_VGM,"VGM");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_ZSM,"ZSM");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_FONT,"Font");
              UI_COLOR_CONFIG(GUI_COLOR_FILE_OTHER,"Other");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Oscilloscope")) {
              UI_COLOR_CONFIG(GUI_COLOR_OSC_BORDER,"Border");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_BG1,"Background (top-left)");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_BG2,"Background (top-right)");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_BG3,"Background (bottom-left)");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_BG4,"Background (bottom-right)");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE,"Waveform");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_WAVE_PEAK,"Waveform (clip)");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_REF,"Reference");
              UI_COLOR_CONFIG(GUI_COLOR_OSC_GUIDE,"Guide");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Volume Meter")) {
              UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_LOW,"Low");
              UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_HIGH,"High");
              UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_PEAK,"Clip");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Orders")) {
              UI_COLOR_CONFIG(GUI_COLOR_ORDER_ROW_INDEX,"Order number");
              UI_COLOR_CONFIG(GUI_COLOR_ORDER_ACTIVE,"Current order background");
              UI_COLOR_CONFIG(GUI_COLOR_ORDER_SIMILAR,"Similar patterns");
              UI_COLOR_CONFIG(GUI_COLOR_ORDER_INACTIVE,"Inactive patterns");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Envelope View")) {
              UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE,"Envelope");
              UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE_SUS_GUIDE,"Sustain guide");
              UI_COLOR_CONFIG(GUI_COLOR_FM_ENVELOPE_RELEASE,"Release");

              ImGui::TreePop();
            }
            if (ImGui::TreeNode("FM Editor")) {
              UI_COLOR_CONFIG(GUI_COLOR_FM_ALG_BG,"Algorithm background");
              UI_COLOR_CONFIG(GUI_COLOR_FM_ALG_LINE,"Algorithm lines");
              UI_COLOR_CONFIG(GUI_COLOR_FM_MOD,"Modulator");
              UI_COLOR_CONFIG(GUI_COLOR_FM_CAR,"Carrier");

              UI_COLOR_CONFIG(GUI_COLOR_FM_SSG,"SSG-EG");
              UI_COLOR_CONFIG(GUI_COLOR_FM_WAVE,"Waveform");

              ImGui::TextWrapped("(the following colors only apply when \"Use separate colors for carriers/modulators in FM editor\" is on!)");

              UI_COLOR_CONFIG(GUI_COLOR_FM_PRIMARY_MOD,"Mod. accent (primary)");
              UI_COLOR_CONFIG(GUI_COLOR_FM_SECONDARY_MOD,"Mod. accent (secondary)");
              UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_MOD,"Mod. border");
              UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_SHADOW_MOD,"Mod. border shadow");

              UI_COLOR_CONFIG(GUI_COLOR_FM_PRIMARY_CAR,"Car. accent (primary");
              UI_COLOR_CONFIG(GUI_COLOR_FM_SECONDARY_CAR,"Car. accent (secondary)");
              UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_CAR,"Car. border");
              UI_COLOR_CONFIG(GUI_COLOR_FM_BORDER_SHADOW_CAR,"Car. border shadow");

              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Macro Editor")) {
              UI_COLOR_CONFIG(GUI_COLOR_MACRO_VOLUME,"Volume");
              UI_COLOR_CONFIG(GUI_COLOR_MACRO_PITCH,"Pitch");
              UI_COLOR_CONFIG(GUI_COLOR_MACRO_WAVE,"Wave");
              UI_COLOR_CONFIG(GUI_COLOR_MACRO_OTHER,"Other");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Instrument Types")) {
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_FM,"FM (OPN)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_STD,"SN76489/Sega PSG");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_T6W28,"T6W28");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_GB,"Game Boy");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_C64,"C64");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_AMIGA,"Amiga/Generic Sample");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_PCE,"PC Engine");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY,"AY-3-8910/SSG");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY8930,"AY8930");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_TIA,"TIA");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_SAA1099,"SAA1099");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_VIC,"VIC");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_PET,"PET");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6,"VRC6");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6_SAW,"VRC6 (saw)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPLL,"FM (OPLL)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL,"FM (OPL)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_FDS,"FDS");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_VBOY,"Virtual Boy");
              // special case
              String c163Label=fmt::sprintf("%s##CC_GUI_COLOR_INSTR_N163",settings.c163Name);
              if (ImGui::ColorEdit4(c163Label.c_str(),(float*)&uiColors[GUI_COLOR_INSTR_N163])) {
                applyUISettings(false);
              }
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_SCC,"Konami SCC");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPZ,"FM (OPZ)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEY,"POKEY");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_BEEPER,"PC Beeper");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_SWAN,"WonderSwan");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_MIKEY,"Lynx");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_VERA,"VERA");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_X1_010,"X1-010");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_ES5506,"ES5506");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_MULTIPCM,"MultiPCM");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_SNES,"SNES");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_SU,"Sound Unit");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_NAMCO,"Namco WSG");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL_DRUMS,"FM (OPL Drums)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPM,"FM (OPM)");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_NES,"NES");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM6258,"MSM6258");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM6295,"MSM6295");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_ADPCMA,"ADPCM-A");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_ADPCMB,"ADPCM-B");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_SEGAPCM,"Sega PCM");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_QSOUND,"QSound");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_YMZ280B,"YMZ280B");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_RF5C68,"RF5C68");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_MSM5232,"MSM5232");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_K007232,"K007232");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_GA20,"GA20");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEMINI,"Pokémon Mini");
              UI_COLOR_CONFIG(GUI_COLOR_INSTR_UNKNOWN,"Other/Unknown");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Channel")) {
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_BG,"Single color (background)");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_FG,"Single color (text)");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_FM,"FM");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_PULSE,"Pulse");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_NOISE,"Noise");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_PCM,"PCM");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_WAVE,"Wave");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_OP,"FM operator");
              UI_COLOR_CONFIG(GUI_COLOR_CHANNEL_MUTED,"Muted");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Pattern")) {
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_PLAY_HEAD,"Playhead");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR,"Cursor");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_HOVER,"Cursor (hovered)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_ACTIVE,"Cursor (clicked)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION,"Selection");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_HOVER,"Selection (hovered)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_ACTIVE,"Selection (clicked)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_1,"Highlight 1");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_2,"Highlight 2");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX,"Row number");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX_HI1,"Row number (highlight 1)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX_HI2,"Row number (highlight 2)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE,"Note");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE_HI1,"Note (highlight 1)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE_HI2,"Note (highlight 2)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE,"Blank");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE_HI1,"Blank (highlight 1)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE_HI2,"Blank (highlight 2)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS,"Instrument");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS_WARN,"Instrument (invalid type)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS_ERROR,"Instrument (out of range)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_MIN,"Volume (0%)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_HALF,"Volume (50%)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_VOLUME_MAX,"Volume (100%)");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_INVALID,"Invalid effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_PITCH,"Pitch effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_VOLUME,"Volume effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_PANNING,"Panning effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SONG,"Song effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_TIME,"Time effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SPEED,"Speed effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,"Primary specific effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,"Secondary specific effect");
              UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_MISC,"Miscellaneous");
              UI_COLOR_CONFIG(GUI_COLOR_EE_VALUE,"External command output");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Sample Editor")) {
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_BG,"Background");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_FG,"Waveform");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_TIME_BG,"Time background");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_TIME_FG,"Time text");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_LOOP,"Loop region");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CENTER,"Center guide");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_GRID,"Grid");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_SEL,"Selection");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_SEL_POINT,"Selection points");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_NEEDLE,"Preview needle");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_NEEDLE_PLAYING,"Playing needles");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_LOOP_POINT,"Loop markers");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_DISABLED,"Chip select: disabled");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_ENABLED,"Chip select: enabled");
              UI_COLOR_CONFIG(GUI_COLOR_SAMPLE_CHIP_WARNING,"Chip select: enabled (failure)");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Pattern Manager")) {
              UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_NULL,"Unallocated");
              UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_UNUSED,"Unused");
              UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_USED,"Used");
              UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_OVERUSED,"Overused");
              UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_EXTREMELY_OVERUSED,"Really overused");
              UI_COLOR_CONFIG(GUI_COLOR_PAT_MANAGER_COMBO_BREAKER,"Combo Breaker");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Piano")) {
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_BACKGROUND,"Background");
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP,"Upper key");
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP_HIT,"Upper key (feedback)");
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_TOP_ACTIVE,"Upper key (pressed)");
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM,"Lower key");
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM_HIT,"Lower key (feedback)");
              UI_COLOR_CONFIG(GUI_COLOR_PIANO_KEY_BOTTOM_ACTIVE,"Lower key (pressed)");
              ImGui::TreePop();
            }
            if (ImGui::TreeNode("Log Viewer")) {
              UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_ERROR,"Log level: Error");
              UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_WARNING,"Log level: Warning");
              UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_INFO,"Log level: Info");
              UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_DEBUG,"Log level: Debug");
              UI_COLOR_CONFIG(GUI_COLOR_LOGLEVEL_TRACE,"Log level: Trace/Verbose");
              ImGui::TreePop();
            }
            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Keyboard")) {
        ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
        settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
        if (ImGui::BeginChild("SettingsView",settingsViewSize)) {
          if (ImGui::Button("Import")) {
            openFileDialog(GUI_FILE_IMPORT_KEYBINDS);
          }
          ImGui::SameLine();
          if (ImGui::Button("Export")) {
            openFileDialog(GUI_FILE_EXPORT_KEYBINDS);
          }
          ImGui::SameLine();
          if (ImGui::Button("Reset defaults")) {
            showWarning("Are you sure you want to reset the keyboard settings?",GUI_WARN_RESET_KEYBINDS);
          }
          if (ImGui::TreeNode("Global hotkeys")) {
            KEYBIND_CONFIG_BEGIN("keysGlobal");

            UI_KEYBIND_CONFIG(GUI_ACTION_OPEN);
            UI_KEYBIND_CONFIG(GUI_ACTION_OPEN_BACKUP);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAVE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAVE_AS);
            UI_KEYBIND_CONFIG(GUI_ACTION_UNDO);
            UI_KEYBIND_CONFIG(GUI_ACTION_REDO);
            UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_TOGGLE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PLAY);
            UI_KEYBIND_CONFIG(GUI_ACTION_STOP);
            UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_START);
            UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_REPEAT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_CURSOR);
            UI_KEYBIND_CONFIG(GUI_ACTION_STEP_ONE);
            UI_KEYBIND_CONFIG(GUI_ACTION_OCTAVE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_OCTAVE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_STEP_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_STEP_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_TOGGLE_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_METRONOME);
            UI_KEYBIND_CONFIG(GUI_ACTION_REPEAT_PATTERN);
            UI_KEYBIND_CONFIG(GUI_ACTION_FOLLOW_ORDERS);
            UI_KEYBIND_CONFIG(GUI_ACTION_FOLLOW_PATTERN);
            UI_KEYBIND_CONFIG(GUI_ACTION_FULLSCREEN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PANIC);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Window activation")) {
            KEYBIND_CONFIG_BEGIN("keysWindow");

            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_EDIT_CONTROLS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_ORDERS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_INS_LIST);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_INS_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SONG_INFO);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SUBSONGS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_PATTERN);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_WAVE_LIST);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_WAVE_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SAMPLE_LIST);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SAMPLE_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_ABOUT);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SETTINGS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_MIXER);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_DEBUG);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_OSCILLOSCOPE);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_CHAN_OSC);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_EFFECT_LIST);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_VOL_METER);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_STATS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_COMPAT_FLAGS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_PIANO);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_NOTES);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_CHANNELS);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_REGISTER_VIEW);
            UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_LOG);

            UI_KEYBIND_CONFIG(GUI_ACTION_COLLAPSE_WINDOW);
            UI_KEYBIND_CONFIG(GUI_ACTION_CLOSE_WINDOW);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Note input")) {
            std::vector<MappedInput> sorted;
            if (ImGui::BeginTable("keysNoteInput",4)) {
              for (std::map<int,int>::value_type& i: noteKeys) {
                std::vector<MappedInput>::iterator j;
                for (j=sorted.begin(); j!=sorted.end(); j++) {
                  if (j->val>i.second) {
                    break;
                  }
                }
                sorted.insert(j,MappedInput(i.first,i.second));
              }

              static char id[4096];

              ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
              ImGui::TableNextColumn();
              ImGui::Text("Key");
              ImGui::TableNextColumn();
              ImGui::Text("Type");
              ImGui::TableNextColumn();
              ImGui::Text("Value");
              ImGui::TableNextColumn();
              ImGui::Text("Remove");

              for (MappedInput& i: sorted) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s",SDL_GetScancodeName((SDL_Scancode)i.scan));
                ImGui::TableNextColumn();
                if (i.val==102) {
                  snprintf(id,4095,"Macro release##SNType_%d",i.scan);
                  if (ImGui::Button(id)) {
                    noteKeys[i.scan]=0;
                  }
                } else if (i.val==101) {
                  snprintf(id,4095,"Note release##SNType_%d",i.scan);
                  if (ImGui::Button(id)) {
                    noteKeys[i.scan]=102;
                  }
                } else if (i.val==100) {
                  snprintf(id,4095,"Note off##SNType_%d",i.scan);
                  if (ImGui::Button(id)) {
                    noteKeys[i.scan]=101;
                  }
                } else {
                  snprintf(id,4095,"Note##SNType_%d",i.scan);
                  if (ImGui::Button(id)) {
                    noteKeys[i.scan]=100;
                  }
                }
                ImGui::TableNextColumn();
                if (i.val<100) {
                  snprintf(id,4095,"##SNValue_%d",i.scan);
                  if (ImGui::InputInt(id,&i.val,1,1)) {
                    if (i.val<0) i.val=0;
                    if (i.val>96) i.val=96;
                    noteKeys[i.scan]=i.val;
                  }
                }
                ImGui::TableNextColumn();
                snprintf(id,4095,ICON_FA_TIMES "##SNRemove_%d",i.scan);
                if (ImGui::Button(id)) {
                  noteKeys.erase(i.scan);
                }
              }
              ImGui::EndTable();

              if (ImGui::BeginCombo("##SNAddNew","Add...")) {
                for (int i=0; i<SDL_NUM_SCANCODES; i++) {
                  const char* sName=SDL_GetScancodeName((SDL_Scancode)i);
                  if (sName==NULL) continue;
                  if (sName[0]==0) continue;
                  snprintf(id,4095,"%s##SNNewKey_%d",sName,i);
                  if (ImGui::Selectable(id)) {
                    noteKeys[(SDL_Scancode)i]=0;
                  }
                }
                ImGui::EndCombo();
              }
            }
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Pattern")) {
            KEYBIND_CONFIG_BEGIN("keysPattern");

            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_NOTE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_NOTE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_OCTAVE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_OCTAVE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_VALUE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_VALUE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_VALUE_UP_COARSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_VALUE_DOWN_COARSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECT_ALL);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CUT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_COPY);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PASTE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PASTE_MIX);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PASTE_MIX_BG);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PASTE_FLOOD);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PASTE_OVERFLOW);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_LEFT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_RIGHT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_UP_ONE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_DOWN_ONE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_BEGIN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_END);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_UP_COARSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_DOWN_COARSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_LEFT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_RIGHT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_UP_ONE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_DOWN_ONE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_BEGIN);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_END);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_UP_COARSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_DOWN_COARSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_DELETE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PULL_DELETE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_INSERT);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_MUTE_CURSOR);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SOLO_CURSOR);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_UNMUTE_ALL);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_NEXT_ORDER);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PREV_ORDER);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_COLLAPSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_INCREASE_COLUMNS);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_DECREASE_COLUMNS);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_INTERPOLATE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_FADE);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_INVERT_VALUES);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_FLIP_SELECTION);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_COLLAPSE_ROWS);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_EXPAND_ROWS);
            UI_KEYBIND_CONFIG(GUI_ACTION_PAT_LATCH);

            // TODO: collapse/expand pattern and song

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Instrument list")) {
            KEYBIND_CONFIG_BEGIN("keysInsList");

            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_ADD);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_DUPLICATE);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_OPEN);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_OPEN_REPLACE);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_SAVE);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_MOVE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_MOVE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_DELETE);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_DOWN);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Wavetable list")) {
            KEYBIND_CONFIG_BEGIN("keysWaveList");

            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_ADD);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_DUPLICATE);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_OPEN);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_SAVE);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_MOVE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_DELETE);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_DOWN);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Sample list")) {
            KEYBIND_CONFIG_BEGIN("keysSampleList");

            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_ADD);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_OPEN);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_SAVE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_DELETE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_EDIT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_PREVIEW);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Orders")) {
            KEYBIND_CONFIG_BEGIN("keysOrders");

            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_LEFT);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_RIGHT);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_INCREASE);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DECREASE);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_EDIT_MODE);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_LINK);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_ADD);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DUPLICATE);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DEEP_CLONE);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DUPLICATE_END);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DEEP_CLONE_END);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_REMOVE);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_MOVE_UP);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_MOVE_DOWN);
            UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_REPLAY);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Sample editor")) {
            KEYBIND_CONFIG_BEGIN("keysSampleEdit");

            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SELECT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_DRAW);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_CUT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_COPY);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PASTE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PASTE_REPLACE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PASTE_MIX);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SELECT_ALL);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_RESIZE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_RESAMPLE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_AMPLIFY);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_NORMALIZE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_FADE_IN);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_FADE_OUT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_INSERT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SILENCE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_DELETE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_TRIM);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_REVERSE);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_INVERT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SIGN);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_FILTER);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PREVIEW);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_STOP_PREVIEW);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_ZOOM_IN);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_ZOOM_OUT);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_ZOOM_AUTO);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_MAKE_INS);
            UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SET_LOOP);

            KEYBIND_CONFIG_END;
            ImGui::TreePop();
          }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
      }
      if (nonLatchNibble) {
        // ok, so you decided to read the code.
        // these are the cheat codes:
        // "Debug" - toggles mobile UI
        // "Nice Amiga cover of the song!" - enables hidden systems (YMU759/SoundUnit/Dummy)
        // "42 63" - enables all instrument types
        if (ImGui::BeginTabItem("Cheat Codes")) {
          ImVec2 settingsViewSize=ImGui::GetContentRegionAvail();
          settingsViewSize.y-=ImGui::GetFrameHeight()+ImGui::GetStyle().WindowPadding.y;
          if (ImGui::BeginChild("SettingsView",settingsViewSize)) {
            ImGui::Text("Enter code:");
            ImGui::InputText("##CheatCode",&mmlString[31]);
            if (ImGui::Button("Submit")) {
              unsigned int checker=0x11111111;
              unsigned int checker1=0;
              int index=0;
              mmlString[30]="invalid code";

              for (char& i: mmlString[31]) {
                checker^=((unsigned int)i)<<index;
                checker1+=i;
                checker=(checker>>1|(((checker)^(checker>>2)^(checker>>3)^(checker>>5))&1)<<31);
                checker1<<=1;
                index=(index+1)&31;
              }
              if (checker==0x90888b65 && checker1==0x1482) {
                mmlString[30]="toggled alternate UI";
                toggleMobileUI(!mobileUI);
              }
              if (checker==0x5a42a113 && checker1==0xe4ef451e) {
                mmlString[30]=":smile: :star_struck: :sunglasses: :ok_hand:";
                settings.hiddenSystems=!settings.hiddenSystems;
              }
              if (checker==0xe888896b && checker1==0xbde) {
                mmlString[30]="enabled all instrument types";
                settings.displayAllInsTypes=!settings.displayAllInsTypes;
              }

              mmlString[31]="";
            }
            ImGui::Text("%s",mmlString[30].c_str());
          }
          ImGui::EndChild();
          ImGui::EndTabItem();
        }
      }
      ImGui::EndTabBar();
    }
    ImGui::Separator();
    if (ImGui::Button("OK##SettingsOK")) {
      settingsOpen=false;
      willCommit=true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel##SettingsCancel")) {
      settingsOpen=false;
      syncSettings();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SETTINGS;
  ImGui::End();
}

#define clampSetting(x,minV,maxV) \
  if (x<minV) { \
    x=minV; \
  } \
  if (x>maxV) { \
    x=maxV; \
  }

void FurnaceGUI::syncSettings() {
  settings.mainFontSize=e->getConfInt("mainFontSize",18);
  settings.patFontSize=e->getConfInt("patFontSize",18);
  settings.iconSize=e->getConfInt("iconSize",16);
  settings.audioEngine=(e->getConfString("audioEngine","SDL")=="SDL")?1:0;
  settings.audioDevice=e->getConfString("audioDevice","");
  settings.midiInDevice=e->getConfString("midiInDevice","");
  settings.midiOutDevice=e->getConfString("midiOutDevice","");
  settings.c163Name=e->getConfString("c163Name",DIV_C163_DEFAULT_NAME);
  settings.audioQuality=e->getConfInt("audioQuality",0);
  settings.audioBufSize=e->getConfInt("audioBufSize",1024);
  settings.audioRate=e->getConfInt("audioRate",44100);
  settings.arcadeCore=e->getConfInt("arcadeCore",0);
  settings.ym2612Core=e->getConfInt("ym2612Core",0);
  settings.snCore=e->getConfInt("snCore",0);
  settings.nesCore=e->getConfInt("nesCore",0);
  settings.fdsCore=e->getConfInt("fdsCore",0);
  settings.c64Core=e->getConfInt("c64Core",1);
  settings.pokeyCore=e->getConfInt("pokeyCore",1);
  settings.opnCore=e->getConfInt("opnCore",1);
  settings.pcSpeakerOutMethod=e->getConfInt("pcSpeakerOutMethod",0);
  settings.yrw801Path=e->getConfString("yrw801Path","");
  settings.tg100Path=e->getConfString("tg100Path","");
  settings.mu5Path=e->getConfString("mu5Path","");
  settings.mainFont=e->getConfInt("mainFont",0);
  settings.patFont=e->getConfInt("patFont",0);
  settings.mainFontPath=e->getConfString("mainFontPath","");
  settings.patFontPath=e->getConfString("patFontPath","");
  settings.patRowsBase=e->getConfInt("patRowsBase",0);
  settings.orderRowsBase=e->getConfInt("orderRowsBase",1);
  settings.soloAction=e->getConfInt("soloAction",0);
  settings.pullDeleteBehavior=e->getConfInt("pullDeleteBehavior",1);
  settings.wrapHorizontal=e->getConfInt("wrapHorizontal",0);
  settings.wrapVertical=e->getConfInt("wrapVertical",0);
  settings.macroView=e->getConfInt("macroView",0);
  settings.fmNames=e->getConfInt("fmNames",0);
  settings.allowEditDocking=e->getConfInt("allowEditDocking",1);
  settings.chipNames=e->getConfInt("chipNames",0);
  settings.overflowHighlight=e->getConfInt("overflowHighlight",0);
  settings.partyTime=e->getConfInt("partyTime",0);
  settings.germanNotation=e->getConfInt("germanNotation",0);
  settings.stepOnDelete=e->getConfInt("stepOnDelete",0);
  settings.scrollStep=e->getConfInt("scrollStep",0);
  settings.sysSeparators=e->getConfInt("sysSeparators",1);
  settings.forceMono=e->getConfInt("forceMono",0);
  settings.controlLayout=e->getConfInt("controlLayout",3);
  settings.restartOnFlagChange=e->getConfInt("restartOnFlagChange",1);
  settings.statusDisplay=e->getConfInt("statusDisplay",0);
  settings.dpiScale=e->getConfFloat("dpiScale",0.0f);
  settings.viewPrevPattern=e->getConfInt("viewPrevPattern",1);
  settings.guiColorsBase=e->getConfInt("guiColorsBase",0);
  settings.guiColorsShading=e->getConfInt("guiColorsShading",0);
  settings.avoidRaisingPattern=e->getConfInt("avoidRaisingPattern",0);
  settings.insFocusesPattern=e->getConfInt("insFocusesPattern",1);
  settings.stepOnInsert=e->getConfInt("stepOnInsert",0);
  settings.unifiedDataView=e->getConfInt("unifiedDataView",0);
  settings.sysFileDialog=e->getConfInt("sysFileDialog",SYS_FILE_DIALOG_DEFAULT);
  settings.roundedWindows=e->getConfInt("roundedWindows",1);
  settings.roundedButtons=e->getConfInt("roundedButtons",1);
  settings.roundedMenus=e->getConfInt("roundedMenus",0);
  settings.loadJapanese=e->getConfInt("loadJapanese",0);
  settings.loadChinese=e->getConfInt("loadChinese",0);
  settings.loadChineseTraditional=e->getConfInt("loadChineseTraditional",0);
  settings.loadKorean=e->getConfInt("loadKorean",0);
  settings.fmLayout=e->getConfInt("fmLayout",0);
  settings.sampleLayout=e->getConfInt("sampleLayout",0);
  settings.waveLayout=e->getConfInt("waveLayout",0);
  settings.susPosition=e->getConfInt("susPosition",0);
  settings.effectCursorDir=e->getConfInt("effectCursorDir",1);
  settings.cursorPastePos=e->getConfInt("cursorPastePos",1);
  settings.titleBarInfo=e->getConfInt("titleBarInfo",1);
  settings.titleBarSys=e->getConfInt("titleBarSys",1);
  settings.frameBorders=e->getConfInt("frameBorders",0);
  settings.effectDeletionAltersValue=e->getConfInt("effectDeletionAltersValue",1);
  settings.oscRoundedCorners=e->getConfInt("oscRoundedCorners",1);
  settings.oscTakesEntireWindow=e->getConfInt("oscTakesEntireWindow",0);
  settings.oscBorder=e->getConfInt("oscBorder",1);
  settings.oscEscapesBoundary=e->getConfInt("oscEscapesBoundary",0);
  settings.separateFMColors=e->getConfInt("separateFMColors",0);
  settings.insEditColorize=e->getConfInt("insEditColorize",0);
  settings.metroVol=e->getConfInt("metroVol",100);
  settings.pushNibble=e->getConfInt("pushNibble",0);
  settings.scrollChangesOrder=e->getConfInt("scrollChangesOrder",0);
  settings.oplStandardWaveNames=e->getConfInt("oplStandardWaveNames",0);
  settings.cursorMoveNoScroll=e->getConfInt("cursorMoveNoScroll",0);
  settings.lowLatency=e->getConfInt("lowLatency",0);
  settings.notePreviewBehavior=e->getConfInt("notePreviewBehavior",1);
  settings.powerSave=e->getConfInt("powerSave",POWER_SAVE_DEFAULT);
  settings.absorbInsInput=e->getConfInt("absorbInsInput",0);
  settings.eventDelay=e->getConfInt("eventDelay",0);
  settings.moveWindowTitle=e->getConfInt("moveWindowTitle",1);
  settings.hiddenSystems=e->getConfInt("hiddenSystems",0);
  settings.horizontalDataView=e->getConfInt("horizontalDataView",0);
  settings.noMultiSystem=e->getConfInt("noMultiSystem",0);
  settings.oldMacroVSlider=e->getConfInt("oldMacroVSlider",0);
  settings.displayAllInsTypes=e->getConfInt("displayAllInsTypes",0);
  settings.noteCellSpacing=e->getConfInt("noteCellSpacing",0);
  settings.insCellSpacing=e->getConfInt("insCellSpacing",0);
  settings.volCellSpacing=e->getConfInt("volCellSpacing",0);
  settings.effectCellSpacing=e->getConfInt("effectCellSpacing",0);
  settings.effectValCellSpacing=e->getConfInt("effectValCellSpacing",0);
  settings.doubleClickColumn=e->getConfInt("doubleClickColumn",1);
  settings.blankIns=e->getConfInt("blankIns",0);
  settings.dragMovesSelection=e->getConfInt("dragMovesSelection",2);
  settings.unsignedDetune=e->getConfInt("unsignedDetune",0);
  settings.noThreadedInput=e->getConfInt("noThreadedInput",0);
  settings.saveWindowPos=e->getConfInt("saveWindowPos",1);
  settings.initialSysName=e->getConfString("initialSysName","");
  settings.clampSamples=e->getConfInt("clampSamples",0);
  settings.noteOffLabel=e->getConfString("noteOffLabel","OFF");
  settings.noteRelLabel=e->getConfString("noteRelLabel","===");
  settings.macroRelLabel=e->getConfString("macroRelLabel","REL");
  settings.emptyLabel=e->getConfString("emptyLabel","...");
  settings.emptyLabel2=e->getConfString("emptyLabel2","..");
  settings.saveUnusedPatterns=e->getConfInt("saveUnusedPatterns",0);
  settings.channelColors=e->getConfInt("channelColors",1);
  settings.channelTextColors=e->getConfInt("channelTextColors",0);
  settings.channelStyle=e->getConfInt("channelStyle",1);
  settings.channelVolStyle=e->getConfInt("channelVolStyle",0);
  settings.channelFeedbackStyle=e->getConfInt("channelFeedbackStyle",1);
  settings.channelFont=e->getConfInt("channelFont",1);
  settings.channelTextCenter=e->getConfInt("channelTextCenter",1);
  settings.maxRecentFile=e->getConfInt("maxRecentFile",10);
  settings.midiOutClock=e->getConfInt("midiOutClock",0);
  settings.midiOutMode=e->getConfInt("midiOutMode",1);
  settings.centerPattern=e->getConfInt("centerPattern",0);
  settings.ordersCursor=e->getConfInt("ordersCursor",1);
  settings.persistFadeOut=e->getConfInt("persistFadeOut",1);
  settings.exportLoops=e->getConfInt("exportLoops",0);
  settings.exportFadeOut=e->getConfDouble("exportFadeOut",0.0);
  settings.macroLayout=e->getConfInt("macroLayout",0);

  clampSetting(settings.mainFontSize,2,96);
  clampSetting(settings.patFontSize,2,96);
  clampSetting(settings.iconSize,2,48);
  clampSetting(settings.audioEngine,0,1);
  clampSetting(settings.audioQuality,0,1);
  clampSetting(settings.audioBufSize,32,4096);
  clampSetting(settings.audioRate,8000,384000);
  clampSetting(settings.arcadeCore,0,1);
  clampSetting(settings.ym2612Core,0,1);
  clampSetting(settings.snCore,0,1);
  clampSetting(settings.nesCore,0,1);
  clampSetting(settings.fdsCore,0,1);
  clampSetting(settings.c64Core,0,1);
  clampSetting(settings.pokeyCore,0,1);
  clampSetting(settings.opnCore,0,1);
  clampSetting(settings.pcSpeakerOutMethod,0,4);
  clampSetting(settings.mainFont,0,6);
  clampSetting(settings.patFont,0,6);
  clampSetting(settings.patRowsBase,0,1);
  clampSetting(settings.orderRowsBase,0,1);
  clampSetting(settings.soloAction,0,2);
  clampSetting(settings.pullDeleteBehavior,0,1);
  clampSetting(settings.wrapHorizontal,0,2);
  clampSetting(settings.wrapVertical,0,2);
  clampSetting(settings.macroView,0,1);
  clampSetting(settings.fmNames,0,2);
  clampSetting(settings.allowEditDocking,0,1);
  clampSetting(settings.chipNames,0,1);
  clampSetting(settings.overflowHighlight,0,1);
  clampSetting(settings.partyTime,0,1);
  clampSetting(settings.germanNotation,0,1);
  clampSetting(settings.stepOnDelete,0,1);
  clampSetting(settings.scrollStep,0,1);
  clampSetting(settings.sysSeparators,0,1);
  clampSetting(settings.forceMono,0,1);
  clampSetting(settings.controlLayout,0,3);
  clampSetting(settings.statusDisplay,0,3);
  clampSetting(settings.dpiScale,0.0f,4.0f);
  clampSetting(settings.viewPrevPattern,0,1);
  clampSetting(settings.guiColorsBase,0,1);
  clampSetting(settings.guiColorsShading,0,100);
  clampSetting(settings.avoidRaisingPattern,0,1);
  clampSetting(settings.insFocusesPattern,0,1);
  clampSetting(settings.stepOnInsert,0,1);
  clampSetting(settings.unifiedDataView,0,1);
  clampSetting(settings.sysFileDialog,0,1);
  clampSetting(settings.roundedWindows,0,1);
  clampSetting(settings.roundedButtons,0,1);
  clampSetting(settings.roundedMenus,0,1);
  clampSetting(settings.loadJapanese,0,1);
  clampSetting(settings.loadChinese,0,1);
  clampSetting(settings.loadChineseTraditional,0,1);
  clampSetting(settings.loadKorean,0,1);
  clampSetting(settings.fmLayout,0,6);
  clampSetting(settings.susPosition,0,1);
  clampSetting(settings.effectCursorDir,0,2);
  clampSetting(settings.cursorPastePos,0,1);
  clampSetting(settings.titleBarInfo,0,3);
  clampSetting(settings.titleBarSys,0,1);
  clampSetting(settings.frameBorders,0,1);
  clampSetting(settings.effectDeletionAltersValue,0,1);
  clampSetting(settings.sampleLayout,0,1);
  clampSetting(settings.waveLayout,0,1);
  clampSetting(settings.separateFMColors,0,1);
  clampSetting(settings.insEditColorize,0,1);
  clampSetting(settings.metroVol,0,200);
  clampSetting(settings.pushNibble,0,1);
  clampSetting(settings.scrollChangesOrder,0,1);
  clampSetting(settings.oplStandardWaveNames,0,1);
  clampSetting(settings.cursorMoveNoScroll,0,1);
  clampSetting(settings.lowLatency,0,1);
  clampSetting(settings.notePreviewBehavior,0,3);
  clampSetting(settings.powerSave,0,1);
  clampSetting(settings.absorbInsInput,0,1);
  clampSetting(settings.eventDelay,0,1);
  clampSetting(settings.moveWindowTitle,0,1);
  clampSetting(settings.hiddenSystems,0,1);
  clampSetting(settings.horizontalDataView,0,1);
  clampSetting(settings.noMultiSystem,0,1);
  clampSetting(settings.oldMacroVSlider,0,1);
  clampSetting(settings.displayAllInsTypes,0,1);
  clampSetting(settings.noteCellSpacing,0,32);
  clampSetting(settings.insCellSpacing,0,32);
  clampSetting(settings.volCellSpacing,0,32);
  clampSetting(settings.effectCellSpacing,0,32);
  clampSetting(settings.effectValCellSpacing,0,32);
  clampSetting(settings.doubleClickColumn,0,1);
  clampSetting(settings.blankIns,0,1);
  clampSetting(settings.dragMovesSelection,0,2);
  clampSetting(settings.unsignedDetune,0,1);
  clampSetting(settings.noThreadedInput,0,1);
  clampSetting(settings.saveWindowPos,0,1);
  clampSetting(settings.clampSamples,0,1);
  clampSetting(settings.saveUnusedPatterns,0,1);
  clampSetting(settings.channelColors,0,2);
  clampSetting(settings.channelTextColors,0,2);
  clampSetting(settings.channelStyle,0,5);
  clampSetting(settings.channelVolStyle,0,4);
  clampSetting(settings.channelFeedbackStyle,0,3);
  clampSetting(settings.channelFont,0,1);
  clampSetting(settings.channelTextCenter,0,1);
  clampSetting(settings.maxRecentFile,0,30);
  clampSetting(settings.midiOutClock,0,1);
  clampSetting(settings.midiOutMode,0,2);
  clampSetting(settings.centerPattern,0,1);
  clampSetting(settings.ordersCursor,0,1);
  clampSetting(settings.persistFadeOut,0,1);
  clampSetting(settings.macroLayout,0,4);

  if (settings.exportLoops<0.0) settings.exportLoops=0.0;
  if (settings.exportFadeOut<0.0) settings.exportFadeOut=0.0;

  String initialSys2=e->getConfString("initialSys2","");
  if (initialSys2.empty()) {
    initialSys2=e->decodeSysDesc(e->getConfString("initialSys",""));
  }
  settings.initialSys.clear();
  settings.initialSys.loadFromBase64(initialSys2.c_str());
  if (settings.initialSys.getInt("id0",0)==0) {
    settings.initialSys.clear();
    settings.initialSys.set("id0",e->systemToFileFur(DIV_SYSTEM_YM2612));
    settings.initialSys.set("vol0",64);
    settings.initialSys.set("pan0",0);
    settings.initialSys.set("flags0","");
    settings.initialSys.set("id1",e->systemToFileFur(DIV_SYSTEM_SMS));
    settings.initialSys.set("vol1",64);
    settings.initialSys.set("pan1",0);
    settings.initialSys.set("flags1","");
  }

  // keybinds
  for (int i=0; i<GUI_ACTION_MAX; i++) {
    if (guiActions[i].defaultBind==-1) continue; // not a bind
    actionKeys[i]=e->getConfInt(String("keybind_GUI_ACTION_")+String(guiActions[i].name),guiActions[i].defaultBind);
  }

  decodeKeyMap(noteKeys,e->getConfString("noteKeys",DEFAULT_NOTE_KEYS));

  parseKeybinds();

  midiMap.read(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");
  midiMap.compile();

  e->setMidiDirect(midiMap.directChannel);
  e->setMetronomeVol(((float)settings.metroVol)/100.0f);
}

void FurnaceGUI::commitSettings() {
  bool sampleROMsChanged=settings.yrw801Path!=e->getConfString("yrw801Path","") ||
    settings.tg100Path!=e->getConfString("tg100Path","") ||
    settings.mu5Path!=e->getConfString("mu5Path","");

  bool coresChanged=(
    settings.arcadeCore!=e->getConfInt("arcadeCore",0) ||
    settings.ym2612Core!=e->getConfInt("ym2612Core",0) ||
    settings.snCore!=e->getConfInt("snCore",0) ||
    settings.nesCore!=e->getConfInt("nesCore",0) ||
    settings.fdsCore!=e->getConfInt("fdsCore",0) ||
    settings.c64Core!=e->getConfInt("c64Core",1) ||
    settings.pokeyCore!=e->getConfInt("pokeyCore",1) ||
    settings.opnCore!=e->getConfInt("opnCore",1)
  );

  e->setConf("mainFontSize",settings.mainFontSize);
  e->setConf("patFontSize",settings.patFontSize);
  e->setConf("iconSize",settings.iconSize);
  e->setConf("audioEngine",String(audioBackends[settings.audioEngine]));
  e->setConf("audioDevice",settings.audioDevice);
  e->setConf("midiInDevice",settings.midiInDevice);
  e->setConf("midiOutDevice",settings.midiOutDevice);
  e->setConf("c163Name",settings.c163Name);
  e->setConf("audioQuality",settings.audioQuality);
  e->setConf("audioBufSize",settings.audioBufSize);
  e->setConf("audioRate",settings.audioRate);
  e->setConf("arcadeCore",settings.arcadeCore);
  e->setConf("ym2612Core",settings.ym2612Core);
  e->setConf("snCore",settings.snCore);
  e->setConf("nesCore",settings.nesCore);
  e->setConf("fdsCore",settings.fdsCore);
  e->setConf("c64Core",settings.c64Core);
  e->setConf("pokeyCore",settings.pokeyCore);
  e->setConf("opnCore",settings.opnCore);
  e->setConf("pcSpeakerOutMethod",settings.pcSpeakerOutMethod);
  e->setConf("yrw801Path",settings.yrw801Path);
  e->setConf("tg100Path",settings.tg100Path);
  e->setConf("mu5Path",settings.mu5Path);
  e->setConf("mainFont",settings.mainFont);
  e->setConf("patFont",settings.patFont);
  e->setConf("mainFontPath",settings.mainFontPath);
  e->setConf("patFontPath",settings.patFontPath);
  e->setConf("patRowsBase",settings.patRowsBase);
  e->setConf("orderRowsBase",settings.orderRowsBase);
  e->setConf("soloAction",settings.soloAction);
  e->setConf("pullDeleteBehavior",settings.pullDeleteBehavior);
  e->setConf("wrapHorizontal",settings.wrapHorizontal);
  e->setConf("wrapVertical",settings.wrapVertical);
  e->setConf("macroView",settings.macroView);
  e->setConf("fmNames",settings.fmNames);
  e->setConf("allowEditDocking",settings.allowEditDocking);
  e->setConf("chipNames",settings.chipNames);
  e->setConf("overflowHighlight",settings.overflowHighlight);
  e->setConf("partyTime",settings.partyTime);
  e->setConf("germanNotation",settings.germanNotation);
  e->setConf("stepOnDelete",settings.stepOnDelete);
  e->setConf("scrollStep",settings.scrollStep);
  e->setConf("sysSeparators",settings.sysSeparators);
  e->setConf("forceMono",settings.forceMono);
  e->setConf("controlLayout",settings.controlLayout);
  e->setConf("restartOnFlagChange",settings.restartOnFlagChange);
  e->setConf("statusDisplay",settings.statusDisplay);
  e->setConf("dpiScale",settings.dpiScale);
  e->setConf("viewPrevPattern",settings.viewPrevPattern);
  e->setConf("guiColorsBase",settings.guiColorsBase);
  e->setConf("guiColorsShading",settings.guiColorsShading);
  e->setConf("avoidRaisingPattern",settings.avoidRaisingPattern);
  e->setConf("insFocusesPattern",settings.insFocusesPattern);
  e->setConf("stepOnInsert",settings.stepOnInsert);
  e->setConf("unifiedDataView",settings.unifiedDataView);
  e->setConf("sysFileDialog",settings.sysFileDialog);
  e->setConf("roundedWindows",settings.roundedWindows);
  e->setConf("roundedButtons",settings.roundedButtons);
  e->setConf("roundedMenus",settings.roundedMenus);
  e->setConf("loadJapanese",settings.loadJapanese);
  e->setConf("loadChinese",settings.loadChinese);
  e->setConf("loadChineseTraditional",settings.loadChineseTraditional);
  e->setConf("loadKorean",settings.loadKorean);
  e->setConf("fmLayout",settings.fmLayout);
  e->setConf("sampleLayout",settings.sampleLayout);
  e->setConf("waveLayout",settings.waveLayout);
  e->setConf("susPosition",settings.susPosition);
  e->setConf("effectCursorDir",settings.effectCursorDir);
  e->setConf("cursorPastePos",settings.cursorPastePos);
  e->setConf("titleBarInfo",settings.titleBarInfo);
  e->setConf("titleBarSys",settings.titleBarSys);
  e->setConf("frameBorders",settings.frameBorders);
  e->setConf("effectDeletionAltersValue",settings.effectDeletionAltersValue);
  e->setConf("oscRoundedCorners",settings.oscRoundedCorners);
  e->setConf("oscTakesEntireWindow",settings.oscTakesEntireWindow);
  e->setConf("oscBorder",settings.oscBorder);
  e->setConf("oscEscapesBoundary",settings.oscEscapesBoundary);
  e->setConf("separateFMColors",settings.separateFMColors);
  e->setConf("insEditColorize",settings.insEditColorize);
  e->setConf("metroVol",settings.metroVol);
  e->setConf("pushNibble",settings.pushNibble);
  e->setConf("scrollChangesOrder",settings.scrollChangesOrder);
  e->setConf("oplStandardWaveNames",settings.oplStandardWaveNames);
  e->setConf("cursorMoveNoScroll",settings.cursorMoveNoScroll);
  e->setConf("lowLatency",settings.lowLatency);
  e->setConf("notePreviewBehavior",settings.notePreviewBehavior);
  e->setConf("powerSave",settings.powerSave);
  e->setConf("absorbInsInput",settings.absorbInsInput);
  e->setConf("eventDelay",settings.eventDelay);
  e->setConf("moveWindowTitle",settings.moveWindowTitle);
  e->setConf("hiddenSystems",settings.hiddenSystems);
  e->setConf("initialSys2",settings.initialSys.toBase64());
  e->setConf("initialSysName",settings.initialSysName);
  e->setConf("horizontalDataView",settings.horizontalDataView);
  e->setConf("noMultiSystem",settings.noMultiSystem);
  e->setConf("oldMacroVSlider",settings.oldMacroVSlider);
  e->setConf("displayAllInsTypes",settings.displayAllInsTypes);
  e->setConf("noteCellSpacing",settings.noteCellSpacing);
  e->setConf("insCellSpacing",settings.insCellSpacing);
  e->setConf("volCellSpacing",settings.volCellSpacing);
  e->setConf("effectCellSpacing",settings.effectCellSpacing);
  e->setConf("effectValCellSpacing",settings.effectValCellSpacing);
  e->setConf("doubleClickColumn",settings.doubleClickColumn);
  e->setConf("blankIns",settings.blankIns);
  e->setConf("dragMovesSelection",settings.dragMovesSelection);
  e->setConf("unsignedDetune",settings.unsignedDetune);
  e->setConf("noThreadedInput",settings.noThreadedInput);
  e->setConf("saveWindowPos",settings.saveWindowPos);
  e->setConf("clampSamples",settings.clampSamples);
  e->setConf("noteOffLabel",settings.noteOffLabel);
  e->setConf("noteRelLabel",settings.noteRelLabel);
  e->setConf("macroRelLabel",settings.macroRelLabel);
  e->setConf("emptyLabel",settings.emptyLabel);
  e->setConf("emptyLabel2",settings.emptyLabel2);
  e->setConf("saveUnusedPatterns",settings.saveUnusedPatterns);
  e->setConf("channelColors",settings.channelColors);
  e->setConf("channelTextColors",settings.channelTextColors);
  e->setConf("channelStyle",settings.channelStyle);
  e->setConf("channelVolStyle",settings.channelVolStyle);
  e->setConf("channelFeedbackStyle",settings.channelFeedbackStyle);
  e->setConf("channelFont",settings.channelFont);
  e->setConf("channelTextCenter",settings.channelTextCenter);
  e->setConf("maxRecentFile",settings.maxRecentFile);
  e->setConf("midiOutClock",settings.midiOutClock);
  e->setConf("midiOutMode",settings.midiOutMode);
  e->setConf("centerPattern",settings.centerPattern);
  e->setConf("ordersCursor",settings.ordersCursor);
  e->setConf("persistFadeOut",settings.persistFadeOut);
  e->setConf("exportLoops",settings.exportLoops);
  e->setConf("exportFadeOut",settings.exportFadeOut);
  e->setConf("macroLayout",settings.macroLayout);

  // colors
  for (int i=0; i<GUI_COLOR_MAX; i++) {
    e->setConf(guiColors[i].name,(int)ImGui::ColorConvertFloat4ToU32(uiColors[i]));
  }

  // keybinds
  for (int i=0; i<GUI_ACTION_MAX; i++) {
    if (guiActions[i].defaultBind==-1) continue; // not a bind
    e->setConf(String("keybind_GUI_ACTION_")+String(guiActions[i].name),actionKeys[i]);
  }

  parseKeybinds();

  e->setConf("noteKeys",encodeKeyMap(noteKeys));

  midiMap.compile();
  midiMap.write(e->getConfigPath()+DIR_SEPARATOR_STR+"midiIn_"+stripName(settings.midiInDevice)+".cfg");

  e->saveConf();

  while (!recentFile.empty() && (int)recentFile.size()>settings.maxRecentFile) {
    recentFile.pop_back();
  }

  if (sampleROMsChanged) {
    if (e->loadSampleROMs()) {
      showError(e->getLastError());
    }
  }

  if (!e->switchMaster(coresChanged)) {
    showError("could not initialize audio!");
  }

  ImGui::GetIO().Fonts->Clear();

  applyUISettings();

  ImGui_ImplSDLRenderer_DestroyFontsTexture();
  if (!ImGui::GetIO().Fonts->Build()) {
    logE("error while building font atlas!");
    showError("error while loading fonts! please check your settings.");
    ImGui::GetIO().Fonts->Clear();
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    patFont=mainFont;
    ImGui_ImplSDLRenderer_DestroyFontsTexture();
    if (!ImGui::GetIO().Fonts->Build()) {
      logE("error again while building font atlas!");
    }
  }
}

bool FurnaceGUI::importColors(String path) {
  FILE* f=ps_fopen(path.c_str(),"rb");
  if (f==NULL) {
    logW("error while opening color file for import: %s",strerror(errno));
    return false;
  }
  resetColors();
  char line[4096];
  while (!feof(f)) {
    String key="";
    String value="";
    bool keyOrValue=false;
    if (fgets(line,4095,f)==NULL) {
      break;
    }
    for (char* i=line; *i; i++) {
      if (*i=='\n') continue;
      if (keyOrValue) {
        value+=*i;
      } else {
        if (*i=='=') {
          keyOrValue=true;
        } else {
          key+=*i;
        }
      }
    }
    if (keyOrValue) {
      // unoptimal
      const char* cs=key.c_str();
      bool found=false;
      for (int i=0; i<GUI_COLOR_MAX; i++) {
        try {
          if (strcmp(cs,guiColors[i].name)==0) {
            uiColors[i]=ImGui::ColorConvertU32ToFloat4(std::stoi(value));
            found=true;
            break;
          }
        } catch (std::out_of_range& e) {
          break;
        } catch (std::invalid_argument& e) {
          break;
        }
      }
      if (!found) logW("line invalid: %s",line);
    }
  }
  fclose(f);
  applyUISettings(false);
  return true;
}

bool FurnaceGUI::exportColors(String path) {
  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while opening color file for export: %s",strerror(errno));
    return false;
  }
  for (int i=0; i<GUI_COLOR_MAX; i++) {
    if (fprintf(f,"%s=%d\n",guiColors[i].name,ImGui::ColorConvertFloat4ToU32(uiColors[i]))<0) {
      logW("error while exporting colors: %s",strerror(errno));
      break;
    }
  }
  fclose(f);
  return true;
}

bool FurnaceGUI::importKeybinds(String path) {
  FILE* f=ps_fopen(path.c_str(),"rb");
  if (f==NULL) {
    logW("error while opening keybind file for import: %s",strerror(errno));
    return false;
  }
  resetKeybinds();
  char line[4096];
  while (!feof(f)) {
    String key="";
    String value="";
    bool keyOrValue=false;
    if (fgets(line,4095,f)==NULL) {
      break;
    }
    for (char* i=line; *i; i++) {
      if (*i=='\n') continue;
      if (keyOrValue) {
        value+=*i;
      } else {
        if (*i=='=') {
          keyOrValue=true;
        } else {
          key+=*i;
        }
      }
    }
    if (keyOrValue) {
      // unoptimal
      const char* cs=key.c_str();
      bool found=false;
      for (int i=0; i<GUI_ACTION_MAX; i++) {
        try {
          if (strcmp(cs,guiActions[i].name)==0) {
            actionKeys[i]=std::stoi(value);
            found=true;
            break;
          }
        } catch (std::out_of_range& e) {
          break;
        } catch (std::invalid_argument& e) {
          break;
        }
      }
      if (!found) logW("line invalid: %s",line);
    }
  }
  fclose(f);
  return true;
}

bool FurnaceGUI::exportKeybinds(String path) {
  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while opening keybind file for export: %s",strerror(errno));
    return false;
  }
  for (int i=0; i<GUI_ACTION_MAX; i++) {
    if (guiActions[i].defaultBind==-1) continue;
    if (fprintf(f,"%s=%d\n",guiActions[i].name,actionKeys[i])<0) {
      logW("error while exporting keybinds: %s",strerror(errno));
      break;
    }
  }
  fclose(f);
  return true;
}

bool FurnaceGUI::importLayout(String path) {
  if (mobileUI) {
    logW("but you are on the mobile UI!");
    return false;
  }
  FILE* f=ps_fopen(path.c_str(),"rb");
  if (f==NULL) {
    logW("error while opening keybind file for import: %s",strerror(errno));
    return false;
  }
  if (fseek(f,0,SEEK_END)<0) {
    fclose(f);
    return false;
  }
  ssize_t len=ftell(f);
  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    return false;
  }
  if (len<1) {
    if (len==0) {
      logE("that file is empty!");
      lastError="file is empty";
    } else {
      perror("tell error");
    }
    fclose(f);
    return false;
  }
  if (fseek(f,0,SEEK_SET)<0) {
    perror("size error");
    lastError=fmt::sprintf("on get size: %s",strerror(errno));
    fclose(f);
    return false;
  }
  unsigned char* file=new unsigned char[len];
  if (fread(file,1,(size_t)len,f)!=(size_t)len) {
    perror("read error");
    lastError=fmt::sprintf("on read: %s",strerror(errno));
    fclose(f);
    delete[] file;
    return false;
  }
  fclose(f);

  ImGui::LoadIniSettingsFromMemory((const char*)file,len);
  delete[] file;
  return true;
}

bool FurnaceGUI::exportLayout(String path) {
  if (mobileUI) {
    logW("but you are on the mobile UI!");
    return false;
  }
  FILE* f=ps_fopen(path.c_str(),"wb");
  if (f==NULL) {
    logW("error while opening layout file for export: %s",strerror(errno));
    return false;
  }
  size_t dataSize=0;
  const char* data=ImGui::SaveIniSettingsToMemory(&dataSize);
  if (fwrite(data,1,dataSize,f)!=dataSize) {
    logW("error while exporting layout: %s",strerror(errno));
  }
  fclose(f);
  return true;
}

void FurnaceGUI::resetColors() {
  for (int i=0; i<GUI_COLOR_MAX; i++) {
    uiColors[i]=ImGui::ColorConvertU32ToFloat4(guiColors[i].defaultColor);
  }
}

void FurnaceGUI::resetKeybinds() {
  for (int i=0; i<GUI_COLOR_MAX; i++) {
    if (guiActions[i].defaultBind==-1) continue;
    actionKeys[i]=guiActions[i].defaultBind;
  }
  parseKeybinds();
}

void FurnaceGUI::parseKeybinds() {
  actionMapGlobal.clear();
  actionMapPat.clear();
  actionMapInsList.clear();
  actionMapWaveList.clear();
  actionMapSampleList.clear();
  actionMapSample.clear();
  actionMapOrders.clear();

  for (int i=GUI_ACTION_GLOBAL_MIN+1; i<GUI_ACTION_GLOBAL_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapGlobal[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_PAT_MIN+1; i<GUI_ACTION_PAT_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapPat[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_INS_LIST_MIN+1; i<GUI_ACTION_INS_LIST_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapInsList[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_WAVE_LIST_MIN+1; i<GUI_ACTION_WAVE_LIST_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapWaveList[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_SAMPLE_LIST_MIN+1; i<GUI_ACTION_SAMPLE_LIST_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapSampleList[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_SAMPLE_MIN+1; i<GUI_ACTION_SAMPLE_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapSample[actionKeys[i]]=i;
    }
  }

  for (int i=GUI_ACTION_ORDERS_MIN+1; i<GUI_ACTION_ORDERS_MAX; i++) {
    if (actionKeys[i]&FURK_MASK) {
      actionMapOrders[actionKeys[i]]=i;
    }
  }
}

void FurnaceGUI::pushAccentColors(const ImVec4& one, const ImVec4& two, const ImVec4& border, const ImVec4& borderShadow) {
  float hue, sat, val;

  ImVec4 primaryActive=one;
  ImVec4 primaryHover, primary;
  primaryHover.w=primaryActive.w;
  primary.w=primaryActive.w;
  ImGui::ColorConvertRGBtoHSV(primaryActive.x,primaryActive.y,primaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    primary=primaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,primaryActive.x,primaryActive.y,primaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.8,val*0.35,primary.x,primary.y,primary.z);
  }

  ImVec4 secondaryActive=two;
  ImVec4 secondaryHover, secondary, secondarySemiActive;
  secondarySemiActive.w=secondaryActive.w;
  secondaryHover.w=secondaryActive.w;
  secondary.w=secondaryActive.w;
  ImGui::ColorConvertRGBtoHSV(secondaryActive.x,secondaryActive.y,secondaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    secondary=secondaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.7,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,secondaryActive.x,secondaryActive.y,secondaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.75,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.25,secondary.x,secondary.y,secondary.z);
  }

  ImGui::PushStyleColor(ImGuiCol_Button,primary);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,primaryHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_Tab,primary);
  ImGui::PushStyleColor(ImGuiCol_TabHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_TabActive,secondarySemiActive);
  ImGui::PushStyleColor(ImGuiCol_TabUnfocused,primary);
  ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive,primaryHover);
  ImGui::PushStyleColor(ImGuiCol_Header,secondary);
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_HeaderActive,secondaryActive);
  ImGui::PushStyleColor(ImGuiCol_ResizeGrip,secondary);
  ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_ResizeGripActive,secondaryActive);
  ImGui::PushStyleColor(ImGuiCol_FrameBg,secondary);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive,secondaryActive);
  ImGui::PushStyleColor(ImGuiCol_SliderGrab,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_SliderGrabActive,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive,primary);
  ImGui::PushStyleColor(ImGuiCol_CheckMark,primaryActive);
  ImGui::PushStyleColor(ImGuiCol_TextSelectedBg,secondaryHover);
  ImGui::PushStyleColor(ImGuiCol_Border,border);
  ImGui::PushStyleColor(ImGuiCol_BorderShadow,borderShadow);
}

void FurnaceGUI::popAccentColors() {
  ImGui::PopStyleColor(24);
}

#define IGFD_FileStyleByExtension IGFD_FileStyleByExtention

#ifdef _WIN32
#define SYSTEM_FONT_PATH_1 "C:\\Windows\\Fonts\\segoeui.ttf"
#define SYSTEM_FONT_PATH_2 "C:\\Windows\\Fonts\\tahoma.ttf"
// TODO!
#define SYSTEM_FONT_PATH_3 "C:\\Windows\\Fonts\\tahoma.ttf"
// TODO!
#define SYSTEM_PAT_FONT_PATH_1 "C:\\Windows\\Fonts\\consola.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "C:\\Windows\\Fonts\\cour.ttf"
// GOOD LUCK WITH THIS ONE - UNTESTED
#define SYSTEM_PAT_FONT_PATH_3 "C:\\Windows\\Fonts\\vgasys.fon"
#elif defined(__APPLE__)
#define SYSTEM_FONT_PATH_1 "/System/Library/Fonts/SFAANS.ttf"
#define SYSTEM_FONT_PATH_2 "/System/Library/Fonts/Helvetica.ttc"
#define SYSTEM_FONT_PATH_3 "/System/Library/Fonts/Helvetica.dfont"
#define SYSTEM_PAT_FONT_PATH_1 "/System/Library/Fonts/SFNSMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/System/Library/Fonts/Courier New.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/System/Library/Fonts/Courier New.ttf"
#elif defined(ANDROID)
#define SYSTEM_FONT_PATH_1 "/system/fonts/Roboto-Regular.ttf"
#define SYSTEM_FONT_PATH_2 "/system/fonts/DroidSans.ttf"
#define SYSTEM_FONT_PATH_3 "/system/fonts/DroidSans.ttf"
// ???
#define SYSTEM_PAT_FONT_PATH_1 "/system/fonts/RobotoMono-Regular.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/system/fonts/DroidSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/system/fonts/CutiveMono.ttf"
#else
#define SYSTEM_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_3 "/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
#define SYSTEM_PAT_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/usr/share/fonts/ubuntu/UbuntuMono-R.ttf"
#endif

void setupLabel(const char* lStr, char* label, int len) {
  memset(label,0,32);
  for (int i=0, p=0; i<len; i++) {
    signed char cl;
    if (lStr[p]==0) {
      strncat(label," ",32);
    } else {
      decodeUTF8((const unsigned char*)&lStr[p],cl);
      memcpy(label+p,lStr+p,cl);
      p+=cl;
    }
  }
}

void FurnaceGUI::applyUISettings(bool updateFonts) {
  ImGuiStyle sty;
  if (settings.guiColorsBase) {
    ImGui::StyleColorsLight(&sty);
  } else {
    ImGui::StyleColorsDark(&sty);
  }

  setupLabel(settings.noteOffLabel.c_str(),noteOffLabel,3);
  setupLabel(settings.noteRelLabel.c_str(),noteRelLabel,3);
  setupLabel(settings.macroRelLabel.c_str(),macroRelLabel,3);
  setupLabel(settings.emptyLabel.c_str(),emptyLabel,3);
  setupLabel(settings.emptyLabel2.c_str(),emptyLabel2,2);

  if (settings.dpiScale>=0.5f) dpiScale=settings.dpiScale;

  // colors
  if (updateFonts) {
    for (int i=0; i<GUI_COLOR_MAX; i++) {
      uiColors[i]=ImGui::ColorConvertU32ToFloat4(e->getConfInt(guiColors[i].name,guiColors[i].defaultColor));
    }
  }

  for (int i=0; i<64; i++) {
    ImVec4 col1=uiColors[GUI_COLOR_PATTERN_VOLUME_MIN];
    ImVec4 col2=uiColors[GUI_COLOR_PATTERN_VOLUME_HALF];
    ImVec4 col3=uiColors[GUI_COLOR_PATTERN_VOLUME_MAX];
    volColors[i]=ImVec4(col1.x+((col2.x-col1.x)*float(i)/64.0f),
                        col1.y+((col2.y-col1.y)*float(i)/64.0f),
                        col1.z+((col2.z-col1.z)*float(i)/64.0f),
                        1.0f);
    volColors[i+64]=ImVec4(col2.x+((col3.x-col2.x)*float(i)/64.0f),
                           col2.y+((col3.y-col2.y)*float(i)/64.0f),
                           col2.z+((col3.z-col2.z)*float(i)/64.0f),
                           1.0f);
  }

  float hue, sat, val;

  ImVec4 primaryActive=uiColors[GUI_COLOR_ACCENT_PRIMARY];
  ImVec4 primaryHover, primary;
  primaryHover.w=primaryActive.w;
  primary.w=primaryActive.w;
  ImGui::ColorConvertRGBtoHSV(primaryActive.x,primaryActive.y,primaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    primary=primaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,primaryActive.x,primaryActive.y,primaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,primaryHover.x,primaryHover.y,primaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.8,val*0.35,primary.x,primary.y,primary.z);
  }

  ImVec4 secondaryActive=uiColors[GUI_COLOR_ACCENT_SECONDARY];
  ImVec4 secondaryHoverActual, secondaryHover, secondary, secondarySemiActive;
  secondarySemiActive.w=secondaryActive.w;
  secondaryHover.w=secondaryActive.w;
  secondary.w=secondaryActive.w;
  ImGui::ColorConvertRGBtoHSV(secondaryActive.x,secondaryActive.y,secondaryActive.z,hue,sat,val);
  if (settings.guiColorsBase) {
    secondary=secondaryActive;
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.7,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.9,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat,val*0.5,secondaryActive.x,secondaryActive.y,secondaryActive.z);
  } else {
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.75,secondarySemiActive.x,secondarySemiActive.y,secondarySemiActive.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.5,secondaryHover.x,secondaryHover.y,secondaryHover.z);
    ImGui::ColorConvertHSVtoRGB(hue,sat*0.9,val*0.25,secondary.x,secondary.y,secondary.z);
  }

  secondaryHoverActual=secondaryHover;

  if (mobileUI) { // disable all hovered colors
    primaryHover=primary;
    secondaryHover=secondary;
  }

  sty.Colors[ImGuiCol_WindowBg]=uiColors[GUI_COLOR_FRAME_BACKGROUND];
  sty.Colors[ImGuiCol_ModalWindowDimBg]=uiColors[GUI_COLOR_MODAL_BACKDROP];
  sty.Colors[ImGuiCol_Text]=uiColors[GUI_COLOR_TEXT];

  sty.Colors[ImGuiCol_Button]=primary;
  sty.Colors[ImGuiCol_ButtonHovered]=primaryHover;
  sty.Colors[ImGuiCol_ButtonActive]=primaryActive;
  sty.Colors[ImGuiCol_Tab]=primary;
  sty.Colors[ImGuiCol_TabHovered]=secondaryHover;
  sty.Colors[ImGuiCol_TabActive]=secondarySemiActive;
  sty.Colors[ImGuiCol_TabUnfocused]=primary;
  sty.Colors[ImGuiCol_TabUnfocusedActive]=primaryHover;
  sty.Colors[ImGuiCol_Header]=secondary;
  sty.Colors[ImGuiCol_HeaderHovered]=secondaryHover;
  sty.Colors[ImGuiCol_HeaderActive]=secondaryActive;
  sty.Colors[ImGuiCol_ResizeGrip]=secondary;
  sty.Colors[ImGuiCol_ResizeGripHovered]=secondaryHover;
  sty.Colors[ImGuiCol_ResizeGripActive]=secondaryActive;
  sty.Colors[ImGuiCol_FrameBg]=secondary;
  sty.Colors[ImGuiCol_FrameBgHovered]=secondaryHover;
  sty.Colors[ImGuiCol_FrameBgActive]=secondaryActive;
  sty.Colors[ImGuiCol_SliderGrab]=primaryActive;
  sty.Colors[ImGuiCol_SliderGrabActive]=primaryActive;
  sty.Colors[ImGuiCol_TitleBgActive]=primary;
  sty.Colors[ImGuiCol_CheckMark]=primaryActive;
  sty.Colors[ImGuiCol_TextSelectedBg]=secondaryHoverActual;
  sty.Colors[ImGuiCol_PlotHistogram]=uiColors[GUI_COLOR_MACRO_OTHER];
  sty.Colors[ImGuiCol_PlotHistogramHovered]=uiColors[GUI_COLOR_MACRO_OTHER];
  sty.Colors[ImGuiCol_Border]=uiColors[GUI_COLOR_BORDER];
  sty.Colors[ImGuiCol_BorderShadow]=uiColors[GUI_COLOR_BORDER_SHADOW];

  if (settings.roundedWindows) sty.WindowRounding=8.0f;
  if (settings.roundedButtons) {
    sty.FrameRounding=6.0f;
    sty.GrabRounding=6.0f;
  }
  if (settings.roundedMenus) sty.PopupRounding=8.0f;

  if (settings.frameBorders) {
    sty.FrameBorderSize=1.0f;
  } else {
    sty.FrameBorderSize=0.0f;
  }

  if (settings.guiColorsShading>0) {
    sty.FrameShading=(float)settings.guiColorsShading/100.0f;
  }

  if (mobileUI) {
    sty.FramePadding=ImVec2(8.0f,6.0f);
  }

  sty.ScaleAllSizes(dpiScale);

  ImGui::GetStyle()=sty;

  ImGui::GetIO().ConfigInputTrickleEventQueue=settings.eventDelay;
  ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly=settings.moveWindowTitle;
  ImGui::GetIO().ConfigInertialScrollToleranceSqr=pow(dpiScale*4.0f,2.0f);

  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_PITCH];
    pitchGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_ACTIVE];
    noteGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_PANNING];
    panGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_INS];
    insGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=volColors[i/2];
    volGrad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY];
    sysCmd1Grad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }
  for (int i=0; i<256; i++) {
    ImVec4& base=uiColors[GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY];
    sysCmd2Grad[i]=ImGui::GetColorU32(ImVec4(base.x,base.y,base.z,((float)i/255.0f)*base.w));
  }

  if (updateFonts) {
    // set to 800 for now due to problems with unifont
    static const ImWchar upTo800[]={0x20,0x7e,0xa0,0x800,0};
    ImFontGlyphRangesBuilder range;
    ImVector<ImWchar> outRange;

    range.AddRanges(upTo800);
    if (settings.loadJapanese) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
    }
    if (settings.loadChinese) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
    }
    if (settings.loadChineseTraditional) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
    }
    if (settings.loadKorean) {
      range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesKorean());
    }
    // I'm terribly sorry
    range.UsedChars[0x80>>5]=0;

    range.BuildRanges(&outRange);
    if (fontRange!=NULL) delete[] fontRange;
    fontRange=new ImWchar[outRange.size()];
    int index=0;
    for (ImWchar& i: outRange) {
      fontRange[index++]=i;
    }

    if (settings.mainFont<0 || settings.mainFont>6) settings.mainFont=0;
    if (settings.patFont<0 || settings.patFont>6) settings.patFont=0;

    if (settings.mainFont==6 && settings.mainFontPath.empty()) {
      logW("UI font path is empty! reverting to default font");
      settings.mainFont=0;
    }
    if (settings.patFont==6 && settings.patFontPath.empty()) {
      logW("pattern font path is empty! reverting to default font");
      settings.patFont=0;
    }

    ImFontConfig fc1;
    fc1.MergeMode=true;

    if (settings.mainFont==6) { // custom font
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.mainFontPath.c_str(),e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
        logW("could not load UI font! reverting to default font");
        settings.mainFont=0;
        if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
          logE("could not load UI font! falling back to Proggy Clean.");
          mainFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    } else if (settings.mainFont==5) { // system font
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_1,e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
        if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_2,e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
          if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_3,e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
            logW("could not load UI font! reverting to default font");
            settings.mainFont=0;
            if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
              logE("could not load UI font! falling back to Proggy Clean.");
              mainFont=ImGui::GetIO().Fonts->AddFontDefault();
            }
          }
        }
      }
    } else {
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
        logE("could not load UI font! falling back to Proggy Clean.");
        mainFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
    }

    // two fallback fonts
    mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_liberationSans_compressed_data,font_liberationSans_compressed_size,e->getConfInt("mainFontSize",18)*dpiScale,&fc1,fontRange);
    mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_unifont_compressed_data,font_unifont_compressed_size,e->getConfInt("mainFontSize",18)*dpiScale,&fc1,fontRange);

    ImFontConfig fc;
    fc.MergeMode=true;
    fc.GlyphMinAdvanceX=e->getConfInt("iconSize",16)*dpiScale;
    static const ImWchar fontRangeIcon[]={ICON_MIN_FA,ICON_MAX_FA,0};
    if ((iconFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(iconFont_compressed_data,iconFont_compressed_size,e->getConfInt("iconSize",16)*dpiScale,&fc,fontRangeIcon))==NULL) {
      logE("could not load icon font!");
    }

    if (settings.mainFontSize==settings.patFontSize && settings.patFont<5 && builtinFontM[settings.patFont]==builtinFont[settings.mainFont]) {
      logD("using main font for pat font.");
      patFont=mainFont;
    } else {
      if (settings.patFont==6) { // custom font
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.patFontPath.c_str(),e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
          logW("could not load pattern font! reverting to default font");
          settings.patFont=0;
          if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
            logE("could not load pattern font! falling back to Proggy Clean.");
            patFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      } else if (settings.patFont==5) { // system font
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_1,e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
          if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_2,e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
            if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_3,e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
              logW("could not load pattern font! reverting to default font");
              settings.patFont=0;
              if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
                logE("could not load pattern font! falling back to Proggy Clean.");
                patFont=ImGui::GetIO().Fonts->AddFontDefault();
              }
            }
          }
        }
      } else {
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
          logE("could not load pattern font!");
          patFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    }

    // 0x39B = Λ
    static const ImWchar bigFontRange[]={0x20,0xFF,0x39b,0x39b,0};
    if ((bigFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_plexSans_compressed_data,font_plexSans_compressed_size,40*dpiScale,NULL,bigFontRange))==NULL) {
      logE("could not load big UI font!");
    }

    mainFont->FallbackChar='?';
    mainFont->DotChar='.';
  }

  // TODO: allow changing these colors.
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir,"",uiColors[GUI_COLOR_FILE_DIR],ICON_FA_FOLDER_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile,"",uiColors[GUI_COLOR_FILE_OTHER],ICON_FA_FILE_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fur",uiColors[GUI_COLOR_FILE_SONG_NATIVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fui",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fuw",uiColors[GUI_COLOR_FILE_WAVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmf",uiColors[GUI_COLOR_FILE_SONG_NATIVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmp",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmw",uiColors[GUI_COLOR_FILE_WAVE],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".wav",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmc",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".brr",uiColors[GUI_COLOR_FILE_AUDIO],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgm",uiColors[GUI_COLOR_FILE_VGM],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".zsm",uiColors[GUI_COLOR_FILE_ZSM],ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".otf",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttc",uiColors[GUI_COLOR_FILE_FONT],ICON_FA_FONT);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".mod",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ftm",uiColors[GUI_COLOR_FILE_SONG_IMPORT],ICON_FA_FILE);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".tfi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".s3i",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".sbi",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".opli",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".opni",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".y12",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".bnk",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fti",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".bti",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ff",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".opm",uiColors[GUI_COLOR_FILE_INSTR],ICON_FA_FILE);

  if (updateFonts) {
    if (fileDialog!=NULL) delete fileDialog;
    fileDialog=new FurnaceGUIFileDialog(settings.sysFileDialog);

    fileDialog->mobileUI=mobileUI;
  }
}
