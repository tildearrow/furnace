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

#include "lynx.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) {if (!skipRegisterWrites) {mikey->write(a,v); if (dumpWrites) {addWrite(a,v);}}}

#define WRITE_VOLUME(ch,v) rWrite(0x20+(ch<<3),(v))
#define WRITE_FEEDBACK(ch,v) rWrite(0x21+(ch<<3),(v))
#define WRITE_LFSR(ch,v) rWrite(0x23+(ch<<3),(v))
#define WRITE_BACKUP(ch,v) rWrite(0x24+(ch<<3),(v))
#define WRITE_CONTROL(ch,v) rWrite(0x25+(ch<<3),(v))
#define WRITE_OTHER(ch,v) rWrite(0x27+(ch<<3),(v))
#define WRITE_ATTEN(ch,v) rWrite((0x40+ch),(v))
#define WRITE_STEREO(v) rWrite(0x50,(v))

#define CHIP_DIVIDER 64

#if defined( _MSC_VER )

#include <intrin.h>

static int bsr(uint16_t v) {
  unsigned long idx;
  if (_BitScanReverse(&idx,(unsigned long)v)) {
    return idx;
  }
  else {
    return -1;
  }
}

#elif defined( __GNUC__ )

static int bsr(uint16_t v)
{
  if (v) {
    return 32 - __builtin_clz(v);
  }
  else{
    return -1;
  }
}

#else

static int bsr(uint16_t v)
{
  uint16_t mask = 0x8000;
  for (int i = 15; i >= 0; --i) {
    if (v&mask)
      return (int)i;
    mask>>=1;
  }

  return -1;
}

#endif

static int32_t clamp(int32_t v, int32_t lo, int32_t hi)
{
  return v<lo?lo:(v>hi?hi:v);
}

const char* regCheatSheetLynx[]={
  "AUDIO0_VOLCNTRL", "20",
  "AUDIO0_FEEDBACK", "21",
  "AUDIO0_OUTPUT", "22",
  "AUDIO0_SHIFT", "23",
  "AUDIO0_BACKUP", "24",
  "AUDIO0_CONTROL", "25",
  "AUDIO0_COUNTER", "26",
  "AUDIO0_OTHER", "27",
  "AUDIO1_VOLCNTRL", "28",
  "AUDIO1_FEEDBACK", "29",
  "AUDIO1_OUTPUT", "2a",
  "AUDIO1_SHIFT", "2b",
  "AUDIO1_BACKUP", "2c",
  "AUDIO1_CONTROL", "2d",
  "AUDIO1_COUNTER", "2e",
  "AUDIO1_OTHER", "2f",
  "AUDIO2_VOLCNTRL", "30",
  "AUDIO2_FEEDBACK", "31",
  "AUDIO2_OUTPUT", "32",
  "AUDIO2_SHIFT", "33",
  "AUDIO2_BACKUP", "34",
  "AUDIO2_CONTROL", "35",
  "AUDIO2_COUNTER", "36",
  "AUDIO2_OTHER", "37",
  "AUDIO3_VOLCNTRL", "38",
  "AUDIO3_FEEDBACK", "39",
  "AUDIO3_OUTPUT", "3a",
  "AUDIO3_SHIFT", "3b",
  "AUDIO3_BACKUP", "3c",
  "AUDIO3_CONTROL", "3d",
  "AUDIO3_COUNTER", "3e",
  "AUDIO3_OTHER", "3f",
  "ATTENREG0", "40",
  "ATTENREG1", "41",
  "ATTENREG2", "42",
  "ATTENREG3", "43",
  "MPAN", "44",
  "MSTEREO", "50",
  NULL
};


const char** DivPlatformLynx::getRegisterSheet() {
  return regCheatSheetLynx;
}

const char* DivPlatformLynx::getEffectName(unsigned char effect) {
  switch (effect)
  {
  case 0x30: case 0x31: case 0x32: case 0x33:
  case 0x34: case 0x35: case 0x36: case 0x37:
  case 0x38: case 0x39: case 0x3a: case 0x3b:
  case 0x3c: case 0x3d: case 0x3e: case 0x3f:
    return "3xxx: Load LFSR (0 to FFF)";
    break;
  }
  return NULL;
}

void DivPlatformLynx::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  mikey->sampleAudio( bufL + start, bufR + start, len );
}

void DivPlatformLynx::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&127)*MIN(127,chan[i].std.vol.val))>>7;
      WRITE_VOLUME(i,(isMuted[i]?0:(chan[i].outVol&127)));
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp.val);
          chan[i].actualNote=chan[i].std.arp.val;
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp.val);
          chan[i].actualNote=chan[i].note+chan[i].std.arp.val;
        }
        chan[i].freqChanged=true;
      }
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        chan[i].actualNote=chan[i].note;
        chan[i].freqChanged=true;
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
      WRITE_ATTEN(i,chan[i].pan);
    }

    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }

    if (chan[i].freqChanged) {
      if (chan[i].lfsr >= 0) {
        WRITE_LFSR(i, (chan[i].lfsr&0xff));
        WRITE_OTHER(i, ((chan[i].lfsr&0xf00)>>4));
        chan[i].lfsr=-1;
      }
      chan[i].fd=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true)+chan[i].std.pitch.val;
      if (chan[i].std.duty.had) {
        chan[i].duty=chan[i].std.duty.val;
        WRITE_FEEDBACK(i, chan[i].duty.feedback);
      }
      WRITE_CONTROL(i, (chan[i].fd.clockDivider|0x18|chan[i].duty.int_feedback7));
      WRITE_BACKUP( i, chan[i].fd.backup );
      chan[i].freqChanged=false;
    } else if (chan[i].std.duty.had) {
      chan[i].duty = chan[i].std.duty.val;
      WRITE_FEEDBACK(i, chan[i].duty.feedback);
      WRITE_CONTROL(i, (chan[i].fd.clockDivider|0x18|chan[i].duty.int_feedback7));
    }
  }
}

int DivPlatformLynx::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].actualNote=c.value;
        if (chan[c.chan].lfsr<0)
          chan[c.chan].lfsr=0;
      }
      chan[c.chan].active=true;
      WRITE_VOLUME(c.chan,(isMuted[c.chan]?0:(chan[c.chan].vol&127)));
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_MIKEY));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      WRITE_VOLUME(c.chan, 0);
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_LYNX_LFSR_LOAD:
      chan[c.chan].freqChanged=true;
      chan[c.chan].lfsr=c.value;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=c.value;
      //chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_MIKEY));
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) WRITE_VOLUME(c.chan,(isMuted[c.chan]?0:(chan[c.chan].vol&127)));
      }
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].pan=c.value;
      WRITE_ATTEN(c.chan,chan[c.chan].pan);
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
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].actualNote=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_MIKEY));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformLynx::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (chan[ch].active) WRITE_VOLUME(ch,(isMuted[ch]?0:(chan[ch].outVol&127)));
}

bool DivPlatformLynx::isStereo() {
  return true;
}

void DivPlatformLynx::forceIns() {
  for (int i=0; i<4; i++) {
    if (chan[i].active) {
      chan[i].insChanged=true;
      chan[i].freqChanged=true;
    }
    WRITE_ATTEN(i,chan[i].pan);
  }
}

void* DivPlatformLynx::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformLynx::getRegisterPool()
{
  return const_cast<unsigned char*>( mikey->getRegisterPool() );
}

int DivPlatformLynx::getRegisterPoolSize()
{
  return 4*8+4;
}

void DivPlatformLynx::reset() {
  mikey=std::make_unique<Lynx::Mikey>(rate);

  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformLynx::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  WRITE_STEREO(0);
}

bool DivPlatformLynx::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformLynx::keyOffAffectsPorta(int ch) {
  return true;
}

//int DivPlatformLynx::getPortaFloor(int ch) {
//  return 12;
//}

void DivPlatformLynx::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformLynx::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformLynx::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr, i.val);
}

int DivPlatformLynx::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<4; i++) {
    isMuted[i]=false;
  }

  chipClock = 16000000;
  rate = chipClock/128;

  reset();
  return 4;
}

void DivPlatformLynx::quit() {
  mikey.reset();
}

DivPlatformLynx::~DivPlatformLynx() {
}

DivPlatformLynx::MikeyFreqDiv::MikeyFreqDiv(int frequency) {

  int clamped=clamp(frequency, 36, 16383);

  auto top=bsr(clamped);
  
  if (top>7)
  {
    clockDivider=top-7;
    backup=frequency>>(top-7);
  }
  else
  {
    clockDivider=0;
    backup=frequency;
  }
}

DivPlatformLynx::MikeyDuty::MikeyDuty(int duty) {

  //duty:
  //9: int
  //8: f11
  //7: f10
  //6: f7
  //5: f5
  //4: f4
  //3: f3
  //2: f2
  //1: f1
  //0: f0

  //f7 moved to bit 7 and int moved to bit 5
  int_feedback7=((duty&0x40)<<1)|((duty&0x200)>>4);
  //f11 and f10 moved to bits 7 & 6
  feedback=(duty&0x3f)|((duty&0x180)>>1);
}
