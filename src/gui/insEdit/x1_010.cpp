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

const char* x1_010EnvBits[8]={
  _N("enable"),
  _N("oneshot"),
  _N("split L/R"),
  _N("HinvR"),
  _N("VinvR"),
  _N("HinvL"),
  _N("VinvL"),
  NULL
};

static const char* x1EnvBits[4]={
  _N("hold"),
  _N("alternate"),
  _N("direction"),
  _N("enable")
};

void FurnaceGUI::insEditX1_010(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (!ins->amiga.useSample) {
    insTabWavetable(ins);
  }

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,MACRO_WAVE_COUNT,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,15,46,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    if (ins->amiga.useSample) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    } else {
      macroList.push_back(FurnaceGUIMacroDesc(_("Envelope Mode"),&ins->std.ex1Macro,0,7,160,uiColors[GUI_COLOR_MACRO_ENVELOPE],false,NULL,NULL,true,x1_010EnvBits));
      macroList.push_back(FurnaceGUIMacroDesc(_("Envelope"),&ins->std.ex2Macro,0,255,160,uiColors[GUI_COLOR_MACRO_ENVELOPE],false,NULL,NULL,false,x1EnvBits));
      macroList.push_back(FurnaceGUIMacroDesc(_("AutoEnv Num"),&ins->std.ex3Macro,0,15,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
      macroList.push_back(FurnaceGUIMacroDesc(_("AutoEnv Den"),&ins->std.algMacro,0,15,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    }

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }

  insTabSample(ins);
}
