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
            uint8_t connect: 1, feedb: 3, : 4;
            uint8_t wformM: 2, : 6;
            uint8_t wformC: 2, : 6;
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

const int rad_order[4]={
  3, 2, 1, 0
};

typedef struct {
    uint8_t note: 4, octave: 3, prev_inst: 1;
    uint8_t instrument: 7, : 1;
    uint8_t effect: 5, : 3;
    uint8_t effect_param: 7, : 1;
    bool has_effect;
} RAD_pattern_step;

typedef struct {
    RAD_pattern_step step[9][128];
} RAD_pattern;

static int findEmptyEffectSlot(short* data)
{
    for(int i = 0; i < DIV_MAX_EFFECTS; i++)
    {
        if(data[DIV_PAT_FX(i)] == -1)
        {
            return i;
        }
    }

    return -1;
}

void RAD_convert_effect(DivPattern* furnace_pat, RAD_pattern* pat, int i, int j)
{
    unsigned char chan_to_op_map[9] = { 0, 0, 1, 2, 3, 0, 1, 2, 3 };

    switch(pat->step[i][j].effect)
    {
        case 1:
        case 2:
        case 3:
        {
            furnace_pat->newData[j][DIV_PAT_FX(0)] = pat->step[i][j].effect;
            furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = pat->step[i][j].effect_param;
            break;
        }
        case 5:
        {
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x06;

            if(pat->step[i][j].effect_param >= 1 && pat->step[i][j].effect_param <= 49)
            {
                furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = (pat->step[i][j].effect_param * 0xF / 49) & 0xF;
            }
            if(pat->step[i][j].effect_param >= 51 && pat->step[i][j].effect_param <= 99)
            {
                furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = (((pat->step[i][j].effect_param - 51) * 0xF / (99 - 51)) & 0xF) << 4;
            }
            break;
        }
        case ('A' - 'A' + 10):
        {
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x0A;

            if(pat->step[i][j].effect_param >= 1 && pat->step[i][j].effect_param <= 49)
            {
                furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = ((pat->step[i][j].effect_param > 0xF) ? 0xF : pat->step[i][j].effect_param) & 0xF;
            }
            if(pat->step[i][j].effect_param >= 51 && pat->step[i][j].effect_param <= 99)
            {
                furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = (((pat->step[i][j].effect_param - 51) > 0xF ? 0xF : (pat->step[i][j].effect_param - 51)) & 0xF) << 4;
            }
            break;
        }
        case ('C' - 'A' + 10):
        {
            furnace_pat->newData[j][DIV_PAT_VOL] = ((pat->step[i][j].effect_param > 0x3F) ? 0x3F : pat->step[i][j].effect_param); //just instrument volume
            break;
        }
        case ('D' - 'A' + 10):
        {
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x0d;
            furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = pat->step[i][j].effect_param;
            break;
        }
        case ('F' - 'A' + 10):
        {
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x0f;
            furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = pat->step[i][j].effect_param;
            break;
        }
        case ('I' - 'A' + 10):
        {
            //TODO?
            break;
        }
        case ('M' - 'A' + 10):
        {
            if(i > 0)
            {
                unsigned char op = chan_to_op_map[i];

                furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x16;
                furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = (op << 4) | pat->step[i][j].effect_param;
            }
            break;
        }
        case ('R' - 'A' + 10):
        {
            //play global riff
            //convert to non-existent effect just so it hints what should happen there
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0xA0;
            furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = pat->step[i][j].effect_param;
            break;
        }
        case ('T' - 'A' + 10):
        {
            //play global transposed riff
            //convert to non-existent effect just so it hints what should happen there
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0xA1;
            furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = pat->step[i][j].effect_param;
            break;
        }
        case ('U' - 'A' + 10):
        {
            furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x11;
            furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = (pat->step[i][j].effect_param > 7) ? ((pat->step[i][j].effect_param - 10) | 0x10) : pat->step[i][j].effect_param;
            break;
        }
        case ('V' - 'A' + 10):
        {
            if(i > 0)
            {
                unsigned char op = chan_to_op_map[i]; //same thing as with multiplier but for op's TL

                furnace_pat->newData[j][DIV_PAT_FX(0)] = 0x12 + op;
                furnace_pat->newData[j][DIV_PAT_FXVAL(0)] = (0x3F - ((pat->step[i][j].effect_param > 0x3F) ? 0x3F : pat->step[i][j].effect_param));
            }
            break;
        }
        default: break;
    }
}

bool RAD_read_pattern(SafeReader* reader, RAD_pattern* pat, DivPattern** furnace_patterns, DivSubSong* subsong, int version, bool hide_chans)
{
    unsigned short len = (unsigned char)reader->readC();
    len |= ((unsigned char)reader->readC() << 8);

    if((int)reader->size() - (int)reader->tell() < len) 
    {
        logE("pattern data too short!");
        //lastError = _("file too short!");
        return false;
    }
    
    unsigned short bytes_read = 0;

    unsigned char max_line_number = 0;

    while(bytes_read < len && len != 0)
    {
        unsigned char line = reader->readC();
        bytes_read++;

        if(bytes_read >= len) break;

        unsigned char line_number = line & 0x7F;

        if(max_line_number < line_number) max_line_number = line_number;

        while(1)
        {
            unsigned char line_info = reader->readC();
            bytes_read++;

            if(bytes_read >= len) break;

            if(line_info & (1 << 6)) //note/octave byte
            {
                unsigned char byte = reader->readC();
                bytes_read++;

                pat->step[(line_info & 0xF) % 9][line_number].note = byte & 0xF;
                pat->step[(line_info & 0xF) % 9][line_number].octave = (byte >> 4) & 0x7;
                pat->step[(line_info & 0xF) % 9][line_number].prev_inst = (byte >> 7) & 0x1;

                if(bytes_read >= len) break;
            }

            if(line_info & (1 << 5)) //instrument byte
            {
                unsigned char byte = reader->readC();
                bytes_read++;

                if((byte & 0x7F) > 0)
                {
                    pat->step[(line_info & 0xF) % 9][line_number].instrument = (byte & 0x7F) * 2;
                }

                if(bytes_read >= len) break;
            }

            if(line_info & (1 << 4)) //effect/param bytes
            {
                unsigned char byte = reader->readC();
                bytes_read++;

                pat->step[(line_info & 0xF) % 9][line_number].effect = byte & 0x1F;

                pat->step[(line_info & 0xF) % 9][line_number].has_effect = true;

                if(bytes_read >= len) break;

                byte = reader->readC();
                bytes_read++;

                pat->step[(line_info & 0xF) % 9][line_number].effect_param = byte & 0x7F;

                if(bytes_read >= len) break;
            }

            if(line_info & (1 << 7)) break;
        }

        if(line & (1 << 7)) break;
    }

    subsong->patLen = 64;

    if(hide_chans)
    {
        for(int i = 9; i < 18; i++)
        {
            subsong->chanShow[i] = false; //hide unused OPL3 channels
        }
    }

    for(int i = 0; i < 9; i++) //convert pattern data
    {
        DivPattern* furnace_pat = furnace_patterns[i];

        for(int j = 0; j <= max_line_number; j++)
        {
            if(pat->step[i][j].note == 0xF)
            {
                furnace_pat->newData[j][DIV_PAT_NOTE] = DIV_NOTE_REL; //key off
            }
            else if(pat->step[i][j].note > 0)
            {
                if(version == 1)
                {
                    furnace_pat->newData[j][DIV_PAT_NOTE] = (pat->step[i][j].note - 1) + pat->step[i][j].octave * 12 + 60;
                    //furnace_pat->newData[j][1] = pat->step[i][j].octave;
                }
                if(version == 2)
                {
                    if(pat->step[i][j].note == 12)
                    {
                        furnace_pat->newData[j][DIV_PAT_NOTE] = 12 + pat->step[i][j].octave * 12 + 60;
                        //furnace_pat->newData[j][1] = pat->step[i][j].octave;
                    }
                    else
                    {
                        furnace_pat->newData[j][DIV_PAT_NOTE] = pat->step[i][j].note + pat->step[i][j].octave * 12 + 60;
                        //furnace_pat->newData[j][1] = pat->step[i][j].octave;
                    }
                }
            }

            if(pat->step[i][j].instrument > 0)
            {
                furnace_pat->newData[j][DIV_PAT_INS] = pat->step[i][j].instrument;
            }

            if(pat->step[i][j].has_effect)
            {
                RAD_convert_effect(furnace_pat, pat, i, j);
            }
        }
    }

    return true;
}

bool RAD_read_description(DivSong* ds, SafeReader* reader)
{
    size_t description_start_pos = reader->tell();

    bool end = false;

    int description_length = 0;

    do
    {
        if(reader->size() == reader->tell() + 1) 
        {
            logE("file too short!");
            //lastError = _("file too short!");
            return false;
        }

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

    return true;
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

    int riff_subsong_index = 1;

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

        bool two_twoop_inst[257] = { false };

        reader.seek(16, SEEK_SET);

        if(reader.size() == reader.tell() + 1) 
        {
            logE("file too short!");
            lastError = _("file too short!");
            delete[] file;
            return false;
        }

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

            /*for(int i = 9; i < 18; i++)
            {
                ds.subsong[0]->chanShow[i] = false; //hide unused OPL3 channels
            }*/
        }

        if(reader.size() == reader.tell() + 1) 
        {
            logE("file too short!");
            lastError = _("file too short!");
            delete[] file;
            return false;
        }

        unsigned char flags = reader.readC();

        s->speeds.val[0] = flags & 0x1F;

        if(shifted_version == 1) //old RAD
        {
            if(flags & (1 << 7)) //read description
            {
                bool desc = RAD_read_description(&ds, &reader);

                if(!desc)
                {
                    lastError = _("File too short!");
                    delete[] file;
                    return false;
                }
            }
        }

        if(shifted_version == 2) //new RAD
        {
            if(flags & (1 << 5)) //custom BPM
            {
                unsigned short bpm = (unsigned char)reader.readC();
                bpm |= ((unsigned char)reader.readC() << 8);
                
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

            bool desc = RAD_read_description(&ds, &reader);

            if(!desc)
            {
                lastError = _("File too short!");
                delete[] file;
                return false;
            }
        }

        if(shifted_version == 1) //old RAD
        {
            //31 instruments? will delete the excessive ones later
            for(int i = 0; i < 31 + 1; i++)
            {
                ds.ins.push_back(new DivInstrument());
            }
            
            bool list_end = false;

            FM_INST_DATA instr_s;

            while(!list_end)
            {
                if(reader.size() == reader.tell() + 1) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                int inst_num = reader.readC();

                if(inst_num == 0)
                {
                    list_end = true; //end of list
                    break;
                }

                if(inst_num > 31)
                {
                    logE("invalid instrument number!");
                    lastError="invalid instrument number";
                    delete[] file;
                    return false;
                }

                if(reader.size() - reader.tell() < sizeof(FM_INST_DATA)) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                reader.read((void*)&instr_s, sizeof(FM_INST_DATA));
                
                DivInstrument* ins = ds.ins[inst_num];

                ins->type = DIV_INS_OPL;

                ins->fm.op[1].mult = instr_s.multipM;
                ins->fm.op[1].ksr = instr_s.ksrM;
                ins->fm.op[1].sus = instr_s.sustM;
                ins->fm.op[1].vib = instr_s.vibrM;
                ins->fm.op[1].am = instr_s.tremM;
                ins->fm.op[1].tl = instr_s.volM;
                ins->fm.op[1].ksl = instr_s.kslM;
                ins->fm.op[1].ar = instr_s.attckM;
                ins->fm.op[1].dr = instr_s.decM;
                ins->fm.op[1].sl = instr_s.sustnM;
                ins->fm.op[1].rr = instr_s.relM;
                ins->fm.op[1].ws = instr_s.wformM;

                ins->fm.op[0].mult = instr_s.multipC;
                ins->fm.op[0].ksr = instr_s.ksrC;
                ins->fm.op[0].sus = instr_s.sustC;
                ins->fm.op[0].vib = instr_s.vibrC;
                ins->fm.op[0].am = instr_s.tremC;
                ins->fm.op[0].tl = instr_s.volC;
                ins->fm.op[0].ksl = instr_s.kslC;
                ins->fm.op[0].ar = instr_s.attckC;
                ins->fm.op[0].dr = instr_s.decC;
                ins->fm.op[0].sl = instr_s.sustnC;
                ins->fm.op[0].rr = instr_s.relC;
                ins->fm.op[0].ws = instr_s.wformC;

                ins->fm.fb = instr_s.feedb;
                ins->fm.alg = instr_s.connect;
            }
        }

        if(shifted_version == 2) //new RAD
        {
            //127 instruments? will delete the excessive ones later
            for(int i = 0; i < (127 + 1) * 2 + 1; i++)
            {
                ds.ins.push_back(new DivInstrument());
            }
            
            bool list_end = false;

            FM_INST_DATA_new instr_s;
            FM_OP_INST_DATA_new op_s;

            while(!list_end)
            {
                if(reader.size() == reader.tell() + 1) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                unsigned char inst_num = reader.readC();

                if(inst_num == 0)
                {
                    list_end = true; //end of list
                    break;
                }

                if(inst_num > 127)
                {
                    logE("invalid instrument number!");
                    lastError="invalid instrument number";
                    delete[] file;
                    return false;
                }

                char name[257] = { 0 };

                if(reader.size() == reader.tell() + 1) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                unsigned char name_len = reader.readC();

                if(reader.size() - reader.tell() < (size_t)name_len) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                reader.read((void*)&name, name_len);
                
                DivInstrument* ins = ds.ins[(int)inst_num * 2];
                DivInstrument* ins2 = ds.ins[(int)inst_num * 2 + 1];

                ins->type = DIV_INS_OPL;
                ins->name = name;

                if(reader.size() - reader.tell() < sizeof(FM_INST_DATA_new)) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                reader.read((void*)&instr_s, sizeof(FM_INST_DATA_new));

                if(instr_s.connect == 7) //MIDI instrument?
                {
                    unsigned char dummy[3];

                    if(reader.size() - reader.tell() < 3) 
                    {
                        logE("file too short!");
                        lastError = _("file too short!");
                        delete[] file;
                        return false;
                    }

                    reader.read((void*)&dummy, 3);

                    ins->name += " [MIDI]";
                }
                else
                {
                    ins->fm.fb = instr_s.feedb12;
                    ins->fm.alg = instr_s.connect;

                    ins->fm.ops = ins->fm.alg > 1 ? 4 : 2;
                    
                    if(ins->fm.alg <= 3)
                    {
                        for(int i = 0; i < 4; i++) //four ops even for 2-op instruments?
                        {
                            if(reader.size() - reader.tell() < sizeof(FM_OP_INST_DATA_new)) 
                            {
                                logE("file too short!");
                                lastError = _("file too short!");
                                delete[] file;
                                return false;
                            }
                            reader.read((void*)&op_s, sizeof(FM_OP_INST_DATA_new));

                            unsigned char furnace_op = 0;

                            if(ins->fm.ops == 4)
                            {
                                furnace_op = rad_order[opOrder[i]];
                                RAD_import_FM_op(ins, furnace_op, &op_s);
                            }
                            else
                            {
                                if(i < 2)
                                {
                                    furnace_op = 1 - i;
                                    RAD_import_FM_op(ins, furnace_op, &op_s);
                                }
                            }
                        }

                        if(ins->fm.ops == 4)
                        {
                            ins->fm.alg -= 2;
                        }

                        //panning...
                        ins->std.panLMacro.len = 1;
                        ins->std.panLMacro.val[0] = instr_s.pan12;

                        if(ins->std.panLMacro.val[0] == 0) ins->std.panLMacro.val[0] = 3;
                    }
                    else //two instruments, 2-op each
                    {
                        two_twoop_inst[(int)inst_num * 2] = true;

                        ins->type = DIV_INS_OPL;
                        ins->name = name;
                        ins->name += _(" [first two operators]");
                        ins2->type = DIV_INS_OPL;
                        ins2->name = name;
                        ins2->name += _(" [second two operators]");

                        ins->fm.fb = instr_s.feedb12;
                        ins2->fm.fb = instr_s.feedb34;

                        ins->fm.ops = 2;
                        ins2->fm.ops = 2;
                        
                        if(ins->fm.alg == 4)
                        {
                            ins->fm.alg = 0;
                            ins2->fm.alg = 0;
                        }
                        if(ins->fm.alg == 5)
                        {
                            ins->fm.alg = 0;
                            ins2->fm.alg = 1;
                        }
                        if(ins->fm.alg == 6)
                        {
                            ins->fm.alg = 1;
                            ins2->fm.alg = 1;
                        }

                        for(int i = 0; i < 2; i++)
                        {
                            if(reader.size() - reader.tell() < sizeof(FM_OP_INST_DATA_new)) 
                            {
                                logE("file too short!");
                                lastError = _("file too short!");
                                delete[] file;
                                return false;
                            }

                            reader.read((void*)&op_s, sizeof(FM_OP_INST_DATA_new));

                            unsigned char furnace_op = 1 - i;

                            RAD_import_FM_op(ins, furnace_op, &op_s);
                        }

                        for(int i = 0; i < 2; i++)
                        {
                            if(reader.size() - reader.tell() < sizeof(FM_OP_INST_DATA_new)) 
                            {
                                logE("file too short!");
                                lastError = _("file too short!");
                                delete[] file;
                                return false;
                            }

                            reader.read((void*)&op_s, sizeof(FM_OP_INST_DATA_new));

                            unsigned char furnace_op = 1 - i;

                            RAD_import_FM_op(ins2, furnace_op, &op_s);
                        }

                        //panning...
                        ins->std.panLMacro.len = 1;
                        ins->std.panLMacro.val[0] = instr_s.pan12;

                        if(ins->std.panLMacro.val[0] == 0) ins->std.panLMacro.val[0] = 3;

                        ins2->std.panLMacro.len = 1;
                        ins2->std.panLMacro.val[0] = instr_s.pan34;

                        if(ins2->std.panLMacro.val[0] == 0) ins2->std.panLMacro.val[0] = 3;
                    }
                }

                if(instr_s.has_riff)
                {
                    RAD_pattern* riff = new RAD_pattern;
                    memset((void*)riff, 0, sizeof(RAD_pattern));

                    ds.subsong.push_back(new DivSubSong);

                    DivSubSong* riff_subsong = ds.subsong[riff_subsong_index];

                    riff_subsong->name = _("Instrument riff: ") + ins->name;
                    riff_subsong->speeds.val[0] = instr_s.riff_def_speed;

                    DivPattern* patterns[9];

                    for(int i = 0; i < 9; i++)
                    {
                        patterns[i] = riff_subsong->pat[i].getPattern(0, true);
                    }

                    bool pat_read = RAD_read_pattern(&reader, riff, patterns, riff_subsong, shifted_version, true);

                    if(!pat_read)
                    {
                        lastError = _("Incomplete pattern data!");
                        delete[] file;
                        delete riff;
                        return false;
                    }

                    riff_subsong_index++;

                    delete riff;

                    bool end_pitch_slide = true;

                    for(int row = 0; row < riff_subsong->patLen; row++) //try to convert some data from instrument riff into macros
                    {
                        for(int ch = 0; ch < 9; ch++)
                        {
                            if(patterns[ch]->newData[row][DIV_PAT_FX(0)] == 0x0d)
                            {
                                goto end;
                            }

                            if(patterns[ch]->newData[row][DIV_PAT_FX(0)] == 0x01 || patterns[ch]->newData[row][DIV_PAT_FX(0)] == 0x02)
                            {
                                ins->std.pitchMacro.mode = 1; //relative mode

                                if(ins->std.pitchMacro.len + instr_s.riff_def_speed < 255)
                                {
                                    for(int sp = 0; sp < instr_s.riff_def_speed; sp++)
                                    {
                                        ins->std.pitchMacro.val[ins->std.pitchMacro.len] = patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] * ((patterns[ch]->newData[row][DIV_PAT_FX(0)]) == 0x02 ? -6 : 6);
                                        ins->std.pitchMacro.len++;
                                    }
                                }

                                if(patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] == 0)
                                {
                                    end_pitch_slide = false;
                                }
                            }

                            if(patterns[ch]->newData[row][DIV_PAT_FX(0)] >= 0x12 && patterns[ch]->newData[row][DIV_PAT_FX(0)] <= 0x15) //volume (TL)
                            {
                                DivInstrumentMacro* op_volume = NULL;

                                if(instr_s.connect <= 1)
                                {
                                    if(patterns[ch]->newData[row][DIV_PAT_FX(0)] <= 0x13)
                                    {
                                        op_volume = &ins->std.opMacros[rad_order[opOrder[0x13 - patterns[ch]->newData[row][DIV_PAT_FX(0)]]]].tlMacro;
                                    }
                                }
                                if(instr_s.connect > 1 && instr_s.connect <= 3) //one 4-op instrument
                                {
                                    op_volume = &ins->std.opMacros[rad_order[opOrder[0x15 - patterns[ch]->newData[row][DIV_PAT_FX(0)]]]].tlMacro;
                                }
                                if(instr_s.connect > 3) //two 2-op instruments
                                {
                                    if(patterns[ch]->newData[row][DIV_PAT_FX(0)] <= 0x13)
                                    {
                                        op_volume = &ins->std.opMacros[0x13 - patterns[ch]->newData[row][DIV_PAT_FX(0)]].tlMacro;
                                    }
                                    if(patterns[ch]->newData[row][DIV_PAT_FX(0)] >= 0x14)
                                    {
                                        op_volume = &ins2->std.opMacros[0x15 - patterns[ch]->newData[row][DIV_PAT_FX(0)]].tlMacro;
                                    }
                                }

                                if(op_volume != NULL)
                                {
                                    op_volume->speed = instr_s.riff_def_speed;

                                    if(op_volume->len == 0)
                                    {
                                        for(int iii = 0; iii < 256; iii++)
                                        {
                                            op_volume->val[iii] = -1;
                                        }
                                    }

                                    op_volume->len = row + 1;
                                    op_volume->val[row] = patterns[ch]->newData[row][DIV_PAT_FXVAL(0)];

                                    for(int iii = 1; iii < 256; iii++)
                                    {
                                        if(op_volume->val[iii] == -1 && op_volume->val[iii - 1] >= 0)
                                        {
                                            while(op_volume->val[iii] == -1 && iii < row)
                                            {
                                                op_volume->val[iii] = op_volume->val[iii - 1];
                                                iii++;
                                            }

                                            break;
                                        }
                                    }
                                }
                            }

                            if(patterns[ch]->newData[row][DIV_PAT_FX(0)] == 0x16) //multiplier
                            {
                                DivInstrumentMacro* op_mult = NULL;

                                if(instr_s.connect <= 1)
                                {
                                    if((patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4) < 2)
                                    {
                                        op_mult = &ins->std.opMacros[1 - (patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4)].multMacro;
                                    }
                                }
                                if(instr_s.connect > 1 && instr_s.connect <= 3) //one 4-op instrument
                                {
                                    if((patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4) < 4)
                                    {
                                        op_mult = &ins->std.opMacros[rad_order[opOrder[3 - (patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4)]]].multMacro;
                                    }
                                }
                                if(instr_s.connect > 3) //two 2-op instruments
                                {
                                    if((patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4) < 2)
                                    {
                                        op_mult = &ins->std.opMacros[1 - (patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4)].multMacro;
                                    }
                                    if((patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4) >= 2 && (patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4) < 4)
                                    {
                                        op_mult = &ins2->std.opMacros[1 - ((patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] >> 4) - 2)].multMacro;
                                    }
                                }

                                if(op_mult != NULL)
                                {
                                    op_mult->speed = instr_s.riff_def_speed;

                                    if(op_mult->len == 0)
                                    {
                                        for(int iii = 0; iii < 256; iii++)
                                        {
                                            op_mult->val[iii] = -1;
                                        }
                                    }

                                    op_mult->len = row + 1;
                                    op_mult->val[row] = patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] & 0xF;

                                    for(int iii = 1; iii < 256; iii++)
                                    {
                                        if(op_mult->val[iii] == -1 && op_mult->val[iii - 1] >= 0)
                                        {
                                            while(op_mult->val[iii] == -1 && iii < row)
                                            {
                                                op_mult->val[iii] = op_mult->val[iii - 1];
                                                iii++;
                                            }

                                            break;
                                        }
                                    }
                                }
                            }

                            if(patterns[ch]->newData[row][DIV_PAT_FX(0)] == 0x11) //feedback
                            {
                                DivInstrumentMacro* op_fb = NULL;

                                if(instr_s.connect <= 3) //one 2-op or 4-op instrument
                                {
                                    op_fb = &ins->std.fbMacro;
                                }
                                if(instr_s.connect > 3) //two 2-op instruments
                                {
                                    if(patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] & 0x10)
                                    {
                                        op_fb = &ins2->std.fbMacro;
                                        patterns[ch]->newData[row][DIV_PAT_FXVAL(0)] &= ~0x10;
                                    }
                                    else
                                    {
                                        op_fb = &ins->std.fbMacro;
                                    }
                                }

                                if(op_fb != NULL)
                                {
                                    op_fb->speed = instr_s.riff_def_speed;

                                    if(op_fb->len == 0)
                                    {
                                        for(int iii = 0; iii < 256; iii++)
                                        {
                                            op_fb->val[iii] = -1;
                                        }
                                    }

                                    op_fb->len = row + 1;
                                    op_fb->val[row] = patterns[ch]->newData[row][DIV_PAT_FXVAL(0)];

                                    for(int iii = 1; iii < 256; iii++)
                                    {
                                        if(op_fb->val[iii] == -1 && op_fb->val[iii - 1] >= 0)
                                        {
                                            while(op_fb->val[iii] == -1 && iii < row)
                                            {
                                                op_fb->val[iii] = op_fb->val[iii - 1];
                                                iii++;
                                            }

                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    end:;

                    if(end_pitch_slide)
                    {
                        ins->std.pitchMacro.loop = ins->std.pitchMacro.len - 1;
                    }

                    if(ins->std.fbMacro.len > 0)
                    {
                        for(int ii = 0; ii < ins->std.fbMacro.len; ii++)
                        {
                            if(ins->std.fbMacro.val[ii] == -1)
                            {
                                ins->std.fbMacro.val[ii] = ins->fm.fb;
                            }
                        }
                    }

                    for(int ii = 0; ii < ins->fm.ops; ii++)
                    {
                        if(ins->std.opMacros[ii].tlMacro.len > 0)
                        {
                            for(int j = 0; j < ins->std.opMacros[ii].tlMacro.len; j++)
                            {
                                if(ins->std.opMacros[ii].tlMacro.val[j] == -1)
                                {
                                    ins->std.opMacros[ii].tlMacro.val[j] = ins->fm.op[ii].tl;
                                }
                            }
                        }

                        if(ins->std.opMacros[ii].multMacro.len > 0)
                        {
                            for(int j = 0; j < ins->std.opMacros[ii].multMacro.len; j++)
                            {
                                if(ins->std.opMacros[ii].multMacro.val[j] == -1)
                                {
                                    ins->std.opMacros[ii].multMacro.val[j] = ins->fm.op[ii].mult;
                                }
                            }
                        }
                    }

                    if(instr_s.connect > 3) //two 2-op instruments, copy macro(s) to second one
                    {
                        if(ins->std.pitchMacro.len > 0)
                        {
                            memcpy(&ins2->std.pitchMacro, &ins->std.pitchMacro, sizeof(DivInstrumentMacro));
                        }

                        if(ins2->std.fbMacro.len > 0)
                        {
                            for(int ii = 0; ii < ins2->std.fbMacro.len; ii++)
                            {
                                if(ins2->std.fbMacro.val[ii] == -1)
                                {
                                    ins2->std.fbMacro.val[ii] = ins2->fm.fb;
                                }
                            }
                        }

                        for(int ii = 0; ii < ins2->fm.ops; ii++)
                        {
                            if(ins2->std.opMacros[ii].tlMacro.len > 0)
                            {
                                for(int j = 0; j < ins2->std.opMacros[ii].tlMacro.len; j++)
                                {
                                    if(ins2->std.opMacros[ii].tlMacro.val[j] == -1)
                                    {
                                        ins2->std.opMacros[ii].tlMacro.val[j] = ins2->fm.op[ii].tl;
                                    }
                                }
                            }

                            if(ins2->std.opMacros[ii].multMacro.len > 0)
                            {
                                for(int j = 0; j < ins2->std.opMacros[ii].multMacro.len; j++)
                                {
                                    if(ins2->std.opMacros[ii].multMacro.val[j] == -1)
                                    {
                                        ins2->std.opMacros[ii].multMacro.val[j] = ins2->fm.op[ii].mult;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if(reader.size() == reader.tell() + 1) 
        {
            logE("file too short!");
            lastError = _("file too short!");
            delete[] file;
            return false;
        }

        //orders format is the same for old and new RAD
        unsigned char order_len = reader.readC();

        if(order_len == 0 || order_len > 128)
        {
            logE("invalid order length!");
            lastError="invalid order length";
            delete[] file;
            return false;
        }

        ds.subsong[0]->ordersLen = order_len;

        bool has_jump_marker = false;
        int jump_marker_order = -1;
        int jump_dest = -1;

        for(int i = 0; i < order_len; i++)
        {
            if(reader.size() == reader.tell() + 1) 
            {
                logE("file too short!");
                lastError = _("file too short!");
                delete[] file;
                return false;
            }
            unsigned char order = reader.readC();

            //TODO: decrease orders length on jump marker and place 0Bxx effect there

            if(order <= ((shifted_version == 2) ? 0x7F : 0x1F))
            {
                for (int j = 0; j < 9; j++) 
                {
                    if(shifted_version == 1)
                    {
                        ds.subsong[0]->orders.ord[j][i] = order;
                    }
                    if(shifted_version == 2)
                    {
                        ds.subsong[0]->orders.ord[j * 2][i] = order;
                        ds.subsong[0]->orders.ord[j * 2 + 1][i] = order;
                    }
                }
            }
            else
            {
                has_jump_marker = true;
                jump_dest = order - 0x80;
                jump_marker_order = i;
            }
        }

        if(shifted_version == 1) //old RAD
        {
            unsigned short pat_offsets[32] = { 0 };

            if(reader.size() - reader.tell() < 32 * sizeof(pat_offsets[0])) 
            {
                logE("file too short!");
                lastError = _("file too short!");
                delete[] file;
                return false;
            }

            reader.read(pat_offsets, 32 * sizeof(pat_offsets[0]));

            //size_t reader_pos = reader.tell();

            RAD_pattern* pat = new RAD_pattern;
            memset((void*)pat, 0, sizeof(RAD_pattern));

            for(int i = 0; i < 32; i++)
            {
                if(pat_offsets[i] != 0) //old pattern format, not separated into function because v1 RAD doesn't have riffs
                {
                    reader.seek(pat_offsets[i], SEEK_SET);

                    DivPattern* patterns[9];

                    for (int j = 0; j < 9; j++)
                    {
                        patterns[j] = s->pat[j].getPattern(i, true);
                    }

                    while(1)
                    {
                        if(reader.size() == reader.tell() + 1) 
                        {
                            logE("file too short!");
                            lastError = _("file too short!");
                            delete[] file;
                            return false;
                        }
                        unsigned char buff = reader.readC();

                        unsigned char line_number = buff & 0x7F;

                        while(1)
                        {
                            if(reader.size() - reader.tell() < 3) 
                            {
                                logE("file too short!");
                                lastError = _("file too short!");
                                delete[] file;
                                return false;
                            }
                            unsigned char channel = reader.readC();

                            unsigned char note = reader.readC();
                            unsigned char effect = reader.readC();

                            unsigned char effect_param = 0;

                            if((effect & 0xF) != 0)
                            {
                                effect_param = reader.readC();
                            }
                            
                            if((note & 0xF) > 0 && (note & 0xF) <= 12)
                            {
                                patterns[(channel & 0x7F) % 9]->newData[line_number][DIV_PAT_NOTE] = (note & 0xF) + ((note >> 4) & 7) * 12 + 60;
                                //patterns[(channel & 0x7F) % 9]->newData[line_number][1] = (note >> 4) & 7; //octave
                            }
                            if((note & 0xF) == 0xF)
                            {
                                patterns[(channel & 0x7F) % 9]->newData[line_number][DIV_PAT_NOTE] = DIV_NOTE_REL; //key off
                            }

                            unsigned char instrument = ((note >> 7) << 4) | (effect >> 4);

                            if(instrument != 0)
                            {
                                patterns[(channel & 0x7F) % 9]->newData[line_number][DIV_PAT_INS] = instrument;
                            }

                            if((effect & 0xF) != 0)
                            {
                                pat->step[(channel & 0x7F) % 9][line_number].effect = effect & 0xF;
                                pat->step[(channel & 0x7F) % 9][line_number].effect_param = effect_param;

                                RAD_convert_effect(patterns[(channel & 0x7F) % 9], pat, (channel & 0x7F) % 9, line_number);
                            }

                            if(channel & (1 << 7)) break;
                        }

                        if(buff & (1 << 7)) break;
                    }
                }
            }

            delete pat;
        }

        if(shifted_version == 2) //new RAD
        {
            while(1)
            {
                unsigned char pat_number = reader.readC();
                
                if(pat_number == 0xFF) break;

                RAD_pattern* pat = new RAD_pattern;
                memset((void*)pat, 0, sizeof(RAD_pattern));

                DivPattern* patterns[9];

                for(int i = 0; i < 9; i++)
                {
                    patterns[i] = s->pat[i * 2].getPattern(pat_number, true);

                    DivPattern* preallocate = s->pat[i * 2 + 1].getPattern(pat_number, true); //if you remove this then the 2+2-op instruments import involving pattern data duplication will stop working
                    (void)preallocate;
                }

                bool pat_read = RAD_read_pattern(&reader, pat, patterns, s, shifted_version, false);

                if(!pat_read)
                {
                    lastError = _("Incomplete pattern data!");
                    delete[] file;
                    delete pat;
                    return false;
                }

                delete pat;
            }

            //riffs
            unsigned char riff_n = 0xFF;
            DivSubSong* riff_subsong;

            while(1)
            {
                int bytes_read = 0;

                if(reader.size() == reader.tell()) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                unsigned char riff_number = reader.readC();
                
                if(riff_number == 0xFF) break;

                unsigned char channel = (riff_number & 0xF) - 1;

                if(riff_n != (riff_number >> 4))
                {
                    ds.subsong.push_back(new DivSubSong);

                    riff_subsong = ds.subsong[riff_subsong_index];

                    riff_subsong->name = fmt::sprintf(_("Riff #%d"), (riff_number >> 4));

                    riff_n = (riff_number >> 4);

                    riff_subsong_index++;
                }

                if(reader.size() == reader.tell() + 1) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                unsigned short len = (unsigned char)reader.readC();

                if(reader.size() == reader.tell() + 1) 
                {
                    logE("file too short!");
                    lastError = _("file too short!");
                    delete[] file;
                    return false;
                }

                len |= ((unsigned char)reader.readC() << 8);

                RAD_pattern* pat = new RAD_pattern;
                memset((void*)pat, 0, sizeof(RAD_pattern));

                while(1)
                {
                    if(reader.size() == reader.tell() + 1) 
                    {
                        logE("file too short!");
                        lastError = _("file too short!");
                        delete[] file;
                        return false;
                    }

                    unsigned char line = reader.readC();
                    bytes_read++;

                    if(bytes_read >= len) break;

                    unsigned char line_number = line & 0x7F;

                    if(reader.size() == reader.tell() + 1) 
                    {
                        logE("file too short!");
                        lastError = _("file too short!");
                        delete[] file;
                        return false;
                    }
                
                    unsigned char line_info = reader.readC();
                    bytes_read++;

                    if(bytes_read >= len) break;

                    if(line_info & (1 << 6)) //note/octave byte
                    {
                        if(reader.size() == reader.tell() + 1) 
                        {
                            logE("file too short!");
                            lastError = _("file too short!");
                            delete[] file;
                            return false;
                        }

                        unsigned char byte = reader.readC();
                        bytes_read++;

                        pat->step[channel % 9][line_number].note = byte & 0xF;
                        pat->step[channel % 9][line_number].octave = (byte >> 4) & 0x7;
                        pat->step[channel % 9][line_number].prev_inst = (byte >> 7) & 0x1;

                        if(bytes_read >= len) break;
                    }

                    if(line_info & (1 << 5)) //instrument byte
                    {
                        if(reader.size() == reader.tell() + 1) 
                        {
                            logE("file too short!");
                            lastError = _("file too short!");
                            delete[] file;
                            return false;
                        }

                        unsigned char byte = reader.readC();
                        bytes_read++;

                        if((byte & 0x7F) > 0)
                        {
                            pat->step[channel % 9][line_number].instrument = (byte & 0x7F) * 2;
                        }

                        if(bytes_read >= len) break;
                    }

                    if(line_info & (1 << 4)) //effect/param bytes
                    {
                        if(reader.size() == reader.tell() + 1) 
                        {
                            logE("file too short!");
                            lastError = _("file too short!");
                            delete[] file;
                            return false;
                        }

                        unsigned char byte = reader.readC();
                        bytes_read++;

                        pat->step[channel % 9][line_number].effect = byte & 0x1F;

                        pat->step[channel % 9][line_number].has_effect = true;

                        if(bytes_read >= len) break;

                        if(reader.size() == reader.tell() + 1) 
                        {
                            logE("file too short!");
                            lastError = _("file too short!");
                            delete[] file;
                            return false;
                        }

                        byte = reader.readC();
                        bytes_read++;

                        pat->step[channel % 9][line_number].effect_param = byte & 0x7F;

                        if(bytes_read >= len) break;
                    }

                    if(line & (1 << 7)) break;
                }

                DivPattern* furnace_pat = riff_subsong->pat[channel].getPattern(0, true);

                for(int j = 0; j < 64; j++)
                {
                    if(pat->step[channel][j].note == 0xF)
                    {
                        furnace_pat->newData[j][DIV_PAT_NOTE] = DIV_NOTE_REL; //key off
                    }
                    else if(pat->step[channel][j].note > 0)
                    {
                        if((version >> 4) == 1)
                        {
                            furnace_pat->newData[j][DIV_PAT_NOTE] = pat->step[channel][j].note - 1 + pat->step[channel][j].octave * 12 + 60;
                            //furnace_pat->newData[j][1] = pat->step[channel][j].octave;
                        }
                        if((version >> 4) == 2)
                        {
                            if(pat->step[channel][j].note == 12)
                            {
                                furnace_pat->newData[j][DIV_PAT_NOTE] = 12 + pat->step[channel][j].octave * 12 + 60;
                                //furnace_pat->newData[j][1] = pat->step[channel][j].octave;
                            }
                            else
                            {
                                furnace_pat->newData[j][DIV_PAT_NOTE] = pat->step[channel][j].note + pat->step[channel][j].octave * 12 + 60;
                                //furnace_pat->newData[j][1] = pat->step[channel][j].octave;
                            }
                        }
                    }

                    if(pat->step[channel][j].instrument > 0)
                    {
                        furnace_pat->newData[j][DIV_PAT_INS] = pat->step[channel][j].instrument;
                    }

                    if(pat->step[channel][j].has_effect)
                    {
                        RAD_convert_effect(furnace_pat, pat, channel, j);
                    }
                }

                delete pat;
            }
        }

        ds.insLen = ds.ins.size();

        s->makePatUnique(); //needed for non-continuous to continuous effects conversion

        //adapt jump marker
        if(has_jump_marker && jump_marker_order == s->ordersLen - 1 && jump_marker_order > 0)
        {
            DivPattern* pat = s->pat[0].getPattern(s->orders.ord[0][jump_marker_order - 1], false);

            int line = s->patLen - 1;

            for(int i = 0; i < ((shifted_version == 2) ? 18 : 9); i++)
            {
                DivPattern* search = s->pat[0].getPattern(s->orders.ord[i][jump_marker_order - 1], false);

                for(int r = 0; r < s->patLen; r++)
                {
                    if(search->newData[r][DIV_PAT_FX(0)] == 0x0D)
                    {
                        line = r;
                        goto next;
                    }
                }
            }

            next:;

            int fx = findEmptyEffectSlot(pat->newData[line]);
            pat->newData[line][DIV_PAT_FX(fx)] = 0x0B;
            pat->newData[line][DIV_PAT_FXVAL(fx)] = jump_dest;

            s->ordersLen--;
        }

        s->rearrangePatterns();

        s->patLen = 64;

        if(shifted_version == 2)
        {
            for (int ii = 0; ii < 18; ii += 2) //if we have 2+2-op instrument we copy pattern data to adjacent channel and put appropriate instrument there
            {
                bool currently_using_twotwoop_inst = false;

                for (int k = 0; k < s->ordersLen; k++) 
                {
                    DivPattern* pat1 = s->pat[ii].getPattern(s->orders.ord[ii][k], false);
                    DivPattern* pat2 = s->pat[ii + 1].getPattern(s->orders.ord[ii + 1][k], false);

                    if(pat1 == NULL || pat2 == NULL)
                    {
                        logE("error getting pattern data!");
                        lastError="error getting pattern data!";
                        delete[] file;
                        return false;
                    }

                    for (int l = 0; l < s->patLen; l++) 
                    {
                        if (pat1->newData[l][DIV_PAT_INS] != -1 && pat1->newData[l][DIV_PAT_INS] > 0 && pat1->newData[l][DIV_PAT_INS] < 257) 
                        {
                            if(two_twoop_inst[pat1->newData[l][DIV_PAT_INS]])
                            {
                                currently_using_twotwoop_inst = true;
                            }
                            else
                            {
                                currently_using_twotwoop_inst = false;
                            }
                        }

                        if(currently_using_twotwoop_inst)
                        {
                            memcpy((void*)pat2->newData[l], (void*)pat1->newData[l], sizeof(pat1->newData[0]));

                            if(pat1->newData[l][DIV_PAT_INS] != -1)
                            {
                                pat2->newData[l][DIV_PAT_INS]++; //increment inst number so we have second-pair-of-two-2op-instruments inst there
                            }

                            if(pat1->newData[l][DIV_PAT_FX(0)] == 0x0B) //erase jump effect from copy
                            {
                                pat2->newData[l][DIV_PAT_FX(0)] = -1;
                                pat2->newData[l][DIV_PAT_FXVAL(0)] = -1;
                            }
                            if(pat1->newData[l][DIV_PAT_FX(1)] == 0x0B)
                            {
                                pat2->newData[l][DIV_PAT_FX(1)] = -1;
                                pat2->newData[l][DIV_PAT_FXVAL(1)] = -1;
                            }
                        }
                    }
                }
            }
        }
        
        //convert non-continuous effects to continuous
        for(int subs = 0; subs < (int)ds.subsong.size(); subs++)
        {
            DivSubSong* ss = ds.subsong[subs];

            for(int c = 0; c < ((shifted_version == 2) ? 18 : 9); c++)
            {
                int porta_dir[2] = { 0 };
                int vol_slide_dir[2] = { 0 };

                int porta_speed = -1;
                int vol_slide_speed = -1;

                bool porta[2] = { false };
                bool vol_slide[2] = { false };

                int curr_volume = 0;
                
                for(int p = 0; p < ss->ordersLen; p++)
                {
                    start_patt:;

                    for(int r = 0; r < ss->patLen; r++)
                    {
                        start_row:;

                        DivPattern* pat = ss->pat[c].getPattern(ss->orders.ord[c][p], true);

                        short* row_data = pat->newData[r];

                        porta[0] = false;
                        vol_slide[0] = false;

                        for(int eff = 0; eff < DIV_MAX_EFFECTS - 1; eff++)
                        {
                            short effect = row_data[DIV_PAT_FX(eff)];
                            short param = row_data[DIV_PAT_FXVAL(eff)];

                            if(effect == 0x01 || effect == 0x02)
                            {
                                porta[0] = true;
                                porta_dir[0] = effect == 0x01 ? 1 : -1;

                                if(porta_speed == param && (porta_dir[0] == porta_dir[1]))
                                {
                                    row_data[DIV_PAT_FX(eff)] = -1;
                                    row_data[DIV_PAT_FXVAL(eff)] = -1; //delete effect
                                }

                                porta_speed = param;
                            }
                            if(effect == 0x03 && param == 0)
                            {
                                row_data[DIV_PAT_FX(eff)] = -1;
                                row_data[DIV_PAT_FXVAL(eff)] = -1; //delete effect
                            }
                            if(effect == 0x0A)
                            {
                                vol_slide[0] = true;

                                if(vol_slide_speed == param)
                                {
                                    row_data[DIV_PAT_FX(eff)] = -1;
                                    row_data[DIV_PAT_FXVAL(eff)] = -1; //delete effect
                                }

                                vol_slide_speed = param;

                                curr_volume = -1; //signal change in volume so on next note volume command is placed
                            }
                        }

                        if(row_data[DIV_PAT_VOL] != -1)
                        {
                            curr_volume = row_data[DIV_PAT_VOL];
                        }

                        if(row_data[DIV_PAT_NOTE] > 0 && row_data[DIV_PAT_NOTE] < 176) //actual note
                        {
                            if(curr_volume != 0x3F && row_data[DIV_PAT_VOL] == -1)
                            {
                                row_data[DIV_PAT_VOL] = 0x3F;
                                curr_volume = 0x3F;
                            }
                        }

                        if(!porta[0] && porta[1] && row_data[DIV_PAT_FX(0)] != 0x03) //place 0200 style effect to end the effect
                        {
                            int emptyEffSlot = findEmptyEffectSlot(row_data);

                            row_data[DIV_PAT_FX(emptyEffSlot)] = 0x01;
                            row_data[DIV_PAT_FXVAL(emptyEffSlot)] = 0;

                            porta_speed = -1;
                        }

                        if(!vol_slide[0] && vol_slide[1])
                        {
                            int emptyEffSlot = findEmptyEffectSlot(row_data);

                            row_data[DIV_PAT_FX(emptyEffSlot)] = 0x0A;
                            row_data[DIV_PAT_FXVAL(emptyEffSlot)] = 0;

                            vol_slide_speed = -1;
                        }

                        porta_dir[1] = porta_dir[0];
                        vol_slide_dir[1] = vol_slide_dir[0];

                        porta[1] = porta[0];
                        vol_slide[1] = vol_slide[0];

                        for(int s_ch = 0; s_ch < ((shifted_version == 2) ? 18 : 9); s_ch++) //search for 0Dxx/0Bxx and jump accordingly
                        {
                            DivPattern* s_pat = s->pat[s_ch].getPattern(p, true);
                            short* s_row_data = s_pat->newData[r];

                            for(int eff = 0; eff < DIV_MAX_EFFECTS - 1; eff++)
                            {
                                if(s_row_data[DIV_PAT_FX(eff)] == 0x0B && s_row_data[DIV_PAT_FXVAL(eff)] > p) //so we aren't stuck in infinite loop
                                {
                                    p = s_row_data[DIV_PAT_FXVAL(eff)];
                                    goto start_patt;
                                }
                                if(s_row_data[DIV_PAT_FX(eff)] == 0x0D && p < s->ordersLen - 1)
                                {
                                    p++;
                                    r = s_row_data[DIV_PAT_FXVAL(eff)];

                                    goto start_row;
                                }
                            }
                        }
                    }
                }
            }
        }
        

        s->optimizePatterns(); //if after converting effects we still have some duplicates
        s->rearrangePatterns();

        // open hidden effect columns
        for (int c = 0; c < ((shifted_version == 2) ? 18 : 9); c++) 
        {
            int num_fx = 1;

            for (int p = 0; p < s->ordersLen; p++) 
            {
                for (int r = 0; r < s->patLen; r++) 
                {
                    DivPattern* pat = s->pat[c].getPattern(s->orders.ord[c][p], true);
                    short* s_row_data = pat->newData[r];

                    for (int eff = 0; eff < DIV_MAX_EFFECTS - 1; eff++) 
                    {
                        if (s_row_data[DIV_PAT_FX(eff)] != -1 && eff + 1 > num_fx) 
                        {
                            num_fx = eff + 1;
                        }
                    }
                }
            }

            s->pat[c].effectCols = num_fx;
        }

        if (ds.insLen > 0) 
        {
            for (int tries = 0; tries < 5; tries++) // erase unused instruments
            {
                for (int i = 0; i < ((shifted_version == 1) ? 31 : 258); i++) 
                {
                    if (ds.ins.empty()) break;

                    int index = i >= (int)ds.insLen ? ((int)ds.insLen - 1) : i;

                    if (index < 0) index = 0;

                    if (index >= (int)ds.ins.size()) continue;

                    DivInstrument* ins = ds.ins[index];

                    if (ins->type == DIV_INS_FM) 
                    {
                        delete ds.ins[index];
                        ds.ins.erase(ds.ins.begin() + index);
                        ds.insLen = ds.ins.size();

                        for (int ii = 0; ii < 18; ii++) 
                        {
                            for (int j = 0; j < (int)ds.subsong.size(); j++)
                            {
                                for (int k = 0; k < DIV_MAX_PATTERNS; k++) 
                                {
                                    if (ds.subsong[j]->pat[ii].data[k] == NULL) continue;
                                    
                                    for (int l = 0; l < ds.subsong[j]->patLen; l++) 
                                    {
                                        if (ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS] > index)
                                        {
                                            ds.subsong[j]->pat[ii].data[k]->newData[l][DIV_PAT_INS]--;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ds.insLen = ds.ins.size();

        ds.linearPitch = 0;
        ds.pitchMacroIsLinear = false;
        ds.pitchSlideSpeed=8;

        for(int i = 0; i < (int)ds.subsong.size(); i++)
        {
            ds.subsong[i]->hz = 50;
        }

        s->name = _("Main song");

        ds.name = _("Main song");
        ds.author = _("Imported from Reality Adlib Tracker module");

        if(shifted_version == 2)
        {
            ds.category = _("Check subsongs list for instrument-specific and global riffs!");
        }

        ds.subsong.erase(ds.subsong.end() - 1);

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
