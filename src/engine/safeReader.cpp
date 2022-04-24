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
#include "../ta-log.h"

//#define READ_DEBUG

bool SafeReader::seek(ssize_t where, int whence) {
  switch (whence) {
    case SEEK_SET:
      if (where<0) return false;
      if (where>(ssize_t)len) return false;
      curSeek=where;
      break;
    case SEEK_CUR: {
      ssize_t finalSeek=curSeek+where;
      if (finalSeek<0) return false;
      if (finalSeek>(ssize_t)len) return false;
      curSeek=finalSeek;
      break;
    }
    case SEEK_END: {
      ssize_t finalSeek=len-where;
      if (finalSeek<0) return false;
      if (finalSeek>(ssize_t)len) return false;
      curSeek=finalSeek;
      break;
    }
  }
  return true;
}

size_t SafeReader::tell() {
  return curSeek;
}

size_t SafeReader::size() {
  return len;
}

int SafeReader::read(void* where, size_t count) {
#ifdef READ_DEBUG
  logD("SR: reading %d bytes at %x",count,curSeek);
#endif
  if (count==0) return 0;
  if (curSeek+count>len) throw EndOfFileException(this,len);
  memcpy(where,&buf[curSeek],count);
  curSeek+=count;
  return count;
}

signed char SafeReader::readC() {
#ifdef READ_DEBUG
  logD("SR: reading char %x:",curSeek);
#endif
  if (curSeek+1>len) throw EndOfFileException(this,len);
#ifdef READ_DEBUG
  logD("SR: %.2x",buf[curSeek]);
#endif
  return (signed char)buf[curSeek++];
}

short SafeReader::readS() {
#ifdef READ_DEBUG
  logD("SR: reading short %x:",curSeek);
#endif
  if (curSeek+2>len) throw EndOfFileException(this,len);
  short ret=*(short*)(&buf[curSeek]);
#ifdef READ_DEBUG
  logD("SR: %.4x",ret);
#endif
  curSeek+=2;
  return ret;
}

short SafeReader::readS_BE() {
  if (curSeek+2>len) throw EndOfFileException(this,len);
  short ret=*(short*)(&buf[curSeek]);
  curSeek+=2;
  return ((ret>>8)&0xff)|(ret<<8);
}

int SafeReader::readI() {
#ifdef READ_DEBUG
  logD("SR: reading int %x:",curSeek);
#endif
  if (curSeek+4>len) throw EndOfFileException(this,len);
  int ret=*(int*)(&buf[curSeek]);
  curSeek+=4;
#ifdef READ_DEBUG
  logD("SR: %.8x",ret);
#endif
  return ret;
}

int SafeReader::readI_BE() {
  if (curSeek+4>len) throw EndOfFileException(this,len);
  unsigned int ret=*(unsigned int*)(&buf[curSeek]);
  curSeek+=4;
  return (int)((ret>>24)|((ret&0xff0000)>>8)|((ret&0xff00)<<8)|((ret&0xff)<<24));
}

int64_t SafeReader::readL() {
  if (curSeek+8>len) throw EndOfFileException(this,len);
  int64_t ret=*(int64_t*)(&buf[curSeek]);
  curSeek+=8;
  return ret;
}

float SafeReader::readF() {
  if (curSeek+4>len) throw EndOfFileException(this,len);
  float ret=*(float*)(&buf[curSeek]);
  curSeek+=4;
  return ret;
}

double SafeReader::readD() {
  if (curSeek+8>len) throw EndOfFileException(this,len);
  double ret=*(double*)(&buf[curSeek]);
  curSeek+=8;
  return ret;
}

String SafeReader::readString(size_t stlen) {
  String ret;
#ifdef READ_DEBUG
  logD("SR: reading string len %d at %x",stlen,curSeek);
#endif
  size_t curPos=0;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && curPos<stlen) {
    unsigned char c=readC();
    if (c!=0) ret.push_back(c);
    curPos++;
  }
  return ret;
}

String SafeReader::readString() {
  String ret;
  unsigned char c;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && (c=readC())!=0) {
    ret.push_back(c);
  }
  return ret;
}

String SafeReader::readStringLine() {
  String ret;
  unsigned char c;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && (c = readC()) != 0) {
    if (c=='\r'||c=='\n') {
      break;
    }
    ret.push_back(c);
  }
  return ret;
}

String SafeReader::readStringToken(unsigned char delim) {
  String ret;
  unsigned char c;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && (c=readC())!=0) {
    if (c == '\r' || c == '\n') {
      break;
    }
    if (c == delim) {
      if (ret.length() == 0) {
        continue;
      }
      break;
    }
    ret.push_back(c);
  }
  return ret;
}

String SafeReader::readStringToken() {
  return readStringToken(' ');
}
