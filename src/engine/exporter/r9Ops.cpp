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

#include "tiaExporter.h"
#include "r9.h"

#include <fmt/printf.h>
#include <set>
#include "../../ta-log.h"

SafeWriter* R9TrackerBuilder(DivEngine* eng, int sysIndex) {
  R9 r9(eng);
  return r9.buildROM(sysIndex);
}

const int AUDC0 = 0x15;
const int AUDC1 = 0x16;
const int AUDF0 = 0x17;
const int AUDF1 = 0x18;
const int AUDV0 = 0x19;
const int AUDV1 = 0x1A;

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
  String key;
  unsigned short subsong, ord, chan, pat;
  PatternIndex(
    const String& k,
    unsigned short s,
    unsigned short o,
    unsigned short c,
    unsigned short p):
    key(k),
    subsong(s),
    ord(o),
    chan(c),
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

SafeWriter* R9::buildROM(int sysIndex) {

  SafeWriter* w=new SafeWriter;
  w->init();
  w->writeText("; Data exported from Furnace to R9 data track.\n");
  w->writeText(fmt::sprintf("; Song: %s\n", e->song.name));
  w->writeText(fmt::sprintf("; Author: %s\n", e->song.author));

  writeTrackData(w);

  return w;
}

void R9::writeTrackData(SafeWriter *w) {

  // write the orders
  // simultaneously pull patterns to write
  // borrowed from fileops
  w->writeC('\n');
  w->writeText("; orders\n");
  w->writeText("    ALIGN 256\n");
  w->writeText("ORDERS\n");
  std::vector<PatternIndex> patterns;
  bool alreadyAdded[2][256];
  for (size_t i = 0; i < e->song.subsong.size(); i++) {
    DivSubSong* subs = e->song.subsong[i];
    memset(alreadyAdded, 0, 2*256*sizeof(bool));
    for (int j = 0; j < subs->ordersLen; j++) {
      w->writeText("    byte ");
      for (int k = 0; k < e->getChannelCount(DIV_SYSTEM_TIA); k++) {
        if (k > 0) {
          w->writeText(", ");
        }
        unsigned short p = subs->orders.ord[k][j];
        logD("ss: %d ord: %d chan: %d pat: %d", i, j, k, p);
        String key = fmt::sprintf("PAT_S%02x_C%02x_P%02x", i, k, p);
        w->writeText(key);        
        if (alreadyAdded[k][p]) continue;
        patterns.push_back(PatternIndex(key, i, j, k, p));
        alreadyAdded[k][p] = true;
      }
      w->writeText("\n");
    }
  }
  w->writeText("    byte 255\n");

  // emit pattern table
  // this is where we can lookup specific instrument/note/octave combinations
  w->writeC('\n');
  w->writeText("; Pattern Lookup Table\n");
  w->writeText("    ALIGN 256\n");
  w->writeText("PAT_TABLE_START\n");
  for (PatternIndex& patternIndex: patterns) {
    w->writeText(fmt::sprintf("%s = . - PAT_TABLE_START\n", patternIndex.key.c_str()));
    w->writeText(fmt::sprintf("   word %s_ADDR\n", patternIndex.key.c_str()));
  }

  // emit patterns
  // we emit the "note" being played as an assembly variable including the instrument being played
  // later we will figure out what we need to emit as far as TIA register settings
  // this assumes the song has a limited number of unique "notes"
  std::set<String> waveFormSet;
  const char* waveforms[4][128][256][2];
  memset(&waveforms, 0, 4 * 64 * 256 * 2);
  for (PatternIndex& patternIndex: patterns) {
    DivPattern* pat = e->song.subsong[patternIndex.subsong]->pat[patternIndex.chan].getPattern(patternIndex.pat, false);
    w->writeText(fmt::sprintf("; Subsong: %d Channel: %d Pattern: %d / %s\n", patternIndex.subsong, patternIndex.chan, patternIndex.pat, pat->name));
    w->writeText(fmt::sprintf("%s_ADDR", patternIndex.key.c_str()));
    for (int j = 0; j<e->song.subsong[patternIndex.subsong]->patLen; j++) {
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
        "WF_N%d_O%d_I%d_V%d", // TODO: could be more readable in code
        note & 0xff, 
        octave & 0xff,
        instrument & 0xff,
        volume & 0xff);
      w->writeText(key);        
      auto it = waveFormSet.emplace(key);
      if (it.second) {
        waveforms[patternIndex.subsong][patternIndex.ord][j][patternIndex.chan] = it.first->c_str();
      }
    }
    w->writeText("\n    byte 255\n");
  }

  // emit waveform table
  // this is where we can lookup specific instrument/note/octave combinations
  w->writeC('\n');
  w->writeText("; Waveform Lookup Table\n");
  w->writeText("    ALIGN 256\n");
  w->writeText("WF_TABLE_START\n");
  for (auto it = waveFormSet.begin(); it != waveFormSet.end(); ++it) {
    w->writeText(fmt::sprintf("%s = . - WF_TABLE_START\n", *it));
    w->writeText(fmt::sprintf("   word %s_ADDR\n", *it));
  }
    
  // emit waveform data
  // this is done by playing back the song from the unique notes
  w->writeC('\n');
  w->writeText("; Waveforms\n");
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
  for (size_t subsong = 0; subsong < e->song.subsong.size(); subsong++) {
    e->changeSongP(subsong);
    for (int channel = 0; channel < 2; channel++) {
      e->stop();
      e->setRepeatPattern(false);
      e->setOrder(0);
      e->play();
      
      int tick = 0;
      int lastWriteTick = 0;
      bool needsRegisterWrite = false;
      bool needsWriteDuration = false;

      RowIndex curRowIndex(e->getCurrentSubSong(), e->getOrder(), e->getRow());
      const char *key = waveforms[curRowIndex.subsong][curRowIndex.ord][curRowIndex.row][channel];
      if (NULL != key) {
        logD("got key %s", key);
        writeWaveformHeader(w, key);
        needsRegisterWrite = true;
      }

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
        
        // check if we've changed rows
        if (curRowIndex.advance(e->getCurrentSubSong(), e->getOrder(), e->getRow())) {
          if (needsRegisterWrite) {
            writeRegisters(w, currentState, channel);
            needsWriteDuration = true;
          }
          if (needsWriteDuration) {
            // prev seq
            w->writeText(fmt::sprintf("  byte %d, 255\n", tick - lastWriteTick - 1));
            needsWriteDuration = false;
          }
          lastWriteTick = tick = 0;
          logD("advancing %d %d %d %d", curRowIndex.subsong, curRowIndex.ord, curRowIndex.row, channel);
          key = waveforms[curRowIndex.subsong][curRowIndex.ord][curRowIndex.row][channel];
          if (NULL != key) {
            logD("got key %s", key);
            writeWaveformHeader(w, key);
            needsRegisterWrite = true;
          } else {
            needsRegisterWrite = false;
          }
        }
        // get register dumps
        int deltaTick = tick - lastWriteTick - 1;
        for (int i=0; i<e->song.systemLen; i++) {
          // BUGBUG: don't iterate systems this way (will break if multiple systems)
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
            if (needsWriteDuration) {
              w->writeText(fmt::sprintf("  byte %d\n", deltaTick));
            }
            // start next seq
            lastWriteTick = tick;
            needsWriteDuration = true;
            writeRegisters(w, currentState, channel);
            needsRegisterWrite = false;
          }
        }
        tick++;
      }
      if (needsRegisterWrite) {
        writeRegisters(w, currentState, channel);
        needsWriteDuration = true;
      }
      if (needsWriteDuration) {
        // final seq
        w->writeText(fmt::sprintf("  byte %d, 255\n", tick - lastWriteTick - 1));
      }
    }
  }

  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }

  w->writeText("; end of track data\n");
}

void R9::writeRegisters(SafeWriter* w, const TiaRegisters& reg, int channel) {
  if (0 == channel) {
    w->writeText(fmt::sprintf("  byte %d,%d,%d\n", reg.audc0, reg.audf0, reg.audv0));
  } else {
    w->writeText(fmt::sprintf("  byte %d,%d,%d\n", reg.audc1, reg.audf1, reg.audv1));
  }
}

void R9::writeWaveformHeader(SafeWriter* w, const char * key) {
  w->writeText(fmt::sprintf("%s_ADDR\n", key));
}