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

#include "registerDump.h"

void captureSequences(
  DivEngine* e, 
  DivSystem system, 
  int channel,
  std::map<unsigned int, unsigned int> &addressMap,
  std::map<String, DumpSequence> &sequences,
  bool breakOnRow
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
    auto it = sequences.emplace(key, DumpSequence());
    DumpSequence *currentDumpSequence = &(it.first->second);

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
        if (breakOnRow) {
          key = getSequenceKey(curRowIndex.subsong, curRowIndex.ord, curRowIndex.row, channel);
          auto nextIt = sequences.emplace(key, DumpSequence());
          currentDumpSequence = &(nextIt.first->second);
        }
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

void findCommonSubsequences(
  std::map<String, DumpSequence> &sequences,
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