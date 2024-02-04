/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "dave.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 32

const char* regCheatSheetDave[]={
  "Freq0", "00",
  "Control0", "01",
  "Freq1", "02",
  "Control1", "03",
  "Freq2", "04",
  "Control2", "05",
  "Control3", "06",
  "SoundCtrl", "07",
  "Vol0L", "08",
  "Vol1L", "09",
  "Vol2L", "0A",
  "Vol3L", "0B",
  "Vol0R", "0C",
  "Vol1R", "0D",
  "Vol2R", "0E",
  "Vol3R", "0F",
  "ClockDiv", "1F",
  NULL
};

const char** DivPlatformDave::getRegisterSheet() {
  return regCheatSheetDave;
}

void DivPlatformDave::acquire(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    // PCM part
    for (int i=4; i<6; i++) {
      if (chan[i].dacSample!=-1) {
        chan[i].dacPeriod+=chan[i].dacRate;
        if (chan[i].dacPeriod>rate) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->samples<=0) {
            chan[i].dacSample=-1;
            continue;
          }
          signed char dacData=((signed char)((unsigned char)s->data8[chan[i].dacPos]^0x80))>>3;
          chan[i].dacOut=CLAMP(dacData,-16,15);
          chan[i].dacPos++;
          if (s->isLoopable() && chan[i].dacPos>=(unsigned int)s->loopEnd) {
            chan[i].dacPos=s->loopStart;
          } else if (chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
          }
          chan[i].dacPeriod-=rate;
        }
      }
    }
  
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      dave->writePort(w.addr,w.val);
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }

    unsigned int next=dave->runOneCycle();
    unsigned short nextL=next&0xffff;
    unsigned short nextR=next>>16;
    
    buf[0][h]=(short)nextL;
    buf[1][h]=(short)nextR;
  }
}

void DivPlatformDave::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&63,MIN(63,chan[i].std.vol.val),63);
      chan[i].writeVol=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=parent->calcArp(chan[i].note,chan[i].std.arp.val);
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.wave.had) {
      chan[i].wave=chan[i].std.wave.val;
      chan[i].freqChanged=true;
    }
    if (chan[i].std.panL.had) {
      chan[i].panL=chan[i].std.panL.val&63;
    }
    if (chan[i].std.panR.had) {
      chan[i].panR=chan[i].std.panR.val&63;
    }
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      chan[i].writeVol=true;
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
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      if (i>=4) {
        if (chan[i].active && chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          chan[i].dacPos=0;
          chan[i].dacPeriod=0;
          chan[i].keyOn=true;
        }
      }
    }

    if (chan[i].writeVol) {
      if (chan[i].active) {
        rWrite(8+i,(63+chan[i].outVol*chan[i].panL)>>6);
        rWrite(12+i,(63+chan[i].outVol*chan[i].panR)>>6);
      } else {
        rWrite(8+i,0);
        rWrite(12+i,0);
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_PCE);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].freq<1) chan[i].freq=1;
      if (chan[i].freq>4095) chan[i].freq=4095;

      if (i<3) {
        rWrite((i<<1),chan[i].freq&0xff);
        rWrite(1+(i<<1),(chan[i].freq>>8)|((chan[i].wave&3)<<4));
      }

      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff) {
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformDave::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_PCE);
      // TODO: handle DAC
      if (c.chan>=4) break;
      chan[c.chan].sampleNote=DIV_NOTE_NULL;
      chan[c.chan].sampleNoteDelta=0;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].writeVol=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].writeVol=true;
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
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active) {
          }
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
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      chan[c.chan].keyOn=true;
      break;
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
    case DIV_CMD_PANNING: {
      chan[c.chan].panL=c.value>>2;
      chan[c.chan].panR=c.value2>>2;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_PCE));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 31;
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

void DivPlatformDave::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  // TODO
}

void DivPlatformDave::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

void* DivPlatformDave::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformDave::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformDave::getPan(int ch) {
  return (chan[ch].panL<<2)|chan[ch].panR;
}

DivChannelPair DivPlatformDave::getPaired(int ch) {
  return DivChannelPair();
}

DivChannelModeHints DivPlatformDave::getModeHints(int ch) {
  DivChannelModeHints ret;
  return ret;
}

DivSamplePos DivPlatformDave::getSamplePos(int ch) {
  if (ch<4 || ch>=6) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformDave::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformDave::mapVelocity(int ch, float vel) {
  return round(31.0*pow(vel,0.22));
}

unsigned char* DivPlatformDave::getRegisterPool() {
  return regPool;
}

int DivPlatformDave::getRegisterPoolSize() {
  return 32;
}

void DivPlatformDave::reset() {
  writes.clear();
  memset(regPool,0,32);
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformDave::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  dave->reset(true);
  lastPan=0xff;
  cycles=0;
  curChan=-1;
  // set global volume
  rWrite(0,0);
  rWrite(0x01,0xff);
  // set per-channel initial panning
  for (int i=0; i<6; i++) {
  }
}

int DivPlatformDave::getOutputCount() {
  return 2;
}

bool DivPlatformDave::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformDave::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformDave::setFlags(const DivConfig& flags) {
  chipClock=8000000.0;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/16;
  for (int i=0; i<6; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformDave::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformDave::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformDave::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  dave=new Ep128::Dave;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformDave::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  delete dave;
}

DivPlatformDave::~DivPlatformDave() {
}
