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
#include "../intConst.h"

static const char* gbHWSeqCmdTypes[6]={
  _N("Envelope"),
  _N("Sweep"),
  _N("Wait"),
  _N("Wait for Release"),
  _N("Loop"),
  _N("Loop until Release")
};

void FurnaceGUI::drawGBEnv(unsigned char vol, unsigned char len, unsigned char sLen, bool dir, const ImVec2& size) {
  ImDrawList* dl=ImGui::GetWindowDrawList();
  ImGuiWindow* window=ImGui::GetCurrentWindow();

  ImVec2 minArea=window->DC.CursorPos;
  ImVec2 maxArea=ImVec2(
    minArea.x+size.x,
    minArea.y+size.y
  );
  ImRect rect=ImRect(minArea,maxArea);
  ImGuiStyle& style=ImGui::GetStyle();
  ImU32 color=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE]);
  //ImU32 colorS=ImGui::GetColorU32(uiColors[GUI_COLOR_FM_ENVELOPE_SUS_GUIDE]); // Sustain horiz/vert line color
  ImGui::ItemSize(size,style.FramePadding.y);
  if (ImGui::ItemAdd(rect,ImGui::GetID("gbEnv"))) {
    ImGui::RenderFrame(rect.Min,rect.Max,ImGui::GetColorU32(ImGuiCol_FrameBg),true,style.FrameRounding);
    
    float volY=1.0-((float)vol/15.0);
    float lenPos=(sLen>63)?1.0:((float)sLen/384.0);
    float envEndPoint=((float)len/7.0)*((float)(dir?(15-vol):vol)/15.0);

    ImVec2 pos1=ImLerp(rect.Min,rect.Max,ImVec2(0.0,volY));
    ImVec2 pos2;
    if (dir) {
      if (len>0) {
        if (lenPos<envEndPoint) {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY*(1.0-(lenPos/envEndPoint))));
        } else {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(envEndPoint,0.0));
        }
      } else {
        pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY));
      }
    } else {
      if (len>0) {
        if (lenPos<envEndPoint) {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY+(1.0-volY)*(lenPos/envEndPoint)));
        } else {
          pos2=ImLerp(rect.Min,rect.Max,ImVec2(envEndPoint,1.0));
        }
      } else {
        pos2=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,volY));
      }
    }
    ImVec2 pos3=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,(len>0 || sLen<64)?((dir && sLen>62)?0.0:1.0):volY));

    addAALine(dl,pos1,pos2,color);
    if (lenPos>=envEndPoint && sLen<64 && dir) {
      pos3=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,0.0));
      addAALine(dl,pos2,pos3,color);
      ImVec2 pos4=ImLerp(rect.Min,rect.Max,ImVec2(lenPos,1.0));
      addAALine(dl,pos3,pos4,color);
    } else {
      addAALine(dl,pos2,pos3,color);
    }
  }
}


void FurnaceGUI::insEditGB(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

   if (ImGui::BeginTabItem("Game Boy")) {
    P(ImGui::Checkbox(_("Use software envelope"),&ins->gb.softEnv));
    P(ImGui::Checkbox(_("Initialize envelope on every note"),&ins->gb.alwaysInit));
    P(ImGui::Checkbox(_("Double wave length (GBA only)"),&ins->gb.doubleWave));

    ImGui::BeginDisabled(ins->gb.softEnv);
    if (ImGui::BeginTable("GBParams",2)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.6f);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.4f);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::BeginTable("GBParamsI",2)) {
        ImGui::TableSetupColumn("ci0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("ci1",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(_("Volume"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        P(CWSliderScalar("##GBVolume",ImGuiDataType_U8,&ins->gb.envVol,&_ZERO,&_FIFTEEN)); rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(_("Length"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        P(CWSliderScalar("##GBEnvLen",ImGuiDataType_U8,&ins->gb.envLen,&_ZERO,&_SEVEN)); rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(_("Sound Length"));
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        P(CWSliderScalar("##GBSoundLen",ImGuiDataType_U8,&ins->gb.soundLen,&_ZERO,&_SIXTY_FOUR,ins->gb.soundLen>63?_("Infinity"):"%d")); rightClickable

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text(_("Direction"));
        ImGui::TableNextColumn();
        bool goesUp=ins->gb.envDir;
        if (ImGui::RadioButton(_("Up"),goesUp)) { PARAMETER
          goesUp=true;
          ins->gb.envDir=goesUp;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(_("Down"),!goesUp)) { PARAMETER
          goesUp=false;
          ins->gb.envDir=goesUp;
        }

        ImGui::EndTable();
      }

      ImGui::TableNextColumn();
      drawGBEnv(ins->gb.envVol,ins->gb.envLen,ins->gb.soundLen,ins->gb.envDir,ImVec2(ImGui::GetContentRegionAvail().x,100.0f*dpiScale));

      ImGui::EndTable();
    }

    if (ImGui::BeginChild("HWSeq",ImGui::GetContentRegionAvail(),ImGuiChildFlags_Borders,ImGuiWindowFlags_MenuBar)) {
      ImGui::BeginMenuBar();
      ImGui::Text(_("Hardware Sequence"));
      ImGui::EndMenuBar();

      if (ins->gb.hwSeqLen>0) if (ImGui::BeginTable("HWSeqList",3)) {
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
        for (int i=0; i<ins->gb.hwSeqLen; i++) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("%d (#%d)",curFrame,i);
          ImGui::TableNextColumn();
          ImGui::PushID(i);
          if (ins->gb.hwSeq[i].cmd>=DivInstrumentGB::DIV_GB_HWCMD_MAX) {
            ins->gb.hwSeq[i].cmd=0;
          }
          int cmd=ins->gb.hwSeq[i].cmd;
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::Combo("##HWSeqCmd",&cmd,LocalizedComboGetter,gbHWSeqCmdTypes,DivInstrumentGB::DIV_GB_HWCMD_MAX)) {
            if (ins->gb.hwSeq[i].cmd!=cmd) {
              ins->gb.hwSeq[i].cmd=cmd;
              ins->gb.hwSeq[i].data=0;
            }
          }
          bool somethingChanged=false;
          switch (ins->gb.hwSeq[i].cmd) {
            case DivInstrumentGB::DIV_GB_HWCMD_ENVELOPE: {
              int hwsVol=(ins->gb.hwSeq[i].data&0xf0)>>4;
              bool hwsDir=ins->gb.hwSeq[i].data&8;
              int hwsLen=ins->gb.hwSeq[i].data&7;
              int hwsSoundLen=ins->gb.hwSeq[i].data>>8;

              if (CWSliderInt(_("Volume"),&hwsVol,0,15)) {
                somethingChanged=true;
              }
              if (CWSliderInt(_("Env Length"),&hwsLen,0,7)) {
                somethingChanged=true;
              }
              if (CWSliderInt(_("Sound Length"),&hwsSoundLen,0,64,hwsSoundLen>63?_("Infinity"):"%d")) {
                somethingChanged=true;
              }
              if (ImGui::RadioButton(_("Up"),hwsDir)) { PARAMETER
                hwsDir=true;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton(_("Down"),!hwsDir)) { PARAMETER
                hwsDir=false;
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->gb.hwSeq[i].data=(hwsLen&7)|(hwsDir?8:0)|(hwsVol<<4)|(hwsSoundLen<<8);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentGB::DIV_GB_HWCMD_SWEEP: {
              int hwsShift=ins->gb.hwSeq[i].data&7;
              int hwsSpeed=(ins->gb.hwSeq[i].data&0x70)>>4;
              bool hwsDir=ins->gb.hwSeq[i].data&8;

              if (CWSliderInt(_("Shift"),&hwsShift,0,7)) {
                somethingChanged=true;
              }
              if (CWSliderInt(_("Speed"),&hwsSpeed,0,7)) {
                somethingChanged=true;
              }

              if (ImGui::RadioButton(_("Up"),!hwsDir)) { PARAMETER
                hwsDir=false;
                somethingChanged=true;
              }
              ImGui::SameLine();
              if (ImGui::RadioButton(_("Down"),hwsDir)) { PARAMETER
                hwsDir=true;
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->gb.hwSeq[i].data=(hwsShift&7)|(hwsDir?8:0)|(hwsSpeed<<4);
                PARAMETER;
              }
              break;
            }
            case DivInstrumentGB::DIV_GB_HWCMD_WAIT: {
              int len=ins->gb.hwSeq[i].data+1;
              curFrame+=ins->gb.hwSeq[i].data+1;

              if (ImGui::InputInt(_("Ticks"),&len,1,4)) {
                if (len<1) len=1;
                if (len>255) len=256;
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->gb.hwSeq[i].data=len-1;
                PARAMETER;
              }
              break;
            }
            case DivInstrumentGB::DIV_GB_HWCMD_WAIT_REL:
              curFrame++;
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_LOOP:
            case DivInstrumentGB::DIV_GB_HWCMD_LOOP_REL: {
              int pos=ins->gb.hwSeq[i].data;

              if (ImGui::InputInt(_("Position"),&pos,1,1)) {
                if (pos<0) pos=0;
                if (pos>(ins->gb.hwSeqLen-1)) pos=(ins->gb.hwSeqLen-1);
                somethingChanged=true;
              }

              if (somethingChanged) {
                ins->gb.hwSeq[i].data=pos;
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
                ins->gb.hwSeq[i-1].cmd^=ins->gb.hwSeq[i].cmd;
                ins->gb.hwSeq[i].cmd^=ins->gb.hwSeq[i-1].cmd;
                ins->gb.hwSeq[i-1].cmd^=ins->gb.hwSeq[i].cmd;

                ins->gb.hwSeq[i-1].data^=ins->gb.hwSeq[i].data;
                ins->gb.hwSeq[i].data^=ins->gb.hwSeq[i-1].data;
                ins->gb.hwSeq[i-1].data^=ins->gb.hwSeq[i].data;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) {
            if (i<ins->gb.hwSeqLen-1) {
              e->lockEngine([ins,i]() {
                ins->gb.hwSeq[i+1].cmd^=ins->gb.hwSeq[i].cmd;
                ins->gb.hwSeq[i].cmd^=ins->gb.hwSeq[i+1].cmd;
                ins->gb.hwSeq[i+1].cmd^=ins->gb.hwSeq[i].cmd;

                ins->gb.hwSeq[i+1].data^=ins->gb.hwSeq[i].data;
                ins->gb.hwSeq[i].data^=ins->gb.hwSeq[i+1].data;
                ins->gb.hwSeq[i+1].data^=ins->gb.hwSeq[i].data;
              });
            }
            MARK_MODIFIED;
          }
          ImGui::SameLine();
          pushDestColor();
          if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) {
            for (int j=i; j<ins->gb.hwSeqLen-1; j++) {
              ins->gb.hwSeq[j].cmd=ins->gb.hwSeq[j+1].cmd;
              ins->gb.hwSeq[j].data=ins->gb.hwSeq[j+1].data;
            }
            ins->gb.hwSeqLen--;
          }
          popDestColor();
          ImGui::PopID();
        }
        ImGui::EndTable();
      }

      if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) {
        if (ins->gb.hwSeqLen<255) {
          ins->gb.hwSeq[ins->gb.hwSeqLen].cmd=0;
          ins->gb.hwSeq[ins->gb.hwSeqLen].data=0;
          ins->gb.hwSeqLen++;
        }
      }
    }
    ImGui::EndChild();
    ImGui::EndDisabled();
    ImGui::EndTabItem();
  }

  insTabWavetable(ins);

  if (ImGui::BeginTabItem(_("Macros"))) {
    if (ins->gb.softEnv) {
      macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    }
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Duty/Noise"),&ins->std.dutyMacro,0,3,160,uiColors[GUI_COLOR_MACRO_NOISE]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.waveMacro,0,MACRO_WAVE_COUNT,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning"),&ins->std.panLMacro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
