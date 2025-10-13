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

#include "ay8930x.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite2(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER (extMode?extDiv:clockSel?64:32)

const char* regCheatSheetAY8930X[]={
  "FreqL_A", "00",
  "FreqH_A", "01",
  "FreqL_B", "02",
  "FreqH_B", "03",
  "FreqL_C", "04",
  "FreqH_C", "05",
  "FreqNoiseA", "06",
  "Enable", "07",
  "Volume_A", "08",
  "Volume_B", "09",
  "Volume_C", "0A",
  "FreqL_EnvA", "0B",
  "FreqH_EnvA", "0C",
  "Control_EnvA", "0D",
  "Volume_EnvA", "0E",
  "Bank", "0F",
  "FreqL_EnvB", "10",
  "FreqH_EnvB", "11",
  "FreqL_EnvC", "12",
  "FreqH_EnvC", "13",
  "Control_EnvB", "14",
  "Control_EnvC", "15",
  "Duty_A", "16",
  "Duty_B", "17",
  "Duty_C", "18",
  "NoiseAND", "19",
  "NoiseOR", "1A",
  "FreqNoiseB", "1B",
  "FreqNoiseC", "1C",
  "Volume_EnvB", "1D",
  "Volume_EnvC", "1E",
  NULL
};

void DivPlatformAY8930X::immWrite(unsigned char a, unsigned char v) {
  if ((int)bank!=(a>>4)) {
    bank=a>>4;
    immWrite2(0x0f, 0x01|(bank<<1));
  }
  if (a==0x0f) {
    immWrite2(0x0f,0x01|(bank<<1)|(v&0xfc));
  } else {
    immWrite2(a&31,v);
  }
}

const char** DivPlatformAY8930X::getRegisterSheet() {
  return regCheatSheetAY8930X;
}

/* C program to generate this table:

#include <stdio.h>
#include <math.h>

int main(int argc, char** argv) {
  for (int i=0; i<256; i++) {
    if ((i&15)==0) printf("\n ");
    printf(" %d,",(int)round(pow((double)i/255.0,0.3)*31.0));
  }
}
*/

const unsigned char dacLogTableAY8930X[256]={
  0, 6, 7, 8, 9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13,
  14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16,
  17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19,
  19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20,
  20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
  23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
  25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
  26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27,
  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28,
  28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
  28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
  29, 29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
  30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31
};

void DivPlatformAY8930X::runDAC(int runRate, int advance) {
  if (runRate==0) runRate=dacRate;
  for (int i=0; i<3; i++) {
    if (chan[i].active && (chan[i].curPSGMode.val&8) && chan[i].dac.sample!=-1) {
      chan[i].dac.period+=chan[i].dac.rate*advance;
      bool end=false;
      bool changed=false;
      int prevOut=chan[i].dac.out;
      while (chan[i].dac.period>=runRate && !end) {
        DivSample* s=parent->getSample(chan[i].dac.sample);
        if (s->samples<=0 || chan[i].dac.pos<0 || chan[i].dac.pos>=(int)s->samples) {
          chan[i].dac.sample=-1;
          immWrite(0x08+i,0xc0);
          end=true;
          break;
        }
        unsigned char dacData=dacLogTableAY8930X[(unsigned char)s->data8[chan[i].dac.pos]^0x80];
        chan[i].dac.out=(chan[i].active && !isMuted[i])?MAX(0,dacData-(31-chan[i].outVol)):0;
        if (prevOut!=chan[i].dac.out) {
          prevOut=chan[i].dac.out;
          changed=true;
        }
        chan[i].dac.pos++;
        if (s->isLoopable() && chan[i].dac.pos>=s->loopEnd) {
          chan[i].dac.pos=s->loopStart;
        } else if (chan[i].dac.pos>=(int)s->samples) {
          chan[i].dac.sample=-1;
          //immWrite(0x08+i,0xc0);
          end=true;
          break;
        }
        chan[i].dac.period-=runRate;
      }
      if (changed && !end) {
        if (!isMuted[i]) {
          immWrite(0x08+i,chan[i].dac.out|(((chan[i].chVolL^1)&1)<<6)|(((chan[i].chVolR^1)&1)<<7));
        }
      }
    }
  }
}

void DivPlatformAY8930X::checkWrites() {
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    ay->address_w((w.addr&0x0f));
    ay->data_w(w.val);
    if ((w.addr&0x0f)!=0x0f && (regPool[0x0f]&0x02)==0x02) {
      regPool[(w.addr&0x0f)|0x10]=w.val;
    } else {
      regPool[w.addr&0x0f]=w.val;
    }
    writes.pop();
  }
}

void DivPlatformAY8930X::acquireDirect(blip_buffer_t** bb, size_t len) {
  thread_local short ayBuf[6];
  for (int i=0; i<3; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t i=0; i<len; i++) {
    int advance=len-i;
    // heuristic
    if (!writes.empty()) {
      advance=1;
    } else {
      for (int j=0; j<3; j++) {
        // tone counter
        if (!ay->tone_enable(j) && ay->m_tone[j].volume!=0) {
          const int period=MAX(1,ay->m_tone[j].period)*(ay->m_step_mul<<1);
          const int remain=(period-ay->m_tone[j].count)>>5;
          if (remain<advance) {
            advance=remain;
          }
        }

        // count me in if I have noise enabled
        if (!ay->noise_enable(j) && ay->m_tone[j].volume!=0) {
          const int noisePeriod=MAX(1,(int)ay->m_noise[j].period)*ay->m_step_mul;
          const int noiseRemain=noisePeriod-ay->m_noise[j].count_noise;
          if (noiseRemain<advance) {
            advance=noiseRemain;
          }
        }

        // envelope check
        if (ay->m_tone[j].volume&32) {
          if (ay->m_envelope[j].holding==0) {
            const int periodEnv=MAX(1,ay->m_envelope[j].period)*ay->m_env_step_mul;
            const int remainEnv=periodEnv-ay->m_envelope[j].count;
            if (remainEnv<advance) {
              advance=remainEnv;
            }
          }
        }

        // DAC
        if (chan[j].active && (chan[j].curPSGMode.val&8) && chan[j].dac.sample!=-1) {
          if (chan[j].dac.rate<=0) continue;
          const int remainTime=(rate-chan[j].dac.period+chan[j].dac.rate-1)/chan[j].dac.rate;
          if (remainTime<advance) advance=remainTime;
        }

        if (advance<=1) break;
      }
    }

    if (advance<1) advance=1;
    
    runDAC(0,advance);
    checkWrites();

    ay->sound_stream_update(ayBuf,advance);
    i+=advance-1;

    int lout=ayBuf[0]+ayBuf[1]+ayBuf[2];
    int rout=ayBuf[3]+ayBuf[4]+ayBuf[5];
    if (lastOut[0]!=lout) {
      blip_add_delta(bb[0],i,lout-lastOut[0]);
      lastOut[0]=lout;
    }
    if (lastOut[1]!=rout) {
      blip_add_delta(bb[1],i,rout-lastOut[1]);
      lastOut[1]=rout;
    }

    oscBuf[0]->putSample(i,ayBuf[0]+ayBuf[3]);
    oscBuf[1]->putSample(i,ayBuf[1]+ayBuf[4]);
    oscBuf[2]->putSample(i,ayBuf[2]+ayBuf[5]);
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformAY8930X::fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
  writes.clear();
  for (size_t i=0; i<len; i++) {
    runDAC(sRate,1);
    while (!writes.empty()) {
      QueuedWrite& w=writes.front();
      stream.push_back(DivDelayedWrite(i,w.addr,w.val));
      writes.pop_front();
    }
  }
}

void DivPlatformAY8930X::updateOutSel(bool immediate) {
  lastFlag=~((chan[0].curPSGMode.getTone())|
           ((chan[1].curPSGMode.getTone())<<1)|
           ((chan[2].curPSGMode.getTone())<<2)|
           ((chan[0].curPSGMode.getNoise())<<2)|
           ((chan[1].curPSGMode.getNoise())<<3)|
           ((chan[2].curPSGMode.getNoise())<<4));
  if (immediate) {
    immWrite(0x07,lastFlag);
  } else {
    rWrite(0x07,lastFlag);
  }
}

const unsigned char regPeriodL[3]={
  0x0b, 0x10, 0x12
};

const unsigned char regPeriodH[3]={
  0x0c, 0x11, 0x13
};

const unsigned char regMode[3]={
  0x0d, 0x14, 0x15
};

const unsigned char regShift[3]={
  0x0e, 0x1d, 0x1e
};

const unsigned char regNoise[3]={
  0x06, 0x1b, 0x1c
};

void DivPlatformAY8930X::tick(bool sysTick) {
  // PSG
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MIN(31,chan[i].std.vol.val)-(31-(chan[i].vol&31));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (!(chan[i].nextPSGMode.val&8)) {
        if (isMuted[i]) {
          rWrite(0x08+i,0xc0);
        } else {
          if (chan[i].nextPSGMode.getEnvelope()) {
            rWrite(regShift[i],((chan[i].outVol>>2)&7)^7);
          }
          rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].nextPSGMode.getEnvelope())<<3)|(((chan[i].chVolL^1)&1)<<6)|(((chan[i].chVolR^1)&1)<<7));
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
      rWrite(regNoise[i],chan[i].std.duty.val);
    }
    if (chan[i].std.wave.had) {
      if (!(chan[i].nextPSGMode.val&8)) {
        chan[i].nextPSGMode.val=chan[i].std.wave.val&7;
        if (chan[i].active) {
          chan[i].curPSGMode.val=chan[i].nextPSGMode.val;
        }
        if (isMuted[i]) {
          rWrite(0x08+i,0xc0);
        } else {
          if (chan[i].nextPSGMode.getEnvelope()) {
            rWrite(regShift[i],((chan[i].outVol>>2)&7)^7);
          }
          rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].nextPSGMode.getEnvelope())<<3)|(((chan[i].chVolL^1)&1)<<6)|(((chan[i].chVolR^1)&1)<<7));
        }
      }
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-65535,65535);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.panL.had) {
      chan[i].chVolL=(chan[i].std.panL.val&2)>>1;
      chan[i].chVolR=chan[i].std.panL.val&1;
      if (isMuted[i]) {
        rWrite(0x08+i,0xc0);
      } else if (chan[i].nextPSGMode.val&8) {
        rWrite(0x08+i,(chan[i].dac.out&31)|(((chan[i].chVolL^1)&1)<<6)|(((chan[i].chVolR^1)&1)<<7));
      } else {
        rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].nextPSGMode.getEnvelope())<<3)|(((chan[i].chVolL^1)&1)<<6)|(((chan[i].chVolR^1)&1)<<7));
      }
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        if (chan[i].nextPSGMode.val&8) {
          if (dumpWrites) addWrite(0xffff0002+(i<<8),0);
          if (chan[i].dac.sample<0 || chan[i].dac.sample>=parent->song.sampleLen) {
            if (dumpWrites) {
              rWrite(0x08+i,0xc0);
              addWrite(0xffff0000+(i<<8),chan[i].dac.sample);
            }
            if (chan[i].dac.setPos) {
              chan[i].dac.setPos=false;
            } else {
              chan[i].dac.pos=0;
            }
            chan[i].dac.period=0;
            chan[i].keyOn=true;
          }
        }
        oldWrites[0x08+i]=-1;
        oldWrites[regMode[i]]=-1;
      }
    }
    if (chan[i].std.ex1.had) { // duty
      immWrite(0x16+i,chan[i].std.ex1.val);
    }
    if (chan[i].std.ex2.had) {
      chan[i].envelope.mode=chan[i].std.ex2.val;
      rWrite(regMode[i],chan[i].envelope.mode);
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
    if (chan[i].std.fb.had) {
      ayNoiseAnd=chan[i].std.fb.val;
      immWrite(0x19,ayNoiseAnd);
    }
    if (chan[i].std.fms.had) {
      ayNoiseOr=chan[i].std.fms.val;
      immWrite(0x1a,ayNoiseOr);
    }
    if (chan[i].std.ex4.had) {
      chan[i].fixedFreq=chan[i].std.ex4.val;
      chan[i].freqChanged=true;
    }
    if (chan[i].std.ex5.had) {
      chan[i].envelope.period=chan[i].std.ex5.val;
      immWrite(regPeriodL[i],chan[i].envelope.period);
      immWrite(regPeriodH[i],chan[i].envelope.period>>8);
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
            off=parent->getCenterRate()/(double)s->centerRate;
          }
        }
        chan[i].dac.rate=((double)chipClock/2.0)/(double)(MAX(1,off*chan[i].freq));
        if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dac.rate);
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].keyOn) {
        if (!(chan[i].nextPSGMode.val&8)) {
          chan[i].curPSGMode.val=chan[i].nextPSGMode.val;
        }
        if (chan[i].insChanged) {
          if (!chan[i].std.ex1.will) immWrite(0x16+i,chan[i].duty);
          chan[i].insChanged=false;
        }
      }
      if (chan[i].keyOff) {
        chan[i].curPSGMode.val=0;
        rWrite(0x08+i,0xc0);
      }
      if (chan[i].fixedFreq>0) {
        rWrite((i)<<1,chan[i].fixedFreq&0xff);
        rWrite(1+((i)<<1),chan[i].fixedFreq>>8);
      } else {
        rWrite((i)<<1,chan[i].freq&0xff);
        rWrite(1+((i)<<1),chan[i].freq>>8);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
        chan[i].envelope.period=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>4;
        immWrite(regPeriodL[i],chan[i].envelope.period);
        immWrite(regPeriodH[i],chan[i].envelope.period>>8);
      }
      if (chan[i].freqChanged && chan[i].autoNoiseMode) {
        int noiseFreq=chan[i].freq;
        switch (chan[i].autoNoiseMode) {
          case 1: // noise
            noiseFreq+=chan[i].autoNoiseOff;
            if (noiseFreq<0) noiseFreq=0;
            if (noiseFreq>255) noiseFreq=255;
            rWrite(regNoise[i],noiseFreq);
            break;
          case 2: { // noise + OR mask
            if (noiseFreq<0) noiseFreq=0;
            int noiseDiv=(noiseFreq>>8)+1;
            noiseFreq/=noiseDiv;
            ayNoiseOr=noiseDiv;
            immWrite(0x1a,ayNoiseOr);
            noiseFreq+=chan[i].autoNoiseOff;
            if (noiseFreq<0) noiseFreq=0;
            if (noiseFreq>255) noiseFreq=255;
            rWrite(regNoise[i],noiseFreq);
            break;
          }
        }
      }
      chan[i].freqChanged=false;
    }

    if (chan[i].envelope.slide!=0) {
      chan[i].envelope.slideLow+=chan[i].envelope.slide;
      while (chan[i].envelope.slideLow>7) {
        chan[i].envelope.slideLow-=8;
        if (chan[i].envelope.period<0xffff) {
          chan[i].envelope.period++;
          immWrite(regPeriodL[i],chan[i].envelope.period);
          immWrite(regPeriodH[i],chan[i].envelope.period>>8);
        }
      }
      while (chan[i].envelope.slideLow<-7) {
        chan[i].envelope.slideLow+=8;
        if (chan[i].envelope.period>0) {
          chan[i].envelope.period--;
          immWrite(regPeriodL[i],chan[i].envelope.period);
          immWrite(regPeriodH[i],chan[i].envelope.period>>8);
        }
      }
    }
  }

  updateOutSel();
  
  for (int i=0; i<32; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }
}

int DivPlatformAY8930X::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AY8930X);
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
        chan[c.chan].nextPSGMode.val|=8;
      } else if (chan[c.chan].dac.furnaceDAC) {
        chan[c.chan].nextPSGMode.val&=~8;
      }
      if (chan[c.chan].nextPSGMode.val&8) {
        if (skipRegisterWrites) break;
        if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].dac.sample=ins->amiga.getSample(c.value);
            chan[c.chan].sampleNote=c.value;
            c.value=ins->amiga.getFreq(c.value);
            chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
          } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
            chan[c.chan].dac.sample=ins->amiga.getSample(chan[c.chan].sampleNote);
            c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
          }
          if (chan[c.chan].dac.sample<0 || chan[c.chan].dac.sample>=parent->song.sampleLen) {
            chan[c.chan].dac.sample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            if (dumpWrites) {
              rWrite(0x08+c.chan,0xc0);
              addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dac.sample);
            }
          }
          if (chan[c.chan].dac.setPos) {
            chan[c.chan].dac.setPos=false;
          } else {
            chan[c.chan].dac.pos=0;
          }
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
          if (chan[c.chan].dac.setPos) {
            chan[c.chan].dac.setPos=false;
          } else {
            chan[c.chan].dac.pos=0;
          }
          chan[c.chan].dac.period=0;
          chan[c.chan].dac.rate=parent->getSample(chan[c.chan].dac.sample)->rate*4096;
          if (dumpWrites) {
            rWrite(0x08+c.chan,0xc0);
            addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dac.rate);
          }
          chan[c.chan].dac.furnaceDAC=false;
        }
        chan[c.chan].curPSGMode.val&=~8;
        chan[c.chan].curPSGMode.val|=chan[c.chan].nextPSGMode.val&8;
        break;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].fixedFreq=0;
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (!(chan[c.chan].nextPSGMode.val&8)) {
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0xc0);
        } else if (chan[c.chan].nextPSGMode.getEnvelope()) {
          rWrite(regShift[c.chan],((chan[c.chan].vol>>2)&7)^7);
        }
        rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].nextPSGMode.getEnvelope())<<3)|(((chan[c.chan].chVolL^1)&1)<<6)|(((chan[c.chan].chVolR^1)&1)<<7));
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dac.sample=-1;
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].nextPSGMode.val&=~8;
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
      if (!(chan[c.chan].nextPSGMode.val&8)) {
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0xc0);
        } else {
          if (chan[c.chan].active) {
            if (chan[c.chan].nextPSGMode.getEnvelope()) {
              rWrite(regShift[c.chan],((chan[c.chan].vol>>2)&7)^7);
            }
            rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].nextPSGMode.getEnvelope())<<3)|(((chan[c.chan].chVolL^1)&1)<<6)|(((chan[c.chan].chVolR^1)&1)<<7));
          }
        }
        break;
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
    case DIV_CMD_PANNING: {
      chan[c.chan].chVolL=(c.value>0);
      chan[c.chan].chVolR=(c.value2>0);
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0xc0);
      } else if (chan[c.chan].nextPSGMode.val&8) {
        rWrite(0x08+c.chan,(chan[c.chan].dac.out&31)|(((chan[c.chan].chVolL^1)&1)<<6)|(((chan[c.chan].chVolR^1)&1)<<7));
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].outVol&31)|((chan[c.chan].nextPSGMode.getEnvelope())<<3)|(((chan[c.chan].chVolL^1)&1)<<6)|(((chan[c.chan].chVolR^1)&1)<<7));
      }
      break;
    }
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
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      if (c.value<0x10) {
        if (!(chan[c.chan].nextPSGMode.val&8)) {
          chan[c.chan].nextPSGMode.val=(c.value+1)&7;
          if (chan[c.chan].active) {
            chan[c.chan].curPSGMode.val=chan[c.chan].nextPSGMode.val;
          }
          if (isMuted[c.chan]) {
            rWrite(0x08+c.chan,0xc0);
          } else if (chan[c.chan].active) {
            if (chan[c.chan].nextPSGMode.getEnvelope()) {
              rWrite(regShift[c.chan],((chan[c.chan].vol>>2)&7)^7);
            }
            rWrite(0x08+c.chan,(chan[c.chan].outVol&31)|((chan[c.chan].nextPSGMode.getEnvelope())<<3)|(((chan[c.chan].chVolL^1)&1)<<6)|(((chan[c.chan].chVolR^1)&1)<<7));
          }
        }
      } else {
        chan[c.chan].duty=(c.value-0x10)&31;
        immWrite(0x16+c.chan,chan[c.chan].duty);
      }
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      rWrite(regNoise[c.chan],c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      chan[c.chan].envelope.mode=c.value>>4;
      rWrite(regMode[c.chan],chan[c.chan].envelope.mode);
      if (c.value&15) {
        chan[c.chan].nextPSGMode.val|=4;
      } else {
        chan[c.chan].nextPSGMode.val&=~4;
      }
      if (!(chan[c.chan].nextPSGMode.val&8) && chan[c.chan].active) {
        chan[c.chan].curPSGMode.val=chan[c.chan].nextPSGMode.val;
      }
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0xc0);
      } else {
        if (chan[c.chan].nextPSGMode.getEnvelope()) {
          rWrite(regShift[c.chan],((chan[c.chan].vol>>2)&7)^7);
        }
        rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].nextPSGMode.getEnvelope())<<3)|(((chan[c.chan].chVolL^1)&1)<<6)|(((chan[c.chan].chVolR^1)&1)<<7));
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      chan[c.chan].envelope.period&=0xff00;
      chan[c.chan].envelope.period|=c.value;
      immWrite(regPeriodL[c.chan],chan[c.chan].envelope.period);
      immWrite(regPeriodH[c.chan],chan[c.chan].envelope.period>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      chan[c.chan].envelope.period&=0xff;
      chan[c.chan].envelope.period|=c.value<<8;
      immWrite(regPeriodL[c.chan],chan[c.chan].envelope.period);
      immWrite(regPeriodH[c.chan],chan[c.chan].envelope.period>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      chan[c.chan].envelope.slide=c.value;
      break;
    case DIV_CMD_AY_NOISE_MASK_AND:
      ayNoiseAnd=c.value;
      immWrite(0x19,ayNoiseAnd);
      break;
    case DIV_CMD_AY_NOISE_MASK_OR:
      ayNoiseOr=c.value;
      immWrite(0x1a,ayNoiseOr);
      break;
    case DIV_CMD_AY_AUTO_ENVELOPE:
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_AY_AUTO_PWM:
      chan[c.chan].autoNoiseMode=c.value>>4;
      chan[c.chan].autoNoiseOff=c.value&15;
      if (chan[c.chan].autoNoiseOff>=8) chan[c.chan].autoNoiseOff-=16;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_SAMPLE_MODE:
      if (c.value>0) {
        chan[c.chan].nextPSGMode.val|=8;
      } else {
        chan[c.chan].nextPSGMode.val&=~8;
      }
      if (chan[c.chan].active) {
        chan[c.chan].curPSGMode.val&=~8;
        chan[c.chan].curPSGMode.val|=chan[c.chan].nextPSGMode.val&8;
      }
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].dac.pos=c.value;
      chan[c.chan].dac.setPos=true;
      if (dumpWrites) addWrite(0xffff0005,chan[c.chan].dac.pos);
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
    case DIV_CMD_GET_VOLMAX:
      return 31;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AY8930X));
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

void DivPlatformAY8930X::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(0x08+ch,0xc0);
  } else if (chan[ch].active) {
    if (chan[ch].nextPSGMode.val&8) {
      rWrite(0x08+ch,(chan[ch].dac.out&31)|(((chan[ch].chVolL^1)&1)<<6)|(((chan[ch].chVolR^1)&1)<<7));
    } else {
      if (chan[ch].nextPSGMode.getEnvelope()) {
        rWrite(regShift[ch],((chan[ch].vol>>2)&7)^7);
      }
      rWrite(0x08+ch,(chan[ch].outVol&31)|((chan[ch].nextPSGMode.getEnvelope())<<3)|(((chan[ch].chVolL^1)&1)<<6)|(((chan[ch].chVolR^1)&1)<<7));
    }
  }
}

void DivPlatformAY8930X::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].curPSGMode.val&=~8;
    chan[i].nextPSGMode.val&=~8;
    immWrite(regPeriodL[i],chan[i].envelope.period);
    immWrite(regPeriodH[i],chan[i].envelope.period>>8);
    immWrite(regMode[i],chan[i].envelope.mode);
    immWrite(regShift[i],((chan[i].outVol>>2)&7)^7);
  }
}

void* DivPlatformAY8930X::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformAY8930X::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformAY8930X::getPan(int ch) {
  return (chan[ch].chVolL<<8)|(chan[ch].chVolR);
}

DivSamplePos DivPlatformAY8930X::getSamplePos(int ch) {
  if (ch>=3) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dac.sample,
    chan[ch].dac.pos,
    chan[ch].dac.rate
  );
}

DivDispatchOscBuffer* DivPlatformAY8930X::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformAY8930X::mapVelocity(int ch, float vel) {
  return round(31.0*pow(vel,0.22));
}

float DivPlatformAY8930X::getGain(int ch, int vol) {
  if (vol==0) return 0;
  return 1.0/pow(10.0,(float)(31-vol)*1.5/20.0);
}

unsigned char* DivPlatformAY8930X::getRegisterPool() {
  return regPool;
}

int DivPlatformAY8930X::getRegisterPoolSize() {
  return 32;
}

void DivPlatformAY8930X::flushWrites() {
  while (!writes.empty()) writes.pop();
}

void DivPlatformAY8930X::reset() {
  while (!writes.empty()) writes.pop();
  ay->device_reset();
  memset(regPool,0,32);
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformAY8930X::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=31;
    chan[i].envelope.period=0;
    chan[i].envelope.mode=0;
    chan[i].envelope.slide=0;
    chan[i].envelope.slideLow=0;
    chan[i].noise.period=0;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  for (int i=0; i<32; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastFlag=0;
  sampleBank=0;
  ayNoiseAnd=2;
  ayNoiseOr=0;
  delay=0;
  lastOut[0]=0;
  lastOut[1]=0;

  bank=false;

  immWrite(0x0f,0x01);
  immWrite(0x19,2); // and mask
  immWrite(0x1a,0x00); // or mask
}

int DivPlatformAY8930X::getOutputCount() {
  return 2;
}

bool DivPlatformAY8930X::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformAY8930X::hasAcquireDirect() {
  return true;
}

bool DivPlatformAY8930X::getLegacyAlwaysSetVolume() {
  return false;
}

void DivPlatformAY8930X::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAY8930X::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformAY8930X::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformAY8930X::setExtClockDiv(unsigned int eclk, unsigned char ediv) {
  if (extMode) {
    extClock=eclk;
    extDiv=ediv;
  }
}

void DivPlatformAY8930X::setFlags(const DivConfig& flags) {
  if (extMode) {
    chipClock=extClock;
    rate=chipClock/extDiv;
    clockSel=false;
    dacRate=chipClock/dacRateDiv;
  } else {
    clockSel=flags.getBool("halfClock",false);
    switch (flags.getInt("clockSel",0)) {
      case 1:
        chipClock=COLOR_PAL*16.0/5.0;
        break;
      case 2:
        chipClock=16000000;
        break;
      case 3:
        chipClock=12000000;
        break;
      case 4:
        chipClock=12288000;
        break;
      default:
        chipClock=COLOR_NTSC*4.0;
        break;
    }
    CHECK_CUSTOM_CLOCK;
    rate=chipClock/32;
    dacRate=rate;
  }
  for (int i=0; i<3; i++) {
    oscBuf[i]->setRate(rate);
  }
}

int DivPlatformAY8930X::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  ay=new ay8930x_device(rate,clockSel);
  ay->device_start();
  reset();
  return 3;
}

void DivPlatformAY8930X::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
  delete ay;
}
