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

#include "gb.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {GB_apu_write(gb,a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }
#define immWrite(a,v) {GB_apu_write(gb,a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 16

const char* regCheatSheetGB[]={
  "NR10_Sweep", "10",
  "NR11_DutyLen", "11",
  "NR12_VolEnv", "12",
  "NR13_FreqL", "13",
  "NR14_FreqH", "14",

  "NR21_DutyLen", "16",
  "NR22_VolEnv", "17",
  "NR23_FreqL", "18",
  "NR24_FreqH", "19",

  "NR30_WaveOn", "1A",
  "NR31_Len", "1B",
  "NR32_Vol", "1C",
  "NR33_FreqL", "1D",
  "NR34_FreqH", "1E",

  "NR41_Len", "20",
  "NR42_VolEnv", "21",
  "NR43_Freq", "22",
  "NR44_Control", "23",

  "NR50_MasterVol", "24",
  "NR51_Toggle", "25",
  "NR52_PowerStat", "26",

  "Wave", "30",
  NULL
};

const char** DivPlatformGB::getRegisterSheet() {
  return regCheatSheetGB;
}

const char* DivPlatformGB::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Change waveform";
      break;
    case 0x11:
      return "11xx: Set noise length (0: long; 1: short)";
      break;
    case 0x12:
      return "12xx: Set duty cycle (0 to 3)";
      break;
    case 0x13:
      return "13xy: Setup sweep (x: time; y: shift)";
      break;
    case 0x14:
      return "14xx: Set sweep direction (0: up; 1: down)";
      break;
  }
  return NULL;
}

void DivPlatformGB::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    GB_advance_cycles(gb,16);
    bufL[i]=gb->apu_output.final_sample.left;
    bufR[i]=gb->apu_output.final_sample.right;
  }
}

void DivPlatformGB::updateWave() {
  rWrite(0x1a,0);
  for (int i=0; i<16; i++) {
    int nibble1=15-ws.output[i<<1];
    int nibble2=15-ws.output[1+(i<<1)];
    rWrite(0x30+i,(nibble1<<4)|nibble2);
  }
}

static unsigned char chanMuteMask[4]={
  0xee, 0xdd, 0xbb, 0x77
};

unsigned char DivPlatformGB::procMute() {
  return lastPan&(isMuted[0]?chanMuteMask[0]:0xff)
                &(isMuted[1]?chanMuteMask[1]:0xff)
                &(isMuted[2]?chanMuteMask[2]:0xff)
                &(isMuted[3]?chanMuteMask[3]:0xff);
}

static unsigned char gbVolMap[16]={
  0x00, 0x00, 0x00, 0x00,
  0x60, 0x60, 0x60, 0x60,
  0x40, 0x40, 0x40, 0x40,
  0x20, 0x20, 0x20, 0x20
};

static unsigned char noiseTable[256]={
  0,
  0xf7, 0xf6, 0xf5, 0xf4,
  0xe7, 0xe6, 0xe5, 0xe4,
  0xd7, 0xd6, 0xd5, 0xd4,
  0xc7, 0xc6, 0xc5, 0xc4,
  0xb7, 0xb6, 0xb5, 0xb4,
  0xa7, 0xa6, 0xa5, 0xa4,
  0x97, 0x96, 0x95, 0x94,
  0x87, 0x86, 0x85, 0x84,
  0x77, 0x76, 0x75, 0x74,
  0x67, 0x66, 0x65, 0x64,
  0x57, 0x56, 0x55, 0x54,
  0x47, 0x46, 0x45, 0x44,
  0x37, 0x36, 0x35, 0x34,
  0x27, 0x26, 0x25, 0x24,
  0x17, 0x16, 0x15, 0x14,
  0x07, 0x06, 0x05, 0x04,
  0x03, 0x02, 0x01, 0x00,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void DivPlatformGB::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.arp.had) {
      if (i==3) { // noise
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=chan[i].std.arp.val+24;
        } else {
          chan[i].baseFreq=chan[i].note+chan[i].std.arp.val;
        }
        if (chan[i].baseFreq>255) chan[i].baseFreq=255;
        if (chan[i].baseFreq<0) chan[i].baseFreq=0;
      } else {
        if (!chan[i].inPorta) {
          if (chan[i].std.arp.mode) {
            chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp.val+24);
          } else {
            chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp.val);
          }
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (i!=2) {
        rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
      } else {
        if (parent->song.waveDutyIsVol) {
          rWrite(16+i*5+2,gbVolMap[(chan[i].std.duty.val&3)<<2]);
        }
      }
    }
    if (i==2 && chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].keyOn=true;
      }
    }
    if (i==2) {
      if (chan[i].active) {
        if (ws.tick()) {
          updateWave();
          if (!chan[i].keyOff) chan[i].keyOn=true;
        }
      }
    }
    if (chan[i].sweepChanged) {
      chan[i].sweepChanged=false;
      if (i==0) {
        rWrite(16+i*5,chan[i].sweep);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (i==3) { // noise
        int ntPos=chan[i].baseFreq;
        if (ntPos<0) ntPos=0;
        if (ntPos>255) ntPos=255;
        chan[i].freq=noiseTable[ntPos];
      } else {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true)+chan[i].std.pitch.val;
        if (chan[i].freq>2047) chan[i].freq=2047;
        if (chan[i].freq<0) chan[i].freq=0;
      }
      if (chan[i].keyOn) {
        if (i==2) { // wave
          rWrite(16+i*5,0x80);
          rWrite(16+i*5+2,gbVolMap[chan[i].vol]);
        } else {
          rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
          rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
        }
      }
      if (chan[i].keyOff) {
        if (i==2) {
          rWrite(16+i*5+2,0);
        } else {
          rWrite(16+i*5+2,8);
        }
      }
      if (i==3) { // noise
        rWrite(16+i*5+3,(chan[i].freq&0xff)|(chan[i].duty?8:0));
        rWrite(16+i*5+4,((chan[i].keyOn||chan[i].keyOff)?0x80:0x00)|((ins->gb.soundLen<64)<<6));
      } else {
        rWrite(16+i*5+3,(2048-chan[i].freq)&0xff);
        rWrite(16+i*5+4,(((2048-chan[i].freq)>>8)&7)|((chan[i].keyOn||chan[i].keyOff)?0x80:0x00)|((ins->gb.soundLen<63)<<6));
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

void DivPlatformGB::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  rWrite(0x25,procMute());
}

int DivPlatformGB::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.value!=DIV_NOTE_NULL) {
        if (c.chan==3) { // noise
          chan[c.chan].baseFreq=c.value;
        } else {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        }
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(ins);
      if (c.chan==2) {
        if (chan[c.chan].wave<0) {
          chan[c.chan].wave=0;
          ws.changeWave1(chan[c.chan].wave);
        }
        ws.init(ins,32,15,chan[c.chan].insChanged);
      }
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
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
        chan[c.chan].insChanged=true;
        if (c.chan!=2) {
          DivInstrument* ins=parent->getIns(chan[c.chan].ins);
          chan[c.chan].vol=ins->gb.envVol;
          if (parent->song.gbInsAffectsEnvelope) {
            rWrite(16+c.chan*5+2,((chan[c.chan].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
          }
        }
      }
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      if (c.chan==2) {
        rWrite(16+c.chan*5+2,gbVolMap[chan[c.chan].vol]);
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
      if (c.chan!=2) break;
      chan[c.chan].wave=c.value;
      ws.changeWave1(chan[c.chan].wave);
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
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=c.value;
      if (c.chan!=2) {
        chan[c.chan].freqChanged=true;
        rWrite(16+c.chan*5+1,((chan[c.chan].duty&3)<<6)|(63-(parent->getIns(chan[c.chan].ins)->gb.soundLen&63)));
      }
      break;
    case DIV_CMD_PANNING: {
      lastPan&=~(0x11<<c.chan);
      if (c.value==0) c.value=0x11;
      c.value=((c.value&15)>0)|(((c.value>>4)>0)<<4);
      lastPan|=c.value<<c.chan;
      rWrite(0x25,procMute());
      break;
    }
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GB_SWEEP_DIR:
      if (c.chan>0) break;
      chan[c.chan].sweep&=0xf7;
      if (c.value&1) {
        chan[c.chan].sweep|=8;
      }
      chan[c.chan].sweepChanged=true;
      break;
    case DIV_CMD_GB_SWEEP_TIME:
      if (c.chan>0) break;
      chan[c.chan].sweep&=8;
      chan[c.chan].sweep|=c.value&0x77;
      chan[c.chan].sweepChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformGB::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
  }
  immWrite(0x25,procMute());
  updateWave();
}

void* DivPlatformGB::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformGB::getRegisterPool() {
  return regPool;
}

int DivPlatformGB::getRegisterPoolSize() {
  return 64;
}

void DivPlatformGB::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformGB::Channel();
    chan[i].std.setEngine(parent);
  }
  ws.setEngine(parent);
  ws.init(NULL,32,15,false);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  memset(gb,0,sizeof(GB_gameboy_t));
  memset(regPool,0,128);
  gb->model=GB_MODEL_DMG_B;
  GB_apu_init(gb);
  GB_set_sample_rate(gb,rate);
  // enable all channels
  immWrite(0x10,0);
  immWrite(0x26,0x8f);
  lastPan=0xff;
  immWrite(0x25,procMute());
  immWrite(0x24,0x77);
}

bool DivPlatformGB::isStereo() {
  return true;
}

void DivPlatformGB::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGB::notifyWaveChange(int wave) {
  if (chan[2].wave==wave) {
    ws.changeWave1(wave);
    updateWave();
    if (!chan[2].keyOff) chan[2].keyOn=true;
  }
}

void DivPlatformGB::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformGB::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformGB::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformGB::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
  }
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  chipClock=4194304;
  rate=chipClock/16;
  gb=new GB_gameboy_t;
  reset();
  return 4;
}

void DivPlatformGB::quit() {
  delete gb;
}

DivPlatformGB::~DivPlatformGB() {
}
