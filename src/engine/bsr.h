/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
    return idx+1;
  }
  else {
    return -1;
  }
}

static inline int bsr32(unsigned int v) {
  unsigned long idx;
  if (_BitScanReverse(&idx,(unsigned long)v)) {
    return idx+1;
  } else {
    return -1;
  }
}

static inline int bsf32(unsigned int v) {
  unsigned long idx;
  if (_BitScanForward(&idx,(unsigned long)v)) {
    return idx;
  } else {
    return -1;
  }
}

#elif defined( __GNUC__ )

static inline int bsr(unsigned short v) {
  if (v) {
    return 32-__builtin_clz(v);
  } else {
    return -1;
  }
}

static inline int bsr32(unsigned int v) {
  if (v) {
    return 32-__builtin_clz(v);
  } else {
    return -1;
  }
}

static inline int bsf32(unsigned int v) {
  if (v) {
    return __builtin_ctz(v);
  } else {
    return -1;
  }
}

#else

static inline int bsr(unsigned short v) {
  if (v==0) return -1;
  if (v&0x8000) return 16;
  int o=16;
  if (!(v&0xff00)) {
    o-=8;
    v<<=8;
    if (v&0x8000) return o;
  }
  if (!(v&0xf000)) {
    o-=4;
    v<<=4;
    if (v&0x8000) return o;
  }
  if (!(v&0xc000)) {
    o-=2;
    v<<=2;
    if (v&0x8000) return o;
  }
  return (v&0x8000) ? o : o-1;
}

static inline int bsr32(unsigned int v) {
  if (v==0) return -1;
  if (v&0x80000000) return 32;
  int o=32;
  if (!(v&0xffff0000)) {
    o-=16;
    v<<=16;
    if (v&0x80000000) return o;
  }
  if (!(v&0xff000000)) {
    o-=8;
    v<<=8;
    if (v&0x80000000) return o;
  }
  if (!(v&0xf0000000)) {
    o-=4;
    v<<=4;
    if (v&0x80000000) return o;
  }
  if (!(v&0xc0000000)) {
    o-=2;
    v<<=2;
    if (v&0x80000000) return o;
  }
  return (v&0x80000000) ? o : o-1;
}

static inline int bsf32(unsigned int v) {
  int ret=0;
  if (v==0) return -1;
  while (!(v&1)) {
    v>>=1;
    ret++;
  }
  return ret;
}

#endif

static inline unsigned int gcd2(unsigned int val0, unsigned int val1) {
  // check whether one of the operands is zero
  if (val0==0) return val1;
  if (val1==0) return val0;

  // check how many times can we divide by 2
  int shift=bsf32(val0|val1);

  // shift the first operand
  val0>>=bsf32(val0);

  while (val1!=0) {
    // shift the second operand
    val1>>=bsf32(val1);

    // swap operands if the first is greater
    if (val0>val1) {
      val0^=val1;
      val1^=val0;
      val0^=val1;
    }
    val1=val1-val0;
  }

  return val0<<shift;
}