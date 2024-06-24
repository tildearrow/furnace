/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "song.h"
#include "../ta-log.h"

void DivSubSong::clearData() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    pat[i].wipePatterns();
  }

  memset(orders.ord,0,DIV_MAX_CHANS*DIV_MAX_PATTERNS);
  ordersLen=1;
}

void DivSubSong::optimizePatterns() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    logD("optimizing channel %d...",i);
    std::vector<std::pair<int,int>> clearOuts=pat[i].optimize();
    for (auto& j: clearOuts) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (orders.ord[i][k]==j.first) {
          orders.ord[i][k]=j.second;
        }
      }
    }
  }
}

void DivSubSong::rearrangePatterns() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    logD("re-arranging channel %d...",i);
    std::vector<std::pair<int,int>> clearOuts=pat[i].rearrange();
    for (auto& j: clearOuts) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (orders.ord[i][k]==j.first) {
          orders.ord[i][k]=j.second;
        }
      }
    }
  }
}

void DivSubSong::sortOrders() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    DivPattern* patPointer[DIV_MAX_PATTERNS];
    unsigned char orderMap[DIV_MAX_PATTERNS];
    bool seen[DIV_MAX_PATTERNS];
    int orderMapLen=0;

    memcpy(patPointer,pat[i].data,DIV_MAX_PATTERNS*sizeof(void*));
    memset(orderMap,0,DIV_MAX_PATTERNS);
    memset(seen,0,DIV_MAX_PATTERNS*sizeof(bool));

    // 1. sort orders
    for (int j=0; j<ordersLen; j++) {
      if (!seen[orders.ord[i][j]]) {
        orderMap[orders.ord[i][j]]=orderMapLen++;
        seen[orders.ord[i][j]]=true;
      }
    }

    // 2. populate the rest
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (!seen[j]) orderMap[j]=orderMapLen++;
    }
    
    // 3. swap pattern pointers
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      pat[i].data[orderMap[j]]=patPointer[j];
    }

    // 4. swap orders
    for (int j=0; j<ordersLen; j++) {
      orders.ord[i][j]=orderMap[orders.ord[i][j]];
    }
  }
}

void DivSubSong::makePatUnique() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    logD("making channel %d unique...",i);
    bool seen[DIV_MAX_PATTERNS];
    bool used[DIV_MAX_PATTERNS];

    memset(seen,0,DIV_MAX_PATTERNS*sizeof(bool));
    memset(used,0,DIV_MAX_PATTERNS*sizeof(bool));

    // 1. populate used patterns
    for (int j=0; j<ordersLen; j++) {
      used[orders.ord[i][j]]=true;
    }

    // 2. make patterns unique
    for (int j=0; j<ordersLen; j++) {
      if (seen[orders.ord[i][j]]) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (!used[k]) {
            // copy here
            DivPattern* dest=pat[i].getPattern(k,true);
            DivPattern* src=pat[i].getPattern(orders.ord[i][j],false);
            src->copyOn(dest);
            used[k]=true;
            orders.ord[i][j]=k;
            break;
          }
        }
      } else {
        seen[orders.ord[i][j]]=true;
      }
    }
  }
}

void DivSong::findSubSongs() {

}

void DivSong::clearSongData() {
  for (DivSubSong* i: subsong) {
    i->clearData();
    delete i;
  }
  subsong.clear();
  subsong.push_back(new DivSubSong);
}

void DivSong::clearInstruments() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();
  insLen=0;
}

void DivSong::clearWavetables() {
  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();
  waveLen=0;
}

void DivSong::clearSamples() {
  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();
  sampleLen=0;
}

void DivSong::unload() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();
  insLen=0;

  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();
  waveLen=0;

  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();
  sampleLen=0;

  for (DivSubSong* i: subsong) {
    i->clearData();
    delete i;
  }
  subsong.clear();
}
