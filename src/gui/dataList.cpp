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
#include "misc/cpp/imgui_stdlib.h"
#include "plot_nolerp.h"
#include "guiConst.h"
#include <fmt/printf.h>
#include <imgui.h>

const char* sampleNote[12]={
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void FurnaceGUI::drawInsList(bool asChild) {
  if (nextWindow==GUI_WINDOW_INS_LIST) {
    insListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insListOpen && !asChild) return;
  bool began=false;
  if (asChild) {
    began=ImGui::BeginChild("Instruments");
  } else {
    began=ImGui::Begin("Instruments",&insListOpen,globalWinFlags);
  }
  if (began) {
    if (settings.unifiedDataView) settings.horizontalDataView=0;
    if (ImGui::Button(ICON_FA_PLUS "##InsAdd")) {
      if (!settings.unifiedDataView) doAction(GUI_ACTION_INS_LIST_ADD);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Add");
    }
    if (settings.unifiedDataView) {
      if (ImGui::BeginPopupContextItem("UnifiedAdd",ImGuiMouseButton_Left)) {
        if (ImGui::MenuItem("instrument")) {
          doAction(GUI_ACTION_INS_LIST_ADD);
        }
        if (ImGui::MenuItem("wavetable")) {
          doAction(GUI_ACTION_WAVE_LIST_ADD);
        }
        if (ImGui::MenuItem("sample (create)")) {
          doAction(GUI_ACTION_SAMPLE_LIST_ADD);
        }
        ImGui::EndPopup();
      }
    } else {
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        displayInsTypeList=true;
        displayInsTypeListMakeInsSample=-1;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##InsClone")) {
      if (!settings.unifiedDataView) doAction(GUI_ACTION_INS_LIST_DUPLICATE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate");
    }
    if (settings.unifiedDataView) {
      if (ImGui::BeginPopupContextItem("UnifiedClone",ImGuiMouseButton_Left)) {
        if (ImGui::MenuItem("instrument")) {
          doAction(GUI_ACTION_INS_LIST_DUPLICATE);
        }
        if (ImGui::MenuItem("wavetable")) {
          doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
        }
        if (ImGui::MenuItem("sample")) {
          doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
        }
        ImGui::EndPopup();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##InsLoad")) {
      if (!settings.unifiedDataView) doAction(GUI_ACTION_INS_LIST_OPEN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Open");
    }
    if (settings.unifiedDataView) {
      if (ImGui::BeginPopupContextItem("UnifiedLoad",ImGuiMouseButton_Left)) {
        if (ImGui::MenuItem("instrument")) {
          doAction(GUI_ACTION_INS_LIST_OPEN);
        }
        if (ImGui::MenuItem("instrument (replace...)")) {
          doAction((curIns>=0 && curIns<(int)e->song.ins.size())?GUI_ACTION_INS_LIST_OPEN_REPLACE:GUI_ACTION_INS_LIST_OPEN);
        }
        if (ImGui::MenuItem("wavetable")) {
          doAction(GUI_ACTION_WAVE_LIST_OPEN);
        }
        if (ImGui::MenuItem("sample")) {
          doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("instrument from TX81Z")) {
          doAction(GUI_ACTION_TX81Z_REQUEST);
        }
        ImGui::EndPopup();
      }
    } else {
      if (ImGui::BeginPopupContextItem("InsOpenOpt")) {
        if (ImGui::MenuItem("replace...")) {
          doAction((curIns>=0 && curIns<(int)e->song.ins.size())?GUI_ACTION_INS_LIST_OPEN_REPLACE:GUI_ACTION_INS_LIST_OPEN);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("load from TX81Z")) {
          doAction(GUI_ACTION_TX81Z_REQUEST);
        }
        ImGui::EndPopup();
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Open (insert; right-click to replace)");
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##InsSave")) {
      if (!settings.unifiedDataView) doAction(GUI_ACTION_INS_LIST_SAVE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Save");
    }
    if (settings.unifiedDataView) {
      if (ImGui::BeginPopupContextItem("UnifiedSave",ImGuiMouseButton_Left)) {
        if (ImGui::MenuItem("instrument")) {
          doAction(GUI_ACTION_INS_LIST_SAVE);
        }
        if (ImGui::MenuItem("instrument (legacy .fui)")) {
          doAction(GUI_ACTION_INS_LIST_SAVE_OLD);
        }
        if (ImGui::MenuItem("instrument (.dmp)")) {
          doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
        }
        if (ImGui::MenuItem("wavetable")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE);
        }
        if (ImGui::MenuItem("wavetable (.dmw)")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_DMW);
        }
        if (ImGui::MenuItem("wavetable (raw)")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_RAW);
        }
        if (ImGui::MenuItem("sample")) {
          doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
        }
        ImGui::EndPopup();
      }
    } else {
      if (ImGui::BeginPopupContextItem("InsSaveFormats",ImGuiMouseButton_Right)) {
        if (ImGui::MenuItem("save in legacy format...")) {
          doAction(GUI_ACTION_INS_LIST_SAVE_OLD);
        }
        if (ImGui::MenuItem("save as .dmp...")) {
          doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
        }
        ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::ArrowButton("InsUp",ImGuiDir_Up)) {
        doAction(GUI_ACTION_INS_LIST_MOVE_UP);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Move up");
      }
      ImGui::SameLine();
      if (ImGui::ArrowButton("InsDown",ImGuiDir_Down)) {
        doAction(GUI_ACTION_INS_LIST_MOVE_DOWN);
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Move down");
      }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##InsDelete")) {
      if (!settings.unifiedDataView) doAction(GUI_ACTION_INS_LIST_DELETE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Delete");
    }
    if (settings.unifiedDataView) {
      if (ImGui::BeginPopupContextItem("UnifiedDelete",ImGuiMouseButton_Left)) {
        if (ImGui::MenuItem("instrument")) {
          doAction(GUI_ACTION_INS_LIST_DELETE);
        }
        if (ImGui::MenuItem("wavetable")) {
          doAction(GUI_ACTION_WAVE_LIST_DELETE);
        }
        if (ImGui::MenuItem("sample")) {
          doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
        }
        ImGui::EndPopup();
      }
    }
    ImGui::Separator();
    int availableRows=ImGui::GetContentRegionAvail().y/ImGui::GetFrameHeight();
    if (availableRows<1) availableRows=1;
    int columns=settings.horizontalDataView?(int)(ceil((double)(e->song.ins.size()+1)/(double)availableRows)):1;
    if (columns<1) columns=1;
    if (columns>64) columns=64;
    if (ImGui::BeginTable("InsListScroll",columns,(settings.horizontalDataView?ImGuiTableFlags_ScrollX:0)|ImGuiTableFlags_ScrollY)) {
      if (settings.unifiedDataView) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(ICON_FA_TASKS " Instruments");
        ImGui::Indent();
      }

      if (settings.horizontalDataView) {
        ImGui::TableNextRow();
      }

      int curRow=0;
      for (int i=-1; i<(int)e->song.ins.size(); i++) {
        ImGui::PushID(i);
        String name=ICON_FA_CIRCLE_O;
        const char* insType="Bug!";
        if (i>=0) {
          DivInstrument* ins=e->song.ins[i];
          insType=(ins->type>DIV_INS_MAX)?"Unknown":insTypes[ins->type];
          if (ins->type==DIV_INS_N163) insType=settings.c163Name.c_str();
          switch (ins->type) {
            case DIV_INS_FM:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FM]);
              name=fmt::sprintf(ICON_FA_AREA_CHART "##_INS%d",i);
              break;
            case DIV_INS_STD:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_STD]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_GB:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_GB]);
              name=fmt::sprintf(ICON_FA_GAMEPAD "##_INS%d",i);
              break;
            case DIV_INS_C64:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_C64]);
              name=fmt::sprintf(ICON_FA_KEYBOARD_O "##_INS%d",i);
              break;
            case DIV_INS_AMIGA:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AMIGA]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_PCE:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PCE]);
              name=fmt::sprintf(ICON_FA_ID_BADGE "##_INS%d",i);
              break;
            case DIV_INS_AY:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_AY8930:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY8930]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_TIA:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_TIA]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_SAA1099:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SAA1099]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_VIC:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VIC]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_PET:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PET]);
              name=fmt::sprintf(ICON_FA_SQUARE "##_INS%d",i);
              break;
            case DIV_INS_VRC6:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VRC6]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_VRC6_SAW:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VRC6_SAW]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_OPLL:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPLL]);
              name=fmt::sprintf(ICON_FA_AREA_CHART "##_INS%d",i);
              break;
            case DIV_INS_OPL:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPL]);
              name=fmt::sprintf(ICON_FA_AREA_CHART "##_INS%d",i);
              break;
            case DIV_INS_FDS:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FDS]);
              name=fmt::sprintf(ICON_FA_FLOPPY_O "##_INS%d",i);
              break;
            case DIV_INS_VBOY:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VBOY]);
              name=fmt::sprintf(ICON_FA_BINOCULARS "##_INS%d",i);
              break;
            case DIV_INS_N163:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_N163]);
              name=fmt::sprintf(ICON_FA_CALCULATOR "##_INS%d",i);
              break;
            case DIV_INS_SCC:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SCC]);
              name=fmt::sprintf(ICON_FA_CALCULATOR "##_INS%d",i);
              break;
            case DIV_INS_OPZ:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPZ]);
              name=fmt::sprintf(ICON_FA_AREA_CHART "##_INS%d",i);
              break;
            case DIV_INS_POKEY:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_POKEY]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_BEEPER:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_BEEPER]);
              name=fmt::sprintf(ICON_FA_SQUARE "##_INS%d",i);
              break;
            case DIV_INS_SWAN:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SWAN]);
              name=fmt::sprintf(ICON_FA_GAMEPAD "##_INS%d",i);
              break;
            case DIV_INS_MIKEY:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MIKEY]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_VERA:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VERA]);
              name=fmt::sprintf(ICON_FA_KEYBOARD_O "##_INS%d",i);
              break;
            case DIV_INS_X1_010:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_X1_010]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_ES5506:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_ES5506]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_MULTIPCM:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MULTIPCM]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_SNES:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SNES]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_SU:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SU]);
              name=fmt::sprintf(ICON_FA_MICROCHIP "##_INS%d",i);
              break;
            case DIV_INS_NAMCO:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_NAMCO]);
              name=fmt::sprintf(ICON_FA_PIE_CHART "##_INS%d",i);
              break;
            case DIV_INS_OPL_DRUMS:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPL_DRUMS]);
              name=fmt::sprintf(ICON_FA_COFFEE "##_INS%d",i);
              break;
            case DIV_INS_OPM:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPM]);
              name=fmt::sprintf(ICON_FA_AREA_CHART "##_INS%d",i);
              break;
            case DIV_INS_NES:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_NES]);
              name=fmt::sprintf(ICON_FA_GAMEPAD "##_INS%d",i);
              break;
            case DIV_INS_MSM6258:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MSM6258]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_MSM6295:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MSM6295]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_ADPCMA:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_ADPCMA]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_ADPCMB:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_ADPCMB]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_SEGAPCM:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SEGAPCM]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_QSOUND:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_QSOUND]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_YMZ280B:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_YMZ280B]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_RF5C68:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_RF5C68]);
              name=fmt::sprintf(ICON_FA_VOLUME_UP "##_INS%d",i);
              break;
            case DIV_INS_MSM5232:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MSM5232]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_T6W28:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_T6W28]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_K007232:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_K007232]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_GA20:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_GA20]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            case DIV_INS_POKEMINI:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_POKEMINI]);
              name=fmt::sprintf(ICON_FA_BAR_CHART "##_INS%d",i);
              break;
            default:
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_UNKNOWN]);
              name=fmt::sprintf(ICON_FA_QUESTION "##_INS%d",i);
              break;
          }
        } else {
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
        }
        if (!settings.horizontalDataView) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
        } else if (curRow==0) {
          ImGui::TableNextColumn();
        }
        if (ImGui::Selectable(name.c_str(),(i==-1)?(curIns<0 || curIns>=e->song.insLen):(curIns==i))) {
          curIns=i;
          wavePreviewInit=true;
        }
        if (wantScrollList && curIns==i) ImGui::SetScrollHereY();
        if (settings.insFocusesPattern && patternOpen && ImGui::IsItemActivated()) {
          nextWindow=GUI_WINDOW_PATTERN;
          curIns=i;
          wavePreviewInit=true;
        }
        if (ImGui::IsItemHovered() && i>=0 && !mobileUI) {
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
          ImGui::SetTooltip("%s",insType);
          ImGui::PopStyleColor();
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            insEditOpen=true;
            nextWindow=GUI_WINDOW_INS_EDIT;
          }
        }
        if (i>=0) {
          if (ImGui::BeginPopupContextItem("InsRightMenu")) {
            curIns=i;
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
            if (ImGui::MenuItem("replace...")) {
              doAction((curIns>=0 && curIns<(int)e->song.ins.size())?GUI_ACTION_INS_LIST_OPEN_REPLACE:GUI_ACTION_INS_LIST_OPEN);
            }
            if (ImGui::MenuItem("save")) {
              doAction(GUI_ACTION_INS_LIST_SAVE);
            }
            if (ImGui::MenuItem("save (legacy .fui)")) {
              doAction(GUI_ACTION_INS_LIST_SAVE_OLD);
            }
            if (ImGui::MenuItem("save (.dmp)")) {
              doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
            }
            if (ImGui::MenuItem("delete")) {
              doAction(GUI_ACTION_INS_LIST_DELETE);
            }
            ImGui::PopStyleColor();
            ImGui::EndPopup();
          }
        }
        if (i>=0) {
          if (i<(int)e->song.ins.size()) {
            DivInstrument* ins=e->song.ins[i];
            ImGui::SameLine();
            ImGui::Text("%.2X: %s",i,ins->name.c_str());
          }
        } else {
          ImGui::SameLine();
          ImGui::Text("- None -");
        }
        ImGui::PopStyleColor();
        if (settings.horizontalDataView) {
          if (++curRow>=availableRows) curRow=0;
        }
        ImGui::PopID();
      }

      if (settings.unifiedDataView) {
        ImGui::Unindent();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(ICON_FA_AREA_CHART " Wavetables");
        ImGui::Indent();
        actualWaveList();
        ImGui::Unindent();

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(ICON_FA_VOLUME_UP " Samples");
        ImGui::Indent();
        actualSampleList();
        ImGui::Unindent();
      }

      ImGui::EndTable();
    }
  }
  if (asChild) {
    ImGui::EndChild();
  } else {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_LIST;
    ImGui::End();
  }
}

void FurnaceGUI::drawWaveList(bool asChild) {
  if (nextWindow==GUI_WINDOW_WAVE_LIST) {
    waveListOpen=true;
    if (settings.unifiedDataView) {
      ImGui::SetWindowFocus("Instruments");
    } else {
      ImGui::SetNextWindowFocus();
    }
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (settings.unifiedDataView) return;
  if (!waveListOpen && !asChild) return;
  bool began=false;
  if (asChild) {
    began=ImGui::BeginChild("Wavetables");
  } else {
    began=ImGui::Begin("Wavetables",&waveListOpen,globalWinFlags);
  }
  if (began) {
    if (ImGui::Button(ICON_FA_PLUS "##WaveAdd")) {
      doAction(GUI_ACTION_WAVE_LIST_ADD);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Add");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##WaveClone")) {
      doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##WaveLoad")) {
      doAction(GUI_ACTION_WAVE_LIST_OPEN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Open");
    }
    if (ImGui::BeginPopupContextItem("WaveOpenOpt")) {
      if (ImGui::MenuItem("replace...")) {
        doAction((curWave>=0 && curWave<(int)e->song.wave.size())?GUI_ACTION_WAVE_LIST_OPEN_REPLACE:GUI_ACTION_WAVE_LIST_OPEN);
      }
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##WaveSave")) {
      doAction(GUI_ACTION_WAVE_LIST_SAVE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Save");
    }
    if (!settings.unifiedDataView) {
      if (ImGui::BeginPopupContextItem("WaveSaveFormats",ImGuiMouseButton_Right)) {
        if (ImGui::MenuItem("save as .dmw...")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_DMW);
        }
        if (ImGui::MenuItem("save raw...")) {
          doAction(GUI_ACTION_WAVE_LIST_SAVE_RAW);
        }
        ImGui::EndPopup();
      }
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_WAVE_LIST_MOVE_UP);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move up");
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_WAVE_LIST_MOVE_DOWN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move down");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##WaveDelete")) {
      doAction(GUI_ACTION_WAVE_LIST_DELETE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Delete");
    }
    ImGui::Separator();
    if (ImGui::BeginTable("WaveListScroll",1,ImGuiTableFlags_ScrollY)) {
      actualWaveList();
      ImGui::EndTable();
    }
  }
  if (asChild) {
    ImGui::EndChild();
  } else {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_LIST;
    ImGui::End();
  }
}

void FurnaceGUI::drawSampleList(bool asChild) {
  if (nextWindow==GUI_WINDOW_SAMPLE_LIST) {
    sampleListOpen=true;
    if (settings.unifiedDataView) {
      ImGui::SetWindowFocus("Instruments");
    } else {
      ImGui::SetNextWindowFocus();
    }
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (settings.unifiedDataView) return;
  if (!sampleListOpen && !asChild) return;
  bool began=false;
  if (asChild) {
    began=ImGui::BeginChild("Samples");
  } else {
    began=ImGui::Begin("Samples",&sampleListOpen,globalWinFlags);
  }
  if (began) {
    if (ImGui::Button(ICON_FA_FILE "##SampleAdd")) {
      doAction(GUI_ACTION_SAMPLE_LIST_ADD);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Add");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##SampleClone")) {
      doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Duplicate");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##SampleLoad")) {
      doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Open");
    }
    if (ImGui::BeginPopupContextItem("SampleOpenOpt")) {
      if (ImGui::MenuItem("replace...")) {
        doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE:GUI_ACTION_SAMPLE_LIST_OPEN);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("import raw...")) {
        doAction(GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
      }
      if (ImGui::MenuItem("import raw (replace)...")) {
        doAction((curSample>=0 && curSample<(int)e->song.sample.size())?GUI_ACTION_SAMPLE_LIST_OPEN_REPLACE_RAW:GUI_ACTION_SAMPLE_LIST_OPEN_RAW);
      }
      ImGui::EndPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##SampleSave")) {
      doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Save");
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move up");
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Move down");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##SampleDelete")) {
      doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Delete");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSampleL")) {
      doAction(GUI_ACTION_SAMPLE_LIST_PREVIEW);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Preview");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_OFF "##StopSampleL")) {
      doAction(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Stop preview");
    }
    ImGui::Separator();
    if (ImGui::BeginTable("SampleListScroll",1,ImGuiTableFlags_ScrollY)) {
      actualSampleList();
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }
  if (asChild) {
    ImGui::EndChild();
  } else {
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_LIST;
    ImGui::End();
  }
}

void FurnaceGUI::actualWaveList() {
  float wavePreview[257];
  for (int i=0; i<(int)e->song.wave.size(); i++) {
    DivWavetable* wave=e->song.wave[i];
    for (int i=0; i<wave->len; i++) {
      wavePreview[i]=wave->data[i];
    }
    if (wave->len>0) wavePreview[wave->len]=wave->data[wave->len-1];
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (ImGui::Selectable(fmt::sprintf("%d##_WAVE%d\n",i,i).c_str(),curWave==i)) {
      curWave=i;
    }
    if (wantScrollList && curWave==i) ImGui::SetScrollHereY();
    if (ImGui::IsItemHovered()) {
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        waveEditOpen=true;
      }
    }
    ImGui::SameLine();
    PlotNoLerp(fmt::sprintf("##_WAVEP%d",i).c_str(),wavePreview,wave->len+1,0,NULL,0,wave->max);
  }
}

void FurnaceGUI::actualSampleList() {
  for (int i=0; i<(int)e->song.sample.size(); i++) {
    bool memWarning=false;

    DivSample* sample=e->song.sample[i];
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    for (int j=0; j<e->song.systemLen; j++) {
      DivDispatch* dispatch=e->getDispatch(j);
      if (dispatch==NULL) continue;

      for (int k=0; k<DIV_MAX_SAMPLE_TYPE; k++) {
        if (dispatch->getSampleMemCapacity(k)==0) continue;
        if (!dispatch->isSampleLoaded(k,i) && sample->renderOn[k][j]) {
          memWarning=true;
          break;
        }
      }
      if (memWarning) break;
    }
    if (memWarning) ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_SAMPLE_CHIP_WARNING]);
    if (ImGui::Selectable(fmt::sprintf("%d: %s##_SAM%d",i,sample->name,i).c_str(),curSample==i)) {
      curSample=i;
      samplePos=0;
      updateSampleTex=true;
    }
    if (ImGui::IsItemHovered() && !mobileUI) {
      ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
      ImGui::SetTooltip("Bank %d: %s",i/12,sampleNote[i%12]);
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        sampleEditOpen=true;
      }
      ImGui::PopStyleColor();
    }
    if (memWarning) {
      ImGui::SameLine();
      ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE);
      if (ImGui::IsItemHovered() && !mobileUI) {
        ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_TEXT]);
        ImGui::SetTooltip("out of memory for this sample!");
        ImGui::PopStyleColor();
      }
      ImGui::PopStyleColor();
    }
    if (wantScrollList && curSample==i) ImGui::SetScrollHereY();
  }
}
