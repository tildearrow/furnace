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

//P86 8-bit PCM sample bank

/* =======================================
                 Header
=======================================

0x0000           Identifier (12b)
                 "PCM86 DATA(\n)(\0)"
0x000C           Targeted P86DRV version (1b)
                 version <high nibble>.<low nibble>
0x000D           File Length (3b)
0x0010 - 0x060F  256 * {

  Pointer to Sample Data Start  (3b)
  Length of Sample Data         (3b)

  (0x000000 0x000000 -> no sample for this instrument ID)

}


=======================================
                  Body
=======================================

Stream of Sample Data {

  8-Bit Signed
  Mono
  16540Hz
  (above sample rate according to KAJA's documentation
   any sample rate possible, for different base note & octave)

} */

#define P86_BANK_SIZE 256
#define P86_SAMPLE_RATE 16540

#define P86_FILE_SIG "PCM86 DATA\n\0"

typedef struct
{
    uint32_t start_pointer;
    uint32_t sample_length;
} P86_HEADER;

#define UNUSED(x) (void)(x)

uint32_t read_3bytes(SafeReader& reader)
{
    unsigned char arr[3];

    for (int i = 0; i < 3; i++)
    {
        arr[i] = (unsigned char)reader.readC();
    }

    return (arr[0] | (arr[1] << 8) | (arr[2] << 16)); 
}

void DivEngine::loadP86(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    P86_HEADER headers[P86_BANK_SIZE];

    String file_sig = reader.readString(12);
    if(file_sig != P86_FILE_SIG) return;

    uint8_t version = reader.readC();
    UNUSED(version);

    uint32_t file_size = read_3bytes(reader);
    UNUSED(file_size);

    for(int i = 0; i < P86_BANK_SIZE; i++)
    {
        headers[i].start_pointer = read_3bytes(reader);
        headers[i].sample_length = read_3bytes(reader);
    }

    for(int i = 0; i < P86_BANK_SIZE; i++)
    {
        if(headers[i].start_pointer != 0 && headers[i].sample_length != 0)
        {
            DivSample* s = new DivSample;

            s->rate = P86_SAMPLE_RATE;
            s->centerRate = P86_SAMPLE_RATE;
            s->depth = DIV_SAMPLE_DEPTH_8BIT;
            s->init(headers[i].sample_length); //byte per sample

            reader.seek((int)headers[i].start_pointer, SEEK_SET);

            int sample_pos = 0;

            for(uint32_t j = 0; j < headers[i].sample_length; j++)
            {
                unsigned char curr_byte = (unsigned char)reader.readC();
                //curr_byte += 0x80;

                s->data8[sample_pos] = curr_byte;
                sample_pos++;
            }

            ret.push_back(s);

            logI("p86: start %06X len %06X", headers[i].start_pointer, headers[i].sample_length);
        }
    }
  } 
  catch (EndOfFileException& e) 
  {
    lastError=_LE("premature end of file");
    logE("premature end of file");
  }
}