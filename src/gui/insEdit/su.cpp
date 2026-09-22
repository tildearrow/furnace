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
#include "IconsFontAwesome4.h"

static const char* suHWSeqCmdTypes[7]={
  _N("Volume Sweep"),
  _N("Frequency Sweep"),
  _N("Cutoff Sweep"),
  _N("Wait"),
  _N("Wait for Release"),
  _N("Loop"),
  _N("Loop until Release")
};

static const char* suControlBits[5]={
  _N("ring mod"),
  _N("low pass"),
  _N("high pass"),
  _N("band pass"),
  NULL
};

String macroSoundUnitWaves(int id, float val, void* u) {
  const char* label="???";
  switch (((int)val)&7) {
    case 0:
      label=_("Square");
      break;
    case 1:
      label=_("Saw");
      break;
    case 2:
      label=_("Sine");
      break;
    case 3:
      label=_("Triangle");
      break;
    case 4:
      label=_("Noise");
      break;
    case 5:
      label=_("Short Noise");
      break;
    case 6:
      label=_("XOR Sine");
      break;
    case 7:
      label=_("XOR Triangle");
      break;
    default: break;
  }
  return fmt::sprintf("%d: %s",id,label);
}

void FurnaceGUI::insEditSU(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ImGui::BeginTabItem("Sound Unit")) {
    P(ImGui::Checkbox(_("Switch roles of frequency and phase reset timer"),&ins->su.switchRoles));
    if (ImGui::BeginChild("HWSeqSU",ImGui::GetContentRegionAvail(),ImGuiChildFlags_Borders,ImGuiWindowFlags_MenuBar)) {
      ImGui::BeginMenuBar();
      ImGui::Text(_("Hardware Sequence"));
      ImGui::EndMenuBar();

      if (ins->su.hwSeqLen>0) if (ImGui::BeginTable("HWSeqListSU",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
        int curFrame=0;
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        ImGui::TableNextColumn();
        ImGui::Text(_("Tick"));
        ImGui::TableNextColumn();
        ImGui::Text(_("Command"));
        ImGui::TableNextColumn();
        ImGui::Text(_("Move/Remove"));
        for (int i=0; i<ins->su.hwSeqLen; i++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d (#%d)",curFrame,i);
          ImGui::TableNextColumn();
          ImGui::PushID(i);
          if (ins->su.hwSeq[i].cmd>=DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX) {
            ins->su.hwSeq[i].cmd=0;
          }
          int cmd=ins->su.hwSeq[i].cmd;
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##HWSeqCmd",&cmd,LocalizedComboGetter,suHWSeqCmdTypes,DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX)) {
            if (ins->su.hwSeq[i].cmd!=cmd) {
              ins->su.hwSeq[i].cmd=cmd;
              ins->su.hwSeq[i].val=0;
              ins->su.hwSeq[i].bound=0;
              ins->su.hwSeq[i].speed=0;
            }
          }
          bool somethingChanged=false;
          switch (ins->su.hwSeq[i].cmd) {
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_VOL: {
              int swPeriod=ins->su.hwSeq[i].speed;
              int swBound=ins->su.hwSeq[i].bound;
              int swVal=ins->su.hwSeq[i].val&31;
              bool swDir=ins->su.hwSeq[i].val&32;
              bool swLoop=ins->su.hwSeq[i].val&64;
              bool swInvert=ins->su.hwSeq[i].val&128;

              if (ImGui::InputInt(_("Period"),&swPeriod,1,16)) {
                if (swPeriod<0) swPeriod=0;
                if (swPeriod>65535) swPeriod=65535;
                somethingChanged=true;
              }
              if (CWSliderInt(_("Amount"),&swVal,0,31)) {
                somethingChanged=true;
              }
              if (CWSliderInt(_("Bound"),&swBound,0,255)) {
                somethingChanged=true;
              }
              if (ImGui::RadioButton(_("Up"),swDir)) { PARAMETER
                swDir=true;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton(_("Down"),!swDir)) { PARAMETER
                swDir=false;
                somethingChanged=true;
              }
              if (ImGui::Checkbox(_("Loop"),&swLoop)) { PARAMETER
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::Checkbox(_("Flip"),&swInvert)) { PARAMETER
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->su.hwSeq[i].speed=swPeriod;
                ins->su.hwSeq[i].bound=swBound;
                ins->su.hwSeq[i].val=(swVal&31)|(swDir?32:0)|(swLoop?64:0)|(swInvert?128:0);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_PITCH:
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_CUT: {
              int swPeriod=ins->su.hwSeq[i].speed;
              int swBound=ins->su.hwSeq[i].bound;
              int swVal=ins->su.hwSeq[i].val&127;
              bool swDir=ins->su.hwSeq[i].val&128;

              if (ImGui::InputInt(_("Period"),&swPeriod,1,16)) {
                if (swPeriod<0) swPeriod=0;
                if (swPeriod>65535) swPeriod=65535;
                somethingChanged=true;
              }
              if (CWSliderInt(_("Amount"),&swVal,0,31)) {
                somethingChanged=true;
              }
              if (CWSliderInt(_("Bound"),&swBound,0,255)) {
                somethingChanged=true;
              }
              if (ImGui::RadioButton(_("Up"),swDir)) { PARAMETER
                swDir=true;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton(_("Down"),!swDir)) { PARAMETER
                swDir=false;
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->su.hwSeq[i].speed=swPeriod;
                ins->su.hwSeq[i].bound=swBound;
                ins->su.hwSeq[i].val=(swVal&127)|(swDir?128:0);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT: {
              int len=ins->su.hwSeq[i].val+1;
              curFrame+=ins->su.hwSeq[i].val+1;

              if (ImGui::InputInt(_("Ticks"),&len)) {
                if (len<1) len=1;
                if (len>255) len=256;
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->su.hwSeq[i].val=len-1;
                PARAMETER;
              }
              break;
            }
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT_REL:
              curFrame++;
              break;
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP:
            case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP_REL: {
              int pos=ins->su.hwSeq[i].val;

              if (ImGui::InputInt(_("Position"),&pos,1,4)) {
                if (pos<0) pos=0;
                if (pos>(ins->su.hwSeqLen-1)) pos=(ins->su.hwSeqLen-1);
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->su.hwSeq[i].val=pos;
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
              e->lockEngine([ins,i]() {
                ins->su.hwSeq[i-1].cmd^=ins->su.hwSeq[i].cmd;
                ins->su.hwSeq[i].cmd^=ins->su.hwSeq[i-1].cmd;
                ins->su.hwSeq[i-1].cmd^=ins->su.hwSeq[i].cmd;

                ins->su.hwSeq[i-1].speed^=ins->su.hwSeq[i].speed;
                ins->su.hwSeq[i].speed^=ins->su.hwSeq[i-1].speed;
                ins->su.hwSeq[i-1].speed^=ins->su.hwSeq[i].speed;

                ins->su.hwSeq[i-1].val^=ins->su.hwSeq[i].val;
                ins->su.hwSeq[i].val^=ins->su.hwSeq[i-1].val;
                ins->su.hwSeq[i-1].val^=ins->su.hwSeq[i].val;

                ins->su.hwSeq[i-1].bound^=ins->su.hwSeq[i].bound;
                ins->su.hwSeq[i].bound^=ins->su.hwSeq[i-1].bound;
                ins->su.hwSeq[i-1].bound^=ins->su.hwSeq[i].bound;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) {
            if (i<ins->su.hwSeqLen-1) {
              e->lockEngine([ins,i]() {
                ins->su.hwSeq[i+1].cmd^=ins->su.hwSeq[i].cmd;
                ins->su.hwSeq[i].cmd^=ins->su.hwSeq[i+1].cmd;
                ins->su.hwSeq[i+1].cmd^=ins->su.hwSeq[i].cmd;

                ins->su.hwSeq[i+1].speed^=ins->su.hwSeq[i].speed;
                ins->su.hwSeq[i].speed^=ins->su.hwSeq[i+1].speed;
                ins->su.hwSeq[i+1].speed^=ins->su.hwSeq[i].speed;

                ins->su.hwSeq[i+1].val^=ins->su.hwSeq[i].val;
                ins->su.hwSeq[i].val^=ins->su.hwSeq[i+1].val;
                ins->su.hwSeq[i+1].val^=ins->su.hwSeq[i].val;

                ins->su.hwSeq[i+1].bound^=ins->su.hwSeq[i].bound;
                ins->su.hwSeq[i].bound^=ins->su.hwSeq[i+1].bound;
                ins->su.hwSeq[i+1].bound^=ins->su.hwSeq[i].bound;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          pushDestColor();
          if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) {
            for (int j=i; j<ins->su.hwSeqLen-1; j++) {
              ins->su.hwSeq[j].cmd=ins->su.hwSeq[j+1].cmd;
              ins->su.hwSeq[j].speed=ins->su.hwSeq[j+1].speed;
              ins->su.hwSeq[j].val=ins->su.hwSeq[j+1].val;
              ins->su.hwSeq[j].bound=ins->su.hwSeq[j+1].bound;
            }
            ins->su.hwSeqLen--;
          }
          popDestColor();
          ImGui::PopID();
        }
        ImGui::EndTable();
      }

      if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) {
        if (ins->su.hwSeqLen<255) {
          ins->su.hwSeq[ins->su.hwSeqLen].cmd=0;
          ins->su.hwSeq[ins->su.hwSeqLen].speed=0;
          ins->su.hwSeq[ins->su.hwSeqLen].val=0;
          ins->su.hwSeq[ins->su.hwSeqLen].bound=0;
          ins->su.hwSeqLen++;
        }
      }
    }
    ImGui::EndChild();
    ImGui::EndTabItem();
  }

  insTabSample(ins);

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,127,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Duty/Noise"),&ins->std.dutyMacro,0,127,160,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,7,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,macroSoundUnitWaves,false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning"),&ins->std.panLMacro,-127,127,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(_("Cutoff"),&ins->std.ex1Macro,0,16383,160,uiColors[GUI_COLOR_MACRO_FILTER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Resonance"),&ins->std.ex2Macro,0,255,160,uiColors[GUI_COLOR_MACRO_FILTER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Control"),&ins->std.ex3Macro,0,4,64,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,NULL,true,suControlBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset Timer"),&ins->std.ex4Macro,0,65535,160,uiColors[GUI_COLOR_MACRO_PITCH])); // again reuse code from resonance macro but use ex4 instead

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
