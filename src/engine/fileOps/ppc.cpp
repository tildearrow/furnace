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

//PPC PMD's YM2608 ADPCM-B sample bank

/* ========================================
                 General
========================================

ADPCM RAM addresses: see docs/common.txt {

  address_start = 0x0026
  file_header_size = 0x0420

}


========================================
                 Header
========================================

0x0000           Identifier (30b)
                 "ADPCM DATA for  PMD ver.4.4-  "
0x001E           Address of End of Data (32B blocks) in ADPCM RAM (1h)
                 File Size == address -> file offset

0x0020 - 0x041F  256 * {

  Start of Sample (32b blocks) in ADPCM RAM  (1h)
  End of Sample (32b blocks) in ADPCM RAM    (1h)

  (0x0000 0x0000 -> no sample for this instrument ID)

}


========================================
                  Body
========================================

Stream of Sample Data {

  Yamaha ADPCM-B encoding (4-Bit Signed ADPCM)
  Mono
  16kHz
  (above sample rate according to KAJA's documentation
   any sample rate possible, for different base note & octave)

} */

#define PPC_FILE_SIG "ADPCM DATA for  PMD ver.4.4-  "

#define PPC_BANK_SIZE 256
#define PPC_SAMPLE_RATE 16000

typedef struct
{
    uint16_t start_pointer;
    uint16_t end_pointer;
} PPC_HEADER;

#define UNUSED(x) (void)(x)

#define ADPCM_DATA_START 0x0420

void DivEngine::loadPPC(SafeReader& reader, std::vector<DivSample*>& ret, String& stripPath)
{
  try 
  {
    reader.seek(0, SEEK_SET);

    String file_sig = reader.readString(30);
    unsigned short end_of_data = (unsigned short)reader.readS();
    UNUSED(end_of_data);

    if(file_sig != PPC_FILE_SIG) return;

    PPC_HEADER headers[PPC_BANK_SIZE];

    for(int i = 0; i < PPC_BANK_SIZE; i++)
    {
        headers[i].start_pointer = (unsigned short)reader.readS();
        headers[i].end_pointer = (unsigned short)reader.readS();
    }

    for(int i = 0; i < PPC_BANK_SIZE; i++)
    {
        if((headers[i].start_pointer != 0 || headers[i].end_pointer != 0) && headers[i].start_pointer < headers[i].end_pointer)
        {
            DivSample* s = new DivSample;

            s->centerRate = PPC_SAMPLE_RATE;
            s->depth = DIV_SAMPLE_DEPTH_ADPCM_B;
            s->init((headers[i].end_pointer - headers[i].start_pointer) * 32 * 2);

            int sample_pos = 0;
            int sample_length = (headers[i].end_pointer - headers[i].start_pointer) * 32;

            //reader.seek(ADPCM_DATA_START + headers[i].start_pointer * 32, SEEK_SET);

            for(int j = 0; j < sample_length; j++)
            {
                unsigned char curr_byte = (unsigned char)reader.readC();
                //curr_byte=(curr_byte<<4)|(curr_byte>>4);

                s->dataB[sample_pos] = curr_byte;
                sample_pos++;
            }

            logI("ppc: start %d end %d len in bytes %d", headers[i].start_pointer, headers[i].end_pointer, (headers[i].end_pointer - headers[i].start_pointer) * 32);

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
