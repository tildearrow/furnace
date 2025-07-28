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

#include "supervision.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "furIcons.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 32

const char* regCheatSheetSupervision[]={
  "Freq0L", "10",
  "Freq0H", "11",
  "Control0", "12",
  "Len0", "13",

  "Freq1L", "14",
  "Freq1H", "15",
  "Control1", "16",
  "Len1", "17",

  "DMAstartL", "18",
  "DMAstartH", "19",
  "DMAlen", "1a",
  "DMAstep", "1b",
  "DMAon", "1c",

  "NoisDiv", "28",
  "NoisLen", "29",
  "NoisCtrl", "2a",

  NULL
};

const char** DivPlatformSupervision::getRegisterSheet() {
  return regCheatSheetSupervision;
}

void DivPlatformSupervision::acquire(short** buf, size_t len) {
  int mask_bits=0;
  for (int i=0; i<4; i++) {
    mask_bits |= isMuted[i]?0:8>>i;
  }
  supervision_set_mute_mask(&svision,mask_bits);

  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      supervision_memorymap_registers_write(&svision,w.addr|0x2000,w.val);
      regPool[w.addr&0x3f]=w.val;
      writes.pop();
    }

    unsigned char s[6];
    supervision_sound_stream_update(&svision,s,2);
    tempL[0]=(((int)s[0])-128)*256;
    tempR[0]=(((int)s[1])-128)*256;

    for (int i=0; i<4; i++) {
      oscBuf[i]->putSample(h,CLAMP(((int)s[2+i])<<8,-32768,32767));
    }

    tempL[0]=(tempL[0]>>1)+(tempL[0]>>2);
    tempR[0]=(tempR[0]>>1)+(tempR[0]>>2);

    if (tempL[0]<-32768) tempL[0]=-32768;
    if (tempL[0]>32767) tempL[0]=32767;
    if (tempR[0]<-32768) tempR[0]=-32768;
    if (tempR[0]>32767) tempR[0]=32767;
    
    //printf("tempL: %d tempR: %d\n",tempL,tempR);
    buf[0][h]=tempL[0];
    buf[1][h]=tempR[0];
  }

  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSupervision::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        int f=parent->calcArp(chan[i].note,chan[i].std.arp.val);
        if (i==2 || i==3) {
          chan[i].baseFreq=f;
          //if (chan[i].baseFreq>255) chan[i].baseFreq=255;
          if (chan[i].baseFreq<0) chan[i].baseFreq=0;
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(f);
        }
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
    }
    if (chan[i].std.panL.had) {
      chan[i].pan=chan[i].std.panL.val&3;
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
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_PCE);
      if (i<2) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock<<1,CHIP_DIVIDER);
        if (chan[i].freq<1) chan[i].freq=1;
        if (chan[i].freq>2047) chan[i].freq=2047;
        if (chan[i].freqChanged || chan[i].initWrite) {
          rWrite(0x10|(i<<2),chan[i].freq&0xff);
          rWrite(0x11|(i<<2),(chan[i].freq>>8)&0x7);
        }
        chan[i].initWrite=false;
      } else if (i==3) {
        int ntPos=chan[i].baseFreq;
        if (NEW_ARP_STRAT) {
          if (chan[i].fixedArp) {
            ntPos=chan[i].baseNoteOverride;
          } else {
            ntPos+=chan[i].arpOff;
          }
        }
        ntPos+=chan[i].pitch2;
        chan[i].freq=15-(ntPos&15);
        unsigned char r=(chan[i].freq<<4)|(chan[i].outVol&0xf);
        rWrite(0x28,r);
        noiseReg[0]=r;
        rWrite(0x29,0xc8);
        r=((chan[i].duty&1)^dutySwap)|(0x02|0x10)|(chan[i].pan<<2);
        rWrite(0x2A,r);
      }
      if (chan[i].keyOn && i==2) {
        if (chan[i].pcm) {
          int ntPos=chan[i].sampleNote;
          ntPos+=chan[i].pitch2;
          chan[i].freq=3-(ntPos&3);
          int sNum=chan[i].sample;
          DivSample* sample=parent->getSample(sNum);
          if (sample!=NULL && sNum>=0 && sNum<parent->song.sampleLen) {
            unsigned int off=MIN(sampleOff[sNum]+chan[i].hasOffset,sampleOff[sNum]+sampleLen[sNum]);
            unsigned int len=MAX((((int)sampleLen[sNum])-((int)chan[i].hasOffset)),0);
            if (len) {
              rWrite(0x18,off&0xff);
              rWrite(0x19,(off>>8&0x3f)|0x80);
              rWrite(0x1A,MIN(MAX(len>>4,0),255));
              rWrite(0x1B,chan[i].freq|((chan[i].pan&3)<<2)|((off>>14&7)<<4));
              rWrite(0x1C,0x80);
            }
            sampleOffset=chan[i].hasOffset;
            chan[i].hasOffset=0;
          }
        }
      }
      if (chan[i].keyOff && i==2) {
        if (chan[i].pcm) {
          int ntPos=chan[i].sampleNote;
          ntPos+=chan[i].pitch2;
          chan[i].freq=3-(ntPos&3);
          int sNum=chan[i].sample;
          DivSample* sample=parent->getSample(sNum);
          if (sample!=NULL && sNum>=0 && sNum<parent->song.sampleLen) {
            rWrite(0x1C,0x00);
          }
        }
      }
      if (chan[i].keyOn) chan[i].kon=true;
      if (chan[i].keyOff) chan[i].kon=false;
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }

    if (chan[i].kon) {
      if (i<2) {
        rWrite(0x12|(i<<2),(chan[i].outVol&0xf)|((chan[i].duty&3)<<4));
        rWrite(0x13|(i<<2),0xc8);
      } else if (i == 3) {
        int ntPos=chan[i].baseFreq;
        if (NEW_ARP_STRAT) {
          if (chan[i].fixedArp) {
            ntPos=chan[i].baseNoteOverride;
          } else {
            ntPos+=chan[i].arpOff;
          }
        }
        ntPos+=chan[i].pitch2;
        chan[i].freq=15-(ntPos&15);
        unsigned char r=(chan[i].freq<<4)|(chan[i].outVol&0xf);
        if (noiseReg[0] != r) rWrite(0x28,r);
        noiseReg[0]=r;
        rWrite(0x29,0xc8);
        r=((chan[i].duty&1)^dutySwap)|(0x02|0x10)|(chan[i].pan<<2);
        if (noiseReg[2] != r) rWrite(0x2A,r);
        noiseReg[2]=r;
      } else if (i==2) {
        if (chan[i].pcm) {
          int ntPos=chan[i].sampleNote;
          ntPos+=chan[i].pitch2;
          chan[i].freq=3-(ntPos&3);
          int sNum=chan[i].sample;
          DivSample* sample=parent->getSample(sNum);
          if (sample!=NULL && sNum>=0 && sNum<parent->song.sampleLen) {
            unsigned int off=MIN(sampleOff[sNum]+sampleOffset,sampleOff[sNum]+sampleLen[sNum]);
            unsigned int len=MAX((((int)sampleLen[sNum])-((int)sampleOffset)),0);
            if (len) {
              rWrite(0x1A,MIN(MAX(len>>4,0),255));
              if (chan[i].outVol==0) {
                rWrite(0x1B,chan[i].freq|((off>>14&7)<<4));
              } else {
                rWrite(0x1B,chan[i].freq|((chan[i].pan&3)<<2)|((off>>14&7)<<4));
              }
            }
          }
        }
      }
    } else {
      if (i < 2) {
        rWrite(0x12|(i<<2),0);
        rWrite(0x13|(i<<2),0xc8);
      } else if (i == 3) {
        rWrite(0x29,0);
        unsigned char r=0;
        if (noiseReg[2] != r) rWrite(0x2A,r);
        noiseReg[2]=r;
      }
    }

  }
}

int DivPlatformSupervision::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SUPERVISION);
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
        chan[c.chan].pcm=true;
      } else {
        chan[c.chan].pcm=false;
      }
      if (chan[c.chan].pcm) {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          if (c.chan==2) {
            c.value=ins->amiga.getFreq(c.value);
            chan[c.chan].sampleNote=c.value;
          }
        }
      } else {
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=c.chan==3?c.value:NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      //chwrite(c.chan,0x04,0x80|chan[c.chan].vol);
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
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
          if (chan[c.chan].active) {
            //chwrite(c.chan,0x04,0x80|chan[c.chan].outVol);
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
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].hasOffset=c.value;
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=0;
      if (c.value&0xf0) chan[c.chan].pan|=2;
      if (c.value2>>4) chan[c.chan].pan|=1;
      if (chan[c.chan].pan==0) chan[c.chan].pan=3;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SUPERVISION));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSupervision::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformSupervision::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    //chwrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
}

void* DivPlatformSupervision::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSupervision::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSupervision::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSupervision::getRegisterPool() {
  return regPool;
}

int DivPlatformSupervision::getRegisterPoolSize() {
  return 64;
}

void DivPlatformSupervision::reset() {
  writes.clear();
  memset(regPool,0,64);
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformSupervision::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  supervision_sound_reset(&svision);
  memset(tempL,0,32*sizeof(int));
  memset(tempR,0,32*sizeof(int));
  memset(noiseReg,0,3*sizeof(unsigned char));
  noiseReg[2]=0xff;
  sampleOffset=0;
}

int DivPlatformSupervision::getOutputCount() {
  return 2;
}

bool DivPlatformSupervision::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformSupervision::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSupervision::setFlags(const DivConfig& flags) {
  if (flags.getInt("swapDuty",true)) {
    dutySwap=1;
  } else {
    dutySwap=0;
  }
  otherFlags=0;
  if (flags.getInt("sqStereo",false)) {
    otherFlags |= 1;
  }
  chipClock=4000000;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/64;
  for (int i=0; i<4; i++) {
    oscBuf[i]->setRate(rate);
  }
  supervision_sound_set_clock(&svision,(unsigned int)chipClock);
  supervision_sound_set_flags(&svision,(unsigned int)otherFlags);
}

void DivPlatformSupervision::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSupervision::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const void* DivPlatformSupervision::getSampleMem(int index) {
  return index==0?sampleMem:NULL;
}

size_t DivPlatformSupervision::getSampleMemCapacity(int index) {
  return index==0?65536:0;
}

size_t DivPlatformSupervision::getSampleMemUsage(int index) {
  return index==0?sampleMemLen:0;
}

bool DivPlatformSupervision::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>32767) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformSupervision::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformSupervision::renderSamples(int sysID) {
  memset(sampleMem,0,getSampleMemCapacity(0));
  memset(sampleLoaded,0,32768*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample Memory";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }
    unsigned int paddedLen=((s->length8>>1)+31)&(~0x1f);
    logV("%d padded length: %d",i,paddedLen);
    if (paddedLen>=4096) {
      paddedLen=4096;
    }
    if ((memPos&(~0x3fff))!=((memPos+paddedLen)&(~0x3fff))) {
      memPos=(memPos+0x3fff)&(~0x3fff);
    }
    if (memPos>=getSampleMemCapacity(0)) {
      logW("out of memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=getSampleMemCapacity(0)) {
      sampleLen[i]=(getSampleMemCapacity(0)-memPos)>>1;
      for (size_t i=0; i<(getSampleMemCapacity(0)-memPos)>>1; i++) {
          sampleMem[memPos+i]=(((s->data8[i*2+0]+128)>>4)<<4&0xf0)|(((s->data8[i*2+1]+128)>>4)&0xf);
      }
      logW("out of memory for sample %d!",i);
    } else {
      size_t len=MIN((s->length8>>1),paddedLen);
      sampleLen[i]=(unsigned int)(len);
      for (size_t i=0; i<len; i++) {
          sampleMem[memPos+i]=(((s->data8[i*2+0]+128)>>4)<<4&0xf0)|(((s->data8[i*2+1]+128)>>4)&0xf);
      }
      sampleLoaded[i]=true;
    }
    sampleOff[i]=memPos;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
    memPos+=paddedLen;
  }
  sampleMemLen=memPos;

  memCompo.capacity=65536;
  memCompo.used=sampleMemLen;
}

bool DivPlatformSupervision::getDCOffRequired() {
  return true;
}

int DivPlatformSupervision::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=svision.supervision_dma_mem;
  dutySwap=0;
  otherFlags=0;
  sampleOffset=0;
  sampleMemLen=0;
  memset(sampleMem,0,65536);
  svision.ch_mask=15;
  svision.flags=1;
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformSupervision::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
}

// initialization of important arrays
DivPlatformSupervision::DivPlatformSupervision() {
  sampleOff=new unsigned int[32768];
  sampleLoaded=new bool[32768];
}

DivPlatformSupervision::~DivPlatformSupervision() {
  delete[] sampleOff;
  delete[] sampleLoaded;
}
