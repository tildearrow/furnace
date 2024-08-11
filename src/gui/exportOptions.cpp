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
#include "guiConst.h"
#include "../fileutils.h"
#include "misc/cpp/imgui_stdlib.h"
#include <imgui.h>

void FurnaceGUI::drawExportAudio(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(_("Export type:"));

  ImGui::Indent();
  if (ImGui::RadioButton(_("one file"),audioExportOptions.mode==DIV_EXPORT_MODE_ONE)) {
    audioExportOptions.mode=DIV_EXPORT_MODE_ONE;
  }
  if (ImGui::RadioButton(_("multiple files (one per chip)"),audioExportOptions.mode==DIV_EXPORT_MODE_MANY_SYS)) {
    audioExportOptions.mode=DIV_EXPORT_MODE_MANY_SYS;
        }
  if (ImGui::RadioButton(_("multiple files (one per channel)"),audioExportOptions.mode==DIV_EXPORT_MODE_MANY_CHAN)) {
    audioExportOptions.mode=DIV_EXPORT_MODE_MANY_CHAN;
  }
  ImGui::Unindent();

  if (audioExportOptions.mode!=DIV_EXPORT_MODE_MANY_SYS) {
    ImGui::Text(_("Bit depth:"));
    ImGui::Indent();
    if (ImGui::RadioButton(_("16-bit integer"),audioExportOptions.format==DIV_EXPORT_FORMAT_S16)) {
      audioExportOptions.format=DIV_EXPORT_FORMAT_S16;
    }
    if (ImGui::RadioButton(_("32-bit float"),audioExportOptions.format==DIV_EXPORT_FORMAT_F32)) {
      audioExportOptions.format=DIV_EXPORT_FORMAT_F32;
    }
    ImGui::Unindent();
  }

  if (ImGui::InputInt(_("Sample rate"),&audioExportOptions.sampleRate,100,10000)) {
    if (audioExportOptions.sampleRate<8000) audioExportOptions.sampleRate=8000;
    if (audioExportOptions.sampleRate>384000) audioExportOptions.sampleRate=384000;
  }

  if (audioExportOptions.mode!=DIV_EXPORT_MODE_MANY_SYS) {
    if (ImGui::InputInt(_("Channels in file"),&audioExportOptions.chans,1,1)) {
      if (audioExportOptions.chans<1) audioExportOptions.chans=1;
      if (audioExportOptions.chans>16) audioExportOptions.chans=16;
    }
  }

  if (ImGui::InputInt(_("Loops"),&audioExportOptions.loops,1,2)) {
    if (audioExportOptions.loops<0) audioExportOptions.loops=0;
  }
  if (ImGui::InputDouble(_("Fade out (seconds)"),&audioExportOptions.fadeOut,1.0,2.0,"%.1f")) {
    if (audioExportOptions.fadeOut<0.0) audioExportOptions.fadeOut=0.0;
  }

  bool isOneOn=false;
  if (audioExportOptions.mode==DIV_EXPORT_MODE_MANY_CHAN) {
    ImGui::Text(_("Channels to export:"));
    ImGui::SameLine();
    if (ImGui::SmallButton(_("All"))) {
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        audioExportOptions.channelMask[i]=true;
      }
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(_("None"))) {
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        audioExportOptions.channelMask[i]=false;
      }
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(_("Shown in pattern"))) {
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        audioExportOptions.channelMask[i]=e->curSubSong->chanShow[i];
      }
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(_("Shown in oscilloscope"))) {
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        audioExportOptions.channelMask[i]=e->curSubSong->chanShowChanOsc[i];
      }
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(_("Invert"))) {
      for (int i=0; i<DIV_MAX_CHANS; i++) {
        audioExportOptions.channelMask[i]=!audioExportOptions.channelMask[i];
      }
    }

    if (ImGui::BeginChild("Channel Selection",ImVec2(0,200.0f*dpiScale))) {
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        String name=fmt::sprintf("%d. %s##_CE%d",i+1,e->getChannelName(i),i);
        ImGui::Checkbox(name.c_str(),&audioExportOptions.channelMask[i]);
        if (audioExportOptions.channelMask[i]) isOneOn=true;
      }
    }
    ImGui::EndChild();
  } else {
    isOneOn=true;
  }

  if (onWindow) {
    ImGui::Separator();
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }

  if (isOneOn) {
    if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
      switch (audioExportOptions.mode) {
        case DIV_EXPORT_MODE_ONE:
          openFileDialog(GUI_FILE_EXPORT_AUDIO_ONE);
          break;
        case DIV_EXPORT_MODE_MANY_SYS:
          openFileDialog(GUI_FILE_EXPORT_AUDIO_PER_SYS);
          break;
        case DIV_EXPORT_MODE_MANY_CHAN:
          openFileDialog(GUI_FILE_EXPORT_AUDIO_PER_CHANNEL);
          break;
      }
      ImGui::CloseCurrentPopup();
    }
  } else {
    ImGui::Text(_("select at least one channel"));
  }
}

void FurnaceGUI::drawExportVGM(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(_("settings:"));
  if (ImGui::BeginCombo(_("format version"),fmt::sprintf("%d.%.2x",vgmExportVersion>>8,vgmExportVersion&0xff).c_str())) {
    for (int i=0; i<7; i++) {
      if (ImGui::Selectable(fmt::sprintf("%d.%.2x",vgmVersions[i]>>8,vgmVersions[i]&0xff).c_str(),vgmExportVersion==vgmVersions[i])) {
        vgmExportVersion=vgmVersions[i];
      }
    }
    ImGui::EndCombo();
  }
  ImGui::Checkbox(_("loop"),&vgmExportLoop);
  if (vgmExportLoop && e->song.loopModality==2) {
    ImGui::Text(_("loop trail:"));
    ImGui::Indent();
    if (ImGui::RadioButton(_("auto-detect"),vgmExportTrailingTicks==-1)) {
      vgmExportTrailingTicks=-1;
    }
    if (ImGui::RadioButton(_("add one loop"),vgmExportTrailingTicks==-2)) {
      vgmExportTrailingTicks=-2;
    }
    if (ImGui::RadioButton(_("custom"),vgmExportTrailingTicks>=0)) {
      vgmExportTrailingTicks=0;
    }
    if (vgmExportTrailingTicks>=0) {
      ImGui::SameLine();
      if (ImGui::InputInt("##TrailTicks",&vgmExportTrailingTicks,1,100)) {
        if (vgmExportTrailingTicks<0) vgmExportTrailingTicks=0;
      }
    }
    ImGui::Unindent();
  }
  ImGui::Checkbox(_("add pattern change hints"),&vgmExportPatternHints);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip(_(
      "inserts data blocks on pattern changes.\n"
      "useful if you are writing a playback routine.\n\n"

      "the format of a pattern change data block is:\n"
      "67 66 FE ll ll ll ll 01 oo rr pp pp pp ...\n"
      "- ll: length, a 32-bit little-endian number\n"
      "- oo: order\n"
      "- rr: initial row (a 0Dxx effect is able to select a different row)\n"
      "- pp: pattern index (one per channel)\n\n"

      "pattern indexes are ordered as they appear in the song."
    ));
  }
  ImGui::Checkbox(_("direct stream mode"),&vgmExportDirectStream);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip(_(
      "required for DualPCM and MSM6258 export.\n\n"
      "allows for volume/direction changes when playing samples,\n"
      "at the cost of a massive increase in file size."
    ));
  }
  ImGui::Text(_("chips to export:"));
  bool hasOneAtLeast=false;
  for (int i=0; i<e->song.systemLen; i++) {
    int minVersion=e->minVGMVersion(e->song.system[i]);
    ImGui::BeginDisabled(minVersion>vgmExportVersion || minVersion==0);
    ImGui::Checkbox(fmt::sprintf("%d. %s##_SYSV%d",i+1,getSystemName(e->song.system[i]),i).c_str(),&willExport[i]);
    ImGui::EndDisabled();
    if (minVersion>vgmExportVersion) {
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(_("this chip is only available in VGM %d.%.2x and higher!"),minVersion>>8,minVersion&0xff);
      }
    } else if (minVersion==0) {
      if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip(_("this chip is not supported by the VGM format!"));
      }
    } else {
      if (willExport[i]) hasOneAtLeast=true;
    }
  }
  ImGui::Text(_("select the chip you wish to export, but only up to %d of each type."),(vgmExportVersion>=0x151)?2:1);
  if (hasOneAtLeast) {
    if (onWindow) {
      ImGui::Separator();
      if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
      ImGui::SameLine();
    }
    if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
      openFileDialog(GUI_FILE_EXPORT_VGM);
      ImGui::CloseCurrentPopup();
    }
  } else {
    ImGui::Text(_("nothing to export"));
    if (onWindow) {
      ImGui::Separator();
      if (ImGui::Button(_("Cancel"),ImVec2(400.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    }
  }
}

void FurnaceGUI::drawExportZSM(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(_("Commander X16 Zsound Music File"));
  if (ImGui::InputInt(_("Tick Rate (Hz)"),&zsmExportTickRate,1,2)) {
    if (zsmExportTickRate<1) zsmExportTickRate=1;
    if (zsmExportTickRate>44100) zsmExportTickRate=44100;
  }
  ImGui::Checkbox(_("loop"),&zsmExportLoop);
  ImGui::SameLine();
  ImGui::Checkbox(_("optimize size"),&zsmExportOptimize);
  if (onWindow) {
    ImGui::Separator();
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }
  if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
    openFileDialog(GUI_FILE_EXPORT_ZSM);
    ImGui::CloseCurrentPopup();
  }
}

void FurnaceGUI::drawExportTiuna(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(_("for use with TIunA driver. outputs asm source."));
  ImGui::InputText(_("base song label name"),&asmBaseLabel); // TODO: validate label
  if (ImGui::InputInt(_("max size in first bank"),&tiunaFirstBankSize,1,100)) {
    if (tiunaFirstBankSize<0) tiunaFirstBankSize=0;
    if (tiunaFirstBankSize>4096) tiunaFirstBankSize=4096;
  }
  if (ImGui::InputInt(_("max size in other banks"),&tiunaOtherBankSize,1,100)) {
    if (tiunaOtherBankSize<16) tiunaOtherBankSize=16;
    if (tiunaOtherBankSize>4096) tiunaOtherBankSize=4096;
  }
  
  ImGui::Text(_("chips to export:"));
  int selected=0;
  for (int i=0; i<e->song.systemLen; i++) {
    DivSystem sys=e->song.system[i];
    bool isTIA=sys==DIV_SYSTEM_TIA;
    ImGui::BeginDisabled((!isTIA) || (selected>=1));
    ImGui::Checkbox(fmt::sprintf("%d. %s##_SYSV%d",i+1,getSystemName(e->song.system[i]),i).c_str(),&willExport[i]);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
      if (!isTIA) {
        ImGui::SetTooltip(_("this chip is not supported by the file format!"));
      } else if (selected>=1) {
        ImGui::SetTooltip(_("only one Atari TIA is supported!"));
      }
    }
    if (isTIA && willExport[i]) selected++;
  }
  if (selected>0) {
    if (onWindow) {
      ImGui::Separator();
      if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
      ImGui::SameLine();
    }
    if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
      openFileDialog(GUI_FILE_EXPORT_TIUNA);
      ImGui::CloseCurrentPopup();
    }
  } else {
    ImGui::Text(_("nothing to export"));
    if (onWindow) {
      ImGui::Separator();
      if (ImGui::Button(_("Cancel"),ImVec2(400.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    }
  }
}

void FurnaceGUI::drawExportAmigaVal(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(_(
    "this is NOT ROM export! only use for making sure the\n"
    "Furnace Amiga emulator is working properly by\n"
    "comparing it with real Amiga output."
  ));
  ImGui::AlignTextToFramePadding();
  ImGui::Text(_("Directory"));
  ImGui::SameLine();
  ImGui::InputText("##AVDPath",&workingDirROMExport);
  if (onWindow) {
    ImGui::Separator();
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }
  if (ImGui::Button(_("Bake Data"),ImVec2(200.0f*dpiScale,0))) {
    DivROMExport* ex=e->buildROM(DIV_ROM_AMIGA_VALIDATION);
    if (ex->go(e)) {
      ex->wait();
      if (ex->hasFailed()) {
        showError("error!");
      } else {
        if (workingDirROMExport.size()>0) {
          if (workingDirROMExport[workingDirROMExport.size()-1]!=DIR_SEPARATOR) workingDirROMExport+=DIR_SEPARATOR_STR;
        }
        for (DivROMExportOutput& i: ex->getResult()) {
          String path=workingDirROMExport+i.name;
          FILE* outFile=ps_fopen(path.c_str(),"wb");
          if (outFile!=NULL) {
            fwrite(i.data->getFinalBuf(),1,i.data->size(),outFile);
            fclose(outFile);
          }
          i.data->finish();
          delete i.data;
        }
        showError(fmt::sprintf(_("Done! Baked %d files."),(int)ex->getResult().size()));
      }
    } else {
      showError("error!");
    }
    delete ex;
    ImGui::CloseCurrentPopup();
  }
}

void FurnaceGUI::drawExportText(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(
    _("this option exports the song to a text file.\n")
  );
  if (onWindow) {
    ImGui::Separator();
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }
  if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
    openFileDialog(GUI_FILE_EXPORT_TEXT);
    ImGui::CloseCurrentPopup();
  }
}

void FurnaceGUI::drawExportCommand(bool onWindow) {
  exitDisabledTimer=1;
  
  ImGui::Text(_(
    "this option exports a text or binary file which\n"
    "contains a dump of the internal command stream\n"
    "produced when playing the song.\n\n"

    "technical/development use only!"
  ));
  if (onWindow) {
    ImGui::Separator();
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }
  if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
    openFileDialog(GUI_FILE_EXPORT_CMDSTREAM);
    ImGui::CloseCurrentPopup();
  }
}

void FurnaceGUI::drawExportDMF(bool onWindow) {
  exitDisabledTimer=1;

  ImGui::Text(_(
    "export in DefleMask module format.\n"
    "only do it if you really, really need to, or are downgrading an existing .dmf."
  ));

  ImGui::Text(_("format version:"));
  ImGui::RadioButton(_("1.1.3 and higher"),&dmfExportVersion,0);
  ImGui::RadioButton(_("1.0/legacy (0.12)"),&dmfExportVersion,1);

  if (onWindow) {
    ImGui::Separator();
    if (ImGui::Button(_("Cancel"),ImVec2(200.0f*dpiScale,0))) ImGui::CloseCurrentPopup();
    ImGui::SameLine();
  }
  if (ImGui::Button(_("Export"),ImVec2(200.0f*dpiScale,0))) {
    if (dmfExportVersion==1) {
      openFileDialog(GUI_FILE_SAVE_DMF_LEGACY);
    } else {
      openFileDialog(GUI_FILE_SAVE_DMF);
    }
    ImGui::CloseCurrentPopup();
  }
}

void FurnaceGUI::drawExport() {
  if (settings.exportOptionsLayout==1 || curExportType==GUI_EXPORT_NONE) {
    if (ImGui::BeginTabBar("ExportTypes")) {
      if (ImGui::BeginTabItem(_("Audio"))) {
        drawExportAudio(true);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("VGM"))) {
        drawExportVGM(true);
        ImGui::EndTabItem();
      }
      int numZSMCompat=0;
      for (int i=0; i<e->song.systemLen; i++) {
        if ((e->song.system[i]==DIV_SYSTEM_VERA) || (e->song.system[i]==DIV_SYSTEM_YM2151)) numZSMCompat++;
      }
      if (numZSMCompat>0) {
        if (ImGui::BeginTabItem(_("ZSM"))) {
          drawExportZSM(true);
          ImGui::EndTabItem();
        }
      }
      bool hasTiunaCompat=false;
      for (int i=0; i<e->song.systemLen; i++) {
        if (e->song.system[i]==DIV_SYSTEM_TIA) {
          hasTiunaCompat=true;
          break;
        }
      }
      if (hasTiunaCompat) {
        if (ImGui::BeginTabItem("TIunA")) {
          drawExportTiuna(true);
          ImGui::EndTabItem();
        }
      }
      int numAmiga=0;
      for (int i=0; i<e->song.systemLen; i++) {
        if (e->song.system[i]==DIV_SYSTEM_AMIGA) numAmiga++;
      }
      if (numAmiga && settings.iCannotWait) {
        if (ImGui::BeginTabItem(_("Amiga Validation"))) {
          drawExportAmigaVal(true);
          ImGui::EndTabItem();
        }
      }
      if (ImGui::BeginTabItem(_("Text"))) {
        drawExportText(true);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("Command Stream"))) {
        drawExportCommand(true);
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("DMF"))) {
        drawExportDMF(true);
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  } else switch (curExportType) {
    case GUI_EXPORT_AUDIO:
      drawExportAudio(true);
      break;
    case GUI_EXPORT_VGM:
      drawExportVGM(true);
      break;
    case GUI_EXPORT_ZSM:
      drawExportZSM(true);
      break;
    case GUI_EXPORT_TIUNA:
      drawExportTiuna(true);
      break;
    case GUI_EXPORT_AMIGA_VAL:
      drawExportAmigaVal(true);
      break;
    case GUI_EXPORT_TEXT:
      drawExportText(true);
      break;
    case GUI_EXPORT_CMD_STREAM:
      drawExportCommand(true);
      break;
    case GUI_EXPORT_DMF:
      drawExportDMF(true);
      break;
    default:
      ImGui::Text(_("congratulations! you've unlocked a secret panel."));
      if (ImGui::Button(_("Toggle hidden systems"))) {
        settings.hiddenSystems=!settings.hiddenSystems;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button(_("Toggle all instrument types"))) {
        settings.displayAllInsTypes=!settings.displayAllInsTypes;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button(_("Set pitch linearity to Partial"))) {
        e->song.linearPitch=1;
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button(_("Set fat to max"))) {
        ImGuiStyle& sty=ImGui::GetStyle();
        sty.FramePadding=ImVec2(20.0f*dpiScale,20.0f*dpiScale);
        sty.ItemSpacing=ImVec2(10.0f*dpiScale,10.0f*dpiScale);
        sty.ItemInnerSpacing=ImVec2(10.0f*dpiScale,10.0f*dpiScale);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button(_("Set muscle and fat to zero"))) {
        ImGuiStyle& sty=ImGui::GetStyle();
        sty.FramePadding=ImVec2(0,0);
        sty.ItemSpacing=ImVec2(0,0);
        sty.ItemInnerSpacing=ImVec2(0,0);
        ImGui::CloseCurrentPopup();
      }
      if (ImGui::Button(_("Tell tildearrow this must be a mistake"))) {
        showError(_("yeah, it's a bug. write a bug report in the GitHub page and tell me how did you get here."));
        ImGui::CloseCurrentPopup();
      }
      break;
  }
}
