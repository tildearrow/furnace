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

#define _USE_MATH_DEFINES
#include "pcmdac.h"
#include "../engine.h"
#include "../filter.h"
#include <math.h>

// to ease the driver, freqency register is a 8.16 counter relative to output sample rate
#define CHIP_FREQBASE 65536

void DivPlatformPCMDAC::acquire(short** buf, size_t len) {
  const int depthScale=(15-outDepth);
  int output=0;

  oscBuf->begin(len);

  for (size_t h=0; h<len; h++) {
    if (!chan[0].active) {
      buf[0][h]=0;
      buf[1][h]=0;
      oscBuf->putSample(h,0);
      continue;
    }
    if (chan[0].useWave || (chan[0].sample>=0 && chan[0].sample<parent->song.sampleLen)) {
      chan[0].audSub+=chan[0].freq;
      if (chan[0].useWave) {
        while (chan[0].audSub>=0x10000) {
          chan[0].audSub-=0x10000;
          chan[0].audPos+=((!chan[0].useWave) && chan[0].audDir)?-1:1;
          if (chan[0].audPos>=(int)chan[0].audLen) {
            chan[0].audPos%=chan[0].audLen;
            chan[0].audDir=false;
          }
          chan[0].audDat[0]=chan[0].audDat[1];
          chan[0].audDat[1]=chan[0].audDat[2];
          chan[0].audDat[2]=chan[0].audDat[3];
          chan[0].audDat[3]=chan[0].audDat[4];
          chan[0].audDat[4]=chan[0].audDat[5];
          chan[0].audDat[5]=chan[0].audDat[6];
          chan[0].audDat[6]=chan[0].audDat[7];
          chan[0].audDat[7]=(chan[0].ws.output[chan[0].audPos]-0x80)<<8;
        }

        const short s0=chan[0].audDat[0];
        const short s1=chan[0].audDat[1];
        const short s2=chan[0].audDat[2];
        const short s3=chan[0].audDat[3];
        const short s4=chan[0].audDat[4];
        const short s5=chan[0].audDat[5];
        const short s6=chan[0].audDat[6];
        const short s7=chan[0].audDat[7];

        switch (interp) {
          case 1: // linear
            output=s6+(((int)((int)s7-(int)s6)*((chan[0].audSub>>1)&0x7fff))>>15);
            break;
          case 2: { // cubic
            float* cubicTable=DivFilterTables::getCubicTable();
            float* t=&cubicTable[((chan[0].audSub&0xffff)>>6)<<2];
            float result=(float)s4*t[0]+(float)s5*t[1]+(float)s6*t[2]+(float)s7*t[3];
            if (result<-32768) result=-32768;
            if (result>32767) result=32767;
            output=result;
            break;
          }
          case 3: { // sinc
            float* sincTable=DivFilterTables::getSincTable8();
            float* t1=&sincTable[(8191-((chan[0].audSub&0xffff)>>3))<<2];
            float* t2=&sincTable[((chan[0].audSub&0xffff)>>3)<<2];
            float result=(
              s0*t2[3]+
              s1*t2[2]+
              s2*t2[1]+
              s3*t2[0]+
              s4*t1[0]+
              s5*t1[1]+
              s6*t1[2]+
              s7*t1[3]
            );
            if (result<-32768) result=-32768;
            if (result>32767) result=32767;
            output=result;
            break;
          }
          default: // none
            output=s7;
            break;
        }
      } else {
        DivSample* s=parent->getSample(chan[0].sample);
        if (s->samples>0) {
          while (chan[0].audSub>=0x10000) {
            chan[0].audSub-=0x10000;
            chan[0].audPos+=((!chan[0].useWave) && chan[0].audDir)?-1:1;
            if (chan[0].audDir) {
              if (s->isLoopable()) {
                switch (s->loopMode) {
                  case DIV_SAMPLE_LOOP_FORWARD:
                  case DIV_SAMPLE_LOOP_PINGPONG:
                    if (chan[0].audPos<s->loopStart) {
                      chan[0].audPos=s->loopStart+(s->loopStart-chan[0].audPos);
                      chan[0].audDir=false;
                    }
                    break;
                  case DIV_SAMPLE_LOOP_BACKWARD:
                    if (chan[0].audPos<s->loopStart) {
                      chan[0].audPos=s->loopEnd-1-(s->loopStart-chan[0].audPos);
                      chan[0].audDir=true;
                    }
                    break;
                  default:
                    if (chan[0].audPos<0) {
                      chan[0].sample=-1;
                    }
                    break;
                }
              } else if (chan[0].audPos>=(int)s->samples) {
                chan[0].sample=-1;
              }
            } else {
              if (s->isLoopable()) {
                switch (s->loopMode) {
                  case DIV_SAMPLE_LOOP_FORWARD:
                    if (chan[0].audPos>=s->loopEnd) {
                      chan[0].audPos=(chan[0].audPos+s->loopStart)-s->loopEnd;
                      chan[0].audDir=false;
                    }
                    break;
                  case DIV_SAMPLE_LOOP_BACKWARD:
                  case DIV_SAMPLE_LOOP_PINGPONG:
                    if (chan[0].audPos>=s->loopEnd) {
                      chan[0].audPos=s->loopEnd-1-(s->loopEnd-1-chan[0].audPos);
                      chan[0].audDir=true;
                    }
                    break;
                  default:
                    if (chan[0].audPos>=(int)s->samples) {
                      chan[0].sample=-1;
                    }
                    break;
                }
              } else if (chan[0].audPos>=(int)s->samples) {
                chan[0].sample=-1;
              }
            }
            chan[0].audDat[0]=chan[0].audDat[1];
            chan[0].audDat[1]=chan[0].audDat[2];
            chan[0].audDat[2]=chan[0].audDat[3];
            chan[0].audDat[3]=chan[0].audDat[4];
            chan[0].audDat[4]=chan[0].audDat[5];
            chan[0].audDat[5]=chan[0].audDat[6];
            chan[0].audDat[6]=chan[0].audDat[7];
            if (chan[0].audPos>=0 && chan[0].audPos<(int)s->samples) {
              chan[0].audDat[7]=s->data16[chan[0].audPos];
            } else {
              chan[0].audDat[7]=0;
            }
          }
        } else {
          chan[0].sample=-1;
          chan[0].audSub=0;
          chan[0].audPos=0;
        }

        const short s0=chan[0].audDat[0];
        const short s1=chan[0].audDat[1];
        const short s2=chan[0].audDat[2];
        const short s3=chan[0].audDat[3];
        const short s4=chan[0].audDat[4];
        const short s5=chan[0].audDat[5];
        const short s6=chan[0].audDat[6];
        const short s7=chan[0].audDat[7];

        switch (interp) {
          case 1: // linear
            output=s6+(((int)((int)s7-(int)s6)*((chan[0].audSub>>1)&0x7fff))>>15);
            break;
          case 2: { // cubic
            float* cubicTable=DivFilterTables::getCubicTable();
            float* t=&cubicTable[((chan[0].audSub&0xffff)>>6)<<2];
            float result=(float)s4*t[0]+(float)s5*t[1]+(float)s6*t[2]+(float)s7*t[3];
            if (result<-32768) result=-32768;
            if (result>32767) result=32767;
            output=result;
            break;
          }
          case 3: { // sinc
            float* sincTable=DivFilterTables::getSincTable8();
            float* t1=&sincTable[(8191-((chan[0].audSub&0xffff)>>3))<<2];
            float* t2=&sincTable[((chan[0].audSub&0xffff)>>3)<<2];
            float result=(
              s0*t2[3]+
              s1*t2[2]+
              s2*t2[1]+
              s3*t2[0]+
              s4*t1[0]+
              s5*t1[1]+
              s6*t1[2]+
              s7*t1[3]
            );
            if (result<-32768) result=-32768;
            if (result>32767) result=32767;
            output=result;
            break;
          }
          default: // none
            output=s7;
            break;
        }
      }
    }
    if (isMuted) {
      output=0;
    } else {
      output=((output*MIN(volMax,chan[0].vol)*MIN(chan[0].envVol,64))>>6)/volMax;
    }
    oscBuf->putSample(h,((output>>depthScale)<<depthScale)>>1);
    if (outStereo) {
      buf[0][h]=((output*chan[0].panL)>>(depthScale+8))<<depthScale;
      buf[1][h]=((output*chan[0].panR)>>(depthScale+8))<<depthScale;
    } else {
      output=(output>>depthScale)<<depthScale;
      buf[0][h]=output;
      buf[1][h]=output;
    }
  }

  oscBuf->end(len);
}

void DivPlatformPCMDAC::tick(bool sysTick) {
  chan[0].std.next();
  if (chan[0].std.vol.had) {
    chan[0].envVol=chan[0].std.vol.val;
  }
  if (NEW_ARP_STRAT) {
    chan[0].handleArp();
  } else if (chan[0].std.arp.had) {
    if (!chan[0].inPorta) {
      chan[0].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[0].note,chan[0].std.arp.val));
    }
    chan[0].freqChanged=true;
  }
  if (chan[0].useWave && chan[0].std.wave.had) {
    if (chan[0].wave!=chan[0].std.wave.val || chan[0].ws.activeChanged()) {
      chan[0].wave=chan[0].std.wave.val;
      chan[0].ws.changeWave1(chan[0].wave);
      if (!chan[0].keyOff) chan[0].keyOn=true;
    }
  }
  if (chan[0].useWave && chan[0].active) {
    chan[0].ws.tick();
  }
  if (chan[0].std.pitch.had) {
    if (chan[0].std.pitch.mode) {
      chan[0].pitch2+=chan[0].std.pitch.val;
      CLAMP_VAR(chan[0].pitch2,-32768,32767);
    } else {
      chan[0].pitch2=chan[0].std.pitch.val;
    }
    chan[0].freqChanged=true;
  }
  if (chan[0].std.panL.had) {
    int val=chan[0].std.panL.val&0x7f;
    chan[0].panL=val*2;
  }
  if (chan[0].std.panR.had) {
    int val=chan[0].std.panR.val&0x7f;
    chan[0].panR=val*2;
  }
  if (chan[0].std.phaseReset.had) {
    if (chan[0].std.phaseReset.val==1) {
      chan[0].audDir=false;
      chan[0].audPos=0;
    }
  }
  if (chan[0].freqChanged || chan[0].keyOn || chan[0].keyOff) {
    //DivInstrument* ins=parent->getIns(chan[0].ins,DIV_INS_AMIGA);
    double off=1.0;
    if (!chan[0].useWave && chan[0].sample>=0 && chan[0].sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[0].sample);
      off=(s->centerRate>=1)?((double)s->centerRate/parent->getCenterRate()):1.0;
    }
    chan[0].freq=off*parent->calcFreq(chan[0].baseFreq,chan[0].pitch,chan[0].fixedArp?chan[0].baseNoteOverride:chan[0].arpOff,chan[0].fixedArp,false,2,chan[0].pitch2,chipClock,CHIP_FREQBASE);
    if (chan[0].freq>16777215) chan[0].freq=16777215;
    if (chan[0].keyOn) {
      if (!chan[0].std.vol.had) {
        chan[0].envVol=64;
      }
      chan[0].keyOn=false;
    }
    if (chan[0].keyOff) {
      chan[0].keyOff=false;
    }
    chan[0].freqChanged=false;
  }
}

int DivPlatformPCMDAC::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[0].ins,DIV_INS_AMIGA);
      if (ins->amiga.useWave) {
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
        chan[0].useWave=true;
        chan[0].audLen=ins->amiga.waveLen+1;
        if (chan[0].insChanged) {
          if (chan[0].wave<0) {
            chan[0].wave=0;
            chan[0].ws.setWidth(chan[0].audLen);
            chan[0].ws.changeWave1(chan[0].wave);
          }
        }
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[0].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
        }
        chan[0].useWave=false;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[0].baseFreq=round(NOTE_FREQUENCY(c.value));
      }
      if (chan[0].useWave || chan[0].sample<0 || chan[0].sample>=parent->song.sampleLen) {
        chan[0].sample=-1;
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
      }
      if (chan[0].setPos) {
        chan[0].setPos=false;
      } else {
        chan[0].audDir=false;
        chan[0].audPos=0;
      }
      chan[0].audSub=0;
      memset(chan[0].audDat,0,8*sizeof(short));
      if (c.value!=DIV_NOTE_NULL) {
        chan[0].freqChanged=true;
        chan[0].note=c.value;
      }
      chan[0].active=true;
      chan[0].keyOn=true;
      chan[0].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[0].std.vol.will) {
        chan[0].envVol=64;
      }
      if (chan[0].useWave) {
        chan[0].ws.init(ins,chan[0].audLen,255,chan[0].insChanged);
      }
      chan[0].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[0].sample=-1;
      chan[0].active=false;
      chan[0].keyOff=true;
      chan[0].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[0].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[0].ins!=c.value || c.value2==1) {
        chan[0].ins=c.value;
        chan[0].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[0].vol!=c.value) {
        chan[0].vol=c.value;
        if (!chan[0].std.vol.has) {
          chan[0].envVol=64;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[0].vol;
      break;
    case DIV_CMD_PANNING:
      chan[0].panL=c.value;
      chan[0].panR=c.value2;
      break;
    case DIV_CMD_PITCH:
      chan[0].pitch=c.value;
      chan[0].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      if (!chan[0].useWave) break;
      chan[0].wave=c.value;
      chan[0].keyOn=true;
      chan[0].ws.changeWave1(chan[0].wave);
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=round(NOTE_FREQUENCY(c.value2+chan[c.chan].sampleNoteDelta));
      bool return2=false;
      if (destFreq>chan[0].baseFreq) {
        chan[0].baseFreq+=c.value;
        if (chan[0].baseFreq>=destFreq) {
          chan[0].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[0].baseFreq-=c.value;
        if (chan[0].baseFreq<=destFreq) {
          chan[0].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[0].freqChanged=true;
      if (return2) {
        chan[0].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[0].baseFreq=round(NOTE_FREQUENCY(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[0].std.arp.val):(0))));
      chan[0].freqChanged=true;
      chan[0].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[0].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[0].macroInit(parent->getIns(chan[0].ins,DIV_INS_AMIGA));
      }
      chan[0].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      if (chan[0].useWave) break;
      chan[0].audPos=c.value;
      chan[0].setPos=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return volMax;
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

void DivPlatformPCMDAC::muteChannel(int ch, bool mute) {
  isMuted=mute;
}

void DivPlatformPCMDAC::forceIns() {
  chan[0].insChanged=true;
  chan[0].freqChanged=true;
  chan[0].audDir=false;
  chan[0].audPos=0;
  chan[0].sample=-1;
}

void* DivPlatformPCMDAC::getChanState(int ch) {
  return &chan;
}

DivDispatchOscBuffer* DivPlatformPCMDAC::getOscBuffer(int ch) {
  return oscBuf;
}

void DivPlatformPCMDAC::reset() {
  chan[0]=DivPlatformPCMDAC::Channel();
  chan[0].std.setEngine(parent);
  chan[0].ws.setEngine(parent);
  chan[0].ws.init(NULL,32,255);
  memset(chan[0].audDat,0,8*sizeof(short));
}

int DivPlatformPCMDAC::getOutputCount() {
  return 2;
}

DivMacroInt* DivPlatformPCMDAC::getChanMacroInt(int ch) {
  return &chan[0].std;
}

unsigned short DivPlatformPCMDAC::getPan(int ch) {
  return (chan[0].panL<<8)|chan[0].panR;
}

DivSamplePos DivPlatformPCMDAC::getSamplePos(int ch) {
  if (ch>=1) return DivSamplePos();
  return DivSamplePos(
    chan[ch].sample,
    chan[ch].audPos,
    chan[ch].freq
  );
}

void DivPlatformPCMDAC::notifyInsChange(int ins) {
  if (chan[0].ins==ins) {
    chan[0].insChanged=true;
  }
}

void DivPlatformPCMDAC::notifyWaveChange(int wave) {
  if (chan[0].useWave && chan[0].wave==wave) {
    chan[0].ws.changeWave1(wave);
  }
}

void DivPlatformPCMDAC::notifyInsDeletion(void* ins) {
  chan[0].std.notifyInsDeletion((DivInstrument*)ins);
}

void DivPlatformPCMDAC::setFlags(const DivConfig& flags) {
  // default to 44100Hz 16-bit stereo
  rate=flags.getInt("rate",44100);
  // rate can't be too low or the resampler will break
  if (rate<1000) rate=1000;
  chipClock=rate;
  outDepth=(flags.getInt("outDepth",15))&15;
  outStereo=flags.getBool("stereo",true);
  interp=flags.getInt("interpolation",0);
  oscBuf->setRate(rate);
  volMax=flags.getInt("volMax",255);
  if (volMax<1) volMax=1;
}

int DivPlatformPCMDAC::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  oscBuf=new DivDispatchOscBuffer;
  isMuted=false;
  setFlags(flags);
  reset();
  return 1;
}

void DivPlatformPCMDAC::quit() {
  delete oscBuf;
}
