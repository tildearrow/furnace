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

static const char* powerNoiseControlBits[3]={
  _N("enable tap B"),
  _N("AM with slope"),
  NULL
};

void FurnaceGUI::insEditPowerNoise(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(_("Control"),&ins->std.ex1Macro,0,2,32,uiColors[GUI_COLOR_MACRO_NOISE],false,NULL,NULL,true,powerNoiseControlBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Tap A Location"),&ins->std.ex4Macro,0,15,96,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Tap B Location"),&ins->std.ex5Macro,0,15,96,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Load LFSR"),&ins->std.ex8Macro,0,16,256,uiColors[GUI_COLOR_MACRO_NOISE],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("PowerNoise")) {
    int pnOctave=ins->powernoise.octave;
    if (ImGui::InputInt(_("Octave offset"),&pnOctave,1,4)) { PARAMETER
      if (pnOctave<0) pnOctave=0;
      if (pnOctave>15) pnOctave=15;
      ins->powernoise.octave=pnOctave;
    }
    ImGui::Text(_("go to Macros for other parameters."));
    ImGui::EndTabItem();
  }
}
