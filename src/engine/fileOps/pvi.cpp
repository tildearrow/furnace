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

#include "fileOpsCommon.h"

class DivEngine;

//PVI YM2608 ADPCM-B sample bank

/* =======================================
                 General
=======================================

ADPCM RAM addresses: see docs/common.txt {

  address_start = 0x0000
  file_header_size = 0x0210

}

=======================================
                 Header
=======================================

0x0000           Identifier (4b)
                 "PVI2"
0x0004 - 0x0007  Unknown Settings (4b)
                 Unknown mappings PCME switches <-> Values
                 "0x10 0x00 0x10 0x02" in all example files
                 First 1h may be Start Address in ADPCM RAM?
0x0008 - 0x0009  "Delta-N" playback frequency (1h)
                 Default 0x49BA "== 16kHz"??
0x000A           Unknown (1b)
                 RAM type? (Docs indicate values 0 or 8, all example files have value 0x02?)
0x000B           Amount of defined Samples (1b)
0x000C - 0x000F  Unknown (4b)
                 Padding?
                 "0x00 0x00 0x00 0x00" in all example files
0x0010 - 0x020F  128 * {

  Start of Sample (32b blocks) in ADPCM RAM  (1h)
  End of Sample (32b blocks) in ADPCM RAM    (1h)

  (0x0000 0x0000 -> no sample for this instrument ID)

}


=======================================
                  Body
=======================================

Stream of Sample Data {

  Yamaha ADPCM-B encoding (4-Bit Signed ADPCM)
  Mono
  Sample rate as specified earlier
  (examples i have seems Stereo or 32kHz despite "16kHz" playback frequency setting?)

} */

#define PVIV2_FILE_SIG "PVI2"
#define PVIV1_FILE_SIG "PVI1"

#define PVI_BANK_SIZE 128
#define PVI_SAMPLE_RATE 16000

typedef struct
{
    uint16_t start_pointer;
    uint16_t end_pointer;
} PVI_HEADER;

#define UNUSED(x) (void)(x)

#define ADPCM_DATA_START 0x0210

void DivEngine::loadPVI(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    String file_sig = reader.readString(4);
    if(file_sig != PVIV1_FILE_SIG && file_sig != PVIV2_FILE_SIG) return;

    unsigned int unknown_settings = (unsigned int)reader.readI();
    UNUSED(unknown_settings);
    unsigned short delta_n = (unsigned short)reader.readS();
    UNUSED(delta_n);
    unsigned char one_byte = (unsigned char)reader.readC();
    UNUSED(one_byte);
    unsigned char amount_of_samples = (unsigned char)reader.readC();
    UNUSED(amount_of_samples);
    unsigned int padding = (unsigned int)reader.readI();
    UNUSED(padding);

    PVI_HEADER headers[PVI_BANK_SIZE];

    for(int i = 0; i < PVI_BANK_SIZE; i++)
    {
        headers[i].start_pointer = (unsigned short)reader.readS();
        headers[i].end_pointer = (unsigned short)reader.readS();
    }

    for(int i = 0; i < PVI_BANK_SIZE; i++)
    {
        if((headers[i].start_pointer != 0 || headers[i].end_pointer != 0) && headers[i].start_pointer < headers[i].end_pointer)
        {
            DivSample* s = new DivSample;

            s->centerRate = PVI_SAMPLE_RATE;
            s->depth = DIV_SAMPLE_DEPTH_ADPCM_B;
            s->init((headers[i].end_pointer - headers[i].start_pointer) * 32 * 2);

            int sample_pos = 0;
            int sample_length = (headers[i].end_pointer - headers[i].start_pointer) * 32;

            reader.seek(ADPCM_DATA_START + headers[i].start_pointer * 32, SEEK_SET);

            for(int j = 0; j < sample_length; j++)
            {
                unsigned char curr_byte = (unsigned char)reader.readC();
                //curr_byte=(curr_byte<<4)|(curr_byte>>4);

                s->dataB[sample_pos] = curr_byte;
                sample_pos++;
            }

            logI("pvi: start %d end %d len in bytes %d", headers[i].start_pointer, headers[i].end_pointer, (headers[i].end_pointer - headers[i].start_pointer) * 32);

            ret.push_back(s);
        }
    }
  } 
  catch (EndOfFileException& e) 
  {
    lastError=_("premature end of file");
    logE("premature end of file");
  }
}
