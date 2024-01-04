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

#define _USE_MATH_DEFINES
#include "gui.h"
#include "../ta-log.h"
#include "imgui_internal.h"
#include "../engine/macroInt.h"
#include "IconsFontAwesome4.h"
#include "furIcons.h"
#include "misc/cpp/imgui_stdlib.h"
#include "guiConst.h"
#include "intConst.h"
#include <fmt/printf.h>
#include <imgui.h>
#include "plot_nolerp.h"

#include "inst/macroDraw.h"
#include "inst/stringsUtil.h"
#include "inst/sampleDraw.h"
#include "inst/fmEnvUtil.h"

#include "inst/publicVars.h"
#include "inst/fmPublicVars.h"

#include "inst/fm.h"

extern "C" {
#include "../../extern/Nuked-OPLL/opll.h"
}

void FurnaceGUI::drawActualInsEditor()
{
  DivInstrument* ins=e->song.ins[curIns];
  if (updateFMPreview) {
    renderFMPreview(ins);
    updateFMPreview=false;
  }
  if (settings.insEditColorize) {
    if (ins->type>=DIV_INS_MAX) {
      pushAccentColors(uiColors[GUI_COLOR_INSTR_UNKNOWN],uiColors[GUI_COLOR_INSTR_UNKNOWN],uiColors[GUI_COLOR_INSTR_UNKNOWN],ImVec4(0.0f,0.0f,0.0f,0.0f));
    } else {
      pushAccentColors(uiColors[GUI_COLOR_INSTR_STD+ins->type],uiColors[GUI_COLOR_INSTR_STD+ins->type],uiColors[GUI_COLOR_INSTR_STD+ins->type],ImVec4(0.0f,0.0f,0.0f,0.0f));
    }
  }
  if (ImGui::BeginTable("InsProp",3)) {
    ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    String insIndex=fmt::sprintf("%.2X",curIns);
    ImGui::SetNextItemWidth(72.0f*dpiScale);
    if (ImGui::BeginCombo("##InsSelect",insIndex.c_str())) {
      String name;
      for (size_t i=0; i<e->song.ins.size(); i++) {
        name=fmt::sprintf("%.2X: %s##_INSS%d",i,e->song.ins[i]->name,i);
        if (ImGui::Selectable(name.c_str(),curIns==(int)i)) {
          curIns=i;
          ins=e->song.ins[curIns];
          wavePreviewInit=true;
          updateFMPreview=true;
        }
      }
      ImGui::EndCombo();
    }

    ImGui::TableNextColumn();
    ImGui::Text("Name");

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::PushID(2+curIns);
    if (ImGui::InputText("##Name",&ins->name)) {
      MARK_MODIFIED;
    }
    ImGui::PopID();

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    if (ImGui::Button(ICON_FA_FOLDER_OPEN "##IELoad")) {
      doAction(GUI_ACTION_INS_LIST_OPEN_REPLACE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Open");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FLOPPY_O "##IESave")) {
      doAction(GUI_ACTION_INS_LIST_SAVE);
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Save");
    }
    if (ImGui::BeginPopupContextItem("InsSaveFormats",ImGuiMouseButton_Right)) {
      if (ImGui::MenuItem("save as .dmp...")) {
        doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
      }
      ImGui::EndPopup();
    }

    ImGui::TableNextColumn();
    ImGui::Text("Type");

    ImGui::TableNextColumn();
    int insType=ins->type;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    bool warnType=true;
    for (DivInstrumentType i: e->getPossibleInsTypes()) {
      if (i==insType) {
        warnType=false;
      }
    }

    pushWarningColor(warnType,warnType && failedNoteOn);
    if (ImGui::BeginCombo("##Type",(insType>=DIV_INS_MAX)?"Unknown":insTypes[insType][0])) {
      std::vector<DivInstrumentType> insTypeList;
      if (settings.displayAllInsTypes) {
        for (int i=0; insTypes[i][0]; i++) {
          insTypeList.push_back((DivInstrumentType)i);
        }
      } else {
        insTypeList=e->getPossibleInsTypes();
      }
      for (DivInstrumentType i: insTypeList) {
        if (ImGui::Selectable(insTypes[i][0],insType==i)) {
          ins->type=i;

          // reset macro zoom
          
          for(int i = 0; i < (int)ins->std.macros.size(); i++)
          {
            ins->std.macros[i].vZoom = -1;
          }

          for(int j = 0; j < (int)ins->std.ops.size(); j++)
          {
            for(int i = 0; i < (int)ins->std.ops[j].macros.size(); i++)
            {
              ins->std.ops[j].macros[i].vZoom = -1;
            }
          }
        }
      }
      ImGui::EndCombo();
    } else if (warnType) {
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("none of the currently present chips are able to play this instrument type!");
      }
    }
    popWarningColor();

    ImGui::EndTable();
  }

  if (ImGui::BeginTabBar("insEditTab")) {
    //std::vector<FurnaceGUIMacroDesc> macroList;
    macroList.clear();
    macroList.shrink_to_fit();

    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPM || ins->type==DIV_INS_ESFM) {
      drawInsFM(ins);
    }

    if (ins->type==DIV_INS_GB) if (ImGui::BeginTabItem("Game Boy")) {
      P(ImGui::Checkbox("Use software envelope",&ins->gb.softEnv));
      P(ImGui::Checkbox("Initialize envelope on every note",&ins->gb.alwaysInit));

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
          if (ImGui::RadioButton("Up",goesUp)) { PARAMETER
            goesUp=true;
            ins->gb.envDir=goesUp;
          }
          ImGui::SameLine();
          if (ImGui::RadioButton("Down",!goesUp)) { PARAMETER
            goesUp=false;
            ins->gb.envDir=goesUp;
          }

          ImGui::EndTable();
        }

        ImGui::TableNextColumn();
        drawGBEnv(ins->gb.envVol,ins->gb.envLen,ins->gb.soundLen,ins->gb.envDir,ImVec2(ImGui::GetContentRegionAvail().x,100.0f*dpiScale));

        ImGui::EndTable();
      }

      if (ImGui::BeginChild("HWSeq",ImGui::GetContentRegionAvail(),true,ImGuiWindowFlags_MenuBar)) {
        ImGui::BeginMenuBar();
        ImGui::Text("Hardware Sequence");
        ImGui::EndMenuBar();

        if (ins->gb.hwSeqLen>0) if (ImGui::BeginTable("HWSeqList",3)) {
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
            if (ins->gb.get_gb_hw_seq(i, true)->cmd>=DivInstrumentGB::DIV_GB_HWCMD_MAX) {
              ins->gb.get_gb_hw_seq(i, true)->cmd=0;
            }
            int cmd=ins->gb.get_gb_hw_seq(i, true)->cmd;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::Combo("##HWSeqCmd",&cmd,gbHWSeqCmdTypes,DivInstrumentGB::DIV_GB_HWCMD_MAX)) {
              if (ins->gb.get_gb_hw_seq(i, true)->cmd!=cmd) {
                ins->gb.get_gb_hw_seq(i, true)->cmd=cmd;
                ins->gb.get_gb_hw_seq(i, true)->data=0;
              }
            }
            bool somethingChanged=false;
            switch (ins->gb.get_gb_hw_seq(i, true)->cmd) {
              case DivInstrumentGB::DIV_GB_HWCMD_ENVELOPE: {
                int hwsVol=(ins->gb.get_gb_hw_seq(i, true)->data&0xf0)>>4;
                bool hwsDir=ins->gb.get_gb_hw_seq(i, true)->data&8;
                int hwsLen=ins->gb.get_gb_hw_seq(i, true)->data&7;
                int hwsSoundLen=ins->gb.get_gb_hw_seq(i, true)->data>>8;

                if (CWSliderInt("Volume",&hwsVol,0,15)) {
                  somethingChanged=true;
                }
                if (CWSliderInt("Env Length",&hwsLen,0,7)) {
                  somethingChanged=true;
                }
                if (CWSliderInt("Sound Length",&hwsSoundLen,0,64,hwsSoundLen>63?"Infinity":"%d")) {
                  somethingChanged=true;
                }
                if (ImGui::RadioButton("Up",hwsDir)) { PARAMETER
                  hwsDir=true;
                  somethingChanged=true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Down",!hwsDir)) { PARAMETER
                  hwsDir=false;
                  somethingChanged=true;
                }

                if (somethingChanged) {
                  ins->gb.get_gb_hw_seq(i, true)->data=(hwsLen&7)|(hwsDir?8:0)|(hwsVol<<4)|(hwsSoundLen<<8);
                  PARAMETER;
                }
                break;
              }
              case DivInstrumentGB::DIV_GB_HWCMD_SWEEP: {
                int hwsShift=ins->gb.get_gb_hw_seq(i, true)->data&7;
                int hwsSpeed=(ins->gb.get_gb_hw_seq(i, true)->data&0x70)>>4;
                bool hwsDir=ins->gb.get_gb_hw_seq(i, true)->data&8;

                if (CWSliderInt("Shift",&hwsShift,0,7)) {
                  somethingChanged=true;
                }
                if (CWSliderInt("Speed",&hwsSpeed,0,7)) {
                  somethingChanged=true;
                }

                if (ImGui::RadioButton("Up",!hwsDir)) { PARAMETER
                  hwsDir=false;
                  somethingChanged=true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Down",hwsDir)) { PARAMETER
                  hwsDir=true;
                  somethingChanged=true;
                }

                if (somethingChanged) {
                  ins->gb.get_gb_hw_seq(i, true)->data=(hwsShift&7)|(hwsDir?8:0)|(hwsSpeed<<4);
                  PARAMETER;
                }
                break;
              }
              case DivInstrumentGB::DIV_GB_HWCMD_WAIT: {
                int len=ins->gb.get_gb_hw_seq(i, true)->data+1;
                curFrame+=ins->gb.get_gb_hw_seq(i, true)->data+1;

                if (ImGui::InputInt("Ticks",&len,1,4)) {
                  if (len<1) len=1;
                  if (len>255) len=256;
                  somethingChanged=true;
                }

                if (somethingChanged) {
                  ins->gb.get_gb_hw_seq(i, true)->data=len-1;
                  PARAMETER;
                }
                break;
              }
              case DivInstrumentGB::DIV_GB_HWCMD_WAIT_REL:
                curFrame++;
                break;
              case DivInstrumentGB::DIV_GB_HWCMD_LOOP:
              case DivInstrumentGB::DIV_GB_HWCMD_LOOP_REL: {
                int pos=ins->gb.get_gb_hw_seq(i, true)->data;

                if (ImGui::InputInt("Position",&pos,1,1)) {
                  if (pos<0) pos=0;
                  if (pos>(ins->gb.hwSeqLen-1)) pos=(ins->gb.hwSeqLen-1);
                  somethingChanged=true;
                }

                if (somethingChanged) {
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
                e->lockEngine([ins,i]() {
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
              if (i<ins->gb.hwSeqLen-1) {
                e->lockEngine([ins,i]() {
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
            if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) {
              for (int j=i; j<ins->gb.hwSeqLen-1; j++) {
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

        if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) {
          if (ins->gb.hwSeqLen<255) {
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
    if (ins->type==DIV_INS_C64) if (ImGui::BeginTabItem("C64")) {
      ImGui::AlignTextToFramePadding();
      ImGui::Text("Waveform");
      ImGui::SameLine();
      pushToggleColors(ins->c64.triOn);
      if (ImGui::Button("tri")) { PARAMETER
        ins->c64.triOn=!ins->c64.triOn;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->c64.sawOn);
      if (ImGui::Button("saw")) { PARAMETER
        ins->c64.sawOn=!ins->c64.sawOn;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->c64.pulseOn);
      if (ImGui::Button("pulse")) { PARAMETER
        ins->c64.pulseOn=!ins->c64.pulseOn;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->c64.noiseOn);
      if (ImGui::Button("noise")) { PARAMETER
        ins->c64.noiseOn=!ins->c64.noiseOn;
      }
      popToggleColors();

      ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);

      if (ImGui::BeginTable("C64EnvParams",5,ImGuiTableFlags_NoHostExtendX)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
        ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        CENTER_TEXT("A");
        ImGui::TextUnformatted("A");
        ImGui::TableNextColumn();
        CENTER_TEXT("D");
        ImGui::TextUnformatted("D");
        ImGui::TableNextColumn();
        CENTER_TEXT("S");
        ImGui::TextUnformatted("S");
        ImGui::TableNextColumn();
        CENTER_TEXT("R");
        ImGui::TextUnformatted("R");
        ImGui::TableNextColumn();
        CENTER_TEXT("Envelope");
        ImGui::TextUnformatted("Envelope");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->c64.a,&_ZERO,&_FIFTEEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->c64.d,&_ZERO,&_FIFTEEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->c64.s,&_ZERO,&_FIFTEEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->c64.r,&_ZERO,&_FIFTEEN)); rightClickable
        ImGui::TableNextColumn();
        drawFMEnv(0,16-ins->c64.a,16-ins->c64.d,15-ins->c64.r,15-ins->c64.r,15-ins->c64.s,0,0,0,15,16,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

        ImGui::EndTable();
      }

      P(CWSliderScalar("Duty",ImGuiDataType_U16,&ins->c64.duty,&_ZERO,&_FOUR_THOUSAND_NINETY_FIVE)); rightClickable

      bool ringMod=ins->c64.ringMod;
      if (ImGui::Checkbox("Ring Modulation",&ringMod)) { PARAMETER
        ins->c64.ringMod=ringMod;
      }
      bool oscSync=ins->c64.oscSync;
      if (ImGui::Checkbox("Oscillator Sync",&oscSync)) { PARAMETER
        ins->c64.oscSync=oscSync;
      }

      P(ImGui::Checkbox("Enable filter",&ins->c64.toFilter));
      P(ImGui::Checkbox("Initialize filter",&ins->c64.initFilter));
      
      P(CWSliderScalar("Cutoff",ImGuiDataType_U16,&ins->c64.cut,&_ZERO,&_TWO_THOUSAND_FORTY_SEVEN)); rightClickable
      P(CWSliderScalar("Resonance",ImGuiDataType_U8,&ins->c64.res,&_ZERO,&_FIFTEEN)); rightClickable

      ImGui::AlignTextToFramePadding();
      ImGui::Text("Filter Mode");
      ImGui::SameLine();
      pushToggleColors(ins->c64.lp);
      if (ImGui::Button("low")) { PARAMETER
        ins->c64.lp=!ins->c64.lp;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->c64.bp);
      if (ImGui::Button("band")) { PARAMETER
        ins->c64.bp=!ins->c64.bp;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->c64.hp);
      if (ImGui::Button("high")) { PARAMETER
        ins->c64.hp=!ins->c64.hp;
      }
      popToggleColors();
      ImGui::SameLine();
      pushToggleColors(ins->c64.ch3off);
      if (ImGui::Button("ch3off")) { PARAMETER
        ins->c64.ch3off=!ins->c64.ch3off;
      }
      popToggleColors();

      if (ImGui::Checkbox("Absolute Cutoff Macro",&ins->c64.filterIsAbs)) {
        ins->std.get_macro(DIV_MACRO_ALG, true)->vZoom=-1;
        PARAMETER;
      }
      if (ImGui::Checkbox("Absolute Duty Macro",&ins->c64.dutyIsAbs)) {
        ins->std.get_macro(DIV_MACRO_DUTY, true)->vZoom=-1;
        PARAMETER;
      }
      P(ImGui::Checkbox("Don't test before new note",&ins->c64.noTest));
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_ES5503) if (ImGui::BeginTabItem("ES5503")) {
      ImGui::AlignTextToFramePadding();
      ImGui::Text("Oscillator mode:");
      ImGui::SameLine();
      bool freerun = (ins->es5503.initial_osc_mode == OSC_MODE_FREERUN);
      bool oneshot = (ins->es5503.initial_osc_mode == OSC_MODE_ONESHOT);
      bool sync_am = (ins->es5503.initial_osc_mode == OSC_MODE_SYNC_OR_AM);
      bool swap = (ins->es5503.initial_osc_mode == OSC_MODE_SWAP);
      if (ImGui::RadioButton("Freerun",freerun)) { PARAMETER
        freerun=true;
        oneshot=false;
        sync_am=false;
        swap=false;
        ins->es5503.initial_osc_mode=OSC_MODE_FREERUN;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Oneshot",oneshot)) { PARAMETER
        freerun=false;
        oneshot=true;
        sync_am=false;
        swap=false;
        ins->es5503.initial_osc_mode=OSC_MODE_ONESHOT;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Sync/AM",sync_am)) { PARAMETER
        freerun=false;
        oneshot=false;
        sync_am=true;
        swap=false;
        ins->es5503.initial_osc_mode=OSC_MODE_SYNC_OR_AM;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Swap",swap)) { PARAMETER
        freerun=false;
        oneshot=false;
        sync_am=true;
        swap=false;
        ins->es5503.initial_osc_mode=OSC_MODE_SWAP;
      }

      P(ImGui::Checkbox("Virtual softpan channel",&ins->es5503.softpan_virtual_channel));
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Combines odd and next even channel into one virtual channel with 256-step panning.\nInstrument, volume and effects need to be placed on the odd channel (e.g. 1st, 3rd, 5th etc.)");
      }

      P(ImGui::Checkbox("Phase reset on key-on",&ins->es5503.phase_reset_on_start));
      
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_SU) if (ImGui::BeginTabItem("Sound Unit")) {
      P(ImGui::Checkbox("Switch roles of frequency and phase reset timer",&ins->su.switchRoles));
      if (ImGui::BeginChild("HWSeqSU",ImGui::GetContentRegionAvail(),true,ImGuiWindowFlags_MenuBar)) {
        ImGui::BeginMenuBar();
        ImGui::Text("Hardware Sequence");
        ImGui::EndMenuBar();

        if (ins->su.hwSeqLen>0) if (ImGui::BeginTable("HWSeqListSU",3)) {
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
          for (int i=0; i<ins->su.hwSeqLen; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d (#%d)",curFrame,i);
            ImGui::TableNextColumn();
            ImGui::PushID(i);
            if (ins->su.get_su_hw_seq(i, true)->cmd>=DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX) {
              ins->su.get_su_hw_seq(i, true)->cmd=0;
            }
            int cmd=ins->su.get_su_hw_seq(i, true)->cmd;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::Combo("##HWSeqCmd",&cmd,suHWSeqCmdTypes,DivInstrumentSoundUnit::DIV_SU_HWCMD_MAX)) {
              if (ins->su.get_su_hw_seq(i, true)->cmd!=cmd) {
                ins->su.get_su_hw_seq(i, true)->cmd=cmd;
                ins->su.get_su_hw_seq(i, true)->val=0;
                ins->su.get_su_hw_seq(i, true)->bound=0;
                ins->su.get_su_hw_seq(i, true)->speed=0;
              }
            }
            bool somethingChanged=false;
            switch (ins->su.get_su_hw_seq(i, true)->cmd) {
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_VOL: {
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
                if (CWSliderInt("Amount",&swVal,0,31)) {
                  somethingChanged=true;
                }
                if (CWSliderInt("Bound",&swBound,0,255)) {
                  somethingChanged=true;
                }
                if (ImGui::RadioButton("Up",swDir)) { PARAMETER
                  swDir=true;
                  somethingChanged=true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Down",!swDir)) { PARAMETER
                  swDir=false;
                  somethingChanged=true;
                }
                if (ImGui::Checkbox("Loop",&swLoop)) { PARAMETER
                  somethingChanged=true;
                }
                ImGui::SameLine();
                if (ImGui::Checkbox("Flip",&swInvert)) { PARAMETER
                  somethingChanged=true;
                }

                if (somethingChanged) {
                  ins->su.get_su_hw_seq(i, true)->speed=swPeriod;
                  ins->su.get_su_hw_seq(i, true)->bound=swBound;
                  ins->su.get_su_hw_seq(i, true)->val=(swVal&31)|(swDir?32:0)|(swLoop?64:0)|(swInvert?128:0);
                  PARAMETER;
                }
                break;
              }
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_PITCH:
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_CUT: {
                int swPeriod=ins->su.get_su_hw_seq(i, true)->speed;
                int swBound=ins->su.get_su_hw_seq(i, true)->bound;
                int swVal=ins->su.get_su_hw_seq(i, true)->val&127;
                bool swDir=ins->su.get_su_hw_seq(i, true)->val&128;

                if (ImGui::InputInt("Period",&swPeriod,1,16)) {
                  if (swPeriod<0) swPeriod=0;
                  if (swPeriod>65535) swPeriod=65535;
                  somethingChanged=true;
                }
                if (CWSliderInt("Amount",&swVal,0,31)) {
                  somethingChanged=true;
                }
                if (CWSliderInt("Bound",&swBound,0,255)) {
                  somethingChanged=true;
                }
                if (ImGui::RadioButton("Up",swDir)) { PARAMETER
                  swDir=true;
                  somethingChanged=true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Down",!swDir)) { PARAMETER
                  swDir=false;
                  somethingChanged=true;
                }

                if (somethingChanged) {
                  ins->su.get_su_hw_seq(i, true)->speed=swPeriod;
                  ins->su.get_su_hw_seq(i, true)->bound=swBound;
                  ins->su.get_su_hw_seq(i, true)->val=(swVal&127)|(swDir?128:0);
                  PARAMETER;
                }
                break;
              }
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT: {
                int len=ins->su.get_su_hw_seq(i, true)->val+1;
                curFrame+=ins->su.get_su_hw_seq(i, true)->val+1;

                if (ImGui::InputInt("Ticks",&len)) {
                  if (len<1) len=1;
                  if (len>255) len=256;
                  somethingChanged=true;
                }

                if (somethingChanged) {
                  ins->su.get_su_hw_seq(i, true)->val=len-1;
                  PARAMETER;
                }
                break;
              }
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_WAIT_REL:
                curFrame++;
                break;
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP:
              case DivInstrumentSoundUnit::DIV_SU_HWCMD_LOOP_REL: {
                int pos=ins->su.get_su_hw_seq(i, true)->val;

                if (ImGui::InputInt("Position",&pos,1,4)) {
                  if (pos<0) pos=0;
                  if (pos>(ins->su.hwSeqLen-1)) pos=(ins->su.hwSeqLen-1);
                  somethingChanged=true;
                }

                if (somethingChanged) {
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
            if (ImGui::Button(ICON_FA_CHEVRON_UP "##HWCmdUp")) {
              if (i>0) {
                e->lockEngine([ins,i]() {
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
            if (ImGui::Button(ICON_FA_CHEVRON_DOWN "##HWCmdDown")) {
              if (i<ins->su.hwSeqLen-1) {
                e->lockEngine([ins,i]() {
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
            if (ImGui::Button(ICON_FA_TIMES "##HWCmdDel")) {
              for (int j=i; j<ins->su.hwSeqLen-1; j++) {
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

        if (ImGui::Button(ICON_FA_PLUS "##HWCmdAdd")) {
          if (ins->su.hwSeqLen<255) {
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
    if (ins->type==DIV_INS_MSM6258 ||
        ins->type==DIV_INS_MSM6295 ||
        ins->type==DIV_INS_ADPCMA ||
        ins->type==DIV_INS_ADPCMB ||
        ins->type==DIV_INS_SEGAPCM ||
        ins->type==DIV_INS_QSOUND ||
        ins->type==DIV_INS_YMZ280B ||
        ins->type==DIV_INS_RF5C68 ||
        ins->type==DIV_INS_AMIGA ||
        ins->type==DIV_INS_MULTIPCM ||
        ins->type==DIV_INS_SU ||
        ins->type==DIV_INS_SNES ||
        ins->type==DIV_INS_ES5506 ||
        ins->type==DIV_INS_K007232 ||
        ins->type==DIV_INS_GA20 ||
        ins->type==DIV_INS_K053260 ||
        ins->type==DIV_INS_C140 ||
        ins->type==DIV_INS_C219) {
      insTabSample(ins);
    }
    if (ins->type==DIV_INS_N163) if (ImGui::BeginTabItem("Namco 163")) {
      bool preLoad=ins->n163.waveMode&0x1;
      if (ImGui::Checkbox("Load waveform",&preLoad)) { PARAMETER
        ins->n163.waveMode=(ins->n163.waveMode&~0x1)|(preLoad?0x1:0);
      }

      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("when enabled, a waveform will be loaded into RAM.\nwhen disabled, only the offset and length change.");
      }

      if (preLoad) {
        if (ImGui::InputInt("Waveform##WAVE",&ins->n163.wave,1,10)) { PARAMETER
          if (ins->n163.wave<0) ins->n163.wave=0;
          if (ins->n163.wave>=e->song.waveLen) ins->n163.wave=e->song.waveLen-1;
        }
      }

      ImGui::Separator();

      P(ImGui::Checkbox("Per-channel wave position/length",&ins->n163.perChanPos));

      if (ins->n163.perChanPos) {
        if (ImGui::BeginTable("N1PerChPos",3)) {
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

          for (int i=0; i<8; i++) {
            ImGui::PushID(64+i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Dummy(ImVec2(dpiScale,ImGui::GetFrameHeightWithSpacing()));
            ImGui::SameLine();
            ImGui::Text("%d",i+1);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##pcOff",&ins->n163.wavePosCh[i],1,16)) { PARAMETER
              if (ins->n163.wavePosCh[i]<0) ins->n163.wavePosCh[i]=0;
              if (ins->n163.wavePosCh[i]>255) ins->n163.wavePosCh[i]=255;
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##pcLen",&ins->n163.waveLenCh[i],4,16)) { PARAMETER
              if (ins->n163.waveLenCh[i]<0) ins->n163.waveLenCh[i]=0;
              if (ins->n163.waveLenCh[i]>252) ins->n163.waveLenCh[i]=252;
              ins->n163.waveLenCh[i]&=0xfc;
            }
            ImGui::PopID();
          }

          ImGui::EndTable();
        }
      } else {
        if (ImGui::InputInt("Position##WAVEPOS",&ins->n163.wavePos,1,16)) { PARAMETER
          if (ins->n163.wavePos<0) ins->n163.wavePos=0;
          if (ins->n163.wavePos>255) ins->n163.wavePos=255;
        }
        if (ImGui::InputInt("Length##WAVELEN",&ins->n163.waveLen,4,16)) { PARAMETER
          if (ins->n163.waveLen<0) ins->n163.waveLen=0;
          if (ins->n163.waveLen>252) ins->n163.waveLen=252;
          ins->n163.waveLen&=0xfc;
        }
      }
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_FDS) if (ImGui::BeginTabItem("FDS")) {
      float modTable[32];
      int modTableInt[256];
      ImGui::Checkbox("Compatibility mode",&ins->fds.initModTableWithFirstWave);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("only use for compatibility with .dmf modules!\n- initializes modulation table with first wavetable\n- does not alter modulation parameters on instrument change");
      }
      if (ImGui::InputInt("Modulation depth",&ins->fds.modDepth,1,4)) {
        if (ins->fds.modDepth<0) ins->fds.modDepth=0;
        if (ins->fds.modDepth>63) ins->fds.modDepth=63;
      }
      if (ImGui::InputInt("Modulation speed",&ins->fds.modSpeed,1,4)) {
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
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
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
      if (ImGui::InputText("##MMLModTable",&mmlStringModTable)) {
        int discardIt=0;
        memset(modTableInt,0,256*sizeof(int));
        decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,-4,3,false);
        for (int i=0; i<32; i++) {
          if (i>=discardIt) {
            modTableInt[i]=0;
          } else {
            if (modTableInt[i]<-4) modTableInt[i]=-4;
            if (modTableInt[i]>3) modTableInt[i]=3;
          }
          ins->fds.modTable[i]=modTableInt[i];
        }
        MARK_MODIFIED;
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,false);
      }
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_VBOY) if (ImGui::BeginTabItem("Virtual Boy")) {
      float modTable[32];
      int modTableInt[256];
      P(ImGui::Checkbox("Set modulation table (channel 5 only)",&ins->fds.initModTableWithFirstWave));

      ImGui::BeginDisabled(!ins->fds.initModTableWithFirstWave);
      for (int i=0; i<32; i++) {
        modTable[i]=ins->fds.modTable[i];
        modTableInt[i]=modTableHex?((unsigned char)ins->fds.modTable[i]):ins->fds.modTable[i];
      }
      ImVec2 modTableSize=ImVec2(ImGui::GetContentRegionAvail().x,256.0f*dpiScale);
      PlotCustom("ModTable",modTable,32,0,NULL,-128,127,modTableSize,sizeof(float),ImVec4(1.0f,1.0f,1.0f,1.0f),0,NULL,NULL,true);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
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
      
      if (ImGui::Button(modTableHex?"Hex##MTHex":"Dec##MTHex")) {
        modTableHex=!modTableHex;
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x); // wavetable text input size found here
      if (ImGui::InputText("##MMLModTable",&mmlStringModTable)) {
        int discardIt=0;
        memset(modTableInt,0,256*sizeof(int));
        decodeMMLStrW(mmlStringModTable,modTableInt,discardIt,modTableHex?0:-128,modTableHex?255:127,modTableHex);
        for (int i=0; i<32; i++) {
          if (i>=discardIt) {
            modTableInt[i]=0;
          } else {
            if (modTableInt[i]>=128) modTableInt[i]-=256;
          }
          ins->fds.modTable[i]=modTableInt[i];
        }
        MARK_MODIFIED;
      }
      if (!ImGui::IsItemActive()) {
        encodeMMLStr(mmlStringModTable,modTableInt,32,-1,-1,modTableHex);
      }
      ImGui::SameLine();

      ImGui::EndDisabled();
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_ES5506) if (ImGui::BeginTabItem("ES5506")) {
      if (ImGui::BeginTable("ESParams",2,ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
        // filter
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWSliderScalar("Filter Mode",ImGuiDataType_U8,&ins->es5506.filter.mode,&_ZERO,&_THREE,es5506FilterModes[ins->es5506.filter.mode&3]));
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWSliderScalar("Filter K1",ImGuiDataType_U16,&ins->es5506.filter.k1,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
        ImGui::TableNextColumn();
        P(CWSliderScalar("Filter K2",ImGuiDataType_U16,&ins->es5506.filter.k2,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
        // envelope
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWSliderScalar("Envelope count",ImGuiDataType_U16,&ins->es5506.envelope.ecount,&_ZERO,&_FIVE_HUNDRED_ELEVEN)); rightClickable
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWSliderScalar("Left Volume Ramp",ImGuiDataType_S8,&ins->es5506.envelope.lVRamp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWSliderScalar("Right Volume Ramp",ImGuiDataType_S8,&ins->es5506.envelope.rVRamp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        P(CWSliderScalar("Filter K1 Ramp",ImGuiDataType_S8,&ins->es5506.envelope.k1Ramp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
        ImGui::TableNextColumn();
        P(CWSliderScalar("Filter K2 Ramp",ImGuiDataType_S8,&ins->es5506.envelope.k2Ramp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Checkbox("K1 Ramp Slowdown",&ins->es5506.envelope.k1Slow);
        ImGui::TableNextColumn();
        ImGui::Checkbox("K2 Ramp Slowdown",&ins->es5506.envelope.k2Slow);
        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_MULTIPCM) {
      if (ImGui::BeginTabItem("MultiPCM")) {
        ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);
        if (ImGui::BeginTable("MultiPCMADSRParams",7,ImGuiTableFlags_NoHostExtendX)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthStretch);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          CENTER_TEXT("AR");
          ImGui::TextUnformatted("AR");
          if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Attack Rate");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("D1R");
          ImGui::TextUnformatted("D1R");
          if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Decay 1 Rate");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("DL");
          ImGui::TextUnformatted("DL");
          if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Decay Level");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("D2R");
          ImGui::TextUnformatted("D2R");
          if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Decay 2 Rate");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("RR");
          ImGui::TextUnformatted("RR");
          if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Release Rate");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("RC");
          ImGui::TextUnformatted("RC");
          if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Rate Correction");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("Envelope");
          ImGui::TextUnformatted("Envelope");

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Attack Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.ar,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Decay 1 Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.d1r,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Decay Level",sliderSize,ImGuiDataType_U8,&ins->multipcm.dl,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Decay 2 Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.d2r,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Release Rate",sliderSize,ImGuiDataType_U8,&ins->multipcm.rr,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Rate Correction",sliderSize,ImGuiDataType_U8,&ins->multipcm.rc,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          drawFMEnv(0,ins->multipcm.ar,ins->multipcm.d1r,ins->multipcm.d2r,ins->multipcm.rr,ins->multipcm.dl,0,0,0,127,15,15,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);
          ImGui::EndTable();
        }
        if (ImGui::BeginTable("MultiPCMLFOParams",3,ImGuiTableFlags_SizingStretchSame)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.0);
          ImGui::TableNextColumn();
          P(CWSliderScalar("LFO Rate",ImGuiDataType_U8,&ins->multipcm.lfo,&_ZERO,&_SEVEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWSliderScalar("PM Depth",ImGuiDataType_U8,&ins->multipcm.vib,&_ZERO,&_SEVEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWSliderScalar("AM Depth",ImGuiDataType_U8,&ins->multipcm.am,&_ZERO,&_SEVEN)); rightClickable
          ImGui::EndTable();
        }
        ImGui::EndTabItem();
      }
    }
    if (ins->type==DIV_INS_SNES) if (ImGui::BeginTabItem("SNES")) {
      P(ImGui::Checkbox("Use envelope",&ins->snes.useEnv));
      ImVec2 sliderSize=ImVec2(20.0f*dpiScale,128.0*dpiScale);
      if (ins->snes.useEnv) {
        if (ImGui::BeginTable("SNESEnvParams",ins->snes.sus?6:5,ImGuiTableFlags_NoHostExtendX)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          if (ins->snes.sus) {
            ImGui::TableSetupColumn("c2x",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          }
          ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);
          ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          CENTER_TEXT("A");
          ImGui::TextUnformatted("A");
          ImGui::TableNextColumn();
          CENTER_TEXT("D");
          ImGui::TextUnformatted("D");
          ImGui::TableNextColumn();
          CENTER_TEXT("S");
          ImGui::TextUnformatted("S");
          if (ins->snes.sus) {
            ImGui::TableNextColumn();
            CENTER_TEXT("D2");
            ImGui::TextUnformatted("D2");
          }
          ImGui::TableNextColumn();
          CENTER_TEXT("R");
          ImGui::TextUnformatted("R");
          ImGui::TableNextColumn();
          CENTER_TEXT("Envelope");
          ImGui::TextUnformatted("Envelope");

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Attack",sliderSize,ImGuiDataType_U8,&ins->snes.a,&_ZERO,&_FIFTEEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Decay",sliderSize,ImGuiDataType_U8,&ins->snes.d,&_ZERO,&_SEVEN)); rightClickable
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Sustain",sliderSize,ImGuiDataType_U8,&ins->snes.s,&_ZERO,&_SEVEN)); rightClickable
          if (ins->snes.sus) {
            ImGui::TableNextColumn();
            P(CWVSliderScalar("##Decay2",sliderSize,ImGuiDataType_U8,&ins->snes.d2,&_ZERO,&_THIRTY_ONE)); rightClickable
          }
          ImGui::TableNextColumn();
          P(CWVSliderScalar("##Release",sliderSize,ImGuiDataType_U8,&ins->snes.r,&_ZERO,&_THIRTY_ONE)); rightClickable
          ImGui::TableNextColumn();
          drawFMEnv(0,ins->snes.a+1,1+ins->snes.d*2,ins->snes.sus?ins->snes.d2:ins->snes.r,ins->snes.sus?ins->snes.r:31,(14-ins->snes.s*2),(ins->snes.r==0 || (ins->snes.sus && ins->snes.d2==0)),0,0,7,16,31,ImVec2(ImGui::GetContentRegionAvail().x,sliderSize.y),ins->type);

          ImGui::EndTable();
        }
        ImGui::Text("Sustain/release mode:");
        if (ImGui::RadioButton("Direct (cut on release)",ins->snes.sus==0)) {
          ins->snes.sus=0;
        }
        if (ImGui::RadioButton("Effective (linear decrease)",ins->snes.sus==1)) {
          ins->snes.sus=1;
        }
        if (ImGui::RadioButton("Effective (exponential decrease)",ins->snes.sus==2)) {
          ins->snes.sus=2;
        }
        if (ImGui::RadioButton("Delayed (write R on release)",ins->snes.sus==3)) {
          ins->snes.sus=3;
        }
      } else {
        if (ImGui::BeginTable("SNESGainParams",2,ImGuiTableFlags_NoHostExtendX)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,sliderSize.x);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          CENTER_TEXT("Gain Mode");
          ImGui::TextUnformatted("Gain Mode");
          ImGui::TableNextColumn();
          CENTER_TEXT("Gain");
          ImGui::TextUnformatted("Gain");

          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          if (ImGui::RadioButton("Direct",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT)) {
            ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DIRECT;
            PARAMETER;
          }
          if (ImGui::RadioButton("Decrease (linear)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LINEAR)) {
            ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LINEAR;
            PARAMETER;
          }
          if (ImGui::RadioButton("Decrease (logarithmic)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LOG)) {
            ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LOG;
            PARAMETER;
          }
          if (ImGui::RadioButton("Increase (linear)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_INC_LINEAR)) {
            ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_INC_LINEAR;
            PARAMETER;
          }
          if (ImGui::RadioButton("Increase (bent line)",ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_INC_INVLOG)) {
            ins->snes.gainMode=DivInstrumentSNES::GAIN_MODE_INC_INVLOG;
            PARAMETER;
          }

          ImGui::TableNextColumn();
          unsigned char gainMax=(ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT)?127:31;
          if (ins->snes.gain>gainMax) ins->snes.gain=gainMax;
          P(CWVSliderScalar("##Gain",sliderSize,ImGuiDataType_U8,&ins->snes.gain,&_ZERO,&gainMax)); rightClickable

          ImGui::EndTable();
        }
        if (ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LINEAR || ins->snes.gainMode==DivInstrumentSNES::GAIN_MODE_DEC_LOG) {
          ImGui::TextWrapped("using decrease modes will not produce any sound at all, unless you know what you are doing.\nit is recommended to use the Gain macro for decrease instead.");
        }
      }
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_GB ||
        (ins->type==DIV_INS_AMIGA && ins->amiga.useWave) ||
        (ins->type==DIV_INS_X1_010 && !ins->amiga.useSample) ||
        ins->type==DIV_INS_N163 ||
        ins->type==DIV_INS_FDS ||
        (ins->type==DIV_INS_SWAN && !ins->amiga.useSample) ||
        (ins->type==DIV_INS_PCE && !ins->amiga.useSample) ||
        (ins->type==DIV_INS_ES5503 && !ins->amiga.useSample) ||
        (ins->type==DIV_INS_VBOY) ||
        ins->type==DIV_INS_SCC ||
        ins->type==DIV_INS_SNES ||
        ins->type==DIV_INS_NAMCO ||
        ins->type==DIV_INS_SM8521) {
      if (ImGui::BeginTabItem("Wavetable")) {
        switch (ins->type) {
          case DIV_INS_GB:
          case DIV_INS_NAMCO:
          case DIV_INS_SM8521:
          case DIV_INS_SWAN:
            wavePreviewLen=32;
            wavePreviewHeight=15;
            break;
          case DIV_INS_PCE:
            wavePreviewLen=32;
            wavePreviewHeight=31;
            break;
          case DIV_INS_VBOY:
            wavePreviewLen=32;
            wavePreviewHeight=63;
            break;
          case DIV_INS_SCC:
            wavePreviewLen=32;
            wavePreviewHeight=255;
            break;
          case DIV_INS_FDS:
            wavePreviewLen=64;
            wavePreviewHeight=63;
            break;
          case DIV_INS_N163:
            wavePreviewLen=ins->n163.waveLen;
            wavePreviewHeight=15;
            break;
          case DIV_INS_X1_010:
            wavePreviewLen=128;
            wavePreviewHeight=255;
            break;
          case DIV_INS_AMIGA:
            wavePreviewLen=ins->amiga.waveLen+1;
            wavePreviewHeight=255;
            break;
          case DIV_INS_SNES:
            wavePreviewLen=ins->amiga.waveLen+1;
            wavePreviewHeight=15;
            break;
          case DIV_INS_ES5503:
            wavePreviewLen=256;
            wavePreviewHeight=255;
            break;
          default:
            wavePreviewLen=32;
            wavePreviewHeight=31;
            break;
        }
        if (ImGui::Checkbox("Enable synthesizer",&ins->ws.enabled)) {
          wavePreviewInit=true;
        }
        if (ins->ws.enabled) {
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ins->ws.effect&0x80) {
            if ((ins->ws.effect&0x7f)>=DIV_WS_DUAL_MAX) {
              ins->ws.effect=0;
              wavePreviewInit=true;
            }
          } else {
            if ((ins->ws.effect&0x7f)>=DIV_WS_SINGLE_MAX) {
              ins->ws.effect=0;
              wavePreviewInit=true;
            }
          }
          if (ImGui::BeginCombo("##WSEffect",(ins->ws.effect&0x80)?dualWSEffects[ins->ws.effect&0x7f]:singleWSEffects[ins->ws.effect&0x7f])) {
            ImGui::Text("Single-waveform");
            ImGui::Indent();
            for (int i=0; i<DIV_WS_SINGLE_MAX; i++) {
              if (ImGui::Selectable(singleWSEffects[i])) {
                ins->ws.effect=i;
                wavePreviewInit=true;
              }
            }
            ImGui::Unindent();
            ImGui::Text("Dual-waveform");
            ImGui::Indent();
            for (int i=129; i<DIV_WS_DUAL_MAX; i++) {
              if (ImGui::Selectable(dualWSEffects[i-128])) {
                ins->ws.effect=i;
                wavePreviewInit=true;
              }
            }
            ImGui::Unindent();
            ImGui::EndCombo();
          }
          const bool isSingleWaveFX=(ins->ws.effect>=128);
          if (ImGui::BeginTable("WSPreview",isSingleWaveFX?3:2)) {
            DivWavetable* wave1=e->getWave(ins->ws.wave1);
            DivWavetable* wave2=e->getWave(ins->ws.wave2);
            if (wavePreviewInit) {
              wavePreview.init(ins,wavePreviewLen,wavePreviewHeight,true);
              wavePreviewInit=false;
            }
            float wavePreview1[257];
            float wavePreview2[257];
            float wavePreview3[257];
            for (int i=0; i<wave1->len; i++) {
              if (wave1->data[i]>wave1->max) {
                wavePreview1[i]=wave1->max;
              } else {
                wavePreview1[i]=wave1->data[i];
              }
            }
            if (wave1->len>0) {
              wavePreview1[wave1->len]=wave1->data[wave1->len-1];
            }
            for (int i=0; i<wave2->len; i++) {
              if (wave2->data[i]>wave2->max) {
                wavePreview2[i]=wave2->max;
              } else {
                wavePreview2[i]=wave2->data[i];
              }
            }
            if (wave2->len>0) {
              wavePreview2[wave2->len]=wave2->data[wave2->len-1];
            }
            if (ins->ws.enabled && (!wavePreviewPaused || wavePreviewInit)) {
              wavePreview.tick(true);
              WAKE_UP;
            }
            for (int i=0; i<wavePreviewLen; i++) {
              wavePreview3[i]=wavePreview.output[i];
            }
            if (wavePreviewLen>0) {
              wavePreview3[wavePreviewLen]=wavePreview3[wavePreviewLen-1];
            }

            float ySize=(isSingleWaveFX?96.0f:128.0f)*dpiScale;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImVec2 size1=ImVec2(ImGui::GetContentRegionAvail().x,ySize);
            PlotNoLerp("##WaveformP1",wavePreview1,wave1->len+1,0,"Wave 1",0,wave1->max,size1);
            if (isSingleWaveFX) {
              ImGui::TableNextColumn();
              ImVec2 size2=ImVec2(ImGui::GetContentRegionAvail().x,ySize);
              PlotNoLerp("##WaveformP2",wavePreview2,wave2->len+1,0,"Wave 2",0,wave2->max,size2);
            }
            ImGui::TableNextColumn();
            ImVec2 size3=ImVec2(ImGui::GetContentRegionAvail().x,ySize);
            PlotNoLerp("##WaveformP3",wavePreview3,wavePreviewLen+1,0,"Result",0,wavePreviewHeight,size3);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ins->std.get_macro(DIV_MACRO_WAVE, true)->len>0) {
              ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_WARNING]);
              ImGui::AlignTextToFramePadding();
              ImGui::Text("Wave 1 " ICON_FA_EXCLAMATION_TRIANGLE);
              ImGui::PopStyleColor();
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("waveform macro is controlling wave 1!\nthis value will be ineffective.");
              }
            } else {
              ImGui::AlignTextToFramePadding();
              ImGui::Text("Wave 1");
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::InputInt("##SelWave1",&ins->ws.wave1,1,4)) {
              if (ins->ws.wave1<0) ins->ws.wave1=0;
              if (ins->ws.wave1>=(int)e->song.wave.size()) ins->ws.wave1=e->song.wave.size()-1;
              wavePreviewInit=true;
            }
            if (ins->std.get_macro(DIV_MACRO_WAVE, true)->len>0) {
              if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("waveform macro is controlling wave 1!\nthis value will be ineffective.");
              }
            }
            if (isSingleWaveFX) {
              ImGui::TableNextColumn();
              ImGui::AlignTextToFramePadding();
              ImGui::Text("Wave 2");
              ImGui::SameLine();
              ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
              if (ImGui::InputInt("##SelWave2",&ins->ws.wave2,1,4)) {
                if (ins->ws.wave2<0) ins->ws.wave2=0;
                if (ins->ws.wave2>=(int)e->song.wave.size()) ins->ws.wave2=e->song.wave.size()-1;
                wavePreviewInit=true;
              }
            }
            ImGui::TableNextColumn();
            if (ImGui::Button(wavePreviewPaused?(ICON_FA_PLAY "##WSPause"):(ICON_FA_PAUSE "##WSPause"))) {
              wavePreviewPaused=!wavePreviewPaused;
            }
            if (ImGui::IsItemHovered()) {
              if (wavePreviewPaused) {
                ImGui::SetTooltip("Resume preview");
              } else {
                ImGui::SetTooltip("Pause preview");
              }
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_REPEAT "##WSRestart")) {
              wavePreviewInit=true;
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Restart preview");
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_UPLOAD "##WSCopy")) {
              curWave=e->addWave();
              if (curWave==-1) {
                showError("too many wavetables!");
              } else {
                wantScrollList=true;
                MARK_MODIFIED;
                RESET_WAVE_MACRO_ZOOM;
                nextWindow=GUI_WINDOW_WAVE_EDIT;

                DivWavetable* copyWave=e->song.wave[curWave];
                copyWave->len=wavePreviewLen;
                copyWave->max=wavePreviewHeight;
                memcpy(copyWave->data,wavePreview.output,256*sizeof(int));
              }
            }
            if (ImGui::IsItemHovered()) {
              ImGui::SetTooltip("Copy to new wavetable");
            }
            ImGui::SameLine();
            ImGui::Text("(%d×%d)",wavePreviewLen,wavePreviewHeight+1);
            ImGui::EndTable();
          }

          if (ImGui::InputScalar("Update Rate",ImGuiDataType_U8,&ins->ws.rateDivider,&_ONE,&_EIGHT)) {
            wavePreviewInit=true;
          }
          int speed=ins->ws.speed+1;
          if (ImGui::InputInt("Speed",&speed,1,8)) {
            if (speed<1) speed=1;
            if (speed>256) speed=256;
            ins->ws.speed=speed-1;
            wavePreviewInit=true;
          }

          if (ImGui::InputScalar("Amount",ImGuiDataType_U8,&ins->ws.param1,&_ONE,&_EIGHT)) {
            wavePreviewInit=true;
          }

          if (ins->ws.effect==DIV_WS_PHASE_MOD) {
            if (ImGui::InputScalar("Power",ImGuiDataType_U8,&ins->ws.param2,&_ONE,&_EIGHT)) {
              wavePreviewInit=true;
            }
          }

          if (ImGui::Checkbox("Global",&ins->ws.global)) {
            wavePreviewInit=true;
          }
        } else {
          ImGui::TextWrapped("wavetable synthesizer disabled.\nuse the Waveform macro to set the wave for this instrument.");
        }

        ImGui::EndTabItem();
      }
    }
    if (ins->type<DIV_INS_MAX) if (ImGui::BeginTabItem("Macros")) {
      //const char* volumeLabel="Volume";

      int volMax=15;
      int volMin=0;
      if (ins->type==DIV_INS_PCE || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SM8521) {
        volMax=31;
      }
      if (ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_VERA || ins->type==DIV_INS_VRC6_SAW || ins->type==DIV_INS_ESFM) {
        volMax=63;
      }
      if (ins->type==DIV_INS_AMIGA) {
        volMax=64;
      }
      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_SEGAPCM || ins->type==DIV_INS_MIKEY ||
          ins->type==DIV_INS_MULTIPCM || ins->type==DIV_INS_SU || ins->type==DIV_INS_OPZ ||
          ins->type==DIV_INS_OPM || ins->type==DIV_INS_SNES || ins->type==DIV_INS_MSM5232 ||
          ins->type==DIV_INS_K053260) {
        volMax=127;
      }
      if (ins->type==DIV_INS_GB) {
        if (ins->gb.softEnv) {
          volMax=15;
        } else {
          volMax=0;
        }
      }
      if (ins->type==DIV_INS_PET || ins->type==DIV_INS_BEEPER || ins->type==DIV_INS_PV1000) {
        volMax=1;
      }
      if (ins->type==DIV_INS_FDS) {
        volMax=32;
      }
      if (ins->type==DIV_INS_ES5506) {
        volMax=4095;
      }
      if (ins->type==DIV_INS_MSM6258) {
        volMax=0;
      }
      if (ins->type==DIV_INS_MSM6295 || ins->type==DIV_INS_TED) {
        volMax=8;
      }
      if (ins->type==DIV_INS_ADPCMA) {
        volMax=31;
      }
      if (ins->type==DIV_INS_ADPCMB || ins->type==DIV_INS_YMZ280B || ins->type==DIV_INS_RF5C68 ||
          ins->type==DIV_INS_GA20 || ins->type==DIV_INS_C140 || ins->type==DIV_INS_C219 || ins->type==DIV_INS_ES5503) {
        volMax=255;
      }
      if (ins->type==DIV_INS_QSOUND) {
        volMax=16383;
      }
      if (ins->type==DIV_INS_POKEMINI) {
        volMax=2;
      }

      //const char* dutyLabel="Duty/Noise";
      int dutyMin=0;
      int dutyMax=3;
      if (ins->type==DIV_INS_C64) {
        dutyLabel="Duty";
        if (ins->c64.dutyIsAbs) {
          dutyMax=4095;
        } else {
          dutyMin=-4095;
          dutyMax=4095;
        }
      }
      if (ins->type==DIV_INS_STD) {
        dutyLabel="Duty";
      }
      if (ins->type==DIV_INS_OPM || ins->type==DIV_INS_OPZ) {
        dutyMax=32;
      }
      if (ins->type==DIV_INS_AY) {
        dutyMax=ins->amiga.useSample?0:31;
      }
      if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_FM || ins->type==DIV_INS_OPM) {
        dutyLabel="Noise Freq";
      }
      if (ins->type==DIV_INS_POKEY) {
        dutyLabel="AUDCTL";
        dutyMax=8;
      }
      if (ins->type==DIV_INS_MIKEY) {
        dutyLabel="Duty/Int";
        dutyMax=ins->amiga.useSample?0:10;
      }
      if (ins->type==DIV_INS_MSM5232) {
        dutyLabel="Group Ctrl";
        dutyMax=5;
      }
      if (ins->type==DIV_INS_C219) {
        dutyLabel="Control";
        dutyMax=3;
      }
      if (ins->type==DIV_INS_BEEPER || ins->type==DIV_INS_POKEMINI) {
        dutyLabel="Pulse Width";
        dutyMax=255;
      }
      if (ins->type==DIV_INS_T6W28) {
        dutyLabel="Noise Type";
        dutyMax=1;
      }
      if (ins->type==DIV_INS_AY8930) {
        dutyMax=ins->amiga.useSample?0:255;
      }
      if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_AMIGA || ins->type==DIV_INS_SCC ||
          ins->type==DIV_INS_PET || ins->type==DIV_INS_SEGAPCM ||
          ins->type==DIV_INS_FM || ins->type==DIV_INS_K007232 || ins->type==DIV_INS_GA20 ||
          ins->type==DIV_INS_SM8521 || ins->type==DIV_INS_PV1000 || ins->type==DIV_INS_K053260 ||
          ins->type==DIV_INS_C140) {
        dutyMax=0;
      }
      if (ins->type==DIV_INS_VBOY) {
        dutyLabel="Noise Length";
        dutyMax=7;
      }
      if (ins->type==DIV_INS_PCE) {
        dutyLabel="Noise";
        dutyMax=(!ins->amiga.useSample)?1:0;
      }
      if (ins->type==DIV_INS_NAMCO) {
        dutyLabel="Noise";
        dutyMax=1;
      }
      if (ins->type==DIV_INS_VIC) {
        dutyLabel="On/Off";
        dutyMax=1;
      }
      if (ins->type==DIV_INS_TED) {
        dutyLabel="Square/Noise";
        dutyMax=2;
      }
      if (ins->type==DIV_INS_SWAN) {
        dutyLabel="Noise";
        dutyMax=ins->amiga.useSample?0:8;
      }
      if (ins->type==DIV_INS_SNES) {
        dutyLabel="Noise Freq";
        dutyMax=31;
      }
      if (ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS ||
          ins->type==DIV_INS_VRC6_SAW || ins->type==DIV_INS_FDS || ins->type==DIV_INS_MULTIPCM) {
        dutyMax=0;
      }
      if (ins->type==DIV_INS_VERA) {
        dutyLabel="Duty";
        dutyMax=63;
      }
      if (ins->type==DIV_INS_N163) {
        dutyLabel="Wave Pos";
        dutyMax=255;
      }
      if (ins->type==DIV_INS_VRC6) {
        dutyLabel="Duty";
        dutyMax=ins->amiga.useSample?0:7;
      }
      if (ins->type==DIV_INS_ES5506) {
        dutyLabel="Filter Mode";
        dutyMax=3;
      }
      if (ins->type==DIV_INS_SU) {
        dutyMax=127;
      }
      if (ins->type==DIV_INS_ES5506) {
        dutyLabel="Filter Mode";
        dutyMax=3;
      }
      if (ins->type==DIV_INS_MSM6258) {
        dutyLabel="Frequency Divider";
        dutyMax=2;
      }
      if (ins->type==DIV_INS_MSM6295) {
        dutyLabel="Frequency Divider";
        dutyMax=1;
      }
      if (ins->type==DIV_INS_ADPCMA) {
        dutyLabel="Global Volume";
        dutyMax=63;
      }
      if (ins->type==DIV_INS_ADPCMB || ins->type==DIV_INS_YMZ280B || ins->type==DIV_INS_RF5C68) {
        dutyMax=0;
      }
      if (ins->type==DIV_INS_QSOUND) {
        dutyLabel="Echo Level";
        dutyMax=32767;
      }
      if (ins->type==DIV_INS_ESFM) {
        dutyLabel="OP4 Noise Mode";
      }
      if (ins->type==DIV_INS_ES5503) {
        dutyLabel="Osc. mode";
        dutyMax=3;
      }

      //const char* waveLabel="Waveform";
      int waveMax=(ins->type==DIV_INS_VERA)?3:(MAX(1,e->song.waveLen-1));
      bool waveBitMode=false;
      if (ins->type==DIV_INS_C64 || ins->type==DIV_INS_SAA1099) {
        waveBitMode=true;
      }
      if (ins->type==DIV_INS_STD || ins->type==DIV_INS_VRC6_SAW || ins->type==DIV_INS_NES ||
          ins->type==DIV_INS_T6W28 || ins->type==DIV_INS_PV1000)
          waveMax=0;
      if (ins->type==DIV_INS_TIA || ins->type==DIV_INS_VIC || ins->type==DIV_INS_OPLL) waveMax=15;
      if (ins->type==DIV_INS_C64) waveMax=4;
      if (ins->type==DIV_INS_ES5503) waveMax=ins->amiga.useSample?0:255;
      if (ins->type==DIV_INS_SAA1099) waveMax=2;
      if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPM || ins->type==DIV_INS_ESFM) waveMax=0;
      if (ins->type==DIV_INS_MIKEY) waveMax=0;
      if (ins->type==DIV_INS_MULTIPCM) waveMax=0;
      if (ins->type==DIV_INS_ADPCMA) waveMax=0;
      if (ins->type==DIV_INS_ADPCMB) waveMax=0;
      if (ins->type==DIV_INS_QSOUND) waveMax=0;
      if (ins->type==DIV_INS_YMZ280B) waveMax=0;
      if (ins->type==DIV_INS_RF5C68) waveMax=0;
      if (ins->type==DIV_INS_MSM5232) waveMax=0;
      if (ins->type==DIV_INS_MSM6258) waveMax=0;
      if (ins->type==DIV_INS_MSM6295) waveMax=0;
      if (ins->type==DIV_INS_SEGAPCM) waveMax=0;
      if (ins->type==DIV_INS_K007232) waveMax=0;
      if (ins->type==DIV_INS_ES5506) waveMax=0;
      if (ins->type==DIV_INS_GA20) waveMax=0;
      if (ins->type==DIV_INS_K053260) waveMax=0;
      if (ins->type==DIV_INS_BEEPER) waveMax=0;
      if (ins->type==DIV_INS_POKEMINI) waveMax=0;
      if (ins->type==DIV_INS_TED) waveMax=0;
      if (ins->type==DIV_INS_C140) waveMax=0;
      if (ins->type==DIV_INS_C219) waveMax=0;
      if (ins->type==DIV_INS_SU || ins->type==DIV_INS_POKEY) waveMax=7;
      if (ins->type==DIV_INS_PET) {
        waveMax=8;
        waveBitMode=true;
      }
      if (ins->type==DIV_INS_VRC6) {
        waveMax=ins->amiga.useSample?(MAX(1,e->song.waveLen-1)):0;
      }
      
      if (ins->type==DIV_INS_OPLL) {
        waveLabel="Patch";
      }

      if (ins->type==DIV_INS_ES5503) {
        waveLabel="Wavetable";
      }

      if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930) {
        waveMax=ins->amiga.useSample?0:3;
        waveBitMode=ins->amiga.useSample?false:true;
      }

      const char** waveNames=NULL;
      if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || ins->type==DIV_INS_SAA1099) waveNames=ayShapeBits;
      if (ins->type==DIV_INS_C64) waveNames=c64ShapeBits;

      int ex1Max=(ins->type==DIV_INS_AY8930)?8:0;
      int ex2Max=(ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?4:0;
      bool ex2Bit=true;

      if (ins->type==DIV_INS_C64) {
        ex1Max=4;
        ex2Max=15;
      }
      if (ins->type==DIV_INS_X1_010) {
        dutyMax=0;
        ex1Max=ins->amiga.useSample?0:7;
        ex2Max=ins->amiga.useSample?0:255;
        ex2Bit=false;
      }
      if (ins->type==DIV_INS_N163) {
        ex1Max=252;
      }
      if (ins->type==DIV_INS_FDS) {
        ex1Max=63;
        ex2Max=4095;
      }
      if (ins->type==DIV_INS_SU) {
        ex1Max=16383;
        ex2Max=255;
      }
      if (ins->type==DIV_INS_MSM6258) {
        ex1Max=1;
      }
      if (ins->type==DIV_INS_QSOUND) {
        ex1Max=16383;
        ex2Max=2725;
      }
      if (ins->type==DIV_INS_SAA1099) ex1Max=8;
      if (ins->type==DIV_INS_ES5506) {
        ex1Max=65535;
        ex2Max=65535;
      }
      if (ins->type==DIV_INS_SNES) {
        ex1Max=5;
        ex2Max=255;
      }
      if (ins->type==DIV_INS_MSM5232) {
        ex1Max=5;
        ex2Max=11;
      }
      if (ins->type==DIV_INS_ES5503) {
        ex1Max=255;
        ex2Max=7;
      }

      int panMin=0;
      int panMax=0;
      bool panSingle=false;
      bool panSingleNoBit=false;
      if (ins->type==DIV_INS_STD ||//Game Gear
          ins->type==DIV_INS_FM ||
          ins->type==DIV_INS_OPM ||
          ins->type==DIV_INS_GB ||
          ins->type==DIV_INS_OPZ ||
          ins->type==DIV_INS_MSM6258 ||
          ins->type==DIV_INS_VERA ||
          ins->type==DIV_INS_ADPCMA ||
          ins->type==DIV_INS_ADPCMB ||
          ins->type==DIV_INS_ESFM) {
        panMax=2;
        panSingle=true;
      }
      if (ins->type==DIV_INS_OPL ||
          ins->type==DIV_INS_OPL_DRUMS) {
        panMax=4;
        panSingle=true;
      }
      if (ins->type==DIV_INS_X1_010 || ins->type==DIV_INS_PCE || ins->type==DIV_INS_MIKEY ||
          ins->type==DIV_INS_SAA1099 || ins->type==DIV_INS_NAMCO || ins->type==DIV_INS_RF5C68 ||
          ins->type==DIV_INS_VBOY || ins->type==DIV_INS_T6W28 || ins->type==DIV_INS_K007232) {
        panMax=15;
      }
      if (ins->type==DIV_INS_SEGAPCM) {
        panMax=127;
      }
      if (ins->type==DIV_INS_AMIGA) {
        if (ins->std.get_macro(DIV_MACRO_PAN_LEFT, true)->mode) {
          panMin=-16;
          panMax=16;
        } else {
          panMin=0;
          panMax=127;
        }
      }
      if (ins->type==DIV_INS_QSOUND) {
        panMin=-16;
        panMax=16;
        panSingleNoBit=true;
      }
      if (ins->type==DIV_INS_MULTIPCM || ins->type==DIV_INS_YMZ280B) {
        panMin=-7;
        panMax=7;
        panSingleNoBit=true;
      }
      if (ins->type==DIV_INS_K053260) {
        panMin=-3;
        panMax=3;
        panSingleNoBit=true;
      }
      if (ins->type==DIV_INS_SU) {
        panMin=-127;
        panMax=127;
        panSingleNoBit=true;
      }
      if (ins->type==DIV_INS_SNES) {
        panMin=0;
        panMax=127;
      }
      if (ins->type==DIV_INS_C140 || ins->type==DIV_INS_C219) {
        panMin=0;
        panMax=255;
      }
      if (ins->type==DIV_INS_ES5506) {
        panMax=4095;
      }
      if (ins->type==DIV_INS_ES5503 && ins->es5503.softpan_virtual_channel) {
        panMax=255;
      }

      if (volMax>0) {
        macroList.push_back(FurnaceGUIMacroDesc(volumeLabel,ins,DIV_MACRO_VOL,0xff,volMin,volMax,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
      }
      if (ins->type!=DIV_INS_MSM6258 && ins->type!=DIV_INS_MSM6295 && ins->type!=DIV_INS_ADPCMA) {
        macroList.push_back(FurnaceGUIMacroDesc("Arpeggio",ins,DIV_MACRO_ARP,0xff,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,0,true,ins->std.get_macro(DIV_MACRO_ARP, true)->val));
      }
      if (dutyMax>0) {
        if (ins->type==DIV_INS_MIKEY) {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,mikeyFeedbackBits));
        } else if (ins->type==DIV_INS_POKEY) {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,pokeyCtlBits));
        } else if (ins->type==DIV_INS_TED) {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,0,dutyMax,80,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,tedControlBits));
        } else if (ins->type==DIV_INS_MSM5232) {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,0,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,msm5232ControlBits));
        } else if (ins->type==DIV_INS_ES5506) {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,&macroHoverES5506FilterMode));
        } else if (ins->type==DIV_INS_C219) {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,0,dutyMax,120,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c219ControlBits));
        } else {
          macroList.push_back(FurnaceGUIMacroDesc(dutyLabel,ins,DIV_MACRO_DUTY,0xff,dutyMin,dutyMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        }
      }
      if (waveMax>0) {
        macroList.push_back(FurnaceGUIMacroDesc(waveLabel,ins,DIV_MACRO_WAVE,0xff,0,waveMax,(waveBitMode && ins->type!=DIV_INS_PET)?64:160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,NULL,waveBitMode,waveNames,((ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930)?1:0)));
      }
      if (panMax>0) {
        if (panSingle) {
          macroList.push_back(FurnaceGUIMacroDesc("Panning",ins,DIV_MACRO_PAN_LEFT,0xff,0,panMax,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,panBits));
        } else if (ins->type==DIV_INS_QSOUND) {
          macroList.push_back(FurnaceGUIMacroDesc("Panning",ins,DIV_MACRO_PAN_LEFT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER]));
          macroList.push_back(FurnaceGUIMacroDesc("Surround",ins,DIV_MACRO_PAN_RIGHT,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        } else {
          if (panSingleNoBit || (ins->type==DIV_INS_AMIGA && ins->std.get_macro(DIV_MACRO_PAN_LEFT, true)->mode)) {
            macroList.push_back(FurnaceGUIMacroDesc("Panning",ins,DIV_MACRO_PAN_LEFT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
          } else {
            macroList.push_back(FurnaceGUIMacroDesc("Panning (left)",ins,DIV_MACRO_PAN_LEFT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER],false,(ins->type==DIV_INS_AMIGA)?macroQSoundMode:NULL));
          }
          if (!panSingleNoBit) {
            if (ins->type==DIV_INS_AMIGA && ins->std.get_macro(DIV_MACRO_PAN_LEFT, true)->mode) {
              macroList.push_back(FurnaceGUIMacroDesc("Surround",ins,DIV_MACRO_PAN_RIGHT,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
            } else {
              macroList.push_back(FurnaceGUIMacroDesc("Panning (right)",ins,DIV_MACRO_PAN_RIGHT,0xff,panMin,panMax,CLAMP(31+panMax-panMin,32,160),uiColors[GUI_COLOR_MACRO_OTHER]));
            }
          }
        }
      }
      if (ins->type!=DIV_INS_MSM5232 && ins->type!=DIV_INS_MSM6258 && ins->type!=DIV_INS_MSM6295 && ins->type!=DIV_INS_ADPCMA) {
        macroList.push_back(FurnaceGUIMacroDesc("Pitch",ins,DIV_MACRO_PITCH,0xff,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
      }
      if (ins->type==DIV_INS_FM ||
          ins->type==DIV_INS_OPM ||
          ins->type==DIV_INS_STD ||
          ins->type==DIV_INS_NES ||
          ins->type==DIV_INS_OPL ||
          ins->type==DIV_INS_OPL_DRUMS ||
          ins->type==DIV_INS_OPZ ||
          ins->type==DIV_INS_PCE ||
          ins->type==DIV_INS_GB ||
          ins->type==DIV_INS_MSM6258 ||
          ins->type==DIV_INS_MSM6295 ||
          ins->type==DIV_INS_ADPCMA ||
          ins->type==DIV_INS_ADPCMB ||
          ins->type==DIV_INS_SEGAPCM ||
          ins->type==DIV_INS_QSOUND ||
          ins->type==DIV_INS_YMZ280B ||
          ins->type==DIV_INS_RF5C68 ||
          ins->type==DIV_INS_AMIGA ||
          ins->type==DIV_INS_OPLL ||
          ins->type==DIV_INS_AY ||
          ins->type==DIV_INS_AY8930 ||
          ins->type==DIV_INS_SWAN ||
          ins->type==DIV_INS_MULTIPCM ||
          (ins->type==DIV_INS_VRC6 && ins->amiga.useSample) ||
          ins->type==DIV_INS_SU ||
          ins->type==DIV_INS_MIKEY ||
          ins->type==DIV_INS_ES5506 ||
          ins->type==DIV_INS_T6W28 ||
          ins->type==DIV_INS_VBOY ||
          (ins->type==DIV_INS_X1_010 && ins->amiga.useSample) ||
          ins->type==DIV_INS_K007232 ||
          ins->type==DIV_INS_GA20 ||
          ins->type==DIV_INS_K053260 ||
          ins->type==DIV_INS_C140 ||
          ins->type==DIV_INS_C219 ||
          ins->type==DIV_INS_TED ||
          ins->type==DIV_INS_ESFM ||
          ins->type==DIV_INS_ES5503) {
        macroList.push_back(FurnaceGUIMacroDesc("Phase Reset",ins,DIV_MACRO_PHASE_RESET,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      }
      if (ex1Max>0) {
        if (ins->type==DIV_INS_C64) {
          int cutoffMin=-2047;
          int cutoffMax=2047;

          if (ins->c64.filterIsAbs) {
            cutoffMin=0;
            cutoffMax=2047;
          }
          macroList.push_back(FurnaceGUIMacroDesc("Cutoff",ins,DIV_MACRO_ALG,0xff,cutoffMin,cutoffMax,160,uiColors[GUI_COLOR_MACRO_OTHER]));
          macroList.push_back(FurnaceGUIMacroDesc("Filter Mode",ins,DIV_MACRO_EX1,0xff,0,ex1Max,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,filtModeBits));
        } else if (ins->type==DIV_INS_SAA1099) {
          macroList.push_back(FurnaceGUIMacroDesc("Envelope",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,saaEnvBits));
        } else if (ins->type==DIV_INS_X1_010 && !ins->amiga.useSample) {
          macroList.push_back(FurnaceGUIMacroDesc("Envelope Mode",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,x1_010EnvBits));
        } else if (ins->type==DIV_INS_N163) {
          macroList.push_back(FurnaceGUIMacroDesc("Wave Length",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_FDS) {
          macroList.push_back(FurnaceGUIMacroDesc("Mod Depth",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_SU) {
          macroList.push_back(FurnaceGUIMacroDesc("Cutoff",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_ES5506) {
          macroList.push_back(FurnaceGUIMacroDesc("Filter K1",ins,DIV_MACRO_EX1,0xff,((ins->std.get_macro(DIV_MACRO_EX1, true)->mode==1)?(-ex1Max):0),ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,macroRelativeMode));
        } else if (ins->type==DIV_INS_MSM6258) {
          macroList.push_back(FurnaceGUIMacroDesc("Clock Divider",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_QSOUND) {
          macroList.push_back(FurnaceGUIMacroDesc("Echo Feedback",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_SNES) {
          macroList.push_back(FurnaceGUIMacroDesc("Special",ins,DIV_MACRO_EX1,0xff,0,ex1Max,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,snesModeBits));
        } else if (ins->type==DIV_INS_MSM5232) {
          macroList.push_back(FurnaceGUIMacroDesc("Group Attack",ins,DIV_MACRO_EX1,0xff,0,ex1Max,96,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_ES5503) {
          macroList.push_back(FurnaceGUIMacroDesc("Wave/sample pos.",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else {
          macroList.push_back(FurnaceGUIMacroDesc("Duty",ins,DIV_MACRO_EX1,0xff,0,ex1Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        }
      }
      if (ex2Max>0) {
        if (ins->type==DIV_INS_C64) {
          macroList.push_back(FurnaceGUIMacroDesc("Resonance",ins,DIV_MACRO_EX2,0xff,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_FDS) {
          macroList.push_back(FurnaceGUIMacroDesc("Mod Speed",ins,DIV_MACRO_EX2,0xff,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_SU) {
          macroList.push_back(FurnaceGUIMacroDesc("Resonance",ins,DIV_MACRO_EX2,0xff,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_ES5506) {
          macroList.push_back(FurnaceGUIMacroDesc("Filter K2",ins,DIV_MACRO_EX2,0xff,((ins->std.get_macro(DIV_MACRO_EX2, true)->mode==1)?(-ex2Max):0),ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER],false,macroRelativeMode));
        } else if (ins->type==DIV_INS_QSOUND) {
          macroList.push_back(FurnaceGUIMacroDesc("Echo Length",ins,DIV_MACRO_EX2,0xff,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_SNES) {
          macroList.push_back(FurnaceGUIMacroDesc("Gain",ins,DIV_MACRO_EX2,0xff,0,ex2Max,256,uiColors[GUI_COLOR_MACRO_VOLUME],false,NULL,macroHoverGain,false));
          macroList.push_back(FurnaceGUIMacroDesc("Group Decay",ins,DIV_MACRO_EX2,0xff,0,ex2Max,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else if (ins->type==DIV_INS_ES5503) {
          macroList.push_back(FurnaceGUIMacroDesc("Osc. output",ins,DIV_MACRO_EX2,0xff,0,ex2Max,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        } else {
          macroList.push_back(FurnaceGUIMacroDesc("Envelope",ins,DIV_MACRO_EX2,0xff,0,ex2Max,ex2Bit?64:160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,ex2Bit,ayEnvBits));
        }
      }
      if (ins->type==DIV_INS_C64) {
        macroList.push_back(FurnaceGUIMacroDesc("Filter toggle",ins,DIV_MACRO_EX3,0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc("Special",ins,DIV_MACRO_EX4,0xff,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,c64TestGateBits));
        macroList.push_back(FurnaceGUIMacroDesc("Attack",ins,DIV_MACRO_EX5, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Decay",ins,DIV_MACRO_EX6, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Sustain",ins,DIV_MACRO_EX7, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Release",ins,DIV_MACRO_EX8, 0xff,0,15,128,uiColors[GUI_COLOR_MACRO_OTHER]));
      }
      if (ins->type==DIV_INS_AY || ins->type==DIV_INS_AY8930 || (ins->type==DIV_INS_X1_010 && !ins->amiga.useSample)) {
        macroList.push_back(FurnaceGUIMacroDesc("AutoEnv Num",ins,DIV_MACRO_EX3, 0xff,0,15,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("AutoEnv Den",ins,DIV_MACRO_ALG,0xff,0,15,160,uiColors[GUI_COLOR_MACRO_OTHER]));
      }
      if (ins->type==DIV_INS_AY8930) {
        // oh my i am running out of macros
        macroList.push_back(FurnaceGUIMacroDesc("Noise AND Mask",ins,DIV_MACRO_FB, 0xff,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
        macroList.push_back(FurnaceGUIMacroDesc("Noise OR Mask",ins,DIV_MACRO_FMS, 0xff,0,8,96,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      }
      if (ins->type==DIV_INS_FDS) {
        macroList.push_back(FurnaceGUIMacroDesc("Mod Position",ins,DIV_MACRO_EX3, 0xff,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
      }
      if (ins->type==DIV_INS_SU) {
        macroList.push_back(FurnaceGUIMacroDesc("Control",ins,DIV_MACRO_EX3, 0xff,0,4,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,suControlBits));
        macroList.push_back(FurnaceGUIMacroDesc("Phase Reset Timer",ins,DIV_MACRO_EX4,0xff,0,65535,160,uiColors[GUI_COLOR_MACRO_OTHER])); // again reuse code from resonance macro but use ex4 instead
      }
      if (ins->type==DIV_INS_ES5506) {
        /*macroList.push_back(FurnaceGUIMacroDesc("Envelope counter",ins,DIV_MACRO_EX3, 0xff,0,511,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Envelope left volume ramp",ins,DIV_MACRO_EX4,0xff,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Envelope right volume ramp",ins,DIV_MACRO_EX5, 0xff,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Envelope K1 ramp",ins,DIV_MACRO_EX6, 0xff,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Envelope K2 ramp",ins,DIV_MACRO_EX7, 0xff,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Envelope mode",ins,DIV_MACRO_EX8, 0xff,0,2,64,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,es5506EnvelopeModes));*/
        macroList.push_back(FurnaceGUIMacroDesc("Outputs",ins,DIV_MACRO_FB, 0xff,0,5,64,uiColors[GUI_COLOR_MACRO_OTHER]));
        macroList.push_back(FurnaceGUIMacroDesc("Control",ins,DIV_MACRO_ALG,0xff,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,es5506ControlModes));
      }
      if (ins->type==DIV_INS_MSM5232) {
        macroList.push_back(FurnaceGUIMacroDesc("Noise",ins,DIV_MACRO_EX3, 0xff,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
      }

      /*for(int i = 0; i < (int)ins->std.macros.size(); i++) // reset macro zoom
      {
        ins->std.macros[i].vZoom = -1;
      }*/

      drawMacros(macroList,macroEditStateMacros);
      ImGui::EndTabItem();
    }
    if (ins->type==DIV_INS_NES ||
        ins->type==DIV_INS_AY ||
        ins->type==DIV_INS_AY8930 ||
        ins->type==DIV_INS_MIKEY ||
        ins->type==DIV_INS_PCE ||
        ins->type==DIV_INS_X1_010 ||
        ins->type==DIV_INS_SWAN ||
        ins->type==DIV_INS_VRC6 ||
        ins->type==DIV_INS_ES5503) {
      insTabSample(ins);
    }
    if (ins->type>=DIV_INS_MAX) {
      if (ImGui::BeginTabItem("Error")) {
        ImGui::Text("invalid instrument type! change it first.");
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  if (settings.insEditColorize) {
    popAccentColors();
  }
}

void FurnaceGUI::drawInsEdit() {
  if (nextWindow==GUI_WINDOW_INS_EDIT) {
    insEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!insEditOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Instrument Editor",&insEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking))) {
    if (curIns<0 || curIns>=(int)e->song.ins.size()) {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()*(e->song.ins.empty()?2.0f:3.0f)+ImGui::GetStyle().ItemSpacing.y)*0.5f);
      CENTER_TEXT("no instrument selected");
      ImGui::Text("no instrument selected");
      if (ImGui::BeginTable("noAssetCenter",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (e->song.ins.size()>0) {
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##InsSelect","select one...")) {
            String name;
            for (size_t i=0; i<e->song.ins.size(); i++) {
              name=fmt::sprintf("%.2X: %s##_INSS%d",i,e->song.ins[i]->name,i);
              if (ImGui::Selectable(name.c_str(),curIns==(int)i)) {
                curIns=i;
                wavePreviewInit=true;
                updateFMPreview=true;
              }
            }
            ImGui::EndCombo();
          }
          ImGui::AlignTextToFramePadding();
          ImGui::TextUnformatted("or");
          ImGui::SameLine();
        }
        if (ImGui::Button("Open")) {
          doAction(GUI_ACTION_INS_LIST_OPEN);
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("or");
        ImGui::SameLine();
        if (ImGui::Button("Create New")) {
          doAction(GUI_ACTION_INS_LIST_ADD);
        }

        ImGui::TableNextColumn();
        ImGui::EndTable();
      }
    } else {
      drawActualInsEditor();
    }
    
    if (displayMacroMenu) {
      displayMacroMenu=false;
      if (lastMacroDesc.get_macro()!=NULL) {
        ImGui::OpenPopup("macroMenu");
      }
    }
    if (ImGui::BeginPopup("macroMenu",ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoSavedSettings)) {
      if (ImGui::MenuItem("copy")) {
        String mmlStr;
        encodeMMLStr(mmlStr,lastMacroDesc.get_macro()->val,lastMacroDesc.get_macro()->len,lastMacroDesc.get_macro()->loop,lastMacroDesc.get_macro()->rel);
        SDL_SetClipboardText(mmlStr.c_str());
      }
      if (ImGui::MenuItem("paste")) {
        String mmlStr;
        char* clipText=SDL_GetClipboardText();
        if (clipText!=NULL) {
          if (clipText[0]) {
            mmlStr=clipText;
          }
          SDL_free(clipText);
        }
        if (!mmlStr.empty()) {
          decodeMMLStr(mmlStr,lastMacroDesc.get_macro()->val,lastMacroDesc.get_macro()->len,lastMacroDesc.get_macro()->loop,lastMacroDesc.min,(lastMacroDesc.isBitfield)?((1<<(lastMacroDesc.isBitfield?lastMacroDesc.max:0))-1):lastMacroDesc.max,lastMacroDesc.get_macro()->rel);
        }
      }
      ImGui::Separator();
      if (ImGui::MenuItem("clear")) {
        lastMacroDesc.get_macro()->len=0;
        lastMacroDesc.get_macro()->loop=255;
        lastMacroDesc.get_macro()->rel=255;
        for (int i=0; i<256; i++) {
          lastMacroDesc.get_macro()->val[i]=0;
        }
      }
      if (ImGui::MenuItem("clear contents")) {
        for (int i=0; i<256; i++) {
          lastMacroDesc.get_macro()->val[i]=0;
        }
      }
      ImGui::Separator();
      if (ImGui::BeginMenu("offset...")) {
        ImGui::InputInt("X",&macroOffX,1,10);
        ImGui::InputInt("Y",&macroOffY,1,10);
        if (ImGui::Button("offset")) {
          int oldData[256];
          memset(oldData,0,256*sizeof(int));
          memcpy(oldData,lastMacroDesc.get_macro()->val,lastMacroDesc.get_macro()->len*sizeof(int));

          for (int i=0; i<lastMacroDesc.get_macro()->len; i++) {
            int val=0;
            if ((i-macroOffX)>=0 && (i-macroOffX)<lastMacroDesc.get_macro()->len) {
              val=oldData[i-macroOffX]+macroOffY;
              if (val<lastMacroDesc.min) val=lastMacroDesc.min;
              if (val>lastMacroDesc.max) val=lastMacroDesc.max;
            }
            lastMacroDesc.get_macro()->val[i]=val;
          }

          if (lastMacroDesc.get_macro()->loop<lastMacroDesc.get_macro()->len) {
            lastMacroDesc.get_macro()->loop+=macroOffX;
          } else {
            lastMacroDesc.get_macro()->loop=255;
          }
          if ((lastMacroDesc.get_macro()->rel+macroOffX)>=0 && (lastMacroDesc.get_macro()->rel+macroOffX)<lastMacroDesc.get_macro()->len) {
            lastMacroDesc.get_macro()->rel+=macroOffX;
          } else {
            lastMacroDesc.get_macro()->rel=255;
          }

          ImGui::CloseCurrentPopup();
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("scale...")) {
        if (ImGui::InputFloat("X",&macroScaleX,1.0f,10.0f,"%.2f%%")) {
          if (macroScaleX<0.1) macroScaleX=0.1;
          if (macroScaleX>12800.0) macroScaleX=12800.0;
        }
        ImGui::InputFloat("Y",&macroScaleY,1.0f,10.0f,"%.2f%%");
        if (ImGui::Button("scale")) {
          int oldData[256];
          memset(oldData,0,256*sizeof(int));
          memcpy(oldData,lastMacroDesc.get_macro()->val,lastMacroDesc.get_macro()->len*sizeof(int));

          lastMacroDesc.get_macro()->len=MIN(128,((double)lastMacroDesc.get_macro()->len*(macroScaleX/100.0)));

          for (int i=0; i<lastMacroDesc.get_macro()->len; i++) {
            int val=0;
            double posX=round((double)i*(100.0/macroScaleX)-0.01);
            if (posX>=0 && posX<lastMacroDesc.get_macro()->len) {
              val=round((double)oldData[(int)posX]*(macroScaleY/100.0));
              if (val<lastMacroDesc.min) val=lastMacroDesc.min;
              if (val>lastMacroDesc.max) val=lastMacroDesc.max;
            }
            lastMacroDesc.get_macro()->val[i]=val;
          }

          ImGui::CloseCurrentPopup();
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("randomize...")) {
        if (macroRandMin<lastMacroDesc.min) macroRandMin=lastMacroDesc.min;
        if (macroRandMin>lastMacroDesc.max) macroRandMin=lastMacroDesc.max;
        if (macroRandMax<lastMacroDesc.min) macroRandMax=lastMacroDesc.min;
        if (macroRandMax>lastMacroDesc.max) macroRandMax=lastMacroDesc.max;
        ImGui::InputInt("Min",&macroRandMin,1,10);
        ImGui::InputInt("Max",&macroRandMax,1,10);
        if (ImGui::Button("randomize")) {
          for (int i=0; i<lastMacroDesc.get_macro()->len; i++) {
            int val=0;
            if (macroRandMax<=macroRandMin) {
              val=macroRandMin;
            } else {
              val=macroRandMin+(rand()%(macroRandMax-macroRandMin+1));
            }
            lastMacroDesc.get_macro()->val[i]=val;
          }

          ImGui::CloseCurrentPopup();
        }
        ImGui::EndMenu();
      }
      
      ImGui::EndPopup();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_EDIT;
  ImGui::End();
}