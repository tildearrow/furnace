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

long brrEncode(short* buf, unsigned char* out, long len, long loopStart) {
  if (len==0) return 0;

  // encoding process:
  // 1. read next group of 16 samples
  // 2. is this the first block?
  //    - if yes, don't filter. output and then go to 1
  // 3. is this the loop block?
  //    - if yes, don't filter. output and then go to 1
  // 4. try encoding using 4 filters
  // 5. which one of these yields the least amount of error?
  // 6. output the one which does
  // 7. is this the last block?
  //    - if yes, mark end and finish
  // 8. go to 1
  long total=0;
  unsigned char next[9];
  unsigned char filter=0;
  unsigned char range=0;
  unsigned char o=0;
  short o0=0;

  len&=~15;
  loopStart&=~15;
  for (long i=0; i<len; i+=16) {
    // don't filter on the first or loop block
    if (i && i!=loopStart) {

    } else {
      filter=0;
    }

    range=0;
    for (int j=0; j<16; j++) {
      short s=buf[j];
      if (s<0) s=-s;
      while (range<11 && s>((8<<range)-1)) range++;
    }
    next[0]=(range<<4)|(filter<<2)|((i+16>len)?1:0);
    switch (filter) {
      case 0:
        for (int j=0; j<16; j++) {
          o0=buf[j]>>range;
          if (o0>7) o0=7;
          if (o0<-8) o0=-8;
          o=o0&15;
          if (j&1) {
            next[1+(j>>1)]|=o;
          } else {
            next[1+(j>>1)]=o<<4;
          }
        }
        break;
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
    }

    out[0]=next[0];
    out[1]=next[1];
    out[2]=next[2];
    out[3]=next[3];
    out[4]=next[4];
    out[5]=next[5];
    out[6]=next[6];
    out[7]=next[7];
    out[8]=next[8];
    buf+=16;
    out+=9;
    total+=9;
  }
  return total;
}

#define DO_ONE_SAMPLE \
  if (next&8) next|=0xfffffff0; \
\
  next<<=(buf[0]>>4); /* range */ \
  next>>=1; \
\
  switch (control&0xc) { /* filter */ \
    case 0: \
      break; \
    case 4: \
      next+=last1+((-last1)>>4); \
      break; \
    case 8: \
      next+=last1*2+((-last1*3)>>5)-last2+(last2>>4); \
      break; \
    case 12: \
      next+=last1*2+((-last1*13)>>6)-last2+((last2*3)>>4); \
      break; \
  } \
\
  if (next>32767) next=32767; \
  if (next<-32768) next=-32768; \
  next&=0x7fff; \
  if (next&0x4000) next|=0xffff8000; \
\
  last2=last1; \
  last1=next; \
  *out=next<<1; \
  out++;

// TODO:
// - what happens during overflow?
// - what happens when range values 12 to 15 are used?
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
