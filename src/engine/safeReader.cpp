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
  if (curSeek+count<curSeek) throw EndOfFileException(this,len);
  memcpy(where,&buf[curSeek],count);
  curSeek+=count;
  return count;
}

signed char SafeReader::readC() {
#ifdef READ_DEBUG
  logD("SR: reading char %x:",curSeek);
#endif
  if (curSeek+1>len) throw EndOfFileException(this,len);
  if (curSeek+1<curSeek) throw EndOfFileException(this,len);
#ifdef READ_DEBUG
  logD("SR: %.2x",buf[curSeek]);
#endif
  return (signed char)buf[curSeek++];
}

#ifdef TA_BIG_ENDIAN
short SafeReader::readS_BE() {
#ifdef READ_DEBUG
  logD("SR: reading short %x:",curSeek);
#endif
  if (curSeek+2>len) throw EndOfFileException(this,len);
  if (curSeek+2<curSeek) throw EndOfFileException(this,len);
  short ret;
  memcpy(&ret,&buf[curSeek],2);
#ifdef READ_DEBUG
  logD("SR: %.4x",ret);
#endif
  curSeek+=2;
  return ret;
}

short SafeReader::readS() {
  if (curSeek+2>len) throw EndOfFileException(this,len);
  if (curSeek+2<curSeek) throw EndOfFileException(this,len);
  short ret;
  memcpy(&ret,&buf[curSeek],2);
  curSeek+=2;
  return ((ret>>8)&0xff)|(ret<<8);
}

int SafeReader::readI_BE() {
#ifdef READ_DEBUG
  logD("SR: reading int %x:",curSeek);
#endif
  if (curSeek+4>len) throw EndOfFileException(this,len);
  if (curSeek+4<curSeek) throw EndOfFileException(this,len);
  int ret;
  memcpy(&ret,&buf[curSeek],4);
  curSeek+=4;
#ifdef READ_DEBUG
  logD("SR: %.8x",ret);
#endif
  return ret;
}

int SafeReader::readI() {
  if (curSeek+4>len) throw EndOfFileException(this,len);
  if (curSeek+4<curSeek) throw EndOfFileException(this,len);
  unsigned int ret;
  memcpy(&ret,&buf[curSeek],4);
  curSeek+=4;
  return (int)((ret>>24)|((ret&0xff0000)>>8)|((ret&0xff00)<<8)|((ret&0xff)<<24));
}

int64_t SafeReader::readL() {
  if (curSeek+8>len) throw EndOfFileException(this,len);
  if (curSeek+8<curSeek) throw EndOfFileException(this,len);
  unsigned char ret[8];
  memcpy(ret,&buf[curSeek],8);
  curSeek+=8;
  return (int64_t)(ret[0]|(ret[1]<<8)|(ret[2]<<16)|(ret[3]<<24)|((uint64_t)ret[4]<<32)|((uint64_t)ret[5]<<40)|((uint64_t)ret[6]<<48)|((uint64_t)ret[7]<<56));
}

float SafeReader::readF() {
  if (curSeek+4>len) throw EndOfFileException(this,len);
  if (curSeek+4<curSeek) throw EndOfFileException(this,len);
  unsigned int ret;
  memcpy(&ret,&buf[curSeek],4);
  curSeek+=4;
  ret=((ret>>24)|((ret&0xff0000)>>8)|((ret&0xff00)<<8)|((ret&0xff)<<24));
  float realRet;
  memcpy(&realRet,&ret,4);
  return realRet;
}

double SafeReader::readD() {
  if (curSeek+8>len) throw EndOfFileException(this,len);
  if (curSeek+8<curSeek) throw EndOfFileException(this,len);
  unsigned char ret[8];
  unsigned char retB[8];
  memcpy(ret,&buf[curSeek],8);
  curSeek+=8;
  retB[0]=ret[7];
  retB[1]=ret[6];
  retB[2]=ret[5];
  retB[3]=ret[4];
  retB[4]=ret[3];
  retB[5]=ret[2];
  retB[6]=ret[1];
  retB[7]=ret[0];
  double realRet;
  memcpy(&realRet,retB,8);
  return realRet;
}
#else
short SafeReader::readS() {
#ifdef READ_DEBUG
  logD("SR: reading short %x:",curSeek);
#endif
  if (curSeek+2>len) throw EndOfFileException(this,len);
  if (curSeek+2<curSeek) throw EndOfFileException(this,len);
  short ret;
  memcpy(&ret,&buf[curSeek],2);
#ifdef READ_DEBUG
  logD("SR: %.4x",ret);
#endif
  curSeek+=2;
  return ret;
}

short SafeReader::readS_BE() {
  if (curSeek+2>len) throw EndOfFileException(this,len);
  if (curSeek+2<curSeek) throw EndOfFileException(this,len);
  short ret;
  memcpy(&ret,&buf[curSeek],2);
  curSeek+=2;
  return ((ret>>8)&0xff)|(ret<<8);
}

int SafeReader::readI() {
#ifdef READ_DEBUG
  logD("SR: reading int %x:",curSeek);
#endif
  if (curSeek+4>len) throw EndOfFileException(this,len);
  if (curSeek+4<curSeek) throw EndOfFileException(this,len);
  int ret;
  memcpy(&ret,&buf[curSeek],4);
  curSeek+=4;
#ifdef READ_DEBUG
  logD("SR: %.8x",ret);
#endif
  return ret;
}

int SafeReader::readI_BE() {
  if (curSeek+4>len) throw EndOfFileException(this,len);
  if (curSeek+4<curSeek) throw EndOfFileException(this,len);
  unsigned int ret;
  memcpy(&ret,&buf[curSeek],4);
  curSeek+=4;
  return (int)((ret>>24)|((ret&0xff0000)>>8)|((ret&0xff00)<<8)|((ret&0xff)<<24));
}

int64_t SafeReader::readL() {
  if (curSeek+8>len) throw EndOfFileException(this,len);
  if (curSeek+8<curSeek) throw EndOfFileException(this,len);
  int64_t ret;
  memcpy(&ret,&buf[curSeek],8);
  curSeek+=8;
  return ret;
}

float SafeReader::readF() {
  if (curSeek+4>len) throw EndOfFileException(this,len);
  if (curSeek+4<curSeek) throw EndOfFileException(this,len);
  float ret;
  memcpy(&ret,&buf[curSeek],4);
  curSeek+=4;
  return ret;
}

double SafeReader::readD() {
  if (curSeek+8>len) throw EndOfFileException(this,len);
  if (curSeek+8<curSeek) throw EndOfFileException(this,len);
  double ret;
  memcpy(&ret,&buf[curSeek],8);
  curSeek+=8;
  return ret;
}
#endif

String SafeReader::readStringWithEncoding(DivStringEncoding encoding, size_t stlen) {
  String ret;
#ifdef READ_DEBUG
  logD("SR: reading string len %d at %x",stlen,curSeek);
#endif
  size_t curPos=0;
  if (isEOF()) throw EndOfFileException(this, len);
  bool zero=false;

  while (!isEOF() && curPos<stlen) {
    unsigned char c=readC();
    if (c==0) {
      zero=true;
    }
    if (!zero) {
      if (encoding==DIV_ENCODING_LATIN1) {
        if (c>=0x20) {
          if (c&0x80) {
            if (c>=0xa0) {
              ret.push_back(0xc0|(c>>6));
              ret.push_back(0x80|(c&63));
            }
          } else {
            ret.push_back(c);
          }
        }
      } else if (encoding==DIV_ENCODING_LATIN1_SPECIAL) {
        if (c&0x80) {
          if (c>=0xa0) {
            ret.push_back(0xc0|(c>>6));
            ret.push_back(0x80|(c&63));
          }
        } else {
          ret.push_back(c);
        }
      } else {
        ret.push_back(c);
      }
    }
    curPos++;
  }
  return ret;
}

String SafeReader::readStringWithEncoding(DivStringEncoding encoding) {
  String ret;
  unsigned char c;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && (c=readC())!=0) {
    if (encoding==DIV_ENCODING_LATIN1) {
      if (c>=0x20) {
        if (c&0x80) {
          if (c>=0xa0) {
            ret.push_back(0xc0|(c>>6));
            ret.push_back(0x80|(c&63));
          }
        } else {
          ret.push_back(c);
        }
      }
    } else if (encoding==DIV_ENCODING_LATIN1_SPECIAL) {
      if (c&0x80) {
        if (c>=0xa0) {
          ret.push_back(0xc0|(c>>6));
          ret.push_back(0x80|(c&63));
        }
      } else {
        ret.push_back(c);
      }
    } else {
      ret.push_back(c);
    }
  }
  return ret;
}

String SafeReader::readString() {
  return readStringWithEncoding(DIV_ENCODING_NONE);
}

String SafeReader::readString(size_t stlen) {
  return readStringWithEncoding(DIV_ENCODING_NONE,stlen);
}

String SafeReader::readStringLatin1() {
  return readStringWithEncoding(DIV_ENCODING_LATIN1);
}

String SafeReader::readStringLatin1(size_t stlen) {
  return readStringWithEncoding(DIV_ENCODING_LATIN1,stlen);
}

String SafeReader::readStringLatin1Special() {
  return readStringWithEncoding(DIV_ENCODING_LATIN1_SPECIAL);
}

String SafeReader::readStringLatin1Special(size_t stlen) {
  return readStringWithEncoding(DIV_ENCODING_LATIN1_SPECIAL,stlen);
}

String SafeReader::readStringLine() {
  String ret;
  unsigned char c;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && (c=readC())!=0) {
    if (c=='\r' || c=='\n') {
      break;
    }
    ret.push_back(c);
  }
  return ret;
}

String SafeReader::readStringToken(unsigned char delim, bool stripContiguous) {
  String ret;
  unsigned char c;
  if (isEOF()) throw EndOfFileException(this, len);

  while (!isEOF() && (c=readC())!=0) {
    if (c=='\r' || c=='\n') {
      break;
    }
    if (c==delim) {
      if (ret.length()==0 && stripContiguous) {
        continue;
      }
      break;
    }
    ret.push_back(c);
  }
  return ret;
}

String SafeReader::readStringToken() {
  // This will strip LHS whitespace and only return contents after it.
  return readStringToken(' ', true);
}
