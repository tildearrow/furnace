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

#include "swan.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);}}

#define CHIP_DIVIDER 32

const char* regCheatSheetWS[]={
  "CH1_Pitch", "00",
  "CH2_Pitch", "02",
  "CH3_Pitch", "04",
  "CH4_Pitch", "06",
  "CH1_Vol", "08",
  "CH2_Vol", "09",
  "CH3_Vol", "0A",
  "CH4_Vol", "0B",
  "Sweep_Value", "0C",
  "Sweep_Time", "0D",
  "Noise", "0E",
  "Wave_Base", "0F",
  "Ctrl", "10",
  "Output", "11",
  "Random", "12",
  "Voice_Ctrl", "14",
  "Wave_Mem", "40",
  NULL
};

const char** DivPlatformSwan::getRegisterSheet() {
  return regCheatSheetWS;
}

void DivPlatformSwan::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    // PCM part
    if (pcm && dacSample!=-1) {
      dacPeriod+=dacRate;
      while (dacPeriod>rate) {
        DivSample* s=parent->getSample(dacSample);
        if (s->samples<=0) {
          dacSample=-1;
          dacPeriod=0;
          break;
        }
        rWrite(0x09,(unsigned char)s->data8[dacPos++]+0x80);
        if (s->isLoopable() && dacPos>=(unsigned int)s->loopEnd) {
          dacPos=s->loopStart;
        } else if (dacPos>=s->samples) {
          dacSample=-1;
        }
        dacPeriod-=rate;
      }
    }
  
    // the rest
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      regPool[w.addr]=w.val;
      if (w.addr<0x40) ws->SoundWrite(w.addr|0x80,w.val);
      else ws->RAMWrite(w.addr&0x3f,w.val);
      writes.pop();
    }
    int16_t samp[2]{0, 0};
    ws->SoundUpdate(16);
    ws->SoundFlush(samp, 1);
    bufL[h]=samp[0];
    bufR[h]=samp[1];
    for (int i=0; i<4; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(ws->sample_cache[i][0]+ws->sample_cache[i][1])<<6;
    }
  }
}

void DivPlatformSwan::updateWave(int ch) {
  unsigned char addr=0x40+ch*16;
  for (int i=0; i<16; i++) {
    int nibble1=chan[ch].ws.output[i<<1];
    int nibble2=chan[ch].ws.output[1+(i<<1)];
    rWrite(addr+i,nibble1|(nibble2<<4));
  }
}

void DivPlatformSwan::calcAndWriteOutVol(int ch, int env) {
  int vl=chan[ch].vol*((chan[ch].pan>>4)&0x0f)*env/225;
  int vr=chan[ch].vol*(chan[ch].pan&0x0f)*env/225;
  if (ch==1&&pcm) {
    vl=(vl>0)?((vl>7)?3:2):0;
    vr=(vr>0)?((vr>7)?3:2):0;
    chan[1].outVol=vr|(vl<<2);
  } else {
    chan[ch].outVol=vr|(vl<<4);
  }
  writeOutVol(ch);
}

void DivPlatformSwan::writeOutVol(int ch) {
  unsigned char val=isMuted[ch]?0:chan[ch].outVol;
  if (ch==1&&pcm) {
    rWrite(0x14,val)
  } else {
    rWrite(0x08+ch,val);
  }
}

void DivPlatformSwan::tick(bool sysTick) {
  unsigned char sndCtrl=(pcm?0x20:0)|(sweep?0x40:0)|((noise>0)?0x80:0);
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      int env=chan[i].std.vol.val;
      if(parent->getIns(chan[i].ins,DIV_INS_SWAN)->type==DIV_INS_AMIGA) {
        env=MIN(env/4,15);
      }
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
    if (chan[i].std.wave.had && !(i==1 && pcm)) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
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
      calcAndWriteOutVol(i,chan[i].std.vol.will?chan[i].std.vol.val:15);
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
      sndCtrl|=(1<<i);
      if (chan[i].ws.tick()) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (i==1 && pcm && furnaceDac) {
        double off=1.0;
        if (dacSample>=0 && dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=8363.0/(double)s->centerRate;
          }
        }
        dacRate=((double)chipClock/2)/MAX(1,off*chan[i].freq);
        if (dumpWrites) addWrite(0xffff0001,dacRate);
      }
      if (chan[i].freq>2048) chan[i].freq=2048;
      if (chan[i].freq<1) chan[i].freq=1;
      int rVal=2048-chan[i].freq;
      rWrite(i*2,rVal&0xff);
      rWrite(i*2+1,rVal>>8);
      if (chan[i].keyOn) {
        if (!chan[i].std.vol.will) {
          calcAndWriteOutVol(i,15);
        }
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chan[i].keyOff=false;
      }
      chan[i].freqChanged=false;
    }
  }
  if (chan[3].std.duty.had) {
    if (noise!=chan[3].std.duty.val) {
      noise=chan[3].std.duty.val;
      if (noise>0) {
        rWrite(0x0e,((noise-1)&0x07)|0x18);
        sndCtrl|=0x80;
      } else {
        sndCtrl&=~0x80;
      }
    }
  }
  rWrite(0x10,sndCtrl);
}

int DivPlatformSwan::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SWAN);
      if (c.chan==1) {
        if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
          pcm=true;
        } else if (furnaceDac) {
          pcm=false;
        }
        if (pcm) {
          if (skipRegisterWrites) break;
          dacPos=0;
          dacPeriod=0;
          if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
            if (c.value!=DIV_NOTE_NULL) dacSample=ins->amiga.getSample(c.value);
            if (dacSample<0 || dacSample>=parent->song.sampleLen) {
              dacSample=-1;
              if (dumpWrites) addWrite(0xffff0002,0);
              break;
            } else {
              if (dumpWrites) {
                addWrite(0xffff0000,dacSample);
              }
            }
            if (c.value!=DIV_NOTE_NULL) {
              chan[1].baseFreq=NOTE_PERIODIC(c.value);
              chan[1].freqChanged=true;
              chan[1].note=c.value;
            }
            chan[1].active=true;
            chan[1].keyOn=true;
            chan[1].macroInit(ins);
            furnaceDac=true;
          } else {
            if (c.value!=DIV_NOTE_NULL) {
              chan[1].note=c.value;
            }
            dacSample=12*sampleBank+chan[1].note%12;
            if (dacSample>=parent->song.sampleLen) {
              dacSample=-1;
              if (dumpWrites) addWrite(0xffff0002,0);
              break;
            } else {
              if (dumpWrites) addWrite(0xffff0000,dacSample);
            }
            dacRate=parent->getSample(dacSample)->rate;
            if (dumpWrites) {
              addWrite(0xffff0001,dacRate);
            }
            chan[1].active=true;
            chan[1].keyOn=true;
            furnaceDac=false;
          }
          break;
        }
      }
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
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }
      chan[c.chan].ws.init(ins,32,15,chan[c.chan].insChanged);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan==1&&pcm) {
        dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
        pcm=false;
      }
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
      chan[c.chan].wave=c.value;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_WS_SWEEP_TIME:
      if (c.chan==2) {
        if (c.value==0) {
          sweep=false;
        } else {
          sweep=true;
          rWrite(0x0d,(c.value-1)&0xff);
        }
      }
      break;
    case DIV_CMD_WS_SWEEP_AMOUNT:
      if (c.chan==2) {
        rWrite(0x0c,c.value&0xff);
      }
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
      if (c.chan==3) {
        noise=c.value&0xff;
        if (noise>0) rWrite(0x0e,((noise-1)&0x07)|0x18);
      }
      break;
    case DIV_CMD_SAMPLE_MODE:
      if (c.chan==1) pcm=c.value;
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      calcAndWriteOutVol(c.chan,chan[c.chan].std.vol.will?chan[c.chan].std.vol.val:15);
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SWAN));
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

void DivPlatformSwan::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  writeOutVol(ch);
}

void DivPlatformSwan::forceIns() {
  noise=0;
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
    writeOutVol(i);
  }
}

void* DivPlatformSwan::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSwan::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSwan::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSwan::getRegisterPool() {
  // get Random from emulator
  regPool[0x12]=ws->SoundRead(0x92);
  regPool[0x13]=ws->SoundRead(0x93);
  return regPool;
}

int DivPlatformSwan::getRegisterPoolSize() {
  return 128;
}

void DivPlatformSwan::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,128);
  for (int i=0; i<4; i++) {
    chan[i]=Channel();
    chan[i].vol=15;
    chan[i].pan=0xff;
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,15,false);
    rWrite(0x08+i,0xff);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  ws->SoundReset();
  pcm=false;
  sweep=false;
  furnaceDac=false;
  noise=0;
  dacPeriod=0;
  dacRate=0;
  dacPos=0;
  dacSample=-1;
  sampleBank=0;
  rWrite(0x0f,0x00); // wave table at 0x0000
  rWrite(0x11,0x09); // enable speakers
}

bool DivPlatformSwan::isStereo() {
  return true;
}

void DivPlatformSwan::notifyWaveChange(int wave) {
  for (int i=0; i<4; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformSwan::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSwan::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSwan::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformSwan::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  chipClock=3072000;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/16; // = 192000kHz, should be enough
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->rate=rate;
  }
  ws=new WSwan();
  reset();
  return 4;
}

void DivPlatformSwan::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  delete ws;
}

DivPlatformSwan::~DivPlatformSwan() {
}
