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

#include "ay.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "sound/ay8910.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(regRemap(a),v); if (dumpWrites) {addWrite(regRemap(a),v);} }

#define CHIP_DIVIDER 8

const char* regCheatSheetAY[]={
  "FreqL_A", "0",
  "FreqH_A", "1",
  "FreqL_B", "2",
  "FreqH_B", "3",
  "FreqL_C", "4",
  "FreqH_C", "5",
  "FreqNoise", "6",
  "Enable", "7",
  "Volume_A", "8",
  "Volume_B", "9",
  "Volume_C", "A",
  "FreqL_Env", "B",
  "FreqH_Env", "C",
  "Control_Env", "D",
  "PortA", "E",
  "PortB", "F",
  NULL
};

const char* regCheatSheetAY8914[]={
  "FreqL_A", "0",
  "FreqL_B", "1",
  "FreqL_C", "2",
  "FreqL_Env", "3",
  "FreqH_A", "4",
  "FreqH_B", "5",
  "FreqH_C", "6",
  "FreqH_Env", "7",
  "Enable", "8",
  "FreqNoise", "9",
  "Control_Env", "A",
  "Volume_A", "B",
  "Volume_B", "C",
  "Volume_C", "D",
  "PortA", "E",
  "PortB", "F",
  NULL
};

const char** DivPlatformAY8910::getRegisterSheet() {
  return intellivision?regCheatSheetAY8914:regCheatSheetAY;
}

const char* DivPlatformAY8910::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x20:
      return "20xx: Set channel mode (bit 0: square; bit 1: noise; bit 2: envelope)";
      break;
    case 0x21:
      return "21xx: Set noise frequency (0 to 1F)";
      break;
    case 0x22:
      return "22xy: Set envelope mode (x: shape, y: enable for this channel)";
      break;
    case 0x23:
      return "23xx: Set envelope period low byte";
      break;
    case 0x24:
      return "24xx: Set envelope period high byte";
      break;
    case 0x25:
      return "25xx: Envelope slide up";
      break;
    case 0x26:
      return "26xx: Envelope slide down";
      break;
    case 0x29:
      return "29xy: Set auto-envelope (x: numerator; y: denominator)";
      break;
    case 0x2e:
      return "2Exx: Write to I/O port A";
      break;
    case 0x2f:
      return "2Fxx: Write to I/O port B";
      break;
  }
  return NULL;
}

void DivPlatformAY8910::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (ayBufLen<len) {
    ayBufLen=len;
    for (int i=0; i<3; i++) {
      delete[] ayBuf[i];
      ayBuf[i]=new short[ayBufLen];
    }
  }
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    if (intellivision) {
      ay8914_device* ay8914=(ay8914_device*)ay;
      ay8914->write(w.addr,w.val);
    } else {
      ay->address_w(w.addr);
      ay->data_w(w.val);
    }
    regPool[w.addr&0x0f]=w.val;
    writes.pop();
  }
  ay->sound_stream_update(ayBuf,len);
  if (sunsoft) {
    for (size_t i=0; i<len; i++) {
      bufL[i+start]=ayBuf[0][i];
      bufR[i+start]=bufL[i+start];
    }
  } else if (stereo) {
    for (size_t i=0; i<len; i++) {
      bufL[i+start]=ayBuf[0][i]+ayBuf[1][i];
      bufR[i+start]=ayBuf[1][i]+ayBuf[2][i];
    }
  } else {
    for (size_t i=0; i<len; i++) {
      bufL[i+start]=ayBuf[0][i]+ayBuf[1][i]+ayBuf[2][i];
      bufR[i+start]=bufL[i+start];
    }
  }
}

void DivPlatformAY8910::updateOutSel(bool immediate) {
  if (immediate) {
    immWrite(0x07,
          ~((chan[0].psgMode&1)|
           ((chan[1].psgMode&1)<<1)|
           ((chan[2].psgMode&1)<<2)|
           ((chan[0].psgMode&2)<<2)|
           ((chan[1].psgMode&2)<<3)|
           ((chan[2].psgMode&2)<<4)|
           ((!ioPortA)<<6)|
           ((!ioPortB)<<7)));
  } else {
    rWrite(0x07,
          ~((chan[0].psgMode&1)|
           ((chan[1].psgMode&1)<<1)|
           ((chan[2].psgMode&1)<<2)|
           ((chan[0].psgMode&2)<<2)|
           ((chan[1].psgMode&2)<<3)|
           ((chan[2].psgMode&2)<<4)|
           ((!ioPortA)<<6)|
           ((!ioPortB)<<7)));
  }
}

void DivPlatformAY8910::tick(bool sysTick) {
  // PSG
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MIN(15,chan[i].std.vol.val)-(15-(chan[i].vol&15));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(0x08+i,0);
      } else if (intellivision && (chan[i].psgMode&4)) {
        rWrite(0x08+i,(chan[i].outVol&0xc)<<2);
      } else {
        rWrite(0x08+i,(chan[i].outVol&15)|((chan[i].psgMode&4)<<2));
      }
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp.val);
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
      rWrite(0x06,31-chan[i].std.duty.val);
    }
    if (chan[i].std.wave.had) {
      chan[i].psgMode=(chan[i].std.wave.val+1)&7;
      if (isMuted[i]) {
        rWrite(0x08+i,0);
      } else if (intellivision && (chan[i].psgMode&4)) {
        rWrite(0x08+i,(chan[i].outVol&0xc)<<2);
      } else {
        rWrite(0x08+i,(chan[i].outVol&15)|((chan[i].psgMode&4)<<2));
      }
    }
    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        oldWrites[0x08+i]=-1;
        oldWrites[0x0d]=-1;
      }
    }
    if (chan[i].std.ex2.had) {
      ayEnvMode=chan[i].std.ex2.val;
      rWrite(0x0d,ayEnvMode);
    }
    if (chan[i].std.ex3.had) {
      chan[i].autoEnvNum=chan[i].std.ex3.val;
      chan[i].freqChanged=true;
      if (!chan[i].std.alg.will) chan[i].autoEnvDen=1;
    }
    if (chan[i].std.alg.had) {
      chan[i].autoEnvDen=chan[i].std.alg.val;
      chan[i].freqChanged=true;
      if (!chan[i].std.ex3.will) chan[i].autoEnvNum=1;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true)+chan[i].std.pitch.val;
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
      }
      if (chan[i].keyOff) {
        rWrite(0x08+i,0);
      }
      rWrite((i)<<1,chan[i].freq&0xff);
      rWrite(1+((i)<<1),chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
        ayEnvPeriod=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>4;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
      chan[i].freqChanged=false;
    }
  }

  updateOutSel();

  if (ayEnvSlide!=0) {
    ayEnvSlideLow+=ayEnvSlide;
    while (ayEnvSlideLow>7) {
      ayEnvSlideLow-=8;
      if (ayEnvPeriod<0xffff) {
        ayEnvPeriod++;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
    }
    while (ayEnvSlideLow<-7) {
      ayEnvSlideLow+=8;
      if (ayEnvPeriod>0) {
        ayEnvPeriod--;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
    }
  }
  
  for (int i=0; i<16; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }
}

int DivPlatformAY8910::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AY);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(ins);
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0);
      } else if (intellivision && (chan[c.chan].psgMode&4)) {
        rWrite(0x08+c.chan,(chan[c.chan].vol&0xc)<<2);
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].std.init(NULL);
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
        rWrite(0x08+c.chan,0);
      } else {
        if (chan[c.chan].active) {
          if (intellivision && (chan[c.chan].psgMode&4)) {
            rWrite(0x08+c.chan,(chan[c.chan].vol&0xc)<<2);
          } else {
            rWrite(0x08+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
          }
        }
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
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      if (c.value<16) {
        chan[c.chan].psgMode=(c.value+1)&7;
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0);
        } else if (chan[c.chan].active) {
          if (intellivision && (chan[c.chan].psgMode&4)) {
            rWrite(0x08+c.chan,(chan[c.chan].outVol&0xc)<<2);
          } else {
            rWrite(0x08+c.chan,(chan[c.chan].outVol&15)|((chan[c.chan].psgMode&4)<<2));
          }
        }
      }
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      rWrite(0x06,31-c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      ayEnvMode=c.value>>4;
      rWrite(0x0d,ayEnvMode);
      if (c.value&15) {
        chan[c.chan].psgMode|=4;
      } else {
        chan[c.chan].psgMode&=~4;
      }
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0);
      } else if (intellivision && (chan[c.chan].psgMode&4)) {
        rWrite(0x08+c.chan,(chan[c.chan].vol&0xc)<<2);
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      ayEnvPeriod&=0xff00;
      ayEnvPeriod|=c.value;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      ayEnvPeriod&=0xff;
      ayEnvPeriod|=c.value<<8;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      ayEnvSlide=c.value;
      break;
    case DIV_CMD_AY_AUTO_ENVELOPE:
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_AY_IO_WRITE:
      if (c.value==255) break;
      if (c.value) { // port B
        ioPortB=true;
        portBVal=c.value2;
        logI("AY I/O port B write: %x",portBVal);
      } else { // port A
        ioPortA=true;
        portAVal=c.value2;
        logI("AY I/O port A write: %x",portAVal);
      }
      updateOutSel(true);
      immWrite(14+(c.value?1:0),(c.value?portBVal:portAVal));
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_AY));
      }
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

void DivPlatformAY8910::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(0x08+ch,0);
  } else if (intellivision && (chan[ch].psgMode&4)) {
    rWrite(0x08+ch,(chan[ch].vol&0xc)<<2);
  } else {
    rWrite(0x08+ch,(chan[ch].outVol&15)|((chan[ch].psgMode&4)<<2));
  }
}

void DivPlatformAY8910::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
  }
  immWrite(0x0b,ayEnvPeriod);
  immWrite(0x0c,ayEnvPeriod>>8);
  immWrite(0x0d,ayEnvMode);
}

void* DivPlatformAY8910::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformAY8910::getRegisterPool() {
  return regPool;
}

int DivPlatformAY8910::getRegisterPoolSize() {
  return 16;
}

void DivPlatformAY8910::flushWrites() {
  while (!writes.empty()) writes.pop();
}

bool DivPlatformAY8910::getDCOffRequired() {
  return true;
}

void DivPlatformAY8910::reset() {
  while (!writes.empty()) writes.pop();
  ay->device_reset();
  memset(regPool,0,16);
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformAY8910::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x0f;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  for (int i=0; i<16; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  dacMode=0;
  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;
  ayEnvPeriod=0;
  ayEnvMode=0;
  ayEnvSlide=0;
  ayEnvSlideLow=0;

  delay=0;

  extMode=false;

  ioPortA=false;
  ioPortB=false;
  portAVal=0;
  portBVal=0;
}

bool DivPlatformAY8910::isStereo() {
  return true;
}

bool DivPlatformAY8910::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformAY8910::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAY8910::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformAY8910::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformAY8910::setFlags(unsigned int flags) {
  switch (flags&15) {
    case 1:
      chipClock=COLOR_PAL*2.0/5.0;
      break;
    case 2:
      chipClock=1750000;
      break;
    case 3:
      chipClock=2000000;
      break;
    case 4:
      chipClock=1500000;
      break;
    case 5:
      chipClock=1000000;
      break;
    case 6:
      chipClock=COLOR_NTSC/4.0;
      break;
    case 7:
      chipClock=COLOR_PAL*3.0/8.0;
      break;
    case 8:
      chipClock=COLOR_PAL*3.0/16.0;
      break;
    case 9:
      chipClock=COLOR_PAL/4.0;
      break;
    case 10:
      chipClock=2097152;
      break;
    default:
      chipClock=COLOR_NTSC/2.0;
      break;
  }
  rate=chipClock/8;

  if (ay!=NULL) delete ay;
  switch ((flags>>4)&3) {
    case 1:
      ay=new ym2149_device(rate);
      sunsoft=false;
      intellivision=false;
      break;
    case 2:
      ay=new sunsoft_5b_sound_device(rate);
      sunsoft=true;
      intellivision=false;
      break;
    case 3:
      ay=new ay8914_device(rate);
      sunsoft=false;
      intellivision=true;
      break;
    default:
      ay=new ay8910_device(rate);
      sunsoft=false;
      intellivision=false;
      break;
  }
  ay->device_start();

  stereo=flags>>6;
}

int DivPlatformAY8910::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
  }
  ay=NULL;
  setFlags(flags);
  ayBufLen=65536;
  for (int i=0; i<3; i++) ayBuf[i]=new short[ayBufLen];
  reset();
  return 3;
}

void DivPlatformAY8910::quit() {
  for (int i=0; i<3; i++) delete[] ayBuf[i];
  if (ay!=NULL) delete ay;
}
