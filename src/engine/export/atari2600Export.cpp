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

#include "atari2600Export.h"

#include <fmt/printf.h>
#include <set>
#include "../../ta-log.h"

const int TICKS_PER_SECOND = 1000000;
const int TICKS_AT_60HZ = TICKS_PER_SECOND / 60;
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

struct RowIndex {
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

struct DumpSequence {

  std::vector<TiaNote> notes;

  void dumpRegisters(const TiaRegisters& registers) {
    notes.emplace_back(TiaNote(registers));
  }

  int writeDuration(const int ticks, const int remainder,  const int freq) {
    int total = ticks + remainder;
    int cycles = total / freq;
    notes.back().duration = cycles;
    return total - (cycles * freq);
  }

  size_t size() {
    return notes.size();
  }

  size_t hash() {
    // rolling polyhash: see https://cp-algorithms.com/string/string-hashing.html CC 4.0 license
    const int p = 31;
    const int m = 1e9 + 9;
    size_t pp = 1;
    size_t value = 0;
    for (auto& x : notes) {
      value = value + (x.hash() * pp) % m;
      pp = (pp * p) % m;
    }
    return value;
  }

};

std::vector<DivROMExportOutput> DivExportAtari2600::go(DivEngine* e) {
  std::vector<DivROMExportOutput> ret;
  ret.reserve(2);

  // create title data (optional)
  SafeWriter* titleData=new SafeWriter;
  titleData->init();
  auto title = (e->song.name + " by " + e->song.author);
  if (title.length() > 26) {
    title = title.substr(23) + "...";
  }
  writeTextGraphics(titleData, title.c_str());
  ret.push_back(DivROMExportOutput("Track_title.asm", titleData));

  // create track data
  SafeWriter* trackData=new SafeWriter;
  trackData->init();
  trackData->writeText(fmt::sprintf("; Song: %s\n", e->song.name));
  trackData->writeText(fmt::sprintf("; Author: %s\n", e->song.author));
  writeTrackData_CRD(e, trackData);
  ret.push_back(DivROMExportOutput("Track_data.asm", trackData));

  return ret;
}

inline auto getSequenceKey(unsigned short subsong, unsigned short ord, unsigned short row, unsigned short channel) {
  return fmt::sprintf(
        "SEQ_S%02x_O%02x_R%02x_C%02x",
         subsong, 
         ord,
         row,
         channel);
}

inline auto getPatternKey(unsigned short subsong, unsigned short channel, unsigned short pattern) {
  return fmt::sprintf(
    "PAT_S%02x_C%02x_P%02x",
    subsong,
    channel,
    pattern
  );
}

/**
 * 
 * we first play back the song to create a register dump then compress it
 * into common subsequences
 */
void DivExportAtari2600::writeTrackData_CRD(DivEngine* e, SafeWriter *w) {

  // capture all sequences
  std::map<String, DumpSequence> sequences;
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
  for (size_t subsong = 0; subsong < e->song.subsong.size(); subsong++) {
    e->changeSongP(subsong);
    for (int channel = 0; channel < e->getChannelCount(DIV_SYSTEM_TIA); channel++) {
      e->stop();
      e->setRepeatPattern(false);
      e->setOrder(0);
      e->play();
      
      int lastWriteTicks = e->getTotalTicks();
      int lastWriteSeconds = e->getTotalSeconds();
      int deltaTicksR = 0;
      int deltaTicks = 0;

      bool needsRegisterDump = false;
      bool needsWriteDuration = false;      

      RowIndex curRowIndex(e->getCurrentSubSong(), e->getOrder(), e->getRow());

      String key = getSequenceKey(curRowIndex.subsong, curRowIndex.ord, curRowIndex.row, channel);
      auto it = sequences.emplace(key, DumpSequence());
      DumpSequence *currentDumpSequence = &(it.first->second);

      TiaRegisters currentState;
      memset(&currentState, 0, sizeof(currentState));

      bool done=false;
      while (!done && e->isPlaying()) {
        
        int currentTicks = e->getTotalTicks();
        int currentSeconds = e->getTotalSeconds();
        done = e->tick(false);
        deltaTicks = 
          currentTicks - lastWriteTicks + 
          (TICKS_PER_SECOND * (currentSeconds - lastWriteSeconds));

        // check if we've changed rows
        if (curRowIndex.advance(e->getCurrentSubSong(), e->getOrder(), e->getRow())) {
          if (needsRegisterDump) {
            currentDumpSequence->dumpRegisters(currentState);
            needsWriteDuration = true;
          }
          if (needsWriteDuration) {
            // prev seq
            deltaTicksR = currentDumpSequence->writeDuration(deltaTicks, deltaTicksR, TICKS_AT_60HZ);
            deltaTicks = 0;
            lastWriteTicks = currentTicks;
            lastWriteSeconds = currentSeconds;
            needsWriteDuration = false;
          }
          // new sequence
          key = getSequenceKey(curRowIndex.subsong, curRowIndex.ord, curRowIndex.row, channel);
          auto nextIt = sequences.emplace(key, DumpSequence());
          currentDumpSequence = &(nextIt.first->second);
          needsRegisterDump = true;
        }

        // get register dumps
        bool isDirty = false;
        for (int i=0; i<e->song.systemLen; i++) {
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
        }

        if (isDirty) {
          // end last seq
          if (needsWriteDuration) {
            deltaTicksR = currentDumpSequence->writeDuration(deltaTicks, deltaTicksR, TICKS_AT_60HZ);
            lastWriteTicks = currentTicks;
            lastWriteSeconds = currentSeconds;
            deltaTicks = 0;
          }
          // start next seq
          needsWriteDuration = true;
          currentDumpSequence->dumpRegisters(currentState);
          needsRegisterDump = false;
        }
      }
      if (needsRegisterDump) {
        currentDumpSequence->dumpRegisters(currentState);
        needsWriteDuration = true;
      }
      if (needsWriteDuration) {
        // final seq
        currentDumpSequence->writeDuration(deltaTicks, deltaTicksR, TICKS_AT_60HZ);
      }
    }
  }
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }

  // compress the patterns into common subsequences
  std::map<size_t, String> commonSubSequences;
  std::map<String, String> representativeSequenceMap;
  for (auto& x: sequences) {
    size_t hash = x.second.hash();
    auto it = commonSubSequences.emplace(hash, x.first);
    // TODO: verify no hash collision...?
    representativeSequenceMap.emplace(x.first, it.first->second);
  }

  // emit song table
  size_t songTableSize = 0;
  w->writeText("\n; Song Lookup Table\n");
  w->writeText(fmt::sprintf("NUM_SONGS = %d\n", e->song.subsong.size()));
  w->writeText("SONG_TABLE_START_LO\n");
  for (size_t i = 0; i < e->song.subsong.size(); i++) {
    w->writeText(fmt::sprintf("SONG_%d = . - SONG_TABLE_START_LO\n", i));
    w->writeText(fmt::sprintf("    byte <SONG_%d_ADDR\n", i));
    songTableSize++;
  }
  w->writeText("SONG_TABLE_START_HI\n");
  for (size_t i = 0; i < e->song.subsong.size(); i++) {
    w->writeText(fmt::sprintf("    byte >SONG_%d_ADDR\n", i));
    songTableSize++;
  }

  // collect and emit song data
  // borrowed from fileops
  size_t songDataSize = 0;
  w->writeText("; songs\n");
  std::vector<PatternIndex> patterns;
  bool alreadyAdded[2][256];
  for (size_t i = 0; i < e->song.subsong.size(); i++) {
    w->writeText(fmt::sprintf("SONG_%d_ADDR\n", i));
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
        String key = getPatternKey(i, k, p);
        w->writeText(key);
        songDataSize++;
        if (alreadyAdded[k][p]) continue;
        patterns.push_back(PatternIndex(key, i, j, k, p));
        alreadyAdded[k][p] = true;
      }
      w->writeText("\n");
    }
    w->writeText("    byte 255\n");
    songDataSize++;
  }

  // pattern lookup
  size_t patternTableSize = 0;
  w->writeC('\n');
  w->writeText("; Pattern Lookup Table\n");
  w->writeText(fmt::sprintf("NUM_PATTERNS = %d\n", patterns.size()));
  w->writeText("PAT_TABLE_START_LO\n");
  for (PatternIndex& patternIndex: patterns) {
    w->writeText(fmt::sprintf("%s = . - PAT_TABLE_START_LO\n", patternIndex.key.c_str()));
    w->writeText(fmt::sprintf("   byte <%s_ADDR\n", patternIndex.key.c_str()));
    patternTableSize++;
  }
  w->writeText("PAT_TABLE_START_HI\n");
  for (PatternIndex& patternIndex: patterns) {
    w->writeText(fmt::sprintf("   byte >%s_ADDR\n", patternIndex.key.c_str()));
    patternTableSize++;
  }

  // emit sequences
  // we emit the "note" being played as an assembly variable 
  // later we will figure out what we need to emit as far as TIA register settings
  // this assumes the song has a limited number of unique "notes"
  size_t patternDataSize = 0;
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
      String key = getSequenceKey(patternIndex.subsong, patternIndex.ord, j, patternIndex.chan);
      w->writeText(representativeSequenceMap[key]); // the representative
      patternDataSize++;
    }
    w->writeText("\n    byte 255\n");
    patternDataSize++;
  }

  // emit waveform table
  // this is where we can lookup specific instrument/note/octave combinations
  // can be quite expensive to store this table (2 bytes per waveform)
  size_t waveformTableSize = 0;
  w->writeC('\n');
  w->writeText("; Waveform Lookup Table\n");
  w->writeText(fmt::sprintf("NUM_WAVEFORMS = %d\n", commonSubSequences.size()));
  w->writeText("WF_TABLE_START_LO\n");
  for (auto& x: commonSubSequences) {
    w->writeText(fmt::sprintf("%s = . - WF_TABLE_START_LO\n", x.second.c_str()));
    w->writeText(fmt::sprintf("   byte <%s_ADDR\n", x.second.c_str()));
    waveformTableSize++;
  }
  w->writeText("WF_TABLE_START_HI\n");
  for (auto& x: commonSubSequences) {
    w->writeText(fmt::sprintf("   byte >%s_ADDR\n", x.second.c_str()));
    waveformTableSize++;
  }
    
  // emit waveforms
  size_t waveformDataSize = 0;
  w->writeC('\n');
  w->writeText("; Waveforms\n");
  for (auto& x: commonSubSequences) {
    writeWaveformHeader(w, x.second.c_str());
    w->writeText(fmt::sprintf("; Hash %d\n", x.first));
    auto& dump = sequences[x.second];
    TiaChannelState state(255, 255, 255);
    for (auto& n: dump.notes) {
      waveformDataSize += writeNote(w, n, state);
    }
    w->writeText("    byte 0\n");
    waveformDataSize++;
  }

  // metadata
  w->writeC('\n');
  w->writeText(fmt::sprintf("; Song Table Size %d\n", songTableSize));
  w->writeText(fmt::sprintf("; Song Data Size %d\n", songDataSize));
  w->writeText(fmt::sprintf("; Pattern Lookup Table Size %d\n", patternTableSize));
  w->writeText(fmt::sprintf("; Pattern Data Size %d\n", patternDataSize));
  w->writeText(fmt::sprintf("; Waveform Lookup Table Size %d\n", waveformTableSize));
  w->writeText(fmt::sprintf("; Waveform Data Size %d\n", waveformDataSize));
  size_t totalDataSize = 
    songTableSize + songDataSize + patternTableSize + 
    patternDataSize + waveformTableSize + waveformDataSize;
  w->writeText(fmt::sprintf("; Total Data Size %d\n", totalDataSize));

}

/**
 *  Write note data. Format:
 * 
 *   fffff010 wwwwvvvv           frequency + waveform + volume, duration 1
 *   fffff100 wwwwvvvv           " " ", duration 2
 *   fffff110 dddddddd wwwwvvvv  " " ", duration d
 *   xxxx0001                    volume = x >> 4, duration 1 
 *   xxxx1001                    volume = x >> 4, duration 2
 *   xxxx0101                    wave = x >> 4, duration 1
 *   xxxx1101                    wave = x >> 4, duration 2
 *   xxxxx011                    frequency = x >> 3, duration 1
 *   xxxxx111                    frequency = x >> 3, duration 2
 *   00000000                    stop
 * 
 */
size_t DivExportAtari2600::writeNote(SafeWriter* w, const TiaNote& note, TiaChannelState& state) {
  size_t bytesWritten = 0;
  unsigned char dmod = 0; // if duration is small, store in top bits of frequency

  // KLUDGE: assume only one channel at a time. If both are zero...won't matter
  int channel = (note.registers.audc0 | note.registers.audf0 | note.registers.audv0) != 0 ? 0 : 1; 
  unsigned char audfx = (channel == 0) ? note.registers.audf0 : note.registers.audf1;
  unsigned char audcx = (channel == 0) ? note.registers.audc0 : note.registers.audc1;
  unsigned char audvx = (channel == 0) ? note.registers.audv0 : note.registers.audv1;

  w->writeText(fmt::sprintf("    ;F%d C%d V%d D%d\n", audfx, audcx, audvx, note.duration));

  int cc = audcx != state.audcx ? 1 : 0;
  int fc = audfx != state.audfx ? 1 : 0;
  int vc = audvx != state.audvx ? 1 : 0;

  if ( ((cc + fc + vc) == 1) && note.duration < 3) {
    // write a delta row - only change one register
    dmod = note.duration > 0 ? note.duration - 1 : 1; // BUGBUG: when duration is zero... we force to 1...
    unsigned char rx;
    if (fc > 0) {
      // frequency
      rx = audfx << 3 | dmod << 2 | 0x03; //  d11
    } else if (cc > 0 ) {
      // waveform
      rx = audcx << 3 | dmod << 3 | 0x05; // d101
    } else {
      // volume 
      rx = audvx << 3 | dmod << 3 | 0x01; // d001
    }
    w->writeText(fmt::sprintf("    byte %d\n", rx));
    bytesWritten += 1;

  } else {
    // write all registers
    if (note.duration < 3) {
      // short duration
      dmod = note.duration;
    } else {
      dmod = 3;
    }
    // frequency
    w->writeText(fmt::sprintf("    byte %d", audfx << 3 | dmod << 1 ));
    if (dmod == 3) {
      w->writeText(fmt::sprintf(",%d", note.duration));
      bytesWritten += 1;
    }
    // waveform and volume
    w->writeText(fmt::sprintf(",%d\n", (audcx << 4) + audvx));
    bytesWritten += 2;

  }

  state.audcx = audcx;
  state.audfx = audfx;
  state.audvx = audvx;

  return bytesWritten;

}

void DivExportAtari2600::writeWaveformHeader(SafeWriter* w, const char * key) {
  w->writeText(fmt::sprintf("%s_ADDR\n", key));
}


int getFontIndex(const char c) {
  if ('0' <= c && c <= '9') return c - '0';
  if (c == ' ' || c == 0) return 10;
  if (c == '.') return 12;
  if (c == '<') return 13;
  if (c == '>') return 14;
  if ('a' <= c && c <= 'z') return 15 + c - 'a';
  if ('A' <= c && c <= 'Z') return 15 + c - 'A';
  return 11;
}

// 4x6 font data used to encode title
unsigned char FONT_DATA[41][6] = {
  {0x00, 0x04, 0x0a, 0x0a, 0x0a, 0x04}, // SYMBOL_ZERO
  {0x00, 0x0e, 0x04, 0x04, 0x04, 0x0c}, // SYMBOL_ONE
  {0x00, 0x0e, 0x08, 0x06, 0x02, 0x0c}, // SYMBOL_TWO
  {0x00, 0x0c, 0x02, 0x06, 0x02, 0x0c}, // SYMBOL_THREE
  {0x00, 0x02, 0x02, 0x0e, 0x0a, 0x0a}, // SYMBOL_FOUR
  {0x00, 0x0c, 0x02, 0x0c, 0x08, 0x06}, // SYMBOL_FIVE
  {0x00, 0x06, 0x0a, 0x0c, 0x08, 0x06}, // SYMBOL_SIX
  {0x00, 0x08, 0x08, 0x04, 0x02, 0x0e}, // SYMBOL_SEVEN
  {0x00, 0x06, 0x0a, 0x0e, 0x0a, 0x0c}, // SYMBOL_EIGHT
  {0x00, 0x02, 0x02, 0x0e, 0x0a, 0x0c}, // SYMBOL_NINE
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // SYMBOL_SPACE
  {0x00, 0x0e, 0x00, 0x00, 0x00, 0x00}, // SYMBOL_UNDERSCORE
  {0x00, 0x04, 0x00, 0x00, 0x00, 0x00}, // SYMBOL_DOT
  {0x00, 0x02, 0x04, 0x08, 0x04, 0x02}, // SYMBOL_LT
  {0x00, 0x08, 0x04, 0x02, 0x04, 0x08}, // SYMBOL_GT
  {0x00, 0x0a, 0x0a, 0x0e, 0x0a, 0x0e}, // SYMBOL_A
  {0x00, 0x0e, 0x0a, 0x0c, 0x0a, 0x0e}, // SYMBOL_B
  {0x00, 0x0e, 0x08, 0x08, 0x08, 0x0e}, // SYMBOL_C
  {0x00, 0x0c, 0x0a, 0x0a, 0x0a, 0x0c}, // SYMBOL_D
  {0x00, 0x0e, 0x08, 0x0c, 0x08, 0x0e}, // SYMBOL_E
  {0x00, 0x08, 0x08, 0x0c, 0x08, 0x0e}, // SYMBOL_F
  {0x00, 0x0e, 0x0a, 0x08, 0x08, 0x0e}, // SYMBOL_G
  {0x00, 0x0a, 0x0a, 0x0e, 0x0a, 0x0a}, // SYMBOL_H
  {0x00, 0x04, 0x04, 0x04, 0x04, 0x04}, // SYMBOL_I
  {0x00, 0x0e, 0x0a, 0x02, 0x02, 0x02}, // SYMBOL_J
  {0x00, 0x0a, 0x0a, 0x0c, 0x0a, 0x0a}, // SYMBOL_K
  {0x00, 0x0e, 0x08, 0x08, 0x08, 0x08}, // SYMBOL_L
  {0x00, 0x0a, 0x0a, 0x0e, 0x0e, 0x0e}, // SYMBOL_M
  {0x00, 0x0a, 0x0a, 0x0a, 0x0a, 0x0e}, // SYMBOL_N
  {0x00, 0x0e, 0x0a, 0x0a, 0x0a, 0x0e}, // SYMBOL_O
  {0x00, 0x08, 0x08, 0x0e, 0x0a, 0x0e}, // SYMBOL_P
  {0x00, 0x06, 0x08, 0x0a, 0x0a, 0x0e}, // SYMBOL_Q
  {0x00, 0x0a, 0x0a, 0x0c, 0x0a, 0x0e}, // SYMBOL_R
  {0x00, 0x0e, 0x02, 0x0e, 0x08, 0x0e}, // SYMBOL_S
  {0x00, 0x04, 0x04, 0x04, 0x04, 0x0e}, // SYMBOL_T
  {0x00, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a}, // SYMBOL_U
  {0x00, 0x04, 0x04, 0x0e, 0x0a, 0x0a}, // SYMBOL_V
  {0x00, 0x0e, 0x0e, 0x0e, 0x0a, 0x0a}, // SYMBOL_W
  {0x00, 0x0a, 0x0e, 0x04, 0x0e, 0x0a}, // SYMBOL_X
  {0x00, 0x04, 0x04, 0x0e, 0x0a, 0x0a}, // SYMBOL_Y
  {0x00, 0x0e, 0x08, 0x04, 0x02, 0x0e}  // SYMBOL_Z
};

size_t DivExportAtari2600::writeTextGraphics(SafeWriter* w, const char* value) {
  size_t bytesWritten = 0;

  bool end = false;
  size_t len = 0; 
  while (len < 6 || !end) {
    w->writeText(fmt::sprintf("TITLE_GRAPHICS_%d\n    byte ", len));
    char ax = 0;
    if (!end) {
      ax = *value++;
      if (0 == ax) {
        end = true;
      } else {
        len++;
      }
    } 
    char bx = 0;
    if (!end) {
      bx = *value++;
      if (0 == bx) end = true;
    }
    auto ai = getFontIndex(ax);
    auto bi = getFontIndex(bx);
      for (int i = 0; i < 6; i++) {
      if (i > 0) {
        w->writeText(",");
      }
      const unsigned char c = (FONT_DATA[ai][i] << 4) + FONT_DATA[bi][i];
      w->writeText(fmt::sprintf("%d", c));
      bytesWritten += 1;
    }
    w->writeText("\n");
  }
  w->writeText(fmt::sprintf("TITLE_LENGTH = %d", len));
  return bytesWritten;
}
