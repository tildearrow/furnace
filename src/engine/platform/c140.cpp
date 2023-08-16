/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "c140.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>
#include <map>

#define CHIP_FREQBASE 12582912

#define rWrite(a,v) {if(!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if(dumpWrites) addWrite(a,v); }}

const char* regCheatSheetC140[]={
  "CHx_RVol", "00+x*10",
  "CHx_LVol", "01+x*10",
  "CHx_FreqH", "02+x*10",
  "CHx_FreqL", "03+x*10",
  "CHx_Bank", "04+x*10",
  "CHx_Ctrl", "05+x*10",
  "CHx_StartH", "06+x*10",
  "CHx_StartL", "07+x*10",
  "CHx_EndH", "08+x*10",
  "CHx_EndL", "09+x*10",
  "CHx_LoopH", "0A+x*10",
  "CHx_LoopL", "0B+x*10",
  "Timer", "1FA",
  "IRQ", "1FE",
  NULL
};

const char** DivPlatformC140::getRegisterSheet() {
  return regCheatSheetC140;
}

void DivPlatformC140::acquire(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      c140_write(&c140, w.addr,w.val);
      regPool[w.addr&0x1ff]=w.val;
      writes.pop();
    }

    c140_tick(&c140, 1);
    // scale as 16bit
    c140.lout >>= 10;
    c140.rout >>= 10;

    if (c140.lout<-32768) c140.lout=-32768;
    if (c140.lout>32767) c140.lout=32767;

    if (c140.rout<-32768) c140.rout=-32768;
    if (c140.rout>32767) c140.rout=32767;
  
    buf[0][h]=c140.lout;
    buf[1][h]=c140.rout;

    for (int i=0; i<24; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(c140.voice[i].lout+c140.voice[i].rout)>>10;
    }
  }
}

void DivPlatformC140::tick(bool sysTick) {
  for (int i=0; i<24; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol*MIN(chan[i].macroVolMul,chan[i].std.vol.val))/chan[i].macroVolMul;
      chan[i].volChangedL=true;
      chan[i].volChangedR=true;
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
    if (chan[i].std.panL.had) {
      chan[i].chPanL=(255*(chan[i].std.panL.val&255))/chan[i].macroPanMul;
      chan[i].volChangedL=true;
    }

    if (chan[i].std.panR.had) {
      chan[i].chPanR=(255*(chan[i].std.panR.val&255))/chan[i].macroPanMul;
      chan[i].volChangedR=true;
    }
    
    if (chan[i].std.phaseReset.had) {
      if ((chan[i].std.phaseReset.val==1) && chan[i].active) {
        chan[i].audPos=0;
        chan[i].setPos=true;
      }
    }
    if (chan[i].volChangedL) {
      chan[i].chVolL=(chan[i].outVol*chan[i].chPanL)/255;
      rWrite(1+(i<<4),chan[i].chVolL);
      chan[i].volChangedL=false;
    }
    if (chan[i].volChangedR) {
      chan[i].chVolR=(chan[i].outVol*chan[i].chPanR)/255;
      rWrite(0+(i<<4),chan[i].chVolR);
      chan[i].volChangedR=false;
    }
    if (chan[i].setPos) {
      // force keyon
      chan[i].keyOn=true;
      chan[i].setPos=false;
    } else {
      chan[i].audPos=0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      bool writeCtrl=false;
      DivSample* s=parent->getSample(chan[i].sample);
      unsigned char ctrl=0;
      double off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
      chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      ctrl|=(chan[i].active?0x80:0)|((s->isLoopable())?0x10:0)|((s->depth==DIV_SAMPLE_DEPTH_MULAW)?0x08:0);
      if (chan[i].keyOn) {
        unsigned int bank=0;
        unsigned int start=0;
        unsigned int loop=0;
        unsigned int end=0;
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          bank=(sampleOff[chan[i].sample]>>16)&0xff;
          start=sampleOff[chan[i].sample]&0xffff;
          end=MIN(start+s->length8-1,65535);
        }
        if (chan[i].audPos>0) {
          start=MIN(start+MIN(chan[i].audPos,s->length8),65535);
        }
        if (s->isLoopable()) {
          loop=MIN(start+s->loopStart,65535);
          end=MIN(start+s->loopEnd-1,65535);
        }
        rWrite(0x05+(i<<4),0); // force keyoff first
        rWrite(0x04+(i<<4),bank);
        rWrite(0x06+(i<<4),(start>>8)&0xff);
        rWrite(0x07+(i<<4),start&0xff);
        rWrite(0x08+(i<<4),(end>>8)&0xff);
        rWrite(0x09+(i<<4),end&0xff);
        rWrite(0x0a+(i<<4),(loop>>8)&0xff);
        rWrite(0x0b+(i<<4),loop&0xff);
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
          chan[i].volChangedL=true;
          chan[i].volChangedR=true;
        }
        writeCtrl=true;
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        writeCtrl=true;
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        rWrite(0x02+(i<<4),chan[i].freq>>8);
        rWrite(0x03+(i<<4),chan[i].freq&0xff);
        chan[i].freqChanged=false;
      }
      if (writeCtrl) {
        rWrite(0x05+(i<<4),ctrl);
      }
    }
  }
}

int DivPlatformC140::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
      chan[c.chan].macroPanMul=ins->type==DIV_INS_AMIGA?127:255;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].sample=ins->amiga.getSample(c.value);
        c.value=ins->amiga.getFreq(c.value);
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
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
        chan[c.chan].volChangedL=true;
        chan[c.chan].volChangedR=true;
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
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      chan[c.chan].volChangedL=true;
      chan[c.chan].volChangedR=true;
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].chPanL=c.value;
      chan[c.chan].chPanR=c.value2;
      chan[c.chan].volChangedL=true;
      chan[c.chan].volChangedR=true;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
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
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
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

void DivPlatformC140::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  c140.voice[ch].muted=mute;
}

void DivPlatformC140::forceIns() {
  for (int i=0; i<24; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].volChangedL=true;
    chan[i].volChangedR=true;
    chan[i].sample=-1;
  }
}

void* DivPlatformC140::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformC140::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformC140::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformC140::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,512);
  c140_reset(&c140);
  for (int i=0; i<24; i++) {
    chan[i]=DivPlatformC140::Channel();
    chan[i].std.setEngine(parent);
    rWrite(0x05+(i<<4),0);
  }
}

int DivPlatformC140::getOutputCount() {
  return 2;
}

void DivPlatformC140::notifyInsChange(int ins) {
  for (int i=0; i<24; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformC140::notifyWaveChange(int wave) {
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformC140::notifyInsDeletion(void* ins) {
  for (int i=0; i<24; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformC140::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformC140::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

unsigned char* DivPlatformC140::getRegisterPool() {
  return regPool;
}

int DivPlatformC140::getRegisterPoolSize() {
  return 512;
}

float DivPlatformC140::getPostAmp() {
  return 3.0f;
}

const void* DivPlatformC140::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformC140::getSampleMemCapacity(int index) {
  return index == 0 ? 16777216 : 0;
}

size_t DivPlatformC140::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformC140::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformC140::renderSamples(int sysID) {
  memset(sampleMem,0,getSampleMemCapacity());
  memset(sampleOff,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }

    unsigned int length=s->length16;
    // fit sample size to single bank size
    if (length>(131072)) {
      length=131072;
    }
    if ((memPos&0xfe0000)!=((memPos+length)&0xfe0000)) {
      memPos=((memPos+0x1ffff)&0xfe0000);
    }
    if (memPos>=(getSampleMemCapacity())) {
      logW("out of C140 memory for sample %d!",i);
      break;
    }
    // why is C140 not G.711-compliant? this weird bit mangling had me puzzled for 3 hours...
    if (memPos+length>=(getSampleMemCapacity())) {
      if (s->depth==DIV_SAMPLE_DEPTH_MULAW) {
        for (unsigned int i=0; i<(getSampleMemCapacity())-memPos; i++) {
          unsigned char x=s->dataMuLaw[i]^0xff;
          if (x&0x80) x^=15;
          unsigned char c140Mu=(x&0x80)|((x&15)<<3)|((x&0x70)>>4);
          sampleMem[i+(memPos/sizeof(short))]=((c140Mu)<<8);
        }
      } else {
        memcpy(sampleMem+(memPos/sizeof(short)),s->data16,(getSampleMemCapacity())-memPos);
      }
      logW("out of C140 memory for sample %d!",i);
    } else {
      if (s->depth==DIV_SAMPLE_DEPTH_MULAW) {
        for (unsigned int i=0; i<length; i++) {
          unsigned char x=s->dataMuLaw[i]^0xff;
          if (x&0x80) x^=15;
          unsigned char c140Mu=(x&0x80)|((x&15)<<3)|((x&0x70)>>4);
          sampleMem[i+(memPos/sizeof(short))]=((c140Mu)<<8);
        }
      } else {
        memcpy(sampleMem+(memPos/sizeof(short)),s->data16,length);
      }
    }
    sampleOff[i]=memPos>>1;
    sampleLoaded[i]=true;
    memPos+=length;
  }
  sampleMemLen=memPos+256;
}

void DivPlatformC140::setFlags(const DivConfig& flags) {
  chipClock=32000*256; // 8.192MHz and 12.288MHz input, verified from Assault Schematics
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/192;
  for (int i=0; i<24; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformC140::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<24; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new short[getSampleMemCapacity()>>1];
  sampleMemLen=0;
  c140_init(&c140);
  c140.sample_mem=sampleMem;
  setFlags(flags);
  reset();

  return 24;
}

void DivPlatformC140::quit() {
  delete[] sampleMem;
  for (int i=0; i<24; i++) {
    delete oscBuf[i];
  }
}
