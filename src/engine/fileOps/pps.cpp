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

//PPS AY-3-8910 sample bank

/* =======================================
                 Header
=======================================

0x0000 - 0x0053  14 * {

  Pointer to Sample Data Start  (1h)
  Length of Sample Data         (1h)
  "Pitch"(?)                    (1b)
  Volume Reduction              (1b)

}

  (0x0000 0x0000 [0x00 0x00] -> no sample for this instrument ID)

}


=======================================
                  Body
=======================================

Stream of Sample Data {

  4-Bit Unsigned
  (afaict)
  Mono
  16Hz
  (based on tests, maybe alternatively 8kHz)

} */

#define PPS_BANK_SIZE 14
#define PPS_SAMPLE_RATE 16000

typedef struct
{
    uint16_t start_pointer;
    uint16_t sample_length;
    uint8_t _pitch;
    uint8_t _vol;
} PPS_HEADER;


void DivEngine::loadPPS(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    PPS_HEADER headers[PPS_BANK_SIZE];

    for(int i = 0; i < PPS_BANK_SIZE; i++)
    {
        headers[i].start_pointer = (unsigned short)reader.readS();
        headers[i].sample_length = (unsigned short)reader.readS();
        headers[i]._pitch = (unsigned char)reader.readC();
        headers[i]._vol = (unsigned char)reader.readC();
    }

    for(int i = 0; i < PPS_BANK_SIZE; i++)
    {
        if(headers[i].start_pointer != 0 || headers[i].sample_length != 0
            || headers[i]._pitch != 0 || headers[i]._vol != 0)
        {
            DivSample* s = new DivSample;

            s->rate = PPS_SAMPLE_RATE;
            s->centerRate = PPS_SAMPLE_RATE;
            s->depth = DIV_SAMPLE_DEPTH_8BIT;
            s->init(headers[i].sample_length * 2); //byte per sample

            reader.seek((int)headers[i].start_pointer, SEEK_SET);

            int sample_pos = 0;

            for(int j = 0; j < headers[i].sample_length; j++)
            {
                unsigned char curr_byte = (unsigned char)reader.readC();

                s->data8[sample_pos] = (curr_byte >> 4) | (curr_byte & 0xf0);
                s->data8[sample_pos] += 0x80;
                sample_pos++;
                s->data8[sample_pos] = (curr_byte << 4) | (curr_byte & 0xf);
                s->data8[sample_pos] += 0x80;
                sample_pos++;
            }

            ret.push_back(s);

            logI("pps: start %d len %d", headers[i].start_pointer, headers[i].sample_length);
        }
    }
  } 
  catch (EndOfFileException& e) 
  {
    lastError=_("premature end of file");
    logE("premature end of file");
  }
}
