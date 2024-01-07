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

/*#include "fmEnvUtil.h"
#include "stringsUtil.h"
#include "macroDraw.h"
#include "fmPublicVars.h"
#include "publicVars.h"
#include "fm.h"
#include "../intConst.h"
#include "../plot_nolerp.h"*/

#include "../gui.h"
#include "../../ta-log.h"
#include "../imgui_internal.h"
#include "../../engine/macroInt.h"
#include "../misc/cpp/imgui_stdlib.h"
#include "../guiConst.h"
#include "../intConst.h"
#include <fmt/printf.h>
#include <imgui.h>
#include "../plot_nolerp.h"

#include "macroDraw.h"
#include "stringsUtil.h"
#include "sampleDraw.h"
#include "fmEnvUtil.h"

#include "publicVars.h"
#include "fmPublicVars.h"

#include "fm.h"

extern "C" {
#include "../../../extern/Nuked-OPLL/opll.h"
}

class FurnaceGUI;

void FurnaceGUI::drawInsVBOY(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("Virtual Boy")) 
  {
    float modTable[32];
    int modTableInt[256];
    P(ImGui::Checkbox("Set modulation table (channel 5 only)",&ins->fds.initModTableWithFirstWave));

    ImGui::BeginDisabled(!ins->fds.initModTableWithFirstWave);
    for (int i=0; i<32; i++) 
    {
      modTable[i]=ins->fds.modTable[i];
      modTableInt[i]=modTableHex?((unsigned char)ins->fds.modTable[i]):ins->fds.modTable[i];
    }
    ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,256.0f*dpiScale);
    PlotCustom("ModTable",modTable,32,0,NULL,-128,127,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) 
    {
      macroDragStart=ImGui::GetItemRectMin();
      macroDragAreaSize=modTableSize;
      macroDragMin=-128;
      macroDragMax=127;
      macroDragBitOff=0;
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
    
    if (ImGui::Button(modTableHex?"Hex##MTHex":"Dec##MTHex")) 
    {
      modTableHex=!modTableHex;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
    if (ImGui::InputText("##MMLModTable",&mmlStringModTable)) 
    {
      int discardIt=0;
      memset(modTableInt,0,256*sizeof(int));
      decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,modTableHex?0:-128,modTableHex?255:127,modTableHex);
      for (int i=0; i<32; i++) {
        if (i>=discardIt) 
        {
          modTableInt[i]=0;
        } 
        else 
        {
          if (modTableInt[i]>=128) modTableInt[i]-=256;
        }
        ins->fds.modTable[i]=modTableInt[i];
      }
      MARK_MODIFIED;
    }
    if (!ImGui::IsItemActive()) 
    {
      encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,modTableHex);
    }
    ImGui::SameLine();

    ImGui::EndDisabled();
    ImGui::EndTabItem();
  }

  insTabWave(ins);

  if (ImGui::BeginTabItem("Macros")) 
  {
    panMin=0;
    panMax=15;

    macroList.push_back(FurnaceGUIMacroDesc("Volume",ins,DIV_MACRO_VOL,0xff,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc("Noise Length",ins,DIV_MACRO_DUTY,0xff,0,7,32,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Waveform",ins,DIV_MACRO_WAVE,0xff,0,MAX(1,e->song.waveLen-1),160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL));
    macroList.push_back(FurnaceGUIMacroDesc("Panning (left)",ins,DIV_MACRO_PAN_LEFT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
    macroList.push_back(FurnaceGUIMacroDesc("Panning (right)",ins,DIV_MACRO_PAN_RIGHT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}