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

#include "saa.h"
#include "../engine.h"
#include "sound/saa1099.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 2

const char* regCheatSheetSAA[]={
  "Vol0", "00",
  "Vol1", "01",
  "Vol2", "02",
  "Vol3", "03",
  "Vol4", "04",
  "Vol5", "05",
  "Freq0", "08",
  "Freq1", "09",
  "Freq2", "0A",
  "Freq3", "0B",
  "Freq4", "0C",
  "Freq5", "0D",
  "Octave10", "10",
  "Octave32", "11",
  "Octave54", "12",
  "ToneOn", "14",
  "NoiseOn", "15",
  "NoiseCtl", "16",
  "EnvCtl0", "18",
  "EnvCtl1", "19",
  "Power", "1C",
  NULL
};

const char** DivPlatformSAA1099::getRegisterSheet() {
  return regCheatSheetSAA;
}

void DivPlatformSAA1099::acquire_saaSound(short* bufL, short* bufR, size_t start, size_t len) {
  if (saaBufLen<len*2) {
    saaBufLen=len*2;
    for (int i=0; i<2; i++) {
      delete[] saaBuf[i];
      saaBuf[i]=new short[saaBufLen];
    }
  }
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    saa_saaSound->WriteAddressData(w.addr,w.val);
    regPool[w.addr&0x1f]=w.val;
    writes.pop();
  }
  saa_saaSound->GenerateMany((unsigned char*)saaBuf[0],len,oscBuf);
#ifdef TA_BIG_ENDIAN
  for (size_t i=0; i<len; i++) {
    bufL[i+start]=(short)((((unsigned short)saaBuf[0][1+(i<<1)])<<8)|(((unsigned short)saaBuf[0][1+(i<<1)])>>8));
    bufR[i+start]=(short)((((unsigned short)saaBuf[0][i<<1])<<8)|(((unsigned short)saaBuf[0][i<<1])>>8));
  }
#else
  for (size_t i=0; i<len; i++) {
    bufL[i+start]=saaBuf[0][i<<1];
    bufR[i+start]=saaBuf[0][1+(i<<1)];
  }
#endif
}

void DivPlatformSAA1099::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  acquire_saaSound(bufL,bufR,start,len);
}

inline unsigned char applyPan(unsigned char vol, unsigned char pan) {
  return ((vol*(pan>>4))/15)|(((vol*(pan&15))/15)<<4);
}

void DivPlatformSAA1099::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR_BROKEN(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(i,0);
      } else {
        rWrite(i,applyPan(chan[i].outVol&15,chan[i].pan));
      }
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
      saaNoise[i/3]=chan[i].std.duty.val&3;
      rWrite(0x16,saaNoise[0]|(saaNoise[1]<<4));
    }
    if (chan[i].std.wave.had) {
      chan[i].psgMode=chan[i].std.wave.val&3;
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
      if (isMuted[i]) {
        rWrite(i,0);
      } else {
        if (chan[i].std.vol.had) {
          if (chan[i].active) rWrite(i,applyPan(chan[i].outVol&15,chan[i].pan));
        } else {
          if (chan[i].active) rWrite(i,applyPan(chan[i].vol&15,chan[i].pan));
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
      saaEnv[i/3]=chan[i].std.ex1.val;
      rWrite(0x18+(i/3),saaEnv[i/3]);
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].freq>=32768) {
        chan[i].freqH=7;
      } else if (chan[i].freq>=16384) {
        chan[i].freqH=6;
      } else if (chan[i].freq>=8192) {
        chan[i].freqH=5;
      } else if (chan[i].freq>=4096) {
        chan[i].freqH=4;
      } else if (chan[i].freq>=2048) {
        chan[i].freqH=3;
      } else if (chan[i].freq>=1024) {
        chan[i].freqH=2;
      } else if (chan[i].freq>=512) {
        chan[i].freqH=1;
      } else {
        chan[i].freqH=0;
      }
      chan[i].freqL=0xff-(chan[i].freq>>chan[i].freqH);
      chan[i].freqH=7-chan[i].freqH;
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff) {
        rWrite(i,0);
      }
      rWrite(0x08+i,chan[i].freqL);
      rWrite(0x10+(i>>1),chan[i&6].freqH|(chan[1+(i&6)].freqH<<4));
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  rWrite(0x14,(chan[0].psgMode&1)|
              ((chan[1].psgMode&1)<<1)|
              ((chan[2].psgMode&1)<<2)|
              ((chan[3].psgMode&1)<<3)|
              ((chan[4].psgMode&1)<<4)|
              ((chan[5].psgMode&1)<<5)
  );

  rWrite(0x15,((chan[0].psgMode&2)>>1)|
              (chan[1].psgMode&2)|
              ((chan[2].psgMode&2)<<1)|
              ((chan[3].psgMode&2)<<2)|
              ((chan[4].psgMode&2)<<3)|
              ((chan[5].psgMode&2)<<4)
  );
}

int DivPlatformSAA1099::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SAA1099);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (isMuted[c.chan]) {
        rWrite(c.chan,0);
      } else {
        rWrite(c.chan,applyPan(chan[c.chan].vol&15,chan[c.chan].pan));
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (isMuted[c.chan]) {
        rWrite(c.chan,0);
      } else {
        if (chan[c.chan].active) rWrite(c.chan,applyPan(chan[c.chan].vol&15,chan[c.chan].pan));
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value*((parent->song.linearPitch==2)?1:(8-chan[c.chan].freqH));
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value*((parent->song.linearPitch==2)?1:(8-chan[c.chan].freqH));
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
    case DIV_CMD_PANNING:
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      if (isMuted[c.chan]) {
        rWrite(c.chan,0);
      } else {
        if (chan[c.chan].active) rWrite(c.chan,applyPan(chan[c.chan].vol&15,chan[c.chan].pan));
      }
      break;
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].psgMode=(c.value&1)|((c.value&16)>>3);
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      saaNoise[c.chan/3]=(c.value&1)|((c.value&16)>>3);
      rWrite(0x16,saaNoise[0]|(saaNoise[1]<<4));
      break;
    case DIV_CMD_SAA_ENVELOPE:
      saaEnv[c.chan/3]=c.value;
      rWrite(0x18+(c.chan/3),c.value);
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SAA1099));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformSAA1099::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(ch,0);
  } else {
    if (chan[ch].active) rWrite(ch,applyPan(chan[ch].outVol&15,chan[ch].pan));
  }
}

void DivPlatformSAA1099::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
  rWrite(0x18,saaEnv[0]);
  rWrite(0x19,saaEnv[1]);
  rWrite(0x16,saaNoise[0]|(saaNoise[1]<<4));
}

void* DivPlatformSAA1099::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSAA1099::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSAA1099::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSAA1099::getRegisterPool() {
  return regPool;
}

int DivPlatformSAA1099::getRegisterPoolSize() {
  return 32;
}

void DivPlatformSAA1099::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,32);
  saa_saaSound->Clear();
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformSAA1099::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x0f;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  lastBusy=60;
  dacMode=0;
  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;
  saaEnv[0]=0;
  saaEnv[1]=0;
  saaNoise[0]=0;
  saaNoise[1]=0;

  delay=0;

  extMode=false;

  rWrite(8,255);
  rWrite(9,255);
  rWrite(10,255);
  rWrite(11,255);
  rWrite(12,255);
  rWrite(13,255);
  rWrite(16,0x77);
  rWrite(17,0x77);
  rWrite(18,0x77);
  rWrite(0x1c,2);
  rWrite(0x1c,1);
}

bool DivPlatformSAA1099::isStereo() {
  return true;
}

int DivPlatformSAA1099::getPortaFloor(int ch) {
  return 12;
}

bool DivPlatformSAA1099::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformSAA1099::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSAA1099::setFlags(const DivConfig& flags) {
  int clockSel=flags.getInt("clockSel",0);
  if (clockSel==2) {
    chipClock=COLOR_PAL*8.0/5.0;
  } else if (clockSel==1) {
    chipClock=COLOR_NTSC*2.0;
  } else {
    chipClock=8000000;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/32;

  for (int i=0; i<6; i++) {
    oscBuf[i]->rate=rate;
  }

  saa_saaSound->SetClockRate(chipClock);
  saa_saaSound->SetSampleRate(rate);
}

void DivPlatformSAA1099::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSAA1099::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformSAA1099::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  saa_saaSound=NULL;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  saa_saaSound=CreateCSAASound();
  saa_saaSound->SetOversample(1);
  saa_saaSound->SetSoundParameters(SAAP_NOFILTER|SAAP_16BIT|SAAP_STEREO);
  setFlags(flags);
  saaBufLen=65536;
  for (int i=0; i<2; i++) saaBuf[i]=new short[saaBufLen];
  reset();
  return 3;
}

void DivPlatformSAA1099::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  if (saa_saaSound!=NULL) {
    DestroyCSAASound(saa_saaSound);
    saa_saaSound=NULL;
  }
  for (int i=0; i<2; i++) delete[] saaBuf[i];
}
