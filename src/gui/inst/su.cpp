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

void FurnaceGUI::drawInsSU(DivInstrument* ins)
{
  if (ImGui::BeginTabItem("Sound Unit")) 
  {
    P(ImGui::Checkbox("Switch roles of frequency and phase reset timer",&ins->su.switchRoles));
    if (ImGui::BeginChild("HWSeqSU",ImGui::GetContentRegionAvail(),true,ImGuiWindowFlags_MenuBar)) 
    {
      ImGui::BeginMenuBar();
      ImGui::Text("Hardware Sequence");
      ImGui::EndMenuBar();

      if (ins->su.hwSeqLen>0) if (ImGui::BeginTable("HWSeqListSU",3)) 
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
        for (int i=0; i<ins->su.hwSeqLen; i++) 
        {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d (#%d)",curFrame,i);
          ImGui::TableNextColumn();
          ImGui::PushID(i);
          if (ins->su.get_su_hw_seq(i, true)->cmd>=DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX)
          {
            ins->su.get_su_hw_seq(i, true)->cmd=0;
          }
          int cmd=ins->su.get_su_hw_seq(i, true)->cmd;
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##HWSeqCmd",&cmd,suHWSeqCmdTypes,DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX)) 
          {
            if (ins->su.get_su_hw_seq(i, true)->cmd!=cmd) 
            {
              ins->su.get_su_hw_seq(i, true)->cmd=cmd;
              ins->su.get_su_hw_seq(i, true)->val=0;
              ins->su.get_su_hw_seq(i, true)->bound=0;
              ins->su.get_su_hw_seq(i, true)->speed=0;
            }
          }
          bool somethingChanged=false;
          switch (ins->su.get_su_hw_seq(i, true)->cmd) 
          {
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_VOL: 
            {
              int swPeriod=ins->su.get_su_hw_seq(i, true)->speed;
              int swBound=ins->su.get_su_hw_seq(i, true)->bound;
              int swVal=ins->su.get_su_hw_seq(i, true)->val&31;
              bool swDir=ins->su.get_su_hw_seq(i, true)->val&32;
              bool swLoop=ins->su.get_su_hw_seq(i, true)->val&64;
              bool swInvert=ins->su.get_su_hw_seq(i, true)->val&128;

              if (ImGui::InputInt("Period",&swPeriod,1,16)) {
                if (swPeriod<0) swPeriod=0;
                if (swPeriod>65535) swPeriod=65535;
                somethingChanged=true;
              }
              if (CWSliderInt("Amount",&swVal,0,31)) 
              {
                somethingChanged=true;
              }
              if (CWSliderInt("Bound",&swBound,0,255)) 
              {
                somethingChanged=true;
              }
              if (ImGui::RadioButton("Up",swDir)) 
              { PARAMETER
                swDir=true;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton("Down",!swDir)) 
              { PARAMETER
                swDir=false;
                somethingChanged=true;
              }
              if (ImGui::Checkbox("Loop",&swLoop)) 
              { PARAMETER
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::Checkbox("Flip",&swInvert)) 
              { PARAMETER
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->su.get_su_hw_seq(i, true)->speed=swPeriod;
                ins->su.get_su_hw_seq(i, true)->bound=swBound;
                ins->su.get_su_hw_seq(i, true)->val=(swVal&31)|(swDir?32:0)|(swLoop?64:0)|(swInvert?128:0);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_PITCH:
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_CUT: 
            {
              int swPeriod=ins->su.get_su_hw_seq(i, true)->speed;
              int swBound=ins->su.get_su_hw_seq(i, true)->bound;
              int swVal=ins->su.get_su_hw_seq(i, true)->val&127;
              bool swDir=ins->su.get_su_hw_seq(i, true)->val&128;

              if (ImGui::InputInt("Period",&swPeriod,1,16))
              {
                if (swPeriod<0) swPeriod=0;
                if (swPeriod>65535) swPeriod=65535;
                somethingChanged=true;
              }
              if (CWSliderInt("Amount",&swVal,0,31)) 
              {
                somethingChanged=true;
              }
              if (CWSliderInt("Bound",&swBound,0,255)) 
              {
                somethingChanged=true;
              }
              if (ImGui::RadioButton("Up",swDir)) 
              { PARAMETER
                swDir=true;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton("Down",!swDir)) 
              { PARAMETER
                swDir=false;
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->su.get_su_hw_seq(i, true)->speed=swPeriod;
                ins->su.get_su_hw_seq(i, true)->bound=swBound;
                ins->su.get_su_hw_seq(i, true)->val=(swVal&127)|(swDir?128:0);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT: 
            {
              int len=ins->su.get_su_hw_seq(i, true)->val+1;
              curFrame+=ins->su.get_su_hw_seq(i, true)->val+1;

              if (ImGui::InputInt("Ticks",&len)) 
              {
                if (len<1) len=1;
                if (len>255) len=256;
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->su.get_su_hw_seq(i, true)->val=len-1;
                PARAMETER;
              }
              break;
            }
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT_REL:
              curFrame++;
              break;
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP:
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP_REL: 
            {
              int pos=ins->su.get_su_hw_seq(i, true)->val;

              if (ImGui::InputInt("Position",&pos,1,4)) 
              {
                if (pos<0) pos=0;
                if (pos>(ins->su.hwSeqLen-1)) pos=(ins->su.hwSeqLen-1);
                somethingChanged=true;
              }

              if (somethingChanged) 
              {
                ins->su.get_su_hw_seq(i, true)->val=pos;
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
          if (ImGui::Button(ICON_FA_CHEVRON_UP "##HWCmdUp")) 
          {
            if (i>0) 
            {
              e->lockEngine([ins,i]() 
              {
                ins->su.get_su_hw_seq(i - 1, true)->cmd^=ins->su.get_su_hw_seq(i, true)->cmd;
                ins->su.get_su_hw_seq(i, true)->cmd^=ins->su.get_su_hw_seq(i - 1, true)->cmd;
                ins->su.get_su_hw_seq(i - 1, true)->cmd^=ins->su.get_su_hw_seq(i, true)->cmd;

                ins->su.get_su_hw_seq(i - 1, true)->speed^=ins->su.get_su_hw_seq(i, true)->speed;
                ins->su.get_su_hw_seq(i, true)->speed^=ins->su.get_su_hw_seq(i - 1, true)->speed;
                ins->su.get_su_hw_seq(i - 1, true)->speed^=ins->su.get_su_hw_seq(i, true)->speed;

                ins->su.get_su_hw_seq(i - 1, true)->val^=ins->su.get_su_hw_seq(i, true)->val;
                ins->su.get_su_hw_seq(i, true)->val^=ins->su.get_su_hw_seq(i - 1, true)->val;
                ins->su.get_su_hw_seq(i - 1, true)->val^=ins->su.get_su_hw_seq(i, true)->val;

                ins->su.get_su_hw_seq(i - 1, true)->bound^=ins->su.get_su_hw_seq(i, true)->bound;
                ins->su.get_su_hw_seq(i, true)->bound^=ins->su.get_su_hw_seq(i - 1, true)->bound;
                ins->su.get_su_hw_seq(i - 1, true)->bound^=ins->su.get_su_hw_seq(i, true)->bound;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) 
          {
            if (i<ins->su.hwSeqLen-1) 
            {
              e->lockEngine([ins,i]() 
              {
                ins->su.get_su_hw_seq(i + 1, true)->cmd^=ins->su.get_su_hw_seq(i, true)->cmd;
                ins->su.get_su_hw_seq(i, true)->cmd^=ins->su.get_su_hw_seq(i + 1, true)->cmd;
                ins->su.get_su_hw_seq(i + 1, true)->cmd^=ins->su.get_su_hw_seq(i, true)->cmd;

                ins->su.get_su_hw_seq(i + 1, true)->speed^=ins->su.get_su_hw_seq(i, true)->speed;
                ins->su.get_su_hw_seq(i, true)->speed^=ins->su.get_su_hw_seq(i + 1, true)->speed;
                ins->su.get_su_hw_seq(i + 1, true)->speed^=ins->su.get_su_hw_seq(i, true)->speed;

                ins->su.get_su_hw_seq(i + 1, true)->val^=ins->su.get_su_hw_seq(i, true)->val;
                ins->su.get_su_hw_seq(i, true)->val^=ins->su.get_su_hw_seq(i + 1, true)->val;
                ins->su.get_su_hw_seq(i + 1, true)->val^=ins->su.get_su_hw_seq(i, true)->val;

                ins->su.get_su_hw_seq(i + 1, true)->bound^=ins->su.get_su_hw_seq(i, true)->bound;
                ins->su.get_su_hw_seq(i, true)->bound^=ins->su.get_su_hw_seq(i + 1, true)->bound;
                ins->su.get_su_hw_seq(i + 1, true)->bound^=ins->su.get_su_hw_seq(i, true)->bound;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          pushDestColor();
          if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) 
          {
            for (int j=i; j<ins->su.hwSeqLen-1; j++) 
            {
              ins->su.get_su_hw_seq(j, true)->cmd=ins->su.get_su_hw_seq(j+1, true)->cmd;
              ins->su.get_su_hw_seq(j, true)->speed=ins->su.get_su_hw_seq(j+1, true)->speed;
              ins->su.get_su_hw_seq(j, true)->val=ins->su.get_su_hw_seq(j+1, true)->val;
              ins->su.get_su_hw_seq(j, true)->bound=ins->su.get_su_hw_seq(j+1, true)->bound;
            }
            ins->su.hwSeqLen--;
          }
          popDestColor();
          ImGui::PopID();
        }
        ImGui::EndTable();
      }

      if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) 
      {
        if (ins->su.hwSeqLen<255) 
        {
          ins->su.get_su_hw_seq(ins->su.hwSeqLen, true)->cmd=0;
          ins->su.get_su_hw_seq(ins->su.hwSeqLen, true)->speed=0;
          ins->su.get_su_hw_seq(ins->su.hwSeqLen, true)->val=0;
          ins->su.get_su_hw_seq(ins->su.hwSeqLen, true)->bound=0;
          ins->su.hwSeqLen++;
        }
      }
    }
    ImGui::EndChild();
    ImGui::EndTabItem();
  }

  insTabSample(ins);

  if (ImGui::BeginTabItem("Macros")) 
  {
    panMin=-127;
    panMax=127;

    macroList.push_back(FurnaceGUIMacroDesc("Volume",ins,DIV_MACRO_VOL,0xff,0,127,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
    macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc("Duty/Noise",ins,DIV_MACRO_DUTY,0xff,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Waveform",ins,DIV_MACRO_WAVE,0xff,0,8,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL));

    macroList.push_back(FurnaceGUIMacroDesc("Panning",ins,DIV_MACRO_PAN_LEFT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));

    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    macroList.push_back(FurnaceGUIMacroDesc("Cutoff",ins,DIV_MACRO_EX1,0xff,0,16383,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Resonance",ins,DIV_MACRO_EX2,0xff,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc("Control",ins,DIV_MACRO_EX3, 0xff,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,suControlBits));
    macroList.push_back(FurnaceGUIMacroDesc("Phase Reset Timer",ins,DIV_MACRO_EX4,0xff,0,65535,160,uiColors[GUI_COLOR_MACRO_OTHER])); // again reuse code from resonance macro but use ex4 instead

    drawMacros(macroList,macroEditStateMacros);
    ImGui::EndTabItem();
  }
}