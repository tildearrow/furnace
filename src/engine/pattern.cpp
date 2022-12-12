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

#include "engine.h"
#include "../ta-log.h"

static DivPattern emptyPat;

DivPattern::DivPattern() {
  memset(data,-1,DIV_MAX_ROWS*DIV_MAX_COLS*sizeof(short));
  for (int i=0; i<DIV_MAX_ROWS; i++) {
    data[i][0]=0;
    data[i][1]=0;
  }
}

DivPattern* DivChannelData::getPattern(int index, bool create) {
  if (data[index]==NULL) {
    if (create) {
      data[index]=new DivPattern;
    } else {
      return &emptyPat;
    }
  }
  return data[index];
}

std::vector<std::pair<int,int>> DivChannelData::optimize() {
  std::vector<std::pair<int,int>> ret;
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    if (data[i]!=NULL) {
      // compare
      for (int j=0; j<DIV_MAX_PATTERNS; j++) {
        if (j==i) continue;
        if (data[j]==NULL) continue;
        if (memcmp(data[i]->data,data[j]->data,DIV_MAX_ROWS*DIV_MAX_COLS*sizeof(short))==0) {
          delete data[j];
          data[j]=NULL;
          logV("%d == %d",i,j);
          ret.push_back(std::pair<int,int>(j,i));
        }
      }
    }
  }
  return ret;
}

std::vector<std::pair<int,int>> DivChannelData::rearrange() {
  std::vector<std::pair<int,int>> ret;
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    if (data[i]==NULL) {
      for (int j=i; j<DIV_MAX_PATTERNS; j++) {
        if (data[j]!=NULL) {
          data[i]=data[j];
          data[j]=NULL;
          logV("%d -> %d",j,i);
          ret.push_back(std::pair<int,int>(j,i));
          if (++i>=DIV_MAX_PATTERNS) break;
        }
      }
    }
  }
  return ret;
}

void DivChannelData::wipePatterns() {
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    if (data[i]!=NULL) {
      delete data[i];
      data[i]=NULL;
    }
  }
}

void DivPattern::copyOn(DivPattern* dest) {
  dest->name=name;
  memcpy(dest->data,data,sizeof(data));
}

DivChannelData::DivChannelData():
  effectCols(1) {
  memset(data,0,DIV_MAX_PATTERNS*sizeof(void*));
}
