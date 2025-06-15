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

void RAD_convert_effect(DivPattern* furnace_pat, RAD_pattern* pat, int i, int j)
{
    unsigned char chan_to_op_map[9] = { 0, 0, 1, 2, 3, 0, 1, 2, 3 };

    switch(pat->step[i][j].effect)
    {
        case 1:
        case 2:
        case 3:
        {
            furnace_pat->data[j][4] = pat->step[i][j].effect;
            furnace_pat->data[j][5] = pat->step[i][j].effect_param;
            break;
        }
        case 5:
        {
            furnace_pat->data[j][4] = 0x06;

            if(pat->step[i][j].effect_param >= 1 && pat->step[i][j].effect_param <= 49)
            {
                furnace_pat->data[j][5] = (pat->step[i][j].effect_param * 0xF / 49) & 0xF;
            }
            if(pat->step[i][j].effect_param >= 51 && pat->step[i][j].effect_param <= 99)
            {
                furnace_pat->data[j][5] = (((pat->step[i][j].effect_param - 51) * 0xF / (99 - 51)) & 0xF) << 4;
            }
            break;
        }
        case ('A' - 'A' + 10): //TODO: letters or 0xA
        {
            furnace_pat->data[j][4] = 0x0A;

            if(pat->step[i][j].effect_param >= 1 && pat->step[i][j].effect_param <= 49)
            {
                furnace_pat->data[j][5] = (pat->step[i][j].effect_param * 0xF / 49) & 0xF;
            }
            if(pat->step[i][j].effect_param >= 51 && pat->step[i][j].effect_param <= 99)
            {
                furnace_pat->data[j][5] = (((pat->step[i][j].effect_param - 51) * 0xF / (99 - 51)) & 0xF) << 4;
            }
            break;
        }
        case ('C' - 'A' + 10):
        {
            furnace_pat->data[j][3] = ((pat->step[i][j].effect_param > 0x3F) ? 0x3F : pat->step[i][j].effect_param); //just instrument volume
            break;
        }
        case ('D' - 'A' + 10):
        {
            furnace_pat->data[j][4] = 0x0d;
            furnace_pat->data[j][5] = pat->step[i][j].effect_param;
            break;
        }
        case ('F' - 'A' + 10):
        {
            furnace_pat->data[j][4] = 0x0f;
            furnace_pat->data[j][5] = pat->step[i][j].effect_param;
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

                furnace_pat->data[j][4] = 0x16;
                furnace_pat->data[j][5] = (op << 4) | pat->step[i][j].effect_param;
            }
            break;
        }
        case ('R' - 'A' + 10):
        {
            //play global riff
            //convert to non-existent effect just so it hints what should happen there
            furnace_pat->data[j][4] = 0xA0;
            furnace_pat->data[j][5] = pat->step[i][j].effect_param;
            break;
        }
        case ('T' - 'A' + 10):
        {
            //play global transposed riff
            //convert to non-existent effect just so it hints what should happen there
            furnace_pat->data[j][4] = 0xA1;
            furnace_pat->data[j][5] = pat->step[i][j].effect_param;
            break;
        }
        case ('U' - 'A' + 10):
        {
            furnace_pat->data[j][4] = 0x11;
            furnace_pat->data[j][5] = pat->step[i][j].effect_param & 7;
            break;
        }
        case ('V' - 'A' + 10):
        {
            if(i > 0)
            {
                unsigned char op = chan_to_op_map[i]; //same thing as with multiplier but for op's TL

                furnace_pat->data[j][4] = 0x12 + op;
                furnace_pat->data[j][5] = (0x3F - ((pat->step[i][j].effect_param > 0x3F) ? 0x3F : pat->step[i][j].effect_param));
            }
            break;
        }
        default: break;
    }
}

void RAD_read_pattern(SafeReader* reader, RAD_pattern* pat, DivPattern** furnace_patterns, DivSubSong* subsong, int version, bool hide_chans)
{
    unsigned short len = (unsigned char)reader->readC();
    len |= ((unsigned char)reader->readC() << 8);
    
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

    subsong->patLen = max_line_number + 1;

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

        for(int j = 0; j < max_line_number; j++)
        {
            if(pat->step[i][j].note == 0xF)
            {
                furnace_pat->data[j][0] = 101; //key off
            }
            else if(pat->step[i][j].note > 0)
            {
                if(version == 1)
                {
                    furnace_pat->data[j][0] = pat->step[i][j].note - 1;
                    furnace_pat->data[j][1] = pat->step[i][j].octave;
                }
                if(version == 2)
                {
                    if(pat->step[i][j].note == 12)
                    {
                        furnace_pat->data[j][0] = 12;
                        furnace_pat->data[j][1] = pat->step[i][j].octave;
                    }
                    else
                    {
                        furnace_pat->data[j][0] = pat->step[i][j].note;
                        furnace_pat->data[j][1] = pat->step[i][j].octave;
                    }
                }
            }

            if(pat->step[i][j].instrument > 0)
            {
                furnace_pat->data[j][2] = pat->step[i][j].instrument;
            }

            if(pat->step[i][j].has_effect)
            {
                RAD_convert_effect(furnace_pat, pat, i, j);
            }
        }
    }
}

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

            RAD_read_description(&ds, &reader);
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

                unsigned char name_len = reader.readC();
                reader.read((void*)&name, name_len);
                
                DivInstrument* ins = ds.ins[(int)inst_num * 2];
                DivInstrument* ins2 = ds.ins[(int)inst_num * 2 + 1];

                ins->type = DIV_INS_OPL;
                ins->name = name;

                reader.read((void*)&instr_s, sizeof(FM_INST_DATA_new));

                if(instr_s.connect == 7) //MIDI instrument?
                {
                    unsigned char dummy[3];
                    reader.read((void*)&dummy, 3);

                    ins->name += " [MIDI]";
                }
                else
                {
                    ins->fm.fb = instr_s.feedb12; //where do I put feedb34???
                    ins->fm.alg = instr_s.connect;

                    ins->fm.ops = ins->fm.alg > 1 ? 4 : 2;
                    
                    if(ins->fm.alg <= 3)
                    {
                        for(int i = 0; i < 4; i++) //four ops even for 2-op instruments?
                        {
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
                            reader.read((void*)&op_s, sizeof(FM_OP_INST_DATA_new));

                            unsigned char furnace_op = 1 - i;

                            RAD_import_FM_op(ins, furnace_op, &op_s);
                        }

                        for(int i = 0; i < 2; i++)
                        {
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
                    //TODO: how the hell RAD treats 4-op instruments? Looks like for some algorithms 4-op instrument is actually two channels in 2-op mode, that's why in some alg schemes there are two feedbacks?
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

                    RAD_read_pattern(&reader, riff, patterns, riff_subsong, shifted_version, true);

                    riff_subsong_index++;

                    delete riff;
                }
            }
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

        for(int i = 0; i < order_len; i++)
        {
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
        }

        if(shifted_version == 1) //old RAD
        {
            unsigned short pat_offsets[32] = { 0 };

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
                        unsigned char buff = reader.readC();

                        unsigned char line_number = buff & 0x7F;

                        while(1)
                        {
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
                                patterns[(channel & 0x7F) % 9]->data[line_number][0] = (note & 0xF);
                                patterns[(channel & 0x7F) % 9]->data[line_number][1] = (note >> 4) & 7; //octave
                            }
                            if((note & 0xF) == 0xF)
                            {
                                patterns[(channel & 0x7F) % 9]->data[line_number][0] = 101; //key off
                            }

                            unsigned char instrument = ((note >> 7) << 4) | (effect >> 4);

                            if(instrument != 0)
                            {
                                patterns[(channel & 0x7F) % 9]->data[line_number][2] = instrument;
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

                RAD_read_pattern(&reader, pat, patterns, s, shifted_version, false);

                delete pat;
            }

            //riffs
            unsigned char riff_n = 0xFF;
            DivSubSong* riff_subsong;

            while(1)
            {
                int bytes_read = 0;

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

                unsigned short len = (unsigned char)reader.readC();
                len |= ((unsigned char)reader.readC() << 8);

                RAD_pattern* pat = new RAD_pattern;
                memset((void*)pat, 0, sizeof(RAD_pattern));

                while(1)
                {
                    unsigned char line = reader.readC();
                    bytes_read++;

                    if(bytes_read >= len) break;

                    unsigned char line_number = line & 0x7F;
                
                    unsigned char line_info = reader.readC();
                    bytes_read++;

                    if(bytes_read >= len) break;

                    if(line_info & (1 << 6)) //note/octave byte
                    {
                        unsigned char byte = reader.readC();
                        bytes_read++;

                        pat->step[channel % 9][line_number].note = byte & 0xF;
                        pat->step[channel % 9][line_number].octave = (byte >> 4) & 0x7;
                        pat->step[channel % 9][line_number].prev_inst = (byte >> 7) & 0x1;

                        if(bytes_read >= len) break;
                    }

                    if(line_info & (1 << 5)) //instrument byte
                    {
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
                        unsigned char byte = reader.readC();
                        bytes_read++;

                        pat->step[channel % 9][line_number].effect = byte & 0x1F;

                        pat->step[channel % 9][line_number].has_effect = true;

                        if(bytes_read >= len) break;

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
                        furnace_pat->data[j][0] = 101; //key off
                    }
                    else if(pat->step[channel][j].note > 0)
                    {
                        if((version >> 4) == 1)
                        {
                            furnace_pat->data[j][0] = pat->step[channel][j].note - 1;
                            furnace_pat->data[j][1] = pat->step[channel][j].octave;
                        }
                        if((version >> 4) == 2)
                        {
                            if(pat->step[channel][j].note == 12)
                            {
                                furnace_pat->data[j][0] = 12;
                                furnace_pat->data[j][1] = pat->step[channel][j].octave;
                            }
                            else
                            {
                                furnace_pat->data[j][0] = pat->step[channel][j].note;
                                furnace_pat->data[j][1] = pat->step[channel][j].octave;
                            }
                        }
                    }

                    if(pat->step[channel][j].instrument > 0)
                    {
                        furnace_pat->data[j][2] = pat->step[channel][j].instrument;
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
        s->rearrangePatterns();

        s->patLen = 64;

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
                    if (pat1->data[l][2] != -1 && pat1->data[l][2] > 0 && pat1->data[l][2] < 257) 
                    {
                        if(two_twoop_inst[pat1->data[l][2]])
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
                        memcpy((void*)pat2->data[l], (void*)pat1->data[l], sizeof(pat1->data[0]));

                        if(pat1->data[l][2] != -1)
                        {
                            pat2->data[l][2]++; //increment inst number so we have second-pair-of-two-2op-instruments inst there
                        }
                    }
                }
            }
        }

        //todo: convert non-continuous to continuous effects

        //s->optimizePatterns(); //if after converting effects we still have some duplicates
        //s->rearrangePatterns();

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
                                        if (ds.subsong[j]->pat[ii].data[k]->data[l][2] > index)
                                        {
                                            ds.subsong[j]->pat[ii].data[k]->data[l][2]--;
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
