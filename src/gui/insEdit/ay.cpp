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

static const char* ayShapeBits[4]={
  _N("tone"),
  _N("noise"),
  _N("envelope"),
  NULL
};

static const char* ayEnvBits[4]={
  _N("hold"),
  _N("alternate"),
  _N("direction"),
  _N("enable")
};

String macroTFXModes(int id, float val, void* u) {
  switch (((int)val)&3) {
    case 0:
      return _("Disabled");
    case 1:
      return _("PWM");
    case 2:
      return _("SyncBuzzer");
    case 3:
      return _("Reserved");
    default:
      return "???";
  }
  return "???";
}


void FurnaceGUI::insEditAY(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    if (!ins->amiga.useSample) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Noise Freq"),&ins->std.dutyMacro,0,31,160,uiColors[GUI_COLOR_MACRO_NOISE]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,3,48,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,true,ayShapeBits));
    }
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(_("Envelope"),&ins->std.ex2Macro,0,4,64,uiColors[GUI_COLOR_MACRO_ENVELOPE],false,NULL,NULL,true,ayEnvBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("AutoEnv Num"),&ins->std.ex3Macro,0,15,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("AutoEnv Den"),&ins->std.algMacro,0,15,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Force Period"),&ins->std.ex4Macro,0,4095,160,uiColors[GUI_COLOR_MACRO_PITCH]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Env Period"),&ins->std.ex5Macro,0,65535,160,uiColors[GUI_COLOR_MACRO_ENVELOPE]));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }

  if (!ins->amiga.useSample) {
    if (ImGui::BeginTabItem(_("Timer Macros")))
    {
      ImGui::Text(_("warning: timer effects require direct stream mode to be enabled during VGM export!"));
      macroList.push_back(FurnaceGUIMacroDesc(_("Timer FX"),&ins->std.ex6Macro,0,2,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,macroTFXModes));
      macroList.push_back(FurnaceGUIMacroDesc(_("Timer Offset"),&ins->std.ex7Macro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode,NULL,false,NULL,false,NULL,false,true));
      macroList.push_back(FurnaceGUIMacroDesc(_("Timer Num"),&ins->std.ex8Macro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("Timer Den"),&ins->std.fmsMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      macroList.push_back(FurnaceGUIMacroDesc(_("PWM Boundary"),&ins->std.amsMacro,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
      drawMacros(macroList,macroEditStateMacros,ins);
      ImGui::EndTabItem();
    }
  }

  insTabSample(ins);
}
