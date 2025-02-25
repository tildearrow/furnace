/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "stringutils.h"

String lowerCaseCopy(const char* str) {
  String lowerCase=str;
  for (char& i: lowerCase) {
    if (i>='A' && i<='Z') i+='a'-'A';
  }
  return lowerCase;
}

void removeFileExt(String& str, const char* ext) {
  String lowerCase=lowerCaseCopy(str.c_str());

  String extLowerCase;
  if (ext[0]=='.') {
    extLowerCase=lowerCaseCopy(ext);
  } else {
    extLowerCase=".";
    extLowerCase+=lowerCaseCopy(ext);
  }

  size_t extPos=lowerCase.rfind(extLowerCase);
  if (extPos!=String::npos) {
    str=str.substr(0,extPos);
  }
}

void splitString(const String& str, char sep, std::vector<String>& out) {
  size_t start=0, end=0;
  while (end!=String::npos) {
    end=str.find(sep,start);
    String s=str.substr(start,end-start);
    if (s.size()>0) out.push_back(s);
    start=end+1;
  }
}
