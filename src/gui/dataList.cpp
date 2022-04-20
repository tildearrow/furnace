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

const char* sampleNote[12]={
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void FurnaceGUI::drawInsList() {
  if (nextWindow==GUI_WINDOW_INS_LIST) {
    insListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insListOpen) return;
  if (ImGui::Begin("Instruments",&insListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##InsAdd")) {
      doAction(GUI_ACTION_INS_LIST_ADD);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##InsClone")) {
      doAction(GUI_ACTION_INS_LIST_DUPLICATE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##InsLoad")) {
      doAction(GUI_ACTION_INS_LIST_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##InsSave")) {
      doAction(GUI_ACTION_INS_LIST_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("InsUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_INS_LIST_MOVE_UP);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("InsDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_INS_LIST_MOVE_DOWN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##InsDelete")) {
      doAction(GUI_ACTION_INS_LIST_DELETE);
    }
    ImGui::Separator();
    if (ImGui::BeginTable("InsListScroll",1,ImGuiTableFlags_ScrollY)) {
      if (settings.unifiedDataView) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(ICON_FA_TASKS " Instruments");
        ImGui::Indent();
      }

      for (int i=0; i<(int)e->song.ins.size(); i++) {
        DivInstrument* ins=e->song.ins[i];
        String name;
        switch (ins->type) {
          case DIV_INS_FM:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FM]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_STD:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_STD]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_GB:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_GB]);
            name=fmt::sprintf(ICON_FA_GAMEPAD " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_C64:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_C64]);
            name=fmt::sprintf(ICON_FA_KEYBOARD_O " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_AMIGA:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AMIGA]);
            name=fmt::sprintf(ICON_FA_VOLUME_UP " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_PCE:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PCE]);
            name=fmt::sprintf(ICON_FA_ID_BADGE " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_AY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_AY8930:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_AY8930]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_TIA:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_TIA]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_SAA1099:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SAA1099]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_VIC:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VIC]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_PET:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_PET]);
            name=fmt::sprintf(ICON_FA_SQUARE " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_VRC6:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VRC6]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_VRC6_SAW:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VRC6_SAW]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_OPLL:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPLL]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_OPL:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPL]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_FDS:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_FDS]);
            name=fmt::sprintf(ICON_FA_FLOPPY_O " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_VBOY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VBOY]);
            name=fmt::sprintf(ICON_FA_BINOCULARS " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_N163:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_N163]);
            name=fmt::sprintf(ICON_FA_CALCULATOR " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_SCC:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SCC]);
            name=fmt::sprintf(ICON_FA_CALCULATOR " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_OPZ:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_OPZ]);
            name=fmt::sprintf(ICON_FA_AREA_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_POKEY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_POKEY]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_BEEPER:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_BEEPER]);
            name=fmt::sprintf(ICON_FA_SQUARE " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_SWAN:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_SWAN]);
            name=fmt::sprintf(ICON_FA_GAMEPAD " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_MIKEY:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_MIKEY]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_VERA:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_VERA]);
            name=fmt::sprintf(ICON_FA_KEYBOARD_O " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_X1_010:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_X1_010]);
            name=fmt::sprintf(ICON_FA_BAR_CHART " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          case DIV_INS_ES5506:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_ES5506]);
            name=fmt::sprintf(ICON_FA_VOLUME_UP " %.2X: %s##_INS%d",i,ins->name,i);
            break;
          default:
            ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_INSTR_UNKNOWN]);
            name=fmt::sprintf(ICON_FA_QUESTION " %.2X: %s##_INS%d",i,ins->name,i);
            break;
        }
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Selectable(name.c_str(),curIns==i)) {
          curIns=i;
        }
        if (settings.insFocusesPattern && patternOpen && ImGui::IsItemActivated()) {
          nextWindow=GUI_WINDOW_PATTERN;
          curIns=i;
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("%s",(ins->type>DIV_INS_MAX)?"Unknown":insTypes[ins->type]);
          if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            insEditOpen=true;
            nextWindow=GUI_WINDOW_INS_EDIT;
          }
        }
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
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_LIST;
  ImGui::End();
}

void FurnaceGUI::drawWaveList() {
  if (nextWindow==GUI_WINDOW_WAVE_LIST) {
    waveListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!waveListOpen) return;
  if (ImGui::Begin("Wavetables",&waveListOpen)) {
    if (ImGui::Button(ICON_FA_PLUS "##WaveAdd")) {
      doAction(GUI_ACTION_WAVE_LIST_ADD);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##WaveClone")) {
      doAction(GUI_ACTION_WAVE_LIST_DUPLICATE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##WaveLoad")) {
      doAction(GUI_ACTION_WAVE_LIST_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##WaveSave")) {
      doAction(GUI_ACTION_WAVE_LIST_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_WAVE_LIST_UP);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("WaveDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_WAVE_LIST_DOWN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##WaveDelete")) {
      doAction(GUI_ACTION_WAVE_LIST_DELETE);
    }
    ImGui::Separator();
    if (ImGui::BeginTable("WaveListScroll",1,ImGuiTableFlags_ScrollY)) {
      actualWaveList();
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_WAVE_LIST;
  ImGui::End();
}

void FurnaceGUI::drawSampleList() {
  if (nextWindow==GUI_WINDOW_SAMPLE_LIST) {
    sampleListOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!sampleListOpen) return;
  if (ImGui::Begin("Samples",&sampleListOpen)) {
    if (ImGui::Button(ICON_FA_FILE "##SampleAdd")) {
      doAction(GUI_ACTION_SAMPLE_LIST_ADD);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILES_O "##SampleClone")) {
      doAction(GUI_ACTION_SAMPLE_LIST_DUPLICATE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##SampleLoad")) {
      doAction(GUI_ACTION_SAMPLE_LIST_OPEN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##SampleSave")) {
      doAction(GUI_ACTION_SAMPLE_LIST_SAVE);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleUp",ImGuiDir_Up)) {
      doAction(GUI_ACTION_SAMPLE_LIST_MOVE_UP);
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("SampleDown",ImGuiDir_Down)) {
      doAction(GUI_ACTION_SAMPLE_LIST_MOVE_DOWN);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_TIMES "##SampleDelete")) {
      doAction(GUI_ACTION_SAMPLE_LIST_DELETE);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_UP "##PreviewSampleL")) {
      doAction(GUI_ACTION_SAMPLE_LIST_PREVIEW);
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_VOLUME_OFF "##StopSampleL")) {
      doAction(GUI_ACTION_SAMPLE_LIST_STOP_PREVIEW);
    }
    ImGui::Separator();
    if (ImGui::BeginTable("SampleListScroll",1,ImGuiTableFlags_ScrollY)) {
      actualSampleList();
      ImGui::EndTable();
    }
    ImGui::Unindent();
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SAMPLE_LIST;
  ImGui::End();
}

void FurnaceGUI::actualWaveList() {
  float wavePreview[256];
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
    DivSample* sample=e->song.sample[i];
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (ImGui::Selectable(fmt::sprintf("%d: %s##_SAM%d",i,sample->name,i).c_str(),curSample==i)) {
      curSample=i;
      samplePos=0;
      updateSampleTex=true;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Bank %d: %s",i/12,sampleNote[i%12]);
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        sampleEditOpen=true;
      }
    }
  }
}
