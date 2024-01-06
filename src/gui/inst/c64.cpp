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

#include "fmEnvUtil.h"
#include "stringsUtil.h"
#include "macroDraw.h"
#include "fmPublicVars.h"
#include "publicVars.h"
#include "fm.h"
#include "../intConst.h"

class FurnaceGUI;

void FurnaceGUI::drawInsC64(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("C64")) 
  {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Waveform");
    ImGui::SameLine();
    pushToggleColors(ins->c64.triOn);
    if (ImGui::Button("tri")) 
    { PARAMETER
      ins->c64.triOn=!ins->c64.triOn;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.sawOn);
    if (ImGui::Button("saw")) 
    { PARAMETER
      ins->c64.sawOn=!ins->c64.sawOn;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.pulseOn);
    if (ImGui::Button("pulse")) 
    { PARAMETER
      ins->c64.pulseOn=!ins->c64.pulseOn;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.noiseOn);
    if (ImGui::Button("noise")) 
    { PARAMETER
      ins->c64.noiseOn=!ins->c64.noiseOn;
    }
    popToggleColors();

    ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);

    if (ImGui::BeginTable("C64EnvParams",5,ImGuiTableFlags_NoHostExtendX))
    {
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
      CENTER_TEXT("Envelope");
      ImGui::TextUnformatted("Envelope");

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

    P(CWSliderScalar("Duty",ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE)); rightClickable

    bool ringMod=ins->c64.ringMod;
    if (ImGui::Checkbox("Ring Modulation",&ringMod)) 
    { PARAMETER
      ins->c64.ringMod=ringMod;
    }
    bool oscSync=ins->c64.oscSync;
    if (ImGui::Checkbox("Oscillator Sync",&oscSync)) 
    { PARAMETER
      ins->c64.oscSync=oscSync;
    }

    P(ImGui::Checkbox("Enable filter",&ins->c64.toFilter));
    P(ImGui::Checkbox("Initialize filter",&ins->c64.initFilter));
    
    P(CWSliderScalar("Cutoff",ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN)); rightClickable
    P(CWSliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN)); rightClickable

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Filter Mode");
    ImGui::SameLine();
    pushToggleColors(ins->c64.lp);
    if (ImGui::Button("low")) 
    { PARAMETER
      ins->c64.lp=!ins->c64.lp;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.bp);
    if (ImGui::Button("band")) 
    { PARAMETER
      ins->c64.bp=!ins->c64.bp;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.hp);
    if (ImGui::Button("high")) 
    { PARAMETER
      ins->c64.hp=!ins->c64.hp;
    }
    popToggleColors();
    ImGui::SameLine();
    pushToggleColors(ins->c64.ch3off);
    if (ImGui::Button("ch3off")) 
    { PARAMETER
      ins->c64.ch3off=!ins->c64.ch3off;
    }
    popToggleColors();

    if (ImGui::Checkbox("Absolute Cutoff Macro",&ins->c64.filterIsAbs)) 
    {
      ins->std.get_macro(DIV_MACRO_ALG, true)->vZoom=-1;
      PARAMETER;
    }
    if (ImGui::Checkbox("Absolute Duty Macro",&ins->c64.dutyIsAbs)) 
    {
      ins->std.get_macro(DIV_MACRO_DUTY, true)->vZoom=-1;
      PARAMETER;
    }
    P(ImGui::Checkbox("Don't test before new note",&ins->c64.noTest));
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Macros")) 
  {
    volMax=15;
    dutyLabel="Duty";
    if (ins->c64.dutyIsAbs)
    {
      dutyMax=4095;
    }

    else 
    {
      dutyMin=-4095;
      dutyMax=4095;
    }

    waveMax=4;
    waveNames=c64ShapeBits;
    ex1Max=4;
    ex2Max=15;

    int cutoffMin=-2047;
    int cutoffMax=2047;

    if (ins->c64.filterIsAbs) 
    {
      cutoffMin=0;
    }

    macroList.push_back(FurnaceGUIMacroDesc(volumeLabel,ins,DIV_MACRO_VOL,0xff,volMin,volMax,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(waveLabel,ins,DIV_MACRO_WAVE,0xff,0,waveMax,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,true,waveNames,0));

    macroList.push_back(FurnaceGUIMacroDesc("Cutoff",ins,DIV_MACRO_ALG,0xff,cutoffMin,cutoffMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Filter Mode",ins,DIV_MACRO_EX1,0xff,0,ex1Max,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,filtModeBits));
    macroList.push_back(FurnaceGUIMacroDesc("Filter Toggle",ins,DIV_MACRO_EX3,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc("Resonance",ins,DIV_MACRO_EX2,0xff,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER]));

    macroList.push_back(FurnaceGUIMacroDesc("Special",ins,DIV_MACRO_EX4,0xff,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c64TestGateBits));
    macroList.push_back(FurnaceGUIMacroDesc("Attack",ins,DIV_MACRO_EX5, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Decay",ins,DIV_MACRO_EX6, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Sustain",ins,DIV_MACRO_EX7, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Release",ins,DIV_MACRO_EX8, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}