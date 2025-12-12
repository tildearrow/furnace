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

#include "rf5c68.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define rWrite(a,v) {if(!skipRegisterWrites) {rf5c68.rf5c68_w(a,v); regPool[a]=v; if(dumpWrites) addWrite(a,v);}}

#define CHIP_FREQBASE 786432

const char* regCheatSheetRF5C68[]={
  "Volume", "0",
  "Panning", "1",
  "FreqL", "2",
  "FreqH", "3",
  "LoopStartL", "4",
  "LoopStartH", "5",
  "StartH", "6",
  "Control", "7",
  "Disable", "8",
  NULL
};

const char** DivPlatformRF5C68::getRegisterSheet() {
  return regCheatSheetRF5C68;
}

void DivPlatformRF5C68::chWrite(unsigned char ch, unsigned int addr, unsigned char val) {
  if (!skipRegisterWrites) {
    if (curChan!=ch) {
      curChan=ch;
      rWrite(7,curChan|0xc0);
    }
    regPool[16+((ch)<<4)+((addr)&0x0f)]=val;
    rWrite(addr,val);
  }
}

// TODO: this code is weird
//       make sure newDispatch didn't break it up
void DivPlatformRF5C68::acquire(short** buf, size_t len) {
  short bufC[16][256];
  short* chBufPtrs[16]={
    bufC[0],bufC[1],bufC[2],bufC[3],bufC[4],bufC[5],bufC[6],bufC[7],
    bufC[8],bufC[9],bufC[10],bufC[11],bufC[12],bufC[13],bufC[14],bufC[15]
  };
  size_t pos=0;

  for (int i=0; i<16; i++) {
    memset(bufC[i],0,256*sizeof(short));
  }

  for (int i=0; i<8; i++) {
    oscBuf[i]->begin(len);
  }

  size_t lenCopy=len;
  while (lenCopy > 0) {
    size_t blockLen=MIN(lenCopy,256);
    short* bufPtrs[2]={&buf[0][pos],&buf[1][pos]};
    rf5c68.sound_stream_update(bufPtrs,chBufPtrs,blockLen);
    for (int i=0; i<8; i++) {
      for (size_t j=0; j<blockLen; j++) {
        oscBuf[i]->putSample(pos+j,(bufC[i*2][j]+bufC[i*2+1][j])>>1);
      }
    }
    pos+=blockLen;
    lenCopy-=blockLen;
  }

  for (int i=0; i<8; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformRF5C68::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&0xff)*MIN(chan[i].macroVolMul,chan[i].std.vol.val))/chan[i].macroVolMul;
      chWrite(i,0,chan[i].outVol);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
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
    // panning registers are reversed in this chip
    if (chan[i].std.panL.had) {
      chan[i].panning&=0xf0;
      chan[i].panning|=chan[i].std.panL.val&15;
    }
    if (chan[i].std.panR.had) {
      chan[i].panning&=0x0f;
      chan[i].panning|=(chan[i].std.panR.val&15)<<4;
    }
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      chWrite(i,1,isMuted[i]?0:chan[i].panning);
    }
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].audPos=0;
        chan[i].setPos=true;
      }
    }
    if (chan[i].setPos) {
      // force keyon
      chan[i].keyOn=true;
      chan[i].setPos=false;
    } else {
      chan[i].audPos=0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      unsigned char keyon=regPool[8]&~(1<<i);
      unsigned char keyoff=keyon|(1<<i);
      DivSample* s=parent->getSample(chan[i].sample);
      double off=(s->centerRate>=1)?((double)s->centerRate/parent->getCenterRate()):1.0;
      chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].keyOn) {
        unsigned int start=0;
        unsigned int loop=0;
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          start=sampleOffRFC[chan[i].sample];
          loop=start+s->length8;
        }
        if (chan[i].audPos>0) {
          start=start+MIN(chan[i].audPos,s->length8);
        }
        if (s->isLoopable()) {
          loop=start+s->loopStart;
        }
        start=MIN(start,getSampleMemCapacity()-32);
        loop=MIN(loop,getSampleMemCapacity()-32);
        rWrite(8,keyoff); // force keyoff first
        chWrite(i,6,start>>8);
        chWrite(i,4,loop&0xff);
        chWrite(i,5,loop>>8);
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
          chWrite(i,0,chan[i].outVol);
        }
        rWrite(8,keyon);
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        rWrite(8,keyoff);
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        chWrite(i,2,chan[i].freq&0xff);
        chWrite(i,3,chan[i].freq>>8);
        chan[i].freqChanged=false;
      }
    }
  }
}

int DivPlatformRF5C68::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].sample=ins->amiga.getSample(c.value);
        chan[c.chan].sampleNote=c.value;
        c.value=ins->amiga.getFreq(c.value);
        chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
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
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
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
          chWrite(c.chan,0,chan[c.chan].outVol);
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
      chWrite(c.chan,1,isMuted[c.chan]?0:chan[c.chan].panning);
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2+chan[c.chan].sampleNoteDelta);
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
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformRF5C68::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chWrite(ch,1,mute?0:chan[ch].panning);
}

void DivPlatformRF5C68::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;
    chWrite(i,1,isMuted[i]?0:chan[i].panning);
  }
}

void* DivPlatformRF5C68::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformRF5C68::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformRF5C68::getPan(int ch) {
  return ((chan[ch].panning&15)<<8)|((chan[ch].panning&0xf0)>>4);
}

DivDispatchOscBuffer* DivPlatformRF5C68::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformRF5C68::reset() {
  memset(regPool,0,144);
  rf5c68.device_reset();
  rWrite(0x08,0xff); // keyoff all channels
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformRF5C68::Channel();
    chan[i].std.setEngine(parent);
    chWrite(i,0,255);
    chWrite(i,1,isMuted[i]?0:255);
  }
}

int DivPlatformRF5C68::getOutputCount() {
  return 2;
}

void DivPlatformRF5C68::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformRF5C68::notifyWaveChange(int wave) {
}

void DivPlatformRF5C68::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformRF5C68::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 1: chipClock=10000000; break;
    case 2: chipClock=12500000; break;
    default: chipClock=8000000; break;
  }
  CHECK_CUSTOM_CLOCK;
  chipType=flags.getInt("chipType",0);
  rate=chipClock/384;
  for (int i=0; i<8; i++) {
    oscBuf[i]->setRate(rate);
  }
  rf5c68=(chipType==1)?rf5c164_device():rf5c68_device();
  rf5c68.device_start(sampleMem);
}

void DivPlatformRF5C68::poke(unsigned int addr, unsigned short val) {
  rWrite(addr&0x0f,val);
}

void DivPlatformRF5C68::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr&0x0f,i.val);
}

unsigned char* DivPlatformRF5C68::getRegisterPool() {
  return regPool;
}

int DivPlatformRF5C68::getRegisterPoolSize() {
  return 144;
}

size_t DivPlatformRF5C68::getSampleMemNum() {
  return 1;
}

const void* DivPlatformRF5C68::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformRF5C68::getSampleMemCapacity(int index) {
  return index == 0 ? 65536 : 0;
}

size_t DivPlatformRF5C68::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformRF5C68::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>32767) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformRF5C68::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformRF5C68::renderSamples(int sysID) {
  memset(sampleMem,0,getSampleMemCapacity());
  memset(sampleOffRFC,0,32768*sizeof(unsigned int));
  memset(sampleLoaded,0,32768*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample Memory";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOffRFC[i]=0;
      continue;
    }

    int length=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos)-32,length);
    if (actualLength>0) {
      sampleOffRFC[i]=memPos;
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+actualLength+32));
      for (int j=0; j<actualLength; j++) {
        // convert to signed magnitude
        signed char val=s->data8[j];
        CLAMP_VAR(val,-127,126);
        sampleMem[memPos++]=(val>0)?(val|0x80):(0-val);
      }
      // write end of sample marker
      memset(&sampleMem[memPos],0xff,32);
      memPos+=32;
    }
    if (actualLength<length) {
      logW("out of RF5C68 PCM memory for sample %d!",i);
      break;
    }
    // align memPos to 256-byte boundary
    memPos=(memPos+0xff)&~0xff;
    sampleLoaded[i]=true;
  }
  sampleMemLen=memPos;

  memCompo.used=sampleMemLen;
  memCompo.capacity=getSampleMemCapacity(0);
}

int DivPlatformRF5C68::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new unsigned char[getSampleMemCapacity()];
  sampleMemLen=0;
  setFlags(flags);
  reset();
  
  return 8;
}

void DivPlatformRF5C68::quit() {
  delete[] sampleMem;
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}

// initialization of important arrays
DivPlatformRF5C68::DivPlatformRF5C68() {
  sampleOffRFC=new unsigned int[32768];
  sampleLoaded=new bool[32768];
}

DivPlatformRF5C68::~DivPlatformRF5C68() {
  delete[] sampleOffRFC;
  delete[] sampleLoaded;
}
