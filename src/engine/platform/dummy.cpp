/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "dummy.h"
#include "../engine.h"
#include <stdio.h>
#include <math.h>

#define CHIP_FREQBASE 2048

#define DUMMY_SOUND_MAX 2

static int expTable[4096];

struct ParamDef {
  bool enable;
  unsigned char shape;
  int al, ar, dr, sl, sr, rr, tl, dt, xp, xr;
  short ir;
  unsigned short xm;
  ParamDef(bool on, unsigned char sh, int lev, int a, int d, int s, int srate, int r, int t, int det, int xorp, int xorr, short irate, unsigned short xorm):
    enable(on),
    shape(sh),
    al(lev),
    ar(a),
    dr(d),
    sl(s),
    sr(srate),
    rr(r),
    tl(t),
    dt(det),
    xp(xorp),
    xr(xorr),
    ir(irate),
    xm(xorm) {}
  ParamDef():
    enable(false),
    shape(0),
    ar(0),
    dr(0),
    sl(0),
    sr(0),
    rr(0),
    tl(0),
    dt(0),
    xp(0),
    xr(0),
    ir(0),
    xm(0) {}
};

static const ParamDef dummySounds[DUMMY_SOUND_MAX*2]={
  // Dummy Keyboard
  ParamDef(true, 1, 0xfffff, 0xfffff, 0x10, 0xa0000, 3, 0x60, 0x400, 0, 0x30000000, 0x1000, 4800, 0xffff),
  ParamDef(true, 1, 0xfffff, 0xfffff, 0x10, 0xa0000, 3, 0x60, 0x400, 80, 0x50000000, 0x00, 4300, 0xffff),
  // Dummy Electric Keyboard
  ParamDef(true, 2, 0xfffff, 0xfffff, 0x7, 0xc0000, 3, 0x60, 0x200, 50, 0x40000000, 0, 3000, 0xffff),
  ParamDef(true, 1, 0xfffff, 0xfffff, 0x7, 0xc0000, 3, 0x60, 0x600, -50, 0x80000000, 0, 700, 0xffff),
  // Dummy Vibraphone
  // Dummy Percussion
  // Dummy Organ
  // Dummy Accordion
  // Dummy Guitar
  // Dummy Overdrive
  // Dummy Bass
  // Dummy Slap
  // Dummy Bow
  // Dummy Expression
  // Dummy String
  // Dummy Voice
  // Dummy Trumpet
  // Dummy Brass
  // Dummy Sax
  // Dummy Horn
  // Dummy Flute
  // Dummy Pipe
  // Dummy Lead 1
  // Dummy Lead 2
  // Dummy Pad 1
  // Dummy Pad 2
  // Dummy Rain
  // Dummy Ambience
  // Dummy Banjo
  // Dummy Fiddle
  // Dummy Bell
  // Dummy Taiko
  // Dummy FX 1
  // Dummy FX 2
};

// DivPlatformDummy::Channel

int DivPlatformDummy::Channel::advance() {
  short (*waveforms[3])(unsigned short,unsigned short)={
    [](unsigned short pos, unsigned short rate) -> short { // saw
      return pos;
    },
    [](unsigned short pos, unsigned short rate) -> short { // pulse
      return 32767;
    },
    [](unsigned short pos, unsigned short rate) -> short { // sine
      return (short)(32767.0f*sin(M_PI*(float)pos/32768.0));
    },
  };

  int sum=0;
  int oscIndex=0;
  for (Oscillator& i: osc) {
    const ParamDef& params=dummySounds[oscIndex+(sound<<1)];
    if (params.enable) {
      // run envelope
      if (active) {
        switch (i.envState) {
          case 0: // attack
            i.env+=params.ar;
            if (i.env>0xfffff) {
              i.env=0xfffff;
              i.envState=1;
            }
            break;
          case 1: // decay
            i.env-=params.dr;
            if (i.env<params.sl) {
              i.env=params.sl;
              i.envState=2;
            }
            break;
          case 2: // sustain
            i.env-=params.sr;
            if (i.env<0) i.env=0;
            break;
        }
      } else {
        // release
        i.env-=params.rr;
        if (i.env<0) i.env=0;
      }

      // calculate output
      short out=waveforms[params.shape](i.pos>>16,freq);
      if (i.xorPos>=i.pos) {
        out^=params.xm;
      }
      i.xorPos+=params.xr;
      if (i.out<out) {
        i.out+=params.ir;
        if (i.out>=out) i.out=out;
      } else {
        i.out-=params.ir;
        if (i.out<=out) i.out=out;
      }

      // calculate attenuation
      sum+=(expTable[CLAMP(i.env-params.tl,0,0xfffff)>>8]*i.out)>>16;

      // advance phase
      i.pos+=(freq*(65536+params.dt));

      oscIndex++;
    }
  }
  return (sum*vol)>>4;
}

void DivPlatformDummy::Channel::start() {
  int oscIndex=0;
  for (Oscillator& i: osc) {
    const ParamDef& params=dummySounds[oscIndex+(sound<<1)];
    i.pos=0;
    i.xorPos=params.xp;
    i.env=params.al;
    i.envState=0;
    oscIndex++;
  }
}

// DivPlatformDummy

void DivPlatformDummy::acquire(short** buf, size_t len) {
  int chanOut;
  for (int i=0; i<chans; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t i=0; i<len; i++) {
    int out=0;
    for (unsigned char j=0; j<chans; j++) {
      chanOut=chan[j].advance();
      if (!isMuted[j]) {
        oscBuf[j]->putSample(i,chanOut);
        out+=chanOut;
      } else {
        oscBuf[j]->putSample(i,0);
      }
      chan[j].pos+=chan[j].freq;
    }
    if (out<-32768) out=-32768;
    if (out>32767) out=32767;
    buf[0][i]=out;
  }
  for (int i=0; i<chans; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformDummy::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformDummy::tick(bool sysTick) {
  for (unsigned char i=0; i<chans; i++) {
    if (chan[i].keyOn) {
      chan[i].start();
      chan[i].keyOn=false;
    }

    if (chan[i].keyOff) {
      chan[i].keyOff=false;
    }

    if (chan[i].freqChanged) {
      chan[i].freqChanged=false;
      chan[i].freq=chan[i].calcFreq();
    }
  }
}

SharedChannel* DivPlatformDummy::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformDummy::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformDummy::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_DUMMY);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value);
        chan[c.chan].freqChanged=true;
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].sound=CLAMP(ins->dummy.sound,0,DUMMY_SOUND_MAX-1);
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      if (chan[c.chan].vol>15) chan[c.chan].vol=15;
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=chan[c.chan].calcBaseFreq(c.value2);
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
      if (return2) return 2;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=chan[c.chan].calcBaseFreq(c.value);
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformDummy::notifyInsChange(int ins) {
  for (int i=0; i<chans; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformDummy::notifyInsDeletion(void* ins) {
  // nothing
}

void DivPlatformDummy::reset() {
  for (int i=0; i<chans; i++) {
    chan[i]=DivPlatformDummy::Channel(parent->song.compatFlags.linearPitch);
    chan[i].pitchTable=&pitchTable;
    chan[i].vol=0x0f;
  }
}

void DivPlatformDummy::notifyPitchTable(int sample) {
  pitchTable.init(parent->song.tuning,chipClock,CHIP_FREQBASE,0xffff,false,parent->song.compatFlags.linearPitch);
}

unsigned int DivPlatformDummy::getMaxFreq(int ch) {
  return 0xffff;
}

int DivPlatformDummy::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=false;
    if (i<channels) {
      oscBuf[i]=new DivDispatchOscBuffer;
      oscBuf[i]->setRate(65536);
    }
  }
  for (int i=0; i<4096; i++) {
    expTable[4095-i]=8192.0/pow(2.0,(double)i/512.0);
  }
  expTable[0]=0;
  rate=65536;
  chipClock=65536;
  notifyPitchTable();
  chans=channels;
  reset();
  return channels;
}

void DivPlatformDummy::quit() {
  for (int i=0; i<chans; i++) {
    delete oscBuf[i];
  }
}

DivPlatformDummy::~DivPlatformDummy() {
}
