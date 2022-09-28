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

#define NEXT_SAMPLE buf[j]-(buf[j]>>3)

#define DO_ONE_DEC(r) \
  if (nextDec&8) nextDec|=0xfffffff0; \
\
  if (r>=13) { /* invalid shift */ \
    nextDec=(nextDec<0)?0xfffff800:0; \
  } else { \
    nextDec<<=r; /* range */ \
    nextDec>>=1; \
  } \
\
  switch (filter) { /* filter */ \
    case 0: \
      break; \
    case 1: \
      nextDec+=last1+((-last1)>>4); \
      break; \
    case 2: \
      nextDec+=last1*2+((-last1*3)>>5)-last2+(last2>>4); \
      break; \
    case 3: \
      nextDec+=last1*2+((-last1*13)>>6)-last2+((last2*3)>>4); \
      break; \
  } \
\
  if (nextDec>32767) nextDec=32767; \
  if (nextDec<-32768) nextDec=-32768; \
  nextDec&=0x7fff; \
  if (nextDec&0x4000) nextDec|=0xffff8000; \
\
  last2=last1; \
  last1=nextDec; \

long brrEncode(short* buf, unsigned char* out, long len, long loopStart) {
  if (len==0) return 0;

  // encoding process:
  // 1. read next group of 16 samples
  // 2. is this the first block?
  //   - if yes, don't filter. output and then go to 1
  // 3. is this the loop block?
  //   - if yes, don't filter. output and then go to 1
  // 4. try encoding using 4 filters
  //   - perform linear prediction
  //   - calculate range
  //   - decode and apply correction to achieve low error
  // 5. which one of these yields the least amount of error?
  // 6. output the one which does
  // 7. is this the last block?
  //   - if yes, mark end and finish
  // 8. go to 1
  long total=0;
  unsigned char next0[8];
  unsigned char next1[8];
  unsigned char next2[8];
  unsigned char next3[8];
  unsigned char filter=0;
  unsigned char range0=0;
  unsigned char range1=0;
  unsigned char range2=0;
  unsigned char range3=0;
  unsigned char o=0;
  int pred1[16];
  int pred2[16];
  int pred3[16];
  short o1=0;
  short o2=0;
  short o0=0;
  short o1f1=0;
  short o1f2=0;
  short o1f3=0;
  //short o2f1=0;
  short o2f2=0;
  short o2f3=0;

  int last1=0;
  int last2=0;
  int nextDec=0;
  int maxError[4];
  int avgError[4];

  len&=~15;
  loopStart&=~15;
  for (long i=0; i<len; i+=16) {
    range0=0;
    // encode with no filter
    for (int j=0; j<16; j++) {
      short s=NEXT_SAMPLE;
      if (s<0) s=-s;
      while (range0<12 && s>((8<<range0)-1)) range0++;
    }
    for (int j=0; j<16; j++) {
      short s=NEXT_SAMPLE;
      o0=s>>range0;
      if (range0) if (s&(1<<(range1>>1))) o0++;
      if (o0>7) o0=7;
      if (o0<-8) o0=-8;
      if (range0>=12) if (o0<-7) o0=-7;
      o=o0&15;
      if (j&1) {
        next0[j>>1]|=o;
      } else {
        next0[j>>1]=o<<4;
      }
    }

    // encode with filter
    if (i && i!=loopStart) {
      // 1: x = o0 - o1 * 15/16
      // 2: x = o0 + o2 * 15/16 - o1 * 61/32
      // 3: x = o0 + o2 * 13/16 - o1 * 115/64
      range1=0;
      range2=0;
      range3=0;
      //o2f1=o2;
      o2f2=o2;
      o2f3=o2;
      o1f1=o1;
      o1f2=o1;
      o1f3=o1;
      // first pass
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;

        pred1[j]=s-(((int)o1*15)>>4);
        if (pred1[j]<-32768) pred1[j]=-32768;
        if (pred1[j]>32767) pred1[j]=32767;

        pred2[j]=s+(((int)o2*15)>>4)-(((int)o1*61)>>5);
        if (pred2[j]<-32768) pred2[j]=-32768;
        if (pred2[j]>32767) pred2[j]=32767;

        pred3[j]=s+(((int)o2*13)>>4)-(((int)o1*115)>>6);
        if (pred3[j]<-32768) pred3[j]=-32768;
        if (pred3[j]>32767) pred3[j]=32767;
        
        o2=o1;
        o1=s;
      }
      // calculate range of values
      for (int j=0; j<16; j++) {
        short s=pred1[j];
        if (s<0) s=-s;
        while (range1<12 && s>((8<<range1)-1)) range1++;

        s=pred2[j];
        if (s<0) s=-s;
        while (range2<12 && s>((8<<range2)-1)) range2++;

        s=pred3[j];
        if (s<0) s=-s;
        while (range3<12 && s>((8<<range3)-1)) range3++;
      }
      // second pass
      int prevLast1=last1;
      int prevLast2=last2;
      filter=1;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;

        pred1[j]=s-(((int)o1f1*15)>>4);
        if (pred1[j]<-32768) pred1[j]=-32768;
        if (pred1[j]>32767) pred1[j]=32767;

        o0=pred1[j]>>range1;
        if (range1) if (pred1[j]&(1<<(range1>>1))) o0++;
        if (o0>7) o0=7;
        if (o0<-8) o0=-8;
        o=o0&15;
        if (j&1) {
          next1[j>>1]|=o;
        } else {
          next1[j>>1]=o<<4;
        }

        nextDec=o;
        DO_ONE_DEC(range1);
        //o2f1=last2<<1;
        o1f1=last1<<1;
      }
      last1=prevLast1;
      last2=prevLast2;
      filter=2;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;
        pred2[j]=s+(((int)o2f2*15)>>4)-(((int)o1f2*61)>>5);
        if (pred2[j]<-32768) pred2[j]=-32768;
        if (pred2[j]>32767) pred2[j]=32767;

        o0=pred2[j]>>range2;
        if (range2) if (pred2[j]&(1<<(range2>>1))) o0++;
        if (o0>7) o0=7;
        if (o0<-8) o0=-8;
        o=o0&15;
        if (j&1) {
          next2[j>>1]|=o;
        } else {
          next2[j>>1]=o<<4;
        }

        nextDec=o;
        DO_ONE_DEC(range2);
        o2f2=last2<<1;
        o1f2=last1<<1;
      }
      last1=prevLast1;
      last2=prevLast2;
      filter=3;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;
        pred3[j]=s+(((int)o2f3*13)>>4)-(((int)o1f3*115)>>6);
        if (pred3[j]<-32768) pred3[j]=-32768;
        if (pred3[j]>32767) pred3[j]=32767;

        o0=pred3[j]>>range3;
        if (range3) if (pred3[j]&(1<<(range3>>1))) o0++;
        if (o0>7) o0=7;
        if (o0<-8) o0=-8;
        o=o0&15;
        if (j&1) {
          next3[j>>1]|=o;
        } else {
          next3[j>>1]=o<<4;
        }

        nextDec=o;
        DO_ONE_DEC(range3);
        o2f3=last2<<1;
        o1f3=last1<<1;
      }
      last1=prevLast1;
      last2=prevLast2;

      // find best filter
      int error=0;

      maxError[0]=0;
      maxError[1]=0;
      maxError[2]=0;
      maxError[3]=0;
      avgError[0]=0;
      avgError[1]=0;
      avgError[2]=0;
      avgError[3]=0;

      // test filter 0
      filter=0;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;
        if (j&1) {
          nextDec=next0[j>>1]&15;
        } else {
          nextDec=next0[j>>1]>>4;
        }
        DO_ONE_DEC(range0);
        error=s-(nextDec<<1);
        if (error<0) error=-error;
        avgError[0]+=error;
        if (error>maxError[0]) maxError[0]=error;
      }
      last1=prevLast1;
      last2=prevLast2;

      // test filter 1
      filter=1;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;
        if (j&1) {
          nextDec=next1[j>>1]&15;
        } else {
          nextDec=next1[j>>1]>>4;
        }
        DO_ONE_DEC(range1);
        error=s-(nextDec<<1);
        if (error<0) error=-error;
        avgError[1]+=error;
        if (error>maxError[1]) maxError[1]=error;
      }
      last1=prevLast1;
      last2=prevLast2;

      // test filter 2
      filter=2;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;
        if (j&1) {
          nextDec=next2[j>>1]&15;
        } else {
          nextDec=next2[j>>1]>>4;
        }
        DO_ONE_DEC(range2);
        error=s-(nextDec<<1);
        if (error<0) error=-error;
        avgError[2]+=error;
        if (error>maxError[2]) maxError[2]=error;
      }
      last1=prevLast1;
      last2=prevLast2;

      // test filter 3
      filter=3;
      for (int j=0; j<16; j++) {
        int s=NEXT_SAMPLE;
        if (j&1) {
          nextDec=next3[j>>1]&15;
        } else {
          nextDec=next3[j>>1]>>4;
        }
        DO_ONE_DEC(range3);
        error=s-(nextDec<<1);
        if (error<0) error=-error;
        avgError[3]+=error;
        if (error>maxError[3]) maxError[3]=error;
      }
      last1=prevLast1;
      last2=prevLast2;

      // pick best filter
      int candError=0x7fffffff;
      for (int j=0; j<4; j++) {
        if (avgError[j]<candError) {
          candError=avgError[j];
          filter=j;
        }
      }
      //printf("block %ld: %8d %8d %8d %8d -> %d\n",i>>4,avgError[0],avgError[1],avgError[2],avgError[3],filter);
    } else {
      // don't filter on the first or loop block
      filter=0;
    }

    switch (filter) {
      case 0:
        for (int j=0; j<8; j++) {
          nextDec=next0[j]>>4;
          DO_ONE_DEC(range0);
          nextDec=next0[j]&15;
          DO_ONE_DEC(range0);
        }
        o2=last2<<1;
        o1=last1<<1;

        out[0]=(range0<<4)|(filter<<2)|((i+16>=len)?((loopStart>=0)?3:1):0);
        out[1]=next0[0];
        out[2]=next0[1];
        out[3]=next0[2];
        out[4]=next0[3];
        out[5]=next0[4];
        out[6]=next0[5];
        out[7]=next0[6];
        out[8]=next0[7];
        break;
      case 1:
        for (int j=0; j<8; j++) {
          nextDec=next1[j]>>4;
          DO_ONE_DEC(range1);
          nextDec=next1[j]&15;
          DO_ONE_DEC(range1);
        }
        o2=last2<<1;
        o1=last1<<1;
        out[0]=(range1<<4)|(filter<<2)|((i+16>=len)?((loopStart>=0)?3:1):0);
        out[1]=next1[0];
        out[2]=next1[1];
        out[3]=next1[2];
        out[4]=next1[3];
        out[5]=next1[4];
        out[6]=next1[5];
        out[7]=next1[6];
        out[8]=next1[7];
        break;
      case 2:
        for (int j=0; j<8; j++) {
          nextDec=next2[j]>>4;
          DO_ONE_DEC(range2);
          nextDec=next2[j]&15;
          DO_ONE_DEC(range2);
        }
        o2=last2<<1;
        o1=last1<<1;
        out[0]=(range2<<4)|(filter<<2)|((i+16>=len)?((loopStart>=0)?3:1):0);
        out[1]=next2[0];
        out[2]=next2[1];
        out[3]=next2[2];
        out[4]=next2[3];
        out[5]=next2[4];
        out[6]=next2[5];
        out[7]=next2[6];
        out[8]=next2[7];
        break;
      case 3:
        for (int j=0; j<8; j++) {
          nextDec=next3[j]>>4;
          DO_ONE_DEC(range3);
          nextDec=next3[j]&15;
          DO_ONE_DEC(range3);
        }
        o2=last2<<1;
        o1=last1<<1;
        out[0]=(range3<<4)|(filter<<2)|((i+16>=len)?((loopStart>=0)?3:1):0);
        out[1]=next3[0];
        out[2]=next3[1];
        out[3]=next3[2];
        out[4]=next3[3];
        out[5]=next3[4];
        out[6]=next3[5];
        out[7]=next3[6];
        out[8]=next3[7];
        break;
    }
    buf+=16;
    out+=9;
    total+=9;
  }
  return total;
}

#define DO_ONE_SAMPLE \
  if (next&8) next|=0xfffffff0; \
\
  if (buf[0]>=0xd0) { /* invalid shift */ \
    next=(next<0)?0xfffff800:0; \
  } else { \
    next<<=(buf[0]>>4); /* range */ \
    next>>=1; \
  } \
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
