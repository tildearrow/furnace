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

void captureSequence(
  DivEngine* e, 
  int subsong,
  int channel,
  DivSystem system, 
  std::map<unsigned int, unsigned int> &addressMap,
  std::vector<String> &sequence,
  std::map<String, DumpSequence> &registerDumps
) {
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(true);
  }
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
  sequence.emplace_back(key);
  auto it = registerDumps.emplace(key, DumpSequence());
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
      key = getSequenceKey(curRowIndex.subsong, curRowIndex.ord, curRowIndex.row, channel);
      sequence.emplace_back(key);
      auto nextIt = registerDumps.emplace(key, DumpSequence());
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
  for (int i=0; i<e->song.systemLen; i++) {
    e->getDispatch(i)->toggleRegisterDump(false);
  }
}



// TODO: remap sequence to common subsequences
// TODO: find common suffixes
// TODO: compress the common suffixes
// TODO: emit the song with merged dumps and common phrases

void findCommonDumpSequences(
  const std::map<String, DumpSequence> &registerDumps,
  std::map<uint64_t, String> &commonDumpSequences,
  std::map<uint64_t, unsigned int> &frequencyMap,
  std::map<String, String> &representativeMap
) {
  for (auto& x: registerDumps) {
    uint64_t hash = x.second.hash();
    auto it = commonDumpSequences.emplace(hash, x.first);
    if (it.second) {
      frequencyMap.emplace(hash, 1);
    } else {
      frequencyMap[hash] += 1;
    }
    // TODO: verify no hash collision...?
    representativeMap.emplace(x.first, it.first->second);
  }
}

struct SuffixTree {

  SuffixTree *parent;
  SuffixTree *slink;
  std::vector<SuffixTree *> children;
  size_t start;
  size_t depth;

  SuffixTree(size_t alphabetSize, size_t d) : parent(NULL), slink(NULL), start(0), depth(d) {
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

  SuffixTree *splice_node(size_t d, const std::vector<size_t> &T) {
    assert(d < depth);
    SuffixTree *child = new SuffixTree(children.size(), d);
    size_t i = start;
    logD("node %d %d", i, d);
    child->start = i;
    child->parent = parent;
    child->children[T.at(i + d)] = this;
    parent->children[T.at(i + parent->depth)] = child;
    parent = child;
    return child;
  }

  SuffixTree *add_leaf(size_t i, size_t d, const std::vector<size_t> &T) {
    logD("leaf %d %d", i, d);
    SuffixTree *child = new SuffixTree(children.size(), T.size() - i);
    child->start = i;
    child->parent = this;
    children[T.at(i + d)] = child;
    return child;
  }

  void compute_slink(const std::vector<size_t> &T) {
    size_t d = depth;
    SuffixTree *v = parent->slink;
    while (v->depth < d - 1) {
      v = v->children.at(T[start + v->depth + 1]);
    }
    if (v->depth > d - 1) {
      v = v->splice_node(d - 1, T);
    }
    slink = v;
  }

  size_t substring_start() {
    return start + (NULL != parent ? parent->depth : 0);
  }

  size_t substring_end() {
    return start + depth;
  }

  size_t substring_len() {
    return substring_end() - substring_start();
  }

};

// build a suffix tree via McCreight's algorithm
// https://www.cs.helsinki.fi/u/tpkarkka/opetus/13s/spa/lecture09-2x4.pdf
//
SuffixTree * createSuffixTree(
  const std::vector<String> &sequence,
  const std::map<uint64_t, String> &commonDumpSequences,
  const std::map<String, String> &representativeMap,
  std::vector<uint64_t> &alphabet
) {
  // construct the alphabet
  alphabet.reserve(commonDumpSequences.size());
  std::map<String, size_t> index;
  for (auto x : commonDumpSequences) {
    index.emplace(x.second, alphabet.size());
    alphabet.emplace_back(x.first);
  }

  // copy string in alphabet
  std::vector<size_t> alphaSequence;
  alphaSequence.reserve(sequence.size()); 
  for (auto key : sequence) {
    alphaSequence.emplace_back(index.at(representativeMap.at(key)));
  }

  // construct suffix tree
  size_t d = 0;
  SuffixTree *root = new SuffixTree(alphabet.size(), d);
  root->slink = root;
  SuffixTree *u = root;
  for (int i = 0; i < alphaSequence.size(); i++) {
    while (d == u->depth) {
      SuffixTree *child = u->children[alphaSequence[i + d]];
      if (NULL == child) break;
      u = child;
      d = d + 1;
      while ((d < u->depth) && (alphaSequence[u->start + d] == alphaSequence[i + d])) {
        d = d + 1;
      }
    } 
    if (d < u->depth) {
      u = u->splice_node(d, alphaSequence);
    }
    u->add_leaf(i, d, alphaSequence);
    if (NULL == u->slink) {
      u->compute_slink(alphaSequence);
    }
    logD("depth %d %d", d, u->depth);
    u = u->slink;
    logD("depth %d %d", d, u->depth);
    d = u->depth;
  }

  // stats
  logD("Alphabet size %d", alphabet.size());
  logD("Sequence length %d", alphaSequence.size());

  // produce root 
  return root;

}

void testCommonSubsequences(const String &input) {

  std::vector<String> sequence;
  std::map<uint64_t, String> commonDumpSequences;
  std::map<String, String> representativeMap;
  for (int i = 0; i < input.size(); i++) {
    char c = input[i];
    String key = input.substr(i, 1);
    sequence.emplace_back(key);
    representativeMap.emplace(key, key);
    uint64_t hash = (u_int64_t)c;
    commonDumpSequences.emplace(hash, key);
  }

  std::vector<uint64_t> alphabet;
  SuffixTree *root = createSuffixTree(
    sequence,
    commonDumpSequences,
    representativeMap,
    alphabet
  );

  std::vector<SuffixTree *> stack;
  stack.emplace_back(root);
  while (stack.size() > 0) {
    SuffixTree * u = stack.back();
    String l = input.substr(u->substring_start(), u->substring_len());
    stack.pop_back();
    logD("NODE %s: start=%d, depth=%d", l, u->start, u->depth);
    for (int i = 0; i < u->children.size(); i++) {
      SuffixTree * child = u->children[i];
      if (NULL == child) continue;
      String c = input.substr(child->substring_start(), child->substring_len());
      logD("   CHILD %s", c);
      stack.push_back(child);
    }
  }

}