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

void FurnaceGUI::drawInsPOWERNOISE(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("PowerNoise")) 
  {
    int pnOctave=ins->powernoise.octave;

    if (ImGui::InputInt("Octave offset",&pnOctave,1,4)) 
    { PARAMETER
      if (pnOctave<0) pnOctave=0;
      if (pnOctave>15) pnOctave=15;
      ins->powernoise.octave=pnOctave;
    }

    ImGui::Text("go to Macros for other parameters.");
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Macros")) 
  {
    panMin = 0;
    panMax = 15;

    macroList.push_back(FurnaceGUIMacroDesc("Volume",ins,DIV_MACRO_VOL,0xff,0,15,64,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));

    macroList.push_back(FurnaceGUIMacroDesc("Panning (left)",ins,DIV_MACRO_PAN_LEFT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
    macroList.push_back(FurnaceGUIMacroDesc("Panning (right)",ins,DIV_MACRO_PAN_RIGHT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER]));

    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    macroList.push_back(FurnaceGUIMacroDesc("Control",ins,DIV_MACRO_EX1,0xff,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,powerNoiseControlBits));
    macroList.push_back(FurnaceGUIMacroDesc("Tap A Location",ins,DIV_MACRO_EX4,0xff,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Tap B Location",ins,DIV_MACRO_EX5,0xff,0,15,64,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Load LFSR",ins,DIV_MACRO_EX8,0xff,0,16,256,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}