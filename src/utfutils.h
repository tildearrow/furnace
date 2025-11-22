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

#ifndef _UTFUTILS_H
#define _UTFUTILS_H
#include "ta-utils.h"

int decodeUTF8(const unsigned char* data, signed char& len);

size_t utf8len(const char* s);
size_t utf8clen(const char* s);
size_t utf8pos(const char* s, size_t inpos);
size_t utf8cpos(const char* s, size_t inpos);
size_t utf8findcpos(const char* s, float inpos);
char utf8csize(const unsigned char* c);

WString utf8To16(const char* in);
String utf16To8(const wchar_t* in);

WString utf8To16(String& in);
String utf16To8(WString& in);

#endif
