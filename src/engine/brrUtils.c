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
#include <stdio.h>
#include <string.h>

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

void brrEncodeBlock(const short* buf, unsigned char* out, unsigned char range, unsigned char filter, short* last1, short* last2, int* errorSum) {
  // encode one block using BRR
  unsigned char nibble=0;
  int preOut=0;
  int pred=0;
  int nextDec=0;
  int nextError=0;
  *errorSum=0;
  for (int j=0; j<16; j++) {
    short s=NEXT_SAMPLE;
    switch (filter) {
      case 0: // no filter
        pred=s;
        break;
      case 1: // simple
        pred=s-(((int)(*last1*2)*15)>>4);
        break;
      case 2: // complex
        pred=s+(((int)(*last2*2)*15)>>4)-(((int)(*last1*2)*61)>>5);
        break;
      case 3:
        pred=s+(((int)(*last2*2)*13)>>4)-(((int)(*last1*2)*115)>>6);
        break;
    }

    if (pred<-32768) pred=-32768;
    if (pred>32767) pred=32767;

    preOut=pred>>range;
    if (range) {
      if (pred&(1<<(range>>1))) preOut++;
      if (filter==0 && range>=12) if (preOut<-7) preOut=-7;
    }
    if (preOut>7) preOut=7;
    if (preOut<-8) preOut=-8;

    nibble=preOut&15;
    if (j&1) {
      out[j>>1]|=nibble;
    } else {
      out[j>>1]=nibble<<4;
    }

    // roll last1/last2
    nextDec=nibble;
    if (nextDec&8) nextDec|=0xfffffff0;

    if (range>=13) { /* invalid shift */
      nextDec=(nextDec<0)?0xfffff800:0;
    } else {
      nextDec<<=range; /* range */
      nextDec>>=1;
    }

    switch (filter) { /* filter */
      case 0:
        break;
      case 1:
        nextDec+=(*last1)+((-(*last1))>>4);
        break;
      case 2:
        nextDec+=(*last1)*2+((-(*last1)*3)>>5)-(*last2)+((*last2)>>4);
        break;
      case 3:
        nextDec+=(*last1)*2+((-(*last1)*13)>>6)-(*last2)+(((*last2)*3)>>4);
        break;
    }

    nextDec&=0x7fff;
    if (nextDec&0x4000) nextDec|=0xffff8000;

    nextError=s-(nextDec<<1);
    if (nextError<0) nextError=-nextError;
    *errorSum+=nextError;

    *last2=*last1;
    *last1=nextDec;
  }
}

long brrEncode(short* buf, unsigned char* out, long len, long loopStart, unsigned char emphasis) {
  if (len==0) return 0;

  // encoding process:
  // 1. read next group of 16 samples
  // 2. is this the first block?
  //   - if yes, don't filter. output and then go to 1
  // 4. try encoding using 3 filters and 12 ranges (besides no filter)
  // 5. which one of these yields the least amount of error?
  // 6. output the one which does
  // 7. do we still have more to encode?
  //   - if so, go to 1
  // 8. is loop point set?
  //   - if not, end process here
  // 9. is transition between last block and loop block smooth?
  //   - if not, encode the loop block again and output it
  long total=0;
  unsigned char filter=0;
  unsigned char range=0;

  short x0=0;
  short x1=0;
  short x2=0;
  int emphOut=0;

  short in[17];

  short last1[4][13];
  short last2[4][13];
  int avgError[4][13];
  unsigned char possibleOut[4][13][8];

  memset(in,0,16*sizeof(short));
  memset(last1,0,4*13*sizeof(short));
  memset(last2,0,4*13*sizeof(short));
  memset(avgError,0,4*13*sizeof(int));
  memset(possibleOut,0,4*13*8);

  for (long i=0; i<len; i+=16) {
    if (i+17>len) {
      long p=i;
      for (int j=0; j<17; j++) {
        if (p>=len) {
          if (loopStart<0 || loopStart>=len) {
            in[j]=0;
          } else {
            p=loopStart;
            in[j]=buf[p++];
          }
        } else {
          in[j]=buf[p++];
        }
      }
    } else {
      memcpy(in,&buf[i],17*sizeof(short));
    }
    
    // emphasis
    if (emphasis) {
      for (int j=0; j<17; j++) {
        x0=x1;
        x1=x2;
        x2=in[j];

        if (j==0) continue;
        emphOut=((x1<<11)-x0*370-in[j]*374)/1305;
        if (emphOut<-32768) emphOut=-32768;
        if (emphOut>32767) emphOut=32767;
        in[j-1]=emphOut;
      }
    }

    // encode
    for (int j=0; j<4; j++) {
      for (int k=0; k<13; k++) {
        brrEncodeBlock(in,possibleOut[j][k],k,j,&last1[j][k],&last2[j][k],&avgError[j][k]);
      }
    }

    // find best filter/range
    int candError=0x7fffffff;
    if (i==0) {
      filter=0;
      for (int k=0; k<13; k++) {
        if (avgError[0][k]<candError) {
          candError=avgError[0][k];
          range=k;
        }
      }
    } else {
      for (int j=0; j<4; j++) {
        for (int k=0; k<13; k++) {
          if (avgError[j][k]<candError) {
            candError=avgError[j][k];
            filter=j;
            range=k;
          }
        }
      }
    }

    // write
    out[0]=(range<<4)|(filter<<2)|((i+16>=len && loopStart<0)?1:0);
    for (int j=0; j<8; j++) {
      out[j+1]=possibleOut[filter][range][j];
    }

    for (int j=0; j<4; j++) {
      for (int k=0; k<13; k++) {
        last1[j][k]=last1[filter][range];
        last2[j][k]=last2[filter][range];
      }
    }
    out+=9;
    total+=9;
  }
  // encode loop block
  if (loopStart>=0) {
    long p=loopStart;
    for (int i=0; i<17; i++) {
      if (p>=len) {
        p=loopStart;
      }
      in[i]=buf[p++];
    }

    if (emphasis) {
      for (int j=0; j<17; j++) {
        x0=x1;
        x1=x2;
        x2=in[j];

        if (j==0) continue;
        emphOut=((x1<<11)-x0*370-in[j]*374)/1305;
        if (emphOut<-32768) emphOut=-32768;
        if (emphOut>32767) emphOut=32767;
        in[j-1]=emphOut;
      }
    }

    // encode (filter 0/1 only)
    for (int j=0; j<2; j++) {
      for (int k=0; k<13; k++) {
        brrEncodeBlock(in,possibleOut[j][k],k,j,&last1[j][k],&last2[j][k],&avgError[j][k]);
      }
    }

    // find best filter/range
    int candError=0x7fffffff;
    for (int j=0; j<2; j++) {
      for (int k=0; k<13; k++) {
        if (avgError[j][k]<candError) {
          candError=avgError[j][k];
          filter=j;
          range=k;
        }
      }
    }

    // write
    out[0]=(range<<4)|(filter<<2)|3;
    for (int j=0; j<8; j++) {
      out[j+1]=possibleOut[filter][range][j];
    }

    for (int j=0; j<4; j++) {
      for (int k=0; k<13; k++) {
        last1[j][k]=last1[filter][range];
        last2[j][k]=last2[filter][range];
      }
    }
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

long brrDecode(unsigned char* buf, short* out, long len, unsigned char emphasis) {
  if (len==0) return 0;

  short* outOrig=out;

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

  if (emphasis) {
    short x0=0;
    short x1=0;
    short x2=0;
    for (long i=0; i<=total; i++) {
      x0=x1;
      x1=x2;
      x2=(i>=total)?0:outOrig[i];

      if (i==0) continue;

      outOrig[i-1]=(x0*370+x1*1305+x2*374)>>11;
    }
  }

  return total;
}
