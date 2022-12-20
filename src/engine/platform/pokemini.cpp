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

#include "pokemini.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define PCSPKR_DIVIDER 16
#define CHIP_DIVIDER 1

const char* regCheatSheetPokeMini[]={
  "TMR3_SCALE", "1C",
  "TMR3_OSC", "1D",
  "TMR3_CTRL_L", "48",
  "TMR3_CTRL_H", "49",
  "TMR3_PRE_L", "4A",
  "TMR3_PRE_H", "4B",
  "TMR3_PVT_L", "4C",
  "TMR3_PVT_H", "4D",
  "TMR3_CNT_L", "4E",
  "TMR3_CNT_H", "4F",
  "IO_DIR", "60",
  "IO_DATA", "61",
  "AUD_CTRL", "70",
  "AUD_VOL", "71",
  NULL
};

const short volTable[4]={
  0, 16384, 16384, 32767
};

const short scaleTable[8]={
  1, 7, 31, 63, 127, 255, 1023, 4095
};

const char** DivPlatformPokeMini::getRegisterSheet() {
  return regCheatSheetPokeMini;
}

void DivPlatformPokeMini::rWrite(unsigned char addr, unsigned char val) {
  if (addr<128) regPool[addr]=val;
  switch (addr) {
    case 0x1c:
      // ignore
      break;
    case 0x1d:
      // ignore
      break;
    case 0x48: case 0x49:
      on=val&4;
      if (val&2) pos=0;
      break;
    case 0x4a:
      preset=(preset&0xff00)|val;
      break;
    case 0x4b:
      preset=(preset&0xff)|(val<<8);
      break;
    case 0x4c:
      pivot=(pivot&0xff00)|val;
      break;
    case 0x4d:
      pivot=(pivot&0xff)|(val<<8);
      break;
    case 0x71:
      vol=val&3;
      break;
  }
}

void DivPlatformPokeMini::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  int out=0;
  for (size_t i=start; i<start+len; i++) {
    for (int j=0; j<PCSPKR_DIVIDER; j++) {
      elapsedMain++;
      if (on) {
        if ((elapsedMain&scaleTable[timerScale&7])==0) {
          pos--;
          if (pos<0) {
            pos=preset;
          }
        }
      }
    }
    if (on) {
      out=(pos>=pivot && !isMuted[0])?volTable[vol&3]:0;
      bufL[i]=out;
      oscBuf->data[oscBuf->needle++]=out;
    } else {
      bufL[i]=0;
      oscBuf->data[oscBuf->needle++]=0;
    }
  }
}

void DivPlatformPokeMini::tick(bool sysTick) {
  for (int i=0; i<1; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol,chan[i].std.vol.val,2);
      rWrite(0x71,(chan[i].outVol==2)?3:chan[i].outVol);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      chan[i].freqChanged=true;
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
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].keyOn) {
        rWrite(0x48,4);
      }
      if (chan[i].keyOff) {
        rWrite(0x48,0);
      }
      rWrite(0x4a,chan[i].freq&0xff);
      rWrite(0x4b,chan[i].freq>>8);
      int pvt=(chan[i].duty*chan[i].freq)>>8;
      rWrite(0x4c,pvt&0xff);
      rWrite(0x4d,pvt>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformPokeMini::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      rWrite(0x71,(chan[c.chan].outVol==2)?3:chan[c.chan].outVol);
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_POKEMINI));
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
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
        }
        if (chan[c.chan].active) {
          rWrite(0x71,(chan[c.chan].outVol==2)?3:chan[c.chan].outVol);
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
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
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_POKEMINI));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
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
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformPokeMini::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformPokeMini::forceIns() {
  for (int i=0; i<1; i++) {
    chan[i].insChanged=true;
  }
}

void* DivPlatformPokeMini::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformPokeMini::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformPokeMini::getOscBuffer(int ch) {
  return oscBuf;
}

unsigned char* DivPlatformPokeMini::getRegisterPool() {
  return regPool;
}

int DivPlatformPokeMini::getRegisterPoolSize() {
  return 128;
}

void DivPlatformPokeMini::reset() {
  for (int i=0; i<1; i++) {
    chan[i]=DivPlatformPokeMini::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  on=false;
  preset=0;
  pivot=0;
  pos=0;
  timerScale=0;
  vol=0;
  preset=0;
  pivot=0;
  elapsedMain=0;

  memset(regPool,0,128);
}

bool DivPlatformPokeMini::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformPokeMini::setFlags(const DivConfig& flags) {
  chipClock=4000000;
  CHECK_CUSTOM_CLOCK;

  rate=chipClock/PCSPKR_DIVIDER;
  oscBuf->rate=rate;
}

void DivPlatformPokeMini::notifyInsDeletion(void* ins) {
  for (int i=0; i<1; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPokeMini::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformPokeMini::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformPokeMini::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<1; i++) {
    isMuted[i]=false;
  }
  oscBuf=new DivDispatchOscBuffer;
  setFlags(flags);

  reset();
  return 5;
}

void DivPlatformPokeMini::quit() {
  delete oscBuf;
}

DivPlatformPokeMini::~DivPlatformPokeMini() {
}
