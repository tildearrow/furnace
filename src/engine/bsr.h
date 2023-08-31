/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#if defined( _MSC_VER )

#include <intrin.h>

static inline int bsr(unsigned short v) {
  unsigned long idx;
  if (_BitScanReverse(&idx,(unsigned long)v)) {
    return idx;
  }
  else {
    return -1;
  }
}

#elif defined( __GNUC__ )

static inline int bsr(unsigned short v)
{
  if (v) {
    return 32 - __builtin_clz(v);
  }
  else{
    return -1;
  }
}

#else

static inline int bsr(unsigned short v)
{
  unsigned short mask = 0x8000;
  for (int i = 15; i >= 0; --i) {
    if (v&mask)
      return (int)i;
    mask>>=1;
  }

  return -1;
}

#endif

