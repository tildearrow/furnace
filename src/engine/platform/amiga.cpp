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

#define _USE_MATH_DEFINES
#include "amiga.h"
#include "../engine.h"
#include <math.h>

#define AMIGA_DIVIDER 8
#define CHIP_DIVIDER 16

const char* regCheatSheetAmiga[]={
  "DMACON", "96",
  "INTENA", "9A",
  "ADKCON", "9E",

  "AUD0LCH", "A0",
  "AUD0LCL", "A2",
  "AUD0LEN", "A4",
  "AUD0PER", "A6",
  "AUD0VOL", "A8",
  "AUD0DAT", "AA",

  "AUD1LCH", "B0",
  "AUD1LCL", "B2",
  "AUD1LEN", "B4",
  "AUD1PER", "B6",
  "AUD1VOL", "B8",
  "AUD1DAT", "BA",

  "AUD2LCH", "C0",
  "AUD2LCL", "C2",
  "AUD2LEN", "C4",
  "AUD2PER", "C6",
  "AUD2VOL", "C8",
  "AUD2DAT", "CA",

  "AUD3LCH", "D0",
  "AUD3LCL", "D2",
  "AUD3LEN", "D4",
  "AUD3PER", "D6",
  "AUD3VOL", "D8",
  "AUD3DAT", "DA",
  NULL
};

const char** DivPlatformAmiga::getRegisterSheet() {
  return regCheatSheetAmiga;
}

const char* DivPlatformAmiga::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Toggle filter (0 disables; 1 enables)";
      break;
    case 0x11:
      return "11xx: Toggle AM with next channel";
      break;
    case 0x12:
      return "12xx: Toggle period modulation with next channel";
      break;
  }
  return NULL;
}

void DivPlatformAmiga::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static int outL, outR;
  for (size_t h=start; h<start+len; h++) {
    outL=0;
    outR=0;
    for (int i=0; i<4; i++) {
      if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
        chan[i].audSub-=AMIGA_DIVIDER;
        if (chan[i].audSub<0) {
          DivSample* s=parent->getSample(chan[i].sample);
          if (s->samples>0) {
            chan[i].audDat=s->data8[chan[i].audPos++];
            if (i<3 && chan[i].useV) {
              chan[i+1].outVol=(unsigned char)chan[i].audDat^0x80;
              if (chan[i+1].outVol>64) chan[i+1].outVol=64;
            }
            if (i<3 && chan[i].useP) {
              chan[i+1].freq=(unsigned char)chan[i].audDat^0x80;
              if (chan[i+1].freq<AMIGA_DIVIDER) chan[i+1].freq=AMIGA_DIVIDER;
            }
            if (chan[i].audPos>=s->samples || chan[i].audPos>=131071) {
              if (s->loopStart>=0 && s->loopStart<(int)s->samples) {
                chan[i].audPos=s->loopStart;
              } else {
                chan[i].sample=-1;
              }
            }
          } else {
            chan[i].sample=-1;
          }
          /*if (chan[i].freq<124) {
            if (++chan[i].busClock>=512) {
              unsigned int rAmount=(124-chan[i].freq)*2;
              if (chan[i].audPos>=rAmount) {
                chan[i].audPos-=rAmount;
              }
              chan[i].busClock=0;
            }
          }*/
          if (bypassLimits) {
            chan[i].audSub+=MAX(AMIGA_DIVIDER,chan[i].freq);
          } else {
            chan[i].audSub+=MAX(114,chan[i].freq);
          }
        }
      }
      if (!isMuted[i]) {
        if (i==0 || i==3) {
          outL+=((chan[i].audDat*chan[i].outVol)*sep1)>>7;
          outR+=((chan[i].audDat*chan[i].outVol)*sep2)>>7;
        } else {
          outL+=((chan[i].audDat*chan[i].outVol)*sep2)>>7;
          outR+=((chan[i].audDat*chan[i].outVol)*sep1)>>7;
        }
      }
    }
    filter[0][0]+=(filtConst*(outL-filter[0][0]))>>12;
    filter[0][1]+=(filtConst*(filter[0][0]-filter[0][1]))>>12;
    filter[1][0]+=(filtConst*(outR-filter[1][0]))>>12;
    filter[1][1]+=(filtConst*(filter[1][0]-filter[1][1]))>>12;
    bufL[h]=filter[0][1];
    bufR[h]=filter[1][1];
  }
}

void DivPlatformAmiga::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=((chan[i].vol%65)*MIN(64,chan[i].std.vol))>>6;
    }
    double off=1.0;
    if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[i].sample);
      if (s->centerRate<1) {
        off=1.0;
      } else {
        off=8363.0/(double)s->centerRate;
      }
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=round(off*NOTE_PERIODIC_NOROUND(chan[i].std.arp));
        } else {
          chan[i].baseFreq=round(off*NOTE_PERIODIC_NOROUND(chan[i].note+chan[i].std.arp));
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=round(off*NOTE_PERIODIC_NOROUND(chan[i].note));
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadWave) {
      if (chan[i].wave!=chan[i].std.wave) {
        chan[i].wave=chan[i].std.wave;
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].note>0x5d) chan[i].freq=0x01;
      if (chan[i].keyOn) {
        if (chan[i].wave<0) {
          chan[i].wave=0;
        }
      }
      if (chan[i].keyOff) {
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformAmiga::dispatch(DivCommand c) {
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
          off=8363.0/(double)s->centerRate;
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(off*NOTE_PERIODIC_NOROUND(c.value));
      }
      if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (chan[c.chan].setPos) {
        chan[c.chan].setPos=false;
      } else {
        chan[c.chan].audPos=0;
      }
      chan[c.chan].audSub=0;
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
          chan[c.chan].outVol=c.value;
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
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      chan[c.chan].sample=ins->amiga.initSample;
      double off=1.0;
      if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[c.chan].sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=8363.0/(double)s->centerRate;
        }
      }
      int destFreq=round(off*NOTE_PERIODIC_NOROUND(c.value2));
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
          off=8363.0/(double)s->centerRate;
        }
      }
      chan[c.chan].baseFreq=round(off*NOTE_PERIODIC_NOROUND(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp-12):(0))));
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
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].audPos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_AMIGA_FILTER:
      filterOn=c.value;
      filtConst=filterOn?filtConstOn:filtConstOff;
      break;
    case DIV_CMD_AMIGA_AM:
      chan[c.chan].useV=c.value;
      break;
    case DIV_CMD_AMIGA_PM:
      chan[c.chan].useP=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 64;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformAmiga::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformAmiga::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].audPos=131072;
    chan[i].audDat=0;
    chan[i].sample=-1;
  }
}

void* DivPlatformAmiga::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformAmiga::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformAmiga::Channel();
    filter[0][i]=0;
    filter[1][i]=0;
  }
  filterOn=false;
  filtConst=filterOn?filtConstOn:filtConstOff;
}

bool DivPlatformAmiga::isStereo() {
  return true;
}

bool DivPlatformAmiga::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformAmiga::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformAmiga::notifyWaveChange(int wave) {
  // TODO when wavetables are added
}

void DivPlatformAmiga::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAmiga::setFlags(unsigned int flags) {
  if (flags&1) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  rate=chipClock/AMIGA_DIVIDER;
  sep1=((flags>>8)&127)+127;
  sep2=127-((flags>>8)&127);
  amigaModel=flags&2;
  bypassLimits=flags&4;
  if (amigaModel) {
    filtConstOff=4000;
    filtConstOn=sin(M_PI*8000.0/(double)rate)*4096.0;
  } else {
    filtConstOff=sin(M_PI*16000.0/(double)rate)*4096.0;
    filtConstOn=sin(M_PI*5500.0/(double)rate)*4096.0;
  }
}

int DivPlatformAmiga::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformAmiga::quit() {
}
