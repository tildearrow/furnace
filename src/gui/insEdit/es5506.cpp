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
#include "../intConst.h"

static const char* es5506FilterModes[4]={
  "HP/K2, HP/K2", "HP/K2, LP/K1", "LP/K2, LP/K2", "LP/K2, LP/K1",
};

static const char* es5506ControlModes[3]={
  _N("pause"),
  _N("reverse"),
  NULL
};

String macroHoverES5506FilterMode(int id, float val, void* u) {
  String mode="???";
  switch (((int)val)&3) {
    case 0:
      mode=_("HP/K2, HP/K2");
      break;
    case 1:
      mode=_("HP/K2, LP/K1");
      break;
    case 2:
      mode=_("LP/K2, LP/K2");
      break;
    case 3:
      mode=_("LP/K2, LP/K1");
      break;
    default:
      break;
  }
  return fmt::sprintf("%d: %s",id,mode);
}


void FurnaceGUI::insEditES5506(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  insTabSample(ins);

  if (ImGui::BeginTabItem("ES5506")) {
    if (ImGui::BeginTable("ESParams",2,ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
      // filter
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Filter Mode"),ImGuiDataType_U8,&ins->es5506.filter.mode,&_ZERO,&_THREE,es5506FilterModes[ins->es5506.filter.mode&3]));
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Filter K1"),ImGuiDataType_U16,&ins->es5506.filter.k1,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Filter K2"),ImGuiDataType_U16,&ins->es5506.filter.k2,&_ZERO,&_SIXTY_FIVE_THOUSAND_FIVE_HUNDRED_THIRTY_FIVE)); rightClickable
      // envelope
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Envelope length"),ImGuiDataType_U16,&ins->es5506.envelope.ecount,&_ZERO,&_FIVE_HUNDRED_ELEVEN)); rightClickable
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Left Volume Ramp"),ImGuiDataType_S8,&ins->es5506.envelope.lVRamp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Right Volume Ramp"),ImGuiDataType_S8,&ins->es5506.envelope.rVRamp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Filter K1 Ramp"),ImGuiDataType_S8,&ins->es5506.envelope.k1Ramp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
      ImGui::TableNextColumn();
      P(CWSliderScalar(_("Filter K2 Ramp"),ImGuiDataType_S8,&ins->es5506.envelope.k2Ramp,&_MINUS_ONE_HUNDRED_TWENTY_EIGHT,&_ONE_HUNDRED_TWENTY_SEVEN)); rightClickable
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Checkbox(_("K1 Ramp Slowdown"),&ins->es5506.envelope.k1Slow);
      ImGui::TableNextColumn();
      ImGui::Checkbox(_("K2 Ramp Slowdown"),&ins->es5506.envelope.k2Slow);
      ImGui::EndTable();
    }
    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(_("Macros"))) {
    float usesAmigaVol=true;
    for (int i=0; i<e->song.systemLen; i++) {
      if (e->song.system[i]==DIV_SYSTEM_ES5506) {
        if (!e->song.systemFlags[i].getBool("amigaVol",false)) {
          usesAmigaVol=false;
          break;
        }
      }
    }
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,usesAmigaVol?64:4095,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Filter Mode"),&ins->std.dutyMacro,0,3,160,uiColors[GUI_COLOR_MACRO_FILTER],false,NULL,&macroHoverES5506FilterMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,4095,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,4095,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Phase Reset"),&ins->std.phaseResetMacro,0,1,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true));
    macroList.push_back(FurnaceGUIMacroDesc(_("Filter K1"),&ins->std.ex1Macro,((ins->std.ex1Macro.mode==1)?(-65535):0),65535,160,uiColors[GUI_COLOR_MACRO_FILTER],false,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Filter K2"),&ins->std.ex2Macro,((ins->std.ex2Macro.mode==1)?(-65535):0),65535,160,uiColors[GUI_COLOR_MACRO_FILTER],false,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Outputs"),&ins->std.fbMacro,0,5,64,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Control"),&ins->std.algMacro,0,2,32,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL,NULL,true,es5506ControlModes));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
