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

#include "segapcm.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

//#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
//#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

const char* DivPlatformSegaPCM::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x20:
      return "20xx: Set PCM frequency";
      break; 
  }
  return NULL;
}

void DivPlatformSegaPCM::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    // do a PCM cycle
    pcmL=0; pcmR=0;
    for (int i=0; i<16; i++) {
      if (chan[i].pcm.sample>=0 && chan[i].pcm.sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[i].pcm.sample);
        if (s->samples<=0) {
          chan[i].pcm.sample=-1;
          continue;
        }
        if (!isMuted[i]) {
          pcmL+=(s->data8[chan[i].pcm.pos>>8]*chan[i].outVolL);
          pcmR+=(s->data8[chan[i].pcm.pos>>8]*chan[i].outVolR);
        }
        chan[i].pcm.pos+=chan[i].pcm.freq;
        if (chan[i].pcm.pos>=(s->samples<<8)) {
          if (s->loopStart>=0 && s->loopStart<(int)s->samples) {
            chan[i].pcm.pos=s->loopStart<<8;
          } else {
            chan[i].pcm.sample=-1;
          }
        }
      }
    }

    os[0]=pcmL;
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=pcmR;
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformSegaPCM::calcAndWriteOutVol(int ch, int env) {
  if (env>64) env=64;
  chan[ch].outVolL=chan[ch].vol*((chan[ch].pan>>4)&0x0f)*env/960;
  chan[ch].outVolR=chan[ch].vol*(chan[ch].pan&0x0f)*env/960;
  if (dumpWrites) {
    addWrite(0x10002+(ch<<3),chan[ch].outVolL);
    addWrite(0x10003+(ch<<3),chan[ch].outVolR);
  }
}

void DivPlatformSegaPCM::tick() {
  for (int i=0; i<16; i++) {
    chan[i].std.next();

    if (chan[i].std.hadVol) {
      calcAndWriteOutVol(i,chan[i].std.vol);
    }

    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=(chan[i].std.arp<<6);
        } else {
          chan[i].baseFreq=((chan[i].note+(signed char)chan[i].std.arp)<<6);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=(chan[i].note<<6);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].keyOn) {
      if (!chan[i].std.hadVol) {
        calcAndWriteOutVol(i,64);
      }
      chan[i].keyOn=false;
    }
    /*if (chan[i].keyOn || chan[i].keyOff) {
      chan[i].keyOff=false;
    }*/
  }

  for (int i=0; i<16; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+(chan[i].pitch>>1)-64;
      if (chan[i].furnacePCM) {
        double off=1.0;
        if (chan[i].pcm.sample>=0 && chan[i].pcm.sample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].pcm.sample);
          off=(double)s->centerRate/8363.0;
        }
        chan[i].pcm.freq=MIN(255,(15625+(off*parent->song.tuning*pow(2.0,double(chan[i].freq+256)/(64.0*12.0)))*255)/31250);
        if (dumpWrites) {
          addWrite(0x10007+(i<<3),chan[i].pcm.freq);
        }
      }
      chan[i].freqChanged=false;
    }
  }
}

void DivPlatformSegaPCM::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

int DivPlatformSegaPCM::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (skipRegisterWrites) break;
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      if (ins->type==DIV_INS_AMIGA) {
        chan[c.chan].pcm.sample=ins->amiga.initSample;
        if (chan[c.chan].pcm.sample<0 || chan[c.chan].pcm.sample>=parent->song.sampleLen) {
          chan[c.chan].pcm.sample=-1;
          if (dumpWrites) {
            addWrite(0x10086+(c.chan<<3),3);
          }
          break;
        }
        chan[c.chan].pcm.pos=0;
        chan[c.chan].baseFreq=(c.value<<6);
        chan[c.chan].freqChanged=true;
        chan[c.chan].furnacePCM=true;
        if (dumpWrites) { // Sega PCM writes
          DivSample* s=parent->getSample(chan[c.chan].pcm.sample);
          addWrite(0x10086+(c.chan<<3),3+((s->offSegaPCM>>16)<<3));
          addWrite(0x10084+(c.chan<<3),(s->offSegaPCM)&0xff);
          addWrite(0x10085+(c.chan<<3),(s->offSegaPCM>>8)&0xff);
          addWrite(0x10006+(c.chan<<3),MIN(255,((s->offSegaPCM&0xffff)+s->length8-1)>>8));
          if (s->loopStart<0 || s->loopStart>=(int)s->length8) {
            addWrite(0x10086+(c.chan<<3),2+((s->offSegaPCM>>16)<<3));
          } else {
            int loopPos=(s->offSegaPCM&0xffff)+s->loopStart+s->loopOffP;
            addWrite(0x10004+(c.chan<<3),loopPos&0xff);
            addWrite(0x10005+(c.chan<<3),(loopPos>>8)&0xff);
            addWrite(0x10086+(c.chan<<3),((s->offSegaPCM>>16)<<3));
          }
        }
        chan[c.chan].std.init(ins);
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].note=c.value;
        }
        chan[c.chan].pcm.sample=12*sampleBank+chan[c.chan].note%12;
        if (chan[c.chan].pcm.sample>=parent->song.sampleLen) {
          chan[c.chan].pcm.sample=-1;
          if (dumpWrites) {
            addWrite(0x10086+(c.chan<<3),3);
          }
          break;
        }
        chan[c.chan].pcm.pos=0;
        chan[c.chan].pcm.freq=MIN(255,(parent->getSample(chan[c.chan].pcm.sample)->rate*255)/31250);
        chan[c.chan].furnacePCM=false;
        if (dumpWrites) { // Sega PCM writes
          DivSample* s=parent->getSample(chan[c.chan].pcm.sample);
          addWrite(0x10086+(c.chan<<3),3+((s->offSegaPCM>>16)<<3));
          addWrite(0x10084+(c.chan<<3),(s->offSegaPCM)&0xff);
          addWrite(0x10085+(c.chan<<3),(s->offSegaPCM>>8)&0xff);
          addWrite(0x10006+(c.chan<<3),MIN(255,((s->offSegaPCM&0xffff)+s->length8-1)>>8));
          if (s->loopStart<0 || s->loopStart>=(int)s->length8) {
            addWrite(0x10086+(c.chan<<3),2+((s->offSegaPCM>>16)<<3));
          } else {
            int loopPos=(s->offSegaPCM&0xffff)+s->loopStart+s->loopOffP;
            addWrite(0x10004+(c.chan<<3),loopPos&0xff);
            addWrite(0x10005+(c.chan<<3),(loopPos>>8)&0xff);
            addWrite(0x10086+(c.chan<<3),((s->offSegaPCM>>16)<<3));
          }
          addWrite(0x10007+(c.chan<<3),chan[c.chan].pcm.freq);
        }
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].pcm.sample=-1;
      if (dumpWrites) {
        addWrite(0x10086+(c.chan<<3),3);
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      // TODO Furnace/Deflemask volume behavior compat flag
      // for now, emulate Deflemask
      if (chan[c.chan].lastVol!=c.value) {
        chan[c.chan].lastVol=c.value;
        chan[c.chan].vol=c.value;
        chan[c.chan].pan=0xff;
        if (!chan[c.chan].std.hasVol) {
          calcAndWriteOutVol(c.chan,64);
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
    case DIV_CMD_PANNING: {
      chan[c.chan].vol=127;
      chan[c.chan].pan=c.value;
      calcAndWriteOutVol(c.chan,chan[c.chan].std.hasVol?chan[c.chan].std.vol:64);
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=(c.value2<<6);
      int newFreq;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*4;
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*4;
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].baseFreq=newFreq;
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=(c.value<<6);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    case DIV_CMD_SAMPLE_FREQ:
      chan[c.chan].pcm.freq=c.value;
      if (dumpWrites) {
        addWrite(0x10007+(c.chan<<3),chan[c.chan].pcm.freq);
      }
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformSegaPCM::forceIns() {
  for (int i=0; i<16; i++) {
    chan[i].insChanged=true;
  }
}

void DivPlatformSegaPCM::notifyInsChange(int ins) {
  for (int i=0; i<16; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void* DivPlatformSegaPCM::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformSegaPCM::getRegisterPool() {
  return regPool;
}

int DivPlatformSegaPCM::getRegisterPoolSize() {
  return 256;
}

void DivPlatformSegaPCM::poke(unsigned int addr, unsigned short val) {
  //immWrite(addr,val);
}

void DivPlatformSegaPCM::poke(std::vector<DivRegWrite>& wlist) {
  //for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformSegaPCM::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,256);
  for (int i=0; i<16; i++) {
    chan[i]=Channel();
  }

  lastBusy=60;
  pcmCycles=0;
  pcmL=0;
  pcmR=0;
  sampleBank=0;
  delay=0;
  amDepth=0x7f;
  pmDepth=0x7f;

  if (dumpWrites) {
    for (int i=0; i<16; i++) {
      addWrite(0x10086+(i<<3),3);
      addWrite(0x10002+(i<<3),0x7f);
      addWrite(0x10003+(i<<3),0x7f);
    }
  }

  extMode=false;
}

void DivPlatformSegaPCM::setFlags(unsigned int flags) {
  chipClock=8000000;
  rate=31250;
}

bool DivPlatformSegaPCM::isStereo() {
  return true;
}

int DivPlatformSegaPCM::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<16; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();

  return 16;
}

void DivPlatformSegaPCM::quit() {
}

DivPlatformSegaPCM::~DivPlatformSegaPCM() {
}
