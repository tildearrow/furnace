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

#include "pcspkr.h"
#include "../engine.h"
#include <math.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/kd.h>
#endif

#define PCSPKR_DIVIDER 4
#define CHIP_DIVIDER 1

const char* regCheatSheetPCSpeaker[]={
  "Period", "0",
  NULL
};

const char** DivPlatformPCSpeaker::getRegisterSheet() {
  return regCheatSheetPCSpeaker;
}

const char* DivPlatformPCSpeaker::getEffectName(unsigned char effect) {
  return NULL;
}

const float cut=0.05;
const float reso=0.06;

void DivPlatformPCSpeaker::acquire_unfilt(short* bufL, short* bufR, size_t start, size_t len) {
  int out=0;
  for (size_t i=start; i<start+len; i++) {
    if (on) {
      pos-=PCSPKR_DIVIDER;
      if (pos>freq) pos=freq;
      while (pos<0) {
        if (freq<1) {
          pos=1;
        } else {
          pos+=freq;
        }
      }
      out=(pos>(freq>>1) && !isMuted[0])?32767:0;
      bufL[i]=out;
      oscBuf->data[oscBuf->needle++]=out;
    } else {
      bufL[i]=0;
      oscBuf->data[oscBuf->needle++]=0;
    }
  }
}

void DivPlatformPCSpeaker::acquire_cone(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    if (on) {
      pos-=PCSPKR_DIVIDER;
      if (pos>freq) pos=freq;
      while (pos<0) {
        if (freq<1) {
          pos=1;
        } else {
          pos+=freq;
        }
      }
      float next=(pos>((freq+16)>>1) && !isMuted[0])?1:0;
      low+=0.04*band;
      band+=0.04*(next-low-band);
      float out=(low+band)*0.75;
      if (out>1.0) out=1.0;
      if (out<-1.0) out=-1.0;
      bufL[i]=out*32767;
      oscBuf->data[oscBuf->needle++]=out*32767;
    } else {
      bufL[i]=0;
      oscBuf->data[oscBuf->needle++]=0;
    }
  }
}

void DivPlatformPCSpeaker::acquire_piezo(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    if (on) {
      pos-=PCSPKR_DIVIDER;
      if (pos>freq) pos=freq;
      while (pos<0) {
        if (freq<1) {
          pos=1;
        } else {
          pos+=freq;
        }
      }
      float next=(pos>((freq+64)>>1) && !isMuted[0])?1:0;
      low+=cut*band;
      band+=cut*(next-low-(reso*band));
      float out=band*0.15-(next-low)*0.06;
      if (out>1.0) out=1.0;
      if (out<-1.0) out=-1.0;
      bufL[i]=out*32767;
      oscBuf->data[oscBuf->needle++]=out*32767;
    } else {
      bufL[i]=0;
      oscBuf->data[oscBuf->needle++]=0;
    }
  }
}

void DivPlatformPCSpeaker::beepFreq(int freq) {
#ifdef __linux__
  static struct input_event ie;
  if (beepFD>=0) {
    gettimeofday(&ie.time,NULL);
    ie.type=EV_SND;
    ie.code=SND_TONE;
    if (freq>0) {
      ie.value=chipClock/freq;
    } else {
      ie.value=0;
    }
    if (write(beepFD,&ie,sizeof(struct input_event))<0) {
      perror("error while writing frequency!");
    } else {
      //printf("writing freq: %d\n",freq);
    }
  }
#endif
}

void DivPlatformPCSpeaker::acquire_real(short* bufL, short* bufR, size_t start, size_t len) {
  int out=0;
  if (lastOn!=on || lastFreq!=freq) {
    lastOn=on;
    lastFreq=freq;
    beepFreq((on && !isMuted[0])?freq:0);
  }
  for (size_t i=start; i<start+len; i++) {
    if (on) {
      pos-=PCSPKR_DIVIDER;
      if (pos>freq) pos=freq;
      while (pos<0) {
        if (freq<1) {
          pos=1;
        } else {
          pos+=freq;
        }
      }
      out=(pos>(freq>>1) && !isMuted[0])?32767:0;
      oscBuf->data[oscBuf->needle++]=out;
    } else {
      oscBuf->data[oscBuf->needle++]=0;
    }
    bufL[i]=0;
  }
}

void DivPlatformPCSpeaker::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  switch (speakerType) {
    case 0:
      acquire_unfilt(bufL,bufR,start,len);
      break;
    case 1:
      acquire_cone(bufL,bufR,start,len);
      break;
    case 2:
      acquire_piezo(bufL,bufR,start,len);
      break;
    case 3:
      acquire_real(bufL,bufR,start,len);
      break;
  }
}

void DivPlatformPCSpeaker::tick(bool sysTick) {
  for (int i=0; i<1; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol && chan[i].std.vol.val);
      on=chan[i].outVol;
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
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-2048,2048);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true,0,chan[i].pitch2)-1;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].keyOn) {
        on=true;
      }
      if (chan[i].keyOff) {
        on=false;
      }
      freq=chan[i].freq;
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformPCSpeaker::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BEEPER));
      break;
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
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) {
          on=chan[c.chan].vol;
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
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BEEPER));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 1;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformPCSpeaker::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformPCSpeaker::forceIns() {
  for (int i=0; i<1; i++) {
    chan[i].insChanged=true;
  }
}

void* DivPlatformPCSpeaker::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformPCSpeaker::getOscBuffer(int ch) {
  return oscBuf;
}

unsigned char* DivPlatformPCSpeaker::getRegisterPool() {
  if (on) {
    regPool[0]=freq;
    regPool[1]=freq>>8;
  } else {
    regPool[0]=0;
    regPool[1]=0;
  }
  return regPool;
}

int DivPlatformPCSpeaker::getRegisterPoolSize() {
  return 2;
}

void DivPlatformPCSpeaker::reset() {
  for (int i=0; i<1; i++) {
    chan[i]=DivPlatformPCSpeaker::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  on=false;
  lastOn=false;
  freq=0;
  lastFreq=0;
  pos=0;
  flip=false;
  low=0;
  band=0;

  if (speakerType==3) {
#ifdef __linux__
    if (beepFD==-1) {
      beepFD=open("/dev/input/by-path/platform-pcspkr-event-spkr",O_WRONLY);
      if (beepFD<0) {
        perror("error while opening PC speaker");
      }
    }
#endif
    beepFreq(0);
  } else {
    beepFreq(0);
  }

  memset(regPool,0,2);
}

bool DivPlatformPCSpeaker::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformPCSpeaker::setFlags(unsigned int flags) {
  chipClock=COLOR_NTSC/3.0;
  rate=chipClock/PCSPKR_DIVIDER;
  speakerType=flags&3;
  oscBuf->rate=rate;
}

void DivPlatformPCSpeaker::notifyInsDeletion(void* ins) {
  for (int i=0; i<1; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPCSpeaker::notifyPlaybackStop() {
  beepFreq(0);
}

void DivPlatformPCSpeaker::poke(unsigned int addr, unsigned short val) {
  // ???
}

void DivPlatformPCSpeaker::poke(std::vector<DivRegWrite>& wlist) {
  // ???
}

int DivPlatformPCSpeaker::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  beepFD=-1;
  for (int i=0; i<1; i++) {
    isMuted[i]=false;
  }
  oscBuf=new DivDispatchOscBuffer;
  setFlags(flags);

  reset();
  return 5;
}

void DivPlatformPCSpeaker::quit() {
  if (speakerType==3) {
    beepFreq(0);
  }
#ifdef __linux__
  if (beepFD>=0) close(beepFD);
#endif
  delete oscBuf;
}

DivPlatformPCSpeaker::~DivPlatformPCSpeaker() {
}
