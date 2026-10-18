// Editor de instrumento do BR811 (chip de som do console ficticio Ompreza).

#include "insEditCommon.h"

static String br811WaveformName(int index, float value, void* data) {
  switch ((int)value) {
    case 0: return String(_("Wave A"));
    case 1: return String(_("Wave B"));
    case 2: return String(_("Noise /512"));
    case 3: return String(_("Noise /1024"));
    case 4: return String(_("Noise /2048"));
    case 5: return String(_("Noise (linked to pitch)"));
    case 6: return String(_("Noise /256 (open hat)"));
    case 7: return String(_("Noise /128 (closed hat)"));
  }
  return String("");
}

void FurnaceGUI::insEditBR811(DivInstrument* ins) {
  std::vector<FurnaceGUIMacroDesc> macroList;

  // aba Sample: PCM nativo do canal 5
  insTabSample(ins);

  if (ImGui::BeginTabItem(_("Macros"))) {
    macroList.push_back(FurnaceGUIMacroDesc(_("Volume"),&ins->std.volMacro,0,15,160,uiColors[GUI_COLOR_MACRO_VOLUME]));
    macroList.push_back(FurnaceGUIMacroDesc(_("Arpeggio"),&ins->std.arpMacro,-120,120,160,uiColors[GUI_COLOR_MACRO_PITCH],true,NULL,macroHoverNote,false,NULL,true,ins->std.arpMacro.val));
    macroList.push_back(FurnaceGUIMacroDesc(_("Waveform"),&ins->std.dutyMacro,0,7,160,uiColors[GUI_COLOR_MACRO_WAVE],false,NULL,br811WaveformName));
    macroList.push_back(FurnaceGUIMacroDesc(_("Pitch"),&ins->std.pitchMacro,-2048,2047,160,uiColors[GUI_COLOR_MACRO_PITCH],true,macroRelativeMode));

    drawMacros(macroList,macroEditStateMacros,ins);
    ImGui::EndTabItem();
  }
}
