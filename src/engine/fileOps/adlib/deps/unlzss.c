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
 * unlzss.c - Custom LZSS decompression routine from Adlib Tracker II
 *            Adapted by Dmitry Smagin <dmitry.s.smagin@gmail.com>
 *            Originally by Stanislav Baranec <subz3ro.altair@gmail.com>
 *
 * REFERENCES:
 * https://github.com/ijsf/at2
 * http://www.adlibtracker.net/
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unlzss.h"

static int input_size, output_size;
static unsigned char *input_ptr, *output_ptr;
static int output_maxsize;

#define N_BITS      12
#define F_BITS      4
#define THRESHOLD   2
#define N           (1 << N_BITS)
#define F           ((1 << F_BITS) + THRESHOLD)

static void LZSS_decode() {
    int input_idx = 0;
    uint8_t code, prevcode;
    uint16_t dx;
    uint32_t ebx, edi;

    unsigned char *work_ptr = calloc(1, 65536);

    output_size = 0;
    ebx = 0;
    dx = 0;
    edi = N - F;

    for (;;) {
        dx = dx >> 1;

        if (!(dx >> 8)) {
            if (input_idx >= input_size)
                break;

            code = input_ptr[input_idx++];
            dx = 0xff00 | code;
        }

        if (dx & 1) {
            if (input_idx >= input_size)
                break;

            code = input_ptr[input_idx++];
            work_ptr[edi] = code;
            edi = (edi + 1) & (N - 1);
	    if (output_size >= output_maxsize) goto error_out;
            output_ptr[output_size++] = code;
            continue;
        }

        if (input_idx >= input_size)
            break;

        prevcode = code = input_ptr[input_idx++];

        if (input_idx >= input_size)
            break;

        code = input_ptr[input_idx++];
        ebx = ((code << 4) & 0xff00) | prevcode;

        int length = (code & 0x0f) + THRESHOLD + 1;

        do {
	    if (output_size >= output_maxsize) goto error_out;
            output_ptr[output_size++] = work_ptr[edi] = work_ptr[ebx];

            ebx = (ebx + 1) & (N - 1);
            edi = (edi + 1) & (N - 1);
        } while (--length > 0);
    }

error_out:
    free(work_ptr);
}

int LZSS_decompress(char *source, char *dest, int source_size, int dest_size)
{
    input_ptr = (unsigned char *)source;
    input_size = source_size;
    output_ptr = (unsigned char *)dest;
    output_maxsize = dest_size;
    
    LZSS_decode();

    return output_size;
}
