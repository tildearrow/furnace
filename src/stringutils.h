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

#ifndef _STRINGUTILS_H
#define _STRINGUTILS_H
#include "ta-utils.h"

// returns a lowercase copy of an ASCII string
String lowerCaseCopy(const char* str);

// removes an extension off the end of a string
// the leading dot in `ext` is optional (e.g., both `.wav` and `wav` are accepted)
void removeFileExt(String& str, const char* ext);

// split `str`, using `sep` as a separator, and pushing the parts into the back of the `out` vector
void splitString(const String& str, char sep, std::vector<String>& out);

#endif
