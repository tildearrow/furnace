/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "qsound.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>
#include <map>

#define CHIP_DIVIDER (1248*2)
#define QS_NOTE_FREQUENCY(x) parent->calcBaseFreq(440,4096,(x)-3,false)

#define rWrite(a,v) {if(!skipRegisterWrites) {qsound_write_data(&chip,a,v); if(dumpWrites) addWrite(a,v); }}
#define immWrite(a,v) {qsound_write_data(&chip,a,v); if(dumpWrites) addWrite(a,v);}

const char* regCheatSheetQSound[]={
  "Ch15_Bank",    "00",
  "Ch00_Start",    "01",
  "Ch00_Freq",    "02",
  "Ch00_Phase",    "03",
  "Ch00_Loop",    "04",
  "Ch00_End",        "05",
  "Ch00_Volume",    "06",
  "Ch00_Bank",    "08",
  "Ch01_Start",    "09",
  "Ch01_Freq",    "0A",
  "Ch01_Phase",    "0B",
  "Ch01_Loop",    "0C",
  "Ch01_End",        "0D",
  "Ch01_Volume",    "0E",
  "Ch01_Bank",    "10",
  "Ch02_Start",    "11",
  "Ch02_Freq",    "12",
  "Ch02_Phase",    "13",
  "Ch02_Loop",    "14",
  "Ch02_End",        "15",
  "Ch02_Volume",    "16",
  "Ch02_Bank",    "18",
  "Ch03_Start",    "19",
  "Ch03_Freq",    "1A",
  "Ch03_Phase",    "1B",
  "Ch03_Loop",    "1C",
  "Ch03_End",        "1D",
  "Ch03_Volume",    "1E",
  "Ch03_Bank",    "20",
  "Ch04_Start",    "21",
  "Ch04_Freq",    "22",
  "Ch04_Phase",    "23",
  "Ch04_Loop",    "24",
  "Ch04_End",        "25",
  "Ch04_Volume",    "26",
  "Ch04_Bank",    "28",
  "Ch05_Start",    "29",
  "Ch05_Freq",    "2A",
  "Ch05_Phase",    "2B",
  "Ch05_Loop",    "2C",
  "Ch05_End",        "2D",
  "Ch05_Volume",    "2E",
  "Ch05_Bank",    "30",
  "Ch06_Start",    "31",
  "Ch06_Freq",    "32",
  "Ch06_Phase",    "33",
  "Ch06_Loop",    "34",
  "Ch06_End",        "35",
  "Ch06_Volume",    "36",
  "Ch06_Bank",    "38",
  "Ch07_Start",    "39",
  "Ch07_Freq",    "3A",
  "Ch07_Phase",    "3B",
  "Ch07_Loop",    "3C",
  "Ch07_End",        "3D",
  "Ch07_Volume",    "3E",
  "Ch07_Bank",    "40",
  "Ch08_Start",    "41",
  "Ch08_Freq",    "42",
  "Ch08_Phase",    "43",
  "Ch08_Loop",    "44",
  "Ch08_End",        "45",
  "Ch08_Volume",    "46",
  "Ch08_Bank",    "48",
  "Ch09_Start",    "49",
  "Ch09_Freq",    "4A",
  "Ch09_Phase",    "4B",
  "Ch09_Loop",    "4C",
  "Ch09_End",        "4D",
  "Ch09_Volume",    "4E",
  "Ch09_Bank",    "50",
  "Ch10_Start",    "51",
  "Ch10_Freq",    "52",
  "Ch10_Phase",    "53",
  "Ch10_Loop",    "54",
  "Ch10_End",        "55",
  "Ch10_Volume",    "56",
  "Ch10_Bank",    "58",
  "Ch11_Start",    "59",
  "Ch11_Freq",    "5A",
  "Ch11_Phase",    "5B",
  "Ch11_Loop",    "5C",
  "Ch11_End",        "5D",
  "Ch11_Volume",    "5E",
  "Ch11_Bank",    "60",
  "Ch12_Start",    "61",
  "Ch12_Freq",    "62",
  "Ch12_Phase",    "63",
  "Ch12_Loop",    "64",
  "Ch12_End",        "65",
  "Ch12_Volume",    "66",
  "Ch12_Bank",    "68",
  "Ch13_Start",    "69",
  "Ch13_Freq",    "6A",
  "Ch13_Phase",    "6B",
  "Ch13_Loop",    "6C",
  "Ch13_End",        "6D",
  "Ch13_Volume",    "6E",
  "Ch13_Bank",    "70",
  "Ch14_Start",    "71",
  "Ch14_Freq",    "72",
  "Ch14_Phase",    "73",
  "Ch14_Loop",    "74",
  "Ch14_End",        "75",
  "Ch14_Volume",    "76",
  "Ch14_Bank",    "78",
  "Ch15_Start",    "79",
  "Ch15_Freq",    "7A",
  "Ch15_Phase",    "7B",
  "Ch15_Loop",    "7C",
  "Ch15_End",        "7D",
  "Ch15_Volume",    "7E",
  "Ch00_Panning",    "80",
  "Ch01_Panning",    "81",
  "Ch02_Panning",    "82",
  "Ch03_Panning",    "83",
  "Ch04_Panning",    "84",
  "Ch05_Panning",    "85",
  "Ch06_Panning",    "86",
  "Ch07_Panning",    "87",
  "Ch08_Panning",    "88",
  "Ch09_Panning",    "89",
  "Ch10_Panning",    "8A",
  "Ch11_Panning",    "8B",
  "Ch12_Panning",    "8C",
  "Ch13_Panning",    "8D",
  "Ch14_Panning",    "8E",
  "Ch15_Panning",    "8F",
  "Adpcm0_Panning","90",
  "Adpcm1_Panning","91",
  "Adpcm2_Panning","92",
  "Echo_Feedback","93",
  "Ch00_Echo",    "BA",
  "Ch01_Echo",    "BB",
  "Ch02_Echo",    "BC",
  "Ch03_Echo",    "BD",
  "Ch04_Echo",    "BE",
  "Ch05_Echo",    "BF",
  "Ch06_Echo",    "C0",
  "Ch07_Echo",    "C1",
  "Ch08_Echo",    "C2",
  "Ch09_Echo",    "C3",
  "Ch10_Echo",    "C4",
  "Ch11_Echo",    "C5",
  "Ch12_Echo",    "C6",
  "Ch13_Echo",    "C7",
  "Ch14_Echo",    "C8",
  "Ch15_Echo",    "C9",
  "Adpcm0_Start",    "CA",
  "Adpcm0_End",    "CB",
  "Adpcm0_Bank",    "CC",
  "Adpcm0_Volume","CD",
  "Adpcm1_Start",    "CE",
  "Adpcm1_End",    "CF",
  "Adpcm1_Bank",    "D0",
  "Adpcm1_Volume","D1",
  "Adpcm2_Start",    "D2",
  "Adpcm2_End",    "D3",
  "Adpcm2_Bank",    "D4",
  "Adpcm2_Volume","D5",
  "Adpcm0_KeyOn",    "D6",
  "Adpcm1_KeyOn",    "D7",
  "Adpcm2_KeyOn",    "D8",
  "Echo_Delay",    "D9",
  "L_Wet_Filter",    "DA",
  "L_Dry_Filter",    "DB",
  "R_Wet_Filter",    "DC",
  "R_Dry_Filter",    "DD",
  "L_Wet_Delay",    "DE",
  "L_Dry_Delay",    "DF",
  "R_Wet_Delay",    "E0",
  "R_Dry_Delay",    "E1",
  "Delay_Flag",    "E2",
  "Mode_Select",    "E3", //valid: 0000,0288,0039,061A,004F
  "L_Wet_Volume",    "E4",
  "L_Dry_Volume",    "E5",
  "R_Wet_Volume",    "E6",
  "R_Dry_Volume",    "E7",
  NULL
};
enum q1_register_name {
  Q1V_BANK = 0,
  Q1V_START = 1,
  Q1V_FREQ = 2,
  Q1V_PHASE = 3,
  Q1V_LOOP = 4,
  Q1V_END = 5,
  Q1V_VOL = 6,
  Q1V_REG_COUNT = 7,

  Q1_PAN = 0x80,
  Q1_ECHO = 0xba,

  Q1A_PAN = 0x90,
  Q1A_START = 0xca,
  Q1A_END = 0xcb,
  Q1A_BANK = 0xcc,
  Q1A_VOL = 0xcd,

  Q1A_KEYON = 0xd6,

  Q1_ECHO_FEEDBACK = 0x93,
  Q1_ECHO_LENGTH = 0xd9,
};

const unsigned char q1_reg_map[Q1V_REG_COUNT][16] = {
  {0x78,0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,0x40,0x48,0x50,0x58,0x60,0x68,0x70},
  {0x01,0x09,0x11,0x19,0x21,0x29,0x31,0x39,0x41,0x49,0x51,0x59,0x61,0x69,0x71,0x79},
  {0x02,0x0a,0x12,0x1a,0x22,0x2a,0x32,0x3a,0x42,0x4a,0x52,0x5a,0x62,0x6a,0x72,0x7a},
  {0x03,0x0b,0x13,0x1b,0x23,0x2b,0x33,0x3b,0x43,0x4b,0x53,0x5b,0x63,0x6b,0x73,0x7b},
  {0x04,0x0c,0x14,0x1c,0x24,0x2c,0x34,0x3c,0x44,0x4c,0x54,0x5c,0x64,0x6c,0x74,0x7c},
  {0x05,0x0d,0x15,0x1d,0x25,0x2d,0x35,0x3d,0x45,0x4d,0x55,0x5d,0x65,0x6d,0x75,0x7d},
  {0x06,0x0e,0x16,0x1e,0x26,0x2e,0x36,0x3e,0x46,0x4e,0x56,0x5e,0x66,0x6e,0x76,0x7e},
};

const unsigned char q1a_start_map[3]={
  0xca, 0xce, 0xd2
};

const unsigned char q1a_end_map[3]={
  0xcb, 0xcf, 0xd3
};

const unsigned char q1a_bank_map[3]={
  0xcc, 0xd0, 0xd4
};

const unsigned char q1a_vol_map[3]={
  0xcd, 0xd1, 0xd5
};

const char** DivPlatformQSound::getRegisterSheet() {
  return regCheatSheetQSound;
}

void DivPlatformQSound::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    qsound_update(&chip);
    bufL[h]=chip.out[0];
    bufR[h]=chip.out[1];

    for (int i=0; i<19; i++) {
      int data=chip.voice_output[i]<<2;
      if (data<-32768) data=-32768;
      if (data>32767) data=32767;
      oscBuf[i]->data[oscBuf[i]->needle++]=data;
    }
  }
}

void DivPlatformQSound::tick(bool sysTick) {
  for (int i=0; i<19; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      if (chan[i].isNewQSound) {
        chan[i].outVol=((chan[i].vol&0xff)*MIN(16383,chan[i].std.vol.val))/16383;
        chan[i].resVol=((chan[i].vol&0xff)*MIN(16383,chan[i].std.vol.val))/255;
      } else {
        chan[i].outVol=((chan[i].vol&0xff)*chan[i].std.vol.val)>>6;
        chan[i].resVol=chan[i].outVol<<4;
      }
      // Check if enabled and write volume
      if (chan[i].active) {
        if (i<16) {
          rWrite(q1_reg_map[Q1V_VOL][i],chan[i].resVol);
        } else {
          rWrite(q1a_vol_map[i-16],chan[i].resVol);
        }
      }
    }
    uint16_t qsound_bank = 0;
    uint16_t qsound_addr = 0;
    uint16_t qsound_loop = 0;
    uint16_t qsound_end = 0;
    if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[i].sample);
      if (i<16) {
        qsound_bank = 0x8000 | (offPCM[chan[i].sample] >> 16);
        qsound_addr = offPCM[chan[i].sample] & 0xffff;
      } else {
        qsound_bank = 0x8000 | (offBS[chan[i].sample] >> 16);
        qsound_addr = offBS[chan[i].sample] & 0xffff;
      }

      int loopStart=s->loopStart;
      int length = s->loopEnd;
      if (length > 65536 - 16) {
        length = 65536 - 16;
      }
      if (loopStart == -1 || loopStart >= length) {
        if (i<16) {
          qsound_end = offPCM[chan[i].sample] + length + 15;
        } else {
          qsound_end = offBS[chan[i].sample] + (length>>1) + 15;
        }
        qsound_loop = 15;
      } else {
        if (i<16) {
          qsound_end = offPCM[chan[i].sample] + length;
        } else {
          qsound_end = offBS[chan[i].sample] + (length>>1);
        }
        qsound_loop = length - loopStart;
      }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=QS_NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].isNewQSound && chan[i].std.duty.had) {
      chan[i].echo=CLAMP(chan[i].std.duty.val,0,32767);
      if (i<16) {
        immWrite(Q1_ECHO+i,chan[i].echo&0x7fff);
      }
    }
    if (chan[i].isNewQSound && chan[i].std.ex1.had) {
      immWrite(Q1_ECHO_FEEDBACK,chan[i].std.ex1.val&0x3fff);
    }
    if (chan[i].isNewQSound && chan[i].std.ex2.had) {
      immWrite(Q1_ECHO_LENGTH,0xfff-(2725-CLAMP(chan[i].std.ex2.val&0xfff,0,2725)));
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.panL.had) { // panning
      chan[i].panning=chan[i].std.panL.val+16;
    }
    if (chan[i].std.panR.had) { // surround
      chan[i].surround=chan[i].std.panR.val;
    }
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      immWrite(Q1_PAN+i,chan[i].panning+0x110+(chan[i].surround?0:0x30));
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active && (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen)) {
        chan[i].keyOn=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_AMIGA);
      double off=1.0;
      if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[i].sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=(double)s->centerRate/24038.0/16.0;
        }
      }
      chan[i].freq=off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,440.0,4096.0);
      if (chan[i].freq>0xefff) chan[i].freq=0xefff;
      if (chan[i].keyOn) {
        if (i<16) {
          rWrite(q1_reg_map[Q1V_BANK][i], qsound_bank);
          rWrite(q1_reg_map[Q1V_END][i], qsound_end);
          rWrite(q1_reg_map[Q1V_LOOP][i], qsound_loop);
          rWrite(q1_reg_map[Q1V_START][i], qsound_addr);
          rWrite(q1_reg_map[Q1V_PHASE][i], 0x8000);
        } else {
          rWrite(Q1A_KEYON+(i-16),0);
          rWrite(q1a_bank_map[i-16], qsound_bank);
          rWrite(q1a_end_map[i-16], qsound_end);
          rWrite(q1a_start_map[i-16], qsound_addr);
          rWrite(Q1A_KEYON+(i-16),1);
        }
        //logV("ch %d bank=%04x, addr=%04x, end=%04x, loop=%04x!",i,qsound_bank,qsound_addr,qsound_end,qsound_loop);
        // Write sample address. Enable volume
        if (!chan[i].std.vol.had) {
          if (chan[i].isNewQSound) {
            chan[i].resVol=(chan[i].vol*16383)/255;
          } else {
            chan[i].resVol=chan[i].vol<<4;
          }
          if (i<16) {
            rWrite(q1_reg_map[Q1V_VOL][i],chan[i].resVol);
          } else {
            rWrite(q1a_vol_map[i-16],chan[i].resVol);
          }
        }
      }
      if (chan[i].keyOff) {
        // Disable volume
        if (i<16) {
          rWrite(q1_reg_map[Q1V_VOL][i],0);
          rWrite(q1_reg_map[Q1V_FREQ][i],0);
        } else {
          rWrite(q1a_vol_map[i-16],0);
          rWrite(Q1A_KEYON+(i-16),0);
          rWrite(q1a_end_map[i-16], 1);
          rWrite(q1a_start_map[i-16], 0);
          rWrite(Q1A_KEYON+(i-16),1);
        }
      } else if (chan[i].active) {
        //logV("ch %d frequency set to %04x, off=%f, note=%d, %04x!",i,chan[i].freq,off,chan[i].note,QS_NOTE_FREQUENCY(chan[i].note));
        if (i<16) {
          rWrite(q1_reg_map[Q1V_FREQ][i],chan[i].freq);
        }
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformQSound::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].isNewQSound=(ins->type==DIV_INS_QSOUND);
      if (c.value!=DIV_NOTE_NULL) chan[c.chan].sample=ins->amiga.getSample(c.value);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=QS_NOTE_FREQUENCY(c.value);
      }
      if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
        if (chan[c.chan].isNewQSound) {
          chan[c.chan].resVol=(chan[c.chan].outVol*16383)/255;
        } else {
          chan[c.chan].resVol=chan[c.chan].outVol<<4;
        }
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          // Check if enabled and write volume
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].isNewQSound) {
            chan[c.chan].resVol=(chan[c.chan].outVol*16383)/255;
          } else {
            chan[c.chan].resVol=chan[c.chan].outVol<<4;
          }
          if (chan[c.chan].active) {
            if (c.chan<16) {
              rWrite(q1_reg_map[Q1V_VOL][c.chan],chan[c.chan].resVol);
            } else {
              rWrite(q1a_vol_map[c.chan-16],chan[c.chan].resVol);
            }
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].panning=parent->convertPanSplitToLinearLR(c.value,c.value2,32);
      immWrite(Q1_PAN+c.chan,chan[c.chan].panning+0x110+(chan[c.chan].surround?0:0x30));
      break;
    case DIV_CMD_QSOUND_ECHO_LEVEL:
      chan[c.chan].echo=c.value<<7;
      if (c.chan>=16) break;
      immWrite(Q1_ECHO+c.chan,chan[c.chan].echo&0x7fff);
      break;
    case DIV_CMD_QSOUND_ECHO_FEEDBACK:
      immWrite(Q1_ECHO_FEEDBACK, c.value << 6);
      break;
    case DIV_CMD_QSOUND_ECHO_DELAY:
      immWrite(Q1_ECHO_LENGTH, (c.value > 2725 ? 0xfff : 0xfff - (2725 - c.value)));
      break;
    case DIV_CMD_QSOUND_SURROUND:
      chan[c.chan].surround=c.value;
      immWrite(Q1_PAN+c.chan,chan[c.chan].panning+0x110+(chan[c.chan].surround?0:0x30));
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=QS_NOTE_FREQUENCY(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=QS_NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=QS_NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 255;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformQSound::muteChannel(int ch, bool mute) {
  if (mute) {
    chip.mute_mask|=(1<<ch);
  } else {
    chip.mute_mask&=~(1<<ch);
  }
}

void DivPlatformQSound::forceIns() {
  // TODO: what?
  for (int i=0; i<19; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;
  }
}

void* DivPlatformQSound::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformQSound::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformQSound::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformQSound::reset() {
  for (int i=0; i<19; i++) {
    chan[i]=DivPlatformQSound::Channel();
    chan[i].std.setEngine(parent);
  }
  qsound_reset(&chip);
  while(!chip.ready_flag) {
    qsound_update(&chip);
  }

  immWrite(Q1_ECHO_LENGTH, 0xfff - (2725 - echoDelay));
  immWrite(Q1_ECHO_FEEDBACK, echoFeedback << 6);
}

bool DivPlatformQSound::isStereo() {
  return true;
}

bool DivPlatformQSound::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformQSound::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformQSound::notifyWaveChange(int wave) {
  // yeah won't add wavetables
}

void DivPlatformQSound::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformQSound::setFlags(const DivConfig& flags) {
  echoDelay = 2725 - flags.getInt("echoDelay",0);
  echoFeedback = flags.getInt("echoFeedback",0) & 255;

  if (echoDelay < 0) {
    echoDelay = 0;
  }
  if (echoDelay > 2725) {
    echoDelay = 2725;
  }
  //rate=chipClock/CHIP_DIVIDER;
}

void DivPlatformQSound::poke(unsigned int addr, unsigned short val) {
  immWrite(addr, val);
  immWrite(addr, val);
}

void DivPlatformQSound::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

unsigned char* DivPlatformQSound::getRegisterPool() {
  unsigned short* regPoolPtr = regPool;
  for(int i=0; i<256; i++)
  {
      uint16_t data = qsound_read_data(&chip, i);
      *regPoolPtr++ = data;
  }
  return (unsigned char*)regPool;
}

int DivPlatformQSound::getRegisterPoolSize() {
  return 256;
}

int DivPlatformQSound::getRegisterPoolDepth() {
  return 16;
}

const void* DivPlatformQSound::getSampleMem(int index) {
  return (index == 0 || index == 1) ? sampleMem : NULL;
}

size_t DivPlatformQSound::getSampleMemCapacity(int index) {
  return (index == 0 || index == 1) ? 16777216 : 0;
}

size_t DivPlatformQSound::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : index == 1 ? sampleMemLenBS : 0;
}

bool DivPlatformQSound::isSampleLoaded(int index, int sample) {
  if (index<0 || index>1) return false;
  if (sample<0 || sample>255) return false;
  if (index==1) return sampleLoadedBS[sample];
  return sampleLoaded[sample];
}

const char* DivPlatformQSound::getSampleMemName(int index) {
  return index == 0 ? "PCM" : index == 1 ? "ADPCM" : NULL;
}

void DivPlatformQSound::renderSamples(int sysID) {
  memset(sampleMem,0,getSampleMemCapacity());
  memset(sampleLoaded,0,256*sizeof(bool));
  memset(sampleLoadedBS,0,256*sizeof(bool));

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      offPCM[i]=0;
      continue;
    }

    int length=s->length8;
    if (length>65536-16) {
      length=65536-16;
    }
    if ((memPos&0xff0000)!=((memPos+length)&0xff0000)) {
      memPos=(memPos+0xffff)&0xff0000;
    }
    if (memPos>=getSampleMemCapacity()) {
      logW("out of QSound PCM memory for sample %d!",i);
      break;
    }
    if (memPos+length>=getSampleMemCapacity()) {
      for (unsigned int i=0; i<getSampleMemCapacity()-(memPos+length); i++) {
        sampleMem[(memPos+i)^0x8000]=s->data8[i];
      }
      logW("out of QSound PCM memory for sample %d!",i);
    } else {
      for (int i=0; i<length; i++) {
        sampleMem[(memPos+i)^0x8000]=s->data8[i];
      }
      sampleLoaded[i]=true;
    }
    offPCM[i]=memPos^0x8000;
    memPos+=length+16;
  }
  sampleMemLen=memPos+256;

  memPos=(memPos+0xffff)&0xff0000;

  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[1][sysID]) {
      offBS[i]=0;
      continue;
    }

    int length=s->lengthQSoundA;
    if (length>65536) {
      length=65536;
    }
    if ((memPos&0xff0000)!=((memPos+length)&0xff0000)) {
      memPos=(memPos+0xffff)&0xff0000;
    }
    if (memPos>=getSampleMemCapacity()) {
      logW("out of QSound ADPCM memory for sample %d!",i);
      break;
    }
    if (memPos+length>=getSampleMemCapacity()) {
      for (unsigned int i=0; i<getSampleMemCapacity()-(memPos+length); i++) {
        sampleMem[(memPos+i)]=s->dataQSoundA[i];
      }
      logW("out of QSound ADPCM memory for sample %d!",i);
    } else {
      for (int i=0; i<length; i++) {
        sampleMem[(memPos+i)]=s->dataQSoundA[i];
      }
      sampleLoadedBS[i]=true;
    }
    offBS[i]=memPos;
    memPos+=length+16;
  }
  sampleMemLenBS=memPos+256;
}

int DivPlatformQSound::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<19; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    //isMuted[i]=false;
  }
  setFlags(flags);

  chipClock=60000000;
  rate = qsound_start(&chip, chipClock);
  sampleMem=new unsigned char[getSampleMemCapacity()];
  sampleMemLen=0;
  sampleMemLenBS=0;
  chip.rom_data=sampleMem;
  chip.rom_mask=0xffffff;
  reset();

  for (int i=0; i<19; i++) {
    oscBuf[i]->rate=rate;
  }
  return 19;
}

void DivPlatformQSound::quit() {
  delete[] sampleMem;
  for (int i=0; i<19; i++) {
    delete oscBuf[i];
  }
}
