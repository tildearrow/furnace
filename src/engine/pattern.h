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

#include "safeReader.h"
#include <vector>

struct DivPattern {
  String name;
  short data[DIV_MAX_ROWS][DIV_MAX_COLS];

  /**
   * copy this pattern to another.
   * @param dest the destination pattern.
   */
  void copyOn(DivPattern* dest);
  DivPattern();
};

struct DivChannelData {
  unsigned char effectCols;
  // data goes as follows: data[ROW][TYPE]
  // TYPE is:
  // 0: note
  // 1: octave
  // 2: instrument
  // 3: volume
  // 4-5+: effect/effect value
  // do NOT access directly unless you know what you're doing!
  DivPattern* data[DIV_MAX_PATTERNS];

  /**
   * get a pattern from this channel, or the empty pattern if not initialized.
   * @param index the pattern ID.
   * @param create whether to initialize a new pattern if not init'ed. always use true if you're going to modify it!
   * @return a DivPattern.
   */
  DivPattern* getPattern(int index, bool create);

  /**
   * optimize pattern data.
   * not thread-safe! use a mutex!
   * @return a list of From -> To pairs
   */
  std::vector<std::pair<int,int>> optimize();

  /**
   * re-arrange NULLs.
   * not thread-safe! use a mutex!
   * @return a list of From -> To pairs
   */
  std::vector<std::pair<int,int>> rearrange();

  /**
   * destroy all patterns on this DivChannelData.
   */
  void wipePatterns();
  DivChannelData();
};
