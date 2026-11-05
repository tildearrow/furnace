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

#ifndef _SAFEWRITER_H
#define _SAFEWRITER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "safeReader.h"
#include "../ta-utils.h"

class SafeWriter {
  bool operative;
  unsigned char* buf;
  size_t bufLen;
  size_t len;

  size_t curSeek;

  void checkSize(size_t amount);

  public:
    unsigned char* getFinalBuf();

    bool seek(ssize_t where, int whence);
    size_t tell();
    size_t size();

    int write(const void* what, size_t count);

    int writeC(signed char val);
    int writeS(short val);
    int writeS_BE(short val);
    int writeI(int val);
    int writeI_BE(int val);
    int writeL(int64_t val);
    int writeL_BE(int64_t val);
    int writeF(float val);
    int writeF_BE(float val);
    int writeD(double val);
    int writeD_BE(double val);
    int writeWString(WString val, bool pascal);
    int writeString(String val, bool pascal);
    int writeText(String val);

    void init();
    SafeReader* toReader();
    void finish();
    void disown();

    SafeWriter():
      operative(false),
      buf(NULL),
      bufLen(0),
      len(0),
      curSeek(0) {}
};

#endif
