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

#include "gui.h"
#include "imgui.h"
#include "intConst.h"

void FurnaceGUI::drawCompatFlags() {
  if (nextWindow==GUI_WINDOW_COMPAT_FLAGS) {
    compatFlagsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!compatFlagsOpen) return;
  if (ImGui::Begin("Compatibility Flags",&compatFlagsOpen,globalWinFlags,_L("Compatibility Flags###Compatibility Flags"))) {
    ImGui::TextWrapped("these flags are designed to provide better DefleMask/older Furnace compatibility.\nit is recommended to disable most of these unless you rely on specific quirks.");
    if (ImGui::BeginTabBar("settingsTab")) {
      if (ImGui::BeginTabItem("DefleMask")) {
        ImGui::Checkbox("Game Boy instrument duty is wave volume",&e->song.waveDutyIsVol);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("if enabled, an instrument with duty macro in the wave channel will be mapped to wavetable volume.");
        }

        ImGui::Checkbox("Restart macro on portamento",&e->song.resetMacroOnPorta);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, a portamento effect will reset the channel's macro if used in combination with a note.");
        }
        ImGui::Checkbox("Ignore duplicate slide effects",&e->song.ignoreDuplicateSlides);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("if this is on, only the first slide of a row in a channel will be considered.");
        }
        ImGui::Checkbox("Ignore 0Dxx on the last order",&e->song.ignoreJumpAtEnd);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("if this is on, a jump to next row effect will not take place when it is on the last order of a song.");
        }
        InvCheckbox("Don't apply Game Boy envelope on note-less instrument change",&e->song.gbInsAffectsEnvelope);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("if this is on, an instrument change will not affect the envelope.");
        }
        ImGui::Checkbox("Ignore DAC mode change outside of intended channel in ExtCh mode",&e->song.ignoreDACModeOutsideIntendedChannel);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("if this is on, 17xx has no effect on the operator channels in YM2612.");
        }
        ImGui::Checkbox("SN76489 duty macro always resets phase",&e->song.snDutyReset);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, duty macro will always reset phase, even if its value hasn't changed.");
        }
        InvCheckbox("Don't persist volume macro after it finishes",&e->song.volMacroLinger);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, a value in the volume column that happens after the volume macro is done will disregard the macro.");
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(".mod import")) {
        ImGui::Checkbox("Don't slide on the first tick of a row",&e->song.noSlidesOnFirstTick);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("simulates ProTracker's behavior of not applying volume/pitch slides on the first tick of a row.");
        }
        ImGui::Checkbox("Reset arpeggio position on row change",&e->song.rowResetsArpPos);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("simulates ProTracker's behavior of arpeggio being bound to the current tick of a row.");
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Pitch/Playback")) {
        ImGui::Text("Pitch linearity:");
        ImGui::Indent();
        if (ImGui::RadioButton("None",e->song.linearPitch==0)) {
          e->song.linearPitch=0;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("like ProTracker/FamiTracker");
        }
        if (ImGui::RadioButton("Full",e->song.linearPitch==2)) {
          e->song.linearPitch=2;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("like Impulse Tracker");
        }
        ImGui::Unindent();

        if (e->song.linearPitch==2) {
          ImGui::SameLine();
          ImGui::SetNextItemWidth(120.0f*dpiScale);
          if (ImGui::InputScalar("Pitch slide speed multiplier",ImGuiDataType_U8,&e->song.pitchSlideSpeed,&_ONE,&_ONE)) {
            if (e->song.pitchSlideSpeed<1) e->song.pitchSlideSpeed=1;
            if (e->song.pitchSlideSpeed>64) e->song.pitchSlideSpeed=64;
          }
        }

        ImGui::Text("Loop modality:");
        ImGui::Indent();
        if (ImGui::RadioButton("Reset channels",e->song.loopModality==0)) {
          e->song.loopModality=0;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("select to reset channels on loop. may trigger a voltage click on every loop!");
        }
        if (ImGui::RadioButton("Soft reset channels",e->song.loopModality==1)) {
          e->song.loopModality=1;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("select to turn channels off on loop.");
        }
        if (ImGui::RadioButton("Do nothing",e->song.loopModality==2)) {
          e->song.loopModality=2;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("select to not reset channels on loop.");
        }
        ImGui::Unindent();

        ImGui::Text("Cut/delay effect policy:");
        ImGui::Indent();
        if (ImGui::RadioButton("Strict",e->song.delayBehavior==0)) {
          e->song.delayBehavior=0;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("only when time is less than speed (like DefleMask/ProTracker)");
        }
        if (ImGui::RadioButton("Lax",e->song.delayBehavior==2)) {
          e->song.delayBehavior=2;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("no checks");
        }
        ImGui::Unindent();

        ImGui::Text("Simultaneous jump (0B+0D) treatment:");
        ImGui::Indent();
        if (ImGui::RadioButton("Normal",e->song.jumpTreatment==0)) {
          e->song.jumpTreatment=0;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("accept 0B+0D to jump to a specific row of an order");
        }
        ImGui::Unindent();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Other")) {
        ImGui::Checkbox("Auto-insert one tick gap between notes",&e->song.oneTickCut);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, a one-tick note cut will be inserted between non-legato/non-portamento notes.\nthis simulates the behavior of some Amiga/SNES music engines.\n\nineffective on C64.");
        }

        ImGui::Separator();

        InvCheckbox("Don't reset slides after note off",&e->song.noteOffResetsSlides);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, note off will not reset the channel's slide effect.");
        }
        InvCheckbox("Don't reset portamento after reaching target",&e->song.targetResetsSlides);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, the slide effect will not be disabled after it reaches its target.");
        }
        ImGui::Checkbox("Continuous vibrato",&e->song.continuousVibrato);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, vibrato phase/position will not be reset on a new note.");
        }
        InvCheckbox("Pitch macro is not linear",&e->song.pitchMacroIsLinear);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, the pitch macro of an instrument is in frequency/period space.");
        }
        ImGui::Checkbox("Reset arpeggio effect position on new note",&e->song.resetArpPhaseOnNewNote);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, arpeggio effect (00xy) position is reset on a new note.");
        }
        ImGui::Checkbox("Volume scaling rounds up",&e->song.ceilVolumeScaling);
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("when enabled, volume macros round up when applied\nthis prevents volume scaling from causing vol=0, which is silent on some chips\n\nineffective on logarithmic channels");
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_COMPAT_FLAGS;
  ImGui::End();
}