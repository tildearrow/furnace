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

void FurnaceGUI::insEditKlattsch(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  if (ImGui::BeginTabItem("klattsch")) {
    ImGui::TextWrapped(_("These defaults shape the phoneme-bank voice. Pattern effects override them; reset values return to this profile."));
    ImGui::Separator();

    P(CWSliderScalar(_("Transition time (ticks)"),ImGuiDataType_U8,&ins->klattsch.transition,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE)); rightClickable

    String valueText=(ins->klattsch.voicing==0xff)?_("bank value"):fmt::sprintf("%.3f",(float)ins->klattsch.voicing/255.0f);
    P(CWSliderScalar(_("Voicing"),ImGuiDataType_U8,&ins->klattsch.voicing,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    valueText=(ins->klattsch.aspiration==0xff)?_("bank value"):fmt::sprintf("%.3f",(float)ins->klattsch.aspiration/255.0f);
    P(CWSliderScalar(_("Aspiration"),ImGuiDataType_U8,&ins->klattsch.aspiration,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    if (ins->klattsch.tilt==0xff) {
      valueText=_("bank value");
    } else {
      int tilt=(ins->klattsch.tilt<0x80)?ins->klattsch.tilt:(ins->klattsch.tilt-0x100);
      valueText=fmt::sprintf("%d",tilt);
    }
    P(CWSliderScalar(_("Spectral tilt"),ImGuiDataType_U8,&ins->klattsch.tilt,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    valueText=(ins->klattsch.effort==0xff)?_("bank value"):fmt::sprintf("%.3f",(float)ins->klattsch.effort/255.0f);
    P(CWSliderScalar(_("Glottal effort"),ImGuiDataType_U8,&ins->klattsch.effort,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    valueText=(ins->klattsch.gain==0)?_("bank value"):fmt::sprintf("%.3f",(float)ins->klattsch.gain/16.0f);
    P(CWSliderScalar(_("Gain"),ImGuiDataType_U8,&ins->klattsch.gain,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    valueText=(ins->klattsch.bandwidth==0)?_("neutral"):fmt::sprintf("%.3f",(float)ins->klattsch.bandwidth/64.0f);
    P(CWSliderScalar(_("Formant bandwidth scale"),ImGuiDataType_U8,&ins->klattsch.bandwidth,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    valueText=(ins->klattsch.formantShift==0)?_("neutral"):fmt::sprintf("%.3f",(float)ins->klattsch.formantShift/64.0f);
    P(CWSliderScalar(_("Formant shift"),ImGuiDataType_U8,&ins->klattsch.formantShift,&_ZERO,&_TWO_HUNDRED_FIFTY_FIVE,valueText.c_str())); rightClickable

    ImGui::SeparatorText(_("Modulation defaults"));
    int vibRate=(ins->klattsch.vibrato>>4)&15;
    int vibDepth=ins->klattsch.vibrato&15;
    if (CWSliderInt(_("Vibrato rate (Hz)"),&vibRate,0,15)) {
      ins->klattsch.vibrato=(vibRate<<4)|vibDepth;
      PARAMETER
    }
    if (CWSliderInt(_("Vibrato depth (4 Hz steps)"),&vibDepth,0,15)) {
      ins->klattsch.vibrato=(vibRate<<4)|vibDepth;
      PARAMETER
    }

    int tremRate=(ins->klattsch.tremolo>>4)&15;
    int tremDepth=ins->klattsch.tremolo&15;
    if (CWSliderInt(_("Tremolo rate (Hz)"),&tremRate,0,15)) {
      ins->klattsch.tremolo=(tremRate<<4)|tremDepth;
      PARAMETER
    }
    if (CWSliderInt(_("Tremolo depth"),&tremDepth,0,15)) {
      ins->klattsch.tremolo=(tremRate<<4)|tremDepth;
      PARAMETER
    }

    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,255,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (left)"),&ins->std.panLMacro,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER],false,NULL));
    macroList.push_back(FurnaceGUIMacroDesc(_("Panning (right)"),&ins->std.panRMacro,0,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));
    macroList.push_back(FurnaceGUIMacroDesc(_("Voicing"),&ins->std.ex1Macro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Aspiration"),&ins->std.ex2Macro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Spectral Tilt"),&ins->std.ex3Macro,-128,127,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Glottal Effort"),&ins->std.ex4Macro,0,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Formant Shift"),&ins->std.ex5Macro,1,255,160,uiColors[GUI_COLOR_MACRO_OTHER]));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}