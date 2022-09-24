/* brrUtils - BRR audio codec utilities
 * Copyright (C) 2022 tildearrow 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "brrUtils.h"

long brrEncode(short* buf, unsigned char* out, long len) {
  if (len==0) return 0;
  // TODO
  return 0;
}

#define DO_ONE_SAMPLE \
  if (next&8) next|=0xfffffff8; \
\
  next<<=(buf[0]>>4); /* range */ \
\
  switch (control&0xc) { /* filter */ \
    case 0: \
      break; \
    case 4: \
      next+=(last1*15)/16; \
      break; \
    case 8: \
      next+=((last1*61)/32)-((last2*15)/16); \
      break; \
    case 12: \
      next+=((last1*115)/64)-((last2*13)/16); \
      break; \
  } \
\
  if (next>32767) next=32767; \
  if (next<-32768) next=-32768; \
\
  last2=last1; \
  last1=next; \
  *out=next; \
  out++;

long brrDecode(unsigned char* buf, short* out, long len) {
  if (len==0) return 0;

  long total=0;

  int last1=0;
  int last2=0;
  int next=0;

  // don't read out of bounds
  len-=8;

  for (long i=0; i<len; i+=9) {
    unsigned char control=buf[0];

    for (unsigned char j=1; j<9; j++) {
      next=buf[j]>>4;
      DO_ONE_SAMPLE;

      next=buf[j]&15;
      DO_ONE_SAMPLE;
    }

    // end bit
    total+=16;
    if (control&1) break;
    buf+=9;
  }

  return total;
}
