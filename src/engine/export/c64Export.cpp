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

#include "c64Export.h"
#include "registerDump.h"

#include <fmt/printf.h>
#include <set>
#include "../../ta-log.h"

const unsigned int FreqL0 = 0x00;
const unsigned int FreqH0 = 0x01;
const unsigned int PWL0 = 0x02;
const unsigned int PWH0 = 0x03;
const unsigned int Control0 = 0x04;
const unsigned int AtkDcy0 = 0x05;
const unsigned int StnRis0 = 0x06;
const unsigned int FreqL1 = 0x07;
const unsigned int FreqH1 = 0x08;
const unsigned int PWL1 = 0x09;
const unsigned int PWH1 = 0x0A;
const unsigned int Control1 = 0x0B;
const unsigned int AtkDcy1 = 0x0C;
const unsigned int StnRis1 = 0x0D;
const unsigned int FreqL2 = 0x0E;
const unsigned int FreqH2 = 0x0F;
const unsigned int PWL2 = 0x10;
const unsigned int PWH2 = 0x11;
const unsigned int Control2 = 0x12;
const unsigned int AtkDcy2 = 0x13;
const unsigned int StnRis2 = 0x14;
const unsigned int FCL = 0x15;
const unsigned int FCH = 0x16;
const unsigned int FilterRes = 0x17;
const unsigned int FilterMode = 0x18;

std::map<unsigned int, unsigned int> voice1AddressMap = {
  {FreqL0, 0},
  {FreqH0, 1},
  {PWL0, 2},
  {PWH0, 3},
  {Control0, 4},
  {AtkDcy0, 5},
  {StnRis0, 6}
};

std::map<unsigned int, unsigned int> voice2AddressMap = {
  {FreqL1, 0},
  {FreqH1, 1},
  {PWL1, 2},
  {PWH1, 3},
  {Control1, 4},
  {AtkDcy1, 5},
  {StnRis1, 6}
};

std::map<unsigned int, unsigned int> voice3AddressMap = {
  {FreqL2, 0},
  {FreqH2, 1},
  {PWL2, 2},
  {PWH2, 3},
  {Control2, 4},
  {AtkDcy2, 5},
  {StnRis2, 6}
};

std::map<unsigned int, unsigned int> filterAddressMap = {
  {FCL, 0},
  {FCH, 1},
  {FilterRes, 2},
  {FilterMode, 3}
};

struct SidVoiceRegisters {

  unsigned char freqLx;
  unsigned char freqHx;
  unsigned char pwLx;
  unsigned char pwHx;
  unsigned char controlx;
  unsigned char atkDcyx;
  unsigned char stnRisx;

  SidVoiceRegisters() {}

  SidVoiceRegisters(unsigned char c) : 
    freqLx(c),
    freqHx(c),
    pwLx(c),
    pwHx(c),
    controlx(c),
    atkDcyx(c),
    stnRisx(c)
  {}

  bool write(const unsigned int addr, const unsigned int value) {
    unsigned char val = value;
    switch (addr) {
      case 0:
        if (val == freqLx) return false;
        freqLx = val;
        return true;
      case 1:
        if (val == freqHx) return false;
        freqHx = val;
        return true;
      case 2:
        if (val == pwLx) return false;
        pwLx = val;
        return true;
      case 3:
        if (val == pwHx) return false;
        pwHx = val;
        return true;
      case 4:
        if (val == controlx) return false;
        controlx = val;
        return true;
      case 5:
        if (val == atkDcyx) return false;
        atkDcyx = val;
        return true;
      case 6:
        if (val == stnRisx) return false;
        stnRisx = val;
        return true;        
    }
    return false;
  }

  uint64_t hash_interval(const char duration) {
    return ((uint64_t)freqLx) +
           (((uint64_t)freqHx) << 8) +
           (((uint64_t)pwLx) << 16) +
           (((uint64_t)pwHx) << 24) +
           (((uint64_t)controlx) << 32) +
           (((uint64_t)atkDcyx) << 40) +
           (((uint64_t)stnRisx) << 48) +
           (((uint64_t)duration) << 56);
  }

};

struct SidFilterRegisters {

  unsigned char fcL;
  unsigned char fcH;
  unsigned char filterRes;
  unsigned char filterMode;

  SidFilterRegisters() {}

  SidFilterRegisters(char c) : fcL(0), fcH(0), filterRes(0), filterMode(0) {}

  bool write(const unsigned int addr, const unsigned int value) {
    unsigned char val = value;
    switch (addr) {
      case 0:
        if (val == fcL) return false;
        fcL = val;
        return true;
      case 1:
        if (val == fcH) return false;
        fcH = val;
        return true;
      case 2:
        if (val == filterRes) return false;
        filterRes = val;
        return true;
      case 3:
        if (val == filterMode) return false;
        filterMode = val;
        return true;
    }
    return false;
  }

  uint64_t hash_interval(const char duration) {
    return ((uint64_t)fcL) +  
           (((uint64_t)fcH) << 8) + 
           (((uint64_t)filterRes) << 16) +
           (((uint64_t)filterMode) << 32) +
           (((uint64_t)duration) << 40);
  }

};


std::vector<DivROMExportOutput> DivExportC64::go(DivEngine* e) {

  // capture all sequences
  logD("performing sequence capture");
  std::map<String, DumpSequence<SidVoiceRegisters>> voiceSequences;
  std::map<String, DumpSequence<SidFilterRegisters>> filterSequences;
  captureSequences(e, DIV_SYSTEM_C64_6581, 0, voice1AddressMap, voiceSequences);
  captureSequences(e, DIV_SYSTEM_C64_6581, 1, voice2AddressMap, voiceSequences);
  captureSequences(e, DIV_SYSTEM_C64_6581, 2, voice3AddressMap, voiceSequences);
  captureSequences(e, DIV_SYSTEM_C64_6581, 3, filterAddressMap, filterSequences);
  logD("found %d voice sequences", voiceSequences.size());
  logD("found %d filter sequences", filterSequences.size());

  // sequence frequency stats
  std::map<uint64_t, unsigned int> sequenceFrequency;
  // sequence lookup 
  std::map<String, String> representativeSequenceMap;

  // compress the voices into common subsequences
  logD("performing voice sequence compression");
  std::map<uint64_t, String> commonVoiceSubSequences;
  findCommonSubsequences(
    voiceSequences,
    commonVoiceSubSequences,
    sequenceFrequency,
    representativeSequenceMap);
  logD("found %d common voice sequences", commonVoiceSubSequences.size());

  // compress the filter registers into common subsequences
  logD("performing filter sequence compression");
  std::map<uint64_t, String> commonFilterSubSequences;
  std::map<String, String> representativeFilterSequenceMap;
  findCommonSubsequences(
    filterSequences,
    commonFilterSubSequences,
    sequenceFrequency,
    representativeSequenceMap);
  logD("found %d common filter sequences", commonFilterSubSequences.size());

  std::vector<DivROMExportOutput> ret;
  ret.reserve(1);

  // create track data
  logD("writing track audio data");
  SafeWriter* w = new SafeWriter;
  w->init();

  w->writeText(fmt::sprintf("; Song: %s\n", e->song.name));
  w->writeText(fmt::sprintf("; Author: %s\n", e->song.author));

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

  // emit voice waveform table
  size_t voiceWaveformTableSize = 0;
  w->writeC('\n');
  w->writeText("; Voice Waveform Lookup Table\n");
  w->writeText(fmt::sprintf("NUM_VOICE_WAVEFORMS = %d\n", commonVoiceSubSequences.size()));
  w->writeText("WF_VOICE_TABLE_START_LO\n");
  for (auto& x: commonVoiceSubSequences) {
    w->writeText(fmt::sprintf("%s = . - WF_VOICE_TABLE_START_LO\n", x.second.c_str()));
    w->writeText(fmt::sprintf("   byte <%s_ADDR\n", x.second.c_str()));
    voiceWaveformTableSize++;
  }
  w->writeText("WF_VOICE_TABLE_START_HI\n");
  for (auto& x: commonVoiceSubSequences) {
    w->writeText(fmt::sprintf("   byte >%s_ADDR\n", x.second.c_str()));
    voiceWaveformTableSize++;
  }

  // emit filter waveform table
  size_t filterWaveformTableSize = 0;
  w->writeC('\n');
  w->writeText("; Filter Waveform Lookup Table\n");
  w->writeText(fmt::sprintf("NUM_FILTER_WAVEFORMS = %d\n", commonFilterSubSequences.size()));
  w->writeText("WF_FILTER_TABLE_START_LO\n");
  for (auto& x: commonFilterSubSequences) {
    w->writeText(fmt::sprintf("%s = . - WF_FILTER_TABLE_START_LO\n", x.second.c_str()));
    w->writeText(fmt::sprintf("   byte <%s_ADDR\n", x.second.c_str()));
    filterWaveformTableSize++;
  }
  w->writeText("WF_FILTER_TABLE_START_HI\n");
  for (auto& x: commonFilterSubSequences) {
    w->writeText(fmt::sprintf("   byte >%s_ADDR\n", x.second.c_str()));
    filterWaveformTableSize++;
  }

  // emit voice waveforms
  size_t voiceWaveformDataSize = 0;
  w->writeC('\n');
  w->writeText("; Voice Waveforms\n");
  for (auto& x: commonVoiceSubSequences) {
    auto freq = sequenceFrequency[x.first];
    w->writeText(fmt::sprintf("%s_ADDR\n", x.second.c_str()));
    w->writeText(fmt::sprintf("; Hash %d, Freq %d\n", x.first, freq));
    auto& dump = voiceSequences[x.second];
    for (auto& n: dump.intervals) {
      w->writeText(
        fmt::sprintf(
          "    byte %d,%d,%d,%d,%d,%d,%d,%d\n",
          n.duration,
          n.state.freqLx,
          n.state.freqHx,
          n.state.pwLx,
          n.state.pwHx,
          n.state.controlx,
          n.state.atkDcyx,
          n.state.stnRisx
        )
      );
      voiceWaveformDataSize += 8;
    }
    w->writeText("    byte 255\n");
    voiceWaveformDataSize++;
  }

  // emit filter waveforms
  size_t filterWaveformDataSize = 0;
  w->writeC('\n');
  w->writeText("; Filter Waveforms\n");
  for (auto& x: commonFilterSubSequences) {
    auto freq = sequenceFrequency[x.first];
    w->writeText(fmt::sprintf("%s_ADDR\n", x.second.c_str()));
    w->writeText(fmt::sprintf("; Hash %d, Freq %d\n", x.first, freq));
    auto& dump = filterSequences[x.second];
    for (auto& n: dump.intervals) {
      w->writeText(
        fmt::sprintf(
          "    byte %d,%d,%d,%d,%d\n",
          n.duration,
          n.state.fcL,
          n.state.fcH,
          n.state.filterRes,
          n.state.filterMode
        )
      );
      filterWaveformDataSize += 5;
    }
    w->writeText("    byte 255\n");
    filterWaveformDataSize++;
  }

  w->writeC('\n');
  // audio metadata
  w->writeC('\n');
  w->writeText(fmt::sprintf("; Song Table Size %d\n", songTableSize));
  w->writeText(fmt::sprintf("; Song Data Size %d\n", songDataSize));
  w->writeText(fmt::sprintf("; Pattern Lookup Table Size %d\n", patternTableSize));
  w->writeText(fmt::sprintf("; Pattern Data Size %d\n", patternDataSize));
  w->writeText(fmt::sprintf("; Voice Waveform Table Size %d\n", voiceWaveformTableSize));
  w->writeText(fmt::sprintf("; Filter Waveform Table Size %d\n", filterWaveformTableSize));
  w->writeText(fmt::sprintf("; Voice Waveform Data Size %d\n", voiceWaveformDataSize));
  w->writeText(fmt::sprintf("; Filter Waveform Data Size %d\n", filterWaveformDataSize));
  size_t totalDataSize = 
    songTableSize + songDataSize + patternTableSize + 
    patternDataSize + voiceWaveformTableSize + filterWaveformTableSize +
    voiceWaveformDataSize + filterWaveformDataSize;
  w->writeText(fmt::sprintf("; Total Data Size %d\n", totalDataSize));

  ret.push_back(DivROMExportOutput("Track_data.asm", w));

  return ret;

}
