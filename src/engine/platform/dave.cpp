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

#include "dave.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 8

const char* regCheatSheetDave[]={
  "Freq0", "00",
  "Control0", "01",
  "Freq1", "02",
  "Control1", "03",
  "Freq2", "04",
  "Control2", "05",
  "Control3", "06",
  "SoundCtrl", "07",
  "Vol0L", "08",
  "Vol1L", "09",
  "Vol2L", "0A",
  "Vol3L", "0B",
  "Vol0R", "0C",
  "Vol1R", "0D",
  "Vol2R", "0E",
  "Vol3R", "0F",
  "ClockDiv", "1F",
  NULL
};

const unsigned char snapPeriodLong[15]={
  0, 1, 3, 3, 3, 6, 6, 7, 7, 10, 10, 12, 12, 13, 13
};

const unsigned char snapPeriodShort[15]={
  2, 2, 2, 2, 5, 5, 5, 8, 8, 8, 11, 11, 11, 11, 11
};

const unsigned char waveMap[8]={
  0, 1, 1, 2, 3, 0, 0, 0
};

const char** DivPlatformDave::getRegisterSheet() {
  return regCheatSheetDave;
}

void DivPlatformDave::acquire(short** buf, size_t len) {
  for (int i=0; i<6; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    for (int i=4; i<6; i++) {
      if (chan[i].dacSample!=-1) {
        chan[i].dacPeriod+=chan[i].dacRate;
        while (chan[i].dacPeriod>rate) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->samples<=0 || chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
            writeControl=true;
            chan[0].writeVol=true;
            chan[i].dacPeriod-=rate;
            continue;
          }
          signed char dacData=(s->data8[chan[i].dacPos]*chan[i].outVol)>>8;
          chan[i].dacOut=dacData+32;
          chan[i].dacPos++;
          if (!isMuted[i]) {
            rWrite(8+((i-4)<<2),chan[i].dacOut&0x3f);
          }
          if (s->isLoopable() && chan[i].dacPos>=(unsigned int)s->loopEnd) {
            chan[i].dacPos=s->loopStart;
          } else if (chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
            writeControl=true;
            chan[0].writeVol=true;
          }
          chan[i].dacPeriod-=rate;
        }
      }
    }
  
    if (!writes.empty()) {
      QueuedWrite w=writes.front();
      dave->writePort(w.addr,w.val);
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }

    unsigned int next=dave->runOneCycle();
    unsigned short nextL=next&0xffff;
    unsigned short nextR=next>>16;
    
    if ((regPool[7]&0x18)==0x18) {
      oscBuf[0]->putSample(h,0);
      oscBuf[1]->putSample(h,0);
      oscBuf[2]->putSample(h,0);
      oscBuf[3]->putSample(h,0);
      oscBuf[4]->putSample(h,dave->chn0_left<<9);
      oscBuf[5]->putSample(h,dave->chn0_right<<9);
    } else if (regPool[7]&0x08) {
      oscBuf[0]->putSample(h,dave->chn0_state?(dave->chn0_right<<8):0);
      oscBuf[1]->putSample(h,dave->chn1_state?(dave->chn1_right<<8):0);
      oscBuf[2]->putSample(h,dave->chn2_state?(dave->chn2_right<<8):0);
      oscBuf[3]->putSample(h,dave->chn3_state?(dave->chn3_right<<8):0);
      oscBuf[4]->putSample(h,dave->chn0_left<<9);
      oscBuf[5]->putSample(h,0);
    } else if (regPool[7]&0x10) {
      oscBuf[0]->putSample(h,dave->chn0_state?(dave->chn0_left<<8):0);
      oscBuf[1]->putSample(h,dave->chn1_state?(dave->chn1_left<<8):0);
      oscBuf[2]->putSample(h,dave->chn2_state?(dave->chn2_left<<8):0);
      oscBuf[3]->putSample(h,dave->chn3_state?(dave->chn3_left<<8):0);
      oscBuf[4]->putSample(h,0);
      oscBuf[5]->putSample(h,dave->chn0_right<<9);
    } else {
      oscBuf[0]->putSample(h,dave->chn0_state?((dave->chn0_left+dave->chn0_right)<<8):0);
      oscBuf[1]->putSample(h,dave->chn1_state?((dave->chn1_left+dave->chn1_right)<<8):0);
      oscBuf[2]->putSample(h,dave->chn2_state?((dave->chn2_left+dave->chn2_right)<<8):0);
      oscBuf[3]->putSample(h,dave->chn3_state?((dave->chn3_left+dave->chn3_right)<<8):0);
      oscBuf[4]->putSample(h,0);
      oscBuf[5]->putSample(h,0);
    }
    
    buf[0][h]=(short)nextL;
    buf[1][h]=(short)nextR;
  }

  for (int i=0; i<6; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformDave::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&63,MIN(63,chan[i].std.vol.val),63);
      chan[i].writeVol=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].noiseFreq=chan[i].std.duty.val&3;
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (i>=4) {
          chan[i].baseFreq=parent->calcBaseFreq(1,1,parent->calcArp(chan[i].note,chan[i].std.arp.val),false);
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
        }
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.wave.had) {
      chan[i].wave=chan[i].std.wave.val&7;
      if (i==3 && chan[i].wave>3) chan[i].wave=3;
      chan[i].freqChanged=true;
    }
    if (chan[i].std.panL.had) {
      chan[i].panL=chan[i].std.panL.val&63;
    }
    if (chan[i].std.panR.had) {
      chan[i].panR=chan[i].std.panR.val&63;
    }
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      chan[i].writeVol=true;
    }
    if (chan[i].std.ex1.had) {
      chan[i].highPass=chan[i].std.ex1.val&1;
      chan[i].ringMod=chan[i].std.ex1.val&2;
      chan[i].swapCounters=chan[i].std.ex1.val&4;
      chan[i].lowPass=chan[i].std.ex1.val&8;
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
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      if (i>=4) {
        if (chan[i].active && chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          if (chan[i].setPos) {
            chan[i].setPos=false;
          } else {
            chan[i].dacPos=0;
          }
          chan[i].dacPeriod=0;
          chan[i].keyOn=true;
        }
      } else {
        chan[i].resetPhase=true;
        writeControl=true;
      }
    }

    if (chan[i].writeVol) {
      if (i<4) {
        if (chan[i].active && !isMuted[i]) {
          if (i!=0 || chan[4].dacSample<0 || isMuted[4]) {
            rWrite(8+i,(63+chan[i].outVol*chan[i].panL)>>6);
          }
          if (i!=0 || chan[5].dacSample<0 || isMuted[5]) {
            rWrite(12+i,(63+chan[i].outVol*chan[i].panR)>>6);
          }
        } else {
          if (i!=0 || chan[4].dacSample<0 || isMuted[4]) {
            rWrite(8+i,0);
          }
          if (i!=0 || chan[5].dacSample<0 || isMuted[5]) {
            rWrite(12+i,0);
          }
        }
      }
      chan[i].writeVol=false;
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (i>=4) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,0,chan[i].pitch2,1,1);

        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          off=(double)s->centerRate/parent->getCenterRate();
        }
        chan[i].dacRate=chan[i].freq*off;
      } else {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      }

      if (i<3) {
        switch (chan[i].wave) {
          case 0:
            chan[i].freq>>=2;
            break;
          case 1:
            chan[i].freq/=5;
            chan[i].freq>>=1;
            break;
          case 2:
            chan[i].freq/=15;
            chan[i].freq>>=1;
            break;
          case 3:
            chan[i].freq/=63;
            break;
          case 4:
            chan[i].freq>>=5;
            break;
        }
      }

      if (i<4) {
        if (chan[i].freq<1) chan[i].freq=1;
        if (chan[i].freq>4095) chan[i].freq=4095;
      }

      if (i<3) {
        if (chan[i].wave==1) { // short 1
          chan[i].freq=15*(chan[i].freq/15)+snapPeriodShort[(chan[i].freq%15)];
        } else if (chan[i].wave==2) { // long 1
          chan[i].freq=15*(chan[i].freq/15)+snapPeriodLong[(chan[i].freq%15)];
        } else if (chan[i].wave==3) { // long 2 (30, 61, 92, 123... result in silence)
          if ((chan[i].freq%30)==(chan[i].freq/30)-1) chan[i].freq++;
        }
        rWrite((i<<1),chan[i].freq&0xff);
        rWrite(1+(i<<1),(chan[i].freq>>8)|((waveMap[chan[i].wave])<<4)|(chan[i].highPass?0x40:0)|(chan[i].ringMod?0x80:0));
      } else if (i==3) {
        rWrite(6,(chan[i].noiseFreq&3)|((chan[i].wave&3)<<2)|(chan[i].swapCounters?0x10:0)|(chan[i].lowPass?0x20:0)|(chan[i].highPass?0x40:0)|(chan[i].ringMod?0x80:0));
      }

      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  if (writeControl) {
    rWrite(7,(chan[0].resetPhase?1:0)|(chan[1].resetPhase?2:0)|(chan[2].resetPhase?4:0)|((chan[4].dacSample>=0 && !isMuted[4])?8:0)|((chan[5].dacSample>=0 && !isMuted[5])?16:0));
    rWrite(7,((chan[4].dacSample>=0 && !isMuted[4])?8:0)|((chan[5].dacSample>=0 && !isMuted[5])?16:0));
    chan[0].resetPhase=false;
    chan[1].resetPhase=false;
    chan[2].resetPhase=false;
    chan[3].resetPhase=false;
    writeControl=false;
  }
}

int DivPlatformDave::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=NULL;
      // DAC
      if (c.chan>=4) {
        ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].dacSample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          chan[c.chan].dacSample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
        }
        if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
          chan[c.chan].dacSample=-1;
          chan[0].writeVol=true;
        }
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value,false);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
        if (chan[c.chan].setPos) {
          chan[c.chan].setPos=false;
        } else {
          chan[c.chan].dacPos=0;
        }
        chan[c.chan].dacPeriod=0;
        writeControl=true;
      } else {
        ins=parent->getIns(chan[c.chan].ins,DIV_INS_DAVE);
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].writeVol=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].writeVol=true;
      if (c.chan>=4) {
        chan[c.chan].dacSample=-1;
        chan[0].writeVol=true;
        writeControl=true;
      }
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
          if (chan[c.chan].active) {
            chan[c.chan].writeVol=true;
          }
        }
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
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      if (chan[c.chan].wave>4) chan[c.chan].wave=4;
      if (c.chan==3 && chan[c.chan].wave>3) chan[c.chan].wave=3;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].noiseFreq=c.value&3;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_DAVE_HIGH_PASS:
      chan[c.chan].highPass=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_DAVE_RING_MOD:
      chan[c.chan].ringMod=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_DAVE_SWAP_COUNTERS:
      chan[c.chan].swapCounters=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_DAVE_LOW_PASS:
      chan[c.chan].lowPass=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_DAVE_CLOCK_DIV:
      clockDiv=c.value;
      rWrite(31,clockDiv?2:0);
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2+chan[c.chan].sampleNoteDelta);
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
    case DIV_CMD_PANNING: {
      chan[c.chan].panL=c.value>>2;
      chan[c.chan].panR=c.value2>>2;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta);
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_DAVE));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      if (c.chan<4) break;
      chan[c.chan].dacPos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 63;
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

void DivPlatformDave::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].writeVol=true;
  if (ch>=4) {
    chan[0].writeVol=true;
    writeControl=true;
  }
}

void DivPlatformDave::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].writeVol=true;
  }
  writeControl=true;
  rWrite(31,clockDiv?2:0);
}

void* DivPlatformDave::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformDave::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformDave::getPan(int ch) {
  if (ch==5) return 1;
  if (ch==4) return 0x100;
  return (chan[ch].panL<<8)|chan[ch].panR;
}

void DivPlatformDave::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (chan[ch].highPass) {
    ret.push_back(DivChannelPair(_("high"),(ch+1)&3));
  }
  if (chan[ch].ringMod) {
    ret.push_back(DivChannelPair(_("ring"),(ch+2)&3));
  }
  if (chan[ch].lowPass && ch==3) {
    ret.push_back(DivChannelPair(_("low"),2));
  }
}

DivChannelModeHints DivPlatformDave::getModeHints(int ch) {
  DivChannelModeHints ret;
  return ret;
}

DivSamplePos DivPlatformDave::getSamplePos(int ch) {
  if (ch<4 || ch>=6) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformDave::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformDave::getRegisterPool() {
  return regPool;
}

int DivPlatformDave::getRegisterPoolSize() {
  return 32;
}

void DivPlatformDave::reset() {
  writes.clear();
  memset(regPool,0,32);
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformDave::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  writeControl=false;
  clockDiv=false;
  dave->reset(true);
}

int DivPlatformDave::getOutputCount() {
  return 2;
}

bool DivPlatformDave::hasSoftPan(int ch) {
  return true;
}

bool DivPlatformDave::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformDave::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformDave::setFlags(const DivConfig& flags) {
  chipClock=8000000.0;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/16;
  for (int i=0; i<6; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformDave::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformDave::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformDave::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  dave=new Ep128::Dave;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformDave::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  delete dave;
}

DivPlatformDave::~DivPlatformDave() {
}
