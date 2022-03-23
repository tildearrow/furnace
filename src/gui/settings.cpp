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
#include "util.h"
#include "ImGuiFileDialog.h"
#include "IconsFontAwesome4.h"
#include "misc/cpp/imgui_stdlib.h"
#include <SDL_keycode.h>
#include <SDL_scancode.h>
#include <fmt/printf.h>
#include <imgui.h>

#ifdef __APPLE__
#define FURKMOD_CMD FURKMOD_META
#else
#define FURKMOD_CMD FURKMOD_CTRL
#endif

#define DEFAULT_NOTE_KEYS "5:7;6:4;7:3;8:16;10:6;11:8;12:24;13:10;16:11;17:9;18:26;19:28;20:12;21:17;22:1;23:19;24:23;25:5;26:14;27:2;28:21;29:0;30:100;31:13;32:15;34:18;35:20;36:22;38:25;39:27;43:100;46:101;47:29;48:31;53:102;"

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

const char* saaCores[]={
  "MAME",
  "SAASound"
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
  ImGui::ColorEdit4(label "##CC_" #what,(float*)&uiColors[what]);

#define KEYBIND_CONFIG_BEGIN(id) \
  if (ImGui::BeginTable(id,2)) {

#define KEYBIND_CONFIG_END \
    ImGui::EndTable(); \
  }

#define UI_KEYBIND_CONFIG(what,label) \
  ImGui::TableNextRow(); \
  ImGui::TableNextColumn(); \
  ImGui::Text(label); \
  ImGui::TableNextColumn(); \
  if (ImGui::Button(fmt::sprintf("%s##KC_" #what,(bindSetPending && bindSetTarget==what)?"Press key...":getKeyName(actionKeys[what])).c_str())) { \
    promptKey(what); \
  } \
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) actionKeys[what]=0;

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
  if (ImGui::Begin("Settings",NULL,ImGuiWindowFlags_NoDocking)) {
    if (ImGui::BeginTabBar("settingsTab")) {
      if (ImGui::BeginTabItem("General")) {
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

        bool pullDeleteBehaviorB=settings.pullDeleteBehavior;
        if (ImGui::Checkbox("Move cursor up on backspace-delete",&pullDeleteBehaviorB)) {
          settings.pullDeleteBehavior=pullDeleteBehaviorB;
        }

        bool stepOnDeleteB=settings.stepOnDelete;
        if (ImGui::Checkbox("Move cursor by edit step on delete",&stepOnDeleteB)) {
          settings.stepOnDelete=stepOnDeleteB;
        }

        bool stepOnInsertB=settings.stepOnInsert;
        if (ImGui::Checkbox("Move cursor by edit step on insert (push)",&stepOnInsertB)) {
          settings.stepOnInsert=stepOnInsertB;
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
        if (ImGui::Checkbox("Restart song when changing system properties",&restartOnFlagChangeB)) {
          settings.restartOnFlagChange=restartOnFlagChangeB;
        }

        bool sysFileDialogB=settings.sysFileDialog;
        if (ImGui::Checkbox("Use system file picker",&sysFileDialogB)) {
          settings.sysFileDialog=sysFileDialogB;
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

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Audio")) {
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

        bool forceMonoB=settings.forceMono;
        if (ImGui::Checkbox("Force mono audio",&forceMonoB)) {
          settings.forceMono=forceMonoB;
        }

        TAAudioDesc& audioWant=e->getAudioDescWant();
        TAAudioDesc& audioGot=e->getAudioDescGot();

        ImGui::Text("want: %d samples @ %.0fHz\n",audioWant.bufsize,audioWant.rate);
        ImGui::Text("got: %d samples @ %.0fHz\n",audioGot.bufsize,audioGot.rate);

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Emulation")) {
        ImGui::Text("Arcade/YM2151 core");
        ImGui::SameLine();
        ImGui::Combo("##ArcadeCore",&settings.arcadeCore,arcadeCores,2);

        ImGui::Text("Genesis/YM2612 core");
        ImGui::SameLine();
        ImGui::Combo("##YM2612Core",&settings.ym2612Core,ym2612Cores,2);

        ImGui::Text("SAA1099 core");
        ImGui::SameLine();
        ImGui::Combo("##SAACore",&settings.saaCore,saaCores,2);

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Appearance")) {
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

        bool macroViewB=settings.macroView;
        if (ImGui::Checkbox("Classic macro view (standard macros only; deprecated!)",&macroViewB)) {
          settings.macroView=macroViewB;
        }

        bool unifiedDataViewB=settings.unifiedDataView;
        if (ImGui::Checkbox("Unified instrument/wavetable/sample list",&unifiedDataViewB)) {
          settings.unifiedDataView=unifiedDataViewB;
        }

        bool chipNamesB=settings.chipNames;
        if (ImGui::Checkbox("Use chip names instead of system names",&chipNamesB)) {
          settings.chipNames=chipNamesB;
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
        
        // sorry. temporarily disabled until ImGui has a way to add separators in tables arbitrarily.
        /*bool sysSeparatorsB=settings.sysSeparators;
        if (ImGui::Checkbox("Add separators between systems in Orders",&sysSeparatorsB)) {
          settings.sysSeparators=sysSeparatorsB;
        }*/

        bool partyTimeB=settings.partyTime;
        if (ImGui::Checkbox("About screen party time",&partyTimeB)) {
          settings.partyTime=partyTimeB;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("Warning: may cause epileptic seizures.");
        }

        ImGui::Separator();

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

        ImGui::Separator();

        if (ImGui::TreeNode("Color scheme")) {
          if (ImGui::TreeNode("General")) {
            ImGui::Text("Color scheme type:");
            if (ImGui::RadioButton("Dark##gcb0",settings.guiColorsBase==0)) {
              settings.guiColorsBase=0;
            }
            if (ImGui::RadioButton("Light##gcb1",settings.guiColorsBase==1)) {
              settings.guiColorsBase=1;
            }
            UI_COLOR_CONFIG(GUI_COLOR_BACKGROUND,"Background");
            UI_COLOR_CONFIG(GUI_COLOR_FRAME_BACKGROUND,"Window background");
            UI_COLOR_CONFIG(GUI_COLOR_MODAL_BACKDROP,"Modal backdrop");
            UI_COLOR_CONFIG(GUI_COLOR_HEADER,"Header");
            UI_COLOR_CONFIG(GUI_COLOR_TEXT,"Text");
            UI_COLOR_CONFIG(GUI_COLOR_ACCENT_PRIMARY,"Primary");
            UI_COLOR_CONFIG(GUI_COLOR_ACCENT_SECONDARY,"Secondary");
            UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_ON,"Toggle on");
            UI_COLOR_CONFIG(GUI_COLOR_TOGGLE_OFF,"Toggle off");
            UI_COLOR_CONFIG(GUI_COLOR_EDITING,"Editing");
            UI_COLOR_CONFIG(GUI_COLOR_SONG_LOOP,"Song loop");
            UI_COLOR_CONFIG(GUI_COLOR_PLAYBACK_STAT,"Playback status");
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Volume Meter")) {
            UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_LOW,"Low");
            UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_HIGH,"High");
            UI_COLOR_CONFIG(GUI_COLOR_VOLMETER_PEAK,"Clip");
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
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_FM,"FM (4-operator)");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_STD,"Standard");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_GB,"Game Boy");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_C64,"C64");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_AMIGA,"Amiga/Sample");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_PCE,"PC Engine");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY,"AY-3-8910/SSG");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_AY8930,"AY8930");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_TIA,"TIA");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_SAA1099,"SAA1099");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_VIC,"VIC");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_PET,"PET");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_VRC6,"VRC6");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPLL,"FM (OPLL)");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPL,"FM (OPL)");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_FDS,"FDS");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_VBOY,"Virtual Boy");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_N163,"Namco 163");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_SCC,"Konami SCC");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_OPZ,"FM (OPZ)");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_POKEY,"POKEY");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_BEEPER,"PC Beeper");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_SWAN,"WonderSwan");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_MIKEY,"Lynx");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_VERA,"VERA");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_X1_010,"X1-010");
            UI_COLOR_CONFIG(GUI_COLOR_INSTR_UNKNOWN,"Other/Unknown");
            ImGui::TreePop();
          }
          if (ImGui::TreeNode("Channel")) {
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
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR,"Cursor");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_HOVER,"Cursor (hovered)");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_CURSOR_ACTIVE,"Cursor (clicked)");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION,"Selection");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_HOVER,"Selection (hovered)");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_SELECTION_ACTIVE,"Selection (clicked)");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_1,"Highlight 1");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_HI_2,"Highlight 2");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ROW_INDEX,"Row number");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_ACTIVE,"Note");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INACTIVE,"Blank");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_INS,"Instrument");
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
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,"Primary system effect");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,"Secondary system effect");
            UI_COLOR_CONFIG(GUI_COLOR_PATTERN_EFFECT_MISC,"Miscellaneous");
            UI_COLOR_CONFIG(GUI_COLOR_EE_VALUE,"External command output");
            ImGui::TreePop();
          }
          ImGui::TreePop();
        }

        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Keyboard")) {
        if (ImGui::TreeNode("Global hotkeys")) {
          KEYBIND_CONFIG_BEGIN("keysGlobal");

          UI_KEYBIND_CONFIG(GUI_ACTION_OPEN,"Open file");
          UI_KEYBIND_CONFIG(GUI_ACTION_OPEN_BACKUP,"Restore backup");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAVE,"Save file");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAVE_AS,"Save as");
          UI_KEYBIND_CONFIG(GUI_ACTION_UNDO,"Undo");
          UI_KEYBIND_CONFIG(GUI_ACTION_REDO,"Redo");
          UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_TOGGLE,"Play/Stop (toggle)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PLAY,"Play");
          UI_KEYBIND_CONFIG(GUI_ACTION_STOP,"Stop");
          UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_REPEAT,"Play (repeat pattern)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PLAY_CURSOR,"Play from cursor");
          UI_KEYBIND_CONFIG(GUI_ACTION_STEP_ONE,"Step row");
          UI_KEYBIND_CONFIG(GUI_ACTION_OCTAVE_UP,"Octave up");
          UI_KEYBIND_CONFIG(GUI_ACTION_OCTAVE_DOWN,"Octave down");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_UP,"Previous instrument");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_DOWN,"Next instrument");
          UI_KEYBIND_CONFIG(GUI_ACTION_STEP_UP,"Increase edit step");
          UI_KEYBIND_CONFIG(GUI_ACTION_STEP_DOWN,"Decrease edit step");
          UI_KEYBIND_CONFIG(GUI_ACTION_TOGGLE_EDIT,"Toggle edit mode");
          UI_KEYBIND_CONFIG(GUI_ACTION_METRONOME,"Metronome");
          UI_KEYBIND_CONFIG(GUI_ACTION_REPEAT_PATTERN,"Toggle repeat pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_FOLLOW_ORDERS,"Follow orders");
          UI_KEYBIND_CONFIG(GUI_ACTION_FOLLOW_PATTERN,"Follow pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_PANIC,"Panic");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Window activation")) {
          KEYBIND_CONFIG_BEGIN("keysWindow");

          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_EDIT_CONTROLS,"Edit Controls");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_ORDERS,"Orders");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_INS_LIST,"Instrument List");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_INS_EDIT,"Instrument Editor");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SONG_INFO,"Song Information");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_PATTERN,"Pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_WAVE_LIST,"Wavetable List");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_WAVE_EDIT,"Wavetable Editor");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SAMPLE_LIST,"Sample List");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SAMPLE_EDIT,"Sample Editor");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_ABOUT,"About");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_SETTINGS,"Settings");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_MIXER,"Mixer");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_DEBUG,"Debug Menu");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_OSCILLOSCOPE,"Oscilloscope");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_VOL_METER,"Volume Meter");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_STATS,"Statistics");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_COMPAT_FLAGS,"Compatibility Flags");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_PIANO,"Piano");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_NOTES,"Song Comments");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_CHANNELS,"Channels");
          UI_KEYBIND_CONFIG(GUI_ACTION_WINDOW_REGISTER_VIEW,"Register View");

          UI_KEYBIND_CONFIG(GUI_ACTION_COLLAPSE_WINDOW,"Collapse/expand current window");
          UI_KEYBIND_CONFIG(GUI_ACTION_CLOSE_WINDOW,"Close current window");

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
                snprintf(id,4095,"Envelope release##SNType_%d",i.scan);
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

          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_NOTE_UP,"Transpose (semitone up)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_NOTE_DOWN,"Transpose (semitone down");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_OCTAVE_UP,"Transpose (octave up)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_OCTAVE_DOWN,"Transpose (octave down)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECT_ALL,"Select all");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CUT,"Cut");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_COPY,"Copy");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PASTE,"Paste");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_UP,"Move cursor up");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_DOWN,"Move cursor down");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_LEFT,"Move cursor left");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_RIGHT,"Move cursor right");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_UP_ONE,"Move cursor up by one (override Edit Step)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_DOWN_ONE,"Move cursor down by one (override Edit Step)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL,"Move cursor to previous channel");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL,"Move cursor to next channel");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL,"Move cursor to previous channel (overflow)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL,"Move cursor to next channel (overflow)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_BEGIN,"Move cursor to beginning of pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_END,"Move cursor to end of pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_UP_COARSE,"Move cursor up (coarse)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_CURSOR_DOWN_COARSE,"Move cursor down (coarse)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_UP,"Expand selection upwards");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_DOWN,"Expand selection downwards");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_LEFT,"Expand selection to the left");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_RIGHT,"Expand selection to the right");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_UP_ONE,"Expand selection upwards by one (override Edit Step)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_DOWN_ONE,"Expand selection downwards by one (override Edit Step)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_BEGIN,"Expand selection to beginning of pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_END,"Expand selection to end of pattern");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_UP_COARSE,"Expand selection upwards (coarse)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SELECTION_DOWN_COARSE,"Expand selection downwards (coarse)");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_DELETE,"Delete");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PULL_DELETE,"Pull delete");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_INSERT,"Insert");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_MUTE_CURSOR,"Mute channel at cursor");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_SOLO_CURSOR,"Solo channel at cursor");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_UNMUTE_ALL,"Unmute all channels");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_NEXT_ORDER,"Go to next order");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_PREV_ORDER,"Go to previous order");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_COLLAPSE,"Collapse channel at cursor");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_INCREASE_COLUMNS,"Increase effect columns");
          UI_KEYBIND_CONFIG(GUI_ACTION_PAT_DECREASE_COLUMNS,"Decrease effect columns");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Instrument list")) {
          KEYBIND_CONFIG_BEGIN("keysInsList");

          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_ADD,"Add");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_DUPLICATE,"Duplicate");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_OPEN,"Open");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_SAVE,"Save");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_MOVE_UP,"Move up");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_MOVE_DOWN,"Move down");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_DELETE,"Delete");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_EDIT,"Edit");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_UP,"Cursor up");
          UI_KEYBIND_CONFIG(GUI_ACTION_INS_LIST_DOWN,"Cursor down");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Wavetable list")) {
          KEYBIND_CONFIG_BEGIN("keysWaveList");

          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_ADD,"Add");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_DUPLICATE,"Duplicate");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_OPEN,"Open");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_SAVE,"Save");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_MOVE_UP,"Move up");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_MOVE_DOWN,"Move down");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_DELETE,"Delete");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_EDIT,"Edit");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_UP,"Cursor up");
          UI_KEYBIND_CONFIG(GUI_ACTION_WAVE_LIST_DOWN,"Cursor down");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Sample list")) {
          KEYBIND_CONFIG_BEGIN("keysSampleList");

          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_ADD,"Add");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_DUPLICATE,"Duplicate");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_OPEN,"Open");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_SAVE,"Save");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_MOVE_UP,"Move up");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN,"Move down");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_DELETE,"Delete");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_EDIT,"Edit");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_UP,"Cursor up");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_DOWN,"Cursor down");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_PREVIEW,"Preview");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW,"Stop preview");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Orders")) {
          KEYBIND_CONFIG_BEGIN("keysOrders");

          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_UP,"Previous order");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DOWN,"Next order");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_LEFT,"Cursor left");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_RIGHT,"Cursor right");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_INCREASE,"Increase value");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DECREASE,"Decrease value");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_EDIT_MODE,"Switch edit mode");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_LINK,"Toggle alter entire row");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_ADD,"Add");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DUPLICATE,"Duplicate");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DEEP_CLONE,"Deep clone");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DUPLICATE_END,"Duplicate to end of song");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_DEEP_CLONE_END,"Deep clone to end of song");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_REMOVE,"Remove");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_MOVE_UP,"Move up");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_MOVE_DOWN,"Move down");
          UI_KEYBIND_CONFIG(GUI_ACTION_ORDERS_REPLAY,"Replay");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        if (ImGui::TreeNode("Sample editor")) {
          KEYBIND_CONFIG_BEGIN("keysSampleEdit");

          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SELECT,"Edit mode: Select");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_DRAW,"Edit mode: Draw");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_CUT,"Cut");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_COPY,"Copy");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PASTE,"Paste");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PASTE_REPLACE,"Paste replace");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PASTE_MIX,"Paste mix");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SELECT_ALL,"Select all");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_RESIZE,"Resize");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_RESAMPLE,"Resample");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_AMPLIFY,"Amplify");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_NORMALIZE,"Normalize");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_FADE_IN,"Fade in");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_FADE_OUT,"Fade out");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_INSERT,"Insert silence");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SILENCE,"Apply silence");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_DELETE,"Delete");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_TRIM,"Trim");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_REVERSE,"Reverse");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_INVERT,"Invert");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_SIGN,"Signed/unsigned exchange");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_FILTER,"Apply filter");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_PREVIEW,"Preview sample");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_STOP_PREVIEW,"Stop sample preview");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_ZOOM_IN,"Zoom in");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_ZOOM_OUT,"Zoom out");
          UI_KEYBIND_CONFIG(GUI_ACTION_SAMPLE_ZOOM_AUTO,"Toggle auto-zoom");

          KEYBIND_CONFIG_END;
          ImGui::TreePop();
        }
        ImGui::EndTabItem();
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

#define LOAD_KEYBIND(x,y) \
  actionKeys[x]=e->getConfInt("keybind_" #x,y);

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
  settings.audioQuality=e->getConfInt("audioQuality",0);
  settings.audioBufSize=e->getConfInt("audioBufSize",1024);
  settings.audioRate=e->getConfInt("audioRate",44100);
  settings.arcadeCore=e->getConfInt("arcadeCore",0);
  settings.ym2612Core=e->getConfInt("ym2612Core",0);
  settings.saaCore=e->getConfInt("saaCore",1);
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
  settings.allowEditDocking=e->getConfInt("allowEditDocking",0);
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
  settings.avoidRaisingPattern=e->getConfInt("avoidRaisingPattern",0);
  settings.insFocusesPattern=e->getConfInt("insFocusesPattern",1);
  settings.stepOnInsert=e->getConfInt("stepOnInsert",0);
  settings.unifiedDataView=e->getConfInt("unifiedDataView",0);
  settings.sysFileDialog=e->getConfInt("sysFileDialog",1);
  settings.roundedWindows=e->getConfInt("roundedWindows",1);
  settings.roundedButtons=e->getConfInt("roundedButtons",1);
  settings.roundedMenus=e->getConfInt("roundedMenus",0);
  settings.loadJapanese=e->getConfInt("loadJapanese",0);

  clampSetting(settings.mainFontSize,2,96);
  clampSetting(settings.patFontSize,2,96);
  clampSetting(settings.iconSize,2,48);
  clampSetting(settings.audioEngine,0,1);
  clampSetting(settings.audioQuality,0,1);
  clampSetting(settings.audioBufSize,32,4096);
  clampSetting(settings.audioRate,8000,384000);
  clampSetting(settings.arcadeCore,0,1);
  clampSetting(settings.ym2612Core,0,1);
  clampSetting(settings.saaCore,0,1);
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
  clampSetting(settings.avoidRaisingPattern,0,1);
  clampSetting(settings.insFocusesPattern,0,1);
  clampSetting(settings.stepOnInsert,0,1);
  clampSetting(settings.unifiedDataView,0,1);
  clampSetting(settings.sysFileDialog,0,1);
  clampSetting(settings.roundedWindows,0,1);
  clampSetting(settings.roundedButtons,0,1);
  clampSetting(settings.roundedMenus,0,1);
  clampSetting(settings.loadJapanese,0,1);

  // keybinds
  LOAD_KEYBIND(GUI_ACTION_OPEN,FURKMOD_CMD|SDLK_o);
  LOAD_KEYBIND(GUI_ACTION_OPEN_BACKUP,0);
  LOAD_KEYBIND(GUI_ACTION_SAVE,FURKMOD_CMD|SDLK_s);
  LOAD_KEYBIND(GUI_ACTION_SAVE_AS,FURKMOD_CMD|FURKMOD_SHIFT|SDLK_s);
  LOAD_KEYBIND(GUI_ACTION_UNDO,FURKMOD_CMD|SDLK_z);
  LOAD_KEYBIND(GUI_ACTION_REDO,FURKMOD_CMD|SDLK_y);
  LOAD_KEYBIND(GUI_ACTION_PLAY_TOGGLE,SDLK_RETURN);
  LOAD_KEYBIND(GUI_ACTION_PLAY,0);
  LOAD_KEYBIND(GUI_ACTION_STOP,0);
  LOAD_KEYBIND(GUI_ACTION_PLAY_REPEAT,0);
  LOAD_KEYBIND(GUI_ACTION_PLAY_CURSOR,FURKMOD_SHIFT|SDLK_RETURN);
  LOAD_KEYBIND(GUI_ACTION_STEP_ONE,FURKMOD_CMD|SDLK_RETURN);
  LOAD_KEYBIND(GUI_ACTION_OCTAVE_UP,SDLK_KP_MULTIPLY);
  LOAD_KEYBIND(GUI_ACTION_OCTAVE_DOWN,SDLK_KP_DIVIDE);
  LOAD_KEYBIND(GUI_ACTION_INS_UP,FURKMOD_SHIFT|SDLK_KP_DIVIDE);
  LOAD_KEYBIND(GUI_ACTION_INS_DOWN,FURKMOD_SHIFT|SDLK_KP_MULTIPLY);
  LOAD_KEYBIND(GUI_ACTION_STEP_UP,FURKMOD_CMD|SDLK_KP_MULTIPLY);
  LOAD_KEYBIND(GUI_ACTION_STEP_DOWN,FURKMOD_CMD|SDLK_KP_DIVIDE);
  LOAD_KEYBIND(GUI_ACTION_TOGGLE_EDIT,SDLK_SPACE);
  LOAD_KEYBIND(GUI_ACTION_METRONOME,FURKMOD_CMD|SDLK_m);
  LOAD_KEYBIND(GUI_ACTION_REPEAT_PATTERN,0);
  LOAD_KEYBIND(GUI_ACTION_FOLLOW_ORDERS,0);
  LOAD_KEYBIND(GUI_ACTION_FOLLOW_PATTERN,0);
  LOAD_KEYBIND(GUI_ACTION_PANIC,SDLK_F12);

  LOAD_KEYBIND(GUI_ACTION_WINDOW_EDIT_CONTROLS,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_ORDERS,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_INS_LIST,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_INS_EDIT,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_SONG_INFO,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_PATTERN,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_WAVE_LIST,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_WAVE_EDIT,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_SAMPLE_LIST,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_SAMPLE_EDIT,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_ABOUT,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_SETTINGS,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_MIXER,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_DEBUG,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_OSCILLOSCOPE,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_VOL_METER,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_STATS,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_COMPAT_FLAGS,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_PIANO,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_NOTES,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_CHANNELS,0);
  LOAD_KEYBIND(GUI_ACTION_WINDOW_REGISTER_VIEW,0);

  LOAD_KEYBIND(GUI_ACTION_COLLAPSE_WINDOW,0);
  LOAD_KEYBIND(GUI_ACTION_CLOSE_WINDOW,FURKMOD_SHIFT|SDLK_ESCAPE);

  LOAD_KEYBIND(GUI_ACTION_PAT_NOTE_UP,FURKMOD_CMD|SDLK_F2);
  LOAD_KEYBIND(GUI_ACTION_PAT_NOTE_DOWN,FURKMOD_CMD|SDLK_F1);
  LOAD_KEYBIND(GUI_ACTION_PAT_OCTAVE_UP,FURKMOD_CMD|SDLK_F4);
  LOAD_KEYBIND(GUI_ACTION_PAT_OCTAVE_DOWN,FURKMOD_CMD|SDLK_F3);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECT_ALL,FURKMOD_CMD|SDLK_a);
  LOAD_KEYBIND(GUI_ACTION_PAT_CUT,FURKMOD_CMD|SDLK_x);
  LOAD_KEYBIND(GUI_ACTION_PAT_COPY,FURKMOD_CMD|SDLK_c);
  LOAD_KEYBIND(GUI_ACTION_PAT_PASTE,FURKMOD_CMD|SDLK_v);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_UP,SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN,SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT,SDLK_LEFT);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT,SDLK_RIGHT);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_UP_ONE,FURKMOD_SHIFT|SDLK_HOME);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_ONE,FURKMOD_SHIFT|SDLK_END);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_BEGIN,SDLK_HOME);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_END,SDLK_END);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_UP_COARSE,SDLK_PAGEUP);
  LOAD_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_COARSE,SDLK_PAGEDOWN);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_UP,FURKMOD_SHIFT|SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN,FURKMOD_SHIFT|SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_LEFT,FURKMOD_SHIFT|SDLK_LEFT);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_RIGHT,FURKMOD_SHIFT|SDLK_RIGHT);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_UP_ONE,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_ONE,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_BEGIN,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_END,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_UP_COARSE,FURKMOD_SHIFT|SDLK_PAGEUP);
  LOAD_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_COARSE,FURKMOD_SHIFT|SDLK_PAGEDOWN);
  LOAD_KEYBIND(GUI_ACTION_PAT_DELETE,SDLK_DELETE);
  LOAD_KEYBIND(GUI_ACTION_PAT_PULL_DELETE,SDLK_BACKSPACE);
  LOAD_KEYBIND(GUI_ACTION_PAT_INSERT,SDLK_INSERT);
  LOAD_KEYBIND(GUI_ACTION_PAT_MUTE_CURSOR,FURKMOD_ALT|SDLK_F9);
  LOAD_KEYBIND(GUI_ACTION_PAT_SOLO_CURSOR,FURKMOD_ALT|SDLK_F10);
  LOAD_KEYBIND(GUI_ACTION_PAT_UNMUTE_ALL,FURKMOD_ALT|FURKMOD_SHIFT|SDLK_F9);
  LOAD_KEYBIND(GUI_ACTION_PAT_NEXT_ORDER,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_PREV_ORDER,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_COLLAPSE,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_INCREASE_COLUMNS,0);
  LOAD_KEYBIND(GUI_ACTION_PAT_DECREASE_COLUMNS,0);

  LOAD_KEYBIND(GUI_ACTION_INS_LIST_ADD,SDLK_INSERT);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_DUPLICATE,FURKMOD_CMD|SDLK_d);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_OPEN,0);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_SAVE,0);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_MOVE_UP,FURKMOD_SHIFT|SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_MOVE_DOWN,FURKMOD_SHIFT|SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_DELETE,0);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_EDIT,FURKMOD_SHIFT|SDLK_RETURN);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_UP,SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_INS_LIST_DOWN,SDLK_DOWN);

  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_ADD,SDLK_INSERT);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_DUPLICATE,FURKMOD_CMD|SDLK_d);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_OPEN,0);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_SAVE,0);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_UP,FURKMOD_SHIFT|SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_DOWN,FURKMOD_SHIFT|SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_DELETE,0);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_EDIT,FURKMOD_SHIFT|SDLK_RETURN);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_UP,SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_WAVE_LIST_DOWN,SDLK_DOWN);

  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_ADD,SDLK_INSERT);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_DUPLICATE,FURKMOD_CMD|SDLK_d);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN,0);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_SAVE,0);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_UP,FURKMOD_SHIFT|SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN,FURKMOD_SHIFT|SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_DELETE,0);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_EDIT,FURKMOD_SHIFT|SDLK_RETURN);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_UP,SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_DOWN,SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_PREVIEW,0);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW,0);

  LOAD_KEYBIND(GUI_ACTION_SAMPLE_SELECT,FURKMOD_SHIFT|SDLK_i);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_DRAW,FURKMOD_SHIFT|SDLK_d);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_CUT,FURKMOD_CMD|SDLK_x);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_COPY,FURKMOD_CMD|SDLK_c);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_PASTE,FURKMOD_CMD|SDLK_v);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_PASTE_REPLACE,FURKMOD_CMD|FURKMOD_SHIFT|SDLK_v);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_PASTE_MIX,FURKMOD_CMD|FURKMOD_ALT|SDLK_v);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_SELECT_ALL,FURKMOD_CMD|SDLK_a);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_RESIZE,FURKMOD_CMD|SDLK_r);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_RESAMPLE,FURKMOD_CMD|SDLK_e);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_AMPLIFY,FURKMOD_CMD|SDLK_b);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_NORMALIZE,FURKMOD_CMD|SDLK_n);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_FADE_IN,FURKMOD_CMD|SDLK_i);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_FADE_OUT,FURKMOD_CMD|SDLK_o);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_INSERT,SDLK_INSERT);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_SILENCE,FURKMOD_SHIFT|SDLK_DELETE);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_DELETE,SDLK_DELETE);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_TRIM,FURKMOD_CMD|SDLK_DELETE);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_REVERSE,FURKMOD_CMD|SDLK_t);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_INVERT,FURKMOD_CMD|FURKMOD_SHIFT|SDLK_t);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_SIGN,FURKMOD_CMD|SDLK_u);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_FILTER,FURKMOD_CMD|SDLK_f);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_PREVIEW,0);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_STOP_PREVIEW,0);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_IN,FURKMOD_CMD|SDLK_EQUALS);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_OUT,FURKMOD_CMD|SDLK_MINUS);
  LOAD_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_AUTO,FURKMOD_CMD|SDLK_0);

  LOAD_KEYBIND(GUI_ACTION_ORDERS_UP,SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_DOWN,SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_LEFT,SDLK_LEFT);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_RIGHT,SDLK_RIGHT);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_INCREASE,0);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_DECREASE,0);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_EDIT_MODE,0);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_LINK,FURKMOD_CMD|SDLK_l);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_ADD,SDLK_INSERT);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_DUPLICATE,FURKMOD_CMD|SDLK_d);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE,FURKMOD_CMD|FURKMOD_SHIFT|SDLK_d);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_DUPLICATE_END,FURKMOD_CMD|SDLK_e);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE_END,FURKMOD_CMD|FURKMOD_SHIFT|SDLK_e);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_REMOVE,SDLK_DELETE);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_MOVE_UP,FURKMOD_SHIFT|SDLK_UP);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_MOVE_DOWN,FURKMOD_SHIFT|SDLK_DOWN);
  LOAD_KEYBIND(GUI_ACTION_ORDERS_REPLAY,0);

  decodeKeyMap(noteKeys,e->getConfString("noteKeys",DEFAULT_NOTE_KEYS));

  parseKeybinds();
}

#define PUT_UI_COLOR(source) e->setConf(#source,(int)ImGui::GetColorU32(uiColors[source]));
#define SAVE_KEYBIND(x) e->setConf("keybind_" #x,actionKeys[x]);

void FurnaceGUI::commitSettings() {
  e->setConf("mainFontSize",settings.mainFontSize);
  e->setConf("patFontSize",settings.patFontSize);
  e->setConf("iconSize",settings.iconSize);
  e->setConf("audioEngine",String(audioBackends[settings.audioEngine]));
  e->setConf("audioDevice",settings.audioDevice);
  e->setConf("audioQuality",settings.audioQuality);
  e->setConf("audioBufSize",settings.audioBufSize);
  e->setConf("audioRate",settings.audioRate);
  e->setConf("arcadeCore",settings.arcadeCore);
  e->setConf("ym2612Core",settings.ym2612Core);
  e->setConf("saaCore",settings.saaCore);
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
  e->setConf("avoidRaisingPattern",settings.avoidRaisingPattern);
  e->setConf("insFocusesPattern",settings.insFocusesPattern);
  e->setConf("stepOnInsert",settings.stepOnInsert);
  e->setConf("unifiedDataView",settings.unifiedDataView);
  e->setConf("sysFileDialog",settings.sysFileDialog);
  e->setConf("roundedWindows",settings.roundedWindows);
  e->setConf("roundedButtons",settings.roundedButtons);
  e->setConf("roundedMenus",settings.roundedMenus);
  e->setConf("loadJapanese",settings.loadJapanese);

  PUT_UI_COLOR(GUI_COLOR_BACKGROUND);
  PUT_UI_COLOR(GUI_COLOR_FRAME_BACKGROUND);
  PUT_UI_COLOR(GUI_COLOR_MODAL_BACKDROP);
  PUT_UI_COLOR(GUI_COLOR_HEADER);
  PUT_UI_COLOR(GUI_COLOR_TEXT);
  PUT_UI_COLOR(GUI_COLOR_ACCENT_PRIMARY);
  PUT_UI_COLOR(GUI_COLOR_ACCENT_SECONDARY);
  PUT_UI_COLOR(GUI_COLOR_TOGGLE_ON);
  PUT_UI_COLOR(GUI_COLOR_TOGGLE_OFF);
  PUT_UI_COLOR(GUI_COLOR_EDITING);
  PUT_UI_COLOR(GUI_COLOR_SONG_LOOP);
  PUT_UI_COLOR(GUI_COLOR_VOLMETER_LOW);
  PUT_UI_COLOR(GUI_COLOR_VOLMETER_HIGH);
  PUT_UI_COLOR(GUI_COLOR_VOLMETER_PEAK);
  PUT_UI_COLOR(GUI_COLOR_MACRO_VOLUME);
  PUT_UI_COLOR(GUI_COLOR_MACRO_PITCH);
  PUT_UI_COLOR(GUI_COLOR_MACRO_OTHER);
  PUT_UI_COLOR(GUI_COLOR_MACRO_WAVE);
  PUT_UI_COLOR(GUI_COLOR_INSTR_FM);
  PUT_UI_COLOR(GUI_COLOR_INSTR_STD);
  PUT_UI_COLOR(GUI_COLOR_INSTR_GB);
  PUT_UI_COLOR(GUI_COLOR_INSTR_C64);
  PUT_UI_COLOR(GUI_COLOR_INSTR_AMIGA);
  PUT_UI_COLOR(GUI_COLOR_INSTR_PCE);
  PUT_UI_COLOR(GUI_COLOR_INSTR_AY);
  PUT_UI_COLOR(GUI_COLOR_INSTR_AY8930);
  PUT_UI_COLOR(GUI_COLOR_INSTR_TIA);
  PUT_UI_COLOR(GUI_COLOR_INSTR_SAA1099);
  PUT_UI_COLOR(GUI_COLOR_INSTR_VIC);
  PUT_UI_COLOR(GUI_COLOR_INSTR_PET);
  PUT_UI_COLOR(GUI_COLOR_INSTR_VRC6);
  PUT_UI_COLOR(GUI_COLOR_INSTR_OPLL);
  PUT_UI_COLOR(GUI_COLOR_INSTR_OPL);
  PUT_UI_COLOR(GUI_COLOR_INSTR_FDS);
  PUT_UI_COLOR(GUI_COLOR_INSTR_VBOY);
  PUT_UI_COLOR(GUI_COLOR_INSTR_N163);
  PUT_UI_COLOR(GUI_COLOR_INSTR_SCC);
  PUT_UI_COLOR(GUI_COLOR_INSTR_OPZ);
  PUT_UI_COLOR(GUI_COLOR_INSTR_POKEY);
  PUT_UI_COLOR(GUI_COLOR_INSTR_BEEPER);
  PUT_UI_COLOR(GUI_COLOR_INSTR_SWAN);
  PUT_UI_COLOR(GUI_COLOR_INSTR_MIKEY);
  PUT_UI_COLOR(GUI_COLOR_INSTR_VERA);
  PUT_UI_COLOR(GUI_COLOR_INSTR_X1_010);
  PUT_UI_COLOR(GUI_COLOR_INSTR_UNKNOWN);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_FM);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_PULSE);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_NOISE);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_PCM);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_WAVE);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_OP);
  PUT_UI_COLOR(GUI_COLOR_CHANNEL_MUTED);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_CURSOR);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_CURSOR_HOVER);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_CURSOR_ACTIVE);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_SELECTION);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_SELECTION_HOVER);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_SELECTION_ACTIVE);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_HI_1);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_HI_2);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_ROW_INDEX);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_ACTIVE);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_INACTIVE);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_INS);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_MIN);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_HALF);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_MAX);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_INVALID);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_PITCH);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_VOLUME);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_PANNING);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SONG);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_TIME);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SPEED);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY);
  PUT_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_MISC);
  PUT_UI_COLOR(GUI_COLOR_EE_VALUE);
  PUT_UI_COLOR(GUI_COLOR_PLAYBACK_STAT);

  SAVE_KEYBIND(GUI_ACTION_OPEN);
  SAVE_KEYBIND(GUI_ACTION_OPEN_BACKUP);
  SAVE_KEYBIND(GUI_ACTION_SAVE);
  SAVE_KEYBIND(GUI_ACTION_SAVE_AS);
  SAVE_KEYBIND(GUI_ACTION_UNDO);
  SAVE_KEYBIND(GUI_ACTION_REDO);
  SAVE_KEYBIND(GUI_ACTION_PLAY_TOGGLE);
  SAVE_KEYBIND(GUI_ACTION_PLAY);
  SAVE_KEYBIND(GUI_ACTION_STOP);
  SAVE_KEYBIND(GUI_ACTION_PLAY_REPEAT);
  SAVE_KEYBIND(GUI_ACTION_PLAY_CURSOR);
  SAVE_KEYBIND(GUI_ACTION_STEP_ONE);
  SAVE_KEYBIND(GUI_ACTION_OCTAVE_UP);
  SAVE_KEYBIND(GUI_ACTION_OCTAVE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_INS_UP);
  SAVE_KEYBIND(GUI_ACTION_INS_DOWN);
  SAVE_KEYBIND(GUI_ACTION_STEP_UP);
  SAVE_KEYBIND(GUI_ACTION_STEP_DOWN);
  SAVE_KEYBIND(GUI_ACTION_TOGGLE_EDIT);
  SAVE_KEYBIND(GUI_ACTION_METRONOME);
  SAVE_KEYBIND(GUI_ACTION_REPEAT_PATTERN);
  SAVE_KEYBIND(GUI_ACTION_FOLLOW_ORDERS);
  SAVE_KEYBIND(GUI_ACTION_FOLLOW_PATTERN);
  SAVE_KEYBIND(GUI_ACTION_PANIC);

  SAVE_KEYBIND(GUI_ACTION_WINDOW_EDIT_CONTROLS);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_ORDERS);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_INS_LIST);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_INS_EDIT);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_SONG_INFO);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_PATTERN);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_WAVE_LIST);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_WAVE_EDIT);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_SAMPLE_LIST);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_SAMPLE_EDIT);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_ABOUT);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_SETTINGS);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_MIXER);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_DEBUG);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_OSCILLOSCOPE);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_VOL_METER);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_STATS);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_COMPAT_FLAGS);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_PIANO);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_NOTES);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_CHANNELS);
  SAVE_KEYBIND(GUI_ACTION_WINDOW_REGISTER_VIEW);

  SAVE_KEYBIND(GUI_ACTION_COLLAPSE_WINDOW);
  SAVE_KEYBIND(GUI_ACTION_CLOSE_WINDOW);

  SAVE_KEYBIND(GUI_ACTION_PAT_NOTE_UP);
  SAVE_KEYBIND(GUI_ACTION_PAT_NOTE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_PAT_OCTAVE_UP);
  SAVE_KEYBIND(GUI_ACTION_PAT_OCTAVE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECT_ALL);
  SAVE_KEYBIND(GUI_ACTION_PAT_CUT);
  SAVE_KEYBIND(GUI_ACTION_PAT_COPY);
  SAVE_KEYBIND(GUI_ACTION_PAT_PASTE);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_UP);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_UP_ONE);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_ONE);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_LEFT_CHANNEL);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_RIGHT_CHANNEL);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_NEXT_CHANNEL);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_PREVIOUS_CHANNEL);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_BEGIN);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_END);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_UP_COARSE);
  SAVE_KEYBIND(GUI_ACTION_PAT_CURSOR_DOWN_COARSE);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_UP);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_LEFT);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_RIGHT);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_UP_ONE);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_ONE);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_BEGIN);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_END);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_UP_COARSE);
  SAVE_KEYBIND(GUI_ACTION_PAT_SELECTION_DOWN_COARSE);
  SAVE_KEYBIND(GUI_ACTION_PAT_DELETE);
  SAVE_KEYBIND(GUI_ACTION_PAT_PULL_DELETE);
  SAVE_KEYBIND(GUI_ACTION_PAT_INSERT);
  SAVE_KEYBIND(GUI_ACTION_PAT_MUTE_CURSOR);
  SAVE_KEYBIND(GUI_ACTION_PAT_SOLO_CURSOR);
  SAVE_KEYBIND(GUI_ACTION_PAT_UNMUTE_ALL);
  SAVE_KEYBIND(GUI_ACTION_PAT_NEXT_ORDER);
  SAVE_KEYBIND(GUI_ACTION_PAT_PREV_ORDER);
  SAVE_KEYBIND(GUI_ACTION_PAT_COLLAPSE);
  SAVE_KEYBIND(GUI_ACTION_PAT_INCREASE_COLUMNS);
  SAVE_KEYBIND(GUI_ACTION_PAT_DECREASE_COLUMNS);

  SAVE_KEYBIND(GUI_ACTION_INS_LIST_ADD);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_DUPLICATE);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_OPEN);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_SAVE);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_MOVE_UP);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_MOVE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_DELETE);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_EDIT);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_UP);
  SAVE_KEYBIND(GUI_ACTION_INS_LIST_DOWN);

  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_ADD);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_DUPLICATE);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_OPEN);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_SAVE);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_UP);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_DELETE);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_EDIT);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_UP);
  SAVE_KEYBIND(GUI_ACTION_WAVE_LIST_DOWN);

  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_ADD);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_OPEN);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_SAVE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_DELETE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_EDIT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_UP);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_DOWN);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_PREVIEW);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);

  SAVE_KEYBIND(GUI_ACTION_SAMPLE_SELECT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_DRAW);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_CUT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_COPY);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_PASTE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_PASTE_REPLACE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_PASTE_MIX);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_SELECT_ALL);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_RESIZE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_RESAMPLE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_AMPLIFY);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_NORMALIZE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_FADE_IN);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_FADE_OUT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_INSERT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_SILENCE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_DELETE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_TRIM);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_REVERSE);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_INVERT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_SIGN);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_FILTER);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_PREVIEW);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_STOP_PREVIEW);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_IN);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_OUT);
  SAVE_KEYBIND(GUI_ACTION_SAMPLE_ZOOM_AUTO);

  SAVE_KEYBIND(GUI_ACTION_ORDERS_UP);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_DOWN);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_LEFT);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_RIGHT);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_INCREASE);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_DECREASE);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_EDIT_MODE);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_LINK);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_ADD);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_DUPLICATE);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_DUPLICATE_END);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_DEEP_CLONE_END);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_REMOVE);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_MOVE_UP);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_MOVE_DOWN);
  SAVE_KEYBIND(GUI_ACTION_ORDERS_REPLAY);

  e->setConf("noteKeys",encodeKeyMap(noteKeys));

  e->saveConf();

  if (!e->switchMaster()) {
    showError("could not initialize audio!");
  }

  ImGui::GetIO().Fonts->Clear();

  applyUISettings();

  ImGui_ImplSDLRenderer_DestroyFontsTexture();
  if (!ImGui::GetIO().Fonts->Build()) {
    logE("error while building font atlas!\n");
    showError("error while loading fonts! please check your settings.");
    ImGui::GetIO().Fonts->Clear();
    mainFont=ImGui::GetIO().Fonts->AddFontDefault();
    patFont=mainFont;
    ImGui_ImplSDLRenderer_DestroyFontsTexture();
    if (!ImGui::GetIO().Fonts->Build()) {
      logE("error again while building font atlas!\n");
    }
  }
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

#define IGFD_FileStyleByExtension IGFD_FileStyleByExtention

#define GET_UI_COLOR(target,def) \
  uiColors[target]=ImGui::ColorConvertU32ToFloat4(e->getConfInt(#target,ImGui::GetColorU32(def)));

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
#else
#define SYSTEM_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSans.ttf"
#define SYSTEM_FONT_PATH_3 "/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
#define SYSTEM_PAT_FONT_PATH_1 "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_2 "/usr/share/fonts/TTF/DejaVuSansMono.ttf"
#define SYSTEM_PAT_FONT_PATH_3 "/usr/share/fonts/ubuntu/UbuntuMono-R.ttf"
#endif

void FurnaceGUI::applyUISettings() {
  ImGuiStyle sty;
  if (settings.guiColorsBase) {
    ImGui::StyleColorsLight(&sty);
  } else {
    ImGui::StyleColorsDark(&sty);
  }

  if (settings.dpiScale>=0.5f) dpiScale=settings.dpiScale;

  GET_UI_COLOR(GUI_COLOR_BACKGROUND,ImVec4(0.1f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_FRAME_BACKGROUND,ImVec4(0.0f,0.0f,0.0f,0.85f));
  GET_UI_COLOR(GUI_COLOR_MODAL_BACKDROP,ImVec4(0.0f,0.0f,0.0f,0.55f));
  GET_UI_COLOR(GUI_COLOR_HEADER,ImVec4(0.2f,0.2f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_TEXT,ImVec4(1.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_ACCENT_PRIMARY,ImVec4(0.06f,0.53f,0.98f,1.0f));
  GET_UI_COLOR(GUI_COLOR_ACCENT_SECONDARY,ImVec4(0.26f,0.59f,0.98f,1.0f));
  GET_UI_COLOR(GUI_COLOR_TOGGLE_ON,ImVec4(0.2f,0.6f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_TOGGLE_OFF,ImVec4(0.2f,0.2f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_EDITING,ImVec4(0.2f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_SONG_LOOP,ImVec4(0.3f,0.5f,0.8f,0.4f));
  GET_UI_COLOR(GUI_COLOR_VOLMETER_LOW,ImVec4(0.2f,0.6f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_VOLMETER_HIGH,ImVec4(1.0f,0.9f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_VOLMETER_PEAK,ImVec4(1.0f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_VOLUME,ImVec4(0.2f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_PITCH,ImVec4(1.0f,0.8f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_OTHER,ImVec4(0.0f,0.9f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_MACRO_WAVE,ImVec4(1.0f,0.4f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_FM,ImVec4(0.6f,0.9f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_STD,ImVec4(0.6f,1.0f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_GB,ImVec4(1.0f,1.0f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_C64,ImVec4(0.85f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_AMIGA,ImVec4(1.0f,0.5f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_PCE,ImVec4(1.0f,0.8f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_AY,ImVec4(1.0f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_AY8930,ImVec4(0.7f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_TIA,ImVec4(1.0f,0.6f,0.4f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_SAA1099,ImVec4(0.3f,0.3f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VIC,ImVec4(0.2f,1.0f,0.6f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_PET,ImVec4(1.0f,1.0f,0.8f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VRC6,ImVec4(1.0f,0.9f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_OPLL,ImVec4(0.6f,0.7f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_OPL,ImVec4(0.3f,1.0f,0.9f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_FDS,ImVec4(0.8f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VBOY,ImVec4(1.0f,0.1f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_N163,ImVec4(1.0f,0.4f,0.1f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_SCC,ImVec4(0.7f,1.0f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_OPZ,ImVec4(0.2f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_POKEY,ImVec4(0.5f,1.0f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_BEEPER,ImVec4(0.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_SWAN,ImVec4(0.3f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_MIKEY,ImVec4(0.5f,1.0f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_VERA,ImVec4(0.4f,0.6f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_X1_010,ImVec4(0.3f,0.5f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_INSTR_UNKNOWN,ImVec4(0.3f,0.3f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_FM,ImVec4(0.2f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_PULSE,ImVec4(0.4f,1.0f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_NOISE,ImVec4(0.8f,0.8f,0.8f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_PCM,ImVec4(1.0f,0.9f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_WAVE,ImVec4(1.0f,0.5f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_OP,ImVec4(0.2f,0.4f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_CHANNEL_MUTED,ImVec4(0.5f,0.5f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_CURSOR,ImVec4(0.1f,0.3f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_CURSOR_HOVER,ImVec4(0.2f,0.4f,0.6f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_CURSOR_ACTIVE,ImVec4(0.2f,0.5f,0.7f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_SELECTION,ImVec4(0.15f,0.15f,0.2f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_SELECTION_HOVER,ImVec4(0.2f,0.2f,0.3f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_SELECTION_ACTIVE,ImVec4(0.4f,0.4f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_HI_1,ImVec4(0.6f,0.6f,0.6f,0.2f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_HI_2,ImVec4(0.5f,0.8f,1.0f,0.2f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_ROW_INDEX,ImVec4(0.5f,0.8f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_ACTIVE,ImVec4(1.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_INACTIVE,ImVec4(0.5f,0.5f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_INS,ImVec4(0.4f,0.7f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_MIN,ImVec4(0.0f,0.5f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_HALF,ImVec4(0.0f,0.75f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_VOLUME_MAX,ImVec4(0.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_INVALID,ImVec4(1.0f,0.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_PITCH,ImVec4(1.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_VOLUME,ImVec4(0.0f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_PANNING,ImVec4(0.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SONG,ImVec4(1.0f,0.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_TIME,ImVec4(0.5f,0.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SPEED,ImVec4(1.0f,0.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_PRIMARY,ImVec4(0.5f,1.0f,0.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_SYS_SECONDARY,ImVec4(0.0f,1.0f,0.5f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PATTERN_EFFECT_MISC,ImVec4(0.3f,0.3f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_EE_VALUE,ImVec4(0.0f,1.0f,1.0f,1.0f));
  GET_UI_COLOR(GUI_COLOR_PLAYBACK_STAT,ImVec4(0.6f,0.6f,0.6f,1.0f));

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
  sty.Colors[ImGuiCol_TextSelectedBg]=secondaryHover;
  sty.Colors[ImGuiCol_PlotHistogram]=uiColors[GUI_COLOR_MACRO_OTHER];
  sty.Colors[ImGuiCol_PlotHistogramHovered]=uiColors[GUI_COLOR_MACRO_OTHER];

  if (settings.roundedWindows) sty.WindowRounding=8.0f;
  if (settings.roundedButtons) {
    sty.FrameRounding=6.0f;
    sty.GrabRounding=6.0f;
  }
  if (settings.roundedMenus) sty.PopupRounding=8.0f;

  sty.ScaleAllSizes(dpiScale);

  ImGui::GetStyle()=sty;

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

  // set to 800 for now due to problems with unifont
  static const ImWchar upTo800[]={0x20,0x7e,0xa0,0x800,0};
  ImFontGlyphRangesBuilder range;
  ImVector<ImWchar> outRange;

  range.AddRanges(upTo800);
  if (settings.loadJapanese) {
    range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
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
    logW("UI font path is empty! reverting to default font\n");
    settings.mainFont=0;
  }
  if (settings.patFont==6 && settings.patFontPath.empty()) {
    logW("pattern font path is empty! reverting to default font\n");
    settings.patFont=0;
  }

  ImFontConfig fc1;
  fc1.MergeMode=true;

  if (settings.mainFont==6) { // custom font
    if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.mainFontPath.c_str(),e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
      logW("could not load UI font! reverting to default font\n");
      settings.mainFont=0;
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
        logE("could not load UI font! falling back to Proggy Clean.\n");
        mainFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
    }
  } else if (settings.mainFont==5) { // system font
    if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_1,e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
      if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_2,e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
        if ((mainFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_FONT_PATH_3,e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
          logW("could not load UI font! reverting to default font\n");
          settings.mainFont=0;
          if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
            logE("could not load UI font! falling back to Proggy Clean.\n");
            mainFont=ImGui::GetIO().Fonts->AddFontDefault();
          }
        }
      }
    }
  } else {
    if ((mainFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFont[settings.mainFont],builtinFontLen[settings.mainFont],e->getConfInt("mainFontSize",18)*dpiScale,NULL,fontRange))==NULL) {
      logE("could not load UI font! falling back to Proggy Clean.\n");
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
    logE("could not load icon font!\n");
  }
  if (settings.mainFontSize==settings.patFontSize && settings.patFont<5 && builtinFontM[settings.patFont]==builtinFont[settings.mainFont]) {
    logD("using main font for pat font.\n");
    patFont=mainFont;
  } else {
    if (settings.patFont==6) { // custom font
      if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(settings.patFontPath.c_str(),e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
        logW("could not load pattern font! reverting to default font\n");
        settings.patFont=0;
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
          logE("could not load pattern font! falling back to Proggy Clean.\n");
          patFont=ImGui::GetIO().Fonts->AddFontDefault();
        }
      }
    } else if (settings.patFont==5) { // system font
      if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_1,e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
        if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_2,e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
          if ((patFont=ImGui::GetIO().Fonts->AddFontFromFileTTF(SYSTEM_PAT_FONT_PATH_3,e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
            logW("could not load pattern font! reverting to default font\n");
            settings.patFont=0;
            if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
              logE("could not load pattern font! falling back to Proggy Clean.\n");
              patFont=ImGui::GetIO().Fonts->AddFontDefault();
            }
          }
        }
      }
    } else {
      if ((patFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(builtinFontM[settings.patFont],builtinFontMLen[settings.patFont],e->getConfInt("patFontSize",18)*dpiScale,NULL,upTo800))==NULL) {
        logE("could not load pattern font!\n");
        patFont=ImGui::GetIO().Fonts->AddFontDefault();
      }
   }
  }
  if ((bigFont=ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(font_plexSans_compressed_data,font_plexSans_compressed_size,40*dpiScale))==NULL) {
    logE("could not load big UI font!\n");
  }

  mainFont->FallbackChar='?';
  mainFont->DotChar='.';

  // TODO: allow changing these colors.
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir,"",ImVec4(0.0f,1.0f,1.0f,1.0f),ICON_FA_FOLDER_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile,"",ImVec4(0.7f,0.7f,0.7f,1.0f),ICON_FA_FILE_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fur",ImVec4(0.5f,1.0f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fui",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fuw",ImVec4(1.0f,0.75f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmf",ImVec4(0.5f,1.0f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmp",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".dmw",ImVec4(1.0f,0.75f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".wav",ImVec4(1.0f,1.0f,0.5f,1.0f),ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgm",ImVec4(1.0f,1.0f,0.5f,1.0f),ICON_FA_FILE_AUDIO_O);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttf",ImVec4(0.3f,1.0f,0.6f,1.0f),ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".otf",ImVec4(0.3f,1.0f,0.6f,1.0f),ICON_FA_FONT);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".ttc",ImVec4(0.3f,1.0f,0.6f,1.0f),ICON_FA_FONT);

  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".tfi",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".vgi",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".s3i",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".sbi",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".fti",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);
  ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtension,".bti",ImVec4(1.0f,0.5f,0.5f,1.0f),ICON_FA_FILE);

  if (fileDialog!=NULL) delete fileDialog;
  fileDialog=new FurnaceGUIFileDialog(settings.sysFileDialog);
}