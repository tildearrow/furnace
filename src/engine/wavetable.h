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

#ifndef _WAVETABLE_H
#define _WAVETABLE_H
#include "safeWriter.h"
#include "dataErrors.h"

struct DivWavetable {
  int len, min, max;
  int data[256];

  /**
   * save the wavetable to a SafeWriter.
   * @param w the SafeWriter in question.
   */
  void putWaveData(SafeWriter* w);

  /**
   * read wavetable data in .fuw format.
   * @param reader the reader.
   * @param version the format version.
   * @return a DivDataErrors.
   */
  DivDataErrors readWaveData(SafeReader& reader, short version);

  /**
   * save this wavetable to a file.
   * @param path file path.
   * @return whether it was successful.
   */
  bool save(const char* path);

  /**
   * save this wavetable to a file in .dmw format.
   * @param path file path.
   * @return whether it was successful.
   */
  bool saveDMW(const char* path);

  /**
   * save this wavetable to a file in raw format.
   * @param path file path.
   * @return whether it was successful.
   */
  bool saveRaw(const char* path);
  DivWavetable():
    len(32),
    min(0),
    max(31) {
    for (int i=0; i<256; i++) {
      data[i]=i;
    }
  }
};

#endif
