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
#include "pcmdac.h"
#include "../engine.h"
#include <math.h>

// to ease the driver, freqency register is a 8.16 counter relative to output sample rate
#define CHIP_FREQBASE 65536

void DivPlatformPCMDAC::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  const int depthScale=(15-outDepth);
  int output=0;
  for (size_t h=start; h<start+len; h++) {
    if (!chan.active || isMuted) {
      bufL[h]=0;
      bufR[h]=0;
      oscBuf->data[oscBuf->needle++]=0;
      continue;
    }
    if (chan.useWave || (chan.sample>=0 && chan.sample<parent->song.sampleLen)) {
      chan.audPos+=((!chan.useWave) && chan.audDir)?-(chan.freq>>16):(chan.freq>>16);
      chan.audSub+=(chan.freq&0xffff);
      if (chan.audSub>=0x10000) {
        chan.audSub-=0x10000;
        chan.audPos+=((!chan.useWave) && chan.audDir)?-1:1;
      }
      if (chan.useWave) {
        if (chan.audPos>=(int)chan.audLen) {
          chan.audPos%=chan.audLen;
          chan.audDir=false;
        }
        output=(chan.ws.output[chan.audPos]-0x80)<<8;
      } else {
        DivSample* s=parent->getSample(chan.sample);
        if (s->samples>0) {
          if (chan.audDir) {
            if (s->isLoopable()) {
              switch (s->loopMode) {
                case DIV_SAMPLE_LOOP_FORWARD:
                case DIV_SAMPLE_LOOP_PINGPONG:
                  if (chan.audPos<s->loopStart) {
                    chan.audPos=s->loopStart+(s->loopStart-chan.audPos);
                    chan.audDir=false;
                  }
                  break;
                case DIV_SAMPLE_LOOP_BACKWARD:
                  if (chan.audPos<s->loopStart) {
                    chan.audPos=s->loopEnd-1-(s->loopStart-chan.audPos);
                    chan.audDir=true;
                  }
                  break;
                default:
                  if (chan.audPos<0) {
                    chan.sample=-1;
                  }
                  break;
              }
            } else if (chan.audPos>=(int)s->samples) {
              chan.sample=-1;
            }
          } else {
            if (s->isLoopable()) {
              switch (s->loopMode) {
                case DIV_SAMPLE_LOOP_FORWARD:
                  if (chan.audPos>=s->loopEnd) {
                    chan.audPos=(chan.audPos+s->loopStart)-s->loopEnd;
                    chan.audDir=false;
                  }
                  break;
                case DIV_SAMPLE_LOOP_BACKWARD:
                case DIV_SAMPLE_LOOP_PINGPONG:
                  if (chan.audPos>=s->loopEnd) {
                    chan.audPos=s->loopEnd-1-(s->loopEnd-1-chan.audPos);
                    chan.audDir=true;
                  }
                  break;
                default:
                  if (chan.audPos>=(int)s->samples) {
                    chan.sample=-1;
                  }
                  break;
              }
            } else if (chan.audPos>=(int)s->samples) {
              chan.sample=-1;
            }
          }
          if (chan.audPos>=0 && chan.audPos<(int)s->samples) {
            output=s->data16[chan.audPos];
          }
        } else {
          chan.sample=-1;
        }
      }
    }
    output=output*chan.vol*chan.envVol/16384;
    oscBuf->data[oscBuf->needle++]=output;
    if (outStereo) {
      bufL[h]=((output*chan.panL)>>(depthScale+8))<<depthScale;
      bufR[h]=((output*chan.panR)>>(depthScale+8))<<depthScale;
    } else {
      output=(output>>depthScale)<<depthScale;
      bufL[h]=output;
      bufR[h]=output;
    }
  }
}

void DivPlatformPCMDAC::tick(bool sysTick) {
  chan.std.next();
  if (chan.std.vol.had) {
    chan.envVol=chan.std.vol.val;
  }
  if (chan.std.arp.had) {
    if (!chan.inPorta) {
      chan.baseFreq=NOTE_FREQUENCY(parent->calcArp(chan.note,chan.std.arp.val));
    }
    chan.freqChanged=true;
  }
  if (chan.useWave && chan.std.wave.had) {
    if (chan.wave!=chan.std.wave.val || chan.ws.activeChanged()) {
      chan.wave=chan.std.wave.val;
      chan.ws.changeWave1(chan.wave);
      if (!chan.keyOff) chan.keyOn=true;
    }
  }
  if (chan.useWave && chan.active) {
    chan.ws.tick();
  }
  if (chan.std.pitch.had) {
    if (chan.std.pitch.mode) {
      chan.pitch2+=chan.std.pitch.val;
      CLAMP_VAR(chan.pitch2,-32768,32767);
    } else {
      chan.pitch2=chan.std.pitch.val;
    }
    chan.freqChanged=true;
  }
  if (chan.std.panL.had) {
    int val=chan.std.panL.val&0x7f;
    chan.panL=val*2;
  }
  if (chan.std.panR.had) {
    int val=chan.std.panR.val&0x7f;
    chan.panR=val*2;
  }
  if (chan.std.phaseReset.had) {
    if (chan.std.phaseReset.val==1) {
      chan.audDir=false;
      chan.audPos=0;
    }
  }
  if (chan.freqChanged || chan.keyOn || chan.keyOff) {
    //DivInstrument* ins=parent->getIns(chan.ins,DIV_INS_AMIGA);
    double off=1.0;
    if (!chan.useWave && chan.sample>=0 && chan.sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan.sample);
      off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
    }
    chan.freq=off*parent->calcFreq(chan.baseFreq,chan.pitch,false,2,chan.pitch2,chipClock,CHIP_FREQBASE);
    if (chan.freq>16777215) chan.freq=16777215;
    if (chan.keyOn) {
      if (!chan.std.vol.had) {
        chan.envVol=64;
      }
      chan.keyOn=false;
    }
    if (chan.keyOff) {
      chan.keyOff=false;
    }
    chan.freqChanged=false;
  }
}

int DivPlatformPCMDAC::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan.ins,DIV_INS_AMIGA);
      if (ins->amiga.useWave) {
        chan.useWave=true;
        chan.audLen=ins->amiga.waveLen+1;
        if (chan.insChanged) {
          if (chan.wave<0) {
            chan.wave=0;
            chan.ws.setWidth(chan.audLen);
            chan.ws.changeWave1(chan.wave);
          }
        }
      } else {
        chan.sample=ins->amiga.getSample(c.value);
        chan.useWave=false;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan.baseFreq=round(NOTE_FREQUENCY(c.value));
      }
      if (chan.useWave || chan.sample<0 || chan.sample>=parent->song.sampleLen) {
        chan.sample=-1;
      }
      if (chan.setPos) {
        chan.setPos=false;
      } else {
        chan.audDir=false;
        chan.audPos=0;
      }
      chan.audSub=0;
      if (c.value!=DIV_NOTE_NULL) {
        chan.freqChanged=true;
        chan.note=c.value;
      }
      chan.active=true;
      chan.keyOn=true;
      chan.macroInit(ins);
      if (!parent->song.brokenOutVol && !chan.std.vol.will) {
        chan.envVol=64;
      }
      if (chan.useWave) {
        chan.ws.init(ins,chan.audLen,255,chan.insChanged);
      }
      chan.insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan.sample=-1;
      chan.active=false;
      chan.keyOff=true;
      chan.macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan.std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan.ins!=c.value || c.value2==1) {
        chan.ins=c.value;
        chan.insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan.vol!=c.value) {
        chan.vol=c.value;
        if (!chan.std.vol.has) {
          chan.envVol=64;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan.vol;
      break;
    case DIV_CMD_PANNING:
      chan.panL=c.value;
      chan.panR=c.value2;
      break;
    case DIV_CMD_PITCH:
      chan.pitch=c.value;
      chan.freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      if (!chan.useWave) break;
      chan.wave=c.value;
      chan.keyOn=true;
      chan.ws.changeWave1(chan.wave);
      break;
    case DIV_CMD_NOTE_PORTA: {
      DivInstrument* ins=parent->getIns(chan.ins,DIV_INS_AMIGA);
      chan.sample=ins->amiga.getSample(c.value2);
      int destFreq=round(NOTE_FREQUENCY(c.value2));
      bool return2=false;
      if (destFreq>chan.baseFreq) {
        chan.baseFreq+=c.value;
        if (chan.baseFreq>=destFreq) {
          chan.baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan.baseFreq-=c.value;
        if (chan.baseFreq<=destFreq) {
          chan.baseFreq=destFreq;
          return2=true;
        }
      }
      chan.freqChanged=true;
      if (return2) {
        chan.inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan.baseFreq=round(NOTE_FREQUENCY(c.value+((chan.std.arp.will && !chan.std.arp.mode)?(chan.std.arp.val):(0))));
      chan.freqChanged=true;
      chan.note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan.active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan.macroInit(parent->getIns(chan.ins,DIV_INS_AMIGA));
      }
      chan.inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      if (chan.useWave) break;
      chan.audPos=c.value;
      chan.setPos=true;
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

void DivPlatformPCMDAC::muteChannel(int ch, bool mute) {
  isMuted=mute;
}

void DivPlatformPCMDAC::forceIns() {
  chan.insChanged=true;
  chan.freqChanged=true;
  chan.audDir=false;
  chan.audPos=0;
  chan.sample=-1;
}

void* DivPlatformPCMDAC::getChanState(int ch) {
  return &chan;
}

DivDispatchOscBuffer* DivPlatformPCMDAC::getOscBuffer(int ch) {
  return oscBuf;
}

void DivPlatformPCMDAC::reset() {
  chan=DivPlatformPCMDAC::Channel();
  chan.std.setEngine(parent);
  chan.ws.setEngine(parent);
  chan.ws.init(NULL,32,255);
}

bool DivPlatformPCMDAC::isStereo() {
  return true;
}

DivMacroInt* DivPlatformPCMDAC::getChanMacroInt(int ch) {
  return &chan.std;
}

void DivPlatformPCMDAC::notifyInsChange(int ins) {
  if (chan.ins==ins) {
    chan.insChanged=true;
  }
}

void DivPlatformPCMDAC::notifyWaveChange(int wave) {
  if (chan.useWave && chan.wave==wave) {
    chan.ws.changeWave1(wave);
  }
}

void DivPlatformPCMDAC::notifyInsDeletion(void* ins) {
  chan.std.notifyInsDeletion((DivInstrument*)ins);
}

void DivPlatformPCMDAC::setFlags(const DivConfig& flags) {
  // default to 44100Hz 16-bit stereo
  rate=flags.getInt("rate",44100);
  // rate can't be too low or the resampler will break
  if (rate<1000) rate=1000;
  chipClock=rate;
  outDepth=(flags.getInt("outDepth",15))&15;
  outStereo=flags.getBool("stereo",true);
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
