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

#include "x1_010.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) { x1_010->ram_w(a,v); if (dumpWrites) { addWrite(a,v); } }

#define chRead(c,a) x1_010->ram_r((c<<3)|(a&7))
#define chWrite(c,a,v) rWrite((c<<3)|(a&7),v)
#define waveWrite(c,a,v) rWrite(0x1000|(chan[c].waveBank<<11)|(c<<7)|(a&0x7f),(v-128)&0xff)
#define envFill(c,a) rWrite(0x800|(c<<7)|(a&0x7f),(chan[c].lvol<<4)|chan[c].rvol)
#define envWrite(c,a,l,r) rWrite(0x800|(c<<7)|(a&0x7f),(((chan[c].lvol*(l))/15)<<4)|((chan[c].rvol*(r))/15))

#define refreshControl(c) chWrite(c,0,chan[c].active?(chan[c].pcm?1:((chan[c].env.flag.envEnable&&chan[c].env.flag.envOneshot)?7:3)):0);

#define CHIP_FREQBASE 4194304

const char* regCheatSheetX1_010[]={
  // Channel registers
  "Ch00_Control",       "0000",
  "Ch00_PCMVol_WavSel", "0001",
  "Ch00_FreqL",         "0002",
  "Ch00_FreqH",         "0003",
  "Ch00_Start_EnvFrq",  "0004",
  "Ch00_End_EnvSel",    "0005",
  "Ch01_Control",       "0008",
  "Ch01_PCMVol_WavSel", "0009",
  "Ch01_FreqL",         "000A",
  "Ch01_FreqH",         "000B",
  "Ch01_Start_EnvFrq",  "000C",
  "Ch01_End_EnvSel",    "000D",
  "Ch02_Control",       "0010",
  "Ch02_PCMVol_WavSel", "0011",
  "Ch02_FreqL",         "0012",
  "Ch02_FreqH",         "0013",
  "Ch02_Start_EnvFrq",  "0014",
  "Ch02_End_EnvSel",    "0015",
  "Ch03_Control",       "0018",
  "Ch03_PCMVol_WavSel", "0019",
  "Ch03_FreqL",         "001A",
  "Ch03_FreqH",         "001B",
  "Ch03_Start_EnvFrq",  "001C",
  "Ch03_End_EnvSel",    "001D",
  "Ch04_Control",       "0020",
  "Ch04_PCMVol_WavSel", "0021",
  "Ch04_FreqL",         "0022",
  "Ch04_FreqH",         "0023",
  "Ch04_Start_EnvFrq",  "0024",
  "Ch04_End_EnvSel",    "0025",
  "Ch05_Control",       "0028",
  "Ch05_PCMVol_WavSel", "0029",
  "Ch05_FreqL",         "002A",
  "Ch05_FreqH",         "002B",
  "Ch05_Start_EnvFrq",  "002C",
  "Ch05_End_EnvSel",    "002D",
  "Ch06_Control",       "0030",
  "Ch06_PCMVol_WavSel", "0031",
  "Ch06_FreqL",         "0032",
  "Ch06_FreqH",         "0033",
  "Ch06_Start_EnvFrq",  "0034",
  "Ch06_End_EnvSel",    "0035",
  "Ch07_Control",       "0038",
  "Ch07_PCMVol_WavSel", "0039",
  "Ch07_FreqL",         "003A",
  "Ch07_FreqH",         "003B",
  "Ch07_Start_EnvFrq",  "003C",
  "Ch07_End_EnvSel",    "003D",
  "Ch08_Control",       "0040",
  "Ch08_PCMVol_WavSel", "0041",
  "Ch08_FreqL",         "0042",
  "Ch08_FreqH",         "0043",
  "Ch08_Start_EnvFrq",  "0044",
  "Ch08_End_EnvSel",    "0045",
  "Ch09_Control",       "0048",
  "Ch09_PCMVol_WavSel", "0049",
  "Ch09_FreqL",         "004A",
  "Ch09_FreqH",         "004B",
  "Ch09_Start_EnvFrq",  "004C",
  "Ch09_End_EnvSel",    "004D",
  "Ch10_Control",       "0050",
  "Ch10_PCMVol_WavSel", "0051",
  "Ch10_FreqL",         "0052",
  "Ch10_FreqH",         "0053",
  "Ch10_Start_EnvFrq",  "0054",
  "Ch10_End_EnvSel",    "0055",
  "Ch11_Control",       "0058",
  "Ch11_PCMVol_WavSel", "0059",
  "Ch11_FreqL",         "005A",
  "Ch11_FreqH",         "005B",
  "Ch11_Start_EnvFrq",  "005C",
  "Ch11_End_EnvSel",    "005D",
  "Ch12_Control",       "0060",
  "Ch12_PCMVol_WavSel", "0061",
  "Ch12_FreqL",         "0062",
  "Ch12_FreqH",         "0063",
  "Ch12_Start_EnvFrq",  "0064",
  "Ch12_End_EnvSel",    "0065",
  "Ch13_Control",       "0068",
  "Ch13_PCMVol_WavSel", "0069",
  "Ch13_FreqL",         "006A",
  "Ch13_FreqH",         "006B",
  "Ch13_Start_EnvFrq",  "006C",
  "Ch13_End_EnvSel",    "006D",
  "Ch14_Control",       "0070",
  "Ch14_PCMVol_WavSel", "0071",
  "Ch14_FreqL",         "0072",
  "Ch14_FreqH",         "0073",
  "Ch14_Start_EnvFrq",  "0074",
  "Ch14_End_EnvSel",    "0075",
  "Ch15_Control",       "0078",
  "Ch15_PCMVol_WavSel", "0079",
  "Ch15_FreqL",         "007A",
  "Ch15_FreqH",         "007B",
  "Ch15_Start_EnvFrq",  "007C",
  "Ch15_End_EnvSel",    "007D",
  // Envelope data
  "Env01Data",          "0080",
  "Env02Data",          "0100",
  "Env03Data",          "0180",
  "Env04Data",          "0200",
  "Env05Data",          "0280",
  "Env06Data",          "0300",
  "Env07Data",          "0380",
  "Env08Data",          "0400",
  "Env09Data",          "0480",
  "Env10Data",          "0500",
  "Env11Data",          "0580",
  "Env12Data",          "0600",
  "Env13Data",          "0680",
  "Env14Data",          "0700",
  "Env15Data",          "0780",
  "Env16Data",          "0800",
  "Env17Data",          "0880",
  "Env18Data",          "0900",
  "Env19Data",          "0980",
  "Env20Data",          "0A00",
  "Env21Data",          "0A80",
  "Env22Data",          "0B00",
  "Env23Data",          "0B80",
  "Env24Data",          "0C00",
  "Env25Data",          "0C80",
  "Env26Data",          "0D00",
  "Env27Data",          "0D80",
  "Env28Data",          "0E00",
  "Env29Data",          "0E80",
  "Env30Data",          "0F00",
  "Env31Data",          "0F80",
  // Wavetable data
  "Wave00Data",         "1000",
  "Wave01Data",         "1080",
  "Wave02Data",         "1100",
  "Wave03Data",         "1180",
  "Wave04Data",         "1200",
  "Wave05Data",         "1280",
  "Wave06Data",         "1300",
  "Wave07Data",         "1380",
  "Wave08Data",         "1400",
  "Wave09Data",         "1480",
  "Wave10Data",         "1500",
  "Wave11Data",         "1580",
  "Wave12Data",         "1600",
  "Wave13Data",         "1680",
  "Wave14Data",         "1700",
  "Wave15Data",         "1780",
  "Wave16Data",         "1800",
  "Wave17Data",         "1880",
  "Wave18Data",         "1900",
  "Wave19Data",         "1980",
  "Wave20Data",         "1A00",
  "Wave21Data",         "1A80",
  "Wave22Data",         "1B00",
  "Wave23Data",         "1B80",
  "Wave24Data",         "1C00",
  "Wave25Data",         "1C80",
  "Wave26Data",         "1D00",
  "Wave27Data",         "1D80",
  "Wave28Data",         "1E00",
  "Wave29Data",         "1E80",
  "Wave30Data",         "1F00",
  "Wave31Data",         "1F80",
  NULL
};

const char** DivPlatformX1_010::getRegisterSheet() {
  return regCheatSheetX1_010;
}

const char* DivPlatformX1_010::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Change waveform";
      break;
    case 0x11:
      return "11xx: Change envelope shape";
      break;
    case 0x17:
      return "17xx: Toggle PCM mode";
      break;
    case 0x20:
      return "20xx: Set PCM frequency (1 to FF)";
      break;
    case 0x22:
      return "22xx: Set envelope mode (bit 0: enable, bit 1: one-shot, bit 2: split shape to L/R, bit 3: H.invert right, bit 4: V.invert right)";
      break;
    case 0x23:
      return "23xx: Set envelope period";
      break;
    case 0x25:
      return "25xx: Envelope slide up";
      break;
    case 0x26:
      return "26xx: Envelope slide down";
      break;
    case 0x29:
      return "29xy: Set auto-envelope (x: numerator; y: denominator)";
      break;
  }
  return NULL;
}

void DivPlatformX1_010::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    x1_010->tick();

    signed int tempL=x1_010->output(0);
    signed int tempR=x1_010->output(1);

    if (tempL<-32768) tempL=-32768;
    if (tempL>32767) tempL=32767;
    if (tempR<-32768) tempR=-32768;
    if (tempR>32767) tempR=32767;

    //printf("tempL: %d tempR: %d\n",tempL,tempR);
    bufL[h]=stereo?tempL:((tempL+tempR)>>1);
    bufR[h]=stereo?tempR:bufL[h];
  }
}

double DivPlatformX1_010::NoteX1_010(int ch, int note) {
  if (chan[ch].pcm) { // PCM note
    double off=1.0;
    int sample = chan[ch].sample;
    if (sample>=0 && sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(sample);
      if (s->centerRate<1) {
        off=1.0;
      } else {
        off=s->centerRate/8363.0;
      }
    }
    return off*parent->calcBaseFreq(chipClock,8192,note,false);
  }
  // Wavetable note
  return NOTE_FREQUENCY(note);
}

void DivPlatformX1_010::updateWave(int ch) {
  DivWavetable* wt=parent->getWave(chan[ch].wave);
  if (chan[ch].active) {
    chan[ch].waveBank ^= 1;
  }
  for (int i=0; i<128; i++) {
    if (wt->max<1 || wt->len<1) {
      waveWrite(ch,i,0);
    } else {
      waveWrite(ch,i,wt->data[i*wt->len/128]*255/wt->max);
    }
  }
  if (!chan[ch].pcm) {
    chWrite(ch,1,(chan[ch].waveBank<<4)|(ch&0xf));
  }
}

void DivPlatformX1_010::updateEnvelope(int ch) {
  if (!chan[ch].pcm) {
    if (isMuted[ch]) {
      for (int i=0; i<128; i++) {
        rWrite(0x800|(ch<<7)|(i&0x7f),0);
      }
    } else {
      if (!chan[ch].env.flag.envEnable) {
        for (int i=0; i<128; i++) {
          envFill(ch,i);
        }
      } else {
        DivWavetable* wt=parent->getWave(chan[ch].env.shape);
        for (int i=0; i<128; i++) {
          if (wt->max<1 || wt->len<1) {
            envFill(ch,i);
          } else if (chan[ch].env.flag.envHinv||chan[ch].env.flag.envSplit||chan[ch].env.flag.envVinv) { // Stereo config 
            int la = i, ra = i;
            int lo, ro;
            if (chan[ch].env.flag.envHinv) { ra = 127-i; } // horizontal invert right envelope
            if (chan[ch].env.flag.envSplit) { // Split shape to left and right half
              lo = wt->data[la*(wt->len/128/2)]*15/wt->max;
              ro = wt->data[(ra+128)*(wt->len/128/2)]*15/wt->max;
            } else {
              lo = wt->data[la*wt->len/128]*15/wt->max;
              ro = wt->data[ra*wt->len/128]*15/wt->max;
            }
            if (chan[ch].env.flag.envVinv) { ro = 15-ro; } // vertical invert right envelope
            envWrite(ch,i,lo,ro);
          } else {
            int out = wt->data[i*wt->len/128]*15/wt->max;
            envWrite(ch,i,out,out);
          }
        }
      }
    }
    chWrite(ch,5,0x10|(ch&0xf));
  } else {
    chWrite(ch,1,(chan[ch].lvol<<4)|chan[ch].rvol);
  }
}

void DivPlatformX1_010::tick() {
  for (int i=0; i<16; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      signed char macroVol=((chan[i].vol&15)*MIN(chan[i].furnacePCM?64:15,chan[i].std.vol))/(chan[i].furnacePCM?64:15);
      if ((!isMuted[i])&&(macroVol!=chan[i].outVol)) {
        chan[i].outVol=macroVol;
        chan[i].envChanged=true;
      }
    }
    if ((!chan[i].pcm) || chan[i].furnacePCM) {
      if (chan[i].std.hadArp) {
        if (!chan[i].inPorta) {
          if (chan[i].std.arpMode) {
            chan[i].baseFreq=NoteX1_010(i,chan[i].std.arp);
          } else {
            chan[i].baseFreq=NoteX1_010(i,chan[i].note+chan[i].std.arp);
          }
        }
        chan[i].freqChanged=true;
      } else {
        if (chan[i].std.arpMode && chan[i].std.finishedArp) {
          chan[i].baseFreq=NoteX1_010(i,chan[i].note);
          chan[i].freqChanged=true;
        }
      }
    }
    if (chan[i].std.hadWave && !chan[i].pcm) {
      if (chan[i].wave!=chan[i].std.wave) {
        chan[i].wave=chan[i].std.wave;
        if (!chan[i].pcm) {
          updateWave(i);
          if (!chan[i].keyOff) chan[i].keyOn=true;
        }
      }
    }
    if (chan[i].std.hadEx1) {
      bool nextEnable=(chan[i].std.ex1&1);
      if (nextEnable!=(chan[i].env.flag.envEnable)) {
        chan[i].env.flag.envEnable=nextEnable;
        if (!chan[i].pcm) {
          if (!isMuted[i]) {
            chan[i].envChanged=true;
          }
          refreshControl(i);
        }
      }
      bool nextOneshot=(chan[i].std.ex1&2);
      if (nextOneshot!=(chan[i].env.flag.envOneshot)) {
        chan[i].env.flag.envOneshot=nextOneshot;
        if (!chan[i].pcm) {
          refreshControl(i);
        }
      }
      bool nextSplit=(chan[i].std.ex1&4);
      if (nextSplit!=(chan[i].env.flag.envSplit)) {
        chan[i].env.flag.envSplit=nextSplit;
        if (!isMuted[i] && !chan[i].pcm) {
          chan[i].envChanged=true;
        }
      }
      bool nextHinv=(chan[i].std.ex1&8);
      if (nextHinv!=(chan[i].env.flag.envHinv)) {
        chan[i].env.flag.envHinv=nextHinv;
        if (!isMuted[i] && !chan[i].pcm) {
          chan[i].envChanged=true;
        }
      }
      bool nextVinv=(chan[i].std.ex1&16);
      if (nextVinv!=(chan[i].env.flag.envVinv)) {
        chan[i].env.flag.envVinv=nextVinv;
        if (!isMuted[i] && !chan[i].pcm) {
          chan[i].envChanged=true;
        }
      }
    }
    if (chan[i].std.hadEx2) {
      if (chan[i].env.shape!=chan[i].std.ex2) {
        chan[i].env.shape=chan[i].std.ex2;
        if (!chan[i].pcm) {
          if (chan[i].env.flag.envEnable && (!isMuted[i])) {
            chan[i].envChanged=true;
          }
          if (!chan[i].keyOff) chan[i].keyOn=true;
        }
      }
    }
    if (chan[i].std.hadEx3) {
      chan[i].autoEnvNum=chan[i].std.ex3;
      if (!chan[i].pcm) {
        chan[i].freqChanged=true;
        if (!chan[i].std.willAlg) chan[i].autoEnvDen=1;
      }
    }
    if (chan[i].std.hadAlg) {
      chan[i].autoEnvDen=chan[i].std.alg;
      if (!chan[i].pcm) {
        chan[i].freqChanged=true;
        if (!chan[i].std.willEx3) chan[i].autoEnvNum=1;
      }
    }
    if (chan[i].envChanged) {
      if (!isMuted[i]) {
        chan[i].lvol=((chan[i].outVol&0xf)*((chan[i].pan>>4)&0xf))/15;
        chan[i].rvol=((chan[i].outVol&0xf)*((chan[i].pan>>0)&0xf))/15;
      }
      updateEnvelope(i);
      chan[i].envChanged=false;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false);
      if (chan[i].pcm) {
        if (chan[i].freq<1) chan[i].freq=1;
        if (chan[i].freq>255) chan[i].freq=255;
        chWrite(i,2,chan[i].freq&0xff);
      } else {
        if (chan[i].freq>65535) chan[i].freq=65535;
        chWrite(i,2,chan[i].freq&0xff);
        chWrite(i,3,(chan[i].freq>>8)&0xff);
        if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
          chan[i].env.period=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>12;
          chWrite(i,4,chan[i].env.period);
        }
      }
      if (chan[i].keyOn || chan[i].keyOff || (chRead(i,0)&1)) {
        refreshControl(i);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
    if (chan[i].env.slide!=0) {
      chan[i].env.slidefrac+=chan[i].env.slide;
      while (chan[i].env.slidefrac>0xf) {
        chan[i].env.slidefrac-=0x10;
        if (chan[i].env.period<0xff) {
          chan[i].env.period++;
          if (!chan[i].pcm) {
            chWrite(i,4,chan[i].env.period);
          }
        }
      }
      while (chan[i].env.slidefrac<-0xf) {
        chan[i].env.slidefrac+=0x10;
        if (chan[i].env.period>0) {
          chan[i].env.period--;
          if (!chan[i].pcm) {
            chWrite(i,4,chan[i].env.period);
          }
        }
      }
    }
  }
}

int DivPlatformX1_010::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      chWrite(c.chan,0,0); // reset previous note
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if ((ins->type==DIV_INS_AMIGA) || chan[c.chan].pcm) {
        if (ins->type==DIV_INS_AMIGA) {
          chan[c.chan].furnacePCM=true;
        } else {
          chan[c.chan].furnacePCM=false;
        }
        if (skipRegisterWrites) break;
        if (chan[c.chan].furnacePCM) {
          chan[c.chan].pcm=true;
          chan[c.chan].std.init(ins);
          chan[c.chan].sample=ins->amiga.initSample;
          if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(chan[c.chan].sample);
            chWrite(c.chan,4,(s->offX1_010>>12)&0xff);
            int end=(s->offX1_010+s->length8+0xfff)&~0xfff; // padded
            chWrite(c.chan,5,(0x100-(end>>12))&0xff);
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].note=c.value;
              chan[c.chan].baseFreq=NoteX1_010(c.chan,chan[c.chan].note);
              chan[c.chan].freqChanged=true;
            }
          } else {
            chan[c.chan].std.init(NULL);
            chan[c.chan].outVol=chan[c.chan].vol;
            if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
              chWrite(c.chan,0,0); // reset
              chWrite(c.chan,1,0);
              chWrite(c.chan,2,0);
              chWrite(c.chan,4,0);
              chWrite(c.chan,5,0);
              break;
            }
          }
        } else {
          chan[c.chan].std.init(NULL);
          chan[c.chan].outVol=chan[c.chan].vol;
          if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
            chWrite(c.chan,0,0); // reset
            chWrite(c.chan,1,0);
            chWrite(c.chan,2,0);
            chWrite(c.chan,4,0);
            chWrite(c.chan,5,0);
            break;
          }
          DivSample* s=parent->getSample(12*sampleBank+c.value%12);
          chWrite(c.chan,4,(s->offX1_010>>12)&0xff);
          int end=(s->offX1_010+s->length8+0xfff)&~0xfff; // padded
          chWrite(c.chan,5,(0x100-(end>>12))&0xff);
          chan[c.chan].baseFreq=(((unsigned int)s->rate)<<4)/(chipClock/512);
          chan[c.chan].freqChanged=true;
        }
      } else if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].note=c.value;
        chan[c.chan].baseFreq=NoteX1_010(c.chan,chan[c.chan].note);
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].envChanged=true;
      chan[c.chan].std.init(ins);
      refreshControl(c.chan);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].pcm=false;
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
          if (chan[c.chan].outVol!=c.value) {
            chan[c.chan].outVol=c.value;
            if (!isMuted[c.chan]) {
              chan[c.chan].envChanged=true;
            }
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
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      updateWave(c.chan);
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_X1_010_ENVELOPE_SHAPE:
      if (chan[c.chan].env.shape!=c.value) {
        chan[c.chan].env.shape=c.value;
        if (!chan[c.chan].pcm) {
          if (chan[c.chan].env.flag.envEnable&&(!isMuted[c.chan])) {
            chan[c.chan].envChanged=true;
          }
          chan[c.chan].keyOn=true;
        }
      }
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NoteX1_010(c.chan,c.value2);
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
    case DIV_CMD_SAMPLE_MODE:
      if (chan[c.chan].pcm!=(c.value&1)) {
        chan[c.chan].pcm=c.value&1;
        chan[c.chan].freqChanged=true;
        if (!isMuted[c.chan]) {
          chan[c.chan].envChanged=true;
        }
      }
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_PANNING: {
      if (chan[c.chan].pan!=c.value) {
        chan[c.chan].pan=c.value;
        if (!isMuted[c.chan]) {
          chan[c.chan].envChanged=true;
        }
      }
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].note=c.value;
      chan[c.chan].baseFreq=NoteX1_010(c.chan,chan[c.chan].note+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp):(0)));
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_FREQ:
      if (chan[c.chan].pcm) {
        chan[c.chan].freq=MAX(1,c.value&0xff);
        chWrite(c.chan,2,chan[c.chan].freq&0xff);
        if (chRead(c.chan,0)&1) {
          refreshControl(c.chan);
        }
      }
      break;
    case DIV_CMD_X1_010_ENVELOPE_MODE: {
      bool nextEnable=c.value&1;
      if (nextEnable!=(chan[c.chan].env.flag.envEnable)) {
        chan[c.chan].env.flag.envEnable=nextEnable;
        if (!chan[c.chan].pcm) {
          if (!isMuted[c.chan]) {
            chan[c.chan].envChanged=true;
          }
          refreshControl(c.chan);
        }
      }
      bool nextOneshot=c.value&2;
      if (nextOneshot!=(chan[c.chan].env.flag.envOneshot)) {
        chan[c.chan].env.flag.envOneshot=nextOneshot;
        if (!chan[c.chan].pcm) {
          refreshControl(c.chan);
        }
      }
      bool nextSplit=c.value&4;
      if (nextSplit!=(chan[c.chan].env.flag.envSplit)) {
        chan[c.chan].env.flag.envSplit=nextSplit;
        if (!isMuted[c.chan] && !chan[c.chan].pcm) {
          chan[c.chan].envChanged=true;
        }
      }
      bool nextHinv=c.value&8;
      if (nextHinv!=(chan[c.chan].env.flag.envHinv)) {
        chan[c.chan].env.flag.envHinv=nextHinv;
        if (!isMuted[c.chan] && !chan[c.chan].pcm) {
          chan[c.chan].envChanged=true;
        }
      }
      bool nextVinv=c.value&16;
      if (nextVinv!=(chan[c.chan].env.flag.envVinv)) {
        chan[c.chan].env.flag.envVinv=nextVinv;
        if (!isMuted[c.chan] && !chan[c.chan].pcm) {
          chan[c.chan].envChanged=true;
        }
      }
      break;
    }
    case DIV_CMD_X1_010_ENVELOPE_PERIOD:
      chan[c.chan].env.period=c.value;
      if (!chan[c.chan].pcm) {
        chWrite(c.chan,4,chan[c.chan].env.period);
      }
      break;
    case DIV_CMD_X1_010_ENVELOPE_SLIDE:
      chan[c.chan].env.slide=c.value;
      break;
    case DIV_CMD_X1_010_AUTO_ENVELOPE:
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformX1_010::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].envChanged=true;
}

void DivPlatformX1_010::forceIns() {
  for (int i=0; i<16; i++) {
    chan[i].insChanged=true;
    chan[i].envChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
  }
}

void* DivPlatformX1_010::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformX1_010::getRegisterPool() {
  for (int i=0; i<0x2000; i++) {
    regPool[i]=x1_010->ram_r(i);
  }
  return regPool;
}

int DivPlatformX1_010::getRegisterPoolSize() {
  return 0x2000;
}

void DivPlatformX1_010::reset() {
  memset(regPool,0,0x2000);
  for (int i=0; i<16; i++) {
    chan[i]=DivPlatformX1_010::Channel();
    chan[i].reset();
  }
  x1_010->reset();
  sampleBank=0;
  // set per-channel initial panning
  for (int i=0; i<16; i++) {
    chWrite(i,0,0);
  }
}

bool DivPlatformX1_010::isStereo() {
  return stereo;
}

bool DivPlatformX1_010::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformX1_010::notifyWaveChange(int wave) {
  for (int i=0; i<16; i++) {
    if (chan[i].wave==wave) {
      updateWave(i);
    }
  }
}

void DivPlatformX1_010::notifyInsDeletion(void* ins) {
  for (int i=0; i<16; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformX1_010::setFlags(unsigned int flags) {
  switch (flags&15) {
    case 0: // 16MHz (earlier hardwares)
      chipClock=16000000;
      break;
    case 1: // 16.67MHz (later hardwares)
      chipClock=50000000.0/3.0;
      break;
    // Other clock is used?
  }
  rate=chipClock/512;
  stereo=flags&16;
}

void DivPlatformX1_010::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformX1_010::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformX1_010::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  stereo=false;
  for (int i=0; i<16; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  intf.parent=parent;
  x1_010=new x1_010_core(intf);
  x1_010->reset();
  reset();
  return 16;
}

void DivPlatformX1_010::quit() {
  delete x1_010;
}

DivPlatformX1_010::~DivPlatformX1_010() {
}
