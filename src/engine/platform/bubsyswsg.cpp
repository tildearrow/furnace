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

#include "bubsyswsg.h"
#include "../engine.h"
#include <math.h>

#define CHIP_DIVIDER 32

#define rWrite(a,v) {if(!skipRegisterWrites) {regPool[a]=v; if(dumpWrites) addWrite(a,v); }}

const char* regCheatSheetBubSysWSG[]={
  // K005289 timer
  "Freq_A", "0",
  "Freq_B", "1",
  // PROM, DAC control from External logic (Connected to AY PSG ports on Bubble System)
  "WaveVol_A", "2",
  "WaveVol_B", "3",
  NULL
};

const char** DivPlatformBubSysWSG::getRegisterSheet() {
  return regCheatSheetBubSysWSG;
}

void DivPlatformBubSysWSG::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  int chanOut=0;
  for (size_t h=start; h<start+len; h++) {
    signed int out=0;
    // K005289 part
    k005289.tick();

    // Wavetable part
    for (int i=0; i<2; i++) {
      if (isMuted[i]) {
        oscBuf[i]->data[oscBuf[i]->needle++]=0;
        continue;
      } else {
        chanOut=chan[i].waveROM[k005289.addr(i)]*(regPool[2+i]&0xf);
        out+=chanOut;
        if (writeOscBuf==0) {
          oscBuf[i]->data[oscBuf[i]->needle++]=chanOut<<7;
        }
      }
    }

    if (++writeOscBuf>=64) writeOscBuf=0;

    out<<=6; // scale output to 16 bit

    if (out<-32768) out=-32768;
    if (out>32767) out=32767;

    //printf("out: %d\n",out);
    bufL[h]=bufR[h]=out;
  }
}

void DivPlatformBubSysWSG::updateWave(int ch) {
  //DivWavetable* wt=parent->getWave(chan[ch].wave);
  for (int i=0; i<32; i++) {
    // convert to signed
    chan[ch].waveROM[i]=chan[ch].ws.output[i]-8;
  }
  if (chan[ch].active) {
    rWrite(2+ch,(ch<<5)|chan[ch].outVol);
  }
}

void DivPlatformBubSysWSG::tick(bool sysTick) {
  for (int i=0; i<2; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&15)*MIN(15,chan[i].std.vol.val))/15;
      rWrite(2+i,(chan[i].wave<<5)|chan[i].outVol);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
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
    if (chan[i].active) {
      if (chan[i].ws.tick()) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SCC);
      chan[i].freq=0x1000-parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>4095) chan[i].freq=4095;
      k005289.load(i,chan[i].freq);
      rWrite(i,chan[i].freq);
      k005289.update(i);
      if (chan[i].keyOn) {
        // ???
      }
      if (chan[i].keyOff) {
        rWrite(2+i,(chan[i].wave<<5)|0);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformBubSysWSG::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SCC);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      rWrite(2+c.chan,(chan[c.chan].wave<<5)|chan[c.chan].vol);
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }
      chan[c.chan].ws.init(ins,32,15,chan[c.chan].insChanged);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
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
          if (chan[c.chan].active) rWrite(2+c.chan,(chan[c.chan].wave<<5)|chan[c.chan].outVol);
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
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
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
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SCC));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformBubSysWSG::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  //rWrite(2+ch,(chan[ch].wave<<5)|((chan[ch].active && isMuted[ch])?0:chan[ch].outVol));
}

void DivPlatformBubSysWSG::forceIns() {
  for (int i=0; i<2; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
  }
}

void* DivPlatformBubSysWSG::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformBubSysWSG::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformBubSysWSG::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformBubSysWSG::getRegisterPool() {
  return (unsigned char*)regPool;
}

int DivPlatformBubSysWSG::getRegisterPoolSize() {
  return 4;
}

int DivPlatformBubSysWSG::getRegisterPoolDepth() {
  return 16;
}

void DivPlatformBubSysWSG::reset() {
  memset(regPool,0,4*2);
  for (int i=0; i<2; i++) {
    chan[i]=DivPlatformBubSysWSG::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent,8);
    chan[i].ws.init(NULL,32,15,false);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  k005289.reset();
}

bool DivPlatformBubSysWSG::isStereo() {
  return false;
}

bool DivPlatformBubSysWSG::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformBubSysWSG::notifyWaveChange(int wave) {
  for (int i=0; i<2; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(chan[i].wave);
      updateWave(i);
    }
  }
}

void DivPlatformBubSysWSG::notifyInsDeletion(void* ins) {
  for (int i=0; i<2; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformBubSysWSG::setFlags(const DivConfig& flags) {
  chipClock=COLOR_NTSC;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  for (int i=0; i<2; i++) {
    oscBuf[i]->rate=rate/64;
  }
}

void DivPlatformBubSysWSG::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformBubSysWSG::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformBubSysWSG::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  writeOscBuf=0;
  for (int i=0; i<2; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 2;
}

void DivPlatformBubSysWSG::quit() {
  for (int i=0; i<2; i++) {
    delete oscBuf[i];
  }
}

DivPlatformBubSysWSG::~DivPlatformBubSysWSG() {
}
