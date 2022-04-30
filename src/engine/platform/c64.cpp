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

#include "c64.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {sid.write(a,v); regPool[(a)&0x1f]=v; if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 524288

const char* regCheatSheetSID[]={
  "FreqL0", "00",
  "FreqH0", "01",
  "PWL0", "02",
  "PWH0", "03",
  "Control0", "04",
  "AtkDcy0", "05",
  "StnRis0", "06",
  "FreqL1", "07",
  "FreqH1", "08",
  "PWL1", "09",
  "PWH1", "0A",
  "Control1", "0B",
  "AtkDcy1", "0C",
  "StnRis1", "0D",
  "FreqL2", "0E",
  "FreqH2", "0F",
  "PWL2", "10",
  "PWH2", "11",
  "Control2", "12",
  "AtkDcy2", "13",
  "StnRis2", "14",
  "FCL", "15",
  "FCH", "16",
  "FilterRes", "17",
  "FilterMode", "18",
  "PotX", "19",
  "PotY", "1A",
  "Osc3", "1B",
  "Env3", "1C",
  NULL
};

const char** DivPlatformC64::getRegisterSheet() {
  return regCheatSheetSID;
}

const char* DivPlatformC64::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Set waveform (bit 0: triangle; bit 1: saw; bit 2: pulse; bit 3: noise)";
      break;
    case 0x11:
      return "11xx: Set coarse cutoff (not recommended; use 4xxx instead)";
      break;
    case 0x12:
      return "12xx: Set coarse pulse width (not recommended; use 3xxx instead)";
      break;
    case 0x13:
      return "13xx: Set resonance (0 to F)";
      break;
    case 0x14:
      return "14xx: Set filter mode (bit 0: low pass; bit 1: band pass; bit 2: high pass)";
      break;
    case 0x15:
      return "15xx: Set envelope reset time";
      break;
    case 0x1a:
      return "1Axx: Disable envelope reset for this channel (1 disables; 0 enables)";
      break;
    case 0x1b:
      return "1Bxy: Reset cutoff (x: on new note; y: now)";
      break;
    case 0x1c:
      return "1Cxy: Reset pulse width (x: on new note; y: now)";
      break;
    case 0x1e:
      return "1Exy: Change additional parameters";
      break;
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x34: case 0x35: case 0x36: case 0x37:
    case 0x38: case 0x39: case 0x3a: case 0x3b:
    case 0x3c: case 0x3d: case 0x3e: case 0x3f:
      return "3xxx: Set pulse width (0 to FFF)";
      break;
    case 0x40: case 0x41: case 0x42: case 0x43:
    case 0x44: case 0x45: case 0x46: case 0x47:
      return "4xxx: Set cutoff (0 to 7FF)";
      break;
  }
  return NULL;
}

void DivPlatformC64::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    sid.clock();
    bufL[i]=sid.output();
    if (++writeOscBuf>=8) {
      writeOscBuf=0;
      oscBuf[0]->data[oscBuf[0]->needle++]=sid.last_chan_out[0];
      oscBuf[1]->data[oscBuf[1]->needle++]=sid.last_chan_out[1];
      oscBuf[2]->data[oscBuf[2]->needle++]=sid.last_chan_out[2];
    }
  }
}

void DivPlatformC64::updateFilter() {
  rWrite(0x15,filtCut&7);
  rWrite(0x16,filtCut>>3);
  rWrite(0x17,(filtRes<<4)|(chan[2].filter<<2)|(chan[1].filter<<1)|(int)(chan[0].filter));
  rWrite(0x18,(filtControl<<4)|vol);
}

void DivPlatformC64::tick(bool sysTick) {
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_C64);
      if (ins->c64.volIsCutoff) {
        if (ins->c64.filterIsAbs) {
          filtCut=MIN(2047,chan[i].std.vol.val);
        } else {
          filtCut-=((signed char)chan[i].std.vol.val)*7;
          if (filtCut>2047) filtCut=2047;
          if (filtCut<0) filtCut=0;
        }
        updateFilter();
      } else {
        vol=MIN(15,chan[i].std.vol.val);
        updateFilter();
      }
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note+(signed char)chan[i].std.arp.val);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.duty.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_C64);
      if (ins->c64.dutyIsAbs) {
        chan[i].duty=chan[i].std.duty.val;
      } else {
        chan[i].duty-=((signed char)chan[i].std.duty.val)*4;
      }
      rWrite(i*7+2,chan[i].duty&0xff);
      rWrite(i*7+3,chan[i].duty>>8);
    }
    if (sysTick) {
      if (chan[i].testWhen>0) {
        if (--chan[i].testWhen<1) {
          if (!chan[i].resetMask && !chan[i].inPorta) {
            DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_C64);
            rWrite(i*7+5,0);
            rWrite(i*7+6,0);
            rWrite(i*7+4,(chan[i].wave<<4)|(ins->c64.noTest?0:8)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1));
          }
        }
      }
    }
    if (chan[i].std.wave.had) {
      chan[i].wave=chan[i].std.wave.val;
      rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active));
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-2048,2048);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.ex1.had) {
      filtControl=chan[i].std.ex1.val&15;
      updateFilter();
    }
    if (chan[i].std.ex2.had) {
      filtRes=chan[i].std.ex2.val&15;
      updateFilter();
    }
    if (chan[i].std.ex3.had) {
      chan[i].sync=chan[i].std.ex3.val&1;
      chan[i].ring=chan[i].std.ex3.val&2;
      chan[i].freqChanged=true;
      rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active));
    }
    if (chan[i].std.ex4.had) {
      chan[i].test=chan[i].std.ex4.val&1;
      rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active));
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,8,chan[i].pitch2);
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;
      if (chan[i].keyOn) {
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|1);
      }
      if (chan[i].keyOff) {
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|0);
      }
      rWrite(i*7,chan[i].freq&0xff);
      rWrite(i*7+1,chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformC64::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_C64);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].test=false;
      if (chan[c.chan].insChanged || chan[c.chan].resetDuty || ins->std.waveMacro.len>0) {
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].wave=(ins->c64.noiseOn<<3)|(ins->c64.pulseOn<<2)|(ins->c64.sawOn<<1)|(int)(ins->c64.triOn);
        chan[c.chan].attack=ins->c64.a;
        chan[c.chan].decay=(ins->c64.s==15)?0:ins->c64.d;
        chan[c.chan].sustain=ins->c64.s;
        chan[c.chan].release=ins->c64.r;
        chan[c.chan].ring=ins->c64.ringMod;
        chan[c.chan].sync=ins->c64.oscSync;
      }
      if (chan[c.chan].insChanged || chan[c.chan].resetFilter) {
        chan[c.chan].filter=ins->c64.toFilter;
        if (ins->c64.initFilter) {
          filtCut=ins->c64.cut;
          filtRes=ins->c64.res;
          filtControl=(int)(ins->c64.lp)|(ins->c64.bp<<1)|(ins->c64.hp<<2)|(ins->c64.ch3off<<3);
        }
        updateFilter();
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].macroInit(ins);
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
          vol=chan[c.chan].outVol;
        } else {
          vol=chan[c.chan].vol;
        }
        updateFilter();
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
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=(c.value*4095)/100;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      break;
    case DIV_CMD_C64_FINE_DUTY:
      chan[c.chan].duty=c.value;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      break;
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test<<3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active));
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta || !chan[c.chan].inPorta) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_C64));
          chan[c.chan].keyOn=true;
        }
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      if (resetTime) chan[c.chan].testWhen=c.value-resetTime+1;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_C64_CUTOFF:
      if (c.value>100) c.value=100;
      filtCut=c.value*2047/100;
      updateFilter();
      break;
    case DIV_CMD_C64_FINE_CUTOFF:
      filtCut=c.value;
      updateFilter();
      break;
    case DIV_CMD_C64_RESONANCE:
      if (c.value>15) c.value=15;
      filtRes=c.value;
      updateFilter();
      break;
    case DIV_CMD_C64_FILTER_MODE:
      filtControl=c.value&7;
      updateFilter();
      break;
    case DIV_CMD_C64_RESET_TIME:
      resetTime=c.value;
      break;
    case DIV_CMD_C64_RESET_MASK:
      chan[c.chan].resetMask=c.value;
      break;
    case DIV_CMD_C64_FILTER_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_C64);
        if (ins->c64.initFilter) {
          filtCut=ins->c64.cut;
          updateFilter();
        }
      }
      chan[c.chan].resetFilter=c.value>>4;
      break;
    case DIV_CMD_C64_DUTY_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_C64);
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      }
      chan[c.chan].resetDuty=c.value>>4;
      break;
    case DIV_CMD_C64_EXTENDED:
      switch (c.value>>4) {
        case 0:
          chan[c.chan].attack=c.value&15;
          break;
        case 1:
          chan[c.chan].decay=c.value&15;
          break;
        case 2:
          chan[c.chan].sustain=c.value&15;
          break;
        case 3:
          chan[c.chan].release=c.value&15;
          break;
        case 4:
          chan[c.chan].ring=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test<<3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active));
          break;
        case 5:
          chan[c.chan].sync=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test<<3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active));
          break;
        case 6:
          filtControl&=7;
          filtControl|=(!!c.value)<<3;
          break;
      }
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformC64::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  sid.set_is_muted(ch,mute);
}

void DivPlatformC64::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].testWhen=0;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  updateFilter();
}

void DivPlatformC64::notifyInsChange(int ins) {
  for (int i=0; i<3; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformC64::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformC64::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformC64::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformC64::getRegisterPool() {
  return regPool;
}

int DivPlatformC64::getRegisterPoolSize() {
  return 32;
}

bool DivPlatformC64::getDCOffRequired() {
  return true;
}

void DivPlatformC64::reset() {
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformC64::Channel();
    chan[i].std.setEngine(parent);
  }

  sid.reset();
  memset(regPool,0,32);

  rWrite(0x18,0x0f);

  filtControl=7;
  filtRes=0;
  filtCut=2047;
  resetTime=1;
  vol=15;
}

void DivPlatformC64::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformC64::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformC64::setChipModel(bool is6581) {
  if (is6581) {
    sid.set_chip_model(MOS6581);
  } else {
    sid.set_chip_model(MOS8580);
  }
}

void DivPlatformC64::setFlags(unsigned int flags) {
  switch (flags&0xf) {
    case 0x0: // NTSC C64
      rate=COLOR_NTSC*2.0/7.0;
      break;
    case 0x1: // PAL C64
      rate=COLOR_PAL*2.0/9.0;
      break;
    case 0x2: // SSI 2001
    default:
      rate=14318180.0/16.0;
      break;
  }
  chipClock=rate;
  for (int i=0; i<3; i++) {
    oscBuf[i]->rate=rate/16;
  }
}

int DivPlatformC64::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  writeOscBuf=0;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  reset();

  return 3;
}

void DivPlatformC64::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
}

DivPlatformC64::~DivPlatformC64() {
}
