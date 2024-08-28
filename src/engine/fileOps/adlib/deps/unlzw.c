/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * unlzw.c - Custom LZW decompression routine from Adlib Tracker II
 *           Adapted by Dmitry Smagin <dmitry.s.smagin@gmail.com>
 *           Originally by Stanislav Baranec <subz3ro.altair@gmail.com>
 *
 * REFERENCES:
 * https://github.com/ijsf/at2
 * http://www.adlibtracker.net/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "unlzw.h"

static uint16_t bitshift;
static uint32_t prevbitstring;

static uint16_t bitmask[5] = { 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff };

static unsigned char *input_ptr;
static int input_size;
static unsigned char *output_ptr;
static unsigned int output_maxsize;
static int output_size;

static int nextcode()
{
    int input_idx;
    uint32_t bitstring;

    input_idx = prevbitstring >> 3;
    bitstring = input_ptr[input_idx + 0] +
               (input_ptr[input_idx + 1] << 8) +
               (input_ptr[input_idx + 2] << 16);

    bitstring >>= prevbitstring & 7;
    prevbitstring += bitshift;

    return bitstring & bitmask[bitshift - 9];
}

static void LZW_decode()
{
    uint8_t *stack = calloc(1, 65636);
    uint8_t *work_ptr = calloc(1, 65636);

    uint8_t le76, le77;
    uint16_t le6a, le6c, le6e, le70, stringlength, le74;

    int code;
    int output_idx;

    int sp = 65536 - 1;

    stringlength = 0;
    bitshift = 9;
    le70 = 0x102;
    le74 = 0x200;
    output_idx = 0;
    code = 0;
    le6a = 0;
    le6c = 0;
    le6e = 0;
    le76 = 0;
    le77 = 0;
    prevbitstring = 0;

    for (;;) {
        code = nextcode();
        if (code == 0x101)
            break;

        if (code == 0x100) {
            bitshift = 9;
            le74 = 0x200;
            le70 = 0x102;
            code = nextcode();
            le6a = code;
            le6c = code;
            le77 = code;
            le76 = code;

	    if (output_idx >= output_maxsize) goto error_out;
            output_ptr[output_idx++] = code;
            continue;
        }

        le6a = code;
        le6e = code;

        if (code >= le70) {
            code = le6c;
            le6a = code;
            code = (code & 0xff00) + le76;
            sp--;
            stack[sp] = code;
            stringlength++;
        }

        while (le6a > 0xff) {
            code = (code & 0xff00) + work_ptr[(le6a * 3) + 2];
            sp--;
            stack[sp] = code;
            stringlength++;
            code = work_ptr[le6a * 3] + (work_ptr[(le6a * 3) + 1] << 8);
            le6a = code;
        }

        code = le6a;
        le76 = code;
        le77 = code;
        sp--;
        stack[sp] = code;
        stringlength++;

        while (stringlength--) {
	    if (output_idx >= output_maxsize) goto error_out;
            output_ptr[output_idx++] = stack[sp++];
        }

        stringlength = 0;

        work_ptr[(le70 * 3) + 0] = le6c;
        work_ptr[(le70 * 3) + 1] = le6c >> 8;
        work_ptr[(le70 * 3) + 2] = le77;
        le70++;

        code = le6e;
        le6c = code;

        if (le70 >= le74 && bitshift < 14) {
            bitshift++;
            le74 <<= 1;
        }
    }

error_out:
    output_size = output_idx;
    free(stack);
    free(work_ptr);
}

int LZW_decompress(char *source, char *dest, int source_size, int destination_size)
{
    input_ptr = (unsigned char *)source;
    input_size = source_size;
    output_ptr = (unsigned char *)dest;
    output_maxsize = destination_size;

    LZW_decode();

    return output_size;
}
