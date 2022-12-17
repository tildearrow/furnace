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

#include "namcowsg.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 4194304

const char* regCheatSheetNamcoWSG[]={
  "WaveSel0", "05",
  "WaveSel1", "0A",
  "WaveSel2", "0F",
  "FreqS0", "10",
  "FreqL0", "11",
  "FreqM0", "12",
  "FreqH0", "13",
  "FreqX0", "14",
  "Volume0", "15",
  "FreqL1", "16",
  "FreqM1", "17",
  "FreqH1", "18",
  "FreqX1", "19",
  "Volume1", "1A",
  "FreqL2", "1B",
  "FreqM2", "1C",
  "FreqH2", "1D",
  "FreqX2", "1E",
  "Volume2", "1F",
  NULL
};

const char* regCheatSheetNamco15XX[]={
  "Volume0", "03",
  "FreqL0", "04",
  "FreqH0", "05",
  "WaveSel0", "06",

  "Volume1", "0B",
  "FreqL1", "0C",
  "FreqH1", "0D",
  "WaveSel1", "0E",

  "Volume2", "13",
  "FreqL2", "14",
  "FreqH2", "15",
  "WaveSel2", "16",

  "Volume3", "1B",
  "FreqL3", "1C",
  "FreqH3", "1D",
  "WaveSel3", "1E",

  "Volume4", "23",
  "FreqL4", "24",
  "FreqH4", "25",
  "WaveSel4", "26",

  "Volume5", "2B",
  "FreqL5", "2C",
  "FreqH5", "2D",
  "WaveSel5", "2E",

  "Volume6", "33",
  "FreqL6", "34",
  "FreqH6", "35",
  "WaveSel6", "36",

  "Volume7", "3B",
  "FreqL7", "3C",
  "FreqH7", "3D",
  "WaveSel7", "3E",
  
  NULL
};

const char* regCheatSheetNamcoCUS30[]={
  "VolumeL0", "00",
  "WaveSel0", "01",
  "FreqH0", "02",
  "FreqL0", "03",
  "VolumeR0", "04",

  "VolumeL1", "08",
  "WaveSel1", "09",
  "FreqH1", "0A",
  "FreqL1", "0B",
  "VolumeR1", "0C",

  "VolumeL2", "10",
  "WaveSel2", "11",
  "FreqH2", "12",
  "FreqL2", "13",
  "VolumeR2", "14",

  "VolumeL3", "18",
  "WaveSel3", "19",
  "FreqH3", "1A",
  "FreqL3", "1B",
  "VolumeR3", "1C",

  "VolumeL4", "20",
  "WaveSel4", "21",
  "FreqH4", "22",
  "FreqL4", "23",
  "VolumeR4", "24",

  "VolumeL5", "28",
  "WaveSel5", "29",
  "FreqH5", "2A",
  "FreqL5", "2B",
  "VolumeR5", "2C",

  "VolumeL6", "30",
  "WaveSel6", "31",
  "FreqH6", "32",
  "FreqL6", "33",
  "VolumeR6", "34",

  "VolumeL7", "38",
  "WaveSel7", "39",
  "FreqH7", "3A",
  "FreqL7", "3B",
  "VolumeR7", "3C",
  
  NULL
};

const char** DivPlatformNamcoWSG::getRegisterSheet() {
  if (devType==30) return regCheatSheetNamcoCUS30;
  if (devType==15) return regCheatSheetNamco15XX;
  return regCheatSheetNamcoWSG;
}

void DivPlatformNamcoWSG::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    switch (devType) {
      case 1:
        ((namco_device*)namco)->pacman_sound_w(w.addr,w.val);
        break;
      case 2:
        ((namco_device*)namco)->polepos_sound_w(w.addr,w.val);
        break;
      case 15:
        ((namco_15xx_device*)namco)->sharedram_w(w.addr,w.val);
        break;
      case 30:
        ((namco_cus30_device*)namco)->namcos1_cus30_w(w.addr,w.val);
        break;
    }
    regPool[w.addr&0x3f]=w.val;
    writes.pop();
  }
  for (size_t h=start; h<start+len; h++) {
    short* buf[2]={
      bufL+h, bufR+h
    };
    namco->sound_stream_update(buf,1);
    for (int i=0; i<chans; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=namco->m_channel_list[i].last_out*chans;
    }
  }
}

void DivPlatformNamcoWSG::updateWave(int ch) {
  if (devType==30) {
    for (int i=0; i<32; i++) {
      ((namco_cus30_device*)namco)->namcos1_cus30_w(i+ch*32,chan[ch].ws.output[i]);
    }
  } else {
    for (int i=0; i<32; i++) {
      namco->update_namco_waveform(i+ch*32,chan[ch].ws.output[i]);
    }
  }
}

void DivPlatformNamcoWSG::tick(bool sysTick) {
  for (int i=0; i<chans; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&15)*MIN(15,chan[i].std.vol.val))>>4;
    }
    if (chan[i].std.duty.had && i>=4) {
      chan[i].noise=chan[i].std.duty.val;
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
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
    if (chan[i].std.panL.had) {
      chan[i].pan&=0x0f;
      chan[i].pan|=(chan[i].std.panL.val&15)<<4;
    }
    if (chan[i].std.panR.had) {
      chan[i].pan&=0xf0;
      chan[i].pan|=chan[i].std.panR.val&15;
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
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1)) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_PCE);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq>1048575) chan[i].freq=1048575;
      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff) {
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  // update state
  switch (devType) {
    case 1:
      if (chan[0].active && !isMuted[0]) {
        rWrite(0x15,chan[0].outVol);
      } else {
        rWrite(0x15,0);
      }
      if (chan[1].active && !isMuted[1]) {
        rWrite(0x1a,chan[1].outVol);
      } else {
        rWrite(0x1a,0);
      }
      if (chan[2].active && !isMuted[2]) {
        rWrite(0x1f,chan[2].outVol);
      } else {
        rWrite(0x1f,0);
      }

      rWrite(0x10,(chan[0].freq)&15);
      rWrite(0x11,(chan[0].freq>>4)&15);
      rWrite(0x12,(chan[0].freq>>8)&15);
      rWrite(0x13,(chan[0].freq>>12)&15);
      rWrite(0x14,(chan[0].freq>>16)&15);

      rWrite(0x16,(chan[1].freq>>4)&15);
      rWrite(0x17,(chan[1].freq>>8)&15);
      rWrite(0x18,(chan[1].freq>>12)&15);
      rWrite(0x19,(chan[1].freq>>16)&15);

      rWrite(0x1b,(chan[2].freq>>4)&15);
      rWrite(0x1c,(chan[2].freq>>8)&15);
      rWrite(0x1d,(chan[2].freq>>12)&15);
      rWrite(0x1e,(chan[2].freq>>16)&15);

      rWrite(0x05,0);
      rWrite(0x0a,1);
      rWrite(0x0f,2);
      break;
    case 15:
      for (int i=0; i<8; i++) {
        if (chan[i].active && !isMuted[i]) {
          rWrite((i<<3)+0x03,chan[i].outVol);
        } else {
          rWrite((i<<3)+0x03,0);
        }
        rWrite((i<<3)+0x04,chan[i].freq&0xff);
        rWrite((i<<3)+0x05,(chan[i].freq>>8)&0xff);
        rWrite((i<<3)+0x06,((chan[i].freq>>16)&15)|(i<<4));
      }
      break;
    case 30:
      for (int i=0; i<8; i++) {
        if (chan[i].active && !isMuted[i]) {
          rWrite((i<<3)+0x100,(chan[i].outVol*((chan[i].pan>>4)&15))/15);
          rWrite((i<<3)+0x104,((chan[i].outVol*(chan[i].pan&15))/15)|(chan[(i+1)&7].noise?0x80:0));
        } else {
          rWrite((i<<3)+0x100,0);
          rWrite((i<<3)+0x104,(chan[(i+1)&7].noise?0x80:0));
        }
        rWrite((i<<3)+0x103,chan[i].freq&0xff);
        rWrite((i<<3)+0x102,(chan[i].freq>>8)&0xff);
        rWrite((i<<3)+0x101,((chan[i].freq>>16)&15)|(i<<4));
      }
      break;
  }
}

int DivPlatformNamcoWSG::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_PCE);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
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
      int destFreq=NOTE_FREQUENCY(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value*((parent->song.linearPitch==2)?1:8);
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value*((parent->song.linearPitch==2)?1:8);
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
      chan[c.chan].noise=c.value;
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_PCE));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
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

void DivPlatformNamcoWSG::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformNamcoWSG::forceIns() {
  for (int i=0; i<chans; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
  }
}

void* DivPlatformNamcoWSG::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformNamcoWSG::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformNamcoWSG::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformNamcoWSG::getRegisterPool() {
  return regPool;
}

int DivPlatformNamcoWSG::getRegisterPoolSize() {
  return (devType==1)?32:64;
}

void DivPlatformNamcoWSG::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,128);
  for (int i=0; i<chans; i++) {
    chan[i]=DivPlatformNamcoWSG::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,15,false);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  // TODO: wave memory
  namco->set_voices(chans);
  namco->set_stereo((devType==2 || devType==30));
  namco->device_start(NULL);
}

bool DivPlatformNamcoWSG::isStereo() {
  return (devType==30);
}

bool DivPlatformNamcoWSG::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformNamcoWSG::notifyWaveChange(int wave) {
  for (int i=0; i<chans; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformNamcoWSG::notifyInsDeletion(void* ins) {
  for (int i=0; i<chans; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformNamcoWSG::setDeviceType(int type) {
  devType=type;
  switch (type) {
    case 15:
      chans=8;
      break;
    case 30:
      chans=8;
      break;
    case 1:
      chans=3;
      break;
    case 2:
      chans=8;
      break;
  }
}

void DivPlatformNamcoWSG::setFlags(const DivConfig& flags) {
  chipClock=3072000;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/32;
  namco->device_clock_changed(rate);
  for (int i=0; i<chans; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformNamcoWSG::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformNamcoWSG::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformNamcoWSG::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<chans; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  switch (devType) {
    case 15:
      namco=new namco_15xx_device(3072000);
      break;
    case 30:
      namco=new namco_cus30_device(3072000);
      break;
    default:
      namco=new namco_device(3072000);
      break;
  }
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformNamcoWSG::quit() {
  for (int i=0; i<chans; i++) {
    delete oscBuf[i];
  }
  delete namco;
}

DivPlatformNamcoWSG::~DivPlatformNamcoWSG() {
}
