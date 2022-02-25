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
#define QS_NOTE_FREQUENCY(x) parent->calcBaseFreq(440,0x1000,(x)-3,false)

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

const char** DivPlatformQSound::getRegisterSheet() {
  return regCheatSheetQSound;
}

const char* DivPlatformQSound::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Set echo feedback level (00 to FF)";
      break;
    case 0x11:
      return "11xx: Set channel echo level (00 to FF)";
      break;
    default:
      if ((effect & 0xf0) == 0x30) {
        return "3xxx: Set echo delay buffer length (000 to AA5)";
      }
  }
  return NULL;
}
void DivPlatformQSound::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  chip.rom_data = parent->qsoundMem;
  chip.rom_mask = 0xffffff;
  for (size_t h=start; h<start+len; h++) {
    qsound_update(&chip);
    bufL[h]=chip.out[0];
    bufR[h]=chip.out[1];
  }
}

void DivPlatformQSound::tick() {
  for (int i=0; i<16; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=((chan[i].vol&0xff)*MIN(255,chan[i].std.vol<<2))>>8;
      // Check if enabled and write volume
      if (chan[i].active) {
        rWrite(q1_reg_map[Q1V_VOL][i], chan[i].outVol << 4);
      }
    }
    uint16_t qsound_bank = 0;
    uint16_t qsound_addr = 0;
    uint16_t qsound_loop = 0;
    uint16_t qsound_end = 0;
    double off=1.0;
    if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[i].sample);
      if (s->centerRate<1) {
        off=1.0;
      } else {
        off=(double)s->centerRate/24038.0/16.0;
      }
      qsound_bank = 0x8000 | (s->offQSound >> 16);
      qsound_addr = s->offQSound & 0xffff;

      int length = s->samples;
      if (length > 65536 - 16) {
        length = 65536 - 16;
      }
      if (s->loopStart == -1 || s->loopStart >= length) {
        qsound_end = s->offQSound + length + 15;
        qsound_loop = 15;
      } else {
        qsound_end = s->offQSound + length;
        qsound_loop = length - s->loopStart;
      }
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=off*QS_NOTE_FREQUENCY(chan[i].std.arp);
        } else {
          chan[i].baseFreq=off*QS_NOTE_FREQUENCY(chan[i].note+chan[i].std.arp);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=off*QS_NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false);
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;
      if (chan[i].keyOn) {
        rWrite(q1_reg_map[Q1V_BANK][i], qsound_bank);
        rWrite(q1_reg_map[Q1V_END][i], qsound_end);
        rWrite(q1_reg_map[Q1V_LOOP][i], qsound_loop);
        rWrite(q1_reg_map[Q1V_START][i], qsound_addr);
        rWrite(q1_reg_map[Q1V_PHASE][i], 0x8000);
        //logW("ch %d bank=%04x, addr=%04x, end=%04x, loop=%04x!\n",i,qsound_bank,qsound_addr,qsound_end,qsound_loop);
        // Write sample address. Enable volume
        if (!chan[i].std.hadVol) {
          rWrite(q1_reg_map[Q1V_VOL][i], chan[i].vol << 4);
        }
      }
      if (chan[i].keyOff) {
        // Disable volume
        rWrite(q1_reg_map[Q1V_VOL][i], 0);
        rWrite(q1_reg_map[Q1V_FREQ][i], 0);
      } else if (chan[i].active) {
        //logW("ch %d frequency set to %04x, off=%f, note=%d, %04x!\n",i,chan[i].freq,off,chan[i].note,QS_NOTE_FREQUENCY(chan[i].note));
        rWrite(q1_reg_map[Q1V_FREQ][i], chan[i].freq);
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
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      chan[c.chan].sample=ins->amiga.initSample;
      double off=1.0;
      if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[c.chan].sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=(double)s->centerRate/24038.0/16.0;
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=off*QS_NOTE_FREQUENCY(c.value);
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
      chan[c.chan].std.init(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].std.init(NULL);
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
        if (!chan[c.chan].std.hasVol) {
          // Check if enabled and write volume
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active && c.chan < 16) {
            rWrite(q1_reg_map[Q1V_VOL][c.chan], chan[c.chan].outVol << 4);
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.hasVol) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING:
      immWrite(Q1_PAN+c.chan, c.value + 0x110);
      break;
    case DIV_CMD_QSOUND_ECHO_LEVEL:
      immWrite(Q1_ECHO+c.chan, c.value << 7);
      break;
    case DIV_CMD_QSOUND_ECHO_FEEDBACK:
      immWrite(Q1_ECHO_FEEDBACK, c.value << 6);
      break;
    case DIV_CMD_QSOUND_ECHO_DELAY:
      immWrite(Q1_ECHO_LENGTH, (c.value > 2725 ? 0xfff : 0xfff - (2725 - c.value)));
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      double off=1.0;
      if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[c.chan].sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=(double)s->centerRate/24038.0/16.0;
        }
      }
      int destFreq=off*QS_NOTE_FREQUENCY(c.value2);
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
      double off=1.0;
      if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[c.chan].sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=(double)s->centerRate/24038.0/16.0;
        }
      }
      chan[c.chan].baseFreq=off*QS_NOTE_FREQUENCY(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 255;
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
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;
  }
}

void* DivPlatformQSound::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformQSound::reset() {
  for (int i=0; i<16; i++) {
    chan[i]=DivPlatformQSound::Channel();
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
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformQSound::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformQSound::setFlags(unsigned int flags) {
  echoDelay = 2725 - (flags & 0xfff);
  echoFeedback = (flags >> 12) & 255;

  if(echoDelay < 0) {
    echoDelay = 0;
  }
  if(echoDelay > 2725) {
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

int DivPlatformQSound::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  //for (int i=0; i<16; i++) {
  //  isMuted[i]=false;
  //}
  setFlags(flags);

  chipClock=60000000;
  rate = qsound_start(&chip, chipClock);
  chip.rom_data = (unsigned char*)&chip.rom_mask;
  chip.rom_mask = 0;
  reset();
  return 19;
}

void DivPlatformQSound::quit() {
}
