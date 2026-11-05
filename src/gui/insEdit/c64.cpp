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

static const char* c64ShapeBits[5]={
  _N("triangle"),
  _N("saw"),
  _N("pulse"),
  _N("noise"),
  NULL
};

static const char* filtModeBits[5]={
  _N("low"),
  _N("band"),
  _N("high"),
  _N("ch3off"),
  NULL
};

static const char* c64TestGateBits[5]={
  _N("gate"),
  _N("sync"),
  _N("ring"),
  _N("test"),
  NULL
};


void FurnaceGUI::insEditC64(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;
  if (ImGui::BeginTabItem("C64")) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text(_("Waveform"));
    ImGui::SameLine();
    pushToggleColors(ins->c64.triOn);
    if (ImGui::Button(_("tri"))) { PARAMETER
      ins->c64.triOn=!ins->c64.triOn;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.sawOn);
    if (ImGui::Button(_("saw"))) { PARAMETER
      ins->c64.sawOn=!ins->c64.sawOn;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.pulseOn);
    if (ImGui::Button(_("pulse"))) { PARAMETER
      ins->c64.pulseOn=!ins->c64.pulseOn;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.noiseOn);
    if (ImGui::Button(_("noise"))) { PARAMETER
      ins->c64.noiseOn=!ins->c64.noiseOn;
    }
    popToggleColors();

    ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);

    if (ImGui::BeginTable("C64EnvParams",5,ImGuiTableFlags_NoHostExtendX)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
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
      ImGui::TableNextColumn();
      CENTER_TEXT("R");
      ImGui::TextUnformatted("R");
      ImGui::TableNextColumn();
      CENTER_TEXT(_("Envelope"));
      ImGui::TextUnformatted(_("Envelope"));

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN)); rightClickable
      ImGui::TableNextColumn();
      P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN)); rightClickable
      ImGui::TableNextColumn();
      drawFMEnv(0,16-ins->c64.a,16-ins->c64.d,15-ins->c64.r,15-ins->c64.r,15-ins->c64.s,0,0,0,15,16,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

      ImGui::EndTable();
    }

    P(CWSliderScalar(_("Duty"),ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE)); rightClickable

    bool resetDuty=ins->c64.resetDuty;
    if (ImGui::Checkbox(_("Reset duty on new note"),&resetDuty)) { PARAMETER
      ins->c64.resetDuty=resetDuty;
    }

    bool ringMod=ins->c64.ringMod;
    if (ImGui::Checkbox(_("Ring Modulation"),&ringMod)) { PARAMETER
      ins->c64.ringMod=ringMod;
    }
    bool oscSync=ins->c64.oscSync;
    if (ImGui::Checkbox(_("Oscillator Sync"),&oscSync)) { PARAMETER
      ins->c64.oscSync=oscSync;
    }

    P(ImGui::Checkbox(_("Enable filter"),&ins->c64.toFilter));
    P(ImGui::Checkbox(_("Initialize filter"),&ins->c64.initFilter));
    
    P(CWSliderScalar(_("Cutoff"),ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN)); rightClickable
    P(CWSliderScalar(_("Resonance"),ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN)); rightClickable

    ImGui::AlignTextToFramePadding();
    ImGui::Text(_("Filter Mode"));
    ImGui::SameLine();
    pushToggleColors(ins->c64.lp);
    if (ImGui::Button(_("low"))) { PARAMETER
      ins->c64.lp=!ins->c64.lp;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.bp);
    if (ImGui::Button(_("band"))) { PARAMETER
      ins->c64.bp=!ins->c64.bp;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.hp);
    if (ImGui::Button(_("high"))) { PARAMETER
      ins->c64.hp=!ins->c64.hp;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.ch3off);
    if (ImGui::Button(_("ch3off"))) { PARAMETER
      ins->c64.ch3off=!ins->c64.ch3off;
    }
    popToggleColors();

    if (ImGui::Checkbox(_("Absolute Cutoff Macro"),&ins->c64.filterIsAbs)) {
      ins->temp.vZoom[DIV_MACRO_ALG]=-1;
      PARAMETER;
    }
    if (ImGui::Checkbox(_("Absolute Duty Macro"),&ins->c64.dutyIsAbs)) {
      ins->temp.vZoom[DIV_MACRO_DUTY]=-1;
      PARAMETER;
    }

    P(ImGui::Checkbox(_("Don't test before new note"),&ins->c64.noTest));
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Duty"),&ins->std.dutyMacro,ins->c64.dutyIsAbs?0:-4095,4095,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,4,64,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,true,c64ShapeBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Cutoff"),&ins->std.algMacro,ins->c64.filterIsAbs?0:-2047,2047,160,uiColors[GUI_COLOR_MACRO_FILTER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Resonance"),&ins->std.ex2Macro,0,15,64,uiColors[GUI_COLOR_MACRO_FILTER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Filter Mode"),&ins->std.ex1Macro,0,4,64,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true,filtModeBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Filter Toggle"),&ins->std.ex3Macro,0,1,32,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(_("Special"),&ins->std.ex4Macro,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c64TestGateBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Attack"),&ins->std.ex5Macro,0,15,128,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Decay"),&ins->std.ex6Macro,0,15,128,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Sustain"),&ins->std.ex7Macro,0,15,128,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Release"),&ins->std.ex8Macro,0,15,128,uiColors[GUI_COLOR_MACRO_ENVELOPE]));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
