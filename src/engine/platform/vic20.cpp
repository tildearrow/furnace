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

#include "vic20.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) {regPool[(a)]=(v)&0xff; vic_sound_machine_store(vic,a,(v)&0xff);}

#define CHIP_DIVIDER 32
#define SAMP_DIVIDER 4

const char* regCheatSheetVIC[]={
  "CH1_Pitch", "0A",
  "CH2_Pitch", "0B",
  "CH3_Pitch", "0C",
  "Noise_Pitch", "0D",
  "Volume", "0E",
  NULL
};

const char** DivPlatformVIC20::getRegisterSheet() {
  return regCheatSheetVIC;
}

void DivPlatformVIC20::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  const unsigned char loadFreq[3] = {0x7e, 0x7d, 0x7b};
  const unsigned char wavePatterns[16] = {
    0b0,     0b10,    0b100,   0b110,   0b1000,  0b1010,   0b1011,   0b1110,
    0b10010, 0b10100, 0b10110, 0b11000, 0b11010, 0b100100, 0b101010, 0b101100
  };
  for (size_t h=start; h<start+len; h++) {
    if (hasWaveWrite) {
      hasWaveWrite=false;
      for (int i=0; i<3; i++) {
        if (chan[i].waveWriteCycle>=0) {
          if (chan[i].waveWriteCycle>=16*7) {
            // empty shift register first
            rWrite(10+i,126);
          } else if (chan[i].waveWriteCycle>=16) {
            unsigned bit=8-(chan[i].waveWriteCycle/16);
            rWrite(10+i,loadFreq[i]|((wavePatterns[chan[i].wave]<<bit)&0x80));
          } else {
            rWrite(10+i,255-chan[i].freq);
          }
          chan[i].waveWriteCycle-=SAMP_DIVIDER;
          hasWaveWrite=true;
        }
      }
    }
    short samp;
    vic_sound_machine_calculate_samples(vic,&samp,1,1,0,SAMP_DIVIDER);
    bufL[h]=samp;
    bufR[h]=samp;
    for (int i=0; i<4; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=vic->ch[i].out?(vic->volume<<11):0;
    }
  }
}

void DivPlatformVIC20::calcAndWriteOutVol(int ch, int env) {
  chan[ch].outVol=MIN(chan[ch].vol*env/15,15);
  writeOutVol(ch);
}

void DivPlatformVIC20::writeOutVol(int ch) {
  if (!isMuted[ch] && chan[ch].active) {
    rWrite(14,chan[ch].outVol);
  }
}

void DivPlatformVIC20::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      int env=chan[i].std.vol.val;
      calcAndWriteOutVol(i,env);
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
      if (chan[i].wave!=chan[i].std.wave.val) {
        chan[i].wave=chan[i].std.wave.val&0x0f;
        chan[i].keyOn=true;
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
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (i<3) {
        chan[i].freq>>=(2-i);
      } else {
        chan[i].freq>>=1;
      }
      if (chan[i].freq<1) chan[i].freq=1;
      if (chan[i].freq>127) chan[i].freq=0;
      if (isMuted[i]) chan[i].keyOn=false;
      if (chan[i].keyOn) {
        if (i<3) {
          // 128*16 cycles for lowest channel to finish counting at lowest freq
          // 2*16 cycles for lowest channel to empty first 2 bits
          // 7*16 cycles to write 7 bits
          chan[i].waveWriteCycle=137*16-1;
          hasWaveWrite=true;
        } else {
          rWrite(10+i,255-chan[i].freq);
        }
        chan[i].keyOn=false;
      } else if (chan[i].freqChanged && chan[i].active && !isMuted[i]) {
        rWrite(10+i,255-chan[i].freq);
      }
      if (chan[i].keyOff) {
        rWrite(10+i,0);
        chan[i].keyOff=false;
      }
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformVIC20::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_VIC);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
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
          calcAndWriteOutVol(c.chan,15);
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
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value&0x0f;
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
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_VIC));
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

void DivPlatformVIC20::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (mute) {
    chan[ch].keyOff=true;
  } else if (chan[ch].active) {
    chan[ch].keyOn=true;
  }
}

void DivPlatformVIC20::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    writeOutVol(i);
  }
}

void* DivPlatformVIC20::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformVIC20::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformVIC20::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformVIC20::getRegisterPool() {
  return regPool;
}

int DivPlatformVIC20::getRegisterPoolSize() {
  return 16;
}

void DivPlatformVIC20::reset() {
  memset(regPool,0,16);
  for (int i=0; i<4; i++) {
    chan[i]=Channel();
    chan[i].std.setEngine(parent);
  }
  vic_sound_machine_init(vic,rate,chipClock);
  hasWaveWrite=false;
  rWrite(14,15);
  // hack: starting noise channel right away after this would result in a dead
  // channel as the LFSR state is 0, so clock it a bit
  vic_sound_clock(vic,4);
}

bool DivPlatformVIC20::isStereo() {
  return false;
}

void DivPlatformVIC20::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformVIC20::setFlags(const DivConfig& flags) {
  if (flags.getInt("clockSel",0)) {
    chipClock=COLOR_PAL/4.0;
  } else {
    chipClock=COLOR_NTSC*2.0/7.0;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/4;
  for (int i=0; i<4; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformVIC20::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformVIC20::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformVIC20::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  vic=new sound_vic20_t();
  reset();
  return 4;
}

void DivPlatformVIC20::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  delete vic;
}

DivPlatformVIC20::~DivPlatformVIC20() {
}
