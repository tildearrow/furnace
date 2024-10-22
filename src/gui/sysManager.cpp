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

#include "gui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include <fmt/printf.h>
#include <imgui.h>

void FurnaceGUI::drawSysManager() {
  if (nextWindow==GUI_WINDOW_SYS_MANAGER) {
    sysManagerOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!sysManagerOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)):ImVec2(canvasW-(0.16*canvasH),canvasH));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    //ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Chip Manager",&sysManagerOpen,globalWinFlags,_("Chip Manager"))) {
    ImGui::Checkbox(_("Preserve channel order"),&preserveChanPos);
    ImGui::SameLine();
    ImGui::Checkbox(_("Clone channel data"),&sysDupCloneChannels);
    ImGui::SameLine();
    ImGui::Checkbox(_("Clone at end"),&sysDupEnd);
    if (ImGui::BeginTable("SystemList",3)) {
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::TableNextColumn();
      ImGui::Text(_("Name"));
      ImGui::TableNextColumn();
      ImGui::Text(_("Actions"));
      for (unsigned char i=0; i<e->song.systemLen; i++) {
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_ARROWS)) {
        }
        if (ImGui::BeginDragDropSource()) {
          sysToMove=i;
          ImGui::SetDragDropPayload("FUR_SYS",NULL,0,ImGuiCond_Once);
          ImGui::Button(ICON_FA_ARROWS "##SysDrag");
          ImGui::EndDragDropSource();
        } else if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("(drag to swap chips)"));
        }
        if (ImGui::BeginDragDropTarget()) {
          const ImGuiPayload* dragItem=ImGui::AcceptDragDropPayload("FUR_SYS");
          if (dragItem!=NULL) {
            if (dragItem->IsDataType("FUR_SYS")) {
              if (sysToMove!=i && sysToMove>=0) {
                e->swapSystem(sysToMove,i,preserveChanPos);
                MARK_MODIFIED;
              }
              sysToMove=-1;
            }
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::TableNextColumn();
        bool isNotCollapsed=true;
        if (ImGui::TreeNode(fmt::sprintf("%d. %s##_SYSM%d",i+1,getSystemName(e->song.system[i]),i).c_str())) {
          drawSysConf(i,i,e->song.system[i],e->song.systemFlags[i],true);
          isNotCollapsed=false;
          ImGui::TreePop();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary) && isNotCollapsed) {
          if (e->song.system[i]!=DIV_SYSTEM_NULL) {
            const DivSysDef* sysDef=e->getSystemDef(e->song.system[i]);
            if (ImGui::BeginTooltip()) {
              ImGui::Dummy(ImVec2(MIN(scrW*dpiScale,400.0f*dpiScale),0.0f));
              ImGui::PushTextWrapPos(MIN(scrW*dpiScale,400.0f*dpiScale)); // arbitrary constant
              ImGui::TextWrapped("%s",sysDef->description);
              ImGui::Separator();
              drawSystemChannelInfoText(sysDef);
              drawSystemChannelInfo(sysDef);
              ImGui::PopTextWrapPos();
              ImGui::EndTooltip();
            }
          }
        }
        ImGui::TableNextColumn();
        if (ImGui::Button(_("Clone##SysDup"))) {
          if (!e->duplicateSystem(i,sysDupCloneChannels,sysDupEnd)) {
            showError(fmt::sprintf(_("cannot clone chip! (%s)"),e->getLastError()));
          } else {
            if (e->song.autoSystem) {
              autoDetectSystem();
              updateWindowTitle();
            }
            updateROMExportAvail();
            MARK_MODIFIED;
          }
        }
        ImGui::SameLine();
        ImGui::Button(_("Change##SysChange"));
        if (ImGui::BeginPopupContextItem("SysPickerC",ImGuiPopupFlags_MouseButtonLeft)) {
          DivSystem picked=systemPicker(false);
          if (picked!=DIV_SYSTEM_NULL) {
            if (e->changeSystem(i,picked,preserveChanPos)) {
              MARK_MODIFIED;
              if (e->song.autoSystem) {
                autoDetectSystem();
              }
              updateWindowTitle();
              updateROMExportAvail();
            } else {
              showError(fmt::sprintf(_("cannot change chip! (%s)"),e->getLastError()));
            }
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(e->song.systemLen<=1);
        pushDestColor();
        if (ImGui::Button(ICON_FA_TIMES "##SysRemove")) {
          sysToDelete=i;
          showWarning(_("Are you sure you want to remove this chip?"),GUI_WARN_SYSTEM_DEL);
        }
        popDestColor();
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Remove"));
        }
        ImGui::EndDisabled();
        ImGui::PopID();
      }
      if (e->song.systemLen<DIV_MAX_CHIPS) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();
        ImGui::Button(ICON_FA_PLUS "##SysAdd");
        if (ImGui::BeginPopupContextItem("SysPickerA",ImGuiPopupFlags_MouseButtonLeft)) {
          DivSystem picked=systemPicker(false);
          if (picked!=DIV_SYSTEM_NULL) {
            if (!e->addSystem(picked)) {
              showError(fmt::sprintf(_("cannot add chip! (%s)"),e->getLastError()));
            } else {
              MARK_MODIFIED;
            }
            if (e->song.autoSystem) {
              autoDetectSystem();
            }
            updateWindowTitle();
            updateROMExportAvail();
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_SYS_MANAGER;
  ImGui::End();
}
