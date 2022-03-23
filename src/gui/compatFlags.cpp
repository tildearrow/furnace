/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

void FurnaceGUI::drawCompatFlags() {
  if (nextWindow==GUI_WINDOW_COMPAT_FLAGS) {
    compatFlagsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!compatFlagsOpen) return;
  if (ImGui::Begin("Compatibility Flags",&compatFlagsOpen)) {
    ImGui::TextWrapped("these flags are designed to provide better DefleMask/older Furnace compatibility.");
    ImGui::Checkbox("Limit slide range",&e->song.limitSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, slides are limited to a compatible range.\nmay cause problems with slides in negative octaves.");
    }
    ImGui::Checkbox("Linear pitch control",&e->song.linearPitch);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("linear pitch:\n- slides work in frequency/period space\n- E5xx and 04xx effects work in tonality space\nnon-linear pitch:\n- slides work in frequency/period space\n- E5xx and 04xx effects work on frequency/period space");
    }
    ImGui::Checkbox("Proper noise layout on NES and PC Engine",&e->song.properNoiseLayout);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("use a proper noise channel note mapping (0-15) instead of a rather unusual compatible one.\nunlocks all noise frequencies on PC Engine.");
    }
    ImGui::Checkbox("Game Boy instrument duty is wave volume",&e->song.waveDutyIsVol);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if enabled, an instrument with duty macro in the wave channel will be mapped to wavetable volume.");
    }

    ImGui::Checkbox("Restart macro on portamento",&e->song.resetMacroOnPorta);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, a portamento effect will reset the channel's macro if used in combination with a note.");
    }
    ImGui::Checkbox("Legacy volume slides",&e->song.legacyVolumeSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("simulate glitchy volume slide behavior by silently overflowing the volume when the slide goes below 0.");
    }
    ImGui::Checkbox("Compatible arpeggio",&e->song.compatibleArpeggio);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("delay arpeggio by one tick on every new note.");
    }
    ImGui::Checkbox("Reset slides after note off",&e->song.noteOffResetsSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, note off will reset the channel's slide effect.");
    }
    ImGui::Checkbox("Reset portamento after reaching target",&e->song.targetResetsSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, the slide effect is disabled after it reaches its target.");
    }
    ImGui::Checkbox("Ignore duplicate slide effects",&e->song.ignoreDuplicateSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if this is on, only the first slide of a row in a channel will be considered.");
    }
    ImGui::Checkbox("Continuous vibrato",&e->song.continuousVibrato);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, vibrato will not be reset on a new note.");
    }
    ImGui::Checkbox("Broken DAC mode",&e->song.brokenDACMode);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, the DAC in YM2612 will be disabled if there isn't any sample playing.");
    }
    ImGui::Checkbox("Auto-insert one tick gap between notes",&e->song.oneTickCut);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, a one-tick note cut will be inserted between non-legato/non-portamento notes.\nthis simulates the behavior of some Amiga/SNES music engines.");
    }
    ImGui::Checkbox("Broken speed alternation",&e->song.brokenSpeedSel);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("determines next speed based on whether the row is odd/even instead of alternating between speeds.");
    }

    ImGui::Text("Loop modality:");
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

    ImGui::Separator();

    ImGui::TextWrapped("the following flags are for compatibility with older Furnace versions.");

    ImGui::Checkbox("Arpeggio inhibits non-porta slides",&e->song.arpNonPorta);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.5.5");
    }
    ImGui::Checkbox("Wack FM algorithm macro",&e->song.algMacroBehavior);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.5.5");
    }
    ImGui::Checkbox("Broken shortcut slides (E1xy/E2xy)",&e->song.brokenShortcutSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.5.7");
    }
    ImGui::Checkbox("Stop portamento on note off",&e->song.stopPortaOnNoteOff);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6");
    }
    ImGui::Checkbox("Allow instrument change during slides",&e->song.newInsTriggersInPorta);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6");
    }
    ImGui::Checkbox("Reset note to base on arpeggio stop",&e->song.arp0Reset);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6");
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_COMPAT_FLAGS;
  ImGui::End();
}