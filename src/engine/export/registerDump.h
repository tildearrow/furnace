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

/**
 * Template struct comprised of captured register values combined with a Hz duration.
 */
template <typename ChannelState>
struct DumpInterval {

  ChannelState state;
  char duration;

  DumpInterval() {}

  DumpInterval(const DumpInterval<ChannelState> &n) : state(n.state), duration(n.duration) {}

  DumpInterval(const ChannelState &state) : state(state), duration(-1) {}

  uint64_t hash() {
    return state.hash_interval(duration);
  }

}; 

/**
 * Template struct used to analyze sequences of register dumps
 */
template <typename ChannelState> 
struct DumpSequence {

  std::vector<DumpInterval<ChannelState>> intervals;

  void dumpRegisters(const ChannelState &state) {
    intervals.emplace_back(DumpInterval<ChannelState>(state));
  }

  int writeDuration(const int ticks, const int remainder, const int freq) {
    int total = ticks + remainder;
    int cycles = total / freq;
    intervals.back().duration = cycles;
    return total - (cycles * freq);
  }

  size_t size() {
    return intervals.size();
  }

  uint64_t hash() {
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
template <typename ChannelState> 
void captureSequences(
  DivEngine* e, 
  DivSystem system, 
  int channel,
  std::map<unsigned int, unsigned int> &addressMap,
  std::map<String, DumpSequence<ChannelState>> &sequences
) {
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
  for (size_t subsong = 0; subsong < e->song.subsong.size(); subsong++) {
    e->changeSongP(subsong);
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
    auto it = sequences.emplace(key, DumpSequence<ChannelState>());
    DumpSequence<ChannelState> *currentDumpSequence = &(it.first->second);

    ChannelState currentState(0);

    bool done=false;
    while (!done && e->isPlaying()) {
      
      done = e->nextTick(false, true);
      int currentTicks = e->getTotalTicks();
      int currentSeconds = e->getTotalSeconds();
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
        auto nextIt = sequences.emplace(key, DumpSequence<ChannelState>());
        currentDumpSequence = &(nextIt.first->second);
        needsRegisterDump = true;
      }

      // get register dumps
      bool isDirty = false;
      for (int i=0; i<e->song.systemLen; i++) {
        std::vector<DivRegWrite>& registerWrites=e->getDispatch(i)->getRegisterWrites();
        for (DivRegWrite& registerWrite: registerWrites) {
          auto it = addressMap.find(registerWrite.addr);
          if (it == addressMap.end()) {
            continue;
          }
          isDirty |= currentState.write(it->second, registerWrite.val);
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
      int currentTicks = e->getTotalTicks();
      int currentSeconds = e->getTotalSeconds();
      deltaTicks = 
        currentTicks - lastWriteTicks + 
        (TICKS_PER_SECOND * (currentSeconds - lastWriteSeconds));
      // final seq
      currentDumpSequence->writeDuration(deltaTicks, deltaTicksR, TICKS_AT_60HZ);
    }
  }
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }
}

template <typename ChannelState> 
void findCommonSubsequences(
  std::map<String, DumpSequence<ChannelState>> &sequences,
  std::map<uint64_t, String> &commonSubSequences,
  std::map<uint64_t, unsigned int> &sequenceFrequency,
  std::map<String, String> &representativeSequenceMap
) {
  for (auto& x: sequences) {
    uint64_t hash = x.second.hash();
    auto it = commonSubSequences.emplace(hash, x.first);
    if (it.second) {
      sequenceFrequency.emplace(hash, 1);
    } else {
      sequenceFrequency[hash] += 1;
    }
    // TODO: verify no hash collision...?
    representativeSequenceMap.emplace(x.first, it.first->second);
  }
}

#endif // _REGISTERDUMP_H