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

static const char* powerNoiseSlopeControlBits[7]={
  _N("invert B"),
  _N("invert A"),
  _N("reset B"),
  _N("reset A"),
  _N("clip B"),
  _N("clip A"),
  NULL
};

void FurnaceGUI::insEditPowerNoiseSlope(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(_("Control"),&ins->std.ex1Macro,0,6,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,powerNoiseSlopeControlBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Portion A Length"),&ins->std.ex2Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Portion B Length"),&ins->std.ex3Macro,0,255,128,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Portion A Offset"),&ins->std.ex6Macro,0,15,96,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Portion B Offset"),&ins->std.ex7Macro,0,15,96,uiColors[GUI_COLOR_MACRO_OTHER]));

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
