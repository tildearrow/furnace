/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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
  if (ImGui::Begin("Compatibility Flags",&compatFlagsOpen,globalWinFlags,_("Compatibility Flags"))) {
    ImGui::TextWrapped(_("these flags are designed to provide better DefleMask/older Furnace compatibility.\nit is recommended to disable most of these unless you rely on specific quirks."));
    if (ImGui::BeginTabBar("settingsTab")) {
      if (ImGui::BeginTabItem(_("DefleMask"))) {
        if (ImGui::Checkbox(_("Limit slide range"),&e->song.limitSlides)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, slides are limited to a compatible range.\nmay cause problems with slides in negative octaves."));
        }
        if (InvCheckbox(_("Compatible noise layout on NES and PC Engine"),&e->song.properNoiseLayout)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("use a rather unusual compatible noise frequency layout.\nremoves some noise frequencies on PC Engine."));
        }
        if (ImGui::Checkbox(_("Game Boy instrument duty is wave volume"),&e->song.waveDutyIsVol)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if enabled, an instrument with duty macro in the wave channel will be mapped to wavetable volume."));
        }

        if (ImGui::Checkbox(_("Restart macro on portamento"),&e->song.resetMacroOnPorta)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, a portamento effect will reset the channel's macro if used in combination with a note."));
        }
        if (ImGui::Checkbox(_("Legacy volume slides"),&e->song.legacyVolumeSlides)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("simulate glitchy volume slide behavior by silently overflowing the volume when the slide goes below 0."));
        }
        if (ImGui::Checkbox(_("Compatible arpeggio"),&e->song.compatibleArpeggio)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("delay arpeggio by one tick on every new note."));
        }
        if (ImGui::Checkbox(_("Disable DAC when sample ends"),&e->song.brokenDACMode)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, the DAC in YM2612 will be disabled if there isn't any sample playing."));
        }
        if (ImGui::Checkbox(_("Broken speed alternation"),&e->song.brokenSpeedSel)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("determines next speed based on whether the row is odd/even instead of alternating between speeds."));
        }
        if (ImGui::Checkbox(_("Ignore duplicate slide effects"),&e->song.ignoreDuplicateSlides)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if this is on, only the first slide of a row in a channel will be considered."));
        }
        if (ImGui::Checkbox(_("Ignore 0Dxx on the last order"),&e->song.ignoreJumpAtEnd)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if this is on, a jump to next row effect will not take place when it is on the last order of a song."));
        }
        if (ImGui::Checkbox(_("Buggy portamento after pitch slide"),&e->song.buggyPortaAfterSlide)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("simulates a bug in where portamento does not work after sliding."));
        }
        if (ImGui::Checkbox(_("FM pitch slide octave boundary odd behavior"),&e->song.fbPortaPause)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if this is on, a pitch slide that crosses the octave boundary will stop for one tick and then continue from the nearest octave boundary.\nfor .dmf compatibility."));
        }
        if (InvCheckbox(_("Don't apply Game Boy envelope on note-less instrument change"),&e->song.gbInsAffectsEnvelope)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if this is on, an instrument change will not affect the envelope."));
        }
        if (ImGui::Checkbox(_("Ignore DAC mode change outside of intended channel in ExtCh mode"),&e->song.ignoreDACModeOutsideIntendedChannel)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if this is on, 17xx has no effect on the operator channels in YM2612."));
        }
        if (ImGui::Checkbox(_("E1xy/E2xy also take priority over slide stops"),&e->song.e1e2AlsoTakePriority)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("does this make any sense by now?"));
        }
        if (ImGui::Checkbox(_("E1xy/E2xy stop when repeating the same note"),&e->song.e1e2StopOnSameNote)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("ugh, if only this wasn't a thing..."));
        }
        if (ImGui::Checkbox(_("SN76489 duty macro always resets phase"),&e->song.snDutyReset)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, duty macro will always reset phase, even if its value hasn't changed."));
        }
        if (InvCheckbox(_("Broken volume scaling strategy"),&e->song.newVolumeScaling)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled:\n- log scaling: multiply\n- linear scaling: subtract\nwhen disabled:\n- log scaling: subtract\n- linear scaling: multiply"));
        }
        if (InvCheckbox(_("Don't persist volume macro after it finishes"),&e->song.volMacroLinger)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, a value in the volume column that happens after the volume macro is done will disregard the macro."));
        }
        if (ImGui::Checkbox(_("Broken output volume on instrument change"),&e->song.brokenOutVol)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("if enabled, no checks for the presence of a volume macro will be made.\nthis will cause the last macro value to linger unless a value in the volume column is present."));
        }
        if (ImGui::Checkbox(_("Broken output volume - Episode 2 (PLEASE KEEP ME DISABLED)"),&e->song.brokenOutVol2)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("these compatibility flags are getting SO damn ridiculous and out of control.\nas you may have guessed, this one exists due to yet ANOTHER DefleMask-specific behavior.\nplease keep this off at all costs, because I will not support it when ROM export comes.\noh, and don't start an argument out of it. Furnace isn't a DefleMask replacement, and no,\nI am not trying to make it look like one with all these flags.\n\noh, and what about the other flags that don't have to do with DefleMask?\nthose are for .mod import, future FamiTracker import and personal taste!\n\nend of rant"));
        }
        if (ImGui::Checkbox(_("Treat SN76489 periods under 8 as 1"),&e->song.snNoLowPeriods)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, any SN period under 8 will be written as 1 instead.\nthis replicates DefleMask behavior, but reduces available period range."));
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("Old Furnace"))) {
        if (ImGui::Checkbox(_("Arpeggio inhibits non-porta slides"),&e->song.arpNonPorta)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.5.5"));
        }
        if (ImGui::Checkbox(_("Wack FM algorithm macro"),&e->song.algMacroBehavior)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.5.5"));
        }
        if (ImGui::Checkbox(_("Broken shortcut slides (E1xy/E2xy)"),&e->song.brokenShortcutSlides)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.5.7"));
        }
        if (ImGui::Checkbox(_("Stop portamento on note off"),&e->song.stopPortaOnNoteOff)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (InvCheckbox(_("Don't allow instrument change during slides"),&e->song.newInsTriggersInPorta)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (InvCheckbox(_("Don't reset note to base on arpeggio stop"),&e->song.arp0Reset)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (InvCheckbox(_("ExtCh channel status is not shared among operators"),&e->song.sharedExtStat)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (InvCheckbox(_("Disable new SegaPCM features (macros and better panning)"),&e->song.newSegaPCM)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (ImGui::Checkbox(_("Old FM octave boundary behavior"),&e->song.oldOctaveBoundary)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (ImGui::Checkbox(_("Disable OPN2 DAC volume control"),&e->song.noOPN2Vol)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1"));
        }
        if (ImGui::Checkbox(_("Broken initial position of portamento after arpeggio"),&e->song.brokenPortaArp)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre1.5"));
        }
        if (ImGui::Checkbox(_("Disable new sample features"),&e->song.disableSampleMacro)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre2"));
        }
        if (ImGui::Checkbox(_("Old arpeggio macro + pitch slide strategy"),&e->song.oldArpStrategy)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre2"));
        }
        if (ImGui::Checkbox(_("Broken portamento during legato"),&e->song.brokenPortaLegato)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre4"));
        }
        if (ImGui::Checkbox(_("Broken macros in some FM chips after note off"),&e->song.brokenFMOff)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre5"));
        }
        if (ImGui::Checkbox(_("Pre-note does not take effects into consideration"),&e->song.preNoteNoEffect)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6pre9"));
        }
        if (ImGui::Checkbox(_("Disable new NES DPCM features"),&e->song.oldDPCM)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6.1"));
        }
        if (ImGui::Checkbox(_("Legacy technical ALWAYS_SET_VOLUME behavior"),&e->song.oldAlwaysSetVolume)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6.1\nthis flag will be removed if I find out that none of the songs break after disabling it."));
        }
        if (ImGui::Checkbox(_("Old sample offset effect"),&e->song.oldSampleOffset)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("behavior changed in 0.6.3"));
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_(".mod import"))) {
        if (ImGui::Checkbox(_("Don't slide on the first tick of a row"),&e->song.noSlidesOnFirstTick)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("simulates ProTracker's behavior of not applying volume/pitch slides on the first tick of a row."));
        }
        if (ImGui::Checkbox(_("Reset arpeggio position on row change"),&e->song.rowResetsArpPos)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("simulates ProTracker's behavior of arpeggio being bound to the current tick of a row."));
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("Pitch/Playback"))) {
        ImGui::Text(_("Pitch linearity:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("None"),!e->song.linearPitch)) {
          e->song.linearPitch=0;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("like ProTracker/FamiTracker"));
        }
        if (ImGui::RadioButton(_("Full"),e->song.linearPitch)) {
          e->song.linearPitch=1;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("like Impulse Tracker"));
        }
        ImGui::Unindent();

        if (e->song.linearPitch) {
          ImGui::SameLine();
          ImGui::SetNextItemWidth(120.0f*dpiScale);
          if (ImGui::InputScalar(_("Pitch slide speed multiplier"),ImGuiDataType_U8,&e->song.pitchSlideSpeed,&_ONE,&_ONE)) {
            if (e->song.pitchSlideSpeed<1) e->song.pitchSlideSpeed=1;
            if (e->song.pitchSlideSpeed>64) e->song.pitchSlideSpeed=64;
            MARK_MODIFIED;
          }
        }

        ImGui::Text(_("Loop modality:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Reset channels"),e->song.loopModality==0)) {
          e->song.loopModality=0;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("select to reset channels on loop. may trigger a voltage click on every loop!"));
        }
        if (ImGui::RadioButton(_("Soft reset channels"),e->song.loopModality==1)) {
          e->song.loopModality=1;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("select to turn channels off on loop."));
        }
        if (ImGui::RadioButton(_("Do nothing"),e->song.loopModality==2)) {
          e->song.loopModality=2;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("select to not reset channels on loop."));
        }
        ImGui::Unindent();

        ImGui::Text(_("Cut/delay effect policy:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Strict"),e->song.delayBehavior==0)) {
          e->song.delayBehavior=0;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("only when time is less than speed (like DefleMask/ProTracker)"));
        }
        if (ImGui::RadioButton(_("Strict (old)"),e->song.delayBehavior==1)) {
          e->song.delayBehavior=1;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("only when time is less than or equal to speed (original buggy behavior)"));
        }
        if (ImGui::RadioButton(_("Lax"),e->song.delayBehavior==2)) {
          e->song.delayBehavior=2;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("no checks"));
        }
        ImGui::Unindent();

        ImGui::Text(_("Simultaneous jump (0B+0D) treatment:"));
        ImGui::Indent();
        if (ImGui::RadioButton(_("Normal"),e->song.jumpTreatment==0)) {
          e->song.jumpTreatment=0;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("accept 0B+0D to jump to a specific row of an order"));
        }
        if (ImGui::RadioButton(_("Old Furnace"),e->song.jumpTreatment==1)) {
          e->song.jumpTreatment=1;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("only accept the first jump effect"));
        }
        if (ImGui::RadioButton(_("DefleMask"),e->song.jumpTreatment==2)) {
          e->song.jumpTreatment=2;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("only accept 0Dxx"));
        }
        ImGui::Unindent();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem(_("Other"))) {
        if (ImGui::Checkbox(_("Auto-insert one tick gap between notes"),&e->song.oneTickCut)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, a one-tick note cut will be inserted between non-legato/non-portamento notes.\nthis simulates the behavior of some Amiga/SNES music engines.\n\nineffective on C64."));
        }

        ImGui::Separator();

        if (InvCheckbox(_("Don't reset slides after note off"),&e->song.noteOffResetsSlides)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, note off will not reset the channel's slide effect."));
        }
        if (InvCheckbox(_("Don't reset portamento after reaching target"),&e->song.targetResetsSlides)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, the slide effect will not be disabled after it reaches its target."));
        }
        if (ImGui::Checkbox(_("Continuous vibrato"),&e->song.continuousVibrato)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, vibrato phase/position will not be reset on a new note."));
        }
        if (InvCheckbox(_("Pitch macro is not linear"),&e->song.pitchMacroIsLinear)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, the pitch macro of an instrument is in frequency/period space."));
        }
        if (ImGui::Checkbox(_("Reset arpeggio effect position on new note"),&e->song.resetArpPhaseOnNewNote)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, arpeggio effect (00xy) position is reset on a new note."));
        }
        if (ImGui::Checkbox(_("Volume scaling rounds up"),&e->song.ceilVolumeScaling)) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("when enabled, volume macros round up when applied\nthis prevents volume scaling from causing vol=0, which is silent on some chips\n\nineffective on logarithmic channels"));
        }
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_COMPAT_FLAGS;
  ImGui::End();
}
