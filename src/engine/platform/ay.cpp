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

#include "ay.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "sound/ay8910.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(regRemap(a),v)); if (dumpWrites) {addWrite(regRemap(a),v);} } 

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

void DivPlatformAY8910::runDAC(int runRate, int advance) {
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
        chan[i].dac.period-=runRate;
      }
      if (changed && !end) {
        if (!isMuted[i]) {
          immWrite(0x08+i,chan[i].dac.out);
        }
      }
    }
  }
}

void DivPlatformAY8910::runTFX(int runRate, int advance) {
  /*
  developer's note: if you are checking for intellivision
  make sure to add "&& selCore"
  because for some reason, the register remap doesn't work
  when the user uses AtomicSSG core
  */
  float counterRatio=advance;
  if (runRate!=0) counterRatio=(double)rate/(double)runRate;
  int timerPeriod, output;
  for (int i=0; i<3; i++) {
    if (chan[i].active && (chan[i].curPSGMode.val&16) && !(chan[i].curPSGMode.val&8)) {
      if (chan[i].tfx.mode == -1 && !isMuted[i]) {
        if (intellivision && chan[i].curPSGMode.getEnvelope()) {
          immWrite(0x08+i,(chan[i].outVol&0xc)<<2);
          continue;
        } else {
          immWrite(0x08+i,(chan[i].outVol&15)|((chan[i].curPSGMode.getEnvelope())<<2));
          continue;
        }
      }
      chan[i].tfx.counter += counterRatio;
      if (chan[i].tfx.counter >= chan[i].tfx.period) {
        chan[i].tfx.counter -= chan[i].tfx.period;
        switch (chan[i].tfx.mode) {
          case 0:
            // pwm
            // we will handle the modulator gen after this switch... if we don't, crackling happens
            chan[i].tfx.out ^= 1;
            break;
          case 1:
            // syncbuzzer
            if (!isMuted[i]) {
              if (intellivision && chan[i].curPSGMode.getEnvelope()) {
                immWrite(0x0b + i, (chan[i].outVol & 0xc) << 2);
              }
              else {
                immWrite(0x08 + i, (chan[i].outVol & 15) | ((chan[i].curPSGMode.getEnvelope()) << 2));
              }
            }
            if (intellivision && selCore) {
              immWrite(0xa, ayEnvMode);
            }
            else {
              immWrite(0xd, ayEnvMode);
            }
            break;
          case 2:
          default:
            // unimplemented, or invalid effects here
            break;
        }
      }
      if (chan[i].tfx.mode == 0) {
        // pwm
        output = ((chan[i].tfx.out) ? chan[i].outVol : (chan[i].tfx.lowBound-(15-chan[i].outVol)));
        output = (output <= 0) ? 0 : output; // underflow
        output = (output >= 15) ? 15 : output; // overflow
        output &= 15; // i don't know if i need this but i'm too scared to remove it
        if (!isMuted[i]) {
          // TODO: ???????
          if (intellivision && selCore) {
            immWrite(0x0b+i,(output&0xc)<<2);
          } else {
            immWrite(0x08+i,output|(chan[i].curPSGMode.getEnvelope()<<2));
          }
        }
      }
    }
    if (chan[i].tfx.num > 0) {
      timerPeriod = chan[i].freq*chan[i].tfx.den/chan[i].tfx.num;
    } else {
      timerPeriod = chan[i].freq*chan[i].tfx.den;
    }
    if (chan[i].tfx.num > 0 && chan[i].tfx.den > 0) chan[i].tfx.period=timerPeriod+chan[i].tfx.offset;
    // stupid pitch correction because:
    // YM2149 half-clock and Sunsoft 5B: timers run an octave too high
    // on AtomicSSG core timers run 2 octaves too high
    if (clockSel || sunsoft) chan[i].tfx.period = chan[i].tfx.period * 2;
    if (selCore && !intellivision) chan[i].tfx.period = chan[i].tfx.period * 4;
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

void DivPlatformAY8910::acquire_mame(blip_buffer_t** bb, size_t len) {
  thread_local short ayBuf[3];

  for (int i=0; i<3; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t i=0; i<len; i++) {
    int advance=len-i;
    bool careAboutEnv=false;
    bool careAboutNoise=false;
    // heuristic
    if (!writes.empty()) {
      advance=1;
    } else {
      for (int j=0; j<3; j++) {
        // tone counter
        if (!ay->tone_enable(j) && ay->m_tone[j].volume!=0) {
          const int period=MAX(1,ay->m_tone[j].period)*(ay->m_step_mul<<1);
          const int remain=(period-ay->m_tone[j].count)>>1;
          if (remain<advance) {
            advance=remain;
          }
        }

        // count me in if I have noise enabled
        if (!ay->noise_enable(j) && ay->m_tone[j].volume!=0) {
          careAboutNoise=true;
        }

        // envelope check
        if (ay->m_tone[j].volume&16) {
          careAboutEnv=true;
        }

        // DAC
        if (chan[j].active && (chan[j].curPSGMode.val&8) && chan[j].dac.sample!=-1) {
          if (chan[j].dac.rate<=0) continue;
          const int remainTime=(rate-chan[j].dac.period+chan[j].dac.rate-1)/chan[j].dac.rate;
          if (remainTime<advance) advance=remainTime;
        }

        // TFX
        if (chan[j].active && (chan[j].curPSGMode.val&16) && !(chan[j].curPSGMode.val&8) && chan[j].tfx.mode!=-1) {
          const int remainTime=chan[j].tfx.period-chan[j].tfx.counter;
          if (remainTime<advance) advance=remainTime;
        }

        if (advance<=1) break;
      }
      // envelope
      if (careAboutEnv) {
        if (ay->m_envelope[0].holding==0) {
          const int periodEnv=MAX(1,ay->m_envelope[0].period)*ay->m_env_step_mul;
          const int remainEnv=periodEnv-ay->m_envelope[0].count;
          if (remainEnv<advance) {
            advance=remainEnv;
          }
        }
      }
      // noise
      if (careAboutNoise) {
        const int noisePeriod=((int)ay->noise_period())*ay->m_step_mul;
        const int noiseRemain=noisePeriod-ay->m_count_noise;
        if (noiseRemain<advance) {
          advance=noiseRemain;
        }
      }
    }

    if (advance<1) advance=1;

    runDAC(0,advance);
    runTFX(0,advance);
    checkWrites();

    ay->sound_stream_update(ayBuf,advance);
    i+=advance-1;

    if (sunsoft) {
      if (lastOut[0]!=ayBuf[0]) {
        blip_add_delta(bb[0],i,ayBuf[0]-lastOut[0]);
        blip_add_delta(bb[1],i,ayBuf[0]-lastOut[0]);
        lastOut[0]=ayBuf[0];
      }

      oscBuf[0]->putSample(i,CLAMP(sunsoftVolTable[31-(ay->lastIndx&31)]<<3,-32768,32767));
      oscBuf[1]->putSample(i,CLAMP(sunsoftVolTable[31-((ay->lastIndx>>5)&31)]<<3,-32768,32767));
      oscBuf[2]->putSample(i,CLAMP(sunsoftVolTable[31-((ay->lastIndx>>10)&31)]<<3,-32768,32767));
    } else {
      if (stereo) {
        int out0=ayBuf[0]+ayBuf[1]+((ayBuf[2]*stereoSep)>>8);
        int out1=((ayBuf[0]*stereoSep)>>8)+ayBuf[1]+ayBuf[2];
        if (lastOut[0]!=out0) {
          blip_add_delta(bb[0],i,out0-lastOut[0]);
          lastOut[0]=out0;
        }
        if (lastOut[1]!=out1) {
          blip_add_delta(bb[1],i,out1-lastOut[1]);
          lastOut[1]=out1;
        }
      } else {
        int out=ayBuf[0]+ayBuf[1]+ayBuf[2];
        if (lastOut[0]!=out) {
          blip_add_delta(bb[0],i,out-lastOut[0]);
          blip_add_delta(bb[1],i,out-lastOut[0]);
          lastOut[0]=out;
        }
      }

      oscBuf[0]->putSample(i,ayBuf[0]<<2);
      oscBuf[1]->putSample(i,ayBuf[1]<<2);
      oscBuf[2]->putSample(i,ayBuf[2]<<2);
    }
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformAY8910::acquire_atomic(short** buf, size_t len) {
  for (int i=0; i<3; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t i=0; i<len; i++) {
    runDAC(0,1);
    runTFX(0,1);

    if (!writes.empty()) {
      QueuedWrite w=writes.front();
      SSG_Write(&ay_atomic,w.addr&0x0f,w.val);
      regPool[w.addr&0x0f]=w.val;
      writes.pop();
    }

    SSG_Clock(&ay_atomic,0);
    SSG_Clock(&ay_atomic,1);

    if (stereo) {
      buf[0][i]=ay_atomic.o_analog[0]+ay_atomic.o_analog[1]+((ay_atomic.o_analog[2]*stereoSep)>>8);
      buf[1][i]=((ay_atomic.o_analog[0]*stereoSep)>>8)+ay_atomic.o_analog[1]+ay_atomic.o_analog[2];
    } else {
      buf[0][i]=ay_atomic.o_analog[0]+ay_atomic.o_analog[1]+ay_atomic.o_analog[2];
      buf[1][i]=buf[0][i];
    }

    oscBuf[0]->putSample(i,ay_atomic.o_analog[0]);
    oscBuf[1]->putSample(i,ay_atomic.o_analog[1]);
    oscBuf[2]->putSample(i,ay_atomic.o_analog[2]);
  }
  for (int i=0; i<3; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformAY8910::acquireDirect(blip_buffer_t** bb, size_t len) {
  if (selCore && !intellivision) return;
  acquire_mame(bb,len);
}

void DivPlatformAY8910::acquire(short** buf, size_t len) {
  if (selCore && !intellivision) {
    acquire_atomic(buf,len);
  }
}

void DivPlatformAY8910::fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
  writes.clear();
  for (size_t i=0; i<len; i++) {
    runDAC(sRate,1);
    runTFX(sRate,1);
    while (!writes.empty()) {
      QueuedWrite& w=writes.front();
      stream.push_back(DivDelayedWrite(i,w.addr,w.val));
      writes.pop_front();
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
      if (!(chan[i].nextPSGMode.val&8) || !(chan[i].nextPSGMode.val&16)) {
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
      if (!(chan[i].nextPSGMode.val&8)) {
        chan[i].nextPSGMode.val=chan[i].std.wave.val&7;
        chan[i].nextPSGMode.val|=(chan[i].curPSGMode.val&16);
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
        chan[i].tfx.counter = 0;
        chan[i].tfx.out = 0;
        if (chan[i].nextPSGMode.val&8) {
          //if (dumpWrites) addWrite(0xffff0002+(i<<8),0);
          if (chan[i].dac.sample<0 || chan[i].dac.sample>=parent->song.sampleLen) {
            if (dumpWrites) {
              rWrite(0x08+i,0);
              //addWrite(0xffff0000+(i<<8),chan[i].dac.sample);
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
    if (chan[i].std.ex4.had) {
      chan[i].fixedFreq=chan[i].std.ex4.val;
      chan[i].freqChanged=true;
    }
    if (chan[i].std.ex5.had) {
      ayEnvPeriod=chan[i].std.ex5.val;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
    }
    if (chan[i].std.ex6.had) {
      // 0 - disable timer
      // 1 - pwm
      // 2 - syncbuzzer
      switch (chan[i].std.ex6.val) {
        case 1:
          chan[i].nextPSGMode.val|=16;
          chan[i].tfx.mode = 0;
          break;
        case 2:
          chan[i].nextPSGMode.val|=16;
          chan[i].tfx.mode = 1;
          break;
        case 3:
          chan[i].nextPSGMode.val|=16;
          chan[i].tfx.mode = 2;
          break;
        default:
          chan[i].nextPSGMode.val|=16;
          chan[i].tfx.mode = -1; // this is a workaround!
          break;
      }
    }
    if (chan[i].std.ex7.had) {
      chan[i].tfx.offset=chan[i].std.ex7.val;
    }
    if (chan[i].std.ex8.had) {
      chan[i].tfx.num=chan[i].std.ex8.val;
      chan[i].freqChanged=true;
      if (!chan[i].std.fms.will) chan[i].tfx.den=1;
    }
    if (chan[i].std.fms.had) {
      chan[i].tfx.den=chan[i].std.fms.val;
      chan[i].freqChanged=true;
      if (!chan[i].std.ex8.will) chan[i].tfx.num=1;
    }
    if (chan[i].std.ams.had) {
      chan[i].tfx.lowBound=chan[i].std.ams.val;
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
        chan[i].dac.rate=((double)rate*((sunsoft||clockSel)?8.0:16.0))/(double)(MAX(1,off*chan[i].freq));
        //if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dac.rate);
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].fixedFreq>4095) chan[i].fixedFreq=4095;
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
        if (!(chan[i].nextPSGMode.val&8)) {
          chan[i].curPSGMode.val=chan[i].nextPSGMode.val;
        }
      }
      if (chan[i].keyOff) {
        chan[i].curPSGMode.val=0;
        rWrite(0x08+i,0);
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
        chan[c.chan].nextPSGMode.val|=8;
      } else if (chan[c.chan].dac.furnaceDAC) {
        chan[c.chan].nextPSGMode.val&=~8;
      }
      if (chan[c.chan].nextPSGMode.val&8) {
        if (skipRegisterWrites) break;
        if (!parent->song.disableSampleMacro && (ins->type==DIV_INS_AMIGA || ins->amiga.useSample)) {
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
            //if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            if (dumpWrites) {
              rWrite(0x08+c.chan,0);
              //addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dac.sample);
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
            //if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            //if (dumpWrites) addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dac.sample);
          }
          if (chan[c.chan].dac.setPos) {
            chan[c.chan].dac.setPos=false;
          } else {
            chan[c.chan].dac.pos=0;
          }
          chan[c.chan].dac.period=0;
          chan[c.chan].dac.rate=parent->getSample(chan[c.chan].dac.sample)->rate*2048;
          if (dumpWrites) {
            rWrite(0x08+c.chan,0);
            //addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dac.rate);
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
      //if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
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
      if (!(chan[c.chan].nextPSGMode.val&8)) {
        chan[c.chan].nextPSGMode.val|=16;
        chan[c.chan].tfx.mode=(((c.value&0xf0)>>4)&3)-1;
        if ((c.value&15)<16) {
          chan[c.chan].nextPSGMode.val=(c.value+1)&7;
          chan[c.chan].nextPSGMode.val|=chan[c.chan].curPSGMode.val&16;
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
        chan[c.chan].nextPSGMode.val|=4;
      } else {
        chan[c.chan].nextPSGMode.val&=~4;
      }
      if (!(chan[c.chan].nextPSGMode.val&8) && chan[c.chan].active) {
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
    case DIV_CMD_AY_NOISE_MASK_AND:
      chan[c.chan].tfx.num=c.value>>4;
      chan[c.chan].tfx.den=c.value&15;
      break;
    case DIV_CMD_AY_AUTO_PWM: {
      // best way i could find to do signed :/
      signed char signVal=c.value;
      chan[c.chan].tfx.offset=signVal;
      break;
    }
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
      //if (dumpWrites) addWrite(0xffff0005,chan[c.chan].dac.pos);
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
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
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
  } else if (chan[ch].active && (chan[ch].nextPSGMode.val&8)) {
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
    chan[i].curPSGMode.val&=~8;
    chan[i].nextPSGMode.val&=~8;
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
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

DivSamplePos DivPlatformAY8910::getSamplePos(int ch) {
  if (ch>=3) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dac.sample,
    chan[ch].dac.pos,
    chan[ch].dac.rate
  );
}

DivDispatchOscBuffer* DivPlatformAY8910::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformAY8910::mapVelocity(int ch, float vel) {
  return round(15.0*pow(vel,0.33));
}

float DivPlatformAY8910::getGain(int ch, int vol) {
  if (vol==0) return 0;
  return 1.0/pow(10.0,(float)(15-vol)*2.0/20.0);
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

  SSG_Reset(&ay_atomic);
  SSG_SetType(&ay_atomic,
    (yamaha?0:1)|
    (intellivision?2:0)
  );

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
  lastOut[0]=0;
  lastOut[1]=0;

  ioPortA=false;
  ioPortB=false;
  portAVal=0;
  portBVal=0;
}

int DivPlatformAY8910::getOutputCount() {
  return 2;
}

bool DivPlatformAY8910::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformAY8910::hasAcquireDirect() {
  return (!selCore || intellivision);
}

bool DivPlatformAY8910::getLegacyAlwaysSetVolume() {
  return false;
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

void DivPlatformAY8910::setCore(unsigned char core) {
  selCore=core;
}

void DivPlatformAY8910::setFlags(const DivConfig& flags) {
  if (extMode) {
    chipClock=extClock;
    rate=chipClock/extDiv;
    clockSel=false;
    dacRate=chipClock/dacRateDiv;
  } else {
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
  }

  if (ay!=NULL) delete ay;
  switch (flags.getInt("chipType",0)) {
    case 1:
      clockSel=flags.getBool("halfClock",false);
      ay=new ym2149_device(rate,clockSel);
      yamaha=true;
      sunsoft=false;
      intellivision=false;
      break;
    case 2:
      ay=new sunsoft_5b_sound_device(rate);
      yamaha=true;
      sunsoft=true;
      intellivision=false;
      clockSel=false;
      break;
    case 3:
      ay=new ay8914_device(rate);
      yamaha=false;
      sunsoft=false;
      intellivision=true;
      clockSel=false;
      break;
    default:
      ay=new ay8910_device(rate);
      yamaha=false;
      sunsoft=false;
      intellivision=false;
      clockSel=false;
      break;
  }
  ay->device_start();
  ay->device_reset();

  if (selCore && !intellivision) {
    rate=chipClock/2;
    dacRate=chipClock*2;
    if (sunsoft || clockSel) {
      rate=chipClock/4;
      dacRate=chipClock/2;
    }
  } else {
    rate=chipClock/8;
    dacRate=rate;
  }

  for (int i=0; i<3; i++) {
    oscBuf[i]->setRate(rate);
  }

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
  reset();
  return 3;
}

void DivPlatformAY8910::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
  if (ay!=NULL) delete ay;
}
