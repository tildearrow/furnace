/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "assetDir.h"
#include "../ta-log.h"

void moveAsset(std::vector<DivAssetDir>& dir, int before, int after) {
  if (before<0 || after<0) return;
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase matching entry
      if (i.entries[j]==before) {
        i.entries[j]=after;
      } else if (i.entries[j]==after) {
        i.entries[j]=before;
      }
    }
  }
}

void removeAsset(std::vector<DivAssetDir>& dir, int entry) {
  if (entry<0) return;
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase matching entry
      if (i.entries[j]==entry) {
        i.entries.erase(i.entries.begin()+j);
        j--;
      } else if (i.entries[j]>entry) {
        i.entries[j]--;
      }
    }
  }
}

void checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries) {
  bool* inAssetDir=new bool[entries];
  memset(inAssetDir,0,entries*sizeof(bool));

  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase invalid entry
      if (i.entries[j]<0 || i.entries[j]>=(int)entries) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }

      // erase duplicate entry
      if (inAssetDir[i.entries[j]]) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }
      
      // mark entry as present
      inAssetDir[i.entries[j]]=true;
    }
  }

  // get unsorted directory
  DivAssetDir* unsortedDir=NULL;
  for (DivAssetDir& i: dir) {
    if (i.name.empty()) {
      unsortedDir=&i;
      break;
    }
  }

  // add missing items to unsorted directory
  for (size_t i=0; i<entries; i++) {
    if (!inAssetDir[i]) {
      // create unsorted directory if it doesn't exist
      if (unsortedDir==NULL) {
        dir.push_back(DivAssetDir(""));
        unsortedDir=&(*dir.rbegin());
      }
      unsortedDir->entries.push_back(i);
    }
  }

  delete[] inAssetDir;
}

void putAssetDirData(SafeWriter* w, std::vector<DivAssetDir>& dir) {
  size_t blockStartSeek, blockEndSeek;

  w->write("ADIR",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeI(dir.size());

  for (DivAssetDir& i: dir) {
    w->writeString(i.name,false);
    w->writeS(i.entries.size());
    for (int j: i.entries) {
      w->writeC(j);
    }
  }

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);
}

DivDataErrors readAssetDirData(SafeReader& reader, std::vector<DivAssetDir>& dir) {
  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"ADIR",4)!=0) {
    logV("header is invalid: %c%c%c%c",magic[0],magic[1],magic[2],magic[3]);
    return DIV_DATA_INVALID_HEADER;
  }
  reader.readI(); // reserved

  unsigned int numDirs=reader.readI();

  dir.reserve(numDirs);
  for (unsigned int i=0; i<numDirs; i++) {
    DivAssetDir d;

    d.name=reader.readString();
    unsigned short numEntries=reader.readS();

    d.entries.reserve(numEntries);
    for (unsigned short j=0; j<numEntries; j++) {
      d.entries.push_back(((unsigned char)reader.readC()));
    }

    dir.push_back(d);
  }

  return DIV_DATA_SUCCESS;
}

