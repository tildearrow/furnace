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

#ifndef _SAFEREADER_H
#define _SAFEREADER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../ta-utils.h"

enum Endianness {
  LittleEndian=0,
  BigEndian
};

enum DivStringEncoding {
  DIV_ENCODING_NONE=0,
  DIV_ENCODING_UTF8,
  DIV_ENCODING_LATIN1,
  DIV_ENCODING_LATIN1_SPECIAL,
  DIV_ENCODING_SHIFT_JIS
};

class SafeReader;

struct EndOfFileException {
  SafeReader* reader;
  size_t finalSize;
  EndOfFileException(SafeReader* r, size_t fs):
    reader(r),
    finalSize(fs) {}
};

class SafeReader {
  const unsigned char* buf;
  size_t len;

  size_t curSeek;

  public:
    bool seek(ssize_t where, int whence);
    size_t tell();
    size_t size();

    int read(void* where, size_t count);

    // these functions may throw EndOfFileException.
    signed char readC();
    short readS();
    short readS_BE();
    int readI();
    int readI_BE();
    int64_t readL();
    int64_t readL_BE();
    float readF();
    float readF_BE();
    double readD();
    double readD_BE();
    String readStringWithEncoding(DivStringEncoding encoding);
    String readStringWithEncoding(DivStringEncoding encoding, size_t len);
    String readString();
    String readString(size_t len);
    String readStringLatin1();
    String readStringLatin1(size_t len);
    String readStringLatin1Special();
    String readStringLatin1Special(size_t len);
    String readStringLine();
    String readStringToken(unsigned char delim, bool stripContiguous);
    String readStringToken();
    inline bool isEOF() { return curSeek >= len; };

    SafeReader(const void* b, size_t l):
      buf((const unsigned char*)b),
      len(l),
      curSeek(0) {}
};

#endif
