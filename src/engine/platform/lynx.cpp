/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "lynx.h"
#include "../engine.h"
#include "../bsr.h"
#include <math.h>

#define rWrite(a,v) {if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);}}}

#define WRITE_VOLUME(ch,v) rWrite(0x20+(ch<<3),(v))
#define WRITE_FEEDBACK(ch,v) rWrite(0x21+(ch<<3),(v))
#define WRITE_OUTPUT(ch,v) rWrite(0x22+(ch<<3),(v))
#define WRITE_LFSR(ch,v) rWrite(0x23+(ch<<3),(v))
#define WRITE_BACKUP(ch,v) rWrite(0x24+(ch<<3),(v))
#define WRITE_CONTROL(ch,v) rWrite(0x25+(ch<<3),(v))
#define WRITE_OTHER(ch,v) rWrite(0x27+(ch<<3),(v))
#define WRITE_ATTEN(ch,v) rWrite((0x40+ch),(v))
#define WRITE_STEREO(v) rWrite(0x50,(v))

#define CHIP_FREQBASE 16000000

static int32_t clamp(int32_t v, int32_t lo, int32_t hi)
{
  return v<lo?lo:(v>hi?hi:v);
}

const int DUTY_DIVIDERS[]={
     1,   2,   4,   3,   6,   7,   7,   4,   8,  15,   6,  14,  15,  12,  14,   5,
    10,  21,  31,  28,  31,  30,  12,  31,  21,   8,  30,  31,  28,  31,  31,   6,
    12,  63,  14,  62,   9,  28,  62,  15,  14,  42,   8,  21,  62,  63,  15,  60,
    63,  20,  42,  63,  28,  12,  63,  62,  62,  63,  21,  24,  15,  62,  60,   7,
    16,  63,  30, 254, 217,  84, 186, 217,  12, 254,  28,  15, 254,  51, 255,  84,
   217, 120, 210,  63,  60, 255, 255, 254, 254, 217,  63, 252,  17,  42, 124,  85,
    30, 254,  24, 217, 210,  21, 255, 252,  28, 217,  10, 186,  63, 124, 254, 255,
   186, 255, 255,  56, 255,  70,  36,  30, 255,  60, 254,  85, 124,  85,  21, 254,
    22,1533,2047, 868,1953,2046, 420, 651,1533,2044,2046,2047,1016,1785, 105, 126,
   595, 630, 204,1533,1302,2047,2047,2044,2044, 119, 372,1778,1953, 120, 682,1905,
   595, 868, 510,2047,2044, 762, 279, 682, 210,1953, 595,1524,1533, 210, 248, 635,
    60,2047,2047,  62,1905,2044,2046,  42,2047, 510, 252,1905,1778,2047,1533,1016,
  1953,1860,1778, 219,  48,1905,2047,1778, 682,2047, 465,1020,1785, 126,2044, 434,
  2044,  63,2047,2046,2047, 280,  30,1953, 210, 682, 868,  89,2046,2047,2047,1524,
  1302,1785,2047,1860,2047,2046,1016, 372,2047,  84, 630,  70, 252,2047,1533, 682,
  1905,2046, 372,1905,  90,1533, 217, 168, 340,2047,2047,1302,1533,  28, 186,2047,
    24,3255, 126,1190,  45,4092, 178, 315,  28, 438, 124, 315,3570, 255,1023,2044,
   819,1016, 930,3937,1260,1302, 511,4094,3810, 819, 195,2604,1023,4094,  84,1365,
    18,3810,  56,1023,  42,3937, 819,4092, 124,4095,  30,4094,  85,1524,3906,  63,
  3906,1023,3255, 120,4095,3570,1020,3937,2667,3556,4094, 195,2044,4095,4095, 762,
    28,1302,  84,4095,3906,1023, 255, 292,  16,1085,  42,3066, 315,3556,4094,4095,
  3066,4095,3937, 372, 511,4094, 620,1302, 273, 504,1190, 819,4092,4095,1023, 210,
   124,1023, 126,3570,3937, 252, 438,4095,  30,4094, 120,4095,4094, 255,  63,1020,
  4095,1364, 558,2667, 420,4095, 105,1270,1190,3255,  93,1016,  91, 372,4092,3937,
  3255,  44,3066,1365,1736, 510,  91,4094,1302, 511,3937, 420, 105,3906,4092, 819,
   252, 585, 255, 210,3937,1016,3570,1023, 455,3066,2044,1365,4094, 126,2667,4092,
  3810,  93,  63,1364,1365,3906, 120,  28,1023,2044, 238,4095,3556, 255,4095, 372,
   511,1190,1260,1023,3066, 255, 819, 408,2044, 255,1302,4094, 455,2604,4094, 585,
   438, 511, 455, 292, 455, 434,1364, 102,1085, 204,3570, 255, 240,4095,  93,4094,
  3937,4094,  84,  63,4094, 315, 819, 140,1260,3937,4095,3066,2667, 680,4094, 511,
  4095,2044, 178,1023,1020, 819,  21,3810,4094,  20,1023,3556,3937, 210,2040, 273,
   252,2667,4095,3906,  63, 124, 930, 455, 510,4094,4092, 511,3570,4095,  30, 744
};

const char* regCheatSheetLynx[]={
  "AUDIO0_VOLCNTRL", "20",
  "AUDIO0_FEEDBACK", "21",
  "AUDIO0_OUTPUT", "22",
  "AUDIO0_SHIFT", "23",
  "AUDIO0_BACKUP", "24",
  "AUDIO0_CONTROL", "25",
  "AUDIO0_COUNTER", "26",
  "AUDIO0_OTHER", "27",
  "AUDIO1_VOLCNTRL", "28",
  "AUDIO1_FEEDBACK", "29",
  "AUDIO1_OUTPUT", "2a",
  "AUDIO1_SHIFT", "2b",
  "AUDIO1_BACKUP", "2c",
  "AUDIO1_CONTROL", "2d",
  "AUDIO1_COUNTER", "2e",
  "AUDIO1_OTHER", "2f",
  "AUDIO2_VOLCNTRL", "30",
  "AUDIO2_FEEDBACK", "31",
  "AUDIO2_OUTPUT", "32",
  "AUDIO2_SHIFT", "33",
  "AUDIO2_BACKUP", "34",
  "AUDIO2_CONTROL", "35",
  "AUDIO2_COUNTER", "36",
  "AUDIO2_OTHER", "37",
  "AUDIO3_VOLCNTRL", "38",
  "AUDIO3_FEEDBACK", "39",
  "AUDIO3_OUTPUT", "3a",
  "AUDIO3_SHIFT", "3b",
  "AUDIO3_BACKUP", "3c",
  "AUDIO3_CONTROL", "3d",
  "AUDIO3_COUNTER", "3e",
  "AUDIO3_OTHER", "3f",
  "ATTENREG0", "40",
  "ATTENREG1", "41",
  "ATTENREG2", "42",
  "ATTENREG3", "43",
  "MPAN", "44",
  "MSTEREO", "50",
  NULL
};


const char** DivPlatformLynx::getRegisterSheet() {
  return regCheatSheetLynx;
}

void DivPlatformLynx::processDAC(int sRate) {
  for (int i=0; i<4; i++) {
    if (chan[i].pcm && chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
      chan[i].sampleAccum-=chan[i].sampleFreq;
      while (chan[i].sampleAccum<0) {
        chan[i].sampleAccum+=sRate;
        DivSample* s=parent->getSample(chan[i].sample);
        if (s!=NULL) {
          if (isMuted[i]) {
            WRITE_OUTPUT(i,0);
          } else {
            if (chan[i].samplePos<0 || chan[i].samplePos>=(int)s->samples) {
              WRITE_OUTPUT(i,0);
            } else {
              WRITE_OUTPUT(i,CLAMP((s->data8[chan[i].samplePos]*chan[i].outVol)>>7,-128,127));
            }
          }
          chan[i].samplePos++;

          if (s->isLoopable() && chan[i].samplePos>=s->loopEnd) {
            chan[i].samplePos=s->loopStart;
          } else if (chan[i].samplePos>=(int)s->samples) {
            chan[i].sample=-1;
          }
        }
      }
    }
  }
}

void DivPlatformLynx::acquire(short** buf, size_t len) {
  thread_local int chanBuf[4];

  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    processDAC(rate);

    while (!writes.empty()) {
      QueuedWrite& w=writes.front();
      mikey->write(w.addr,w.val);
      writes.pop_front();
    }

    mikey->sampleAudio(buf[0]+h,buf[1]+h,1,chanBuf);

    for (int i=0; i<4; i++) {
      oscBuf[i]->putSample(h,chanBuf[i]);
    }
  }

  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformLynx::fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
  writes.clear();
  for (size_t i=0; i<len; i++) {
    processDAC(sRate);

    while (!writes.empty()) {
      QueuedWrite& w=writes.front();
      stream.push_back(DivDelayedWrite(i,w.addr,w.val));
      writes.pop_front();
    }
  }
  regWrites.clear();
}

void DivPlatformLynx::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      if (chan[i].pcm) {
        chan[i].outVol=((chan[i].vol&127)*MIN(chan[i].macroVolMul,chan[i].std.vol.val))/chan[i].macroVolMul;
      } else {
        chan[i].outVol=((chan[i].vol&127)*MIN(127,chan[i].std.vol.val))>>7;
      }
      WRITE_VOLUME(i,(isMuted[i]?0:(chan[i].outVol&127)));
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        double CHIP_DIVIDER=tuned?DUTY_DIVIDERS[chan[i].duty.val&0x1ff]*8:64;
        chan[i].actualNote=parent->calcArp(chan[i].note,chan[i].std.arp.val);
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].actualNote);
        if (chan[i].pcm) chan[i].sampleBaseFreq=NOTE_FREQUENCY(chan[i].actualNote);
        chan[i].freqChanged=true;
      }
    }

    if (chan[i].std.panL.had) {
      chan[i].pan&=0x0f;
      chan[i].pan|=(chan[i].std.panL.val&15)<<4;
    }

    if (chan[i].std.panR.had) {
      chan[i].pan&=0xf0;
      chan[i].pan|=chan[i].std.panR.val&15;
    }

    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      WRITE_ATTEN(i,chan[i].pan);
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

    if (chan[i].std.ex1.had) {
      chan[i].lfsr=chan[i].std.ex1.val&0xfff;
      chan[i].freqChanged=true;
      chan[i].updateLFSR=true;
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        if (chan[i].pcm && chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          chan[i].sampleAccum=0;
          if (chan[i].setPos) {
            chan[i].setPos=false;
          } else {
            chan[i].samplePos=0;
          }
        }
        chan[i].freqChanged=true;
        chan[i].updateLFSR=true;
      }
    }

    if (chan[i].freqChanged) {
      if (chan[i].pcm) {
        double off=1.0;
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].sample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=(double)s->centerRate/parent->getCenterRate();
          }
        }
        chan[i].sampleFreq=off*parent->calcFreq(chan[i].sampleBaseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
        if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].sampleFreq);
      } else {
        if (chan[i].updateLFSR) {
          WRITE_LFSR(i, (chan[i].lfsr&0xff));
          WRITE_OTHER(i, ((chan[i].lfsr&0xf00)>>4));
          chan[i].updateLFSR=false;
        }
        if (chan[i].std.duty.had) {
          chan[i].duty=chan[i].std.duty.val;
          if (!chan[i].pcm) {
            WRITE_FEEDBACK(i, chan[i].duty.feedback);
          }
        }
        double divider=tuned?DUTY_DIVIDERS[chan[i].duty.val&0x1ff]*8:64;
        chan[i].fd=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,divider);
        WRITE_CONTROL(i, (chan[i].fd.clockDivider|0x18|chan[i].duty.int_feedback7));
        WRITE_BACKUP( i, chan[i].fd.backup );
      }
      chan[i].freqChanged=false;
    } else if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      if (!chan[i].pcm) {
        if (tuned) {
          double divider=DUTY_DIVIDERS[chan[i].duty.val&0x1ff]*8;
          chan[i].fd=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,divider);
        }
        WRITE_FEEDBACK(i, chan[i].duty.feedback);
        WRITE_CONTROL(i, (chan[i].fd.clockDivider|0x18|chan[i].duty.int_feedback7));
        if (tuned) WRITE_BACKUP( i, chan[i].fd.backup );
      }
    }
  }
}

int DivPlatformLynx::dispatch(DivCommand c) {
  double CHIP_DIVIDER=tuned?DUTY_DIVIDERS[chan[c.chan].duty.val&0x1ff]*8:64;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      bool prevPCM=chan[c.chan].pcm;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_MIKEY);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:127;
      chan[c.chan].pcm=(ins->type==DIV_INS_AMIGA || ins->amiga.useSample);
      if (chan[c.chan].pcm!=prevPCM) {
        if (chan[c.chan].pcm) {
          WRITE_FEEDBACK(c.chan,0);
          WRITE_CONTROL(c.chan,0x18);
          WRITE_BACKUP(c.chan,0);
        } else {
          chan[c.chan].sampleNote=DIV_NOTE_NULL;
          chan[c.chan].sampleNoteDelta=0;
          WRITE_FEEDBACK(c.chan,chan[c.chan].duty.feedback);
          WRITE_CONTROL(c.chan,(chan[c.chan].fd.clockDivider|0x18|chan[c.chan].duty.int_feedback7));
          WRITE_BACKUP(c.chan,chan[c.chan].fd.backup);
        }
      }
      if (chan[c.chan].pcm) {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
          chan[c.chan].sampleBaseFreq=NOTE_FREQUENCY(c.value);
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
        }
        chan[c.chan].sampleAccum=0;
        if (chan[c.chan].setPos) {
          chan[c.chan].setPos=false;
        } else {
          chan[c.chan].samplePos=0;
        }
        if (dumpWrites) {
          addWrite(0xffff0000+(c.chan<<8),chan[c.chan].sample);
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].actualNote=c.value;
        chan[c.chan].updateLFSR=true;
      }
      chan[c.chan].active=true;
      WRITE_VOLUME(c.chan,(isMuted[c.chan]?0:(chan[c.chan].vol&127)));
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      WRITE_VOLUME(c.chan,0);
      WRITE_CONTROL(c.chan,0);
      chan[c.chan].macroInit(NULL);
      if (chan[c.chan].pcm) {
        chan[c.chan].pcm=false;
        if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      }
      break;
    case DIV_CMD_LYNX_LFSR_LOAD:
      chan[c.chan].freqChanged=true;
      chan[c.chan].lfsr=c.value;
      chan[c.chan].updateLFSR=true;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=c.value;
      //chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_MIKEY));
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active && !chan[c.chan].pcm) WRITE_VOLUME(c.chan,(isMuted[c.chan]?0:(chan[c.chan].vol&127)));
      }
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      WRITE_ATTEN(c.chan,chan[c.chan].pan);
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2+chan[c.chan].sampleNoteDelta);
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
      if (chan[c.chan].pcm && parent->song.linearPitch==2) {
        chan[c.chan].sampleBaseFreq=chan[c.chan].baseFreq;
      }
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      int whatAMess=c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0));
      chan[c.chan].baseFreq=NOTE_PERIODIC(whatAMess);
      if (chan[c.chan].pcm) {
        chan[c.chan].sampleBaseFreq=NOTE_FREQUENCY(whatAMess);
      }
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].actualNote=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_MIKEY));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].samplePos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformLynx::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (chan[ch].active) WRITE_VOLUME(ch,(isMuted[ch]?0:(chan[ch].outVol&127)));
}

int DivPlatformLynx::getOutputCount() {
  return 2;
}

void DivPlatformLynx::forceIns() {
  for (int i=0; i<4; i++) {
    if (chan[i].active) {
      chan[i].insChanged=true;
      chan[i].freqChanged=true;
      if (!chan[i].pcm) {
        WRITE_FEEDBACK(i,chan[i].duty.feedback);
      }
    }
    WRITE_ATTEN(i,chan[i].pan);
  }
}

void* DivPlatformLynx::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformLynx::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformLynx::getPan(int ch) {
  return ((chan[ch].pan&0xf0)<<4)|(chan[ch].pan&15);
}

DivSamplePos DivPlatformLynx::getSamplePos(int ch) {
  if (ch>=4) return DivSamplePos();
  if (!chan[ch].pcm) return DivSamplePos();
  return DivSamplePos(
    chan[ch].sample,
    chan[ch].samplePos,
    chan[ch].sampleFreq
  );
}

DivDispatchOscBuffer* DivPlatformLynx::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformLynx::getRegisterPool()
{
  return const_cast<unsigned char*>( mikey->getRegisterPool() );
}

int DivPlatformLynx::getRegisterPoolSize()
{
  return 4*8+4;
}

void DivPlatformLynx::reset() {
  mikey=std::make_unique<Lynx::Mikey>(rate);

  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformLynx::Channel();
    chan[i].std.setEngine(parent);
  }
  writes.clear();
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  WRITE_STEREO(0);
}

bool DivPlatformLynx::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformLynx::keyOffAffectsPorta(int ch) {
  return true;
}

bool DivPlatformLynx::getLegacyAlwaysSetVolume() {
  return false;
}

//int DivPlatformLynx::getPortaFloor(int ch) {
//  return 12;
//}

void DivPlatformLynx::setFlags(const DivConfig& flags) {
  tuned=flags.getBool("tuned",false);
  chipClock=16000000;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/128;
  for (int i=0; i<4; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformLynx::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformLynx::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformLynx::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr, i.val);
}

int DivPlatformLynx::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  reset();
  return 4;
}

void DivPlatformLynx::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  mikey.reset();
}

DivPlatformLynx::~DivPlatformLynx() {
}

DivPlatformLynx::MikeyFreqDiv::MikeyFreqDiv(int frequency) {

  int clamped=clamp(frequency, 36, 16383);

  auto top=bsr(clamped);
  
  if (top>7)
  {
    clockDivider=top-7;
    backup=clamped>>(top-7);
  }
  else
  {
    clockDivider=0;
    backup=clamped;
  }
}

DivPlatformLynx::MikeyDuty::MikeyDuty(int duty) {

  //duty:
  //9: int
  //8: f11
  //7: f10
  //6: f7
  //5: f5
  //4: f4
  //3: f3
  //2: f2
  //1: f1
  //0: f0

  val=duty;
  //f7 moved to bit 7 and int moved to bit 5
  int_feedback7=((duty&0x40)<<1)|((duty&0x200)>>4);
  //f11 and f10 moved to bits 7 & 6
  feedback=(duty&0x3f)|((duty&0x180)>>1);
}
