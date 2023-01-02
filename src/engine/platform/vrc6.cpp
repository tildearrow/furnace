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

#include "vrc6.h"
#include "../engine.h"
#include <cstddef>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) rWrite(0x9000+(c<<12)+(a&3),v)

const char* regCheatSheetVRC6[]={
  "S0DutyVol", "9000",
  "S0PeriodL", "9001",
  "S0PeriodH", "9002",
  "GlobalCtl", "9003",
  "S1DutyVol", "A000",
  "S1PeriodL", "A001",
  "S1PeriodH", "A002",
  "SawVolume", "B000",
  "SawPeriodL", "B001",
  "SawPeriodH", "B002",
  "TimerLatch", "F000",
  "TimerCtl", "F001",
  "IRQAck", "F002",
  NULL
};

const char** DivPlatformVRC6::getRegisterSheet() {
  return regCheatSheetVRC6;
}

void DivPlatformVRC6::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    // PCM part
    for (int i=0; i<2; i++) {
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        chan[i].dacPeriod+=chan[i].dacRate;
        if (chan[i].dacPeriod>rate) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->samples<=0) {
            chan[i].dacSample=-1;
            chWrite(i,0,0);
            continue;
          }
          unsigned char dacData=(((unsigned char)s->data8[chan[i].dacPos]^0x80)>>4);
          chan[i].dacOut=MAX(0,MIN(15,(dacData*chan[i].outVol)>>4));
          if (!isMuted[i]) {
            chWrite(i,0,0x80|chan[i].dacOut);
          }
          chan[i].dacPos++;
          if (s->isLoopable() && chan[i].dacPos>=(unsigned int)s->loopEnd) {
            chan[i].dacPos=s->loopStart;
          } else if (chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
            chWrite(i,0,0);
          }
          chan[i].dacPeriod-=rate;
        }
      }
    }

    // VRC6 part
    vrc6.tick();
    int sample=vrc6.out()<<9; // scale to 16 bit
    if (sample>32767) sample=32767;
    if (sample<-32768) sample=-32768;
    bufL[i]=bufR[i]=sample;

    // Oscilloscope buffer part
    if (++writeOscBuf>=32) {
      writeOscBuf=0;
      for (int i=0; i<2; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=vrc6.pulse_out(i)<<10;
      }
      oscBuf[2]->data[oscBuf[2]->needle++]=vrc6.sawtooth_out()<<10;
    }

    // Command part
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      switch (w.addr&0xf000) {
        case 0x9000: // Pulse 1
          if (w.addr<=0x9003) {
            if (w.addr==0x9003) {
              vrc6.control_w(w.val);
            } else if (w.addr<=0x9002) {
              vrc6.pulse_w(0,w.addr&3,w.val);
            }
            regPool[w.addr-0x9000]=w.val;
          }
          break;
        case 0xa000: // Pulse 2
          if (w.addr<=0xa002) {
            vrc6.pulse_w(1,w.addr&3,w.val);
            regPool[(w.addr-0xa000)+4]=w.val;
          }
          break;
        case 0xb000: // Sawtooth
          if (w.addr<=0xb002) {
            vrc6.saw_w(w.addr&3,w.val);
            regPool[(w.addr-0xb000)+7]=w.val;
          }
          break;
        case 0xf000: // IRQ/Timer
          if (w.addr<=0xf002) {
            vrc6.timer_w(w.addr&3,w.val);
            regPool[(w.addr-0xf000)+10]=w.val;
          }
          break;
      }
      writes.pop();
    }
  }
}

void DivPlatformVRC6::tick(bool sysTick) {
  for (int i=0; i<3; i++) {
    // 16 for pulse; 14 for saw
    int CHIP_DIVIDER=(i==2)?14:16;
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      if (i==2) { // sawtooth
        chan[i].outVol=((chan[i].vol&63)*MIN(63,chan[i].std.vol.val))/63;
        if (chan[i].outVol<0) chan[i].outVol=0;
        if (chan[i].outVol>63) chan[i].outVol=63;
        if (!isMuted[i]) {
          chWrite(i,0,chan[i].outVol);
        }
      } else { // pulse
        chan[i].outVol=((chan[i].vol&15)*MIN(15,chan[i].std.vol.val))/15;
        if (chan[i].outVol<0) chan[i].outVol=0;
        if (chan[i].outVol>15) chan[i].outVol=15;
        if ((!isMuted[i]) && (!chan[i].pcm)) {
          chWrite(i,0,(chan[i].outVol&0xf)|((chan[i].duty&7)<<4));
        }
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
      chan[i].duty=chan[i].std.duty.val;
      if ((!isMuted[i]) && (i!=2) && (!chan[i].pcm)) { // pulse
        chWrite(i,0,(chan[i].outVol&0xf)|((chan[i].duty&7)<<4));
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
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val && chan[i].active) {
        if ((i!=2) && (!chan[i].pcm)) {
          if (dumpWrites) addWrite(0xffff0002+(i<<8),0);
          DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_VRC6);
          chan[i].dacSample=ins->amiga.getSample(chan[i].note);
          if (chan[i].dacSample<0 || chan[i].dacSample>=parent->song.sampleLen) {
            if (dumpWrites) {
              chWrite(i,2,0x80);
              chWrite(i,0,isMuted[i]?0:0x80);
              addWrite(0xffff0000+(i<<8),chan[i].dacSample);
            }
            chan[i].dacPos=0;
            chan[i].dacPeriod=0;
            chan[i].keyOn=true;
          }
        }
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (i==2) { // sawtooth
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,14)-1;
      } else { // pulse
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,16)-1;
        if (chan[i].furnaceDac) {
          double off=1.0;
          if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(chan[i].dacSample);
            if (s->centerRate<1) {
              off=1.0;
            } else {
              off=8363.0/(double)s->centerRate;
            }
          }
          chan[i].dacRate=((double)chipClock)/MAX(1,off*chan[i].freq);
          if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dacRate);
        }
      }
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].keyOff) {
        chWrite(i,2,0);
      } else if (chan[i].active) {
        chWrite(i,1,chan[i].freq&0xff);
        chWrite(i,2,0x80|((chan[i].freq>>8)&0xf));
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformVRC6::dispatch(DivCommand c) {
  int CHIP_DIVIDER=(c.chan==2)?14:16;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.chan!=2) { // pulse wave
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_VRC6);
        if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
          chan[c.chan].pcm=true;
        } else if (chan[c.chan].furnaceDac) {
          chan[c.chan].pcm=false;
        }
        if (chan[c.chan].pcm) {
          if (skipRegisterWrites) break;
          if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
            if (c.value!=DIV_NOTE_NULL) chan[c.chan].dacSample=ins->amiga.getSample(c.value);
            if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
              chan[c.chan].dacSample=-1;
              if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
              break;
            } else {
              if (dumpWrites) {
                chWrite(c.chan,2,0x80);
                chWrite(c.chan,0,isMuted[c.chan]?0:0x80);
                addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
              }
            }
            chan[c.chan].dacPos=0;
            chan[c.chan].dacPeriod=0;
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
              chan[c.chan].freqChanged=true;
              chan[c.chan].note=c.value;
            }
            chan[c.chan].active=true;
            chan[c.chan].macroInit(ins);
            if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
              chan[c.chan].outVol=chan[c.chan].vol;
            }
            //chan[c.chan].keyOn=true;
            chan[c.chan].furnaceDac=true;
          } else {
            if (c.value!=DIV_NOTE_NULL) {
              chan[c.chan].note=c.value;
            }
            chan[c.chan].dacSample=12*sampleBank+chan[c.chan].note%12;
            if (chan[c.chan].dacSample>=parent->song.sampleLen) {
              chan[c.chan].dacSample=-1;
              if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
              break;
            } else {
              if (dumpWrites) addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
            }
            chan[c.chan].dacPos=0;
            chan[c.chan].dacPeriod=0;
            chan[c.chan].dacRate=parent->getSample(chan[c.chan].dacSample)->rate;
            if (dumpWrites) {
              chWrite(c.chan,2,0x80);
              chWrite(c.chan,0,isMuted[c.chan]?0:0x80);
              addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dacRate);
            }
            chan[c.chan].furnaceDac=false;
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
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_VRC6));
      if (!isMuted[c.chan]) {
        if (c.chan==2) { // sawtooth
          chWrite(c.chan,0,chan[c.chan].vol);
        } else if (!chan[c.chan].pcm) {
          chWrite(c.chan,0,(chan[c.chan].vol&0xf)|((chan[c.chan].duty&7)<<4));
        }
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dacSample=-1;
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].pcm=false;
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
        if (!isMuted[c.chan]) {
          if (chan[c.chan].active) {
            if (c.chan==2) { // sawtooth
              chWrite(c.chan,0,chan[c.chan].vol);
            } else if (!chan[c.chan].pcm) {
              chWrite(c.chan,0,(chan[c.chan].vol&0xf)|((chan[c.chan].duty&7)<<4));
            }
          }
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
    case DIV_CMD_STD_NOISE_MODE:
      if ((c.chan!=2) && (!chan[c.chan].pcm)) { // pulse
        chan[c.chan].duty=c.value;
      }
      break;
    case DIV_CMD_SAMPLE_MODE:
      if (c.chan!=2) { // pulse
        chan[c.chan].pcm=c.value;
      }
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_VRC6));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      if (c.chan==2) return 63; // sawtooth has 6 bit volume
      return 15; // pulse has 4 bit volume
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

void DivPlatformVRC6::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    chWrite(ch,0,0);
  } else if (chan[ch].active) {
    if (ch==2) { // sawtooth
      chWrite(ch,0,chan[ch].outVol);
    } else {
      chWrite(ch,0,chan[ch].pcm?chan[ch].dacOut:((chan[ch].outVol&0xf)|((chan[ch].duty&7)<<4)));
    }
  }
}

void DivPlatformVRC6::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

void* DivPlatformVRC6::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformVRC6::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformVRC6::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformVRC6::getRegisterPool() {
  return regPool;
}

int DivPlatformVRC6::getRegisterPoolSize() {
  return 13;
}

void DivPlatformVRC6::reset() {
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformVRC6::Channel();
    chan[i].std.setEngine(parent);
  }
  // HELP
  chan[2].vol=63;
  chan[2].outVol=63;
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  sampleBank=0;

  vrc6.reset();
  // Initialize control register
  rWrite(0x9003,0);
  // Clear internal IRQ
  rWrite(0xf000,0);
  rWrite(0xf001,0);
  rWrite(0xf002,0);
}

bool DivPlatformVRC6::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformVRC6::setFlags(const DivConfig& flags) {
  int clockSel=flags.getInt("clockSel",0);
  if (clockSel==2) { // Dendy
    chipClock=COLOR_PAL*2.0/5.0;
  } else if (clockSel==1) { // PAL
    chipClock=COLOR_PAL*3.0/8.0;
  } else { // NTSC
    chipClock=COLOR_NTSC/2.0;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  for (int i=0; i<3; i++) {
    oscBuf[i]->rate=rate/32;
  }
}

void DivPlatformVRC6::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformVRC6::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformVRC6::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformVRC6::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
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

void DivPlatformVRC6::quit() {
  for (int i=0; i<3; i++) {
    delete oscBuf[i];
  }
}

DivPlatformVRC6::~DivPlatformVRC6() {
}
