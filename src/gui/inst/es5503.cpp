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

void FurnaceGUI::drawInsES5503(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("ES5503")) 
  {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Oscillator mode:");
    ImGui::SameLine();
    bool freerun = (ins->es5503.initial_osc_mode == OSC_MODE_FREERUN);
    bool oneshot = (ins->es5503.initial_osc_mode == OSC_MODE_ONESHOT);
    bool sync_am = (ins->es5503.initial_osc_mode == OSC_MODE_SYNC_OR_AM);
    bool swap = (ins->es5503.initial_osc_mode == OSC_MODE_SWAP);
    if (ImGui::RadioButton("Freerun",freerun)) 
    { PARAMETER
      freerun=true;
      oneshot=false;
      sync_am=false;
      swap=false;
      ins->es5503.initial_osc_mode=OSC_MODE_FREERUN;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Oneshot",oneshot)) 
    { PARAMETER
      freerun=false;
      oneshot=true;
      sync_am=false;
      swap=false;
      ins->es5503.initial_osc_mode=OSC_MODE_ONESHOT;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Sync/AM",sync_am)) 
    { PARAMETER
      freerun=false;
      oneshot=false;
      sync_am=true;
      swap=false;
      ins->es5503.initial_osc_mode=OSC_MODE_SYNC_OR_AM;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Swap",swap)) 
    { PARAMETER
      freerun=false;
      oneshot=false;
      sync_am=true;
      swap=false;
      ins->es5503.initial_osc_mode=OSC_MODE_SWAP;
    }

    P(ImGui::Checkbox("Virtual softpan channel",&ins->es5503.softpan_virtual_channel));
    if (ImGui::IsItemHovered())
    {
      ImGui::SetTooltip("Combines odd and next even channel into one virtual channel with 256-step panning.\nInstrument, volume and effects need to be placed on the odd channel (e.g. 1st, 3rd, 5th etc.)");
    }

    P(ImGui::Checkbox("Phase reset on key-on",&ins->es5503.phase_reset_on_start));
    
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Macros")) 
  {
    volMax=255;
    dutyLabel="Osc. mode";
    dutyMax=3;
    waveMax=255;
    waveLabel="Wavetable";
    waveMax=255;
    ex1Max=255;
    ex2Max=7;
    panMax=255;

    macroList.push_back(FurnaceGUIMacroDesc(volumeLabel,ins,DIV_MACRO_VOL,0xff,volMin,volMax,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));

    if(!ins->amiga.useSample)
    {
      macroList.push_back(FurnaceGUIMacroDesc(waveLabel,ins,DIV_MACRO_WAVE,0xff,0,waveMax,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL));
    }

    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    macroList.push_back(FurnaceGUIMacroDesc("Wave/sample pos.",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Osc. output",ins,DIV_MACRO_EX2,0xff,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER]));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}