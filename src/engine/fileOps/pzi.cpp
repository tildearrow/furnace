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

#include "shared.h"

#ifdef HAVE_GUI
#include "../gui/gui.h"
extern FurnaceGUI g;
#endif

#define _LE(string) (string)

class DivEngine;

//PZI 8-bit PCM sample bank

/* =======================================
                 Header
=======================================

0x0000           Identifier (4b)
                 "PZI1"
0x0004 - 0x001F  Unknown (28b)
                 Part of identifier? Settings?
                 All (\0)s in all example files
0x0020 - 0x091F  128 * {

  Start of Sample after header (2h)
  Length of Sample (2h)
  Offset of loop start from sample start (2h)
  Offset of loop end from sample start (2h)
  Sample rate (1h)

  (0xFFFFFFFF 0xFFFFFFFF loop offsets -> no loop information)

}


=======================================
                  Body
=======================================

Stream of Sample Data {

  Unsigned 8-Bit
  Mono
  Sample rate as specified in header

} */

#define PZI_BANK_SIZE 128

#define PZI_FILE_SIG "PZI1"

#define NO_LOOP (0xFFFFFFFFU)

#define SAMPLE_DATA_OFFSET 0x0920

#define MAX_SANITY_CAP 9999999

#define HEADER_JUNK_SIZE 28

typedef struct
{
    uint32_t start_pointer;
    uint32_t sample_length;
    uint32_t loop_start;
    uint32_t loop_end;
    uint16_t sample_rate;
} PZI_HEADER;

#define UNUSED(x) (void)(x)

void DivEngine::loadPZI(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    PZI_HEADER headers[PZI_BANK_SIZE];

    String file_sig = reader.readString(4);
    if(file_sig != PZI_FILE_SIG) return;

    for (int i = 0; i < HEADER_JUNK_SIZE; i++)
    {
        unsigned char curr_byte = (unsigned char)reader.readC();
        UNUSED(curr_byte);
    }

    for(int i = 0; i < PZI_BANK_SIZE; i++)
    {
        headers[i].start_pointer = (unsigned int)reader.readI();
        headers[i].sample_length = (unsigned int)reader.readI();
        headers[i].loop_start = (unsigned int)reader.readI();
        headers[i].loop_end = (unsigned int)reader.readI();
        headers[i].sample_rate = (unsigned short)reader.readS();
    }

    for(int i = 0; i < PZI_BANK_SIZE; i++)
    {
        if (headers[i].start_pointer < MAX_SANITY_CAP && headers[i].sample_length < MAX_SANITY_CAP &&
            headers[i].loop_start < MAX_SANITY_CAP && headers[i].loop_end < MAX_SANITY_CAP &&
            headers[i].start_pointer > 0 && headers[i].sample_length > 0)
        {
            DivSample* s = new DivSample;

            s->rate = headers[i].sample_rate;
            s->centerRate = headers[i].sample_rate;
            s->depth = DIV_SAMPLE_DEPTH_8BIT;
            s->init(headers[i].sample_length); //byte per sample

            reader.seek((int)headers[i].start_pointer + SAMPLE_DATA_OFFSET, SEEK_SET);

            int sample_pos = 0;

            for (uint32_t j = 0; j < headers[i].sample_length; j++)
            {
                unsigned char curr_byte = (unsigned char)reader.readC();
                curr_byte += 0x80;

                s->data8[sample_pos] = curr_byte;
                sample_pos++;
            }

            if (headers[i].loop_start != NO_LOOP && headers[i].loop_end != NO_LOOP)
            {
                s->loop = true;
                s->loopMode = DIV_SAMPLE_LOOP_FORWARD;
                s->loopStart = headers[i].loop_start;
                s->loopEnd = headers[i].loop_end;
            }

            ret.push_back(s);

            logI("pzi: start %d len %d sample rate %d loop start %d loop end %d", headers[i].start_pointer, headers[i].sample_length, 
                headers[i].sample_rate, headers[i].loop_start, headers[i].loop_end);
        }
    }
  } 
  catch (EndOfFileException& e) 
  {
    lastError=_LE("premature end of file");
    logE("premature end of file");
  }
}