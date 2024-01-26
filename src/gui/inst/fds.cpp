/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

void FurnaceGUI::drawInsFDS(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("FDS")) 
  {
    float modTable[32];
    int modTableInt[256];
    ImGui::Checkbox("Compatibility mode",&ins->fds.initModTableWithFirstWave);
    if (ImGui::IsItemHovered()) 
    {
      ImGui::SetTooltip("only use for compatibility with .dmf modules!\n- initializes modulation table with first wavetable\n- does not alter modulation parameters on instrument change");
    }
    if (ImGui::InputInt("Modulation depth",&ins->fds.modDepth,1,4)) 
    {
      if (ins->fds.modDepth<0) ins->fds.modDepth=0;
      if (ins->fds.modDepth>63) ins->fds.modDepth=63;
    }
    if (ImGui::InputInt("Modulation speed",&ins->fds.modSpeed,1,4)) 
    {
      if (ins->fds.modSpeed<0) ins->fds.modSpeed=0;
      if (ins->fds.modSpeed>4095) ins->fds.modSpeed=4095;
    }
    ImGui::Text("Modulation table");
    for (int i=0; i<32; i++) {
      modTable[i]=ins->fds.modTable[i];
      modTableInt[i]=ins->fds.modTable[i];
    }
    ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,96.0f*dpiScale);
    PlotCustom("ModTable",modTable,32,0,NULL,-4,3,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) 
    {
      macroDragStart=ImGui::GetItemRectMin();
      macroDragAreaSize=modTableSize;
      macroDragMin=-4;
      macroDragMax=3;
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

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
    if (ImGui::InputText("##MMLModTable",&mmlStringModTable))
    {
      int discardIt=0;
      memset(modTableInt,0,256*sizeof(int));
      decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,-4,3,false);
      for (int i=0; i<32; i++) 
      {
        if (i>=discardIt) 
        {
          modTableInt[i]=0;
        } 
        else 
        {
          if (modTableInt[i]<-4) modTableInt[i]=-4;
          if (modTableInt[i]>3) modTableInt[i]=3;
        }
        ins->fds.modTable[i]=modTableInt[i];
      }
      MARK_MODIFIED;
    }
    if (!ImGui::IsItemActive()) 
    {
      encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,false);
    }
    ImGui::EndTabItem();
  }

  insTabWave(ins);

  if (ImGui::BeginTabItem("Macros")) 
  {
    macroList.push_back(FurnaceGUIMacroDesc("Volume",ins,DIV_MACRO_VOL,0xff,0,32,64,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc("Waveform",ins,DIV_MACRO_WAVE,0xff,0,MAX(1,e->song.waveLen-1),160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL));
    macroList.push_back(FurnaceGUIMacroDesc("Mod Depth",ins,DIV_MACRO_EX1,0xff,0,63,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Mod Speed",ins,DIV_MACRO_EX2,0xff,0,4095,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Mod Position",ins,DIV_MACRO_EX3, 0xff,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}