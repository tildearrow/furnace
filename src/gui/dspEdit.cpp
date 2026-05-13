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

#include "gui.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "../engine/scspdspasm.h"
#include "../engine/platform/scsp.h"
#include "../engine/sysDef.h"

// Find the first SCSP dispatcher in the song. Returns NULL if none.
static DivPlatformSCSP* findSCSPDispatch(DivEngine* e) {
  for (int i=0; i<e->song.systemLen; i++) {
    if (e->song.system[i]==DIV_SYSTEM_SCSP) {
      return (DivPlatformSCSP*)e->getDispatch(i);
    }
  }
  return NULL;
}

void FurnaceGUI::drawSCSPDSP() {
  if (nextWindow==GUI_WINDOW_SCSP_DSP) {
    scspDspOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!scspDspOpen) return;
  if (ImGui::Begin("SCSP DSP",&scspDspOpen,globalWinFlags,_("SCSP DSP"))) {
    DivPlatformSCSP* p=findSCSPDispatch(e);

    if (p==NULL) {
      ImGui::TextWrapped("%s",_("Add a SCSP/Saturn chip to edit the on-chip DSP program."));
    } else {
      // Source editor — large multi-line input. Direct binding to song state.
      ImVec2 avail=ImGui::GetContentRegionAvail();
      avail.y-=ImGui::GetFrameHeightWithSpacing()*4.0f;  // reserve space for controls
      if (avail.y<100.0f*dpiScale) avail.y=100.0f*dpiScale;

      ImGui::InputTextMultiline(
        "##SCSPDSPSource",
        &e->song.scspDspSource,
        avail,
        ImGuiInputTextFlags_AllowTabInput
      );

      // RBL combo (ring-buffer length): 0=8K, 1=16K, 2=32K, 3=64K samples.
      const char* rblLabels[4]={"0 (8K)","1 (16K)","2 (32K)","3 (64K)"};
      int rbl=e->song.scspDspRBL&3;
      ImGui::SetNextItemWidth(150.0f*dpiScale);
      if (ImGui::Combo(_("RBL"),&rbl,rblLabels,4)) {
        e->song.scspDspRBL=(unsigned char)(rbl&3);
      }

      ImGui::SameLine();
      if (ImGui::Button(_("Apply"))) {
        e->reloadSCSPDSP();
      }
      ImGui::SameLine();
      if (ImGui::Button(_("Clear"))) {
        e->song.scspDspSource="";
        e->reloadSCSPDSP();
      }

      // Status line + diagnostics. These come from the last reset() or
      // reloadDSP() call on the platform; surfaces empty if nothing has
      // been compiled yet.
      const std::vector<std::string>& errs=p->getDSPErrors();
      const std::vector<std::string>& warns=p->getDSPWarnings();
      int steps=p->getDSPStepsLoaded();

      if (!errs.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f,0.4f,0.4f,1.0f));
        for (const std::string& e : errs) ImGui::TextWrapped("error: %s",e.c_str());
        ImGui::PopStyleColor();
      } else {
        ImGui::Text(_("loaded: %d step%s"),steps,(steps==1)?"":"s");
      }

      if (!warns.empty()) {
        if (ImGui::TreeNodeEx(
              "warnings",
              ImGuiTreeNodeFlags_DefaultOpen,
              _("warnings (%d)"),(int)warns.size())) {
          for (const std::string& w : warns) ImGui::BulletText("%s",w.c_str());
          ImGui::TreePop();
        }
      }
    }
  }
  ImGui::End();
}
