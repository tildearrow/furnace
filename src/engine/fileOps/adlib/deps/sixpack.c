/********************************************/
/*  SIXPACK.C -- Data compression program   */
/*  Written by Philip G. Gage, April 1991   */
/********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#include "sixpack.h"

#define MAXFREQ 2000
#define MINCOPY 3
#define MAXCOPY 255
#define COPYRANGES 6
#define CODESPERRANGE (MAXCOPY - MINCOPY + 1)
#define TERMINATE 256
#define FIRSTCODE 257
#define MAXCHAR (FIRSTCODE + COPYRANGES * CODESPERRANGE - 1)
#define SUCCMAX (MAXCHAR + 1)
#define TWICEMAX (2 * MAXCHAR + 1)
#define ROOT 1
#define MAXBUF (42 * 1024)
#define MAXDISTANCE 21389
#define MAXSIZE (21389 + MAXCOPY)

static unsigned short bitvalue[14] =
  {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};

static signed short copybits[COPYRANGES] =
  {4, 6, 8, 10, 12, 14};

static signed short copymin[COPYRANGES] =
  {0, 16, 80, 336, 1360, 5456};

void inittree();
void updatefreq(unsigned short a,unsigned short b);
void updatemodel(unsigned short code);
unsigned short inputcode(unsigned short bits);
unsigned short uncompress();
void decode();

static unsigned short ibitcount, ibitbuffer, ibufcount, obufcount, input_size,
	output_size, leftc[MAXCHAR+1], rghtc[MAXCHAR+1],
	dad[TWICEMAX+1], freq[TWICEMAX+1], *wdbuf;
static unsigned int obufsize;
static unsigned char *obuf, *buf;

void inittree()
{
	unsigned short i;

	for(i=2;i<=TWICEMAX;i++) {
		dad[i] = i / 2;
		freq[i] = 1;
	}

	for(i=1;i<=MAXCHAR;i++) {
		leftc[i] = 2 * i;
		rghtc[i] = 2 * i + 1;
	}
}

void updatefreq(unsigned short a,unsigned short b)
{
	do {
		freq[dad[a]] = freq[a] + freq[b];
		a = dad[a];
		if(a != ROOT) {
			if(leftc[dad[a]] == a)
				b = rghtc[dad[a]];
			else
				b = leftc[dad[a]];
		}
	} while(a != ROOT);

	if(freq[ROOT] == MAXFREQ)
		for(a=1;a<=TWICEMAX;a++)
			freq[a] >>= 1;
}

void updatemodel(unsigned short code)
{
	unsigned short a=code+SUCCMAX,b,c,code1,code2;

	freq[a]++;
	if(dad[a] != ROOT) {
		code1 = dad[a];
		if(leftc[code1] == a)
			updatefreq(a,rghtc[code1]);
		else
			updatefreq(a,leftc[code1]);

		do {
			code2 = dad[code1];
			if(leftc[code2] == code1)
				b = rghtc[code2];
			else
				b = leftc[code2];

			if(freq[a] > freq[b]) {
				if(leftc[code2] == code1)
					rghtc[code2] = a;
				else
					leftc[code2] = a;

				if(leftc[code1] == a) {
					leftc[code1] = b;
					c = rghtc[code1];
				} else {
					rghtc[code1] = b;
					c = leftc[code1];
				}

				dad[b] = code1;
				dad[a] = code2;
				updatefreq(b,c);
				a = b;
			}

			a = dad[a];
			code1 = dad[a];
		} while(code1 != ROOT);
	}
}

unsigned short inputcode(unsigned short bits)
{
	unsigned short i,code=0;

	for(i=1;i<=bits;i++) {
		if(!ibitcount) {
			if(ibitcount == MAXBUF)
				ibufcount = 0;
			if (ibufcount >= input_size)
				return 0; /* overflow input buffer */
			ibitbuffer = wdbuf[ibufcount];
			ibufcount++;
			ibitcount = 15;
		} else
			ibitcount--;

		if(ibitbuffer > 0x7fff)
			code |= bitvalue[i-1];
		ibitbuffer <<= 1;
	}

	return code;
}

unsigned short uncompress()
{
	unsigned short a=1;

	do {
		if(!ibitcount) {
			if(ibufcount == MAXBUF)
				ibufcount = 0;
			if (ibufcount >= input_size)
				return TERMINATE; /* overflow input buffer */
			ibitbuffer = wdbuf[ibufcount];
			ibufcount++;
			ibitcount = 15;
		} else
			ibitcount--;

		if(ibitbuffer > 0x7fff)
			a = rghtc[a];
		else
			a = leftc[a];
		ibitbuffer <<= 1;
	} while(a <= MAXCHAR);

	a -= SUCCMAX;
	updatemodel(a);
	return a;
}

void decode()
{
	unsigned short i,j,k,t,c,count=0,dist,len,index;

	inittree();
	c = uncompress();

	while(c != TERMINATE) {
		if(c < 256) {
			if (obufcount < obufsize) /* check for overflow in output buffer */
				obuf[obufcount] = (unsigned char)c;
			obufcount++;
			if(obufcount == MAXBUF) {
				output_size = MAXBUF;
				obufcount = 0;
			}

			buf[count] = (unsigned char)c;
			count++;
			if(count == MAXSIZE)
				count = 0;
		} else {
			t = c - FIRSTCODE;
			index = t / CODESPERRANGE;
			len = t + MINCOPY - index * CODESPERRANGE;
			dist = inputcode(copybits[index]) + len + copymin[index];

			j = count;
			k = count - dist;
			if(count < dist)
				k += MAXSIZE;

			for(i=0;i<=len-1;i++) {
				if (obufcount < obufsize) /* check for overflow in output buffer */
					obuf[obufcount] = buf[k];
				obufcount++;
				if(obufcount == MAXBUF) {
					output_size = MAXBUF;
					obufcount = 0;
				}

				buf[j] = buf[k];
				j++; k++;
				if(j == MAXSIZE) j = 0;
				if(k == MAXSIZE) k = 0;
			}

			count += len;
			if(count >= MAXSIZE)
				count -= MAXSIZE;
		}
		c = uncompress();
	}
	output_size = obufcount;
}

unsigned short sixdepak(unsigned short *source, unsigned char *dest,
				    unsigned short size, unsigned int destsize)
{
	if((unsigned int)size + 4096 > MAXBUF)
		return 0;

	buf = calloc(1, MAXSIZE); //buf = new unsigned char [MAXSIZE];
	input_size = size;
	ibitcount = 0; ibitbuffer = 0;
	obufcount = 0; ibufcount = 0;
	obufsize = destsize;
	wdbuf = source; obuf = dest;

	decode();
	free(buf); //delete [] buf;
	return output_size;
}
