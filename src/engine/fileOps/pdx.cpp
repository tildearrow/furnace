/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "fileOpsCommon.h"

class DivEngine;

//PDX 8-bit OKI ADPCM sample bank

/* File format
The file starts with a header with 96 8-byte pairs, with each pair containing offset and length of ADPCM data chunks for each note value, like so:

[[byte pointer to sample in file -- 32-bit unsigned integer (4 bytes)] [empty: $0000 -- 16-bit integer] [length of sample, amount in bytes -- 16-bit unsigned integer] ×96] [ADPCM data] EOF

The first sample (1) is mapped to 0x80 (= the lowest note within MXDRV) and the last sample (96) is mapped to 0xDF (= the highest note within MXDRV).


Samples are encoded in 4-bit OKI ADPCM encoded nibbles, where each byte contains 2 nibbles: [nibble 2 << 4 || nibble 1]


Unfortunately, sample rates for each samples are not defined within the .PDX file and have to be set manually with the appropriate command for that at play-time */

#define PDX_BANK_SIZE 96
#define PDX_SAMPLE_RATE 16000

typedef struct
{
    unsigned int start_pointer;
    unsigned short sample_length;
} PDX_HEADER;

#define UNUSED(x) (void)(x)

void DivEngine::loadPDX(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    PDX_HEADER headers[PDX_BANK_SIZE];

    for(int i = 0; i < PDX_BANK_SIZE; i++)
    {
        headers[i].start_pointer = (unsigned int)reader.readI_BE();
        unsigned short empty = (unsigned short)reader.readS_BE(); //skip 1st 2 bytes
        UNUSED(empty);
        headers[i].sample_length = (unsigned short)reader.readS_BE();
    }

    for(int i = 0; i < PDX_BANK_SIZE; i++)
    {
        if(headers[i].start_pointer != 0 && headers[i].sample_length != 0)
        {
            DivSample* s = new DivSample;

            s->rate = PDX_SAMPLE_RATE;
            s->centerRate = PDX_SAMPLE_RATE;
            s->depth = DIV_SAMPLE_DEPTH_VOX;
            s->init(headers[i].sample_length * 2);

            reader.seek((int)headers[i].start_pointer, SEEK_SET);

            int sample_pos = 0;

            for(unsigned short j = 0; j < headers[i].sample_length; j++)
            {
                unsigned char curr_byte = (unsigned char)reader.readC();
                curr_byte = (curr_byte << 4) | (curr_byte >> 4);

                s->dataVOX[sample_pos] = curr_byte;
                sample_pos++;
            }

            ret.push_back(s);

            logI("pdx: start %d len %d", headers[i].start_pointer, headers[i].sample_length);
        }
    }
  } 
  catch (EndOfFileException& e) 
  {
    lastError=_("premature end of file");
    logE("premature end of file");
  }
}