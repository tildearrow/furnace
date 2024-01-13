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

void FurnaceGUI::drawInsGB(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("Game Boy")) 
  {
    P(ImGui::Checkbox("Use software envelope",&ins->gb.softEnv));
    P(ImGui::Checkbox("Initialize envelope on every note",&ins->gb.alwaysInit));

    ImGui::BeginDisabled(ins->gb.softEnv);
    if (ImGui::BeginTable("GBParams",2)) 
    {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.6f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.4f);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::BeginTable("GBParamsI",2)) 
      {
        ImGui::TableSetupColumn("ci0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("ci1",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Volume");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        P(CWSliderScalar("##GBVolume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN)); rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Length");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        P(CWSliderScalar("##GBEnvLen",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN)); rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Sound Length");
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        P(CWSliderScalar("##GBSoundLen",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?"Infinity":"%d")); rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Direction");
        ImGui::TableNextColumn();
        bool goesUp=ins->gb.envDir;
        if (ImGui::RadioButton("Up",goesUp)) 
        { PARAMETER
          goesUp=true;
          ins->gb.envDir=goesUp;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Down",!goesUp)) 
        { PARAMETER
          goesUp=false;
          ins->gb.envDir=goesUp;
        }

        ImGui::EndTable();
      }

      ImGui::TableNextColumn();
      drawGBEnv(ins->gb.envVol,ins->gb.envLen,ins->gb.soundLen,ins->gb.envDir,ImVec2(ImGui::GetContentRegionAvail().x,100.0f*dpiScale));

      ImGui::EndTable();
    }

    if (ImGui::BeginChild("HWSeq",ImGui::GetContentRegionAvail(),true,ImGuiWindowFlags_MenuBar)) 
    {
      ImGui::BeginMenuBar();
      ImGui::Text("Hardware Sequence");
      ImGui::EndMenuBar();

      if (ins->gb.hwSeqLen>0) if (ImGui::BeginTable("HWSeqList",3)) 
      {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        int curFrame=0;
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text("Tick");
        ImGui::TableNextColumn();
        ImGui::Text("Command");
        ImGui::TableNextColumn();
        ImGui::Text("Move/Remove");
        for (int i=0; i<ins->gb.hwSeqLen; i++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d (#%d)",curFrame,i);
          ImGui::TableNextColumn();
          ImGui::PushID(i);
          if (ins->gb.get_gb_hw_seq(i, true)->cmd>=DivInstrumentGB::DIV_GB_HWCMD_MAX) 
          {
            ins->gb.get_gb_hw_seq(i, true)->cmd=0;
          }
          int cmd=ins->gb.get_gb_hw_seq(i, true)->cmd;
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##HWSeqCmd",&cmd,gbHWSeqCmdTypes,DivInstrumentGB::DIV_GB_HWCMD_MAX)) 
          {
            if (ins->gb.get_gb_hw_seq(i, true)->cmd!=cmd) 
            {
              ins->gb.get_gb_hw_seq(i, true)->cmd=cmd;
              ins->gb.get_gb_hw_seq(i, true)->data=0;
            }
          }
          bool somethingChanged=false;
          switch (ins->gb.get_gb_hw_seq(i, true)->cmd) 
          {
            case DivInstrumentGB::DIV_GB_HWCMD_ENVELOPE: 
            {
              int hwsVol=(ins->gb.get_gb_hw_seq(i, true)->data&0xf0)>>4;
              bool hwsDir=ins->gb.get_gb_hw_seq(i, true)->data&8;
              int hwsLen=ins->gb.get_gb_hw_seq(i, true)->data&7;
              int hwsSoundLen=ins->gb.get_gb_hw_seq(i, true)->data>>8;

              if (CWSliderInt("Volume",&hwsVol,0,15)) 
              {
                somethingChanged=true;
              }
              if (CWSliderInt("Env Length",&hwsLen,0,7)) 
              {
                somethingChanged=true;
              }
              if (CWSliderInt("Sound Length",&hwsSoundLen,0,64,hwsSoundLen>63?"Infinity":"%d")) 
              {
                somethingChanged=true;
              }
              if (ImGui::RadioButton("Up",hwsDir)) 
              { PARAMETER
                hwsDir=true;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton("Down",!hwsDir)) 
              { PARAMETER
                hwsDir=false;
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->gb.get_gb_hw_seq(i, true)->data=(hwsLen&7)|(hwsDir?8:0)|(hwsVol<<4)|(hwsSoundLen<<8);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentGB::DIV_GB_HWCMD_SWEEP: 
            {
              int hwsShift=ins->gb.get_gb_hw_seq(i, true)->data&7;
              int hwsSpeed=(ins->gb.get_gb_hw_seq(i, true)->data&0x70)>>4;
              bool hwsDir=ins->gb.get_gb_hw_seq(i, true)->data&8;

              if (CWSliderInt("Shift",&hwsShift,0,7)) 
              {
                somethingChanged=true;
              }
              if (CWSliderInt("Speed",&hwsSpeed,0,7)) 
              {
                somethingChanged=true;
              }

              if (ImGui::RadioButton("Up",!hwsDir)) 
              { PARAMETER
                hwsDir=false;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton("Down",hwsDir)) 
              { PARAMETER
                hwsDir=true;
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->gb.get_gb_hw_seq(i, true)->data=(hwsShift&7)|(hwsDir?8:0)|(hwsSpeed<<4);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentGB::DIV_GB_HWCMD_WAIT: 
            {
              int len=ins->gb.get_gb_hw_seq(i, true)->data+1;
              curFrame+=ins->gb.get_gb_hw_seq(i, true)->data+1;

              if (ImGui::InputInt("Ticks",&len,1,4)) 
              {
                if (len<1) len=1;
                if (len>255) len=256;
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->gb.get_gb_hw_seq(i, true)->data=len-1;
                PARAMETER;
              }
              break;
            }
            case DivInstrumentGB::DIV_GB_HWCMD_WAIT_REL:
              curFrame++;
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_LOOP:
            case DivInstrumentGB::DIV_GB_HWCMD_LOOP_REL:
            {
              int pos=ins->gb.get_gb_hw_seq(i, true)->data;

              if (ImGui::InputInt("Position",&pos,1,1)) 
              {
                if (pos<0) pos=0;
                if (pos>(ins->gb.hwSeqLen-1)) pos=(ins->gb.hwSeqLen-1);
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->gb.get_gb_hw_seq(i, true)->data=pos;
                PARAMETER;
              }
              break;
            }
            default:
              break;
          }
          ImGui::PopID();
          ImGui::TableNextColumn();
          ImGui::PushID(i+512);
          if (ImGui::Button(ICON_FA_CHEVRON_UP "##HWCmdUp")) {
            if (i>0) {
              e->lockEngine([ins,i]() 
              {
                ins->gb.get_gb_hw_seq(i - 1, true)->cmd^=ins->gb.get_gb_hw_seq(i, true)->cmd;
                ins->gb.get_gb_hw_seq(i, true)->cmd^=ins->gb.get_gb_hw_seq(i - 1, true)->cmd;
                ins->gb.get_gb_hw_seq(i - 1, true)->cmd^=ins->gb.get_gb_hw_seq(i, true)->cmd;

                ins->gb.get_gb_hw_seq(i - 1, true)->data^=ins->gb.get_gb_hw_seq(i, true)->data;
                ins->gb.get_gb_hw_seq(i, true)->data^=ins->gb.get_gb_hw_seq(i - 1, true)->data;
                ins->gb.get_gb_hw_seq(i - 1, true)->data^=ins->gb.get_gb_hw_seq(i, true)->data;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) {
            if (i<ins->gb.hwSeqLen-1) 
            {
              e->lockEngine([ins,i]() 
              {
                ins->gb.get_gb_hw_seq(i + 1, true)->cmd^=ins->gb.get_gb_hw_seq(i, true)->cmd;
                ins->gb.get_gb_hw_seq(i, true)->cmd^=ins->gb.get_gb_hw_seq(i, true)->cmd;
                ins->gb.get_gb_hw_seq(i + 1, true)->cmd^=ins->gb.get_gb_hw_seq(i, true)->cmd;

                ins->gb.get_gb_hw_seq(i + 1, true)->data^=ins->gb.get_gb_hw_seq(i, true)->data;
                ins->gb.get_gb_hw_seq(i, true)->data^=ins->gb.get_gb_hw_seq(i, true)->data;
                ins->gb.get_gb_hw_seq(i + 1, true)->data^=ins->gb.get_gb_hw_seq(i, true)->data;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          pushDestColor();
          if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) 
          {
            for (int j=i; j<ins->gb.hwSeqLen-1; j++) 
            {
              ins->gb.get_gb_hw_seq(j, true)->cmd=ins->gb.get_gb_hw_seq(j+1, true)->cmd;
              ins->gb.get_gb_hw_seq(j, true)->data=ins->gb.get_gb_hw_seq(j+1, true)->data;
            }
            ins->gb.hwSeqLen--;
          }
          popDestColor();
          ImGui::PopID();
        }
        ImGui::EndTable();
      }

      if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) 
      {
        if (ins->gb.hwSeqLen<255) 
        {
          ins->gb.get_gb_hw_seq(ins->gb.hwSeqLen, true)->cmd=0;
          ins->gb.get_gb_hw_seq(ins->gb.hwSeqLen, true)->data=0;
          ins->gb.hwSeqLen++;
        }
      }
    }
    ImGui::EndChild();
    ImGui::EndDisabled();
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem("Macros")) 
  {
    panMax=2;

    if (ins->gb.softEnv) 
    {
      macroList.push_back(FurnaceGUIMacroDesc("Volume",ins,DIV_MACRO_VOL,0xff,0,15,64,uiColors[GUI_COLOR_MACRO_VOLUME]));
    }
    
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc("Duty/Noise",ins,DIV_MACRO_DUTY,0xff,0,3,64,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Waveform", ins, DIV_MACRO_WAVE, 0xff, 0, MAX(1,e->song.waveLen-1), 160, uiColors[GUI_COLOR_MACRO_WAVE], false, NULL, NULL));
    macroList.push_back(FurnaceGUIMacroDesc("Panning",ins,DIV_MACRO_PAN_LEFT,0xff,0,panMax,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));

    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}