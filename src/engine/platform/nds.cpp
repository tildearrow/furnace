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

#include "nds.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define CHIP_DIVIDER 32

#define rRead8(a) (nds.read8(a))
#define rWrite8(a,v) {if(!skipRegisterWrites) {nds.write8((a),(v)); regPool[(a)]=(v); if(dumpWrites) addWrite((a),(v)); }}
#define rWrite16(a,v) { \
  if(!skipRegisterWrites) { \
    nds.write16((a)>>1,(v)); \
    regPool[(a)+0]=(v)&0xff; \
    regPool[(a)+1]=((v)>>8)&0xff; \
    if(dumpWrites) addWrite((a)+0,(v)&0xff); \
    if(dumpWrites) addWrite((a)+1,((v)>>8)&0xff); \
  } \
}

#define rWrite32(a,v) { \
  if(!skipRegisterWrites) { \
    nds.write32((a)>>2,(v)); \
    regPool[(a)+0]=(v)&0xff; \
    regPool[(a)+1]=((v)>>8)&0xff; \
    regPool[(a)+2]=((v)>>16)&0xff; \
    regPool[(a)+3]=((v)>>24)&0xff; \
    if(dumpWrites) addWrite((a)+0,(v)&0xff); \
    if(dumpWrites) addWrite((a)+1,((v)>>8)&0xff); \
    if(dumpWrites) addWrite((a)+2,((v)>>16)&0xff); \
    if(dumpWrites) addWrite((a)+3,((v)>>24)&0xff); \
  } \
}

const char* regCheatSheetNDS[]={
  "CHx_Control", "000+x*10",
  "CHx_Start", "004+x*10",
  "CHx_Freq", "008+x*10",
  "CHx_LoopStart", "00A+x*10",
  "CHx_Length", "00C+x*10",
  "Control", "100",
  "Bias", "104",
  "CAPx_Control", "108+x*1",
  "CAPx_Dest", "110+x*8",
  "CAPx_Length", "114+x*8",
  NULL
};

const char** DivPlatformNDS::getRegisterSheet() {
  return regCheatSheetNDS;
}

void DivPlatformNDS::acquireDirect(blip_buffer_t** bb, size_t len) {
  for (int i=0; i<16; i++) {
    oscBuf[i]->begin(len);
  }

  nds.set_bb(bb[0],bb[1]);
  nds.set_oscbuf(oscBuf);
  nds.resetTS(0);
  nds.tick(len);

  for (int i=0; i<16; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformNDS::postProcess(short* buf, int outIndex, size_t len, int sampleRate) {
  // this is where we handle global volume. it is faster than doing it on each blip...
  for (size_t i=0; i<len; i++) {
    buf[i]=((buf[i]*globalVolume)>>7);
  }
}

u8 DivPlatformNDS::read_byte(u32 addr) {
  if (addr<getSampleMemCapacity()) {
    return sampleMem[addr];
  }
  return 0;
}

void DivPlatformNDS::write_byte(u32 addr, u8 data) {
  if (addr<getSampleMemCapacity()) {
    sampleMem[addr]=data;
  }
}

void DivPlatformNDS::tick(bool sysTick) {
  for (int i=0; i<16; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol&0x7f)*MIN(chan[i].macroVolMul,chan[i].std.vol.val))/chan[i].macroVolMul;
      writeOutVol(i);
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
    if ((i>=8) && (i<14)) {
      if (chan[i].std.duty.had) {
        chan[i].duty=chan[i].std.duty.val;
        if ((!chan[i].pcm)) { // pulse
          rWrite8(0x03+i*16,(rRead8(0x03+i*16)&0xe8)|(chan[i].duty&7));
        }
      }
    }
    if (chan[i].std.panL.had) { // panning
      chan[i].panning=0x40+chan[i].std.panL.val;
      rWrite8(0x02+i*16,chan[i].panning);
    }
    if (chan[i].std.phaseReset.had) {
      if ((chan[i].std.phaseReset.val==1) && chan[i].active) {
        chan[i].audPos=0;
        chan[i].setPos=true;
        if ((rRead8(0x03+i*16)&0x80)==0)
          chan[i].busy=true;
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
      unsigned char ctrl=0;
      if (chan[i].pcm || i<8) {
        DivSample* s=parent->getSample(chan[i].sample);
        switch (s->depth) {
          case DIV_SAMPLE_DEPTH_IMA_ADPCM: ctrl=0x40; break;
          case DIV_SAMPLE_DEPTH_16BIT: ctrl=0x20; break;
          default: ctrl=0x00; break;
        }
        double off=(s->centerRate>=1)?(parent->getCenterRate()/(double)s->centerRate):1.0;
        chan[i].freq=0x10000-(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER));
        if (chan[i].freq<0) chan[i].freq=0;
        if (chan[i].freq>65535) chan[i].freq=65535;
        if ((!chan[i].keyOn) && ((rRead8(0x03+i*16)&0x80)==0))
          chan[i].busy=false;
        ctrl|=(chan[i].busy?0x80:0)|((s->isLoopable())?0x08:0x10);
        if (chan[i].keyOn) {
          unsigned int start=0;
          int loopStart=0;
          int loopEnd=0;
          int end=0;
          if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
            start=sampleOff[chan[i].sample];
            end=s->getCurBufLen()/4;
          }
          if (chan[i].audPos>0) {
            switch (s->depth) {
              case DIV_SAMPLE_DEPTH_IMA_ADPCM: start+=chan[i].audPos/2; end-=(chan[i].audPos/8); break;
              case DIV_SAMPLE_DEPTH_16BIT: start+=chan[i].audPos*2; end-=(chan[i].audPos/2); break;
              default: start+=chan[i].audPos; end-=(chan[i].audPos/4); break;
            }
          }
          if (s->isLoopable()) {
            if (chan[i].audPos>0) {
              switch (s->depth) {
                case DIV_SAMPLE_DEPTH_IMA_ADPCM:
                  loopStart=(s->loopStart-chan[i].audPos)/8;
                  loopEnd=(s->loopEnd-s->loopStart)/8;
                  if (chan[i].audPos>(unsigned int)s->loopStart) {
                    loopStart=0;
                    loopEnd-=(chan[i].audPos-s->loopStart)/8;
                  }
                  break;
                case DIV_SAMPLE_DEPTH_16BIT:
                  loopStart=(s->loopStart-chan[i].audPos)/2;
                  loopEnd=(s->loopEnd-s->loopStart)/2;
                  if (chan[i].audPos>(unsigned int)s->loopStart) {
                    loopStart=0;
                    loopEnd-=(chan[i].audPos-s->loopStart)/2;
                  }
                  break;
                default:
                  loopStart=(s->loopStart-chan[i].audPos)/4;
                  loopEnd=(s->loopEnd-s->loopStart)/4;
                  if (chan[i].audPos>(unsigned int)s->loopStart) {
                    loopStart=0;
                    loopEnd-=(chan[i].audPos-s->loopStart)/4;
                  }
                  break;
              }
            } else {
              switch (s->depth) {
                case DIV_SAMPLE_DEPTH_IMA_ADPCM: loopStart=s->loopStart/8; loopEnd=(s->loopEnd-s->loopStart)/8; break;
                case DIV_SAMPLE_DEPTH_16BIT: loopStart=s->loopStart/2; loopEnd=(s->loopEnd-s->loopStart)/2; break;
                default: loopStart=s->loopStart/4; loopEnd=(s->loopEnd-s->loopStart)/4; break;
              }
            }
            loopEnd=CLAMP(loopEnd,0,0x3fffff);
            loopStart=CLAMP(loopStart,0,0xffff);
            rWrite16(0x0a+i*16,loopStart);
            rWrite32(0x0c+i*16,loopEnd);
          } else {
            end=CLAMP(end,0,0x3fffff);
            rWrite16(0x0a+i*16,0);
            rWrite32(0x0c+i*16,end&0x3fffff);
          }
          rWrite8(0x03+i*16,ctrl&~0x80); // force keyoff first
          rWrite32(0x04+i*16,start&0x7fffffc);
        }
      } else {
        chan[i].freq=0x10000-(parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,8));
        if (chan[i].freq<0) chan[i].freq=0;
        if (chan[i].freq>65535) chan[i].freq=65535;
        ctrl=(chan[i].active?0xe8:0)|(chan[i].duty&7);
        if (chan[i].keyOff || chan[i].keyOn) {
          rWrite8(0x03+i*16,ctrl&~0x80); // force keyoff first
        }
      }
      chan[i].keyOn=false;
      if (chan[i].keyOff) {
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        rWrite16(0x08+i*16,chan[i].freq&0xffff);
        chan[i].freqChanged=false;
      }
      rWrite8(0x03+i*16,ctrl);
    }
  }
}

int DivPlatformNDS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_NDS);
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample || (c.chan<8)) {
        chan[c.chan].pcm=true;
      } else {
        chan[c.chan].pcm=false;
      }
      if (chan[c.chan].pcm || (c.chan<8)) {
        chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:127;
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        }
        if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
          chan[c.chan].sample=-1;
        }
      } else {
        chan[c.chan].macroVolMul=127;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].busy=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
      chan[c.chan].active=false;
      chan[c.chan].busy=false;
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
    case DIV_CMD_ADPCMA_GLOBAL_VOLUME: {
      if (globalVolume!=(c.value&0x7f)) {
        globalVolume=c.value&0x7f;
        rWrite32(0x100,0x8000|globalVolume);
      }
      break;
    }
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          writeOutVol(c.chan);
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
      chan[c.chan].panning=MIN(parent->convertPanSplitToLinearLR(c.value,c.value2,127),127);
      rWrite8(0x02+c.chan*16,chan[c.chan].panning);
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
      if ((c.chan>=8) && (c.chan<14) && (!chan[c.chan].pcm)) { // pulse
        chan[c.chan].duty=c.value;
        rWrite8(0x03+c.chan*16,(rRead8(0x03+c.chan*16)&0xe8)|(chan[c.chan].duty&7));
      }
      break;
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_NDS));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      if (chan[c.chan].pcm || (c.chan<8)) {
        chan[c.chan].audPos=c.value;
        chan[c.chan].setPos=true;
      }
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
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

void DivPlatformNDS::writeOutVol(int ch) {
  unsigned char val=isMuted[ch]?0:chan[ch].outVol;
  rWrite8(0x00+ch*16,val);
}

void DivPlatformNDS::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  writeOutVol(ch);
}

void DivPlatformNDS::forceIns() {
  for (int i=0; i<16; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;

    rWrite8(0x02+i*16,chan[i].panning);
    writeOutVol(i);
  }
}

void* DivPlatformNDS::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformNDS::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformNDS::getPan(int ch) {
  return parent->convertPanLinearToSplit(chan[ch].panning,8,127);
}

DivDispatchOscBuffer* DivPlatformNDS::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformNDS::reset() {
  memset(regPool,0,288);
  nds.reset();
  globalVolume=0x7f;
  lastOut[0]=0;
  lastOut[1]=0;
  rWrite32(0x100,0x8000|globalVolume); // enable keyon
  rWrite32(0x104,0x200); // initialize bias
  for (int i=0; i<16; i++) {
    chan[i]=DivPlatformNDS::Channel();
    chan[i].std.setEngine(parent);
    rWrite32(0x00+i*16,isMuted[i]?0x400000:0x40007f);
  }
}

int DivPlatformNDS::getOutputCount() {
  return 2;
}

bool DivPlatformNDS::hasAcquireDirect() {
  return true;
}

void DivPlatformNDS::notifyInsChange(int ins) {
  for (int i=0; i<16; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformNDS::notifyWaveChange(int wave) {
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformNDS::notifyInsDeletion(void* ins) {
  for (int i=0; i<16; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformNDS::poke(unsigned int addr, unsigned short val) {
  rWrite8(addr,val);
}

void DivPlatformNDS::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite8(i.addr,i.val);
}

unsigned char* DivPlatformNDS::getRegisterPool() {
  return regPool;
}

int DivPlatformNDS::getRegisterPoolSize() {
  return 288;
}

float DivPlatformNDS::getPostAmp() {
  return 1.0f;
}

const void* DivPlatformNDS::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformNDS::getSampleMemCapacity(int index) {
  return index == 0 ? (isDSi?16777216:4194304) : 0;
}

size_t DivPlatformNDS::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformNDS::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformNDS::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformNDS::renderSamples(int sysID) {
  memset(sampleMem,0,16777216);
  memset(sampleOff,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Main Memory";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }

    int length=0;
    unsigned char* src=NULL;
    switch (s->depth) {
      case DIV_SAMPLE_DEPTH_16BIT:
        length=MIN(16777212,s->length16);
        src=(unsigned char*)s->data16;
        break;
      case DIV_SAMPLE_DEPTH_IMA_ADPCM:
        length=MIN(16777212,s->lengthIMA);
        src=(unsigned char*)s->dataIMA;
        break;
      default:
        length=MIN(16777212,s->length8);
        src=(unsigned char*)s->data8;
        break;
    }

    int actualLength=MIN((int)(getSampleMemCapacity()-memPos),length);
    if (actualLength>0) {
      memcpy(&sampleMem[memPos],src,actualLength);
      sampleOff[i]=memPos;
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+actualLength));
      memPos+=actualLength;
    }
    if (actualLength<length) {
      logW("out of NDS PCM memory for sample %d!",i);
      break;
    }
    // align memPos to int
    memPos=(memPos+3)&~3;
    sampleLoaded[i]=true;
  }
  sampleMemLen=memPos;

  memCompo.capacity=(isDSi?16777216:4194304);
  memCompo.used=sampleMemLen;
}

void DivPlatformNDS::setFlags(const DivConfig& flags) {
  isDSi=flags.getBool("chipType",0);
  chipClock=33513982;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/2;
  for (int i=0; i<16; i++) {
    oscBuf[i]->setRate(rate);
  }
  memCompo.capacity=(isDSi?16777216:4194304);
}

int DivPlatformNDS::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<16; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new unsigned char[16777216];
  sampleMemLen=0;
  nds.reset();
  setFlags(flags);
  reset();

  return 16;
}

void DivPlatformNDS::quit() {
  delete[] sampleMem;
  for (int i=0; i<16; i++) {
    delete oscBuf[i];
  }
}
