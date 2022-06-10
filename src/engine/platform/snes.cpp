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

#include "snes.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define CHIP_FREQBASE 4096

#define rWrite(a,v) {dsp.write(a,v); regPool[(a)&0x7f]=v; }

const char* regCheatSheetSNESDSP[]={
  "VxVOLL", "x0",
  "VxVOLR", "x1",
  "VxPITCHL", "x2",
  "VxPITCHH", "x3",
  "VxSRCN", "x4",
  "VxADSR1", "x5",
  "VxADSR2", "x6",
  "VxGAIN", "x7",
  "VxENVX", "x8",
  "VxOUTX", "x9",
  "FIRx", "xF",

  "MVOLL", "0C",
  "MVOLR", "1C",
  "EVOLL", "2C",
  "EVOLR", "3C",
  "KON", "4C",
  "KOFF", "5C",
  "FLG", "6C",
  "ENDX", "7C",

  "EFB", "0D",
  "PMON", "2D",
  "NON", "3D",
  "EON", "4D",
  "DIR", "5D",
  "ESA", "6D",
  "EDL", "7D",
  NULL
};

const char** DivPlatformSNES::getRegisterSheet() {
  return regCheatSheetSNESDSP;
}

const char* DivPlatformSNES::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Set echo feedback level (signed 8-bit)";
      break;
    // TODO
  }
  return NULL;
}

void DivPlatformSNES::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  short out[2];
  short chOut[16];
  for (size_t h=start; h<start+len; h++) {
    dsp.set_output(out,1);
    dsp.run(32);
    dsp.get_voice_outputs(chOut);
    bufL[h]=out[0];
    bufR[h]=out[1];
    for (int i=0; i<8; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=chOut[i*2]+chOut[i*2+1];
    }
  }
}

void DivPlatformSNES::tick() {
  // KON/KOFF can't be written several times per one sample
  // so they have to be accumulated
  unsigned char kon=0;
  unsigned char koff=0;
  for (int i=0; i<8; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      // TODO handle gain writes
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
    if (chan[i].std.panL.had) {
      int val=chan[i].std.panL.val&0x7f;
      chan[i].panL=(val<<1)|(val>>6);
    }
    if (chan[i].std.panR.had) {
      int val=chan[i].std.panR.val&0x7f;
      chan[i].panR=(val<<1)|(val>>6);
    }
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      writeOutVol(i);
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
      double off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
      chan[i].freq=(unsigned int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].freq>16383) chan[i].freq=16383;
      if (chan[i].keyOn) {
        // TODO handle sample offsets
        kon|=(1<<i);
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        koff|=(1<<i);
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        rWrite(2+i*16,chan[i].freq&0xff);
        rWrite(3+i*16,chan[i].freq>>8);
        chan[i].freqChanged=false;
      }
    }
  }
  rWrite(0x4c,kon);
  rWrite(0x5c,koff);
}

int DivPlatformSNES::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].sample=ins->amiga.getSample(c.value);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value));
      }
      if (chan[c.chan].useWave || chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
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
        writeOutVol(c.chan);
      }
      break;
    // case DIV_CMD_GLOBAL_VOLUME:
    //   gblVolL=MIN(c.value,127);
    //   gblVolR=MIN(c.value,127);
    //   rWrite(0x0c,gblVolL);
    //   rWrite(0x1c,gblVolR);
    //   break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].panL=c.value;
      chan[c.chan].panR=c.value2;
      writeOutVol(c.chan);
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=round(NOTE_FREQUENCY(c.value2));
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
      chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0))));
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
    // TODO SNES-specific commands
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSNES::writeOutVol(int ch) {
  // TODO negative (inverted) panning support
  int outL=0;
  int outR=0;
  if (!isMuted[ch]) {
    outL=chan[ch].vol*chan[ch].panL/255;
    outR=chan[ch].vol*chan[ch].panR/255;
  }
  rWrite(0+ch*16,outL);
  rWrite(1+ch*16,outR);
}

void DivPlatformSNES::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  writeOutVol(ch);
}

void DivPlatformSNES::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;
  }
}

void* DivPlatformSNES::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSNES::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSNES::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSNES::getRegisterPool() {
  // get states from emulator
  for (int i=0; i<0x80; i+=0x10) {
    regPool[i+8]=dsp.read(i+8);
    regPool[i+9]=dsp.read(i+9);
  }
  regPool[0x7c]=dsp.read(0x7c); // ENDX
  return regPool;
}

int DivPlatformSNES::getRegisterPoolSize() {
  return 128;
}

void DivPlatformSNES::reset() {
  dsp.init(sampleMem);
  dsp.set_output(NULL,0);
  memset(regPool,0,128);
  // TODO more initial values
  sampleTableBase=0;
  rWrite(0x5d,sampleTableBase>>8);
  rWrite(0x0c,127); // global volume left
  rWrite(0x1c,127); // global volume right
  for (int i=0; i<8; i++) {
    chan[i]=Channel();
    chan[i].std.setEngine(parent);
    writeOutVol(i);
  }
}

bool DivPlatformSNES::isStereo() {
  return true;
}

void DivPlatformSNES::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformSNES::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSNES::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSNES::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const void* DivPlatformSNES::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformSNES::getSampleMemCapacity(int index) {
  // TODO change it based on current echo buffer size
  return index == 0 ? 65536 : 0;
}

size_t DivPlatformSNES::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

void DivPlatformSNES::renderSamples() {
  memset(sampleMem,0,getSampleMemCapacity());

  // skip past sample table
  size_t memPos=sampleTableBase+0x400;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    int length=s->lengthBRR;
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos)/9*9,length);
    if (actualLength>0) {
      size_t tabAddr=sampleTableBase+i*4;
      size_t loopPos=memPos;
      if (s->loopStart>=0) loopPos+=s->loopStart;
      s->offSNES=memPos;
      sampleMem[tabAddr+0]=memPos&0xff;
      sampleMem[tabAddr+1]=memPos>>8;
      sampleMem[tabAddr+2]=loopPos&0xff;
      sampleMem[tabAddr+3]=loopPos>>8;
      memcpy(&sampleMem[memPos],s->data8,actualLength);
      memPos+=actualLength;
    }
    if (actualLength<length) {
      // terminate the sample
      sampleMem[memPos-9]=1;
      logW("out of BRR memory for sample %d!",i);
      break;
    }
  }
  sampleMemLen=memPos;
}

int DivPlatformSNES::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    isMuted[i]=false;
  }
  sampleMemLen=0;
  chipClock=1024000;
  rate=chipClock/32;
  reset();
  return 8;
}

void DivPlatformSNES::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}
