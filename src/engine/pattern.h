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

struct DivPattern {
  String name;
  short data[256][32];

  /**
   * copy this pattern to another.
   * @param dest the destination pattern.
   */
  void copyOn(DivPattern* dest);

  /**
   * don't use yet!
   * @param len the pattern length
   * @param fxRows number of effect ...columns
   * @return a SafeReader.
   */
  SafeReader* compile(int len=256, int fxRows=1);
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
  DivPattern* data[256];

  /**
   * get a pattern from this channel, or the empty pattern if not initialized.
   * @param index the pattern ID.
   * @param create whether to initialize a new pattern if not init'ed. always use true if you're going to modify it!
   * @return a DivPattern.
   */
  DivPattern* getPattern(int index, bool create);

  /**
   * destroy all patterns on this DivChannelData.
   */
  void wipePatterns();
  DivChannelData();
};
