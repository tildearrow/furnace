/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "insEditCommon.h"

static const char* saaShapeBits[3]={
  _N("tone"),
  _N("noise"),
  NULL
};

static const char* saaEnvBits[9]={
  _N("mirror"),
  _N("mode1"),
  _N("mode2"),
  _N("mode3"),
  _N("low resolution"),
  _N("use timer"),
  _N("timer square"),
  _N("enabled"),
  NULL
};

static const char* saaEnvModes[8]={
  _N("nothing (low)"),
  _N("nothing (high)"),
  _N("one-shot ramp down"),
  _N("saw wave (down)"),
  _N("one-shot triangle"),
  _N("triangle wave"),
  _N("one-shot ramp up"),
  _N("saw wave (up)")
};

void macroHoverSAAEnv(int id, float val, void* u) {
  if (ImGui::BeginTooltip()) {
    ImGui::Text("envelope mode chart:");
    if (ImGui::BeginTable("EnvModeChart",4,ImGuiTableFlags_Borders)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,ImGui::GetTextLineHeight());
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,ImGui::GetTextLineHeight());
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,ImGui::GetTextLineHeight());
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("3");
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("2");
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("1");
      ImGui::TableNextColumn();
      ImGui::TextUnformatted("result");

      for (int i=0; i<8; i++) {
        ImGui::TableNextRow();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,(i==((((int)val)>>1)&7))?ImGui::GetColorU32(ImGuiCol_FrameBgHovered):ImGui::GetColorU32(ImGuiCol_TableRowBg));
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,(i&4)?ImGui::GetColorU32(ImGuiCol_PlotHistogram):ImGui::GetColorU32(ImGuiCol_FrameBg));
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,(i&2)?ImGui::GetColorU32(ImGuiCol_PlotHistogram):ImGui::GetColorU32(ImGuiCol_FrameBg));
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg,(i&1)?ImGui::GetColorU32(ImGuiCol_PlotHistogram):ImGui::GetColorU32(ImGuiCol_FrameBg));
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(_(saaEnvModes[i]));
      }

      ImGui::EndTable();
    }
    ImGui::EndTooltip();
  }
}

void FurnaceGUI::insEditSAA1099(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Duty/Noise"),&ins->std.dutyMacro,0,3,160,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,2,64,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,true,saaShapeBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Envelope"),&ins->std.ex1Macro,0,8,160,uiColors[GUI_COLOR_MACRO_ENVELOPE],false,NULL,macroHoverSAAEnv,true,saaEnvBits));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
