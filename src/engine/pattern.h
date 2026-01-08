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

#include "safeReader.h"
#include "../pch.h"

struct DivPattern {
  String name;
  /**
   * pattern data is stored in this order:
   * - 0 note: 0 (min) is C-(-5), 60 is C-0, and 179 (max) is B-9.
               252 is null/bug, 253 is note off, 254 is note release and
               255 is macro release.
   * - 1 instrument
   * - 2 volume
   * - 3 effect
   * - 4 effect value
   * - 5... (the rest of effects/effect values)
   *
   * use the DIV_PAT_* macros in defines.h for convenience.
   *
   * if a cell is -1, it means "empty".
   */
  short newData[DIV_MAX_ROWS][DIV_MAX_COLS];

  /**
   * check whether this pattern is empty.
   * @return whether it is.
   */
  bool isEmpty();

  /**
   * clear the pattern.
   */
  void clear();

  /**
   * copy this pattern to another.
   * @param dest the destination pattern.
   */
  void copyOn(DivPattern* dest);
  DivPattern();
};

struct DivChannelData {
  unsigned char effectCols;
  /**
   * do NOT access directly unless you know what you're doing!
   * use getPattern() instead.
   */
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
