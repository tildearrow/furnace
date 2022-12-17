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

#include "n163.h"
#include "../engine.h"
#include <math.h>

#define rRead(a,v) n163.addr_w(a); n163.data_r(v);
#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }
#define rWriteMask(a,v,m) if (!skipRegisterWrites) {writes.emplace(a,v,m); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) \
  if (c<=chanMax) { \
    rWrite(0x78-(c<<3)+(a&7),v) \
  }

#define chWriteMask(c,a,v,m) \
  if (c<=chanMax) { \
    rWriteMask(0x78-(c<<3)+(a&7),v,m) \
  }

#define CHIP_FREQBASE (15*32768)

const char* regCheatSheetN163[]={
  "FreqL7", "40",
  "AccL7", "41",
  "FreqM7", "42",
  "AccM7", "43",
  "WavLen_FreqH7", "44",
  "AccH7", "45",
  "WavPos7", "46",
  "Vol7", "47",
  "FreqL6", "48",
  "AccL6", "49",
  "FreqM6", "4A",
  "AccM6", "4B",
  "WavLen_FreqH6", "4C",
  "AccH6", "4D",
  "WavPos6", "4E",
  "Vol6", "4F",
  "FreqL5", "50",
  "AccL5", "51",
  "FreqM5", "52",
  "AccM5", "53",
  "WavLen_FreqH5", "54",
  "AccH5", "55",
  "WavPos5", "56",
  "Vol5", "57",
  "FreqL4", "58",
  "AccL4", "59",
  "FreqM4", "5A",
  "AccM4", "5B",
  "WavLen_FreqH4", "5C",
  "AccH4", "5D",
  "WavPos4", "5E",
  "Vol4", "5F",
  "FreqL3", "60",
  "AccL3", "61",
  "FreqM3", "62",
  "AccM3", "63",
  "WavLen_FreqH3", "64",
  "AccH3", "65",
  "WavPos3", "66",
  "Vol3", "67",
  "FreqL2", "68",
  "AccL2", "69",
  "FreqM2", "6A",
  "AccM2", "6B",
  "WavLen_FreqH2", "6C",
  "AccH2", "6D",
  "WavPos2", "6E",
  "Vol2", "6F",
  "FreqL1", "70",
  "AccL1", "71",
  "FreqM1", "72",
  "AccM1", "73",
  "WavLen_FreqH1", "74",
  "AccH1", "75",
  "WavPos1", "76",
  "Vol1", "77",
  "FreqL0", "78",
  "AccL0", "79",
  "FreqM0", "7A",
  "AccM0", "7B",
  "WavLen_FreqH0", "7C",
  "AccH0", "7D",
  "WavPos0", "7E",
  "ChanMax_Vol0", "7F",
  NULL
};

const char** DivPlatformN163::getRegisterSheet() {
  return regCheatSheetN163;
}

void DivPlatformN163::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    n163.tick();
    int out=(n163.out()<<6)*2; // scale to 16 bit
    if (out>32767) out=32767;
    if (out<-32768) out=-32768;
    bufL[i]=bufR[i]=out;

    if (n163.voice_cycle()==0x78) for (int i=0; i<8; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=n163.voice_out(i)<<7;
    }

    // command queue
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      n163.addr_w(w.addr);
      n163.data_w((n163.data_r()&~w.mask)|(w.val&w.mask));
      writes.pop();
    }
  }
}

void DivPlatformN163::updateWave(int ch, int wave, int pos, int len) {
  len&=0xfc; // 4 nibble boundary
  if (wave<0) {
    // load from wave synth
    for (int i=0; i<len; i++) {
      unsigned char addr=(pos+i); // address (nibble each)
      if (addr>=((0x78-(chanMax<<3))<<1)) { // avoid conflict with channel register area
        break;
      }
      unsigned char mask=(addr&1)?0xf0:0x0f;
      int data=chan[ch].ws.output[i];
      rWriteMask(addr>>1,(addr&1)?(data<<4):(data&0xf),mask);
    }
  } else {
    // load from custom
    DivWavetable* wt=parent->getWave(wave);
    for (int i=0; i<len; i++) {
      unsigned char addr=(pos+i); // address (nibble each)
      if (addr>=((0x78-(chanMax<<3))<<1)) { // avoid conflict with channel register area
        break;
      }
      unsigned char mask=(addr&1)?0xf0:0x0f;
      if (wt->max<1 || wt->len<1) {
        rWriteMask(addr>>1,0,mask);
      } else {
        int data=wt->data[i*wt->len/len]*15/wt->max;
        if (data<0) data=0;
        if (data>15) data=15;
        rWriteMask(addr>>1,(addr&1)?(data<<4):(data&0xf),mask);
      }
    }
  }
}

void DivPlatformN163::updateWaveCh(int ch) {
  if (ch<=chanMax) {
    updateWave(ch,-1,chan[ch].wavePos,chan[ch].waveLen);
    if (chan[ch].active && !isMuted[ch]) {
      chan[ch].volumeChanged=true;
    }
  }
}

void DivPlatformN163::tick(bool sysTick) {
  for (int i=0; i<=chanMax; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(MIN(15,chan[i].std.vol.val)*(chan[i].vol&15))/15;
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (chan[i].outVol>15) chan[i].outVol=15;
      if (chan[i].resVol!=chan[i].outVol) {
        chan[i].resVol=chan[i].outVol;
        if (!isMuted[i]) {
          chan[i].volumeChanged=true;
        }
      }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      if (chan[i].wavePos!=chan[i].std.duty.val) {
        chan[i].wavePos=chan[i].std.duty.val;
        if (chan[i].waveMode&0x2) {
          chan[i].waveUpdated=true;
        }
        chan[i].waveChanged=true;
      }
    }
    if (chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (chan[i].waveMode&0x2) {
          chan[i].waveUpdated=true;
        }
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
    if (chan[i].std.ex1.had) {
      if (chan[i].waveLen!=(chan[i].std.ex1.val&0xfc)) {
        chan[i].waveLen=chan[i].std.ex1.val&0xfc;
        chan[i].ws.setWidth(chan[i].waveLen);
        if (chan[i].waveMode&0x2) {
          chan[i].waveUpdated=true;
        }
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.ex2.had) {
      if ((chan[i].waveMode&0x2)!=(chan[i].std.ex2.val&0x2)) { // update when every waveform changed
        chan[i].waveMode=(chan[i].waveMode&~0x2)|(chan[i].std.ex2.val&0x2);
        if (chan[i].waveMode&0x2) {
          chan[i].waveUpdated=true;
          chan[i].waveChanged=true;
        }
      }
      if ((chan[i].waveMode&0x1)!=(chan[i].std.ex2.val&0x1)) { // update waveform now
        chan[i].waveMode=(chan[i].waveMode&~0x1)|(chan[i].std.ex2.val&0x1);
        if (chan[i].waveMode&0x1) { // rising edge
          chan[i].waveUpdated=true;
          chan[i].waveChanged=true;
        }
      }
    }
    if (chan[i].std.ex3.had) {
      if (chan[i].loadWave!=chan[i].std.ex3.val) {
        chan[i].loadWave=chan[i].std.ex3.val;
        if (chan[i].loadMode&0x2) {
          updateWave(i,chan[i].loadWave,chan[i].loadPos,chan[i].loadLen&0xfc);
        }
      }
    }
    if (chan[i].std.alg.had) {
      if (chan[i].loadPos!=chan[i].std.alg.val) {
        chan[i].loadPos=chan[i].std.alg.val;
      }
    }
    if (chan[i].std.fb.had) {
      if (chan[i].loadLen!=(chan[i].std.fb.val&0xfc)) {
        chan[i].loadLen=chan[i].std.fb.val&0xfc;
      }
    }
    if (chan[i].std.fms.had) {
      if ((chan[i].loadMode&0x2)!=(chan[i].std.fms.val&0x2)) { // load when every waveform changes
        chan[i].loadMode=(chan[i].loadMode&~0x2)|(chan[i].std.fms.val&0x2);
      }
      if ((chan[i].loadMode&0x1)!=(chan[i].std.fms.val&0x1)) { // load now
        chan[i].loadMode=(chan[i].loadMode&~0x1)|(chan[i].std.fms.val&0x1);
        if (chan[i].loadMode&0x1) { // rising edge
          updateWave(i,chan[i].loadWave,chan[i].loadPos,chan[i].loadLen&0xfc);
        }
      }
    }
    if (chan[i].volumeChanged) {
      if (chan[i].active && !isMuted[i]) {
        chWriteMask(i,0x7,chan[i].resVol&0xf,0xf);
      } else {
        chWriteMask(i,0x7,0,0xf);
      }
      chan[i].volumeChanged=false;
    }
    if (chan[i].waveChanged) {
      chWrite(i,0x6,chan[i].wavePos);
      if (chan[i].active) {
        chan[i].freqChanged=true;
      }
      chan[i].waveChanged=false;
    }
    if (chan[i].active) {
      if (chan[i].ws.tick()) {
        chan[i].waveUpdated=true;
      }
    }
    if (chan[i].waveUpdated) {
      updateWaveCh(i);
      if (chan[i].active) {
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
      chan[i].waveUpdated=false;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      // TODO: what is this mess?
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      chan[i].freq=(((chan[i].freq*chan[i].waveLen)*(chanMax+1))/16);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0x3ffff) chan[i].freq=0x3ffff;
      if (chan[i].keyOn) {
        if (chan[i].wave<0) {
          chan[i].wave=0;
          if (chan[i].waveMode&0x2) {
            updateWaveCh(i);
          }
        }
      }
      if (chan[i].keyOff && !isMuted[i]) {
        chWriteMask(i,0x7,0,0xf);
      }
      chWrite(i,0x0,chan[i].freq&0xff);
      chWrite(i,0x2,chan[i].freq>>8);
      chWrite(i,0x4,((256-chan[i].waveLen)&0xfc)|((chan[i].freq>>16)&3));
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformN163::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_N163);
      if (chan[c.chan].insChanged) {
        chan[c.chan].wave=ins->n163.wave;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        chan[c.chan].wavePos=ins->n163.wavePos;
        chan[c.chan].waveLen=ins->n163.waveLen;
        chan[c.chan].waveMode=ins->n163.waveMode;
        chan[c.chan].waveChanged=true;
        if (chan[c.chan].waveMode&0x3 || ins->ws.enabled) {
          chan[c.chan].waveUpdated=true;
        }
        chan[c.chan].insChanged=false;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].resVol=chan[c.chan].vol;
      if (!isMuted[c.chan]) {
        chan[c.chan].volumeChanged=true;
      }
      chan[c.chan].macroInit(ins);
      chan[c.chan].ws.init(ins,chan[c.chan].waveLen,15,chan[c.chan].insChanged);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      //chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          chan[c.chan].resVol=chan[c.chan].outVol;
        } else {
          chan[c.chan].resVol=chan[c.chan].vol;
        }
        if (!isMuted[c.chan]) {
          chan[c.chan].volumeChanged=true;
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
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      if (chan[c.chan].waveMode&0x2) {
        chan[c.chan].waveUpdated=true;
      }
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_N163_WAVE_POSITION:
      chan[c.chan].wavePos=c.value;
      if (chan[c.chan].waveMode&0x2) {
        chan[c.chan].waveUpdated=true;
      }
      chan[c.chan].waveChanged=true;
      break;
    case DIV_CMD_N163_WAVE_LENGTH:
      chan[c.chan].waveLen=c.value&0xfc;
      if (chan[c.chan].waveMode&0x2) {
        chan[c.chan].waveUpdated=true;
      }
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_N163_WAVE_MODE:
      chan[c.chan].waveMode=c.value&0x3;
      if (chan[c.chan].waveMode&0x3) { // update now
        chan[c.chan].waveUpdated=true;
        chan[c.chan].waveChanged=true;
      }
      break;
    case DIV_CMD_N163_WAVE_LOAD:
      chan[c.chan].loadWave=c.value;
      if (chan[c.chan].loadMode&0x2) { // load when every waveform changes
        updateWave(c.chan,chan[c.chan].loadWave,chan[c.chan].loadPos,chan[c.chan].loadLen);
      }
      break;
    case DIV_CMD_N163_WAVE_LOADPOS:
      chan[c.chan].loadPos=c.value;
      break;
    case DIV_CMD_N163_WAVE_LOADLEN:
      chan[c.chan].loadLen=c.value&0xfc;
      break;
    case DIV_CMD_N163_WAVE_LOADMODE:
      chan[c.chan].loadMode=c.value&0x3;
      if (chan[c.chan].loadMode&0x1) { // load now
        updateWave(c.chan,chan[c.chan].loadWave,chan[c.chan].loadPos,chan[c.chan].loadLen);
      }
      break;
    case DIV_CMD_N163_GLOBAL_WAVE_LOAD:
      loadWave=c.value;
      if (loadMode&0x2) { // load when every waveform changes
        updateWave(c.chan,loadWave,loadPos,loadLen);
      }
      break;
    case DIV_CMD_N163_GLOBAL_WAVE_LOADPOS:
      loadPos=c.value;
      break;
    case DIV_CMD_N163_GLOBAL_WAVE_LOADLEN:
      loadLen=c.value&0xfc;
      break;
    case DIV_CMD_N163_GLOBAL_WAVE_LOADMODE:
      loadMode=c.value&0x3;
      if (loadMode&0x3) { // load now
        updateWave(c.chan,loadWave,loadPos,loadLen);
      }
      break;
    case DIV_CMD_N163_CHANNEL_LIMIT:
      if (chanMax!=(c.value&0x7)) {
        chanMax=c.value&0x7;
        rWriteMask(0x7f,chanMax<<4,0x70);
        forceIns();
      }
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_N163));
          chan[c.chan].keyOn=true;
        }
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

void DivPlatformN163::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volumeChanged=true;
}

void DivPlatformN163::forceIns() {
  for (int i=0; i<=chanMax; i++) {
    chan[i].insChanged=true;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
      chan[i].volumeChanged=true;
      chan[i].waveChanged=true;
      if (chan[i].waveMode&0x2) {
        chan[i].waveUpdated=true;
      }
    }
  }
}

void DivPlatformN163::notifyWaveChange(int wave) {
  for (int i=0; i<8; i++) {
    if (chan[i].wave==wave) {
      if (chan[i].waveMode&0x2) {
        chan[i].ws.changeWave1(wave);
        chan[i].waveUpdated=true;
      }
    }
  }
}

void DivPlatformN163::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformN163::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformN163::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformN163::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformN163::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformN163::getRegisterPool() {
  for (int i=0; i<128; i++) {
    regPool[i]=n163.reg(i);
  }
  return regPool;
}

int DivPlatformN163::getRegisterPoolSize() {
  return 128;
}

void DivPlatformN163::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformN163::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,15,false);
  }

  n163.reset();
  memset(regPool,0,128);

  n163.set_disable(false);
  n163.set_multiplex(multiplex);
  chanMax=initChanMax;
  loadWave=-1;
  loadPos=0;
  loadLen=0;
  loadMode=0;
  rWrite(0x7f,initChanMax<<4);
}

void DivPlatformN163::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformN163::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformN163::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 1: // PAL
      chipClock=COLOR_PAL*3.0/8.0;
      break;
    case 2: // Dendy
      chipClock=COLOR_PAL*2.0/5.0;
      break;
    default: // NTSC
      chipClock=COLOR_NTSC/2.0;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  initChanMax=chanMax=flags.getInt("channels",0)&7;
  multiplex=!flags.getBool("multiplex",false); // not accurate in real hardware
  rate=chipClock;
  rate/=15;
  n163.set_multiplex(multiplex);
  rWrite(0x7f,initChanMax<<4);
  for (int i=0; i<8; i++) {
    oscBuf[i]->rate=rate/(initChanMax+1);
  }

  // needed to make sure changing channel count won't trigger glitches
  reset();
}

int DivPlatformN163::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  reset();

  return 8;
}

void DivPlatformN163::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}

DivPlatformN163::~DivPlatformN163() {
}
