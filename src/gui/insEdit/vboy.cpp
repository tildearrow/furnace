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
#include "misc/cpp/imgui_stdlib.h"
#include "../intConst.h"
#include "../plot_nolerp.h"

void FurnaceGUI::insEditVBoy(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ins->type==DIV_INS_VBOY) if (ImGui::BeginTabItem("Virtual Boy")) {
    float modTable[32];
    int modTableInt[256];
    P(ImGui::Checkbox(_("Set modulation table (channel 5 only)"),&ins->fds.initModTableWithFirstWave));

    ImGui::BeginDisabled(!ins->fds.initModTableWithFirstWave);
    for (int i=0; i<32; i++) {
      modTable[i]=ins->fds.modTable[i];
      modTableInt[i]=modTableHex?((unsigned char)ins->fds.modTable[i]):ins->fds.modTable[i];
    }
    ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,256.0f*dpiScale);
    PlotCustom("##ModTable",modTable,32,0,NULL,-128,127,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      macroDragStart=ImGui::GetItemRectMin();
      macroDragAreaSize=modTableSize;
      macroDragMin=-128;
      macroDragMax=127;
      macroDragBitMode=false;
      macroDragInitialValueSet=false;
      macroDragInitialValue=false;
      macroDragLen=32;
      macroDragActive=true;
      macroDragCTarget=(unsigned char*)ins->fds.modTable;
      macroDragChar=true;
      macroDragLineMode=false;
      macroDragLineInitial=ImVec2(0,0);
      processDrags(ImGui::GetMousePos().x,ImGui::GetMousePos().y);
      ImGui::InhibitInertialScroll();
    }

    if (ImGui::Button(modTableHex?"Hex##MTHex":"Dec##MTHex")) {
      modTableHex=!modTableHex;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
    if (ImGui::InputText("##MMLModTable",&mmlStringModTable)) {
      int discardIt=0;
      memset(modTableInt,0,256*sizeof(int));
      decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,modTableHex?0:-128,modTableHex?255:127,modTableHex);
      for (int i=0; i<32; i++) {
        if (i>=discardIt) {
          modTableInt[i]=0;
        } else {
          if (modTableInt[i]>=128) modTableInt[i]-=256;
        }
        ins->fds.modTable[i]=modTableInt[i];
      }
      MARK_MODIFIED;
    }
    if (!ImGui::IsItemActive()) {
      encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,modTableHex);
    }
    ImGui::SameLine();

    ImGui::EndDisabled();
    ImGui::EndTabItem();
  }

  insTabWavetable(ins);

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Noise Length"),&ins->std.dutyMacro,0,7,160,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,MACRO_WAVE_COUNT,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
