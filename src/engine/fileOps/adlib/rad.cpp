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

//Reality Adlib Tracker modules import (.rad versions 1.0 and 2.1) by SudoMaker & Enet4
//Adapted into Furnace by LTVA

#include "../fileOpsCommon.h"

#ifdef HAVE_MOMO
#define ngettext momo_ngettext
#endif

void RAD_read_description(DivSong* ds, SafeReader* reader)
{
    size_t description_start_pos = reader->tell();

    bool end = false;

    int description_length = 0;

    do
    {
        unsigned char next_char = reader->readC();
        if(next_char == '\0')
        {
            end = true;
        }

        if(next_char >= 0x02 && next_char <= 0x1F)
        {
            description_length += next_char;
        }

        if(next_char >= 0x20 || next_char == 0x01)
        {
            description_length++;
        }
    } while(!end);

    char* description = new char[description_length + 1];

    description[description_length] = '\0';

    reader->seek(description_start_pos, SEEK_SET);

    int curr_pos = 0;

    for(curr_pos = 0; curr_pos < description_length;)
    {
        unsigned char next_char = reader->readC();
        
        if(next_char >= 0x02 && next_char <= 0x1F)
        {
            for(unsigned char j = 0; j < next_char; j++)
            {
                description[curr_pos] = ' ';
                curr_pos++;
            }
        }

        if(next_char >= 0x20)
        {
            description[curr_pos] = next_char;
            curr_pos++;
        }

        if(next_char == 0x01)
        {
            description[curr_pos] = '\n';
            curr_pos++;
        }
    }

    ds->notes = description;

    delete[] description;
}

bool DivEngine::loadRAD(unsigned char* file, size_t len) 
{
    SafeReader reader=SafeReader(file,len);
    warnings="";

    try 
    {
        DivSong ds;
        ds.version = DIV_VERSION_RAD;
        ds.subsong.push_back(new DivSubSong);
        DivSubSong* s = ds.subsong[0];
        ds.systemLen = 1;
        ds.system[0] = DIV_SYSTEM_OPL2; //OPL2 by default

        int version = 0;
        int shifted_version = 0;

        reader.seek(16, SEEK_SET);

        version = reader.readC();

        if(version != 0x10 && version != 0x21)
        {
            lastError = _("Unsupported file version!");
            delete[] file;
            return false;
        }

        shifted_version = (abs(version) >> 4);

        logI("RAD version %d", version);

        if(version == 0x21)
        {
            ds.system[0] = DIV_SYSTEM_OPL3;

            for(int i = 9; i < 18; i++)
            {
                ds.subsong[0]->chanShow[i] = false; //hide unused OPL3 channels
            }
        }

        unsigned char flags = reader.readC();

        s->speeds.val[0] = flags & 0x1F;

        if(shifted_version == 1) //old RAD
        {
            if(flags & (1 << 7)) //read description
            {
                RAD_read_description(&ds, &reader);
            }
        }

        if(shifted_version == 2) //new RAD
        {
            if(flags & (1 << 5)) //custom BPM
            {
                uint16_t bpm = reader.readC();
                bpm <<= 8;
                bpm |= reader.readC();
                
                logV("%d BPM", bpm);

                if (bpm == 0) 
                {
                    s->virtualTempoN = s->virtualTempoD;
                } 
                else
                {
                    s->virtualTempoN = bpm;
                }
            }

            RAD_read_description(&ds, &reader);
        }

        ds.linearPitch = 0;
        ds.pitchMacroIsLinear = false;
        ds.pitchSlideSpeed=8;

        if (active) quitDispatch();
        BUSY_BEGIN_SOFT;
        saveLock.lock();
        song.unload();
        song=ds;
        changeSong(0);
        recalcChans();
        saveLock.unlock();
        BUSY_END;

        if (active) 
        {
            initDispatch();
            BUSY_BEGIN;
            renderSamples();
            reset();
            BUSY_END;
        }
    }
    catch (EndOfFileException& e) 
    {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
    }
    
    delete[] file;
    return true;
}