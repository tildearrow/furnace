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

#include "ay.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "sound/ay8910.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(regRemap(a),v); if (dumpWrites) {addWrite(regRemap(a),v);} }

#define CHIP_DIVIDER (extMode?extDiv:((sunsoft||clockSel)?16:8))

const char* regCheatSheetAY[]={
  "FreqL_A", "0",
  "FreqH_A", "1",
  "FreqL_B", "2",
  "FreqH_B", "3",
  "FreqL_C", "4",
  "FreqH_C", "5",
  "FreqNoise", "6",
  "Enable", "7",
  "Volume_A", "8",
  "Volume_B", "9",
  "Volume_C", "A",
  "FreqL_Env", "B",
  "FreqH_Env", "C",
  "Control_Env", "D",
  "PortA", "E",
  "PortB", "F",
  NULL
};

const char* regCheatSheetAY8914[]={
  "FreqL_A", "0",
  "FreqL_B", "1",
  "FreqL_C", "2",
  "FreqL_Env", "3",
  "FreqH_A", "4",
  "FreqH_B", "5",
  "FreqH_C", "6",
  "FreqH_Env", "7",
  "Enable", "8",
  "FreqNoise", "9",
  "Control_Env", "A",
  "Volume_A", "B",
  "Volume_B", "C",
  "Volume_C", "D",
  "PortA", "E",
  "PortB", "F",
  NULL
};

// taken from ay8910.cpp
const int sunsoftVolTable[32]={
  103350, 73770, 52657, 37586, 32125, 27458, 24269, 21451,
  18447, 15864, 14009, 12371, 10506,  8922,  7787,  6796,
  5689,  4763,  4095,  3521,  2909,  2403,  2043,  1737,
  1397,  1123,   925,   762,   578,   438,   332,   251
};

const char** DivPlatformAY8910::getRegisterSheet() {
  return intellivision?regCheatSheetAY8914:regCheatSheetAY;
}

/* C program to generate this table:

#include <stdio.h>
#include <math.h>

int main(int argc, char** argv) {
  for (int i=0; i<256; i++) {
    if ((i&15)==0) printf("\n ");
    printf(" %d,",(int)round(pow((double)i/255.0,0.36)*15.0));
  }
}
*/

const unsigned char dacLogTableAY[256]={
  0, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
  9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11,
  11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
  11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
  12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
  13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

void DivPlatformAY8910::runDAC() {
  for (int i=0; i<3; i++) {
    if (chan[i].active && chan[i].curPSGMode.dac && chan[i].dac.sample!=-1) {
      chan[i].dac.period+=chan[i].dac.rate;
      bool end=false;
      bool changed=false;
      int prevOut=chan[i].dac.out;
      while (chan[i].dac.period>rate && !end) {
        DivSample* s=parent->getSample(chan[i].dac.sample);
        if (s->samples<=0) {
          chan[i].dac.sample=-1;
          immWrite(0x08+i,0);
          end=true;
          break;
        }
        unsigned char dacData=dacLogTableAY[(unsigned char)s->data8[chan[i].dac.pos]^0x80];
        chan[i].dac.out=(chan[i].active && !isMuted[i])?MAX(0,dacData-(15-chan[i].outVol)):0;
        if (prevOut!=chan[i].dac.out) {
          prevOut=chan[i].dac.out;
          changed=true;
        }
        chan[i].dac.pos++;
        if (s->isLoopable() && chan[i].dac.pos>=s->loopEnd) {
          chan[i].dac.pos=s->loopStart;
        } else if (chan[i].dac.pos>=(int)s->samples) {
          chan[i].dac.sample=-1;
          //immWrite(0x08+i,0);
          end=true;
          break;
        }
        chan[i].dac.period-=rate;
      }
      if (changed && !end) {
        if (!isMuted[i]) {
          immWrite(0x08+i,chan[i].dac.out);
        }
      }
    }
  }
}

void DivPlatformAY8910::checkWrites() {
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    if (intellivision) {
      ay8914_device* ay8914=(ay8914_device*)ay;
      ay8914->write(w.addr,w.val);
    } else {
      ay->address_w(w.addr);
      ay->data_w(w.val);
    }
    regPool[w.addr&0x0f]=w.val;
    writes.pop();
  }
}

void DivPlatformAY8910::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (ayBufLen<len) {
    ayBufLen=len;
    for (int i=0; i<3; i++) {
      delete[] ayBuf[i];
      ayBuf[i]=new short[ayBufLen];
    }
  }

  if (sunsoft) {
    for (size_t i=0; i<len; i++) {
      runDAC();
      checkWrites();

      ay->sound_stream_update(ayBuf,1);
      bufL[i+start]=ayBuf[0][0];
      bufR[i+start]=bufL[i+start];

      oscBuf[0]->data[oscBuf[0]->needle++]=sunsoftVolTable[31-(ay->lastIndx&31)]>>3;
      oscBuf[1]->data[oscBuf[1]->needle++]=sunsoftVolTable[31-((ay->lastIndx>>5)&31)]>>3;
      oscBuf[2]->data[oscBuf[2]->needle++]=sunsoftVolTable[31-((ay->lastIndx>>10)&31)]>>3;
    }
  } else {
    for (size_t i=0; i<len; i++) {
      runDAC();
      checkWrites();

      ay->sound_stream_update(ayBuf,1);
      if (stereo) {
        bufL[i+start]=ayBuf[0][0]+ayBuf[1][0]+((ayBuf[2][0]*stereoSep)>>8);
        bufR[i+start]=((ayBuf[0][0]*stereoSep)>>8)+ayBuf[1][0]+ayBuf[2][0];
      } else {
        bufL[i+start]=ayBuf[0][0]+ayBuf[1][0]+ayBuf[2][0];
        bufR[i+start]=bufL[i+start];
      }

      oscBuf[0]->data[oscBuf[0]->needle++]=ayBuf[0][0]<<2;
      oscBuf[1]->data[oscBuf[1]->needle++]=ayBuf[1][0]<<2;
      oscBuf[2]->data[oscBuf[2]->needle++]=ayBuf[2][0]<<2;
    }
  }
}

void DivPlatformAY8910::updateOutSel(bool immediate) {
  if (immediate) {
    immWrite(0x07,
          ~((chan[0].curPSGMode.getTone())|
           ((chan[1].curPSGMode.getTone())<<1)|
           ((chan[2].curPSGMode.getTone())<<2)|
           ((chan[0].curPSGMode.getNoise())<<2)|
           ((chan[1].curPSGMode.getNoise())<<3)|
           ((chan[2].curPSGMode.getNoise())<<4)|
           ((!ioPortA)<<6)|
           ((!ioPortB)<<7)));
  } else {
    rWrite(0x07,
          ~((chan[0].curPSGMode.getTone())|
           ((chan[1].curPSGMode.getTone())<<1)|
           ((chan[2].curPSGMode.getTone())<<2)|
           ((chan[0].curPSGMode.getNoise())<<2)|
           ((chan[1].curPSGMode.getNoise())<<3)|
           ((chan[2].curPSGMode.getNoise())<<4)|
           ((!ioPortA)<<6)|
           ((!ioPortB)<<7)));
  }
}

void DivPlatformAY8910::tick(bool sysTick) {
  // PSG
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MIN(15,chan[i].std.vol.val)-(15-(chan[i].vol&15));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (!chan[i].nextPSGMode.dac) {
        if (isMuted[i]) {
          rWrite(0x08+i,0);
        } else if (intellivision && (chan[i].nextPSGMode.getEnvelope())) {
          rWrite(0x08+i,(chan[i].outVol&0xc)<<2);
        } else {
          rWrite(0x08+i,(chan[i].outVol&15)|((chan[i].nextPSGMode.getEnvelope())<<2));
        }
      }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      rWrite(0x06,31-chan[i].std.duty.val);
    }
    if (chan[i].std.wave.had) {
      if (!chan[i].nextPSGMode.dac) {
        chan[i].nextPSGMode.val=(chan[i].std.wave.val+1)&7;
        if (chan[i].active) {
          chan[i].curPSGMode.val=chan[i].nextPSGMode.val;
        }
        if (isMuted[i]) {
          rWrite(0x08+i,0);
        } else if (intellivision && (chan[i].nextPSGMode.getEnvelope())) {
          rWrite(0x08+i,(chan[i].outVol&0xc)<<2);
        } else {
          rWrite(0x08+i,(chan[i].outVol&15)|((chan[i].nextPSGMode.getEnvelope())<<2));
        }
      }
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
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        if (chan[i].nextPSGMode.dac) {
          if (dumpWrites) addWrite(0xffff0002+(i<<8),0);
          DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_AY);
          chan[i].dac.sample=ins->amiga.getSample(chan[i].note);
          if (chan[i].dac.sample<0 || chan[i].dac.sample>=parent->song.sampleLen) {
            if (dumpWrites) {
              rWrite(0x08+i,0);
              addWrite(0xffff0000+(i<<8),chan[i].dac.sample);
            }
            chan[i].dac.pos=0;
            chan[i].dac.period=0;
            chan[i].keyOn=true;
          }
        }
        oldWrites[0x08+i]=-1;
        oldWrites[0x0d]=-1;
      }
    }
    if (chan[i].std.ex2.had) {
      ayEnvMode=chan[i].std.ex2.val;
      rWrite(0x0d,ayEnvMode);
    }
    if (chan[i].std.ex3.had) {
      chan[i].autoEnvNum=chan[i].std.ex3.val;
      chan[i].freqChanged=true;
      if (!chan[i].std.alg.will) chan[i].autoEnvDen=1;
    }
    if (chan[i].std.alg.had) {
      chan[i].autoEnvDen=chan[i].std.alg.val;
      chan[i].freqChanged=true;
      if (!chan[i].std.ex3.will) chan[i].autoEnvNum=1;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].dac.furnaceDAC) {
        double off=1.0;
        if (chan[i].dac.sample>=0 && chan[i].dac.sample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dac.sample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=8363.0/(double)s->centerRate;
          }
        }
        chan[i].dac.rate=((double)rate*((sunsoft||clockSel)?8.0:16.0))/(double)(MAX(1,off*chan[i].freq));
        if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dac.rate);
      }
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
        if (!chan[i].nextPSGMode.dac) {
          chan[i].curPSGMode.val=chan[i].nextPSGMode.val;
        }
      }
      if (chan[i].keyOff) {
        chan[i].curPSGMode.val=0;
        rWrite(0x08+i,0);
      }
      rWrite((i)<<1,chan[i].freq&0xff);
      rWrite(1+((i)<<1),chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
        ayEnvPeriod=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>4;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
      chan[i].freqChanged=false;
    }
  }

  updateOutSel();

  if (ayEnvSlide!=0) {
    ayEnvSlideLow+=ayEnvSlide;
    while (ayEnvSlideLow>7) {
      ayEnvSlideLow-=8;
      if (ayEnvPeriod<0xffff) {
        ayEnvPeriod++;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
    }
    while (ayEnvSlideLow<-7) {
      ayEnvSlideLow+=8;
      if (ayEnvPeriod>0) {
        ayEnvPeriod--;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
    }
  }
  
  for (int i=0; i<16; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }
}

int DivPlatformAY8910::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AY);
      if (!parent->song.disableSampleMacro && (ins->type==DIV_INS_AMIGA || ins->amiga.useSample)) {
        chan[c.chan].nextPSGMode.dac=true;
      } else if (chan[c.chan].dac.furnaceDAC) {
        chan[c.chan].nextPSGMode.dac=false;
      }
      if (chan[c.chan].nextPSGMode.dac) {
        if (skipRegisterWrites) break;
        if (!parent->song.disableSampleMacro && (ins->type==DIV_INS_AMIGA || ins->amiga.useSample)) {
          if (c.value!=DIV_NOTE_NULL) chan[c.chan].dac.sample=ins->amiga.getSample(c.value);
          if (chan[c.chan].dac.sample<0 || chan[c.chan].dac.sample>=parent->song.sampleLen) {
            chan[c.chan].dac.sample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            if (dumpWrites) {
              rWrite(0x08+c.chan,0);
              addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dac.sample);
            }
          }
          chan[c.chan].dac.pos=0;
          chan[c.chan].dac.period=0;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
            chan[c.chan].freqChanged=true;
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].macroInit(ins);
          if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
          }
          //chan[c.chan].keyOn=true;
          chan[c.chan].dac.furnaceDAC=true;
        } else {
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          chan[c.chan].dac.sample=12*sampleBank+chan[c.chan].note%12;
          if (chan[c.chan].dac.sample>=parent->song.sampleLen) {
            chan[c.chan].dac.sample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            if (dumpWrites) addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dac.sample);
          }
          chan[c.chan].dac.pos=0;
          chan[c.chan].dac.period=0;
          chan[c.chan].dac.rate=parent->getSample(chan[c.chan].dac.sample)->rate*2048;
          if (dumpWrites) {
            rWrite(0x08+c.chan,0);
            addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dac.rate);
          }
          chan[c.chan].dac.furnaceDAC=false;
        }
        chan[c.chan].curPSGMode.dac=chan[c.chan].nextPSGMode.dac;
        break;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (!chan[c.chan].nextPSGMode.dac) {
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0);
        } else if (intellivision && (chan[c.chan].nextPSGMode.getEnvelope())) {
          rWrite(0x08+c.chan,(chan[c.chan].vol&0xc)<<2);
        } else {
          rWrite(0x08+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].nextPSGMode.getEnvelope())<<2));
        }
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dac.sample=-1;
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].nextPSGMode.dac=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (!chan[c.chan].nextPSGMode.dac) {
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0);
        } else {
          if (chan[c.chan].active) {
            if (intellivision && (chan[c.chan].nextPSGMode.getEnvelope())) {
              rWrite(0x08+c.chan,(chan[c.chan].vol&0xc)<<2);
            } else {
              rWrite(0x08+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].nextPSGMode.getEnvelope())<<2));
            }
          }
        }
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
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
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      if (!chan[c.chan].nextPSGMode.dac) {
        if (c.value<16) {
          chan[c.chan].nextPSGMode.val=(c.value+1)&7;
          if (chan[c.chan].active) {
            chan[c.chan].curPSGMode.val=chan[c.chan].nextPSGMode.val;
          }
          if (isMuted[c.chan]) {
            rWrite(0x08+c.chan,0);
          } else if (chan[c.chan].active) {
            if (intellivision && (chan[c.chan].nextPSGMode.getEnvelope())) {
              rWrite(0x08+c.chan,(chan[c.chan].outVol&0xc)<<2);
            } else {
              rWrite(0x08+c.chan,(chan[c.chan].outVol&15)|((chan[c.chan].nextPSGMode.getEnvelope())<<2));
            }
          }
        }
      }
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      rWrite(0x06,31-c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      ayEnvMode=c.value>>4;
      rWrite(0x0d,ayEnvMode);
      if (c.value&15) {
        chan[c.chan].nextPSGMode.envelope|=1;
      } else {
        chan[c.chan].nextPSGMode.envelope&=~1;
      }
      if (!chan[c.chan].nextPSGMode.dac && chan[c.chan].active) {
        chan[c.chan].curPSGMode.val=chan[c.chan].nextPSGMode.val;
      }
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0);
      } else if (intellivision && (chan[c.chan].nextPSGMode.getEnvelope())) {
        rWrite(0x08+c.chan,(chan[c.chan].vol&0xc)<<2);
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].nextPSGMode.getEnvelope())<<2));
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      ayEnvPeriod&=0xff00;
      ayEnvPeriod|=c.value;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      ayEnvPeriod&=0xff;
      ayEnvPeriod|=c.value<<8;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      ayEnvSlide=c.value;
      break;
    case DIV_CMD_AY_AUTO_ENVELOPE:
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_AY_IO_WRITE:
      if (c.value==255) break;
      if (c.value) { // port B
        ioPortB=true;
        portBVal=c.value2;
        logI("AY I/O port B write: %x",portBVal);
      } else { // port A
        ioPortA=true;
        portAVal=c.value2;
        logI("AY I/O port A write: %x",portAVal);
      }
      updateOutSel(true);
      immWrite(14+(c.value?1:0),(c.value?portBVal:portAVal));
      break;
    case DIV_CMD_SAMPLE_MODE:
      chan[c.chan].nextPSGMode.dac=(c.value>0)?1:0;
      if (chan[c.chan].active) {
        chan[c.chan].curPSGMode.dac=chan[c.chan].nextPSGMode.dac;
      }
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      // TODO: FIX wtr_envelope.dmf
      // the brokenPortaArp update broke it
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AY));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformAY8910::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(0x08+ch,0);
  } else if (chan[ch].active && chan[ch].nextPSGMode.dac) {
    rWrite(0x08+ch,chan[ch].dac.out);
  } else {
    if (intellivision && (chan[ch].nextPSGMode.getEnvelope()) && chan[ch].active) {
      rWrite(0x08+ch,(chan[ch].vol&0xc)<<2);
    } else if (chan[ch].active) {
      rWrite(0x08+ch,(chan[ch].outVol&15)|((chan[ch].nextPSGMode.getEnvelope())<<2));
    }
  }
}

void DivPlatformAY8910::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
  }
  immWrite(0x0b,ayEnvPeriod);
  immWrite(0x0c,ayEnvPeriod>>8);
  immWrite(0x0d,ayEnvMode);
}

void* DivPlatformAY8910::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformAY8910::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformAY8910::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformAY8910::getRegisterPool() {
  return regPool;
}

int DivPlatformAY8910::getRegisterPoolSize() {
  return 16;
}

void DivPlatformAY8910::flushWrites() {
  while (!writes.empty()) writes.pop();
}

bool DivPlatformAY8910::getDCOffRequired() {
  return true;
}

void DivPlatformAY8910::reset() {
  while (!writes.empty()) writes.pop();
  ay->device_reset();
  memset(regPool,0,16);
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformAY8910::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x0f;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  for (int i=0; i<16; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  sampleBank=0;
  ayEnvPeriod=0;
  ayEnvMode=0;
  ayEnvSlide=0;
  ayEnvSlideLow=0;

  delay=0;

  ioPortA=false;
  ioPortB=false;
  portAVal=0;
  portBVal=0;
}

bool DivPlatformAY8910::isStereo() {
  return true;
}

bool DivPlatformAY8910::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformAY8910::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAY8910::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformAY8910::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformAY8910::setExtClockDiv(unsigned int eclk, unsigned char ediv) {
  if (extMode) {
    extClock=eclk;
    extDiv=ediv;
  }
}

void DivPlatformAY8910::setFlags(const DivConfig& flags) {
  if (extMode) {
    chipClock=extClock;
    rate=chipClock/extDiv;
    clockSel=false;
  } else {
    clockSel=flags.getBool("halfClock",false);
    switch (flags.getInt("clockSel",0)) {
      case 1:
        chipClock=COLOR_PAL*2.0/5.0;
        break;
      case 2:
        chipClock=1750000;
        break;
      case 3:
        chipClock=2000000;
        break;
      case 4:
        chipClock=1500000;
        break;
      case 5:
        chipClock=1000000;
        break;
      case 6:
        chipClock=COLOR_NTSC/4.0;
        break;
      case 7:
        chipClock=COLOR_PAL*3.0/8.0;
        break;
      case 8:
        chipClock=COLOR_PAL*3.0/16.0;
        break;
      case 9:
        chipClock=COLOR_PAL/4.0;
        break;
      case 10:
        chipClock=2097152;
        break;
      case 11:
        chipClock=COLOR_NTSC;
        break;
      case 12:
        chipClock=3600000;
        break;
      case 13:
        chipClock=20000000/16;
        break;
      case 14:
        chipClock=1536000;
        break;
      case 15:
        chipClock=38400*13*4; // 31948800/16
        break;
      default:
        chipClock=COLOR_NTSC/2.0;
        break;
    }
    CHECK_CUSTOM_CLOCK;
    rate=chipClock/8;
  }
  for (int i=0; i<3; i++) {
    oscBuf[i]->rate=rate;
  }

  if (ay!=NULL) delete ay;
  switch (flags.getInt("chipType",0)) {
    case 1:
      ay=new ym2149_device(rate,clockSel);
      sunsoft=false;
      intellivision=false;
      break;
    case 2:
      ay=new sunsoft_5b_sound_device(rate);
      sunsoft=true;
      intellivision=false;
      break;
    case 3:
      ay=new ay8914_device(rate);
      sunsoft=false;
      intellivision=true;
      break;
    default:
      ay=new ay8910_device(rate);
      sunsoft=false;
      intellivision=false;
      break;
  }
  ay->device_start();
  ay->device_reset();

  stereo=flags.getBool("stereo",false);
  stereoSep=flags.getInt("stereoSep",0)&255;
}

int DivPlatformAY8910::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  ay=NULL;
  setFlags(flags);
  ayBufLen=65536;
  for (int i=0; i<3; i++) ayBuf[i]=new short[ayBufLen];
  reset();
  return 3;
}

void DivPlatformAY8910::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
    delete[] ayBuf[i];
  }
  if (ay!=NULL) delete ay;
}
