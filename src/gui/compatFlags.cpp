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
#include "imgui.h"
#include "intConst.h"

void FurnaceGUI::drawCompatFlags() {
  if (nextWindow==GUI_WINDOW_COMPAT_FLAGS) {
    compatFlagsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!compatFlagsOpen) return;
  if (ImGui::Begin("Compatibility Flags",&compatFlagsOpen,globalWinFlags)) {
    ImGui::TextWrapped("these flags are designed to provide better DefleMask/older Furnace compatibility.");
    ImGui::Checkbox("Limit slide range",&e->song.limitSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, slides are limited to a compatible range.\nmay cause problems with slides in negative octaves.");
    }
    InvCheckbox("Compatible noise layout on NES and PC Engine",&e->song.properNoiseLayout);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("use a rather unusual compatible noise frequency layout.\nremoves some noise frequencies on PC Engine.");
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
    InvCheckbox("Don't reset slides after note off",&e->song.noteOffResetsSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, note off will not reset the channel's slide effect.");
    }
    InvCheckbox("Don't reset portamento after reaching target",&e->song.targetResetsSlides);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, the slide effect will not be disabled after it reaches its target.");
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
      ImGui::SetTooltip("when enabled, a one-tick note cut will be inserted between non-legato/non-portamento notes.\nthis simulates the behavior of some Amiga/SNES music engines.\n\nineffective on C64.");
    }
    ImGui::Checkbox("Broken speed alternation",&e->song.brokenSpeedSel);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("determines next speed based on whether the row is odd/even instead of alternating between speeds.");
    }
    ImGui::Checkbox("Don't slide on the first tick of a row",&e->song.noSlidesOnFirstTick);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("simulates ProTracker's behavior of not applying volume/pitch slides on the first tick of a row.");
    }
    ImGui::Checkbox("Reset arpeggio position on row change",&e->song.rowResetsArpPos);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("simulates ProTracker's behavior of arpeggio being bound to the current tick of a row.");
    }
    ImGui::Checkbox("Ignore 0Dxx on the last order",&e->song.ignoreJumpAtEnd);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if this is on, a jump to next row effect will not take place when it is on the last order of a song.");
    }
    ImGui::Checkbox("Buggy portamento after pitch slide",&e->song.buggyPortaAfterSlide);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("simulates a bug in where portamento does not work after sliding.");
    }
    ImGui::Checkbox("FM pitch slide octave boundary odd behavior",&e->song.fbPortaPause);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if this is on, a pitch slide that crosses the octave boundary will stop for one tick and then continue from the nearest octave boundary.\nfor .dmf compatibility.");
    }
    InvCheckbox("Don't apply Game Boy envelope on note-less instrument change",&e->song.gbInsAffectsEnvelope);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if this is on, an instrument change will not affect the envelope.");
    }
    ImGui::Checkbox("Ignore DAC mode change outside of intended channel in ExtCh mode",&e->song.ignoreDACModeOutsideIntendedChannel);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if this is on, 17xx has no effect on the operator channels in YM2612.");
    }
    ImGui::Checkbox("E1xy/E2xy also take priority over slide stops",&e->song.e1e2AlsoTakePriority);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("does this make any sense by now?");
    }
    ImGui::Checkbox("E1xy/E2xy stop when repeating the same note",&e->song.e1e2StopOnSameNote);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("ugh, if only this wasn't a thing...");
    }
    ImGui::Checkbox("SN76489 duty macro always resets phase",&e->song.snDutyReset);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, duty macro will always reset phase, even if its value hasn't changed.");
    }
    InvCheckbox("Pitch macro is not linear",&e->song.pitchMacroIsLinear);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, the pitch macro of an instrument is in frequency/period space.");
    }
    InvCheckbox("Broken volume scaling strategy",&e->song.newVolumeScaling);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled:\n- log scaling: multiply\n- linear scaling: subtract\nwhen disabled:\n- log scaling: subtract\n- linear scaling: multiply");
    }
    InvCheckbox("Don't persist volume macro after it finishes",&e->song.volMacroLinger);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, a value in the volume column that happens after the volume macro is done will disregard the macro.");
    }
    ImGui::Checkbox("Broken output volume on instrument change",&e->song.brokenOutVol);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("if enabled, no checks for the presence of a volume macro will be made.\nthis will cause the last macro value to linger unless a value in the volume column is present.");
    }
    ImGui::Checkbox("Broken output volume - Episode 2 (PLEASE KEEP ME DISABLED)",&e->song.brokenOutVol2);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("these compatibility flags are getting SO damn ridiculous and out of control.\nas you may have guessed, this one exists due to yet ANOTHER DefleMask-specific behavior.\nplease keep this off at all costs, because I will not support it when ROM export comes.\noh, and don't start an argument out of it. Furnace isn't a DefleMask replacement, and no,\nI am not trying to make it look like one with all these flags.\n\noh, and what about the other flags that don't have to do with DefleMask?\nthose are for .mod import, future FamiTracker import and personal taste!\n\nend of rant");
    }
    ImGui::Checkbox("Treat SN76489 periods under 8 as 1",&e->song.snNoLowPeriods);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("when enabled, any SN period under 8 will be written as 1 instead.\nthis replicates DefleMask behavior, but reduces available period range.");
    }

    ImGui::Text("Pitch linearity:");
    if (ImGui::RadioButton("None",e->song.linearPitch==0)) {
      e->song.linearPitch=0;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("like ProTracker/FamiTracker");
    }
    if (ImGui::RadioButton("Partial (only 04xy/E5xx)",e->song.linearPitch==1)) {
      e->song.linearPitch=1;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("like DefleMask");
    }
    if (ImGui::RadioButton("Full",e->song.linearPitch==2)) {
      e->song.linearPitch=2;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("like Impulse Tracker");
    }

    if (e->song.linearPitch==2) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120.0f*dpiScale);
      if (ImGui::InputScalar("Pitch slide speed multiplier",ImGuiDataType_U8,&e->song.pitchSlideSpeed,&_ONE,&_ONE)) {
        if (e->song.pitchSlideSpeed<1) e->song.pitchSlideSpeed=1;
        if (e->song.pitchSlideSpeed>64) e->song.pitchSlideSpeed=64;
      }
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

    ImGui::Text("Cut/delay effect policy:");
    if (ImGui::RadioButton("Strict",e->song.delayBehavior==0)) {
      e->song.delayBehavior=0;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("only when time is less than speed (like DefleMask/ProTracker)");
    }
    if (ImGui::RadioButton("Strict (old)",e->song.delayBehavior==1)) {
      e->song.delayBehavior=1;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("only when time is less than or equal to speed (original buggy behavior)");
    }
    if (ImGui::RadioButton("Lax",e->song.delayBehavior==2)) {
      e->song.delayBehavior=2;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("no checks (like FamiTracker)");
    }

    ImGui::Text("Simultaneous jump (0B+0D) treatment:");
    if (ImGui::RadioButton("Normal",e->song.jumpTreatment==0)) {
      e->song.jumpTreatment=0;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("accept 0B+0D to jump to a specific row of an order");
    }
    if (ImGui::RadioButton("Old Furnace",e->song.jumpTreatment==1)) {
      e->song.jumpTreatment=1;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("only accept the first jump effect");
    }
    if (ImGui::RadioButton("DefleMask",e->song.jumpTreatment==2)) {
      e->song.jumpTreatment=2;
    }
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("only accept 0Dxx");
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
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    InvCheckbox("Don't allow instrument change during slides",&e->song.newInsTriggersInPorta);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    InvCheckbox("Don't reset note to base on arpeggio stop",&e->song.arp0Reset);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    InvCheckbox("ExtCh channel status is not shared among operators",&e->song.sharedExtStat);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    InvCheckbox("Disable new SegaPCM features (macros and better panning)",&e->song.newSegaPCM);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    ImGui::Checkbox("Old FM octave boundary behavior",&e->song.oldOctaveBoundary);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    ImGui::Checkbox("Disable OPN2 DAC volume control",&e->song.noOPN2Vol);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1");
    }
    ImGui::Checkbox("Broken initial position of portamento after arpeggio",&e->song.brokenPortaArp);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre1.5");
    }
    ImGui::Checkbox("Disable new sample features",&e->song.disableSampleMacro);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre2");
    }
    ImGui::Checkbox("Old arpeggio macro + pitch slide strategy",&e->song.oldArpStrategy);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("behavior changed in 0.6pre2");
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_COMPAT_FLAGS;
  ImGui::End();
}
