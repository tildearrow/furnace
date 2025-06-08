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

typedef struct {
    union {
        struct {
            uint8_t multipM: 4, ksrM: 1, sustM: 1, vibrM: 1, tremM : 1;
            uint8_t multipC: 4, ksrC: 1, sustC: 1, vibrC: 1, tremC : 1;
            uint8_t volM: 6, kslM: 2;
            uint8_t volC: 6, kslC: 2;
            uint8_t decM: 4, attckM: 4;
            uint8_t decC: 4, attckC: 4;
            uint8_t relM: 4, sustnM: 4;
            uint8_t relC: 4, sustnC: 4;
            uint8_t wformM: 2, : 6;
            uint8_t wformC: 2, : 6;
            uint8_t connect: 1, feedb: 3, : 4;
        };
        uint8_t data[11];
    };
} FM_INST_DATA;

static_assert(sizeof(FM_INST_DATA) == 11, "sizeof(FM_INST_DATA) != 11");

typedef struct {
    union {
        struct {
            uint8_t connect: 3, pan12: 2, pan34: 2, has_riff: 1;
            uint8_t feedb12: 4, feedb34: 4;
            uint8_t riff_def_speed: 4, detune: 4;
            uint8_t volume: 6, : 2;
        };
        uint8_t data[4];
    };
} FM_INST_DATA_new;

static_assert(sizeof(FM_INST_DATA_new) == 4, "sizeof(FM_INST_DATA) != 4");

typedef struct {
    union {
        struct {
            uint8_t multip: 4, ksr: 1, sust: 1, vibr: 1, trem: 1;
            uint8_t tl: 6, ksl: 2;
            uint8_t dec: 4, attck: 4;
            uint8_t rel: 4, sustn: 4;
            uint8_t wform: 3, : 5;
        };
        uint8_t data[5];
    };
} FM_OP_INST_DATA_new;

static_assert(sizeof(FM_OP_INST_DATA_new) == 5, "sizeof(FM_OP_INST_DATA_new) != 5");

const int opOrder[4]={
  0, 2, 1, 3
};

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

    reader->readC(); //skip null terminator

    ds->notes = description;

    delete[] description;
}

void RAD_import_FM_op(DivInstrument* ins, unsigned char furnace_op, FM_OP_INST_DATA_new* op_s)
{
    ins->fm.op[furnace_op].mult = op_s->multip;
    ins->fm.op[furnace_op].ksr = op_s->ksr;
    ins->fm.op[furnace_op].sus = op_s->sust;
    ins->fm.op[furnace_op].vib = op_s->vibr;
    ins->fm.op[furnace_op].am = op_s->trem;
    ins->fm.op[furnace_op].tl = op_s->tl;
    ins->fm.op[furnace_op].ksl = op_s->ksl;
    ins->fm.op[furnace_op].ar = op_s->attck;
    ins->fm.op[furnace_op].dr = op_s->dec;
    ins->fm.op[furnace_op].sl = op_s->sustn;
    ins->fm.op[furnace_op].rr = op_s->rel;
    ins->fm.op[furnace_op].ws = op_s->wform;
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

        if(shifted_version == 1) //old RAD
        {
            //31 instruments? will delete the excessive ones later
            for(int i = 0; i < 31; i++)
            {
                ds.ins.push_back(new DivInstrument());
            }
            
            bool list_end = false;

            FM_INST_DATA instr_s;

            while(!list_end)
            {
                int inst_num = reader.readC();

                if(inst_num == 0) list_end = true; //end of list

                if(inst_num > 31)
                {
                    logE("invalid instrument number!");
                    lastError="invalid instrument number";
                    delete[] file;
                    return false;
                }

                reader.read((void*)&instr_s, sizeof(FM_INST_DATA));
                
                DivInstrument* ins = ds.ins[inst_num];

                ins->type = DIV_INS_OPL;

                ins->fm.op[0].mult = instr_s.multipM;
                ins->fm.op[0].ksr = instr_s.ksrM;
                ins->fm.op[0].sus = instr_s.sustM;
                ins->fm.op[0].vib = instr_s.vibrM;
                ins->fm.op[0].am = instr_s.tremM;
                ins->fm.op[0].tl = instr_s.volM;
                ins->fm.op[0].ksl = instr_s.kslM;
                ins->fm.op[0].ar = instr_s.attckM;
                ins->fm.op[0].dr = instr_s.decM;
                ins->fm.op[0].sl = instr_s.sustnM;
                ins->fm.op[0].rr = instr_s.relM;
                ins->fm.op[0].ws = instr_s.wformM;

                ins->fm.op[1].mult = instr_s.multipC;
                ins->fm.op[1].ksr = instr_s.ksrC;
                ins->fm.op[1].sus = instr_s.sustC;
                ins->fm.op[1].vib = instr_s.vibrC;
                ins->fm.op[1].am = instr_s.tremC;
                ins->fm.op[1].tl = instr_s.volC;
                ins->fm.op[1].ksl = instr_s.kslC;
                ins->fm.op[1].ar = instr_s.attckC;
                ins->fm.op[1].dr = instr_s.decC;
                ins->fm.op[1].sl = instr_s.sustnC;
                ins->fm.op[1].rr = instr_s.relC;
                ins->fm.op[1].ws = instr_s.wformC;

                ins->fm.fb = instr_s.feedb;
                ins->fm.alg = instr_s.connect;
            }
        }

        if(shifted_version == 2) //new RAD
        {
            //127 instruments? will delete the excessive ones later
            for(int i = 0; i < 127; i++)
            {
                ds.ins.push_back(new DivInstrument());
            }
            
            bool list_end = false;

            FM_INST_DATA_new instr_s;
            FM_OP_INST_DATA_new op_s;

            while(!list_end)
            {
                unsigned char inst_num = reader.readC();

                if(inst_num == 0) list_end = true; //end of list

                if(inst_num > 127)
                {
                    logE("invalid instrument number!");
                    lastError="invalid instrument number";
                    delete[] file;
                    return false;
                }

                char name[257] = { 0 };

                unsigned char name_len = reader.readC();
                reader.read((void*)&name, name_len);
                
                DivInstrument* ins = ds.ins[inst_num];

                ins->type = DIV_INS_OPL;
                ins->name = name;

                reader.read((void*)&instr_s, sizeof(FM_INST_DATA_new));

                if(instr_s.connect == 7) //MIDI instrument?
                {
                    unsigned char dummy[3];
                    reader.read((void*)&dummy, 3); //TODO: add riff handling. RN it's just skipping the info
                }
                else
                {
                    ins->fm.fb = instr_s.feedb12; //where do I put feedb34???
                    ins->fm.alg = instr_s.connect;

                    ins->fm.ops = ins->fm.alg > 1 ? 4 : 2;
                    
                    for(int i = 0; i < 4; i++) //four ops even for 2-op instruments?
                    {
                        reader.read((void*)&op_s, sizeof(FM_OP_INST_DATA_new));

                        unsigned char furnace_op = 0;

                        if(ins->fm.ops == 4)
                        {
                            furnace_op = opOrder[i];
                            RAD_import_FM_op(ins, furnace_op, &op_s);
                        }
                        else
                        {
                            if(i < 2)
                            {
                                furnace_op = i;
                                RAD_import_FM_op(ins, furnace_op, &op_s);
                            }
                        }
                    }

                    //panning...
                    ins->std.panLMacro.len = 1;
                    ins->std.panLMacro.val[0] = instr_s.pan12;
                }
            }
        }

        ds.insLen = ds.ins.size();

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
