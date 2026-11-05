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
#include "util.h"

#include "insEdit/insEditCommon.h"

void FurnaceGUI::drawInsEdit() {
  // acknowledge a window request
  if (nextWindow==GUI_WINDOW_INS_EDIT) {
    insEditOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  // don't draw the window if not open
  if (!insEditOpen) return;
  // fixed position on mobile
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)-(pianoOpen?(0.4*canvasW):0.0f)):ImVec2(canvasW-(0.16*canvasH),canvasH-(pianoOpen?(0.3*canvasH):0.0f)));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    // give this window a minimum size
    ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Instrument Editor",&insEditOpen,globalWinFlags|(settings.allowEditDocking?0:ImGuiWindowFlags_NoDocking),_("Instrument Editor"))) {
    DivInstrument* ins=NULL;
    // check whether we can actually display the editor
    if (curIns==-2) {
      // ins preview (from file picker)
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()+ImGui::GetStyle().ItemSpacing.y)*0.5f);
      CENTER_TEXT(_("waiting..."));
      ImGui::Text(_("waiting..."));
    } else if (curIns<0 || curIns>=(int)e->song.ins.size()) {
      // no instrument selected
      ImGui::SetCursorPosY(ImGui::GetCursorPosY()+(ImGui::GetContentRegionAvail().y-ImGui::GetFrameHeightWithSpacing()*(e->song.ins.empty()?2.0f:3.0f)+ImGui::GetStyle().ItemSpacing.y)*0.5f);
      CENTER_TEXT(_("no instrument selected"));
      ImGui::Text(_("no instrument selected"));
      // a table is used to center the three or so buttons that are displayed
      if (ImGui::BeginTable("noAssetCenter",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.5f);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch,0.5f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        // if the song has instruments, prompt the user to select one
        if (e->song.ins.size()>0) {
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::BeginCombo("##InsSelect",_("select one..."))) {
            String name;
            for (size_t i=0; i<e->song.ins.size(); i++) {
              name=fmt::sprintf("%.2X: %s##_INSS%d",i,e->song.ins[i]->name,i);
              if (ImGui::Selectable(name.c_str(),curIns==(int)i)) {
                setCurIns(i);
                wavePreviewInit=true;
                updateFMPreview=true;
                ins=e->song.ins[curIns];
              }
            }
            ImGui::EndCombo();
          }
          ImGui::AlignTextToFramePadding();
          ImGui::TextUnformatted(_("or"));
          ImGui::SameLine();
        }
        // open/create
        if (ImGui::Button(_("Open"))) {
          doAction(GUI_ACTION_INS_LIST_OPEN);
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(_("or"));
        ImGui::SameLine();
        if (ImGui::Button(_("Create New"))) {
          doAction(GUI_ACTION_INS_LIST_ADD);
        }

        ImGui::TableNextColumn();
        ImGui::EndTable();
      }
    } else {
      // this is where the actual instrument editor resides...
      ins=e->song.ins[curIns];

      // reset FM preview if needed
      if (updateFMPreview) {
        renderFMPreview(ins);
        updateFMPreview=false;
      }

      // "colorize instrument editor" setting
      if (settings.insEditColorize) {
        if (ins->type>=DIV_INS_MAX) {
          pushAccentColors(uiColors[GUI_COLOR_INSTR_UNKNOWN],uiColors[GUI_COLOR_INSTR_UNKNOWN],uiColors[GUI_COLOR_INSTR_UNKNOWN],ImVec4(0.0f,0.0f,0.0f,0.0f));
        } else {
          pushAccentColors(uiColors[GUI_COLOR_INSTR_STD+ins->type],uiColors[GUI_COLOR_INSTR_STD+ins->type],uiColors[GUI_COLOR_INSTR_STD+ins->type],ImVec4(0.0f,0.0f,0.0f,0.0f));
        }
      }

      // HEADER - instrument name, type, index and load/save buttons
      if (ImGui::BeginTable("InsProp",3)) {
        ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        // instrument selector (index)
        String insIndex=fmt::sprintf("%.2X",curIns);
        ImGui::SetNextItemWidth(72.0f*dpiScale);
        if (ImGui::BeginCombo("##InsSelect",insIndex.c_str())) {
          String name;
          for (size_t i=0; i<e->song.ins.size(); i++) {
            name=fmt::sprintf("%.2X: %s##_INSS%d",i,e->song.ins[i]->name,i);
            if (ImGui::Selectable(name.c_str(),curIns==(int)i)) {
              setCurIns(i);
              ins=e->song.ins[curIns];
              wavePreviewInit=true;
              updateFMPreview=true;
            }
          }
          ImGui::EndCombo();
        }

        // instrument name
        ImGui::TableNextColumn();
        ImGui::Text(_("Name"));

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::PushID(2+curIns);
        if (ImGui::InputText("##Name",&ins->name)) {
          MARK_MODIFIED;
        }
        ImGui::PopID();

        // load/save buttons
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_FOLDER_OPEN "##IELoad")) {
          doAction(GUI_ACTION_INS_LIST_OPEN_REPLACE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Open"));
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_FLOPPY_O "##IESave")) {
          doAction(GUI_ACTION_INS_LIST_SAVE);
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Save (right click for options)"));
        }
        if (ImGui::BeginPopupContextItem("InsSaveFormats",ImGuiMouseButton_Right)) {
          if (ImGui::MenuItem(_("save as .dmp..."))) {
            doAction(GUI_ACTION_INS_LIST_SAVE_DMP);
          }
          ImGui::EndPopup();
        }

        // instrument type
        ImGui::TableNextColumn();
        ImGui::Text(_("Type"));

        ImGui::TableNextColumn();
        int insType=ins->type;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        // check whether the instrument type is supported by the currently present chips
        bool warnType=true;
        for (DivInstrumentType i: e->getPossibleInsTypes()) {
          if (i==insType) {
            warnType=false;
          }
        }

        // if not, use warning colors
        pushWarningColor(warnType,warnType && failedNoteOn);
        if (ImGui::BeginCombo("##Type",(insType>=DIV_INS_MAX)?_("Unknown"):_(insTypes[insType][0]))) {
          // confine options to possible ins types unless the setting to display all instrument types is enabled
          std::vector<DivInstrumentType> insTypeList;
          if (settings.displayAllInsTypes) {
            for (int i=0; insTypes[i][0]; i++) {
              insTypeList.push_back((DivInstrumentType)i);
            }
          } else {
            insTypeList=e->getPossibleInsTypes();
          }
          // display options
          for (DivInstrumentType i: insTypeList) {
            if (ImGui::Selectable(insTypes[i][0],insType==i)) {
              ins->type=i;
              MARK_MODIFIED;

              // reset macro zoom
              memset(ins->temp.vZoom,-1,sizeof(ins->temp.vZoom));
            }
          }
          ImGui::EndCombo();
        } else if (warnType) {
          // provide a warning when hovered
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("none of the currently present chips are able to play this instrument type!"));
          }
        }
        popWarningColor();

        ImGui::EndTable();
      }

      // EDITING AREA
      if (ImGui::BeginTabBar("insEditTab")) {
        switch (ins->type) {
          case DIV_INS_STD:
            insEditSTD(ins);
            break;
          case DIV_INS_FM:
            insEditOPN(ins);
            break;
          case DIV_INS_GB:
            insEditGB(ins);
            break;
          case DIV_INS_C64:
            insEditC64(ins);
            break;
          case DIV_INS_AMIGA:
            insEditGenericSample(ins);
            break;
          case DIV_INS_PCE:
            insEditPCE(ins);
            break;
          case DIV_INS_AY:
            insEditAY(ins);
            break;
          case DIV_INS_AY8930:
            insEditAY8930(ins);
            break;
          case DIV_INS_TIA:
            insEditTIA(ins);
            break;
          case DIV_INS_SAA1099:
            insEditSAA1099(ins);
            break;
          case DIV_INS_VIC:
            insEditVIC(ins);
            break;
          case DIV_INS_PET:
            insEditPET(ins);
            break;
          case DIV_INS_VRC6:
            insEditVRC6(ins);
            break;
          case DIV_INS_OPLL:
            insEditOPLL(ins);
            break;
          case DIV_INS_OPL:
            insEditOPL(ins);
            break;
          case DIV_INS_FDS:
            insEditFDS(ins);
            break;
          case DIV_INS_VBOY:
            insEditVBoy(ins);
            break;
          case DIV_INS_N163:
            insEditN163(ins);
            break;
          case DIV_INS_SCC:
            insEditSCC(ins);
            break;
          case DIV_INS_OPZ:
            insEditOPZ(ins);
            break;
          case DIV_INS_POKEY:
            insEditPOKEY(ins);
            break;
          case DIV_INS_BEEPER:
            insEditBeeper(ins);
            break;
          case DIV_INS_SWAN:
            insEditSwan(ins);
            break;
          case DIV_INS_MIKEY:
            insEditMikey(ins);
            break;
          case DIV_INS_VERA:
            insEditVERA(ins);
            break;
          case DIV_INS_X1_010:
            insEditX1_010(ins);
            break;
          case DIV_INS_VRC6_SAW:
            insEditVRC6Saw(ins);
            break;
          case DIV_INS_ES5506:
            insEditES5506(ins);
            break;
          case DIV_INS_MULTIPCM:
            insEditMultiPCM(ins);
            break;
          case DIV_INS_SNES:
            insEditSNES(ins);
            break;
          case DIV_INS_SU:
            insEditSU(ins);
            break;
          case DIV_INS_NAMCO:
            insEditNamco(ins);
            break;
          case DIV_INS_OPL_DRUMS:
            insEditOPLDrums(ins);
            break;
          case DIV_INS_OPM:
            insEditOPM(ins);
            break;
          case DIV_INS_NES:
            insEditNES(ins);
            break;
          case DIV_INS_MSM6258:
            insEditMSM6258(ins);
            break;
          case DIV_INS_MSM6295:
            insEditMSM6295(ins);
            break;
          case DIV_INS_ADPCMA:
            insEditADPCMA(ins);
            break;
          case DIV_INS_ADPCMB:
            insEditADPCMB(ins);
            break;
          case DIV_INS_SEGAPCM:
            insEditSegaPCM(ins);
            break;
          case DIV_INS_QSOUND:
            insEditQSound(ins);
            break;
          case DIV_INS_YMZ280B:
            insEditYMZ280B(ins);
            break;
          case DIV_INS_RF5C68:
            insEditRF5C68(ins);
            break;
          case DIV_INS_MSM5232:
            insEditMSM5232(ins);
            break;
          case DIV_INS_T6W28:
            insEditT6W28(ins);
            break;
          case DIV_INS_K007232:
            insEditK007232(ins);
            break;
          case DIV_INS_GA20:
            insEditGA20(ins);
            break;
          case DIV_INS_POKEMINI:
            insEditPokeMini(ins);
            break;
          case DIV_INS_SM8521:
            insEditSM8521(ins);
            break;
          case DIV_INS_PV1000:
            insEditPV1000(ins);
            break;
          case DIV_INS_K053260:
            insEditK053260(ins);
            break;
          case DIV_INS_TED:
            insEditTED(ins);
            break;
          case DIV_INS_C140:
            insEditC140(ins);
            break;
          case DIV_INS_C219:
            insEditC219(ins);
            break;
          case DIV_INS_ESFM:
            insEditESFM(ins);
            break;
          case DIV_INS_POWERNOISE:
            insEditPowerNoise(ins);
            break;
          case DIV_INS_POWERNOISE_SLOPE:
            insEditPowerNoiseSlope(ins);
            break;
          case DIV_INS_DAVE:
            insEditDave(ins);
            break;
          case DIV_INS_NDS:
            insEditNDS(ins);
            break;
          case DIV_INS_GBA_DMA:
            insEditGBADMA(ins);
            break;
          case DIV_INS_GBA_MINMOD:
            insEditGBAMinMod(ins);
            break;
          case DIV_INS_BIFURCATOR:
            insEditBifurcator(ins);
            break;
          case DIV_INS_SID2:
            insEditSID2(ins);
            break;
          case DIV_INS_SUPERVISION:
            insEditSupervision(ins);
            break;
          case DIV_INS_UPD1771C:
            insEditSCV(ins);
            break;
          case DIV_INS_SID3:
            insEditSID3(ins);
            break;
          case DIV_INS_KLATTSCH:
            insEditKlattsch(ins);
            break;
          default:
            if (ImGui::BeginTabItem(_("Error"))) {
              ImGui::Text(_("invalid instrument type! change it first."));
              ImGui::EndTabItem();
            }
            break;
        }

        ImGui::EndTabBar();
      }
      if (settings.insEditColorize) {
        popAccentColors();
      }
    }
    handleMacroMenu(ins);
  }

  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_INS_EDIT;
  ImGui::End();
}

void FurnaceGUI::checkRecordInstrumentUndoStep() {
  if (insEditOpen && curIns>=0 && curIns<(int)e->song.ins.size()) {
    DivInstrument* ins=e->song.ins[curIns];

    // invalidate cachedCurIns/any possible changes if the cachedCurIns was referencing a different
    // instrument altgoether
    bool insChanged=ins!=cachedCurInsPtr;
    if (insChanged) {
      insEditMayBeDirty=false;
      cachedCurInsPtr=ins;
      cachedCurIns=*ins;
    }

    cachedCurInsPtr=ins;

    // check against the last cached to see if diff -- note that modifications to instruments
    // happen outside drawInsEdit (e.g. cursor inputs are processed and can directly modify
    // macro data).  but don't check until we think the user input is complete.
    bool delayDiff=ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::GetIO().WantCaptureKeyboard;
    if (!delayDiff && insEditMayBeDirty) {
      bool hasChange=ins->recordUndoStepIfChanged(e->processTime, &cachedCurIns);
      if (hasChange) {
        cachedCurIns=*ins;
      }
      insEditMayBeDirty=false;
    }
  } else {
    cachedCurInsPtr=NULL;
    insEditMayBeDirty=false;
  }
}

void FurnaceGUI::doUndoInstrument() {
  if (!insEditOpen) return;
  if (curIns<0 || curIns>=(int)e->song.ins.size()) return;
  DivInstrument* ins=e->song.ins[curIns];
  // is locking the engine necessary? copied from doUndoSample
  e->lockEngine([this,ins]() {
    ins->undo();
    cachedCurInsPtr=ins;
    cachedCurIns=*ins;
  });
}

void FurnaceGUI::doRedoInstrument() {
  if (!insEditOpen) return;
  if (curIns<0 || curIns>=(int)e->song.ins.size()) return;
  DivInstrument* ins=e->song.ins[curIns];
  // is locking the engine necessary? copied from doRedoSample
  e->lockEngine([this,ins]() {
    ins->redo();
    cachedCurInsPtr=ins;
    cachedCurIns=*ins;
  });
}
