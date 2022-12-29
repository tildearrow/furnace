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

#include "k007232.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define rWrite(a,v) {if(!skipRegisterWrites) {writes.emplace(a,v); if(dumpWrites) addWrite(a,v);}}

#define CHIP_DIVIDER 64

const char* regCheatSheetK007232[]={
  // on-chip
  "CHX_FreqL", "X*6+0",
  "CHX_FreqH", "X*6+1",
  "CHX_StartL", "X*6+2",
  "CHX_StartM", "X*6+3",
  "CHX_StartH", "X*6+4",
  "CHX_Keyon", "X*6+5",
  "SLEV", "C", // external IO
  "Loop", "D",
  // off-chip
  "CHX_Volume", "X*2+10",
  "CHX_Bank", "X*2+12",
  NULL
};

const char** DivPlatformK007232::getRegisterSheet() {
  return regCheatSheetK007232;
}

inline void DivPlatformK007232::chWrite(unsigned char ch, unsigned int addr, unsigned char val) {
  if (!skipRegisterWrites) {
    if ((ch<2) && (addr<6)) {
      rWrite((ch*6)+(addr&7),val);
    }
  }
}

void DivPlatformK007232::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    if ((--delay)<=0) {
      delay=MAX(0,delay);
      if (!writes.empty()) {
        QueuedWrite& w=writes.front();
        // write on-chip register
        if (w.addr<=0xd) {
          k007232.write(w.addr,w.val);
        }
        regPool[w.addr]=w.val;
        writes.pop();
        delay=w.delay;
      }
    }

    k007232.tick();

    if (stereo) {
      const unsigned char vol1=regPool[0x10],vol2=regPool[0x11];
      const signed int lout[2]={(k007232.output(0)*(vol1&0xf)),(k007232.output(1)*(vol2&0xf))};
      const signed int rout[2]={(k007232.output(0)*((vol1>>4)&0xf)),(k007232.output(1)*((vol2>>4)&0xf))};
      bufL[h]=(lout[0]+lout[1])<<4;
      bufR[h]=(rout[0]+rout[1])<<4;
      for (int i=0; i<2; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=(lout[i]+rout[i])<<4;
      }
    } else {
      const unsigned char vol=regPool[0xc];
      const signed int out[2]={(k007232.output(0)*(vol&0xf)),(k007232.output(1)*((vol>>4)&0xf))};
      bufL[h]=bufR[h]=(out[0]+out[1])<<4;
      for (int i=0; i<2; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=out[i]<<5;
      }
    }
  }
}

u8 DivPlatformK007232::read_sample(u8 ne, u32 address) {
  if ((sampleMem!=NULL) && (address<getSampleMemCapacity())) {
    return sampleMem[((regPool[0x12+(ne&1)]<<17)|(address&0x1ffff))&0xffffff];
  }
  return 0;
}

void DivPlatformK007232::tick(bool sysTick) {
  for (int i=0; i<2; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      const signed char macroVol=((chan[i].vol&0xf)*MIN(chan[i].macroVolMul,chan[i].std.vol.val))/chan[i].macroVolMul;
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
    // volume and panning registers are off-chip
    if (chan[i].std.panL.had) {
      chan[i].panning&=0xf0;
      chan[i].panning|=chan[i].std.panL.val&15;
      if ((!isMuted[i]) && stereo) {
        chan[i].volumeChanged=true;
      }
    }
    if (chan[i].std.panR.had) {
      chan[i].panning&=0x0f;
      chan[i].panning|=(chan[i].std.panR.val&15)<<4;
      if ((!isMuted[i]) && stereo) {
        chan[i].volumeChanged=true;
      }
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].audPos=0;
        chan[i].setPos=true;
      }
    }
    if (chan[i].volumeChanged) {
      chan[i].resVol=isMuted[i]?0:chan[i].outVol&0xf;
      if (stereo) {
        chan[i].lvol=((chan[i].resVol&0xf)*((chan[i].panning>>0)&0xf))/15;
        chan[i].rvol=((chan[i].resVol&0xf)*((chan[i].panning>>4)&0xf))/15;
        const int newPan=(chan[i].lvol&0xf)|((chan[i].rvol&0xf)<<4);
        if (chan[i].prevPan!=newPan) {
          rWrite(0x10+i,(chan[i].lvol&0xf)|((chan[i].rvol&0xf)<<4));
          chan[i].prevPan=newPan;
        }
      }
      else {
        const unsigned char prevVolume=lastVolume;
        lastVolume=(lastVolume&~(0xf<<(i<<2)))|((chan[i].resVol&0xf)<<(i<<2));
        if (prevVolume!=lastVolume) {
          rWrite(0xc,lastVolume);
        }
      }
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
      chan[i].freq=0x1000-(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER));
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].keyOn) {
        unsigned int bank=0;
        unsigned int start=0;
        unsigned int loop=0;
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          bank=sampleOffK007232[chan[i].sample]>>17;
          start=sampleOffK007232[chan[i].sample]&0x1ffff;
          loop=start+s->length8;
        }
        if (chan[i].audPos>0) {
          start=start+MIN(chan[i].audPos,MIN(131072-1,s->length8));
        }
        start=MIN(start,MIN(getSampleMemCapacity(),131072)-1);
        loop=MIN(loop,MIN(getSampleMemCapacity(),131072)-1);
        // force keyoff first
        chWrite(i,2,0xff);
        chWrite(i,3,0xff);
        chWrite(i,4,0x1);
        chWrite(i,5,0);
        // keyon
        const unsigned char prevLoop=lastLoop;
        if (s->isLoopable()) {
          loop=start+s->loopStart;
          lastLoop|=(1<<i);
        } else {
          lastLoop&=~(1<<i);
        }
        if (prevLoop!=lastLoop) {
          rWrite(0xd,lastLoop);
        }
        if (chan[i].prevBank!=(int)bank) {
          rWrite(0x12+i,bank);
          chan[i].prevBank=bank;
        }
        if (chan[i].prevFreq!=chan[i].freq) {
          chWrite(i,0,chan[i].freq&0xff);
          chWrite(i,1,(chan[i].freq>>8)&0xf);
          chan[i].prevFreq=chan[i].freq;
        }
        chWrite(i,2,start&0xff);
        chWrite(i,3,start>>8);
        chWrite(i,4,start>>16);
        chWrite(i,5,0);
        if (s->isLoopable() && start!=loop) {
          chWrite(i,2,loop&0xff);
          chWrite(i,3,loop>>8);
          chWrite(i,4,loop>>16);
        }
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
          if (!isMuted[i]) {
            chan[i].volumeChanged=true;
          }
        }
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chWrite(i,2,0xff);
        chWrite(i,3,0xff);
        chWrite(i,4,0x1);
        chWrite(i,5,0);
        const unsigned char prevLoop=lastLoop;
        lastLoop&=~(1<<i);
        if (prevLoop!=lastLoop) {
          rWrite(0xd,lastLoop);
        }
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        if (chan[i].prevFreq!=chan[i].freq) {
          chWrite(i,0,chan[i].freq&0xff);
          chWrite(i,1,(chan[i].freq>>8)&0xf);
          chan[i].prevFreq=chan[i].freq;
        }
        chan[i].freqChanged=false;
      }
    }
  }
}

int DivPlatformK007232::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:15;
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
    case DIV_CMD_PANNING:
      chan[c.chan].panning=(c.value>>4)|(c.value2&0xf0);
      if (!isMuted[c.chan] && stereo) {
        chan[c.chan].volumeChanged=true;
      }
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
      return 15;
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

void DivPlatformK007232::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volumeChanged=true;
}

void DivPlatformK007232::forceIns() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<2; i++) {
    chan[i].insChanged=true;
    chan[i].volumeChanged=true;
    chan[i].freqChanged=true;
    chan[i].prevFreq=-1;
    chan[i].prevBank=-1;
  }
  lastLoop=0;
  lastVolume=0;
}

void* DivPlatformK007232::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformK007232::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformK007232::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformK007232::reset() {
  while (!writes.empty()) {
    writes.pop();
  }
  memset(regPool,0,20);
  k007232.reset();
  lastLoop=0;
  lastVolume=0;
  delay=0;
  for (int i=0; i<2; i++) {
    chan[i]=DivPlatformK007232::Channel();
    chan[i].std.setEngine(parent);
    // keyoff all channels
    chWrite(i,0,0);
    chWrite(i,1,0);
    chWrite(i,2,0xff);
    chWrite(i,3,0xff);
    chWrite(i,4,1);
    chWrite(i,5,0);
  }
}

bool DivPlatformK007232::isStereo() {
  return stereo;
}

void DivPlatformK007232::notifyInsChange(int ins) {
  for (int i=0; i<2; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformK007232::notifyWaveChange(int wave) {
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformK007232::notifyInsDeletion(void* ins) {
  for (int i=0; i<2; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformK007232::setFlags(const DivConfig& flags) {
  chipClock=COLOR_NTSC;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/4;
  stereo=flags.getBool("stereo",false);
  for (int i=0; i<2; i++) {
    oscBuf[i]->rate=rate;
  }
}

void DivPlatformK007232::poke(unsigned int addr, unsigned short val) {
  rWrite(addr&0x1f,val);
}

void DivPlatformK007232::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr&0x1f,i.val);
}

unsigned char* DivPlatformK007232::getRegisterPool() {
  return regPool;
}

int DivPlatformK007232::getRegisterPoolSize() {
  return 20;
}

const void* DivPlatformK007232::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformK007232::getSampleMemCapacity(int index) {
  return index == 0 ? 16777216 : 0;
}

size_t DivPlatformK007232::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformK007232::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformK007232::renderSamples(int sysID) {
  memset(sampleMem,0xc0,getSampleMemCapacity());
  memset(sampleOffK007232,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOffK007232[i]=0;
      continue;
    }

    const int length=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos)-2,length);
    if (actualLength>0) {
      if (actualLength>131072-2) {
        actualLength=131072-2;
      }
      if ((memPos&0xfe0000)!=((memPos+actualLength+1)&0xfe0000)) {
        memPos=(memPos+0x1ffff)&0xfe0000;
      }
      sampleOffK007232[i]=memPos;
      for (int j=0; j<actualLength; j++) {
        // convert to 7 bit unsigned
        unsigned char val=(unsigned char)(s->data8[j])^0x80;
        sampleMem[memPos++]=(val>>1)&0x7f;
      }
      // write end of sample marker
      memset(&sampleMem[memPos],0xc0,1);
      memPos+=1;
    }
    if ((memPos+MAX(actualLength,0))>=(getSampleMemCapacity()-1)) {
      logW("out of K007232 PCM memory for sample %d!",i);
      break;
    } else {
      sampleLoaded[i]=true;
    }
  }
  sampleMemLen=memPos;
}

int DivPlatformK007232::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<2; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new unsigned char[getSampleMemCapacity()];
  sampleMemLen=0;
  setFlags(flags);
  reset();
  
  return 2;
}

void DivPlatformK007232::quit() {
  delete[] sampleMem;
  for (int i=0; i<2; i++) {
    delete oscBuf[i];
  }
}
