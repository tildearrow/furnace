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
#include "IconsFontAwesome4.h"
#include "../plot_nolerp.h"

const char* singleWSEffects[7]={
  _N("None"),
  _N("Invert"),
  _N("Add"),
  _N("Subtract"),
  _N("Average"),
  _N("Phase"),
  _N("Chorus")
};

const char* dualWSEffects[9]={
  _N("None (dual)"),
  _N("Wipe"),
  _N("Fade"),
  _N("Fade (ping-pong)"),
  _N("Overlay"),
  _N("Negative Overlay"),
  _N("Slide"),
  _N("Mix Chorus"),
  _N("Phase Modulation")
};

void FurnaceGUI::insTabWavetable(DivInstrument* ins)
{
  if (ImGui::BeginTabItem(_("Wavetable"))) {
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
      case DIV_INS_GBA_DMA:
        wavePreviewLen=ins->amiga.waveLen+1;
        wavePreviewHeight=255;
        break;
      case DIV_INS_SNES:
        wavePreviewLen=ins->amiga.waveLen+1;
        wavePreviewHeight=15;
        break;
      case DIV_INS_GBA_MINMOD:
        wavePreviewLen=ins->amiga.waveLen+1;
        wavePreviewHeight=255;
        break;
      case DIV_INS_SID3:
        wavePreviewLen=256;
        wavePreviewHeight=255;
        break;
      default:
        wavePreviewLen=32;
        wavePreviewHeight=31;
        break;
    }
    if (ImGui::Checkbox(_("Enable synthesizer"),&ins->ws.enabled)) { PARAMETER
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
        ImGui::Text(_("Single-waveform"));
        ImGui::Indent();
        for (int i=0; i<DIV_WS_SINGLE_MAX; i++) {
          if (ImGui::Selectable(_(singleWSEffects[i]),ins->ws.effect==i)) { PARAMETER
            ins->ws.effect=i;
            wavePreviewInit=true;
          }
          if (ins->ws.effect==i) ImGui::SetItemDefaultFocus();
        }
        ImGui::Unindent();
        ImGui::Text(_("Dual-waveform"));
        ImGui::Indent();
        for (int i=129; i<DIV_WS_DUAL_MAX; i++) {
          if (ImGui::Selectable(_(dualWSEffects[i-128]),ins->ws.effect==i)) { PARAMETER
            ins->ws.effect=i;
            wavePreviewInit=true;
          }
          if (ins->ws.effect==i) ImGui::SetItemDefaultFocus();
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
          wavePreviewAccum=0.0f;
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
          if (wavePreviewInit) {
            wavePreview.tick(true);
          } else {
            wavePreviewAccum+=ImGui::GetIO().DeltaTime;
            double accumRate=e->getCurHz();
            if (accumRate<1.0) accumRate=1.0;
            if (accumRate>1023.0) accumRate=1023.0;
            accumRate=1.0/accumRate;
            while (wavePreviewAccum>=accumRate) {
              wavePreviewAccum-=accumRate;
              wavePreview.tick(true);
            }
          }
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
        if (ins->std.waveMacro.len>0) {
          ImGui::PushStyleColor(ImGuiCol_Text,uiColors[GUI_COLOR_WARNING]);
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Wave 1"));
          ImGui::SameLine();
          ImGui::Text(ICON_FA_EXCLAMATION_TRIANGLE);
          ImGui::PopStyleColor();
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("waveform macro is controlling wave 1!\nthis value will be ineffective."));
          }
        } else {
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Wave 1"));
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputInt("##SelWave1",&ins->ws.wave1,1,4)) { PARAMETER
          if (ins->ws.wave1<0) ins->ws.wave1=0;
          if (ins->ws.wave1>=(int)e->song.wave.size()) ins->ws.wave1=e->song.wave.size()-1;
          wavePreviewInit=true;
        }
        if (ins->std.waveMacro.len>0) {
          if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip(_("waveform macro is controlling wave 1!\nthis value will be ineffective."));
          }
        }
        if (isSingleWaveFX) {
          ImGui::TableNextColumn();
          ImGui::AlignTextToFramePadding();
          ImGui::Text(_("Wave 2"));
          ImGui::SameLine();
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          if (ImGui::InputInt("##SelWave2",&ins->ws.wave2,1,4)) { PARAMETER
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
            ImGui::SetTooltip(_("Resume preview"));
          } else {
            ImGui::SetTooltip(_("Pause preview"));
          }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_REPEAT "##WSRestart")) {
          wavePreviewInit=true;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Restart preview"));
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_UPLOAD "##WSCopy")) {
          curWave=e->addWave();
          if (curWave==-1) {
            showError(_("too many wavetables!"));
          } else {
            wantScrollListWave=true;
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
          ImGui::SetTooltip(_("Copy to new wavetable"));
        }
        ImGui::SameLine();
        ImGui::Text("(%d×%d)",wavePreviewLen,wavePreviewHeight+1);
        ImGui::EndTable();
      }

      if (ImGui::InputScalar(_("Update Rate"),ImGuiDataType_U8,&ins->ws.rateDivider,&_ONE,&_EIGHT)) { PARAMETER
        wavePreviewInit=true;
      }
      int speed=ins->ws.speed+1;
      if (ImGui::InputInt(_("Speed"),&speed,1,8)) { PARAMETER
        if (speed<1) speed=1;
        if (speed>256) speed=256;
        ins->ws.speed=speed-1;
        wavePreviewInit=true;
      }

      if (ImGui::InputScalar(_("Amount"),ImGuiDataType_U8,&ins->ws.param1,&_ONE,&_EIGHT)) { PARAMETER
        wavePreviewInit=true;
      }

      if (ins->ws.effect==DIV_WS_PHASE_MOD) {
        if (ImGui::InputScalar(_("Power"),ImGuiDataType_U8,&ins->ws.param2,&_ONE,&_EIGHT)) { PARAMETER
          wavePreviewInit=true;
        }
      }

      if (ImGui::Checkbox(_("Global"),&ins->ws.global)) { PARAMETER
        wavePreviewInit=true;
      }
    } else {
      ImGui::TextWrapped(_("wavetable synthesizer disabled.\nuse the Waveform macro to set the wave for this instrument."));
    }

    ImGui::EndTabItem();
  }
}

