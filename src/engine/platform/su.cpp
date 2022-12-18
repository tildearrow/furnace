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

#include "su.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) rWrite(((c)<<5)|(a),v);

#define CHIP_DIVIDER 2
#define CHIP_FREQBASE 524288

const char** DivPlatformSoundUnit::getRegisterSheet() {
  return NULL;
}

double DivPlatformSoundUnit::NOTE_SU(int ch, int note) {
  if (chan[ch].switchRoles) {
    return NOTE_PERIODIC(note);
  }
  return NOTE_FREQUENCY(note);
}

void DivPlatformSoundUnit::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      su->Write(w.addr,w.val);
      writes.pop();
    }
    su->NextSample(&bufL[h],&bufR[h]);
    for (int i=0; i<8; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=su->GetSample(i);
    }
  }
}

void DivPlatformSoundUnit::writeControl(int ch) {
  chWrite(ch,0x04,(chan[ch].wave&7)|(chan[ch].pcm<<3)|(chan[ch].control<<4));
}

void DivPlatformSoundUnit::writeControlUpper(int ch) {
  chWrite(ch,0x05,((int)chan[ch].phaseReset)|(chan[ch].filterPhaseReset<<1)|(chan[ch].pcmLoop<<2)|(chan[ch].timerSync<<3)|(chan[ch].freqSweep<<4)|(chan[ch].volSweep<<5)|(chan[ch].cutSweep<<6));
  chan[ch].phaseReset=false;
  chan[ch].filterPhaseReset=false;
}

void DivPlatformSoundUnit::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SU);
      if (ins->type==DIV_INS_AMIGA) {
        chan[i].outVol=((chan[i].vol&127)*MIN(64,chan[i].std.vol.val))>>6;
      } else {
        chan[i].outVol=((chan[i].vol&127)*MIN(127,chan[i].std.vol.val))>>7;
      }
      chWrite(i,0x02,chan[i].outVol);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_SU(i,parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      chWrite(i,0x08,chan[i].duty);
    }
    if (chan[i].std.wave.had) {
      chan[i].wave=chan[i].std.wave.val&7;
      writeControl(i);
    }
    if (chan[i].std.phaseReset.had) {
      chan[i].phaseReset=chan[i].std.phaseReset.val;
      writeControlUpper(i);
    }
    if (chan[i].std.panL.had) {
      chan[i].pan=chan[i].std.panL.val;
      chWrite(i,0x03,chan[i].pan);
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
    if (chan[i].std.ex1.had) {
      chan[i].cutoff=((chan[i].std.ex1.val&16383)*chan[i].baseCutoff)/16380;
      chWrite(i,0x06,chan[i].cutoff&0xff);
      chWrite(i,0x07,chan[i].cutoff>>8);
    }
    if (chan[i].std.ex2.had) {
      chan[i].res=chan[i].std.ex2.val;
      chWrite(i,0x09,chan[i].res);
    }
    if (chan[i].std.ex3.had) {
      chan[i].control=chan[i].std.ex3.val&15;
      writeControl(i);
    }
    if (chan[i].std.ex4.had) {
      chan[i].syncTimer=chan[i].std.ex4.val&65535;
      chan[i].timerSync=(chan[i].syncTimer>0);
      if (chan[i].switchRoles) {
        chWrite(i,0x00,chan[i].syncTimer&0xff);
        chWrite(i,0x01,chan[i].syncTimer>>8);
      } else {
        chWrite(i,0x1e,chan[i].syncTimer&0xff);
        chWrite(i,0x1f,chan[i].syncTimer>>8);
      }
      writeControlUpper(i);
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SU);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,chan[i].switchRoles,2,chan[i].pitch2,chipClock,chan[i].switchRoles?CHIP_DIVIDER:CHIP_FREQBASE);
      if (chan[i].pcm) {
        DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SU);
        // TODO: sample map?
        DivSample* sample=parent->getSample(ins->amiga.getSample(chan[i].note));
        if (sample!=NULL) {
          double off=0.25;
          if (sample->centerRate<1) {
            off=0.25;
          } else {
            off=(double)sample->centerRate/(8363.0*4.0);
          }
          chan[i].freq=(double)chan[i].freq*off;
        }
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (chan[i].switchRoles) {
        chWrite(i,0x1e,chan[i].freq&0xff);
        chWrite(i,0x1f,chan[i].freq>>8);
      } else {
        chWrite(i,0x00,chan[i].freq&0xff);
        chWrite(i,0x01,chan[i].freq>>8);
      }
      if (chan[i].keyOn) {
        if (chan[i].pcm) {
          DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_SU);
          int sNum=ins->amiga.getSample(chan[i].note);
          DivSample* sample=parent->getSample(sNum);
          if (sample!=NULL && sNum>=0 && sNum<parent->song.sampleLen) {
            unsigned int sampleEnd=sampleOffSU[sNum]+(sample->getLoopEndPosition());
            unsigned int off=sampleOffSU[sNum]+chan[i].hasOffset;
            chan[i].hasOffset=0;
            if (sampleEnd>=getSampleMemCapacity(0)) sampleEnd=getSampleMemCapacity(0)-1;
            chWrite(i,0x0a,off&0xff);
            chWrite(i,0x0b,off>>8);
            chWrite(i,0x0c,sampleEnd&0xff);
            chWrite(i,0x0d,sampleEnd>>8);
            if (sample->isLoopable()) {
              unsigned int sampleLoop=sampleOffSU[sNum]+sample->getLoopStartPosition();
              if (sampleLoop>=getSampleMemCapacity(0)) sampleLoop=getSampleMemCapacity(0)-1;
              chWrite(i,0x0e,sampleLoop&0xff);
              chWrite(i,0x0f,sampleLoop>>8);
              chan[i].pcmLoop=true;
            } else {
              chan[i].pcmLoop=false;
            }
            writeControl(i);
            writeControlUpper(i);
          }
        }
      }
      if (chan[i].keyOff) {
        chWrite(i,0x02,0);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformSoundUnit::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SU);
      chan[c.chan].switchRoles=ins->su.switchRoles;
      if (chan[c.chan].pcm && !(ins->type==DIV_INS_AMIGA || ins->amiga.useSample)) {
        chan[c.chan].pcm=(ins->type==DIV_INS_AMIGA || ins->amiga.useSample);
        writeControl(c.chan);
        writeControlUpper(c.chan);
      }
      chan[c.chan].pcm=(ins->type==DIV_INS_AMIGA || ins->amiga.useSample);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_SU(c.chan,c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chWrite(c.chan,0x02,chan[c.chan].vol);
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].insChanged=false;
      break;
    }
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active) chWrite(c.chan,0x02,chan[c.chan].outVol);
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
      chan[c.chan].wave=c.value&7;
      writeControl(c.chan);
      break;
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=c.value&127;
      chWrite(c.chan,0x08,chan[c.chan].duty);
      break;
    case DIV_CMD_C64_RESONANCE:
      chan[c.chan].res=c.value;
      chWrite(c.chan,0x09,chan[c.chan].res);
      break;
    case DIV_CMD_C64_FILTER_MODE:
      chan[c.chan].control=c.value&15;
      break;
    case DIV_CMD_SU_SWEEP_PERIOD_LOW: {
      switch (c.value) {
        case 0:
          chan[c.chan].freqSweepP=(chan[c.chan].freqSweepP&0xff00)|c.value2;
          chWrite(c.chan,0x10,chan[c.chan].freqSweepP&0xff);
          break;
        case 1:
          chan[c.chan].volSweepP=(chan[c.chan].volSweepP&0xff00)|c.value2;
          chWrite(c.chan,0x14,chan[c.chan].volSweepP&0xff);
          break;
        case 2:
          chan[c.chan].cutSweepP=(chan[c.chan].cutSweepP&0xff00)|c.value2;
          chWrite(c.chan,0x18,chan[c.chan].cutSweepP&0xff);
          break;
      }
      break;
    }
    case DIV_CMD_SU_SWEEP_PERIOD_HIGH: {
      switch (c.value) {
        case 0:
          chan[c.chan].freqSweepP=(chan[c.chan].freqSweepP&0xff)|(c.value2<<8);
          chWrite(c.chan,0x11,chan[c.chan].freqSweepP>>8);
          break;
        case 1:
          chan[c.chan].volSweepP=(chan[c.chan].volSweepP&0xff)|(c.value2<<8);
          chWrite(c.chan,0x15,chan[c.chan].volSweepP>>8);
          break;
        case 2:
          chan[c.chan].cutSweepP=(chan[c.chan].cutSweepP&0xff)|(c.value2<<8);
          chWrite(c.chan,0x19,chan[c.chan].cutSweepP>>8);
          break;
      }
      break;
    }
    case DIV_CMD_SU_SWEEP_BOUND: {
      switch (c.value) {
        case 0:
          chan[c.chan].freqSweepB=c.value2;
          chWrite(c.chan,0x13,chan[c.chan].freqSweepB);
          break;
        case 1:
          chan[c.chan].volSweepB=c.value2;
          chWrite(c.chan,0x17,chan[c.chan].volSweepB);
          break;
        case 2:
          chan[c.chan].cutSweepB=c.value2;
          chWrite(c.chan,0x1b,chan[c.chan].cutSweepB);
          break;
      }
      break;
    }
    case DIV_CMD_SU_SWEEP_ENABLE: {
      switch (c.value) {
        case 0:
          chan[c.chan].freqSweepV=c.value2;
          chan[c.chan].freqSweep=(c.value2>0);
          chWrite(c.chan,0x12,chan[c.chan].freqSweepV);
          break;
        case 1:
          chan[c.chan].volSweepV=c.value2;
          chan[c.chan].volSweep=(c.value2>0);
          chWrite(c.chan,0x16,chan[c.chan].volSweepV);
          break;
        case 2:
          chan[c.chan].cutSweepV=c.value2;
          chan[c.chan].cutSweep=(c.value2>0);
          chWrite(c.chan,0x1a,chan[c.chan].cutSweepV);
          break;
      }
      writeControlUpper(c.chan);
      break;
    }
    case DIV_CMD_SU_SYNC_PERIOD_LOW:
      chan[c.chan].syncTimer=(chan[c.chan].syncTimer&0xff00)|c.value;
      chan[c.chan].timerSync=(chan[c.chan].syncTimer>0);
      chWrite(c.chan,0x1e,chan[c.chan].syncTimer&0xff);
      chWrite(c.chan,0x1f,chan[c.chan].syncTimer>>8);
      writeControlUpper(c.chan);
      break;
    case DIV_CMD_SU_SYNC_PERIOD_HIGH:
      chan[c.chan].syncTimer=(chan[c.chan].syncTimer&0xff)|(c.value<<8);
      chan[c.chan].timerSync=(chan[c.chan].syncTimer>0);
      chWrite(c.chan,0x1e,chan[c.chan].syncTimer&0xff);
      chWrite(c.chan,0x1f,chan[c.chan].syncTimer>>8);
      writeControlUpper(c.chan);
      break;
    case DIV_CMD_C64_FINE_CUTOFF:
      chan[c.chan].baseCutoff=c.value*4;
      if (!chan[c.chan].std.ex1.has) {
        chan[c.chan].cutoff=chan[c.chan].baseCutoff;
        chWrite(c.chan,0x06,chan[c.chan].cutoff&0xff);
        chWrite(c.chan,0x07,chan[c.chan].cutoff>>8);
      }
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_SU(c.chan,c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value*((parent->song.linearPitch==2)?1:(1+(chan[c.chan].baseFreq>>9)));
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value*((parent->song.linearPitch==2)?1:(1+(chan[c.chan].baseFreq>>9)));
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
      chan[c.chan].pan=parent->convertPanSplitToLinearLR(c.value,c.value2,254)-127;
      chWrite(c.chan,0x03,chan[c.chan].pan);
      break;
    }
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].hasOffset=c.value;
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_SU(c.chan,c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SU));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_SU(c.chan,chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
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
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSoundUnit::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  su->muted[ch]=mute;
}

void DivPlatformSoundUnit::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;

    // restore channel attributes
    chWrite(i,0x03,chan[i].pan);
    writeControl(i);
    writeControlUpper(i);
  }
}

void* DivPlatformSoundUnit::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSoundUnit::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSoundUnit::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSoundUnit::getRegisterPool() {
  return (unsigned char*)su->chan;
}

int DivPlatformSoundUnit::getRegisterPoolSize() {
  return 256;
}

void DivPlatformSoundUnit::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,128);
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformSoundUnit::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  su->Reset();
  for (int i=0; i<8; i++) {
    chWrite(i,0x08,0x3f);
  }
  lastPan=0xff;
  cycles=0;
  curChan=-1;
  sampleBank=0;
  lfoMode=0;
  lfoSpeed=255;
  delay=500;

  // set initial IL status
  ilCtrl=initIlCtrl;
  ilSize=initIlSize;
  fil1=initFil1;
  echoVol=initEchoVol;
  rWrite(0x9c,echoVol);
  rWrite(0x9d,ilCtrl);
  rWrite(0xbc,ilSize);
  rWrite(0xbd,fil1);
}

bool DivPlatformSoundUnit::isStereo() {
  return true;
}

bool DivPlatformSoundUnit::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformSoundUnit::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSoundUnit::setFlags(const DivConfig& flags) {
  if (flags.getInt("clockSel",0)) {
    chipClock=1190000;
  } else {
    chipClock=1236000;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/4;
  for (int i=0; i<8; i++) {
    oscBuf[i]->rate=rate;
  }
  bool echoOn=flags.getBool("echo",false);
  initIlCtrl=3|(echoOn?4:0);
  initIlSize=((flags.getInt("echoDelay",0))&63)|(echoOn?0x40:0)|(flags.getBool("swapEcho",false)?0x80:0);
  initFil1=flags.getInt("echoFeedback",0)|(flags.getInt("echoResolution",0)<<4);
  initEchoVol=flags.getInt("echoVol",0);

  sampleMemSize=flags.getInt("sampleMemSize",0);

  su->Init(sampleMemSize?65536:8192,flags.getBool("pdm",false));
  renderSamples(sysIDCache);
}

void DivPlatformSoundUnit::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSoundUnit::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const void* DivPlatformSoundUnit::getSampleMem(int index) {
  return (index==0)?su->pcm:NULL;
}

size_t DivPlatformSoundUnit::getSampleMemCapacity(int index) {
  return (index==0)?((sampleMemSize?65536:8192)-((initIlSize&64)?((1+(initIlSize&63))<<7):0)):0;
}

size_t DivPlatformSoundUnit::getSampleMemUsage(int index) {
  return (index==0)?sampleMemLen:0;
}

bool DivPlatformSoundUnit::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformSoundUnit::renderSamples(int sysID) {
  memset(su->pcm,0,getSampleMemCapacity(0));
  memset(sampleOffSU,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (s->data8==NULL) continue;
    if (!s->renderOn[0][sysID]) {
      sampleOffSU[i]=0;
      continue;
    }
    
    int paddedLen=s->length8;
    if (memPos>=getSampleMemCapacity(0)) {
      logW("out of PCM memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=getSampleMemCapacity(0)) {
      memcpy(su->pcm+memPos,s->data8,getSampleMemCapacity(0)-memPos);
      logW("out of PCM memory for sample %d!",i);
    } else {
      memcpy(su->pcm+memPos,s->data8,paddedLen);
      sampleLoaded[i]=true;
    }
    sampleOffSU[i]=memPos;
    memPos+=paddedLen;
  }
  sampleMemLen=memPos;
  sysIDCache=sysID;
}

int DivPlatformSoundUnit::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  su=new SoundUnit();
  sysIDCache=0;
  setFlags(flags);
  reset();
  return 8;
}

void DivPlatformSoundUnit::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
  delete su;
}

DivPlatformSoundUnit::~DivPlatformSoundUnit() {
}
