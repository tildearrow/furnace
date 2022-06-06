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

#include "ymz280b.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>
#include <map>

#define CHIP_FREQBASE 98304

#define rWrite(a,v) {if(!skipRegisterWrites) {ymz280b.write(0,a); ymz280b.write(1,v); regPool[a]=v; if(dumpWrites) addWrite(a,v); }}

const char* regCheatSheetYMZ280B[]={
  "CHx_Freq", "00+x*4",
  "CHx_Control", "01+x*4",
  "CHx_Volume", "02+x*4",
  "CHx_Panning", "03+x*4",
  "CHx_StartH", "20+x*4",
  "CHx_LoopStartH", "21+x*4",
  "CHx_LoopEndH", "22+x*4",
  "CHx_EndH", "23+x*4",
  "CHx_StartM", "40+x*4",
  "CHx_LoopStartM", "41+x*4",
  "CHx_LoopEndM", "42+x*4",
  "CHx_EndM", "43+x*4",
  "CHx_StartL", "60+x*4",
  "CHx_LoopStartL", "61+x*4",
  "CHx_LoopEndL", "62+x*4",
  "CHx_EndL", "63+x*4",
  "DSP_Channel", "80",
  "DSP_Enable", "81",
  "DSP_Data", "82",
  "RAM_AddrH", "84",
  "RAM_AddrM", "85",
  "RAM_AddrL", "86",
  "RAM_Data", "87",
  "IRQ_Enable", "E0",
  "Enable", "FF",
  NULL
};

const char** DivPlatformYMZ280B::getRegisterSheet() {
  return regCheatSheetYMZ280B;
}

const char* DivPlatformYMZ280B::getEffectName(unsigned char effect) {
  return NULL;
}

void DivPlatformYMZ280B::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  short buf[16][256];
  short *bufPtrs[16]={
    buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],
    buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]
  };
  size_t pos=start;
  while (len > 0) {
    size_t blockLen = MIN(len, 256);
    ymz280b.sound_stream_update(bufPtrs, blockLen);
    for (size_t i=0; i<blockLen; i++) {
      int dataL=0;
      int dataR=0;
      for (int j=0; j<8; j++) {
        dataL+=buf[j*2][i];
        dataR+=buf[j*2+1][i];
        oscBuf[j]->data[oscBuf[j]->needle++]=(short)(((int)buf[j*2][i]+buf[j*2+1][i])/2);
      }
      bufL[pos]=(short)(dataL/8);
      bufR[pos]=(short)(dataR/8);
      pos++;
    }
    len-=blockLen;
  }
}

void DivPlatformYMZ280B::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&0xff)*chan[i].std.vol.val)>>6;
      writeOutVol(i);
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note+chan[i].std.arp.val);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
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
    if (chan[i].std.panL.had) { // panning
      chan[i].panning=MIN((chan[i].std.panL.val*15/16+15)/2+1,15);
      rWrite(0x03+i*4,chan[i].panning);
    }
    if (chan[i].setPos) {
      // force keyon
      chan[i].keyOn=true;
      chan[i].setPos=false;
    } else {
      chan[i].audPos=0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivSample* s=parent->getSample(chan[i].sample);
      unsigned char ctrl;
      switch (s->depth) {
        case 3: ctrl=0x20; break;
        case 8: ctrl=0x40; break;
        case 16: ctrl=0x60; break;
        default: ctrl=0;
      }
      double off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
      chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE))-1;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>511) chan[i].freq=511;
      // ADPCM has half the range
      if (s->depth==3 && chan[i].freq>255) chan[i].freq=255;
      ctrl|=(chan[i].active?0x80:0)|((s->loopStart>=0)?0x10:0)|(chan[i].freq>>8);
      if (chan[i].keyOn) {
        unsigned int start=s->offYMZ280B;
        unsigned int loop=0;
        unsigned int end=MIN(start+s->getCurBufLen(),getSampleMemCapacity()-1);
        if (chan[i].audPos>0) {
          switch (s->depth) {
            case 3: start+=chan[i].audPos/2; break;
            case 8: start+=chan[i].audPos; break;
            case 16: start+=chan[i].audPos*2; break;
          }
          start=MIN(start,end);
        }
        if (s->loopStart>=0) {
          switch (s->depth) {
            case 3: loop=start+s->loopStart/2; break;
            case 8: loop=start+s->loopStart; break;
            case 16: loop=start+s->loopStart*2; break;
          }
          loop=MIN(loop,end);
        }
        rWrite(0x01+i*4,ctrl&~0x80); // force keyoff first
        rWrite(0x20+i*4,(start>>16)&0xff);
        rWrite(0x21+i*4,(loop>>16)&0xff);
        rWrite(0x22+i*4,(end>>16)&0xff);
        rWrite(0x23+i*4,(end>>16)&0xff);
        rWrite(0x40+i*4,(start>>8)&0xff);
        rWrite(0x41+i*4,(loop>>8)&0xff);
        rWrite(0x42+i*4,(end>>8)&0xff);
        rWrite(0x43+i*4,(end>>8)&0xff);
        rWrite(0x60+i*4,start&0xff);
        rWrite(0x61+i*4,loop&0xff);
        rWrite(0x62+i*4,end&0xff);
        rWrite(0x63+i*4,end&0xff);
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
          writeOutVol(i);
        }
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        rWrite(0x00+i*4,chan[i].freq&0xff);
        chan[i].freqChanged=false;
      }
      rWrite(0x01+i*4,ctrl);
    }
  }
}

int DivPlatformYMZ280B::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].sample=ins->amiga.getSample(c.value);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      }
      if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
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
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          writeOutVol(c.chan);
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].panning=MIN(parent->convertPanSplitToLinearLR(c.value,c.value2,15)+1,15);
      rWrite(0x03+c.chan*4,chan[c.chan].panning);
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
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
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].audPos=c.value;
      chan[c.chan].setPos=true;
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

void DivPlatformYMZ280B::writeOutVol(int ch) {
  unsigned char val=isMuted[ch]?0:chan[ch].outVol;
  rWrite(0x02+ch*4,val);
}

void DivPlatformYMZ280B::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  writeOutVol(ch);
}

void DivPlatformYMZ280B::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;
  }
}

void* DivPlatformYMZ280B::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformYMZ280B::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformYMZ280B::reset() {
  memset(regPool,0,256);
  ymz280b.device_reset();
  rWrite(0xff,0x80); // enable keyon
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformYMZ280B::Channel();
    chan[i].std.setEngine(parent);
    rWrite(0x02+i*4,255);
    rWrite(0x03+i*4,8);
  }
}

bool DivPlatformYMZ280B::isStereo() {
  return true;
}

void DivPlatformYMZ280B::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformYMZ280B::notifyWaveChange(int wave) {
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformYMZ280B::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformYMZ280B::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformYMZ280B::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

unsigned char* DivPlatformYMZ280B::getRegisterPool() {
  return regPool;
}

int DivPlatformYMZ280B::getRegisterPoolSize() {
  return 256;
}

float DivPlatformYMZ280B::getPostAmp() {
  // according to MAME core's mixing
  return 4.0f;
}

const void* DivPlatformYMZ280B::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformYMZ280B::getSampleMemCapacity(int index) {
  return index == 0 ? 16777216 : 0;
}

size_t DivPlatformYMZ280B::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

void DivPlatformYMZ280B::renderSamples() {
  memset(sampleMem,0,getSampleMemCapacity());

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    int length=s->getCurBufLen();
    unsigned char* src=(unsigned char*)s->getCurBuf();
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos),length);
    if (actualLength>0) {
      memcpy(&sampleMem[memPos],src,actualLength);
      s->offYMZ280B=memPos;
      memPos+=length;
    }
    if (actualLength<length) {
      logW("out of YMZ280B PCM memory for sample %d!",i);
      break;
    }
  }
  sampleMemLen=memPos;
}

void DivPlatformYMZ280B::setChipModel(int type) {
  chipType=type;
}

void DivPlatformYMZ280B::setFlags(unsigned int flags) {
  switch (chipType) {
    default:
    case 280:
      switch (flags&0xff) {
        case 0x00:
          chipClock=16934400;
          break;
        case 0x01:
          chipClock=COLOR_NTSC*4.0;
          break;
        case 0x02:
          chipClock=COLOR_PAL*16.0/5.0;
          break;
        case 0x03:
          chipClock=16000000;
          break;
        case 0x04:
          chipClock=50000000.0/3.0;
          break;
        case 0x05:
          chipClock=14000000;
          break;
      }
      rate=chipClock/384;
      break;
    case 759:
      rate=32000;
      chipClock=rate*384;
      break;
  }
  for (int i=0; i<8; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformYMZ280B::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new unsigned char[getSampleMemCapacity()];
  sampleMemLen=0;
  ymz280b.device_start(sampleMem);
  setFlags(flags);
  reset();

  return 8;
}

void DivPlatformYMZ280B::quit() {
  delete[] sampleMem;
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}
