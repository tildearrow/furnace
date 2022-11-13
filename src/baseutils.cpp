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

#include "baseutils.h"
#include <string.h>

const char* base64Table="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string taEncodeBase64(const std::string& data) {
  std::string ret;

  ret.reserve((2+data.size()*4)/3);

  unsigned int groupOfThree=0;
  unsigned char pos=0;
  for (const char& i: data) {
    groupOfThree|=((unsigned char)i)<<((2-pos)<<3);
    if (++pos>=3) {
      pos=0;
      ret+=base64Table[(groupOfThree>>18)&63];
      ret+=base64Table[(groupOfThree>>12)&63];
      ret+=base64Table[(groupOfThree>>6)&63];
      ret+=base64Table[groupOfThree&63];
      groupOfThree=0;
    }
  }
  if (pos==2) {
    ret+=base64Table[(groupOfThree>>18)&63];
    ret+=base64Table[(groupOfThree>>12)&63];
    ret+=base64Table[(groupOfThree>>6)&63];
    ret+='=';
  } else if (pos==1) {
    ret+=base64Table[(groupOfThree>>18)&63];
    ret+=base64Table[(groupOfThree>>12)&63];
    ret+="==";
  }
  
  return ret;
}

std::string taDecodeBase64(const char* buf) {
  std::string data;

  unsigned int groupOfThree=0;
  signed char pos=18;
  for (const char* i=buf; *i; i++) {
    unsigned char nextVal=0;
    if ((*i)=='/') {
      nextVal=63;
    } else if ((*i)=='+') {
      nextVal=62;
    } else if ((*i)>='0' && (*i)<='9') {
      nextVal=52+((*i)-'0');
    } else if ((*i)>='a' && (*i)<='z') {
      nextVal=26+((*i)-'a');
    } else if ((*i)>='A' && (*i)<='Z') {
      nextVal=((*i)-'A');
    } else {
      nextVal=0;
    }
    groupOfThree|=nextVal<<pos;
    pos-=6;
    if (pos<0) {
      pos=18;
      if ((groupOfThree>>16)&0xff) data+=(groupOfThree>>16)&0xff;
      if ((groupOfThree>>8)&0xff) data+=(groupOfThree>>8)&0xff;
      if (groupOfThree&0xff) data+=groupOfThree&0xff;
      groupOfThree=0;
    }
  }

  return data;
}
