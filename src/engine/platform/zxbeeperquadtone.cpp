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

#include "zxbeeperquadtone.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) regPool[a]=v
#define CHIP_FREQBASE 2048*320
#define CHIP_DIVIDER 16*13

const char** DivPlatformZXBeeperQuadTone::getRegisterSheet() {
  return NULL;
}

void DivPlatformZXBeeperQuadTone::acquire(short** buf, size_t len) {
  for (int i=0; i<5; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    bool sampleActive=false;
    if (curSample>=0 && curSample<parent->song.sampleLen) {
      sampleActive=!isMuted[4];
      while (curSamplePeriod>=chan[4].freq) {
        DivSample* s=parent->getSample(curSample);
        if (s->samples>0) {
          chan[4].out=(s->data8[curSamplePos++]>0);
          if (curSamplePos>=s->samples) curSample=-1;
          // (theoretical) 32KiB limit
          if (curSamplePos>=32768*8) curSample=-1;
        } else {
          curSample=-1;
        }
        curSamplePeriod-=chan[4].freq;
      }
      curSamplePeriod+=40;
    }
    if (sampleActive) {
      buf[0][h]=chan[4].out?32767:0;
      if (outputClock==0) {
        oscBuf[0]->putSample(h,0);
        oscBuf[1]->putSample(h,0);
        oscBuf[2]->putSample(h,0);
        oscBuf[3]->putSample(h,0);
      }
      oscBuf[4]->putSample(h,buf[0][h]);
    } else {
      int ch=outputClock/2;
      int b=ch*4;
      bool o=false;
      if ((outputClock&1)==0) {
        short oscOut;
        chan[ch].sPosition+=(regPool[1+b]<<8)|regPool[0+b];
        chan[ch].out=regPool[3+b]+((((chan[ch].sPosition>>8)&0xff)<regPool[2+b])?1:0);
        if (isMuted[ch] || ((chan[ch].out&0x18)==0)) {
          oscOut=0;
        } else if ((chan[ch].out&0x18)==0x18) {
          oscOut=32767;
        } else {
          oscOut=16383;
        }
        oscBuf[ch]->putSample(h,oscOut);
      }
      if (!isMuted[ch]) o=chan[ch].out&0x10;
      if (noHiss) {
        deHisser[outputClock]=o;
        buf[0][h]=-1;
        for (int i=0; i<8; i++) {
          buf[0][h]+=deHisser[i]?4096:0;
        }
      } else {
        buf[0][h]=o?32767:0;
      }
      chan[ch].out<<=1;
      oscBuf[4]->putSample(h,0);
    }
    outputClock=(outputClock+1)&7;
  }

  for (int i=0; i<5; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformZXBeeperQuadTone::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(MIN(chan[i].vol,2)*MIN(chan[i].std.vol.val,2)/2);
      writeOutVol(i);
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      rWrite(2+i*4,chan[i].duty^0xff);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
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
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (chan[i].active) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
        if (chan[i].freq<0) chan[i].freq=0;
        if (chan[i].freq>32768) chan[i].freq=32768;
        rWrite(0+i*4,chan[i].freq&0xff);
        rWrite(1+i*4,chan[i].freq>>8);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
  if (NEW_ARP_STRAT) {
    chan[4].handleArp();
  } else if (chan[4].std.arp.had) {
    if (!chan[4].inPorta) {
      chan[4].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[4].note,chan[4].std.arp.val));
    }
    chan[4].freqChanged=true;
  }
  if (chan[4].std.pitch.had) {
    if (chan[4].std.pitch.mode) {
      chan[4].pitch2+=chan[4].std.pitch.val;
      CLAMP_VAR(chan[4].pitch2,-65535,65535);
    } else {
      chan[4].pitch2=chan[4].std.pitch.val;
    }
    chan[4].freqChanged=true;
  }
  if (chan[4].freqChanged || chan[4].keyOn || chan[4].keyOff) {
    if (chan[4].active) {
      double off=CHIP_DIVIDER;
      if (curSample>=0 && curSample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(curSample);
        off=(s->centerRate>=1)?(CHIP_DIVIDER*(double)s->centerRate/parent->getCenterRate()):CHIP_DIVIDER;
      }
      chan[4].freq=parent->calcFreq(chan[4].baseFreq,chan[4].pitch,chan[4].fixedArp?chan[4].baseNoteOverride:chan[4].arpOff,chan[4].fixedArp,true,2,chan[4].pitch2,chipClock,off);
      if (chan[4].freq>258) chan[4].freq=258;
      if (chan[4].freq<3) chan[4].freq=3;
      rWrite(16,(chan[4].freq-2)&255);
      chan[4].freq*=13;
    }
    if (chan[4].keyOn) chan[4].keyOn=false;
    if (chan[4].keyOff) chan[4].keyOff=false;
    chan[4].freqChanged=false;
  }
}

int DivPlatformZXBeeperQuadTone::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan<4) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_POKEMINI);
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
        chan[c.chan].active=true;
        chan[c.chan].keyOn=true;
        chan[c.chan].macroInit(ins);
        chan[c.chan].insChanged=false;
        if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
          writeOutVol(c.chan);
        }
      } else {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
        if (c.value!=DIV_NOTE_NULL) {
          curSample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
          // TODO support offset commands
          curSamplePos=0;
          curSamplePeriod=0;
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          curSample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
          // TODO support offset commands
          curSamplePos=0;
          curSamplePeriod=0;
        }
        chan[c.chan].active=true;
        chan[c.chan].keyOn=true;
        chan[c.chan].macroInit(ins);
        chan[c.chan].insChanged=false;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      writeOutVol(c.chan);
      if (c.chan>=4) {
        curSample=-1;
      }
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
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2+chan[c.chan].sampleNoteDelta);
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
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=c.value;
      if (c.chan<4) rWrite(2+c.chan*4,chan[c.chan].duty^0xff);
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_POKEMINI));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 2;
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

void DivPlatformZXBeeperQuadTone::writeOutVol(int ch) {
  if (ch>=4) return;
  unsigned char val=(chan[ch].outVol>=1)?((chan[ch].outVol>=2)?31:7):0;
  rWrite(3+ch*4,(chan[ch].active)?val:0);
}

void DivPlatformZXBeeperQuadTone::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  writeOutVol(ch);
}

void DivPlatformZXBeeperQuadTone::forceIns() {
  for (int i=0; i<5; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

void* DivPlatformZXBeeperQuadTone::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformZXBeeperQuadTone::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformZXBeeperQuadTone::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformZXBeeperQuadTone::getRegisterPool() {
  return regPool;
}

int DivPlatformZXBeeperQuadTone::getRegisterPoolSize() {
  return 17;
}

void DivPlatformZXBeeperQuadTone::reset() {
  memset(regPool,0,17);
  memset(deHisser,0,8);
  for (int i=0; i<5; i++) {
    chan[i]=DivPlatformZXBeeperQuadTone::Channel();
    chan[i].std.setEngine(parent);
    if (i<4) rWrite(2+i*4,128);
  }
  cycles=0;
  curChan=0;
  sOffTimer=0;
  curSample=-1;
  curSamplePos=0;
  curSamplePeriod=0;
  outputClock=0;
}

bool DivPlatformZXBeeperQuadTone::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformZXBeeperQuadTone::notifyWaveChange(int wave) {
}

void DivPlatformZXBeeperQuadTone::notifyInsDeletion(void* ins) {
  for (int i=0; i<5; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformZXBeeperQuadTone::setFlags(const DivConfig& flags) {
  if (flags.getInt("clockSel",0)) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/40;
  noHiss=flags.getBool("noHiss",false);
  for (int i=0; i<5; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformZXBeeperQuadTone::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformZXBeeperQuadTone::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformZXBeeperQuadTone::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  for (int i=0; i<5; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 5;
}

void DivPlatformZXBeeperQuadTone::quit() {
  for (int i=0; i<5; i++) {
    delete oscBuf[i];
  }
}

DivPlatformZXBeeperQuadTone::~DivPlatformZXBeeperQuadTone() {
}
