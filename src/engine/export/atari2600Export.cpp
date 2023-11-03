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
#include "registerDump.h"

#include <fmt/printf.h>
#include <set>
#include "../../ta-log.h"

const unsigned int AUDC0 = 0x15;
const unsigned int AUDC1 = 0x16;
const unsigned int AUDF0 = 0x17;
const unsigned int AUDF1 = 0x18;
const unsigned int AUDV0 = 0x19;
const unsigned int AUDV1 = 0x1A;

std::map<unsigned int, unsigned int> channel0AddressMap = {
  {AUDC0, 0},
  {AUDF0, 1},
  {AUDV0, 2},
};

std::map<unsigned int, unsigned int> channel1AddressMap = {
  {AUDC1, 0},
  {AUDF1, 1},
  {AUDV1, 2},
};

bool TiaVoiceRegisters::write(const unsigned int addr, const unsigned int value) {
  unsigned char val = value;
  switch (addr) {
    case 0:
      if (val == audcx) return false;
      audcx = val;
      return true;
    case 1:
      if (val == audfx) return false;
      audfx = val;
      return true;
    case 2:
      if (val == audvx) return false;
      audvx = val;
      return true;
  }
  return false;
}

uint64_t TiaVoiceRegisters::hash_interval(const char duration) {
  return  ((uint64_t)audcx) +  
          (((uint64_t)audfx) << 8) + 
          (((uint64_t)audvx) << 16) +
          (((uint64_t)duration) << 32);
}

std::vector<DivROMExportOutput> DivExportAtari2600::go(DivEngine* e) {
  std::vector<DivROMExportOutput> ret;
  ret.reserve(2);

  // create meta data (optional)
  logD("writing track title graphics");
  SafeWriter* titleData=new SafeWriter;
  titleData->init();
  titleData->writeText(fmt::sprintf("; Song: %s\n", e->song.name));
  titleData->writeText(fmt::sprintf("; Author: %s\n", e->song.author));
  auto title = (e->song.name.length() > 0) ?
     (e->song.name + " by " + e->song.author) :
     "furnace tracker";
  if (title.length() > 26) {
    title = title.substr(23) + "...";
  }
  writeTextGraphics(titleData, title.c_str());
  ret.push_back(DivROMExportOutput("Track_meta.asm", titleData));

  // create track data
  logD("writing track audio data");
  SafeWriter* trackData=new SafeWriter;
  trackData->init();
  trackData->writeText(fmt::sprintf("; Song: %s\n", e->song.name));
  trackData->writeText(fmt::sprintf("; Author: %s\n", e->song.author));
  writeTrackData_CRD(e, trackData);
  ret.push_back(DivROMExportOutput("Track_data.asm", trackData));

  return ret;
}

/**
 * 
 * we first play back the song to create a register dump then compress it
 * into common subsequences
 */
void DivExportAtari2600::writeTrackData_CRD(DivEngine* e, SafeWriter *w) {

  // capture all sequences
  logD("performing sequence capture");
  std::map<String, DumpSequence<TiaVoiceRegisters>> sequences;
  captureSequences(e, DIV_SYSTEM_TIA, 0, channel0AddressMap, sequences);
  captureSequences(e, DIV_SYSTEM_TIA, 1, channel1AddressMap, sequences);

  // compress the patterns into common subsequences
  logD("performing sequence compression");
  std::map<uint64_t, String> commonSubSequences;
  std::map<uint64_t, unsigned int> sequenceFrequency;
  std::map<String, String> representativeSequenceMap;
  findCommonSubsequences(
    sequences,
    commonSubSequences,
    sequenceFrequency,
    representativeSequenceMap);

  // emit song table
  logD("writing song table");
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
    auto freq = sequenceFrequency[x.first];
    writeWaveformHeader(w, x.second.c_str());
    w->writeText(fmt::sprintf("; Hash %d, Freq %d\n", x.first, freq));
    auto& dump = sequences[x.second];
    TiaVoiceRegisters last(255);
    for (auto& n: dump.intervals) {
      waveformDataSize += writeNote(w, n.state, n.duration, last);
      last = n.state;
    }
    w->writeText("    byte 0\n");
    waveformDataSize++;
  }

  // audio metadata
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
 */
size_t DivExportAtari2600::writeNote(SafeWriter* w, const TiaVoiceRegisters& next, const char duration, const TiaVoiceRegisters& last) {
  size_t bytesWritten = 0;
  unsigned char dmod = 0; // if duration is small, store in top bits of frequency

  unsigned char audfx, audcx, audvx;
  int cc, fc, vc;
  audfx = next.audfx;
  fc = audfx != last.audfx;
  audcx = next.audcx;
  cc = audcx != last.audcx;
  audvx = next.audvx;
  vc = audvx != last.audvx;
  
  w->writeText(fmt::sprintf("    ;F%d C%d V%d D%d\n", audfx, audcx, audvx, duration));

  if ( ((cc + fc + vc) == 1) && duration < 3) {
    // write a delta row - only change one register
    if (duration == 0) {
        logD("0 duration note");
    }
    dmod = duration > 0 ? duration - 1 : 1; // BUGBUG: when duration is zero... we force to 1...
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
    if (duration < 3) {
      // short duration
      dmod = duration;
    } else {
      dmod = 3;
    }
    // frequency
    w->writeText(fmt::sprintf("    byte %d", audfx << 3 | dmod << 1 ));
    if (dmod == 3) {
      w->writeText(fmt::sprintf(",%d", duration));
      bytesWritten += 1;
    }
    // waveform and volume
    w->writeText(fmt::sprintf(",%d\n", (audcx << 4) + audvx));
    bytesWritten += 2;

  }

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
    len++;
    char ax = 0;
    if (!end) {
      ax = *value++;
      if (0 == ax) {
        end = true;
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
