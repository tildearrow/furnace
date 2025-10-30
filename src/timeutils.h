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

#ifndef _TIMEUTILS_H
#define _TIMEUTILS_H

#ifdef __unix__
#include <time.h>
#include <sys/time.h>
#endif

#include "ta-utils.h"

struct TimeMicros {
  int seconds, micros;

  inline float toFloat() {
    return seconds+(float)micros/1000000.0f;
  }
  inline double toDouble() {
    return seconds+(double)micros/1000000.0;
  }

  /**
   * convert this TimeMicros to a String.
   * @param prec maximum digit precision (0-6). use -1 for automatic precision.
   * @param hms how many denominations to include (0: seconds, 1: minutes, 2: hours).
   * @return formatted TimeMicros.
   */
  String toString(signed char prec=-1, unsigned char hms=0);
  static TimeMicros fromString(String s);

  TimeMicros(int s, int u):
    seconds(s), micros(u) {}
#ifdef __unix__
  TimeMicros(struct timeval tv):
    seconds(tv.tv_sec), micros(tv.tv_usec) {}
  TimeMicros(struct timespec ts):
    seconds(ts.tv_sec), micros(ts.tv_nsec/1000) {}
#endif
  TimeMicros():
    seconds(0), micros(0) {}
};

#endif
