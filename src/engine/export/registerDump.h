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

#ifndef _REGISTERDUMP_H
#define _REGISTERDUMP_H

#include "../engine.h"

const int TICKS_PER_SECOND = 1000000;
const int TICKS_AT_60HZ = TICKS_PER_SECOND / 60;

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

struct ChannelState {

  unsigned char registers[4];
  // BUGBUG: make this a uint

  ChannelState() {}

  ChannelState(unsigned char c) {
    memset(registers, c, 4);
  }

  bool write(unsigned int address, unsigned int value) {
    unsigned char v = (unsigned char)value;
    if (registers[address] == v) return false;
    registers[address] = v;
    return true;
  }

  uint64_t hash() const {
    uint64_t h = 0;
    for (int i = 0; i < 4; i++) {
      h = ((uint64_t)registers[i]) + (h << 8);
    }
    return h;
  }

};



/**
 * Dumped register values held over time
 */
struct DumpInterval {

  ChannelState state;
  char duration;

  DumpInterval() {}

  DumpInterval(const DumpInterval &n) : state(n.state), duration(n.duration) {}

  DumpInterval(const ChannelState &state) : state(state), duration(-1) {}

  uint64_t hash() const {
    uint64_t h = state.hash();
    h += ((uint64_t)duration << 56);
    return h;
  }

}; 

/**
 * Sequences of register dumps
 */
struct DumpSequence {

  std::vector<DumpInterval> intervals;

  void dumpRegisters(const ChannelState &state) {
    intervals.emplace_back(DumpInterval(state));
  }

  int writeDuration(const int ticks, const int remainder, const int freq) {
    int total = ticks + remainder;
    int cycles = total / freq;
    intervals.back().duration = cycles;
    return total - (cycles * freq);
  }

  size_t size() const {
    return intervals.size();
  }

  uint64_t hash() const {
    // rolling polyhash: see https://cp-algorithms.com/string/string-hashing.html CC 4.0 license
    const int p = 31;
    const int m = 1e9 + 9;
    uint64_t pp = 1;
    uint64_t value = 0;
    for (auto& x : intervals) {
      value = value + (x.hash() * pp) % m;
      pp = (pp * p) % m;
    }
    return value;
  }

};

/**
 * Extract all register dump sequences in a song.
 * Each sequence is keyed on subsong, ord, row and channel.
 * Depending on the system different channels may map the platform
 * address space to different channel registers.
 */
void captureSequence(
  DivEngine* e, 
  int subsong,
  int channel,
  DivSystem system, 
  std::map<unsigned int, unsigned int> &addressMap,
  std::vector<String> &sequence,
  std::map<String, DumpSequence> &registerDumps 
);

void findCommonDumpSequences(
  const std::map<String, DumpSequence> &registerDumps,
  std::map<uint64_t, String> &commonDumpSequences,
  std::map<uint64_t, unsigned int> &frequencyMap,
  std::map<String, String> &representativeMap
);

typedef uint64_t AlphaCode;
typedef int AlphaChar;

struct SuffixTree {

  SuffixTree *parent;
  SuffixTree *slink;
  std::vector<SuffixTree *> children;
  bool isLeaf;
  size_t start;
  size_t depth;

  SuffixTree(size_t alphabetSize, size_t d) : parent(NULL), slink(NULL), isLeaf(true), start(0), depth(d) {
    children.resize(alphabetSize);
    for (size_t i = 0; i < alphabetSize; i++) {
      children[i] = NULL;
    }
  }

  ~SuffixTree() {
    for (auto x : children) {
      delete x;
    }
  }

  SuffixTree *splice_node(size_t d, const std::vector<AlphaChar> &S);

  SuffixTree *add_leaf(size_t i, size_t d, const std::vector<AlphaChar> &S);

  void compute_slink(const std::vector<AlphaChar> &S);

  size_t substring_start() const;

  size_t substring_end() const;

  size_t substring_len() const;

  SuffixTree *find(const std::vector<AlphaChar> &K, const std::vector<AlphaChar> &S);

  size_t gather_leaves(std::vector<SuffixTree *> &leaves);

  SuffixTree *find_maximal_substring();

  AlphaChar gather_left(std::vector<SuffixTree *> &nodes, const std::vector<AlphaChar> &S);

};


void createAlphabet(
  const std::map<AlphaCode, String> &commonDumpSequences,
  std::vector<AlphaCode> &alphabet,
  std::map<String, AlphaChar> &index
);

void translateString(
    const std::vector<String> &sequence,
    const std::map<String, String> &representativeMap,
    const std::map<String, AlphaChar> &index,
    std::vector<AlphaChar> &alphaSequence
);

SuffixTree * createSuffixTree(
    const std::vector<AlphaCode> &alphabet,
    const std::vector<AlphaChar> &alphaSequence
);


void testCompress(SuffixTree *root, const std::vector<AlphaChar> &alphaSequence);
void testCommonSubsequences(const String &input);
void testCommonSubsequencesBrute(const String &input);

#endif // _REGISTERDUMP_H