/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "ted.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 8

const char* regCheatSheetTED[]={
  "Freq0L", "0e",
  "Freq1L", "0f",
  "Freq1H", "10",
  "Control", "11",
  "Freq0H", "12",
  NULL
};

const char** DivPlatformTED::getRegisterSheet() {
  return regCheatSheetTED;
}

void DivPlatformTED::acquire(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      ted_sound_machine_store(&ted,w.addr,w.val);
      regPool[(w.addr-0x0e)&7]=w.val;
      writes.pop();
    }

    ted_sound_machine_calculate_samples(&ted,&buf[0][h],1,1);
    oscBuf[0]->data[oscBuf[0]->needle++]=(ted.voice0_output_enabled && ted.voice0_sign)?(ted.volume<<1):0;
    oscBuf[1]->data[oscBuf[1]->needle++]=(ted.voice1_output_enabled && ((ted.noise && (!(ted.noise_shift_register&1))) || (!ted.noise && ted.voice1_sign)))?(ted.volume<<1):0;
  }
}

void DivPlatformTED::tick(bool sysTick) {
  bool resetPhase=false;

  for (int _i=0; _i<2; _i++) {
    int i=chanOrder[_i];

    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol,MIN(8,chan[i].std.vol.val),8);
      updateCtrl=true;
      vol=chan[i].outVol;
    }
    if (chan[i].std.duty.had) {
      chan[i].noise=chan[i].std.duty.val&2;
      chan[i].square=chan[i].std.duty.val&1;
      chan[i].freqChanged=true;
      updateCtrl=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
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
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
      if (i==1 && chan[i].noise && !chan[i].square) chan[i].freq>>=4;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>1023) chan[i].freq=1023;

      if (i==1) {
        rWrite(0x0f,(1022-chan[i].freq)&0xff);
        rWrite(0x10,((1022-chan[i].freq)>>8)&0xff);
      } else {
        rWrite(0x0e,(1022-chan[i].freq)&0xff);
        rWrite(0x12,((1022-chan[i].freq)>>8)&0xff);
      }

      if (chan[i].keyOn) {
        updateCtrl=true;
      }
      if (chan[i].keyOff) {
        updateCtrl=true;
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      resetPhase=true;
      updateCtrl=true;
    }
  }

  if (resetPhase) {
    rWrite(0x11,0x80);
  }

  if (updateCtrl) {
    updateCtrl=false;
    rWrite(0x11,(vol&15)|((chan[0].active && chan[0].square && !isMuted[0])?0x10:0)|((chan[1].active && chan[1].square && !isMuted[1])?0x20:0)|((chan[1].active && chan[1].noise && !isMuted[1])?0x40:0));
  }
}

int DivPlatformTED::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_TED);
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
      chan[c.chan].insChanged=false;
      if (keyPriority) {
        if (chanOrder[0]==c.chan) {
          chanOrder[0]=chanOrder[1];
          chanOrder[1]=c.chan;
        }
      }
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
        vol=chan[c.chan].outVol;
        updateCtrl=true;
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
      chan[c.chan].noise=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_TED));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 8;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformTED::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  updateCtrl=true;
}

void DivPlatformTED::forceIns() {
  for (int i=0; i<2; i++) {
    chan[i].freqChanged=true;
  }
  updateCtrl=true;
}

bool DivPlatformTED::isVolGlobal() {
  return true;
}

void* DivPlatformTED::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformTED::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformTED::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformTED::getRegisterPool() {
  return regPool;
}

int DivPlatformTED::getRegisterPoolSize() {
  return 5;
}

void DivPlatformTED::reset() {
  writes.clear();
  memset(regPool,0,8);
  for (int i=0; i<2; i++) {
    chan[i]=DivPlatformTED::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  ted_sound_machine_init(&ted,1,8);
  updateCtrl=true;
  vol=15;

  chanOrder[0]=0;
  chanOrder[1]=1;
}

int DivPlatformTED::getOutputCount() {
  return 1;
}

bool DivPlatformTED::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformTED::notifyInsDeletion(void* ins) {
  for (int i=0; i<2; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformTED::setFlags(const DivConfig& flags) {
  if (flags.getInt("clockSel",0)) {
    chipClock=COLOR_PAL*2.0/5.0;
  } else {
    chipClock=COLOR_NTSC/2.0;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/8;
  for (int i=0; i<2; i++) {
    oscBuf[i]->rate=rate;
  }
  keyPriority=flags.getBool("keyPriority",true);
}

void DivPlatformTED::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformTED::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformTED::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<2; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 2;
}

void DivPlatformTED::quit() {
  for (int i=0; i<2; i++) {
    delete oscBuf[i];
  }
}

DivPlatformTED::~DivPlatformTED() {
}
