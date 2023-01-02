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

#include "ga20.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define rWrite(a,v) {if(!skipRegisterWrites) {writes.emplace(a,v); if(dumpWrites) addWrite(a,v);}}

#define CHIP_DIVIDER 64

const char* regCheatSheetGA20[]={
  // on-chip
  "CHX_StartL", "X*8+0",
  "CHX_StartH", "X*8+1",
  "CHX_EndL", "X*8+2",
  "CHX_EndH", "X*8+3",
  "CHX_Freq", "X*8+4",
  "CHX_Volume", "X*8+5",
  "CHX_Control", "X*8+6",
  "CHX_Status", "X*8+7",
  NULL
};

const char** DivPlatformGA20::getRegisterSheet() {
  return regCheatSheetGA20;
}

inline void DivPlatformGA20::chWrite(unsigned char ch, unsigned int addr, unsigned char val) {
  if (!skipRegisterWrites) {
    if ((ch<4) && (addr<8)) {
      rWrite((ch<<3)+(addr&7),val);
    }
  }
}

void DivPlatformGA20::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (ga20BufLen<len) {
    ga20BufLen=len;
    for (int i=0; i<4; i++) {
      delete[] ga20Buf[i];
      ga20Buf[i]=new short[ga20BufLen];
    }
  }

  for (size_t h=start; h<start+len; h++) {
    if ((--delay)<=0) {
      delay=MAX(0,delay);
      if (!writes.empty()) {
        QueuedWrite& w=writes.front();
        ga20.write(w.addr,w.val);
        regPool[w.addr]=w.val;
        writes.pop();
        delay=w.delay;
      }
    }
    short *buffer[4] = {&ga20Buf[0][h],&ga20Buf[1][h],&ga20Buf[2][h],&ga20Buf[3][h]};
    ga20.sound_stream_update(buffer, 1);
    bufL[h]=(signed int)(ga20Buf[0][h]+ga20Buf[1][h]+ga20Buf[2][h]+ga20Buf[3][h])>>2;
    for (int i=0; i<4; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=ga20Buf[i][h];
    }
  }
}

u8 DivPlatformGA20::read_byte(u32 address) {
  if ((sampleMem!=NULL) && (address<getSampleMemCapacity())) {
    return sampleMem[address&0xfffff];
  }
  return 0;
}

void DivPlatformGA20::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      const signed char macroVol=VOL_SCALE_LOG((chan[i].vol&0xff),(0xff*MIN(chan[i].macroVolMul,chan[i].std.vol.val))/chan[i].macroVolMul,0xff);
      if ((!isMuted[i]) && (macroVol!=chan[i].outVol)) {
        chan[i].outVol=macroVol;
        chan[i].volumeChanged=true;
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
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].audPos=0;
        chan[i].setPos=true;
      }
    }
    if (chan[i].volumeChanged) {
      chan[i].resVol=(chan[i].active && isMuted[i])?0:chan[i].outVol&0xff;
      chWrite(i,0x5,chan[i].resVol);
      chan[i].volumeChanged=false;
    }
    if (chan[i].setPos) {
      // force keyon
      chan[i].keyOn=true;
      chan[i].setPos=false;
    } else {
      chan[i].audPos=0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      double off=1.0;
      int sample=chan[i].sample;
      if (sample>=0 && sample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(sample);
        if (s->centerRate<1) {
          off=1.0;
        } else {
          off=8363.0/s->centerRate;
        }
      }
      DivSample* s=parent->getSample(chan[i].sample);
      chan[i].freq=0x100-(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER));
      if (chan[i].freq>255) chan[i].freq=255;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].keyOn) {
        unsigned int start=0;
        unsigned int end=0;
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          start=sampleOffGA20[chan[i].sample]&0xfffff;
          end=start+s->length8;
        }
        if (chan[i].audPos>0) {
          start=start+MIN(chan[i].audPos,MIN(getSampleMemCapacity()-1,s->length8));
        }
        start=MIN(start,getSampleMemCapacity()-1);
        end=MIN(end,getSampleMemCapacity()-1);
        // force keyoff first
        chWrite(i,5,0);
        chWrite(i,6,0);
        // keyon
        if (chan[i].prevFreq!=chan[i].freq) {
          chWrite(i,4,chan[i].freq&0xff);
          chan[i].prevFreq=chan[i].freq;
        }
        chWrite(i,0,start>>4);
        chWrite(i,1,start>>12);
        chWrite(i,2,end>>4);
        chWrite(i,3,end>>12);
        chWrite(i,5,chan[i].resVol);
        chWrite(i,6,2);
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
          if (!isMuted[i]) {
            chan[i].volumeChanged=true;
          }
        }
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chWrite(i,5,0);
        chWrite(i,6,0);
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        if (chan[i].prevFreq!=chan[i].freq) {
          chWrite(i,4,chan[i].freq&0xff);
          chan[i].prevFreq=chan[i].freq;
        }
        chan[i].freqChanged=false;
      }
    }
  }
}

int DivPlatformGA20::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
      if (c.value!=DIV_NOTE_NULL) chan[c.chan].sample=ins->amiga.getSample(c.value);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      }
      if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
        if (!isMuted[c.chan]) {
          chan[c.chan].volumeChanged=true;
        }
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
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
          if (!isMuted[c.chan]) {
            chan[c.chan].volumeChanged=true;
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
    case DIV_CMD_NOTE_PORTA: {
      const int destFreq=NOTE_PERIODIC(c.value2);
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
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].audPos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 255;
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

void DivPlatformGA20::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volumeChanged=true;
}

void DivPlatformGA20::forceIns() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].volumeChanged=true;
    chan[i].freqChanged=true;
    chan[i].prevFreq=-1;
  }
}

void* DivPlatformGA20::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformGA20::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformGA20::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformGA20::reset() {
  while (!writes.empty()) {
    writes.pop();
  }
  memset(regPool,0,32);
  ga20.device_reset();
  delay=0;
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformGA20::Channel();
    chan[i].std.setEngine(parent);
    // keyoff all channels
    chWrite(i,5,0);
    chWrite(i,6,0);
  }
}

bool DivPlatformGA20::isStereo() {
  return false;
}

void DivPlatformGA20::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGA20::notifyWaveChange(int wave) {
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformGA20::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformGA20::setFlags(const DivConfig& flags) {
  chipClock=COLOR_NTSC;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/4;
  for (int i=0; i<4; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformGA20::poke(unsigned int addr, unsigned short val) {
  rWrite(addr&0x1f,val);
}

void DivPlatformGA20::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr&0x1f,i.val);
}

unsigned char* DivPlatformGA20::getRegisterPool() {
  return regPool;
}

int DivPlatformGA20::getRegisterPoolSize() {
  return 32;
}

const void* DivPlatformGA20::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformGA20::getSampleMemCapacity(int index) {
  return index == 0 ? 1048576 : 0;
}

size_t DivPlatformGA20::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformGA20::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformGA20::renderSamples(int sysID) {
  memset(sampleMem,0x00,getSampleMemCapacity());
  memset(sampleOffGA20,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOffGA20[i]=0;
      continue;
    }

    const int length=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos)-1,length);
    if (actualLength>0) {
      sampleOffGA20[i]=memPos;
      for (int j=0; j<actualLength; j++) {
        // convert to 8 bit unsigned
        unsigned char val=((unsigned char)(s->data8[j]))^0x80;
        sampleMem[memPos++]=CLAMP(val,0x01,0xff);
      }
      // write end of sample marker
      memset(&sampleMem[memPos],0x00,1);
      memPos+=1;
    }
    if ((memPos+MAX(actualLength,0))>=(getSampleMemCapacity()-1)) {
      logW("out of GA20 PCM memory for sample %d!",i);
      break;
    } else {
      sampleLoaded[i]=true;
    }
    // allign to 16 byte
    memPos=(memPos+0xf)&~0xf;
  }
  sampleMemLen=memPos;
}

int DivPlatformGA20::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new unsigned char[getSampleMemCapacity()];
  sampleMemLen=0;
  delay=0;
  setFlags(flags);
  ga20BufLen=65536;
  for (int i=0; i<4; i++) ga20Buf[i]=new short[ga20BufLen];
  reset();
  
  return 4;
}

void DivPlatformGA20::quit() {
  delete[] sampleMem;
  for (int i=0; i<4; i++) {
    delete[] ga20Buf[i];
    delete oscBuf[i];
  }
}
