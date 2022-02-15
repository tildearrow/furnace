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

#include "ta-log.h"

int logLevel=LOGLEVEL_INFO;

int logD(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_DEBUG) return 0;
#ifdef _WIN32
  printf("[debug] ");
#else
  printf("\x1b[1;34m[debug]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  fflush(stdout);
  return ret;
}

int logI(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_INFO) return 0;
#ifdef _WIN32
  printf("[info] ");
#else
  printf("\x1b[1;32m[info]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

int logW(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_WARN) return 0;
#ifdef _WIN32
  printf("[warning] ");
#else
  printf("\x1b[1;33m[warning]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

int logE(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_ERROR) return 0;
#ifdef _WIN32
  printf("[ERROR] ");
#else
  printf("\x1b[1;31m[ERROR]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

