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

  if (ImGui::BeginTabBar("insEditTab")) 
  {
    macroList.clear();
    macroList.shrink_to_fit();

    switch(ins->type)
    {
      case DIV_INS_STD:
      {
        drawInsSTD(ins); break;
      }

      case DIV_INS_FM:
      {
        drawInsOPN(ins); break;
      }

      case DIV_INS_OPM:
      {
        drawInsOPM(ins); break;
      }

      case DIV_INS_OPZ:
      {
        drawInsOPZ(ins); break;
      }

      case DIV_INS_ESFM:
      {
        drawInsESFM(ins); break;
      }

      case DIV_INS_OPL:
      case DIV_INS_OPL_DRUMS:
      {
        drawInsOPL(ins); break;
      }

      case DIV_INS_OPLL:
      {
        drawInsOPLL(ins); break;
      }

      case DIV_INS_GB:
      {
        drawInsGB(ins); break;
      }

      case DIV_INS_C64:
      {
        drawInsC64(ins); break;
      }

      case DIV_INS_ES5503:
      {
        drawInsES5503(ins); break;
      }

      case DIV_INS_SU:
      {
        drawInsSU(ins); break;
      }

      case DIV_INS_N163:
      {
        drawInsN163(ins); break;
      }

      case DIV_INS_FDS:
      {
        drawInsFDS(ins); break;
      }

      case DIV_INS_VBOY:
      {
        drawInsVBOY(ins); break;
      }

      case DIV_INS_ES5506:
      {
        drawInsES5506(ins); break;
      }

      case DIV_INS_MULTIPCM:
      {
        drawInsMULTIPCM(ins); break;
      }

      case DIV_INS_SNES:
      {
        drawInsSNES(ins); break;
      }

      case DIV_INS_ADPCMA:
      {
        drawInsADPCMA(ins); break;
      }

      case DIV_INS_ADPCMB:
      {
        drawInsADPCMB(ins); break;
      }

      case DIV_INS_AMIGA:
      {
        drawInsAmiga(ins); break;
      }

      case DIV_INS_AY:
      {
        drawInsAY(ins); break;
      }

      case DIV_INS_AY8930:
      {
        drawInsAY8930(ins); break;
      }

      case DIV_INS_BEEPER:
      {
        drawInsBEEPER(ins); break;
      }

      case DIV_INS_C140:
      {
        drawInsC140(ins); break;
      }

      case DIV_INS_C219:
      {
        drawInsC219(ins); break;
      }

      case DIV_INS_GA20:
      {
        drawInsGA20(ins); break;
      }

      case DIV_INS_K007232:
      {
        drawInsK007232(ins); break;
      }

      case DIV_INS_K053260:
      {
        drawInsK053260(ins); break;
      }

      case DIV_INS_MIKEY:
      {
        drawInsMIKEY(ins); break;
      }

      case DIV_INS_MSM5232:
      {
        drawInsMSM5232(ins); break;
      }

      case DIV_INS_MSM6258:
      {
        drawInsMSM6258(ins); break;
      }

      case DIV_INS_MSM6295:
      {
        drawInsMSM6295(ins); break;
      }

      case DIV_INS_NAMCO:
      {
        drawInsNAMCO(ins); break;
      }

      case DIV_INS_NES:
      {
        drawInsNES(ins); break;
      }

      case DIV_INS_PCE:
      {
        drawInsPCE(ins); break;
      }

      case DIV_INS_PET:
      {
        drawInsPET(ins); break;
      }

      case DIV_INS_POKEMINI:
      {
        drawInsPOKEMINI(ins); break;
      }

      case DIV_INS_POKEY:
      {
        drawInsPOKEY(ins); break;
      }

      case DIV_INS_PV1000:
      {
        drawInsPV1000(ins); break;
      }

      case DIV_INS_QSOUND:
      {
        drawInsQSOUND(ins); break;
      }

      case DIV_INS_RF5C68:
      {
        drawInsRF5C68(ins); break;
      }

      case DIV_INS_SAA1099:
      {
        drawInsSAA1099(ins); break;
      }

      case DIV_INS_SCC:
      {
        drawInsSCC(ins); break;
      }

      case DIV_INS_SEGAPCM:
      {
        drawInsSEGAPCM(ins); break;
      }

      case DIV_INS_SM8521:
      {
        drawInsSM8521(ins); break;
      }

      case DIV_INS_SWAN:
      {
        drawInsSWAN(ins); break;
      }

      case DIV_INS_T6W28:
      {
        drawInsT6W28(ins); break;
      }

      case DIV_INS_TED:
      {
        drawInsTED(ins); break;
      }

      case DIV_INS_TIA:
      {
        drawInsTIA(ins); break;
      }

      case DIV_INS_VERA:
      {
        drawInsVERA(ins); break;
      }

      case DIV_INS_VIC:
      {
        drawInsVIC(ins); break;
      }

      case DIV_INS_VRC6:
      {
        drawInsVRC6(ins); break;
      }

      case DIV_INS_VRC6_SAW:
      {
        drawInsVRC6_SAW(ins); break;
      }

      case DIV_INS_X1_010:
      {
        drawInsX1_010(ins); break;
      }

      case DIV_INS_YMZ280B:
      {
        drawInsYMZ280B(ins); break;
      }

      case DIV_INS_POWERNOISE:
      {
        drawInsPOWERNOISE(ins); break;
      }

      case DIV_INS_POWERNOISE_SLOPE:
      {
        drawInsPOWERNOISESLOPE(ins); break;
      }

      default: break;
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
    if (curIns==-2) {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()+ImGui::GetStyle().ItemSpacing.y)*0.5f);
      CENTER_TEXT("waiting...");
      ImGui::Text("waiting...");
    } else if (curIns<0 || curIns>=(int)e->song.ins.size()) {
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