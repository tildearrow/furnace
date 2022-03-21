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
#include <math.h>

#define CHIP_DIVIDER 16

#define rWrite(a,v) {dsp->write(a,v); regPool[(a)&0x7f]=v; }

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

void DivPlatformSNES::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    short out[2];
    dsp->set_output(out,1);
    dsp->run(32);
    bufL[h]=out[0];
    bufR[h]=out[1];
  }
}

void DivPlatformSNES::tick() {
  for (int i=0; i<8; i++) {
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
          chan[i].baseFreq=off*NOTE_PERIODIC(chan[i].std.arp);
        } else {
          chan[i].baseFreq=off*NOTE_PERIODIC(chan[i].note+chan[i].std.arp);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=off*NOTE_PERIODIC(chan[i].note);
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

int DivPlatformSNES::dispatch(DivCommand c) {
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
        chan[c.chan].baseFreq=off*NOTE_PERIODIC(c.value);
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
      double off=1.0;
      if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(chan[c.chan].sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=8363.0/(double)s->centerRate;
        }
      }
      chan[c.chan].baseFreq=off*NOTE_PERIODIC(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp-12):(0)));
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
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSNES::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformSNES::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].audPos=131072;
    chan[i].audDat=0;
    chan[i].sample=-1;
  }
}

void* DivPlatformSNES::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformSNES::getRegisterPool() {
  // get states from emulator
  for (int i=0; i<0x80; i+=0x10) {
    regPool[i+8]=dsp->read(i+8);
    regPool[i+9]=dsp->read(i+9);
  }
  return regPool;
}

int DivPlatformSNES::getRegisterPoolSize() {
  return 128;
}

void DivPlatformSNES::reset() {
  for (int i=0; i<8; i++) {
    chan[i]=Channel();
  }
  dsp->init(&aram);
  dsp->set_output(NULL, 0);
  memset(regPool,0,128);
}

bool DivPlatformSNES::isStereo() {
  return true;
}

bool DivPlatformSNES::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformSNES::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformSNES::notifyWaveChange(int wave) {
  // TODO when wavetables are added
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

int DivPlatformSNES::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dsp=new SPC_DSP;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
  }
  chipClock=1024000;
  rate=chipClock/32;
  reset();
  return 8;
}

void DivPlatformSNES::quit() {
  delete dsp;
}
