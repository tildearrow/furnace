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

#ifndef _ASSET_DIR_H
#define _ASSET_DIR_H

#include "../ta-utils.h"
#include <vector>
#include "dataErrors.h"
#include "safeReader.h"
#include "safeWriter.h"

struct DivAssetDir {
  String name;
  std::vector<int> entries;

  DivAssetDir():
    name("New Directory") {}
  DivAssetDir(String n):
    name(n) {}
};

// check whether an asset directory is complete (UNSAFE)
void checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries);

// move an asset
void moveAsset(std::vector<DivAssetDir>& dir, int before, int after);

// remove an asset
void removeAsset(std::vector<DivAssetDir>& dir, int entry);

// read/write asset dir
void putAssetDirData(SafeWriter* w, std::vector<DivAssetDir>& dir);
DivDataErrors readAssetDirData(SafeReader& reader, std::vector<DivAssetDir>& dir);

#endif
