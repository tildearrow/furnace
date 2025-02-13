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

#ifndef _FILEUTILS_H
#define _FILEUTILS_H
#include <stdio.h>

FILE* ps_fopen(const char* path, const char* mode);
bool moveFiles(const char* src, const char* dest);
bool deleteFile(const char* path);
// returns 1 if file exists, 0 if it doesn't and -1 on error.
int fileExists(const char* path);
bool dirExists(const char* what);
bool makeDir(const char* path);
int touchFile(const char* path);

#endif
