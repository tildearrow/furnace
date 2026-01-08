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

#include "utfutils.h"

int decodeUTF8(const unsigned char* data, signed char& len) {
  int ret=0xfffd;
  if (data[0]<0x80) {
    ret=data[0];
    len=1;
  } else if (data[0]<0xc0) {
    ret=0xfffd; // invalid
    len=1;
  } else if (data[0]<0xe0) {
    if (data[1]>=0x80 && data[1]<0xc0) {
      len=2;
      ret=((data[0]&31)<<6)|
          (data[1]&63);
    } else len=1;
  } else if (data[0]<0xf0) {
    if (data[1]>=0x80 && data[1]<0xc0) {
      if (data[2]>=0x80 && data[2]<0xc0) {
        len=3;
        ret=((data[0]&15)<<12)|
            ((data[1]&63)<<6)|
            (data[2]&63);
      } else len=2;
    } else len=1;
  } else if (data[0]<0xf5) {
    if (data[1]>=0x80 && data[1]<0xc0) {
      if (data[2]>=0x80 && data[2]<0xc0) {
        if (data[3]>=0x80 && data[3]<0xc0) {
          len=4;
          ret=((data[0]&7)<<18)|
              ((data[1]&63)<<12)|
              ((data[2]&63)<<6)|
              (data[3]&63);
        } else len=3;
      } else len=2;
    } else len=1;
  } else {
    len=1;
    return 0xfffd;
  }

  if ((ret>=0xd800 && ret<=0xdfff) || ret>=0x110000) return 0xfffd;
  return ret;
}

size_t utf8len(const char* s) {
  size_t p=0;
  size_t r=0;
  signed char cl;
  while (s[p]!=0) {
    r++;
    decodeUTF8((const unsigned char*)&s[p],cl);
    p+=cl;
  }
  return r;
}

char utf8csize(const unsigned char* c) {
  signed char ret;
  decodeUTF8(c,ret);
  return ret;
}

WString utf8To16(const char* s) {
  WString ret;
  int ch, p;
  signed char chs;
  p=0;
  while (s[p]!=0) {
    ch=decodeUTF8((const unsigned char*)&s[p],chs);
    // surrogates
    if (ch>=0x10000) {
      ch-=0x10000;
      ret+=(unsigned short)(0xd800|((ch>>10)&0x3ff));
      ret+=(unsigned short)(0xdc00|(ch&0x3ff));
    } else if (ch<0xd800 || ch>0xdfff) {
      ret+=(unsigned short)ch;
    }
    p+=chs;
  }
  return ret;
}

String utf16To8(const wchar_t* s) {
  String ret;
  unsigned int next=0;
  for (size_t i=0; i<wcslen(s); i++) {
    if (s[i]>=0xd800 && s[i]<0xdc00) {
      next=0x10000+((s[i]&0x3ff)<<10);
      continue;
    } else if (s[i]>=0xdc00 && s[i]<0xe000) {
      next|=s[i]&0x3ff;
    } else {
      next=s[i];
    }
    if (next<0x80) {
      ret+=next;
    } else if (next<0x800) {
      ret+=(0xc0+((next>>6)&31));
      ret+=(0x80+((next)&63));
    } else if (next<0x10000) {
      ret+=(0xe0+((next>>12)&15));
      ret+=(0x80+((next>>6)&63));
      ret+=(0x80+((next)&63));
    } else {
      ret+=(0xf0+((next>>18)&7));
      ret+=(0x80+((next>>12)&63));
      ret+=(0x80+((next>>6)&63));
      ret+=(0x80+((next)&63));
    }
  }
  return ret;
}

WString utf8To16(String& in) {
  return utf8To16(in.c_str());
}

String utf16To8(WString& in) {
  return utf16To8(in.c_str());
}
