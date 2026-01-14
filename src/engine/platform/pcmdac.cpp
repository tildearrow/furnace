/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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
  int outSum[2];

  // do not process if our channels are null
  if (chan==NULL) {
    for (size_t h=0; h<len; h++) {
      buf[0][h]=0;
      buf[1][h]=0;
    }
    return;
  }

  for (int i=0; i<chans; i++) {
    oscBuf[i].begin(len);
  }

  for (size_t h=0; h<len; h++) {
    outSum[0]=0;
    outSum[1]=0;

    for (int i=0; i<chans; i++) {
      output=0;
      if (!chan[i].active) {
        oscBuf[i].putSample(h,0);
        continue;
      }
      if (chan[i].useWave || (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen)) {
        chan[i].audSub+=chan[i].freq;
        if (chan[i].useWave) {
          while (chan[i].audSub>=0x10000) {
            chan[i].audSub-=0x10000;
            chan[i].audPos+=((!chan[i].useWave) && chan[i].audDir)?-1:1;
            if (chan[i].audPos>=(int)chan[i].audLen) {
              chan[i].audPos%=chan[i].audLen;
              chan[i].audDir=false;
            }
            chan[i].audDat[0]=chan[i].audDat[1];
            chan[i].audDat[1]=chan[i].audDat[2];
            chan[i].audDat[2]=chan[i].audDat[3];
            chan[i].audDat[3]=chan[i].audDat[4];
            chan[i].audDat[4]=chan[i].audDat[5];
            chan[i].audDat[5]=chan[i].audDat[6];
            chan[i].audDat[6]=chan[i].audDat[7];
            chan[i].audDat[7]=(chan[i].ws.output[chan[i].audPos]-0x80)<<8;
          }

          const short s0=chan[i].audDat[0];
          const short s1=chan[i].audDat[1];
          const short s2=chan[i].audDat[2];
          const short s3=chan[i].audDat[3];
          const short s4=chan[i].audDat[4];
          const short s5=chan[i].audDat[5];
          const short s6=chan[i].audDat[6];
          const short s7=chan[i].audDat[7];

          switch (interp) {
            case 1: // linear
              output=s6+(((int)((int)s7-(int)s6)*((chan[i].audSub>>1)&0x7fff))>>15);
              break;
            case 2: { // cubic
              float* cubicTable=DivFilterTables::getCubicTable();
              float* t=&cubicTable[((chan[i].audSub&0xffff)>>6)<<2];
              float result=(float)s4*t[0]+(float)s5*t[1]+(float)s6*t[2]+(float)s7*t[3];
              if (result<-32768) result=-32768;
              if (result>32767) result=32767;
              output=result;
              break;
            }
            case 3: { // sinc
              float* sincTable=DivFilterTables::getSincTable8();
              float* t1=&sincTable[(8191-((chan[i].audSub&0xffff)>>3))<<2];
              float* t2=&sincTable[((chan[i].audSub&0xffff)>>3)<<2];
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
          DivSample* s=parent->getSample(chan[i].sample);
          if (s->samples>0) {
            while (chan[i].audSub>=0x10000) {
              chan[i].audSub-=0x10000;
              chan[i].audPos+=((!chan[i].useWave) && chan[i].audDir)?-1:1;
              if (chan[i].audDir) {
                if (s->isLoopable()) {
                  switch (s->loopMode) {
                    case DIV_SAMPLE_LOOP_FORWARD:
                    case DIV_SAMPLE_LOOP_PINGPONG:
                      if (chan[i].audPos<s->loopStart) {
                        chan[i].audPos=s->loopStart+(s->loopStart-chan[i].audPos);
                        chan[i].audDir=false;
                      }
                      break;
                    case DIV_SAMPLE_LOOP_BACKWARD:
                      if (chan[i].audPos<s->loopStart) {
                        chan[i].audPos=s->loopEnd-1-(s->loopStart-chan[i].audPos);
                        chan[i].audDir=true;
                      }
                      break;
                    default:
                      if (chan[i].audPos<0) {
                        chan[i].sample=-1;
                      }
                      break;
                  }
                } else if (chan[i].audPos>=(int)s->samples) {
                  chan[i].sample=-1;
                }
              } else {
                if (s->isLoopable()) {
                  switch (s->loopMode) {
                    case DIV_SAMPLE_LOOP_FORWARD:
                      if (chan[i].audPos>=s->loopEnd) {
                        chan[i].audPos=(chan[i].audPos+s->loopStart)-s->loopEnd;
                        chan[i].audDir=false;
                      }
                      break;
                    case DIV_SAMPLE_LOOP_BACKWARD:
                    case DIV_SAMPLE_LOOP_PINGPONG:
                      if (chan[i].audPos>=s->loopEnd) {
                        chan[i].audPos=s->loopEnd-1-(s->loopEnd-1-chan[i].audPos);
                        chan[i].audDir=true;
                      }
                      break;
                    default:
                      if (chan[i].audPos>=(int)s->samples) {
                        chan[i].sample=-1;
                      }
                      break;
                  }
                } else if (chan[i].audPos>=(int)s->samples) {
                  chan[i].sample=-1;
                }
              }
              chan[i].audDat[0]=chan[i].audDat[1];
              chan[i].audDat[1]=chan[i].audDat[2];
              chan[i].audDat[2]=chan[i].audDat[3];
              chan[i].audDat[3]=chan[i].audDat[4];
              chan[i].audDat[4]=chan[i].audDat[5];
              chan[i].audDat[5]=chan[i].audDat[6];
              chan[i].audDat[6]=chan[i].audDat[7];
              if (chan[i].audPos>=0 && chan[i].audPos<(int)s->samples) {
                chan[i].audDat[7]=s->data16[chan[i].audPos];
              } else {
                chan[i].audDat[7]=0;
              }
            }
          } else {
            chan[i].sample=-1;
            chan[i].audSub=0;
            chan[i].audPos=0;
          }

          const short s0=chan[i].audDat[0];
          const short s1=chan[i].audDat[1];
          const short s2=chan[i].audDat[2];
          const short s3=chan[i].audDat[3];
          const short s4=chan[i].audDat[4];
          const short s5=chan[i].audDat[5];
          const short s6=chan[i].audDat[6];
          const short s7=chan[i].audDat[7];

          switch (interp) {
            case 1: // linear
              output=s6+(((int)((int)s7-(int)s6)*((chan[i].audSub>>1)&0x7fff))>>15);
              break;
            case 2: { // cubic
              float* cubicTable=DivFilterTables::getCubicTable();
              float* t=&cubicTable[((chan[i].audSub&0xffff)>>6)<<2];
              float result=(float)s4*t[0]+(float)s5*t[1]+(float)s6*t[2]+(float)s7*t[3];
              if (result<-32768) result=-32768;
              if (result>32767) result=32767;
              output=result;
              break;
            }
            case 3: { // sinc
              float* sincTable=DivFilterTables::getSincTable8();
              float* t1=&sincTable[(8191-((chan[i].audSub&0xffff)>>3))<<2];
              float* t2=&sincTable[((chan[i].audSub&0xffff)>>3)<<2];
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
      if (isMuted[i]) {
        output=0;
      } else {
        output=((output*MIN(volMax,chan[i].vol)*MIN(chan[i].envVol,64))>>6)/volMax;
      }
      oscBuf[i].putSample(h,((output>>depthScale)<<depthScale)>>1);
      if (outStereo) {
        outSum[0]+=((output*chan[i].panL)>>(depthScale+8))<<depthScale;
        outSum[1]+=((output*chan[i].panR)>>(depthScale+8))<<depthScale;
      } else {
        output=(output>>depthScale)<<depthScale;
        outSum[0]+=output;
        outSum[1]+=output;
      }
    }

    outSum[0]=(int)((float)outSum[0]*volMult);
    outSum[1]=(int)((float)outSum[1]*volMult);

    if (outSum[0]<-32768) outSum[0]=-32768;
    if (outSum[0]>32767) outSum[0]=32767;
    if (outSum[1]<-32768) outSum[1]=-32768;
    if (outSum[1]>32767) outSum[1]=32767;

    buf[0][h]=outSum[0];
    buf[1][h]=outSum[1];
  }

  for (int i=0; i<chans; i++) {
    oscBuf[i].end(len);
  }
}

void DivPlatformPCMDAC::tick(bool sysTick) {
  for (int i=0; i<chans; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].envVol=chan[i].std.vol.val;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].useWave && chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].useWave && chan[i].active) {
      chan[i].ws.tick();
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
    if (chan[i].std.panL.had) {
      int val=chan[i].std.panL.val&0x7f;
      chan[i].panL=val*2;
    }
    if (chan[i].std.panR.had) {
      int val=chan[i].std.panR.val&0x7f;
      chan[i].panR=val*2;
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].audDir=false;
        chan[i].audPos=0;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_AMIGA);
      double off=1.0;
      if (!chan[i].useWave && chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[i].sample);
        off=(s->centerRate>=1)?((double)s->centerRate/parent->getCenterRate()):1.0;
      }
      chan[i].freq=off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq>16777215) chan[i].freq=16777215;
      if (chan[i].keyOn) {
        if (!chan[i].std.vol.had) {
          chan[i].envVol=64;
        }
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chan[i].keyOff=false;
      }
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformPCMDAC::dispatch(DivCommand c) {
  if (c.chan>=chans) return 0;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      if (ins->amiga.useWave) {
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
        chan[c.chan].useWave=true;
        chan[c.chan].audLen=ins->amiga.waveLen+1;
        if (chan[c.chan].insChanged) {
          if (chan[c.chan].wave<0) {
            chan[c.chan].wave=0;
            chan[c.chan].ws.setWidth(chan[c.chan].audLen);
            chan[c.chan].ws.changeWave1(chan[c.chan].wave);
          }
        }
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
        }
        chan[c.chan].useWave=false;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value));
      }
      if (chan[c.chan].useWave || chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
      }
      if (chan[c.chan].setPos) {
        chan[c.chan].setPos=false;
      } else {
        chan[c.chan].audDir=false;
        chan[c.chan].audPos=0;
      }
      chan[c.chan].audSub=0;
      memset(chan[c.chan].audDat,0,8*sizeof(short));
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].envVol=64;
      }
      if (chan[c.chan].useWave) {
        chan[c.chan].ws.init(ins,chan[c.chan].audLen,255,chan[c.chan].insChanged);
      }
      chan[c.chan].insChanged=false;
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].envVol=64;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].panL=c.value;
      chan[c.chan].panR=c.value2;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      if (!chan[c.chan].useWave) break;
      chan[c.chan].wave=c.value;
      chan[c.chan].keyOn=true;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=round(NOTE_FREQUENCY(c.value2+chan[c.chan].sampleNoteDelta));
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
      chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0))));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      if (chan[c.chan].useWave) break;
      chan[c.chan].audPos=c.value;
      chan[c.chan].setPos=true;
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
  if (ch>=chans) return;
  isMuted[ch]=mute;
}

void DivPlatformPCMDAC::forceIns() {
  for (int i=0; i<chans; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].audDir=false;
    chan[i].audPos=0;
    chan[i].sample=-1;
  }
}

void* DivPlatformPCMDAC::getChanState(int ch) {
  if (ch>=chans) return NULL;
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformPCMDAC::getOscBuffer(int ch) {
  if (ch>=chans) return NULL;
  return &oscBuf[ch];
}

void DivPlatformPCMDAC::reset() {
  for (int i=0; i<chans; i++) {
    chan[i]=DivPlatformPCMDAC::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,255);
    memset(chan[i].audDat,0,8*sizeof(short));
  }
}

int DivPlatformPCMDAC::getOutputCount() {
  return 2;
}

bool DivPlatformPCMDAC::hasSoftPan(int ch) {
  return outStereo;
}

DivMacroInt* DivPlatformPCMDAC::getChanMacroInt(int ch) {
  if (ch>=chans) return NULL;
  return &chan[ch].std;
}

unsigned short DivPlatformPCMDAC::getPan(int ch) {
  if (ch>=chans) return 0;
  return (chan[ch].panL<<8)|chan[ch].panR;
}

DivSamplePos DivPlatformPCMDAC::getSamplePos(int ch) {
  if (ch>=chans) return DivSamplePos();
  return DivSamplePos(
    chan[ch].sample,
    chan[ch].audPos,
    chan[ch].freq
  );
}

void DivPlatformPCMDAC::notifyInsChange(int ins) {
  for (int i=0; i<chans; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformPCMDAC::notifyWaveChange(int wave) {
  for (int i=0; i<chans; i++) {
    if (chan[i].useWave && chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
    }
  }
}

void DivPlatformPCMDAC::notifyInsDeletion(void* ins) {
  for (int i=0; i<chans; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
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
  for (int i=0; i<chans; i++) {
    oscBuf[i].setRate(rate);
  }
  volMax=flags.getInt("volMax",255);
  if (volMax<1) volMax=1;
  volMult=flags.getFloat("volMult",1.0f);
  if (volMult<0.0f) volMult=0.0f;
  if (volMult>1.0f) volMult=1.0f;
}

int DivPlatformPCMDAC::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  oscBuf=new DivDispatchOscBuffer[channels];
  chan=new Channel[channels];
  isMuted=new bool[channels];
  chans=channels;
  for (int i=0; i<channels; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();
  return 1;
}

void DivPlatformPCMDAC::quit() {
  delete[] chan;
  delete[] isMuted;
  delete[] oscBuf;
  chan=NULL;
  isMuted=NULL;
  oscBuf=NULL;
  chans=0;
}
