/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
  // safety check
  if (before<0 || after<0) return;

  // check entries in asset directories
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // swap matching entries
      if (i.entries[j]==before) {
        i.entries[j]=after;
      } else if (i.entries[j]==after) {
        i.entries[j]=before;
      }
    }
  }
}

void removeAsset(std::vector<DivAssetDir>& dir, int entry) {
  // safety check
  if (entry<0) return;

  // find entry in asset directories
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      if (i.entries[j]==entry) {
        // erase matching entry
        i.entries.erase(i.entries.begin()+j);
        j--;
      } else if (i.entries[j]>entry) {
        // asset indexes higher than the matching asset must be decreased
        // by one as their indexes have changed
        i.entries[j]--;
      }
    }
  }
}

void checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries) {
  // check whether all assets are present in asset directories.
  // also check whether there are duplicates.
  bool* inAssetDir=new bool[entries];
  memset(inAssetDir,0,entries*sizeof(bool));

  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase invalid/out of range entries
      if (i.entries[j]<0 || i.entries[j]>=(int)entries) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }

      // erase duplicate entries
      if (inAssetDir[i.entries[j]]) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }
      
      // mark entry as present
      inAssetDir[i.entries[j]]=true;
    }
  }

  // find the "unsorted" directory
  DivAssetDir* unsortedDir=NULL;
  for (DivAssetDir& i: dir) {
    // the "unsorted" directory is simply a directory without a name
    // the GUI will not allow you to create an anonymous directory
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

  // clean up
  delete[] inAssetDir;
}

// write asset directories to a SafeWriter
void putAssetDirData(SafeWriter* w, std::vector<DivAssetDir>& dir) {
  size_t blockStartSeek, blockEndSeek;

  // block header
  w->write("ADI2",4);
  blockStartSeek=w->tell();
  w->writeI(0); // block size - will be written later

  // number of directories
  w->writeI(dir.size());

  // for each directory
  for (DivAssetDir& i: dir) {
    // directory name
    w->writeString(i.name,false);
    // entry count
    w->writeS(i.entries.size());
    // write entries
    for (int j: i.entries) {
      w->writeS(j);
    }
  }

  // go back to the block size location
  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  // calculate block size and write it out
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);
}

// read asset directories.
DivDataErrors readAssetDirData(SafeReader& reader, std::vector<DivAssetDir>& dir) {
  bool isNewFormat=true;

  // check whether the block header is correct
  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"ADI2",4)==0) {
    // asset dirs - new format
    isNewFormat=true;
  } else if (memcmp(magic,"ADIR",4)==0) {
    // asset dirs - old format
    isNewFormat=false;
  } else {
    // invalid block header
    logV("header is invalid: %c%c%c%c",magic[0],magic[1],magic[2],magic[3]);
    return DIV_DATA_INVALID_HEADER;
  }
  reader.readI(); // block size

  // read number of directories
  unsigned int numDirs=reader.readI();

  // create directories
  dir.reserve(numDirs);
  for (unsigned int i=0; i<numDirs; i++) {
    DivAssetDir d;

    // name
    d.name=reader.readString();
    // entry count
    unsigned short numEntries=reader.readS();

    d.entries.reserve(numEntries);
    for (unsigned short j=0; j<numEntries; j++) {
      // read entries
      if (isNewFormat) {
        d.entries.push_back(((unsigned short)reader.readS()));
      } else {
        d.entries.push_back(((unsigned char)reader.readC()));
      }
    }

    // add directory
    dir.push_back(d);
  }

  return DIV_DATA_SUCCESS;
}

