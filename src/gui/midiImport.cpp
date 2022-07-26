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
#include "../ta-log.h"
#include <MidiFile.h>

using namespace smf;

void FurnaceGUI::midiImport(MidiFile* midifile, int midiChannel, int midiTrack, int midiStartMeasure, int targetChannelIdx, int patternIdx, \
                            int patternCount,  bool enableCC = false, bool enableVel = true, bool enableNoteOff = true) {
  logD("Importing MIDI channel %d in track %d starting from measure %d into channel %d, pattern %d",
    midiChannel, midiTrack, midiStartMeasure, targetChannelIdx, patternIdx
  );

  if (midiChannel < 0) {
    logD("Listening to all MIDI channels...");
  }

  MidiEventList events = (*midifile)[midiTrack];
  int eventCount = events.getEventCount();
  logD("Number of events in track: %d", eventCount);

  int ticksPerQuarter = midifile->getTicksPerQuarterNote();
  int ticksPerMeasure = ticksPerQuarter * 4 * midiImportSpeedMultiplier;

  logD("Ticks per quarter: %d, ticks per measure: %d", ticksPerQuarter, ticksPerMeasure);

  int tickOffset = ticksPerMeasure * midiStartMeasure;

  int eventOffset = -1;
  for (int i = 0; i < eventCount; i++) {
    if (events[i].tick >= tickOffset) {
      eventOffset = i;
      break;
    }
  }

  if (eventOffset == -1) {
    showError("no MIDI events found after this offset!");
    return;
  }

  int patternsProcessed = 0;

  while (patternsProcessed < patternCount && eventOffset < eventCount) {
    // get pattern
    DivChannelData& chan = e->curPat[targetChannelIdx];
    DivPattern* pat = chan.getPattern(patternIdx, true);

    int patLen = e->curSubSong->patLen;

    // clear pattern by setting note/octave to 0 and inst/vol/effects to -1
    for (int i=0; i<patLen; i++) {
      for (int j=0; j<32; j++) {
        pat->data[i][j] = (j < 2) ? 0 : -1;
      }
    }
    
    for (; eventOffset < eventCount; eventOffset++) {
      if (events[eventOffset].tick >= tickOffset + ticksPerMeasure)
        break;
      MidiEvent event = events[eventOffset];
      
      if (midiChannel < 0 || event.getChannelNibble() == midiChannel) {
        int patternPos = (int)round((event.tick - tickOffset) / (float)ticksPerMeasure * patLen);
        if (patternPos >= patLen) {
          patternPos = patLen - 1;
        }
        if (enableNoteOff && event.isNoteOff()) {
          // don't overwrite existing notes
          if (pat->data[patternPos][0] == 0) {
            pat->data[patternPos][0] = 100;
            pat->data[patternPos][1] = 0;
          }
        } else if (event.isNoteOn()) {
          int pitch = event.getP1();
          int velocity = event.getP2();

          // pitch, octave
          pat->data[patternPos][0] = pitch % 12;
          pat->data[patternPos][1] = pitch / 12;
          if (pat->data[patternPos][0]==0) {
            pat->data[patternPos][0]=12;
            pat->data[patternPos][1]--;
          }

          // velocity
          if (enableVel) {
            int maxVol=e->getMaxVolumeChan(targetChannelIdx);
            if (latchVol!=-1) {
              pat->data[patternPos][3]=MIN(maxVol,latchVol);
            } else if (velocity!=-1) {
              pat->data[patternPos][3]=(velocity*maxVol)/127;
            }
          }
        } else if (enableCC && event.isController()) {
          int ccNum = event.getP1();
          int ccVal = event.getP2();
          // effect, effect val
          pat->data[patternPos][4] = ccNum;
          pat->data[patternPos][5] = ccVal;
        }
      }
    }
    patternsProcessed++;
  }

  logD("MIDI import complete");
}

void FurnaceGUI::midiImportSong() {
  
}

void FurnaceGUI::midiImportBatch(std::map<String, MidiFile> midis) {
  for (auto& kv : midis) {
    String fn = kv.first;
    MidiFile mf = kv.second;
    
    int targetChannel;
    int pattern;

    // if sscanf returns 1, it reached EOF and not every variable was read (incorrect filename), so skip the file
    int eof = sscanf(fn.c_str(), "ch%d_p%d", &targetChannel, &pattern);
    if (eof) { continue; }

    midiImport(&mf, midiImportOmniChannel ? -1 : midiImportChannel-1, midiImportTrack-1, 0, targetChannel, \
               pattern, 1, midiImportEnableCC, midiImportEnableVel, midiImportEnableNoteOff);
  }

  logD("MIDI batch import complete");
}

void FurnaceGUI::drawMidiDialog() {
  if (nextWindow==GUI_WINDOW_MIDI_DIALOG) {
    midiDialogOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!midiDialogOpen) { return; }

  bool midiFileExists = true;
  bool midiFolderExists = true;

  MidiFile* midifile = e->getMidiImportFile();
  if (midifile == NULL) { midiFileExists = false; }

  int tracks = midifile->getTrackCount();
  if (tracks == 0) {
    showError("no MIDI tracks available!");
    return;
  }

  String midifolder = e->getMidiImportBatchDir();
  if (midifolder.empty()) { midiFolderExists = false; }
  
  if (ImGui::Begin("MIDI Import",&midiDialogOpen,globalWinFlags)) {
    if (ImGui::BeginTabBar("midiImportTab")) {
      ImGui::BeginDisabled(midiFileExists);
      if (ImGui::BeginTabItem("Manual")) {
        ImGui::TextWrapped("MIDI file: %s",e->getMidiImportFilename().c_str());

        if (ImGui::BeginTable("MIDIImportSettings",3,ImGuiTableFlags_SizingStretchProp)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
          
          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          float avail=ImGui::GetContentRegionAvail().x;

          ImGui::BeginDisabled(midiImportOmniChannel);
          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI channel",&midiImportChannel);
          if (midiImportChannel < 1) midiImportChannel = 1;
          if (midiImportChannel > 16) midiImportChannel = 16;
          ImGui::EndDisabled();

          ImGui::TableNextColumn();

          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI track",&midiImportTrack);
          if (midiImportTrack < 1) midiImportTrack = 1;
          if (midiImportTrack > tracks) midiImportTrack = tracks;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI start measure",&midiImportStartMeasure);
          if (midiImportStartMeasure < 1) midiImportStartMeasure = 1;
          // if (midiImportStartMeasure > 255) midiImportStartMeasure = 255;

          ImGui::TableNextColumn();

          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI speed multiplier",&midiImportSpeedMultiplier);
          if (midiImportSpeedMultiplier < 1) midiImportSpeedMultiplier = 1;
          if (midiImportSpeedMultiplier > 16) midiImportSpeedMultiplier = 16;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Listen to all MIDI channels", &midiImportOmniChannel);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Import note off messages", &midiImportEnableNoteOff);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Import velocity", &midiImportEnableVel);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Import CC messages", &midiImportEnableCC);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::SetNextItemWidth(avail * 0.4);
          if (ImGui::BeginCombo("Target channel", e->getChannelName(midiImportTargetChannel))) {
            for (int i = 0; i < e->getTotalChannelCount(); i++) {
              if (ImGui::Selectable(e->getChannelName(i),midiImportTargetChannel==i)) {
                midiImportTargetChannel = i;
              }
            }
            ImGui::EndCombo();
          }

          ImGui::TableNextColumn();
          
          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("Pattern",&midiImportPattern);
          if (midiImportPattern < 0) midiImportPattern = 0;
          if (midiImportPattern > 255) midiImportPattern = 255;
          
          ImGui::EndTable();
        }

        ImGui::Separator();
        if (ImGui::Button("Import##MidiImport")) {
          midiImport(midifile, midiImportOmniChannel ? -1 : midiImportChannel-1, midiImportTrack-1, midiImportStartMeasure-1, \
                    midiImportTargetChannel, midiImportPattern, 1, midiImportEnableCC, midiImportEnableVel, midiImportEnableNoteOff);
        }
        ImGui::SameLine();
        if (ImGui::Button("Close##MidiClose")) {
          midiDialogOpen=false;
        }
        
        ImGui::EndTabItem();
      }
      ImGui::EndDisabled();

      ImGui::BeginDisabled(midiFolderExists);
      if (ImGui::BeginTabItem("Automatic")) {
        ImGui::TextWrapped("MIDI folder: %s",e->getMidiImportBatchDir().c_str());

        if (ImGui::BeginTable("MIDIImportSettings",3,ImGuiTableFlags_SizingStretchProp)) {
          ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthStretch,0.0);
          ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthStretch,0.0);
          
          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          float avail=ImGui::GetContentRegionAvail().x;

          ImGui::BeginDisabled(midiImportOmniChannel);
          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI channel",&midiImportChannel);
          if (midiImportChannel < 1) midiImportChannel = 1;
          if (midiImportChannel > 16) midiImportChannel = 16;
          ImGui::EndDisabled();

          ImGui::TableNextColumn();

          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI track",&midiImportTrack);
          if (midiImportTrack < 1) midiImportTrack = 1;
          if (midiImportTrack > tracks) midiImportTrack = tracks;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::SetNextItemWidth(avail * 0.4);
          ImGui::InputInt("MIDI speed multiplier",&midiImportSpeedMultiplier);
          if (midiImportSpeedMultiplier < 1) midiImportSpeedMultiplier = 1;
          if (midiImportSpeedMultiplier > 16) midiImportSpeedMultiplier = 16;

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Listen to all MIDI channels", &midiImportOmniChannel);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Import note off messages", &midiImportEnableNoteOff);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Import velocity", &midiImportEnableVel);

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Checkbox("Import CC messages", &midiImportEnableCC);

          ImGui::EndTable();
        }

        ImGui::Separator();
        if (ImGui::Button("Batch import##MidiBatchImport")) {
          midiImportBatch(e->getMidiImportBatchFiles());
        }
        ImGui::SameLine();
        if (ImGui::Button("Close##MidiBatchClose")) {
          midiDialogOpen=false;
        }
        
        ImGui::EndTabItem();
      }
      ImGui::EndDisabled();

      ImGui::EndTabBar();
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_MIDI_DIALOG;

    ImGui::End();
  }
}
