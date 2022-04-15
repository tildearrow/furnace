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

#include "ay8930.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "sound/ay8910.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 8

const char* regCheatSheetAY8930[]={
  "FreqL_A", "00",
  "FreqH_A", "01",
  "FreqL_B", "02",
  "FreqH_B", "03",
  "FreqL_C", "04",
  "FreqH_C", "05",
  "FreqNoise", "06",
  "Enable", "07",
  "Volume_A", "08",
  "Volume_B", "09",
  "Volume_C", "0A",
  "FreqL_EnvA", "0B",
  "FreqH_EnvA", "0C",
  "Control_EnvA", "0D",
  "PortA", "0E",
  "PortB", "0F",
  "FreqL_EnvB", "10",
  "FreqH_EnvB", "11",
  "FreqL_EnvC", "12",
  "FreqH_EnvC", "13",
  "Control_EnvB", "14",
  "Control_EnvC", "15",
  "Duty_A", "16",
  "Duty_B", "17",
  "Duty_C", "18",
  "NoiseAND", "19",
  "NoiseOR", "1A",
  "TEST", "1F",
  NULL
};

const char** DivPlatformAY8930::getRegisterSheet() {
  return regCheatSheetAY8930;
}

const char* DivPlatformAY8930::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x12:
      return "12xx: Set duty cycle (0 to 8)";
      break;
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
    case 0x27:
      return "27xx: Set noise AND mask";
      break;
    case 0x28:
      return "28xx: Set noise OR mask";
      break;
    case 0x29:
      return "29xy: Set auto-envelope (x: numerator; y: denominator)";
      break;
    case 0x2d:
      return "2Dxx: NOT TO BE EMPLOYED BY THE COMPOSER";
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

void DivPlatformAY8930::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (ayBufLen<len) {
    ayBufLen=len;
    for (int i=0; i<3; i++) {
      delete[] ayBuf[i];
      ayBuf[i]=new short[ayBufLen];
    }
  }
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    if ((int)bank!=(w.addr>>4)) {
      bank=w.addr>>4;
      ay->address_w(0x0d);
      ay->data_w(0xa0|(bank<<4)|ayEnvMode[0]);
    }
    ay->address_w(w.addr&15);
    if (w.addr==0x0d) {
      ay->data_w(0xa0|(bank<<4)|(w.val&15));
    } else {
      ay->data_w(w.val);
    }
    regPool[w.addr&0x1f]=w.val;
    writes.pop();
  }
  ay->sound_stream_update(ayBuf,len);
  if (stereo) {
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

void DivPlatformAY8930::updateOutSel(bool immediate) {
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

const unsigned char regPeriodL[3]={
  0x0b, 0x10, 0x12
};

const unsigned char regPeriodH[3]={
  0x0c, 0x11, 0x13
};

const unsigned char regMode[3]={
  0x0d, 0x14, 0x15
};

void DivPlatformAY8930::tick() {
  // PSG
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MIN(31,chan[i].std.vol.val)-(31-(chan[i].vol&31));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(0x08+i,0);
      } else {
        rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].psgMode&4)<<3));
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
      rWrite(0x06,chan[i].std.duty.val);
    }
    if (chan[i].std.wave.had) {
      chan[i].psgMode=(chan[i].std.wave.val+1)&7;
      if (isMuted[i]) {
        rWrite(0x08+i,0);
      } else {
        rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].psgMode&4)<<3));
      }
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        oldWrites[0x08+i]=-1;
        oldWrites[regMode[i]]=-1;
      }
    }
    if (chan[i].std.ex1.had) { // duty
      rWrite(0x16+i,chan[i].std.ex1.val);
    }
    if (chan[i].std.ex2.had) {
      ayEnvMode[i]=chan[i].std.ex2.val;
      rWrite(regMode[i],ayEnvMode[i]);
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
    if (chan[i].std.fb.had) {
      ayNoiseAnd=chan[i].std.fb.val;
      immWrite(0x19,ayNoiseAnd);
    }
    if (chan[i].std.fms.had) {
      ayNoiseOr=chan[i].std.fms.val;
      immWrite(0x1a,ayNoiseOr);
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true)+chan[i].std.pitch.val;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].keyOn) {
        if (chan[i].insChanged) {
          if (!chan[i].std.ex1.will) immWrite(0x16+i,chan[i].duty);
          chan[i].insChanged=false;
        }
      }
      if (chan[i].keyOff) {
        rWrite(0x08+i,0);
      }
      rWrite((i)<<1,chan[i].freq&0xff);
      rWrite(1+((i)<<1),chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
        ayEnvPeriod[i]=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>4;
        immWrite(regPeriodL[i],ayEnvPeriod[i]);
        immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
      }
      chan[i].freqChanged=false;
    }

    if (ayEnvSlide[i]!=0) {
      ayEnvSlideLow[i]+=ayEnvSlide[i];
      while (ayEnvSlideLow[i]>7) {
        ayEnvSlideLow[i]-=8;
        if (ayEnvPeriod[i]<0xffff) {
          ayEnvPeriod[i]++;
          immWrite(regPeriodL[i],ayEnvPeriod[i]);
          immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
        }
      }
      while (ayEnvSlideLow[i]<-7) {
        ayEnvSlideLow[i]+=8;
        if (ayEnvPeriod[i]>0) {
          ayEnvPeriod[i]--;
          immWrite(regPeriodL[i],ayEnvPeriod[i]);
          immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
        }
      }
    }
  }

  updateOutSel();
  
  for (int i=0; i<32; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }
}

int DivPlatformAY8930::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
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
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].psgMode&4)<<3));
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
        if (chan[c.chan].active) rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].psgMode&4)<<3));
      }
      break;
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
      if (c.value<0x10) {
        chan[c.chan].psgMode=(c.value+1)&7;
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0);
        } else if (chan[c.chan].active) {
          rWrite(0x08+c.chan,(chan[c.chan].outVol&31)|((chan[c.chan].psgMode&4)<<3));
        }
      } else {
        chan[c.chan].duty=c.value&15;
        immWrite(0x16+c.chan,chan[c.chan].duty);
      }
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      rWrite(0x06,c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      ayEnvMode[c.chan]=c.value>>4;
      rWrite(regMode[c.chan],ayEnvMode[c.chan]);
      if (c.value&15) {
        chan[c.chan].psgMode|=4;
      } else {
        chan[c.chan].psgMode&=~4;
      }
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0);
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].psgMode&4)<<3));
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      ayEnvPeriod[c.chan]&=0xff00;
      ayEnvPeriod[c.chan]|=c.value;
      immWrite(regPeriodL[c.chan],ayEnvPeriod[c.chan]);
      immWrite(regPeriodH[c.chan],ayEnvPeriod[c.chan]>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      ayEnvPeriod[c.chan]&=0xff;
      ayEnvPeriod[c.chan]|=c.value<<8;
      immWrite(regPeriodL[c.chan],ayEnvPeriod[c.chan]);
      immWrite(regPeriodH[c.chan],ayEnvPeriod[c.chan]>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      ayEnvSlide[c.chan]=c.value;
      break;
    case DIV_CMD_AY_NOISE_MASK_AND:
      ayNoiseAnd=c.value;
      immWrite(0x19,ayNoiseAnd);
      break;
    case DIV_CMD_AY_NOISE_MASK_OR:
      ayNoiseOr=c.value;
      immWrite(0x1a,ayNoiseOr);
      break;
    case DIV_CMD_AY_AUTO_ENVELOPE:
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_AY_IO_WRITE:
      if (c.value==255) {
        immWrite(0x1f,c.value2);
        break;
      }
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
      return 31;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
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

void DivPlatformAY8930::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(0x08+ch,0);
  } else {
    rWrite(0x08+ch,(chan[ch].outVol&31)|((chan[ch].psgMode&4)<<3));
  }
}

void DivPlatformAY8930::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    immWrite(regPeriodL[i],ayEnvPeriod[i]);
    immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
    immWrite(regMode[i],ayEnvMode[i]);
  }
}

void* DivPlatformAY8930::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformAY8930::getRegisterPool() {
  return regPool;
}

int DivPlatformAY8930::getRegisterPoolSize() {
  return 32;
}

void DivPlatformAY8930::reset() {
  while (!writes.empty()) writes.pop();
  ay->device_reset();
  memset(regPool,0,32);
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformAY8930::Channel();
    chan[i].vol=31;
    ayEnvPeriod[i]=0;
    ayEnvMode[i]=0;
    ayEnvSlide[i]=0;
    ayEnvSlideLow[i]=0;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  for (int i=0; i<32; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  ayNoiseAnd=2;
  ayNoiseOr=0;
  delay=0;

  extMode=false;
  bank=false;

  ioPortA=false;
  ioPortB=false;
  portAVal=0;
  portBVal=0;

  immWrite(0x0d,0xa0);
  immWrite(0x19,2); // and mask
  immWrite(0x1a,0x00); // or mask
}

bool DivPlatformAY8930::isStereo() {
  return true;
}

bool DivPlatformAY8930::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformAY8930::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAY8930::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformAY8930::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformAY8930::setFlags(unsigned int flags) {
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
  stereo=flags>>6;
}

int DivPlatformAY8930::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  ay=new ay8930_device(rate);
  ay->device_start();
  ayBufLen=65536;
  for (int i=0; i<3; i++) ayBuf[i]=new short[ayBufLen];
  reset();
  return 3;
}

void DivPlatformAY8930::quit() {
  for (int i=0; i<3; i++) delete[] ayBuf[i];
  delete ay;
}
