/*
 * Schism Tracker - a cross-platform Impulse Tracker clone
 * copyright (c) 2003-2005 Storlek <storlek@rigelseven.com>
 * copyright (c) 2005-2008 Mrs. Brisby <mrs.brisby@nimh.org>
 * copyright (c) 2009 Storlek & Mrs. Brisby
 * copyright (c) 2010-2012 Storlek
 * URL: http://schismtracker.org/
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// slightly modified for Furnace by tildearrow

#include "compression.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define CLAMP(x,xMin,xMax) (MIN(MAX((x),(xMin)),(xMax)))

// ------------------------------------------------------------------------------------------------------------
// IT decompression code from itsex.c (Cubic Player) and load_it.cpp (Modplug)
// (I suppose this could be considered a merge between the two.)

static uint32_t it_readbits(int8_t n, uint32_t *bitbuf, uint32_t *bitnum, const uint8_t **ibuf)
{
	uint32_t value = 0;
	uint32_t i = n;

	// this could be better
	while (i--) {
		if (!*bitnum) {
			*bitbuf = *(*ibuf)++;
			*bitnum = 8;
		}
		value >>= 1;
		value |= (*bitbuf) << 31;
		(*bitbuf) >>= 1;
		(*bitnum)--;
	}
	return value >> (32 - n);
}


uint32_t it_decompress8(void *dest, uint32_t len, const void *file, uint32_t filelen, int it215, int channels)
{
	const uint8_t *filebuf;         // source buffer containing compressed sample data
	const uint8_t *srcbuf;          // current position in source buffer
	int8_t *destpos;                // position in destination buffer which will be returned
	uint16_t blklen;                // length of compressed data block in samples
	uint16_t blkpos;                // position in block
	uint8_t width;                  // actual "bit width"
	uint16_t value;                 // value read from file to be processed
	int8_t d1, d2;                  // integrator buffers (d2 for it2.15)
	int8_t v;                       // sample value
	uint32_t bitbuf, bitnum;        // state for it_readbits

	filebuf = srcbuf = (const uint8_t *) file;
	destpos = (int8_t *) dest;

	// now unpack data till the dest buffer is full
	while (len) {
		// read a new block of compressed data and reset variables
		// block layout: word size, <size> bytes data
		if (srcbuf + 2 > filebuf + filelen
		    || srcbuf + 2 + (srcbuf[0] | (srcbuf[1] << 8)) > filebuf + filelen) {
			// truncated!
			return srcbuf - filebuf;
		}
		srcbuf += 2;
		bitbuf = bitnum = 0;

		blklen = MIN(0x8000, len);
		blkpos = 0;

		width = 9; // start with width of 9 bits
		d1 = d2 = 0; // reset integrator buffers

		// now uncompress the data block
		while (blkpos < blklen) {
			if (width > 9) {
				// illegal width, abort
				printf("Illegal bit width %d for 8-bit sample\n", width);
				return srcbuf - filebuf;
			}
			value = it_readbits(width, &bitbuf, &bitnum, &srcbuf);

			if (width < 7) {
				// method 1 (1-6 bits)
				// check for "100..."
				if (value == 1 << (width - 1)) {
					// yes!
					value = it_readbits(3, &bitbuf, &bitnum, &srcbuf) + 1; // read new width
					width = (value < width) ? value : value + 1; // and expand it
					continue; // ... next value
				}
			} else if (width < 9) {
				// method 2 (7-8 bits)
				uint8_t border = (0xFF >> (9 - width)) - 4; // lower border for width chg
				if (value > border && value <= (border + 8)) {
					value -= border; // convert width to 1-8
					width = (value < width) ? value : value + 1; // and expand it
					continue; // ... next value
				}
			} else {
				// method 3 (9 bits)
				// bit 8 set?
				if (value & 0x100) {
					width = (value + 1) & 0xff; // new width...
					continue; // ... and next value
				}
			}

			// now expand value to signed byte
			if (width < 8) {
				uint8_t shift = 8 - width;
				v = (value << shift);
				v >>= shift;
			} else {
				v = (int8_t) value;
			}

			// integrate upon the sample values
			d1 += v;
			d2 += d1;

			// .. and store it into the buffer
			*destpos = it215 ? d2 : d1;
			destpos += channels;
			blkpos++;
		}

		// now subtract block length from total length and go on
		len -= blklen;
	}
	return srcbuf - filebuf;
}

// Mostly the same as above.
uint32_t it_decompress16(void *dest, uint32_t len, const void *file, uint32_t filelen, int it215, int channels)
{
	const uint8_t *filebuf;         // source buffer containing compressed sample data
	const uint8_t *srcbuf;          // current position in source buffer
	int16_t *destpos;               // position in destination buffer which will be returned
	uint16_t blklen;                // length of compressed data block in samples
	uint16_t blkpos;                // position in block
	uint8_t width;                  // actual "bit width"
	uint32_t value;                 // value read from file to be processed
	int16_t d1, d2;                 // integrator buffers (d2 for it2.15)
	int16_t v;                      // sample value
	uint32_t bitbuf, bitnum;        // state for it_readbits

	filebuf = srcbuf = (const uint8_t *) file;
	destpos = (int16_t *) dest;

	// now unpack data till the dest buffer is full
	while (len) {
		// read a new block of compressed data and reset variables
		// block layout: word size, <size> bytes data
		if (srcbuf + 2 > filebuf + filelen
		    || srcbuf + 2 + (srcbuf[0] | (srcbuf[1] << 8)) > filebuf + filelen) {
			// truncated!
			return srcbuf - filebuf;
		}
		srcbuf += 2;

		bitbuf = bitnum = 0;

		blklen = MIN(0x4000, len); // 0x4000 samples => 0x8000 bytes again
		blkpos = 0;

		width = 17; // start with width of 17 bits
		d1 = d2 = 0; // reset integrator buffers

		// now uncompress the data block
		while (blkpos < blklen) {
			if (width > 17) {
				// illegal width, abort
				printf("Illegal bit width %d for 16-bit sample\n", width);
				return srcbuf - filebuf;
			}
			value = it_readbits(width, &bitbuf, &bitnum, &srcbuf);

			if (width < 7) {
				// method 1 (1-6 bits)
				// check for "100..."
				if (value == (uint32_t) 1 << (width - 1)) {
					// yes!
					value = it_readbits(4, &bitbuf, &bitnum, &srcbuf) + 1; // read new width
					width = (value < width) ? value : value + 1; // and expand it
					continue; // ... next value
				}
			} else if (width < 17) {
				// method 2 (7-16 bits)
				uint16_t border = (0xFFFF >> (17 - width)) - 8; // lower border for width chg
				if (value > border && value <= (uint32_t) (border + 16)) {
					value -= border; // convert width to 1-8
					width = (value < width) ? value : value + 1; // and expand it
					continue; // ... next value
				}
			} else {
				// method 3 (17 bits)
				// bit 16 set?
				if (value & 0x10000) {
					width = (value + 1) & 0xff; // new width...
					continue; // ... and next value
				}
			}

			// now expand value to signed word
			if (width < 16) {
				uint8_t shift = 16 - width;
				v = (value << shift);
				v >>= shift;
			} else {
				v = (int16_t) value;
			}

			// integrate upon the sample values
			d1 += v;
			d2 += d1;

			// .. and store it into the buffer
			*destpos = it215 ? d2 : d1;
			destpos += channels;
			blkpos++;
		}

		// now subtract block length from total length and go on
		len -= blklen;
	}
	return srcbuf - filebuf;
}
