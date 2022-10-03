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

const int AUDC0 = 0x15;
const int AUDC1 = 0x16;
const int AUDF0 = 0x17;
const int AUDF1 = 0x18;
const int AUDV0 = 0x19;
const int AUDV1 = 0x1A;
 
struct TiaRegisters {

  unsigned short audc0;
  unsigned short audc1;
  unsigned short audf0;
  unsigned short audf1;
  unsigned short audv0;
  unsigned short audv1;

  bool write(const DivRegWrite& registerWrite);

};

bool TiaRegisters::write(const DivRegWrite& registerWrite) {
  switch (registerWrite.addr) {
    case AUDC0:
      audc0 = registerWrite.val & 0xff;
      return true;
    case AUDC1:
      audc1 = registerWrite.val & 0xff;
      return true;
    case AUDF0:
      audf0 = registerWrite.val & 0xff;
      return true;
    case AUDF1:
      audf1 = registerWrite.val & 0xff;
      return true;
    case AUDV0:
      audv0 = registerWrite.val & 0xff;
      return true;
    case AUDV1:
      audv1 = registerWrite.val & 0xff;
      return true;
  }
  return false;
}

struct PatternInfo {
  unsigned short subsong, chan, pat;
  PatternInfo(unsigned short s, unsigned short c, unsigned short p):
    subsong(s),
    chan(c),
    pat(p) {}
};

struct RowInfo {
  PatternInfo pattern;
  unsigned short row;
  RowInfo(PatternInfo p, unsigned short r):
    pattern(p),
    row(r) {}
};

SafeWriter* R9TrackerBuilder::buildROM(int sysIndex) {

  SafeWriter* w=new SafeWriter;
  w->init();
  w->writeText("# Data exported from Furnace to R9 (Tia) data track.\n");
  w->writeText(fmt::sprintf("# Song: %s\n", e->song.name));
  w->writeText(fmt::sprintf("# Author: %s\n", e->song.author));
  // TODO: never populated?
  //   w->writeText(fmt::sprintf("# Composer: %s\n", e->song.composer));
  //   w->writeText(fmt::sprintf("# Arranger: %s\n", e->song.arranger));
  //   w->writeText(fmt::sprintf("# Copyright: %s\n", e->song.copyright));
  //   w->writeText(fmt::sprintf("# Created Date: %s\n", e->song.createdDate));

  writeTrackData(w);

  return w;
}

void R9TrackerBuilder::dumpRegisters(SafeWriter *w) {
  e->stop();
  e->setRepeatPattern(false);
  e->setOrder(0);

  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  e->walkSong(loopOrder, loopRow, loopEnd);

  // play the song ourselves
  e->play();

  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
   
  int tick = 0;
  int lastWriteTick = 0;
  TiaRegisters currentState;
  memset(&currentState, 0, sizeof(currentState));
  bool done=false;
  while (!done) {
    if (e->tick() || !e->isPlaying()) {
      done=true;
      for (int i=0; i<e->song.systemLen; i++) {
        e->getDispatch(i)->getRegisterWrites().clear();
      }
      break;
    }
    tick++;
    // get register dumps
    for (int i=0; i<e->song.systemLen; i++) {
      bool isDirty = false;
      std::vector<DivRegWrite>& registerWrites=e->getDispatch(i)->getRegisterWrites();
      for (DivRegWrite& registerWrite: registerWrites) {
        isDirty |= currentState.write(registerWrite);
      }
      registerWrites.clear();

      if (isDirty) {
        // end last seq
        if (lastWriteTick > 0) {
          w->writeText(fmt::sprintf("  byte %d\n", tick - lastWriteTick));
        }
        // start next seq
        lastWriteTick = tick;
        w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc0, currentState.audf0, currentState.audv0));
        w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc1, currentState.audf1, currentState.audv1));      
      }
    }
  }

  // final seq
  w->writeText(fmt::sprintf("  byte %d\n", tick - lastWriteTick));

  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }

}

void R9TrackerBuilder::writeTrackData(SafeWriter *w) {
  // pull patterns to write
  // borrowed from fileops
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

  // emit patterns
  // we emit the "note" being played as an assembly variable including the instrument being played
  // later we will figure out what we need to emit as far as TIA register settings
  // this assumes the song has a limited number of unique "notes"
  std::map<String, RowInfo> waveForms;
  for (PatternInfo& patternInfo: patterns) {
    DivPattern* pat = e->song.subsong[patternInfo.subsong]->pat[patternInfo.chan].getPattern(patternInfo.pat, false);
    w->writeText(fmt::sprintf("; Subsong: %d Channel: %d Pattern: %d / %s\n", patternInfo.subsong, patternInfo.chan, patternInfo.pat, pat->name));
    w->writeText(fmt::sprintf("PAT_S%d_C%d_P%d", patternInfo.subsong, patternInfo.chan, patternInfo.pat));
    for (int j = 0; j<e->song.subsong[patternInfo.subsong]->patLen; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
      int row = j; // TODO: is this right?
      short note = pat->data[j][0];
      short octave = pat->data[j][1];
      short instrument = pat->data[j][2];
      short volume = pat->data[j][3]; 
      String key = fmt::sprintf(
        "WF_N%d_O%d_I%d_V%d",
        note & 0xff, 
        octave & 0xff,
        instrument & 0xff,
        volume & 0xff);
      waveForms.emplace(key, RowInfo(patternInfo, row));
      w->writeText(key);
    }
    w->writeC('\n');
  }

  // emit waveform table
  // this is where we can lookup specific instrument/note/octave combinations
  w->writeC('\n');
  w->writeText("; Waveform Lookup Table\n");
  w->writeText("    ALIGN 256\n");
  w->writeText("WF_TABLE_START\n");
  for (auto it = waveForms.begin(); it != waveForms.end(); ++it) {
    w->writeText(fmt::sprintf("%s = . - WF_TABLE_START\n", it->first));
    w->writeText(fmt::sprintf("   word %s_ADDR\n", it->first, it->first));
  }
    
  // emit waveform data
  // this is done by playing back the song from the unique notes
  w->writeC('\n');
  w->writeText("; Waveforms\n");
  for (auto it = waveForms.begin(); it != waveForms.end(); ++it) {
    w->writeText(fmt::sprintf("; Waveform %s C%d P%d S%d R%d\n",
     it->first, 
    it->second.pattern.chan,
     it->second.pattern.pat, 
     it->second.pattern.subsong,
    it->second.row));
    w->writeText(fmt::sprintf("%s_ADDR\n", it->first));
    int startRow = it->second.row;
    int endRow = startRow + 1;
    writeWaveform(w, it->second.pattern.chan, startRow, endRow);
  }

}  

void R9TrackerBuilder::writeWaveform(SafeWriter *w, unsigned short channel, int startRow, int endRow) {
  e->stop();
  e->setRepeatPattern(false);
  e->setOrder(0);
  e->playToRow(startRow);
  
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
   
  int tick = 0;
  int lastWriteTick = 0;
  TiaRegisters currentState;
  memset(&currentState, 0, sizeof(currentState));
  bool done=false;
  while (!done) {
    if (e->tick() || !e->isPlaying() || endRow <= e->getCurRow()) {
      done=true;
      for (int i=0; i<e->song.systemLen; i++) {
        e->getDispatch(i)->getRegisterWrites().clear();
      }
      break;
    }
    tick++;
    // get register dumps
    for (int i=0; i<e->song.systemLen; i++) {
      bool isDirty = false;
      std::vector<DivRegWrite>& registerWrites=e->getDispatch(i)->getRegisterWrites();
      for (DivRegWrite& registerWrite: registerWrites) {
        isDirty |= currentState.write(registerWrite);
      }
      registerWrites.clear();

      if (isDirty) {
        // end last seq
        if (lastWriteTick > 0) {
          w->writeText(fmt::sprintf("  byte %d\n", tick - lastWriteTick));
        }
        // start next seq
        lastWriteTick = tick;
        if (0 == channel) {
          w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc0, currentState.audf0, currentState.audv0));
        } else {
          w->writeText(fmt::sprintf("  byte %d,%d,%d\n", currentState.audc1, currentState.audf1, currentState.audv1));
        }
      }
    }
  }

  // final seq
  w->writeText(fmt::sprintf("  byte %d, 255\n", tick - lastWriteTick));

  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }

}

