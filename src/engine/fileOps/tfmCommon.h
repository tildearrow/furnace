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

#ifndef TFM_COMMON_H
#define TFM_COMMON_H

#include "fileOpsCommon.h"

class TFMRLEReader;

struct TFMEndOfFileException {
  TFMRLEReader* reader;
  size_t finalSize;
  TFMEndOfFileException(TFMRLEReader* r, size_t fs):
    reader(r),
    finalSize(fs) {}
};


class TFMRLEReader {
  const unsigned char* buf;
  size_t len;
  size_t curSeek;
  bool inTag;
  int tagLenLeft;
  signed char tagChar;

  void decodeRLE(unsigned char prevChar) {
    int lenShift=0;
    tagLenLeft=0;
    unsigned char rleTag=0;

    do {
      rleTag=readCNoRLE();
      tagLenLeft|=(rleTag&0x7F)<<lenShift;
      lenShift+=7;
      //logD("offset: %x, RLE tag: %X, len shift: %d, len left: %d",curSeek,rleTag,lenShift,tagLenLeft);
    } while (!(rleTag&0x80));

    if (tagLenLeft) {
      // sync back since we've already read one character
      inTag=true;
      tagLenLeft--;
      tagChar=prevChar;
    } else {
      tagChar=0x80;
    }
    //logD("tag finished: len left: %d, char: %X",tagLenLeft,tagChar);
  }

public:
  TFMRLEReader(const void* b, size_t l) :
    buf((const unsigned char*)b),
    len(l),
    curSeek(0),
    inTag(false),
    tagLenLeft(0),
    tagChar(0) {}

  // these functions may throw TFMEndOfFileException
  unsigned char readC() {
    if (inTag) {
      if (tagLenLeft<=0) {
        inTag=false;
        return readC();
      }
      tagLenLeft--;
      //logD("one char RLE decompressed, tag left: %d, char: %d",tagLenLeft,tagChar);
      return tagChar;
    }
    if (curSeek>len) throw TFMEndOfFileException(this,len);

    unsigned char ret=buf[curSeek++];

    // MISLEADING DOCUMENTATION: while TFM music maker's documentation says if the next byte
    // is zero, then it's not a tag but just 0x80 (for example: 0x00 0x80 0x00 = 0x00 0x80)
    // this is actually wrong
    // through research and experimentation, there are times that TFM music maker
    // will use 0x80 0x00 for actual tags (for example: 0x00 0x80 0x00 0x84 = 512 times 0x00
    // in certain parts of the header and footer)
    // TFM music maker actually uses double 0x80 to escape the 0x80
    // for example: 0xDA 0x80 0x80 0x00 0x23 = 0xDA 0x80 0x00 0x23)
    if (ret==0x80) {
      decodeRLE(buf[curSeek-2]);
      tagLenLeft--;
      return tagChar;
    }
    return ret;
  }

  signed char readCNoRLE() {
    if (curSeek>len) throw TFMEndOfFileException(this,len);
    return buf[curSeek++];
  }

  void read(unsigned char* b, size_t l) {
    int i=0;
    while(l--) {
      unsigned char nextChar=readC();
      b[i++]=nextChar;
      //logD("read next char: %x, index: %d",nextChar,i);
    }
  }

  void readNoRLE(unsigned char *b, size_t l) {
    int i=0;
    while (l--) {
      b[i++]=buf[curSeek++];
      if (curSeek>len) throw TFMEndOfFileException(this,len);
    }
  }

  short readS() {
    return readC()|readC()<<8;
  }

  short readSNoRLE() {
    if (curSeek+2>len) throw TFMEndOfFileException(this,len);
    short ret=buf[curSeek]|buf[curSeek+1]<<8;
    curSeek+=2;
    return ret;
  }

  String readString(size_t l) {
    String ret;
    ret.reserve(l);
    while (l--) {
      unsigned char byte=readC();
      if (!byte) {
        skip(l);
        break;
      }
      ret += byte;
    }
    return ret;
  }

  unsigned int readI() {
    return readC()|readC()<<8|readC()<<16|readC()<<24;
  }

  short readINoRLE() {
    if (curSeek+4>len) throw TFMEndOfFileException(this,len);
    short ret=buf[curSeek]|buf[curSeek+1]<<8|buf[curSeek+2]<<16|buf[curSeek+3]<<24;
    curSeek+=4;
    return ret;
  }

  void skip(size_t l) {
    // quick and dirty
    while (l--) {
      //logD("skipping l %d",l);
      readC();
    }
  }

};

String TFMparseDate(short date) {
  return fmt::sprintf("%02d.%02d.%02d",date>>11,(date>>7)&0xF,date&0x7F);
}

struct TFMSpeed {
  unsigned char speedEven;
  unsigned char speedOdd;
  unsigned char interleaveFactor;

  bool operator==(const TFMSpeed &s) const {
    return speedEven==s.speedEven && speedOdd==s.speedOdd && interleaveFactor==s.interleaveFactor;
  }
};

// to make it work with map
namespace std {
  template<> struct hash<TFMSpeed>
  {
    size_t operator()(const TFMSpeed& s) const noexcept {
      return s.speedEven<<16|s.speedOdd<<8|s.interleaveFactor;
    }
  };
}

#endif
