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

#ifdef HAVE_GUI
#include "../gui/gui.h"
extern FurnaceGUI g;
#endif

class DivEngine;

//P VOX ADPCM sample bank

/* =======================================
                 Header
=======================================

0x0000 - 0x03FF  256 * {
  Sample start (uint32_t)
    - 0x00000000 = unused
}


=======================================
                  Body
=======================================

Stream of Sample Data {

  MSM6258 ADPCM encoding
    nibble-swapped VOX / Dialogic ADPCM
  Mono
  Sample rate?
    16000Hz seems fine

} */

#define P_BANK_SIZE 256
#define P_SAMPLE_RATE 16000

typedef struct
{
    uint32_t start_pointer;
} P_HEADER;


void DivEngine::loadP(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    P_HEADER headers[P_BANK_SIZE];

    for(int i = 0; i < P_BANK_SIZE; i++)
    {
        headers[i].start_pointer = (unsigned int)reader.readI_BE();
    }

    for(int i = 0; i < P_BANK_SIZE; i++)
    {
        if(headers[i].start_pointer != 0)
        {
            DivSample* s = new DivSample;

            s->rate = P_SAMPLE_RATE;
            s->centerRate = P_SAMPLE_RATE;
            s->depth = DIV_SAMPLE_DEPTH_VOX;

            reader.seek((int)headers[i].start_pointer, SEEK_SET);

            int sample_pos = 0;
            int sample_len = 0;

            if(i < P_BANK_SIZE - 1)
            {
                sample_len = headers[i + 1].start_pointer - headers[i].start_pointer;
            }
            else
            {
                sample_len = (int)reader.size() - headers[i].start_pointer;
            }

            if(sample_len > 0)
            {
                s->init(sample_len * 2);

                for(int j = 0; j < sample_len; j++)
                {
                    unsigned char curr_byte = (unsigned char)reader.readC();
                    curr_byte = (curr_byte << 4) | (curr_byte >> 4);

                    s->dataVOX[sample_pos] = curr_byte;
                    sample_pos++;
                }

                ret.push_back(s);
                logI("p: start %d len %d", headers[i].start_pointer, sample_len);
            }
            else
            {
                delete s;
            }
        }
    }
  } 
  catch (EndOfFileException& e) 
  {
    lastError=_("premature end of file");
    logE("premature end of file");
  }
}