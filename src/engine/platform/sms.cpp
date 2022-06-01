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

#include "sms.h"
#include "../engine.h"
#include <math.h>

#define rWrite(v) {if (!skipRegisterWrites) {writes.push(v); if (dumpWrites) {addWrite(0x200,v);}}}

const char* regCheatSheetSN[]={
  "DATA", "0",
  NULL
};

const char** DivPlatformSMS::getRegisterSheet() {
  return regCheatSheetSN;
}

const char* DivPlatformSMS::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x20:
      return "20xy: Set noise mode (x: preset freq/ch3 freq; y: thin pulse/noise)";
      break; 
  }
  return NULL;
}

void DivPlatformSMS::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  int o=0;
  for (size_t h=start; h<start+len; h++) {
    if (!writes.empty()) {
      unsigned char w=writes.front();
      YMPSG_Write(&sn_nuked,w);
      writes.pop();
    }
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    YMPSG_Clock(&sn_nuked);
    o=YMPSG_GetOutput(&sn_nuked);
    if (o<-32768) o=-32768;
    if (o>32767) o=32767;
    bufL[h]=o;
    /*
    for (int i=0; i<4; i++) {
      if (isMuted[i]) {
        oscBuf[i]->data[oscBuf[i]->needle++]=0;
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=sn->get_channel_output(i);
      }
    }*/
  }
}

void DivPlatformSMS::acquire_mame(short* bufL, short* bufR, size_t start, size_t len) {
  while (!writes.empty()) {
    unsigned char w=writes.front();
    sn->write(w);
    writes.pop();
  }
  for (size_t h=start; h<start+len; h++) {
    sn->sound_stream_update(bufL+h,1);
    for (int i=0; i<4; i++) {
      if (isMuted[i]) {
        oscBuf[i]->data[oscBuf[i]->needle++]=0;
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=sn->get_channel_output(i);
      }
    }
  }
}

void DivPlatformSMS::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (nuked) {
    acquire_nuked(bufL,bufR,start,len);
  } else {
    acquire_mame(bufL,bufR,start,len);
  }
}

int DivPlatformSMS::acquireOne() {
  short v;
  sn->sound_stream_update(&v,1);
  return v;
}

void DivPlatformSMS::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    int CHIP_DIVIDER=64;
    if (i==3 && isRealSN) CHIP_DIVIDER=60;
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MIN(15,chan[i].std.vol.val)-(15-(chan[i].vol&15));
      if (chan[i].outVol<0) chan[i].outVol=0;
      // old formula
      // ((chan[i].vol&15)*MIN(15,chan[i].std.vol.val))>>4;
      rWrite(0x90|(i<<5)|(isMuted[i]?15:(15-(chan[i].outVol&15))));
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp.val);
          chan[i].actualNote=chan[i].std.arp.val;
        } else {
          // TODO: check whether this weird octave boundary thing applies to other systems as well
          int areYouSerious=chan[i].note+chan[i].std.arp.val;
          while (areYouSerious>0x60) areYouSerious-=12;
          chan[i].baseFreq=NOTE_PERIODIC(areYouSerious);
          chan[i].actualNote=areYouSerious;
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
    if (i==3) {
      if (chan[i].std.duty.had) {
        if (chan[i].std.duty.val!=snNoiseMode || parent->song.snDutyReset) {
          snNoiseMode=chan[i].std.duty.val;
          if (chan[i].std.duty.val<2) {
            chan[3].freqChanged=false;
          }
          updateSNMode=true;
        }
      }
      if (chan[i].std.phaseReset.had) {
        if (chan[i].std.phaseReset.val==1) {
          updateSNMode=true;
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
  }
  for (int i=0; i<3; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true,0,chan[i].pitch2,chipClock,64);
      if (chan[i].freq>1023) chan[i].freq=1023;
      if (chan[i].freq<8) chan[i].freq=1;
      //if (chan[i].actualNote>0x5d) chan[i].freq=0x01;
      rWrite(0x80|i<<5|(chan[i].freq&15));
      rWrite(chan[i].freq>>4);
      // what?
      /*if (i==2 && snNoiseMode&2) {
        chan[3].baseFreq=chan[2].baseFreq;
        chan[3].actualNote=chan[2].actualNote;
      }*/
      chan[i].freqChanged=false;
    }
  }
  if (chan[3].freqChanged || updateSNMode) {
    chan[3].freq=parent->calcFreq(chan[3].baseFreq,chan[3].pitch,true,0,chan[3].pitch2,chipClock,isRealSN?60:64);
    if (chan[3].freq>1023) chan[3].freq=1023;
    if (chan[3].actualNote>0x5d) chan[3].freq=0x01;
    if (snNoiseMode&2) { // take period from channel 3
      if (updateSNMode || resetPhase) {
        if (snNoiseMode&1) {
          rWrite(0xe7);
        } else {
          rWrite(0xe3);
        }
        if (updateSNMode) {
          rWrite(0xdf);
        }
      }
      
      if (chan[3].freqChanged) {
        rWrite(0xc0|(chan[3].freq&15));
        rWrite(chan[3].freq>>4);
      }
    } else { // 3 fixed values
      unsigned char value;
      if (chan[3].std.arp.had) {
        if (chan[3].std.arp.mode) {
          value=chan[3].std.arp.val%12;
        } else {
          value=(chan[3].note+chan[3].std.arp.val)%12;
        }
      } else {
        value=chan[3].note%12;
      }
      if (value<3) {
        value=2-value;
        if (value!=oldValue || updateSNMode || resetPhase) {
          oldValue=value;
          rWrite(0xe0|value|((snNoiseMode&1)<<2));
        }
      }
    }
    chan[3].freqChanged=false;
    updateSNMode=false;
  }
}

int DivPlatformSMS::dispatch(DivCommand c) {
  int CHIP_DIVIDER=64;
  if (c.chan==3 && isRealSN) CHIP_DIVIDER=60;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].actualNote=c.value;
      }
      chan[c.chan].active=true;
      rWrite(0x90|c.chan<<5|(isMuted[c.chan]?15:(15-(chan[c.chan].vol&15))));
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      rWrite(0x9f|c.chan<<5);
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=c.value;
      //chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) rWrite(0x90|c.chan<<5|(isMuted[c.chan]?15:(15-(chan[c.chan].vol&15))));
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
      snNoiseMode=(c.value&1)|((c.value&16)>>3);
      updateSNMode=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].actualNote=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_STD));
      }
      chan[c.chan].inPorta=c.value;
      // TODO: pre porta cancel arp compat flag
      //if (chan[c.chan].inPorta) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSMS::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (chan[ch].active) rWrite(0x90|ch<<5|(isMuted[ch]?15:(15-(chan[ch].outVol&15))));
}

void DivPlatformSMS::forceIns() {
  for (int i=0; i<4; i++) {
    if (chan[i].active) {
      chan[i].insChanged=true;
      chan[i].freqChanged=true;
    }
  }
  updateSNMode=true;
}

void* DivPlatformSMS::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformSMS::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformSMS::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformSMS::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  sn->device_start();
  YMPSG_Init(&sn_nuked,isRealSN);
  snNoiseMode=3;
  rWrite(0xe7);
  updateSNMode=false;
  oldValue=0xff;
}

bool DivPlatformSMS::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformSMS::keyOffAffectsPorta(int ch) {
  return true;
}

int DivPlatformSMS::getPortaFloor(int ch) {
  return 12;
}

void DivPlatformSMS::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSMS::poke(unsigned int addr, unsigned short val) {
  rWrite(val);
}

void DivPlatformSMS::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.val);
}

void DivPlatformSMS::setFlags(unsigned int flags) {
  if ((flags&3)==3) {
    chipClock=COLOR_NTSC/2.0;
  } else if ((flags&3)==2) {
    chipClock=4000000;
  } else if ((flags&3)==1) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  resetPhase=!(flags&16);
  
  if (sn!=NULL) delete sn;
  switch ((flags>>2)&3) {
    case 1: // TI
      sn=new sn76496_base_device(0x4000, 0x4000, 0x01, 0x02, true, 1, false, true);
      isRealSN=true;
      break;
    case 2: // TI+Atari
      sn=new sn76496_base_device(0x4000, 0x0f35, 0x01, 0x02, true, 1, false, true);
      isRealSN=true;
      break;
    case 3: // Game Gear (not fully emulated yet!)
      sn=new sn76496_base_device(0x8000, 0x8000, 0x01, 0x08, false, 1, false, false);
      isRealSN=false;
      break;
    default: // Sega
      sn=new sn76496_base_device(0x8000, 0x8000, 0x01, 0x08, false, 1, false, false);
      isRealSN=false;
      break;
  }
  rate=chipClock/16;
  for (int i=0; i<4; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformSMS::setNuked(bool value) {
  nuked=value;
}

int DivPlatformSMS::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  resetPhase=false;
  oldValue=0xff;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sn=NULL;
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformSMS::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  if (sn!=NULL) delete sn;
}

DivPlatformSMS::~DivPlatformSMS() {
}
