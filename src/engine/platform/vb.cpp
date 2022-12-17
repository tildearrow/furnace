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

#include "vb.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) rWrite(0x400+((c)<<6)+((a)<<2),v);

#define CHIP_DIVIDER 16

const char* regCheatSheetVB[]={
  "Wave0", "000",
  "Wave1", "080",
  "Wave2", "100",
  "Wave3", "180",
  "Wave4", "200",
  "ModTable", "280",

  "S1INT", "400",
  "S1LRV", "404",
  "S1FQL", "408",
  "S1FQH", "40C",
  "S1EV0", "410",
  "S1EV1", "414",
  "S1RAM", "418",

  "S2INT", "440",
  "S2LRV", "444",
  "S2FQL", "448",
  "S2FQH", "44C",
  "S2EV0", "450",
  "S2EV1", "454",
  "S2RAM", "458",

  "S3INT", "480",
  "S3LRV", "484",
  "S3FQL", "488",
  "S3FQH", "48C",
  "S3EV0", "480",
  "S3EV1", "484",
  "S3RAM", "488",

  "S4INT", "4C0",
  "S4LRV", "4C4",
  "S4FQL", "4C8",
  "S4FQH", "4CC",
  "S4EV0", "4C0",
  "S4EV1", "4C4",
  "S4RAM", "4C8",

  "S5INT", "500",
  "S5LRV", "504",
  "S5FQL", "508",
  "S5FQH", "50C",
  "S5EV0", "510",
  "S5EV1", "514",
  "S5RAM", "518",

  "S5SWP", "51C",

  "S6INT", "540",
  "S6LRV", "544",
  "S6FQL", "548",
  "S6FQH", "54C",
  "S6EV0", "550",
  "S6EV1", "554",
  "S6RAM", "558",

  "RESET", "580",
  NULL
};

const char** DivPlatformVB::getRegisterSheet() {
  return regCheatSheetVB;
}

void DivPlatformVB::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    cycles=0;
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      vb->Write(cycles,w.addr,w.val);
      regPool[w.addr>>2]=w.val;
      writes.pop();
    }
    vb->EndFrame(16);

    tempL=0;
    tempR=0;
    for (int i=0; i<6; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(vb->last_output[i][0]+vb->last_output[i][1])*8;
      tempL+=vb->last_output[i][0];
      tempR+=vb->last_output[i][1];
    }

    if (tempL<-32768) tempL=-32768;
    if (tempL>32767) tempL=32767;
    if (tempR<-32768) tempR=-32768;
    if (tempR>32767) tempR=32767;
    
    bufL[h]=tempL;
    bufR[h]=tempR;
  }
}

void DivPlatformVB::updateWave(int ch) {
  if (ch>=5) return;

  for (int i=0; i<32; i++) {
    rWrite((ch<<7)+(i<<2),chan[ch].ws.output[i]);
  }
}

void DivPlatformVB::writeEnv(int ch, bool upperByteToo) {
  chWrite(ch,0x04,(chan[ch].outVol<<4)|(chan[ch].envLow&15));
  if (ch<5 || upperByteToo) {
    chWrite(ch,0x05,chan[ch].envHigh);
  }
}

void DivPlatformVB::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      writeEnv(i);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (i==5 && chan[i].std.duty.had) {
      if ((chan[i].std.duty.val&7)!=((chan[i].envHigh>>4)&7)) {
        chan[i].envHigh&=~0x70;
        chan[i].envHigh|=(chan[i].std.duty.val&7)<<4;
        writeEnv(i,true);
      }
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
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      chWrite(i,0x01,isMuted[i]?0:chan[i].pan);
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
      chWrite(i,0x00,0x80);
    }
    if (chan[i].active) {
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1)) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].freq<1) chan[i].freq=1;
      if (chan[i].freq>2047) chan[i].freq=2047;
      chan[i].freq=2048-chan[i].freq;
      chWrite(i,0x02,chan[i].freq&0xff);
      chWrite(i,0x03,chan[i].freq>>8);
      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff) {
        chWrite(i,0x04,0);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformVB::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_PCE);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (chan[c.chan].insChanged && ins->fds.initModTableWithFirstWave) {
        for (int i=0; i<32; i++) {
          modTable[i]=ins->fds.modTable[i];
          rWrite(0x280+(i<<2),modTable[i]);
        }
      }
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
        writeEnv(c.chan);
      }
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }
      chan[c.chan].ws.init(ins,32,63,chan[c.chan].insChanged);
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
          if (chan[c.chan].active) {
            writeEnv(c.chan);
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
    case DIV_CMD_STD_NOISE_MODE:
      if (c.chan!=5) break;
      chan[c.chan].envHigh&=~0x70;
      chan[c.chan].envHigh|=(c.value&7)<<4;
      writeEnv(c.chan,true);
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      chan[c.chan].envHigh&=~3;
      chan[c.chan].envHigh|=(c.value>>4)&3;
      chan[c.chan].envLow=c.value&15;
      writeEnv(c.chan);
      break;
    case DIV_CMD_FDS_MOD_DEPTH: // set modulation
      if (c.chan!=4) break;
      modulation=(c.value&15)<<4;
      modType=true;
      chWrite(4,0x07,modulation);
      if (modulation!=0) {
        chan[c.chan].envHigh&=~0x70;
        chan[c.chan].envHigh|=0x40|(c.value&0xf0);
      } else {
        chan[c.chan].envHigh&=~0x70;
      }
      writeEnv(4);
      break;
    case DIV_CMD_GB_SWEEP_TIME: // set sweep
      if (c.chan!=4) break;
      modulation=c.value;
      modType=false;
      chWrite(4,0x07,modulation);
      if (modulation!=0) {
        chan[c.chan].envHigh&=~0x70;
        chan[c.chan].envHigh|=0x40;
      } else {
        chan[c.chan].envHigh&=~0x70;
      }
      writeEnv(4);
      break;
    case DIV_CMD_FDS_MOD_WAVE: { // set modulation wave
      if (c.chan!=4) break;
      DivWavetable* wt=parent->getWave(c.value);
      for (int i=0; i<32; i++) {
        if (wt->max<1 || wt->len<1) {
          modTable[i]=0;
          rWrite(0x280+(i<<2),0);
        } else {
          int data=(wt->data[i*wt->len/32]*255/wt->max)-128;
          if (data<-128) data=-128;
          if (data>127) data=127;
          modTable[i]=data;
          rWrite(0x280+(i<<2),modTable[i]);
        }
      }
      break;
    }
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      chWrite(c.chan,0x01,isMuted[c.chan]?0:chan[c.chan].pan);
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
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

void DivPlatformVB::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chWrite(ch,0x01,isMuted[ch]?0:chan[ch].pan);
}

void DivPlatformVB::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
    chWrite(i,0x01,isMuted[i]?0:chan[i].pan);
  }
}

void* DivPlatformVB::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformVB::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformVB::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformVB::getRegisterPool() {
  return regPool;
}

int DivPlatformVB::getRegisterPoolSize() {
  return 0x180;
}

int DivPlatformVB::getRegisterPoolDepth() {
  return 8;
}

void DivPlatformVB::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,0x600);
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformVB::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,63,false);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  vb->Power();
  tempL=0;
  tempR=0;
  cycles=0;
  curChan=-1;
  modulation=0;
  modType=false;
  memset(modTable,0,32);
  // set per-channel initial values
  for (int i=0; i<6; i++) {
    chWrite(i,0x01,isMuted[i]?0:chan[i].pan);
    chWrite(i,0x05,0x00);
    chWrite(i,0x00,0x80);
    chWrite(i,0x06,i);
  }
  delay=500;
}

bool DivPlatformVB::isStereo() {
  return true;
}

bool DivPlatformVB::keyOffAffectsArp(int ch) {
  return true;
}

float DivPlatformVB::getPostAmp() {
  return 6.0f;
}

void DivPlatformVB::notifyWaveChange(int wave) {
  for (int i=0; i<6; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformVB::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformVB::setFlags(const DivConfig& flags) {
  chipClock=5000000.0;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/16;
  for (int i=0; i<6; i++) {
    oscBuf[i]->rate=rate;
  }

  if (vb!=NULL) {
    delete vb;
    vb=NULL;
  }
  vb=new VSU;
}

void DivPlatformVB::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformVB::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformVB::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  vb=NULL;
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformVB::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  if (vb!=NULL) {
    delete vb;
    vb=NULL;
  }
}

DivPlatformVB::~DivPlatformVB() {
}
