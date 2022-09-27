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

#include <fmt/printf.h>
#include "tiaExporter.h"

struct PatternInfo {
  unsigned short subsong, chan, pat;
  PatternInfo(unsigned short s, unsigned short c, unsigned short p):
    subsong(s),
    chan(c),
    pat(p) {}
};

SafeWriter* FurnaceTrackerROMBuilder::buildROM(int sysIndex) {

  SafeWriter* w=new SafeWriter;
  w->init();
  w->writeText("# Data exported from Furnace to TIATracker data track.\n");
  w->writeText(fmt::sprintf("# Song: %s\n", e->song.name));
  w->writeText(fmt::sprintf("# Author: %s\n", e->song.author));
// TODO: never populated?
//   w->writeText(fmt::sprintf("# Composer: %s\n", e->song.composer));
//   w->writeText(fmt::sprintf("# Arranger: %s\n", e->song.arranger));
//   w->writeText(fmt::sprintf("# Copyright: %s\n", e->song.copyright));
//   w->writeText(fmt::sprintf("# Created Date: %s\n", e->song.createdDate));
  w->writeText("ft_InsAUDCxTable\n");
  for (int i = 0; i < e->song.insLen; i++) {
    DivInstrument* ins = e->getIns(i, DIV_INS_TIA);
    w->writeText(fmt::sprintf("ft_Ins%d_AUDCx ; Waveforms for %s", i, ins->name));
    for (int j = 0; j < 1; j++) { // TODO: multiple waveforms? ins->std.waveMacro.len; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      w->writeText(fmt::format_int(ins->std.waveMacro.val[j]).c_str());
    }
    w->writeC('\n');
  }
  w->writeText("ft_InsAUDFxAUDVxTable\n");
  for (int i = 0; i < e->song.insLen; i++) {
    DivInstrument* ins = e->getIns(i, DIV_INS_TIA);
    w->writeText(fmt::sprintf("ft_Ins%d_AUDFxAUDVx ; FreqVol for %s", i, ins->name));
    for (int j = 0; j < ins->std.volMacro.len; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      short freqVol = (ins->std.volMacro.val[j] << 4) + ins->std.pitchMacro.val[j];
      w->writeText(fmt::format_int(freqVol).c_str());
    }
    w->writeC('\n');
  }

  // borrow from fileops - pull patterns to write
  std::vector<PatternInfo> patterns;
  bool alreadyAdded[256];
  for (int i = 0; i < e->getChannelCount(DIV_SYSTEM_TIA); i++) {
    for (size_t j = 0; j < e->song.subsong.size(); j++) {
      DivSubSong* subs = e->song.subsong[j];
      memset(alreadyAdded, 0, 256*sizeof(bool));
      for (int k = 0; k < subs->ordersLen; k++) {
        if (alreadyAdded[subs->orders.ord[i][k]]) continue;
        patterns.push_back(PatternInfo(j, i, subs->orders.ord[i][k]));
        alreadyAdded[subs->orders.ord[i][k]] = true;
      }
    }
  }

  for (PatternInfo& i: patterns) {
    DivPattern* pat = e->song.subsong[i.subsong]->pat[i.chan].getPattern(i.pat, false);
    w->writeText(fmt::sprintf("# Subsong: %d Channel: %d Pattern: %d / %s\n", i.subsong, i.chan, i.pat, pat->name));
    w->writeText(fmt::sprintf("ft_subsong%d_chan%d_pat%d_start", i.subsong, i.chan, i.pat));
    for (int j=0; j<e->song.subsong[i.subsong]->patLen; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      short note = pat->data[j][0];
      //short octave = pat->data[j][1];
      short instrument = pat->data[j][2];
      //short volume = pat->data[j][3]; 
      // TODO: effects 
      // TODO: check instrument size
      // TODO: define percussions
      // write instrument to bits 7..5, frequency to bits 0..4 
      char bits = (((0x07 & instrument) + 1) << 5) + (0xf & note); // TODO: note_frequency(note, octave);
      w->writeText(fmt::sprintf("%d", bits));
    }
  }

  return w;
}
