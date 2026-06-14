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

#ifndef _ASSET_DIR_H
#define _ASSET_DIR_H

#include "../ta-utils.h"
#include <vector>
#include "dataErrors.h"
#include "safeReader.h"
#include "safeWriter.h"

/**
 * an asset directory is called a "folder" in the GUI.
 * it allows the user to group several assets and give that group a name.
 * the list of folders is contained in an std::vector within DivSong.
 */
struct DivAssetDir {
  // the directory's name.
  String name;
  // entry asset IDs.
  std::vector<int> entries;

  DivAssetDir():
    name("New Directory") {}
  DivAssetDir(String n):
    name(n) {}
};

/**
 * check whether an asset directory is complete (UNSAFE).
 * @param dir a set of asset directories.
 * @param entries the number of assets. not the number of entries in the asset dir!
 */
void checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries);

/**
 * move an asset.
 * @param dir a set of asset directories.
 * @param before asset ID which has moved.
 * @param after asset ID's new ID.
 */
void moveAsset(std::vector<DivAssetDir>& dir, int before, int after);

/**
 * remove an asset.
 * @param dir a set of asset directories.
 * @param entry the asset ID.
 */
void removeAsset(std::vector<DivAssetDir>& dir, int entry);

/**
 * write asset directory.
 * @param w a SafeWriter.
 * @param dir the asset directory.
 */
void putAssetDirData(SafeWriter* w, std::vector<DivAssetDir>& dir);

/**
 * read asset directory.
 * @param reader a SafeReader.
 * @param dir the asset directory.
 * @return whether an error occurred.
 */
DivDataErrors readAssetDirData(SafeReader& reader, std::vector<DivAssetDir>& dir);

#endif
