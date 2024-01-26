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

#include "fmEnvUtil.h"
#include "stringsUtil.h"
#include "macroDraw.h"
#include "fmPublicVars.h"
#include "publicVars.h"
#include "fm.h"
#include "../intConst.h"

class FurnaceGUI;

void FurnaceGUI::drawInsN163(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("Namco 163")) 
  {
    bool preLoad=ins->n163.waveMode&0x1;
    if (ImGui::Checkbox("Load waveform",&preLoad)) 
    { PARAMETER
      ins->n163.waveMode=(ins->n163.waveMode&~0x1)|(preLoad?0x1:0);
    }

    if (ImGui::IsItemHovered()) 
    {
      ImGui::SetTooltip("when enabled, a waveform will be loaded into RAM.\nwhen disabled, only the offset and length change.");
    }

    if (preLoad) 
    {
      if (ImGui::InputInt("Waveform##WAVE",&ins->n163.wave,1,10)) 
      { PARAMETER
        if (ins->n163.wave<0) ins->n163.wave=0;
        if (ins->n163.wave>=e->song.waveLen) ins->n163.wave=e->song.waveLen-1;
      }
    }

    ImGui::Separator();

    P(ImGui::Checkbox("Per-channel wave position/length",&ins->n163.perChanPos));

    if (ins->n163.perChanPos) 
    {
      if (ImGui::BeginTable("N1PerChPos",3)) 
      {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text("Ch");
        ImGui::TableNextColumn();
        ImGui::Text("Position");
        ImGui::TableNextColumn();
        ImGui::Text("Length");

        for (int i=0; i<8; i++) 
        {
          ImGui::PushID(64+i);
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Dummy(ImVec2(dpiScale,ImGui::GetFrameHeightWithSpacing()));
          ImGui::SameLine();
          ImGui::Text("%d",i+1);

          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##pcOff",&ins->n163.wavePosCh[i],1,16)) 
          { PARAMETER
            if (ins->n163.wavePosCh[i]<0) ins->n163.wavePosCh[i]=0;
            if (ins->n163.wavePosCh[i]>255) ins->n163.wavePosCh[i]=255;
          }

          ImGui::TableNextColumn();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##pcLen",&ins->n163.waveLenCh[i],4,16)) 
          { PARAMETER
            if (ins->n163.waveLenCh[i]<0) ins->n163.waveLenCh[i]=0;
            if (ins->n163.waveLenCh[i]>252) ins->n163.waveLenCh[i]=252;
            ins->n163.waveLenCh[i]&=0xfc;
          }
          ImGui::PopID();
        }

        ImGui::EndTable();
      }
    } 
    else 
    {
      if (ImGui::InputInt("Position##WAVEPOS",&ins->n163.wavePos,1,16)) 
      { PARAMETER
        if (ins->n163.wavePos<0) ins->n163.wavePos=0;
        if (ins->n163.wavePos>255) ins->n163.wavePos=255;
      }
      if (ImGui::InputInt("Length##WAVELEN",&ins->n163.waveLen,4,16)) 
      { PARAMETER
        if (ins->n163.waveLen<0) ins->n163.waveLen=0;
        if (ins->n163.waveLen>252) ins->n163.waveLen=252;
        ins->n163.waveLen&=0xfc;
      }
    }
    ImGui::EndTabItem();
  }

  insTabWave(ins);

  if (ImGui::BeginTabItem("Macros")) 
  {
    macroList.push_back(FurnaceGUIMacroDesc("Volume",ins,DIV_MACRO_VOL,0xff,0,15,64,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc("Wave Pos",ins,DIV_MACRO_DUTY,0xff,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Waveform",ins,DIV_MACRO_WAVE,0xff,0,MAX(1,e->song.waveLen-1),160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL));
    macroList.push_back(FurnaceGUIMacroDesc("Wave Length",ins,DIV_MACRO_EX1,0xff,0,252,160,uiColors[GUI_COLOR_MACRO_OTHER]));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}