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
#include "../intConst.h"

static const char* snesModeBits[6]={
  _N("noise"),
  _N("echo"),
  _N("pitch mod"),
  _N("invert right"),
  _N("invert left"),
  NULL
};

String macroHoverGain(int id, float val, void* u) {
  if (val>=224.0f) {
    return fmt::sprintf(_("%d: +%d (exponential)"),id,(int)(val-224));
  }
  if (val>=192.0f) {
    return fmt::sprintf(_("%d: +%d (linear)"),id,(int)(val-192));
  }
  if (val>=160.0f) {
    return fmt::sprintf(_("%d: -%d (exponential)"),id,(int)(val-160));
  }
  if (val>=128.0f) {
    return fmt::sprintf(_("%d: -%d (linear)"),id,(int)(val-128));
  }
  return fmt::sprintf(_("%d: %d (direct)"),id,(int)val);
}

void FurnaceGUI::insEditSNES(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  insTabSample(ins);

  if (ImGui::BeginTabItem("SNES")) {
    P(ImGui::Checkbox(_("Use envelope"),&ins->snes.useEnv));
    ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);
    if (ins->snes.useEnv) {
      if (ImGui::BeginTable("SNESEnvParams",ins->snes.sus?6:5,ImGuiTableFlags_NoHostExtendX)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        if (ins->snes.sus) {
          ImGui::TableSetupColumn("c2x",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        }
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        CENTER_TEXT("A");
        ImGui::TextUnformatted("A");
        ImGui::TableNextColumn();
        CENTER_TEXT("D");
        ImGui::TextUnformatted("D");
        ImGui::TableNextColumn();
        CENTER_TEXT("S");
        ImGui::TextUnformatted("S");
        if (ins->snes.sus) {
          ImGui::TableNextColumn();
          CENTER_TEXT("D2");
          ImGui::TextUnformatted("D2");
        }
        ImGui::TableNextColumn();
        CENTER_TEXT("R");
        ImGui::TextUnformatted("R");
        ImGui::TableNextColumn();
        CENTER_TEXT(_("Envelope"));
        ImGui::TextUnformatted(_("Envelope"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->snes.a,&_ZERO,&_FIFTEEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->snes.d,&_ZERO,&_SEVEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->snes.s,&_ZERO,&_SEVEN)); rightClickable
        if (ins->snes.sus) {
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Decay2",sliderSize,ImGuiDataType_U8,&ins->snes.d2,&_ZERO,&_THIRTY_ONE)); rightClickable
        }
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->snes.r,&_ZERO,&_THIRTY_ONE)); rightClickable
        ImGui::TableNextColumn();
        drawFMEnv(0,ins->snes.a+1,1+ins->snes.d*2,ins->snes.sus?ins->snes.d2:ins->snes.r,ins->snes.sus?ins->snes.r:31,(14-ins->snes.s*2),(ins->snes.r==0 || (ins->snes.sus && ins->snes.d2==0)),0,0,7,16,31,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

        ImGui::EndTable();
      }
      ImGui::Text(_("Sustain/release mode:"));
      if (ImGui::RadioButton(_("Direct (cut on release)"),ins->snes.sus==0)) {
        ins->snes.sus=0;
      }
      if (ImGui::RadioButton(_("Effective (linear decrease)"),ins->snes.sus==1)) {
        ins->snes.sus=1;
      }
      if (ImGui::RadioButton(_("Effective (exponential decrease)"),ins->snes.sus==2)) {
        ins->snes.sus=2;
      }
      if (ImGui::RadioButton(_("Delayed (write R on release)"),ins->snes.sus==3)) {
        ins->snes.sus=3;
      }
    } else {
      if (ImGui::BeginTable("SNESGainParams",2,ImGuiTableFlags_NoHostExtendX)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        CENTER_TEXT(_("Gain Mode"));
        ImGui::TextUnformatted(_("Gain Mode"));
        ImGui::TableNextColumn();
        CENTER_TEXT(_("Gain"));
        ImGui::TextUnformatted(_("Gain"));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::RadioButton(_("Direct"),ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT)) {
          ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DIRECT;
          PARAMETER;
        }
        if (ImGui::RadioButton(_("Decrease (linear)"),ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LINEAR)) {
          ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LINEAR;
          PARAMETER;
        }
        if (ImGui::RadioButton(_("Decrease (logarithmic)"),ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LOG)) {
          ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LOG;
          PARAMETER;
        }
        if (ImGui::RadioButton(_("Increase (linear)"),ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_INC_LINEAR)) {
          ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_INC_LINEAR;
          PARAMETER;
        }
        if (ImGui::RadioButton(_("Increase (bent line)"),ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_INC_INVLOG)) {
          ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_INC_INVLOG;
          PARAMETER;
        }

        ImGui::TableNextColumn();
        unsigned char gainMax=(ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT)?127:31;
        if (ins->snes.gain>gainMax) ins->snes.gain=gainMax;
        P(CWVSliderScalar("##Gain",sliderSize,ImGuiDataType_U8,&ins->snes.gain,&_ZERO,&gainMax)); rightClickable

        ImGui::EndTable();
      }
      if (ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LINEAR || ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LOG) {
        ImGui::TextWrapped(_("using decrease modes will not produce any sound at all, unless you know what you are doing.\nit is recommended to use the Gain macro for decrease instead."));
      }
    }
    ImGui::EndTabItem();
  }

  insTabWavetable(ins);

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,127,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Noise Freq"),&ins->std.dutyMacro,0,31,160,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,MACRO_WAVE_COUNT,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,127,158,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,127,158,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Special"),&ins->std.ex1Macro,0,5,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,snesModeBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Gain"),&ins->std.ex2Macro,0,255,256,uiColors[GUI_COLOR_MACRO_VOLUME],false,NULL,macroHoverGain,false));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
