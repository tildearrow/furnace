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
#include <set>
#include "tiaExporter.h"
#include "../../ta-log.h"

const int AUDC0 = 0x15;
const int AUDC1 = 0x16;
const int AUDF0 = 0x17;
const int AUDF1 = 0x18;
const int AUDV0 = 0x19;
const int AUDV1 = 0x1A;
 
struct TiaRegisters {

  unsigned char audc0;
  unsigned char audc1;
  unsigned char audf0;
  unsigned char audf1;
  unsigned char audv0;
  unsigned char audv1;

  bool write(const DivRegWrite& registerWrite);

};

bool TiaRegisters::write(const DivRegWrite& registerWrite) {
  const unsigned char val = registerWrite.val;
  switch (registerWrite.addr) {
    case AUDC0:
      if (val == audc0) return false;
      audc0 = val;
      return true;
    case AUDC1:
      if (val == audc1) return false;
      audc1 = val;
      return true;
    case AUDF0:
      if (val == audf0) return false;
      audf0 = val;
      return true;
    case AUDF1:
      if (val == audf1) return false;
      audf1 = val;
      return true;
    case AUDV0:
      if (val == audv0) return false;
      audv0 = val;
      return true;
    case AUDV1:
      if (val == audv1) return false;
      audv1 = val;
      return true;
  }
  return false;
}

struct PatternIndex {
  unsigned short subsong, chan, ord, pat;
  PatternIndex(
    unsigned short s,
    unsigned short c,
    unsigned short o,
    unsigned short p):
    subsong(s),
    chan(c),
    ord(o),
    pat(p) {}
};

struct RowIndex{
  unsigned short subsong, ord, row;
  RowIndex(unsigned short s, unsigned short o, unsigned short r):
    subsong(s),
    ord(o),
    row(r) {}
  bool advance(unsigned short s, unsigned short o, unsigned short r)
  {
    bool changed = false;
    if (subsong != s) {
      subsong = s;
      changed = true;
    }
    if (ord != o) {
      ord = o;
      changed = true;
    }
    if (row != r) {
      row = r;
      changed = true;
    }
    return changed;
  }  
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
  std::vector<PatternIndex> patterns;
  bool alreadyAdded[256];
  for (int i = 0; i < e->getChannelCount(DIV_SYSTEM_TIA); i++) {
    for (size_t j = 0; j < e->song.subsong.size(); j++) {
      DivSubSong* subs = e->song.subsong[j];
      memset(alreadyAdded, 0, 256*sizeof(bool));
      for (int k = 0; k < subs->ordersLen; k++) {
        unsigned short p = subs->orders.ord[i][k];
        if (alreadyAdded[p]) continue;
        patterns.push_back(PatternIndex(j, i, k, p));
        alreadyAdded[p] = true;
      }
    }
  }

  // emit patterns
  // we emit the "note" being played as an assembly variable including the instrument being played
  // later we will figure out what we need to emit as far as TIA register settings
  // this assumes the song has a limited number of unique "notes"
  std::set<String> waveFormSet;
  const char* waveForms[4][2][4][256];
  memset(&waveForms, 0, 8192);
  for (PatternIndex& patternInfo: patterns) {
    DivPattern* pat = e->song.subsong[patternInfo.subsong]->pat[patternInfo.chan].getPattern(patternInfo.pat, false);
    w->writeText(fmt::sprintf("; Subsong: %d Channel: %d Pattern: %d / %s\n", patternInfo.subsong, patternInfo.chan, patternInfo.pat, pat->name));
    String key = fmt::sprintf("PAT_S%02x_C%02x_P%02x", patternInfo.subsong, patternInfo.chan, patternInfo.pat);
    w->writeText(fmt::sprintf("%s = . - AUDIO_TRACKS", key));
    for (int j = 0; j<e->song.subsong[patternInfo.subsong]->patLen; j++) {
      if (j % 8 == 0) {
        w->writeText("\n    byte ");
      } else {
        w->writeText(",");
      }
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
        
      auto it = waveFormSet.emplace(key);
      if (it.second) {
        waveForms[patternInfo.subsong][patternInfo.ord][patternInfo.chan][j] = it.first->c_str();
      }
      w->writeText(key);
    }
    w->writeText(fmt::sprintf("\n    byte 255, %s\n", key));
  }
  // emit waveform table
  // this is where we can lookup specific instrument/note/octave combinations
  w->writeC('\n');
  w->writeText("; Waveform Lookup Table\n");
  w->writeText("    ALIGN 256\n");
  w->writeText("WF_TABLE_START\n");
  for (auto it = waveFormSet.begin(); it != waveFormSet.end(); ++it) {
    w->writeText(fmt::sprintf("%s = . - WF_TABLE_START\n", *it));
    w->writeText(fmt::sprintf("   word %s_ADDR\n", *it, *it));
  }
    
  // emit waveform data
  // this is done by playing back the song from the unique notes
  


  w->writeC('\n');
  w->writeText("; Waveforms\n");
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
  for (int channel = 0; channel < 2; channel++) {
    e->stop();
    e->setRepeatPattern(false);
    e->setOrder(0);
    e->play();
    
    RowIndex curRowIndex(e->getCurrentSubSong(), e->getOrder(), e->getRow());
    const char *key = waveForms[curRowIndex.subsong][curRowIndex.ord][channel][curRowIndex.row];
    if (NULL != key) {
      logI("got key %s", key);
      writeWaveformHeader(w, key);
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
      if (curRowIndex.advance(e->getCurrentSubSong(), e->getOrder(), e->getRow())) {
        if (NULL != key) {
          w->writeText(fmt::sprintf("  byte %d, 255\n", tick - lastWriteTick));
          lastWriteTick = tick;
        }
        logI("advancing %d %d %d %d", curRowIndex.subsong, curRowIndex.ord, curRowIndex.row, channel);
        key = waveForms[curRowIndex.subsong][curRowIndex.ord][channel][curRowIndex.row];
        if (NULL != key) {
          logI("got key %s", key);
          writeWaveformHeader(w, key);
        }
      }
      // get register dumps
      for (int i=0; i<e->song.systemLen; i++) {
        bool isDirty = false;
        std::vector<DivRegWrite>& registerWrites=e->getDispatch(i)->getRegisterWrites();
        for (DivRegWrite& registerWrite: registerWrites) {
          switch (registerWrite.addr) {
            case AUDC0:
            case AUDF0:
            case AUDV0:
              if (1 == channel) continue;
              break;
            case AUDC1:
            case AUDF1:
            case AUDV1:
              if (0 == channel) continue;
              break;
            default:
              continue;
          }
          isDirty |= currentState.write(registerWrite);
        }
        registerWrites.clear();
        if (NULL != key && isDirty) {
          // end last seq
          int deltaTick = tick - lastWriteTick;
          if (lastWriteTick > 0 && deltaTick > 0) {
            w->writeText(fmt::sprintf("  byte %d\n", deltaTick));
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
    if (NULL != key) {
      // final seq
      w->writeText(fmt::sprintf("  byte %d, 255\n", tick - lastWriteTick));
    }
  }
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }

}

void R9TrackerBuilder::writeWaveformHeader(SafeWriter* w, const char * key) {
  w->writeText(fmt::sprintf("%s_ADDR\n", key));
}