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

#include "multipcm.h"
#include "../engine.h"
#include "../bsr.h"
#include "../../ta-log.h"
#include <string.h>
#include <math.h>

const unsigned char slotsMPCM[28]={
  0x00,0x01,0x02,0x03,0x04,0x05,0x06,
  0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,
  0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,
};

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) \
  if (!skipRegisterWrites) { \
    writes.push(QueuedWrite(a,v)); \
    if (dumpWrites) { \
      addWrite(1,slotsMPCM[(a>>3)&0x1f]); \
      addWrite(2,a&0x7); \
      addWrite(0,v); \
    } \
  }

#define chWrite(c,a,v) \
  if (!skipRegisterWrites) { \
    rWrite((c<<3)|(a&0x7),v); \
  }

#define chImmWrite(c,a,v) \
  if (!skipRegisterWrites) { \
    writes.push(QueuedWrite((c<<3)|(a&0x7),v)); \
    if (dumpWrites) { \
      addWrite(1,slotsMPCM[c&0x1f]); \
      addWrite(2,a&0x7); \
      addWrite(0,v); \
    } \
  }

#define CHIP_FREQBASE (117440512)

const char* regCheatSheetMultiPCM[]={
  "Pan", "0",
  "SampleL", "1",
  "SampleH_FreqL", "2",
  "FreqH", "3",
  "KeyOn", "4",
  "TL_LD", "5",
  "LFO_VIB", "6",
  "AM", "7",
  NULL
};

const char** DivPlatformMultiPCM::getRegisterSheet() {
  return regCheatSheetMultiPCM;
}

#define PCM_ADDR_PAN 0 // Panpot
#define PCM_ADDR_WAVE_L 1 // Wavetable number LSB
#define PCM_ADDR_WAVE_H_FN_L 2 // Wavetable number MSB, F-number LSB
#define PCM_ADDR_FN_H_OCT 3 // F-number MSB, Pseudo-reverb, Octave
#define PCM_ADDR_KEY 4 // Key
#define PCM_ADDR_TL 5 // Total level, Level direct

#define PCM_ADDR_LFO_VIB 6
#define PCM_ADDR_AM 7

void DivPlatformMultiPCM::acquire(short** buf, size_t len) {
  thread_local short o[4];
  thread_local int os[2];
  thread_local short pcmBuf[28];

  for (int i=0; i<28; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty() && --delay<0) {
      QueuedWrite& w=writes.front();
      if (w.addr==0xfffffffe) {
        delay=w.val;
      } else {
        delay=1;
        pcm.writeReg(slotsMPCM[(w.addr>>3)&0x1f],w.addr&0x7,w.val);
        regPool[w.addr]=w.val;
      }
      writes.pop();
    }

    pcm.generate(o[0],o[1],o[2],o[3],pcmBuf);
    // stereo output only
    os[0]+=o[0];
    os[1]+=o[1];
    os[0]+=o[2];
    os[1]+=o[3];

    for (int i=0; i<28; i++) {
      oscBuf[i]->putSample(h,CLAMP(pcmBuf[i],-32768,32767));
    }
    
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;

    buf[0][h]=os[0];
    buf[1][h]=os[1];
  }

  for (int i=0; i<28; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformMultiPCM::tick(bool sysTick) {
  for (int i=0; i<28; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG((chan[i].vol&0x7f),(0x7f*chan[i].std.vol.val)/chan[i].macroVolMul,0x7f);
      chImmWrite(i,PCM_ADDR_TL,((0x7f-chan[i].outVol)<<1)|(chan[i].levelDirect?1:0));
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
        CLAMP_VAR(chan[i].pitch2,-131071,131071);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].keyOn=true;
        chan[i].writeCtrl=true;
      }
    }

    if (chan[i].std.panL.had) { // panning
      chan[i].pan=chan[i].std.panL.val&0xf;
      chImmWrite(i,PCM_ADDR_PAN,(isMuted[i]?8:chan[i].pan)<<4);
    }

    if (chan[i].std.ex1.had) {
      chan[i].lfo=chan[i].std.ex1.val&0x7;
      chWrite(i,PCM_ADDR_LFO_VIB,(chan[i].lfo<<3)|(chan[i].vib));
    }

    if (chan[i].std.fms.had) {
      chan[i].vib=chan[i].std.fms.val&0x7;
      chWrite(i,PCM_ADDR_LFO_VIB,(chan[i].lfo<<3)|(chan[i].vib));
    }

    if (chan[i].std.ams.had) {
      chan[i].am=chan[i].std.ams.val&0x7;
      chWrite(i,PCM_ADDR_AM,chan[i].am);
    }
  }

  for (int i=0; i<224; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<28; i++) {
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivSample* s=parent->getSample(parent->getIns(chan[i].ins)->amiga.initSample);
      unsigned char ctrl=0;
      double off=(s->centerRate>=1)?((double)s->centerRate/parent->getCenterRate()):1.0;
      chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].freq<0x400) chan[i].freq=0x400;
      chan[i].freqH=0;
      if (chan[i].freq>0x3ffffff) {
        chan[i].freq=0x3ffffff;
        chan[i].freqH=15;
      } else if (chan[i].freq>=0x800) {
        chan[i].freqH=bsr32(chan[i].freq)-11;
      }
      chan[i].freqL=(chan[i].freq>>chan[i].freqH)&0x3ff;
      chan[i].freqH=8^chan[i].freqH;
      ctrl|=chan[i].active?0x80:0;
      int waveNum=chan[i].sample;
      if (waveNum>=0) {
        if (chan[i].keyOn) {
          chImmWrite(i,PCM_ADDR_KEY,ctrl&~0x80); // force keyoff first
          chImmWrite(i,PCM_ADDR_WAVE_H_FN_L,((chan[i].freqL&0x3f)<<2)|((waveNum>>8)&1));
          chImmWrite(i,PCM_ADDR_WAVE_L,waveNum&0xff);
          if (!chan[i].std.vol.had) {
            chan[i].outVol=chan[i].vol;
            chImmWrite(i,PCM_ADDR_TL,((0x7f-chan[i].outVol)<<1)|(chan[i].levelDirect?1:0));
          }
          chan[i].writeCtrl=true;
          chan[i].keyOn=false;
        }
        if (chan[i].keyOff) {
          chan[i].writeCtrl=true;
          chan[i].keyOff=false;
        }
        if (chan[i].freqChanged) {
          chImmWrite(i,PCM_ADDR_WAVE_H_FN_L,((chan[i].freqL&0x3f)<<2)|((waveNum>>8)&1));
          chImmWrite(i,PCM_ADDR_FN_H_OCT,((chan[i].freqH&0xf)<<4)|((chan[i].freqL>>6)&0xf));
          chan[i].freqChanged=false;
        }
        if (chan[i].writeCtrl) {
          chImmWrite(i,PCM_ADDR_KEY,ctrl);
          chan[i].writeCtrl=false;
        }
      } else {
        // cut if we don't have a sample
        chImmWrite(i,PCM_ADDR_KEY,ctrl&~0x80);
      }
    }
  }
}

void DivPlatformMultiPCM::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chImmWrite(ch,PCM_ADDR_PAN,(isMuted[ch]?8:chan[ch].pan)<<4);
}

int DivPlatformMultiPCM::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_MULTIPCM);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:127;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].sample=chan[c.chan].ins;
        chan[c.chan].sampleNote=c.value;
        chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      }
      if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.insLen) {
        chan[c.chan].sample=-1;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      if (chan[c.chan].insChanged) {
        if (ins->type==DIV_INS_MULTIPCM) {
          chan[c.chan].lfo=ins->multipcm.lfo;
          chan[c.chan].vib=ins->multipcm.vib;
          chan[c.chan].am=ins->multipcm.am;
        } else {
          chan[c.chan].lfo=0;
          chan[c.chan].vib=0;
          chan[c.chan].am=0;
        }
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].sample=-1;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      chImmWrite(c.chan,PCM_ADDR_TL,((0x7f-chan[c.chan].outVol)<<1)|(chan[c.chan].levelDirect?1:0));
      break;
    }
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=8^MIN(parent->convertPanSplitToLinearLR(c.value,c.value2,15)+1,15);
      chImmWrite(c.chan,PCM_ADDR_PAN,(isMuted[c.chan]?8:chan[c.chan].pan)<<4);
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
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
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_MULTIPCM_LFO:
      chan[c.chan].lfo=c.value&7;
      chWrite(c.chan,PCM_ADDR_LFO_VIB,(chan[c.chan].lfo<<3)|(chan[c.chan].vib));
      break;
    case DIV_CMD_MULTIPCM_VIB:
      chan[c.chan].vib=c.value&7;
      chWrite(c.chan,PCM_ADDR_LFO_VIB,(chan[c.chan].lfo<<3)|(chan[c.chan].vib));
      break;
    case DIV_CMD_MULTIPCM_AM:
      chan[c.chan].am=c.value&7;
      chWrite(c.chan,PCM_ADDR_AM,chan[c.chan].am);
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
    case DIV_CMD_GET_VOLMAX:
      return 127;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_MULTIPCM));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformMultiPCM::forceIns() {
  for (int i=0; i<28; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
  for (int i=0; i<224; i++) {
    oldWrites[i]=-1;
  }
}

void DivPlatformMultiPCM::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void* DivPlatformMultiPCM::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformMultiPCM::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformMultiPCM::getPan(int ch) {
  return parent->convertPanLinearToSplit(8^chan[ch].pan,8,15);
}

DivDispatchOscBuffer* DivPlatformMultiPCM::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformMultiPCM::mapVelocity(int ch, float vel) {
  // -0.375dB per step
  // -6: 64: 16
  // -12: 32: 32
  // -18: 16: 48
  // -24: 8: 64
  // -30: 4: 80
  // -36: 2: 96
  // -42: 1: 112
  if (vel==0) return 0;
  if (vel>=1.0) return 127;
  return CLAMP(round(128.0-(112.0-log2(vel*127.0)*16.0)),0,127);
}

float DivPlatformMultiPCM::getGain(int ch, int vol) {
  if (vol==0) return 0;
  return 1.0/pow(10.0,(float)(127-vol)*0.375/20.0);
}

unsigned char* DivPlatformMultiPCM::getRegisterPool() {
  return regPool;
}

int DivPlatformMultiPCM::getRegisterPoolSize() {
  return 224;
}

void DivPlatformMultiPCM::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,224);

  pcm.reset();

  renderInstruments();

  for (int i=0; i<28; i++) {
    chan[i]=DivPlatformMultiPCM::Channel();
    chan[i].std.setEngine(parent);
    chImmWrite(i,PCM_ADDR_PAN,(isMuted[i]?8:chan[i].pan)<<4);
  }

  for (int i=0; i<224; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  curChan=-1;
  curAddr=-1;

  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  delay=0;
}

int DivPlatformMultiPCM::getOutputCount() {
  return 2;
}

bool DivPlatformMultiPCM::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformMultiPCM::keyOffAffectsPorta(int ch) {
  return false;
}

bool DivPlatformMultiPCM::getLegacyAlwaysSetVolume() {
  return false;
}

void DivPlatformMultiPCM::notifyInsChange(int ins) {
  renderInstruments();
  for (int i=0; i<28; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformMultiPCM::notifySampleChange(int sample) {
  renderInstruments();
}

void DivPlatformMultiPCM::notifyInsAddition(int sysID) {
  renderInstruments();
}

void DivPlatformMultiPCM::notifyInsDeletion(void* ins) {
  renderInstruments();
  for (int i=0; i<28; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformMultiPCM::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformMultiPCM::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformMultiPCM::getPortaFloor(int ch) {
  return 0;
}

void DivPlatformMultiPCM::setFlags(const DivConfig& flags) {
  chipClock=10000000.0;
  CHECK_CUSTOM_CLOCK;
  pcm.setClockFrequency(chipClock);
  rate=chipClock/224;

  for (int i=0; i<28; i++) {
    oscBuf[i]->setRate(rate);
  }
}

const void* DivPlatformMultiPCM::getSampleMem(int index) {
  return (index==0)?pcmMem:NULL;
}

size_t DivPlatformMultiPCM::getSampleMemCapacity(int index) {
  return (index==0)?2097152:0;
}

size_t DivPlatformMultiPCM::getSampleMemUsage(int index) {
  return (index==0)?pcmMemLen:0;
}

bool DivPlatformMultiPCM::hasSamplePtrHeader(int index) {
  return (index==0);
}

bool DivPlatformMultiPCM::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>32767) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformMultiPCM::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

// this is called on instrument change, reset and/or renderSamples().
// I am not making this part of DivDispatch as this is the only chip with
// instruments in ROM.
void DivPlatformMultiPCM::renderInstruments() {
  // instrument table
  int insCount=parent->song.insLen;
  for (int i=0; i<insCount; i++) {
    DivInstrument* ins=parent->song.ins[i];
    unsigned int insAddr=(i*12);
    short sample=ins->amiga.initSample;
    if (sample>=0 && sample<parent->song.sampleLen) {
      DivSample* s=parent->song.sample[sample];
      unsigned char bitDepth;
      int startPos=sampleOff[sample];
      int endPos=CLAMP(s->isLoopable()?s->loopEnd:(s->samples+1),1,0x10000);
      int loop=s->isLoopable()?CLAMP(s->loopStart,0,endPos-2):(endPos-2);
      switch (s->depth) {
        case DIV_SAMPLE_DEPTH_8BIT:
          bitDepth=0;
          break;
        case DIV_SAMPLE_DEPTH_12BIT:
          bitDepth=3;
          if (!s->isLoopable()) {
            endPos++;
            loop++;
          }
          break;
        default:
          bitDepth=0;
          break;
      }
      pcmMem[insAddr]=(bitDepth<<6)|((startPos>>16)&0x1f);
      pcmMem[1+insAddr]=(startPos>>8)&0xff;
      pcmMem[2+insAddr]=(startPos)&0xff;
      pcmMem[3+insAddr]=(loop>>8)&0xff;
      pcmMem[4+insAddr]=(loop)&0xff;
      pcmMem[5+insAddr]=((~(endPos-1))>>8)&0xff;
      pcmMem[6+insAddr]=(~(endPos-1))&0xff;
      if (ins->type==DIV_INS_MULTIPCM) {
        pcmMem[7+insAddr]=(ins->multipcm.lfo<<3)|ins->multipcm.vib; // LFO, VIB
        pcmMem[8+insAddr]=(ins->multipcm.ar<<4)|ins->multipcm.d1r; // AR, D1R
        pcmMem[9+insAddr]=(ins->multipcm.dl<<4)|ins->multipcm.d2r; // DL, D2R
        pcmMem[10+insAddr]=(ins->multipcm.rc<<4)|ins->multipcm.rr; // RC, RR
        pcmMem[11+insAddr]=ins->multipcm.am; // AM
      } else {
        pcmMem[7+insAddr]=0; // LFO, VIB
        pcmMem[8+insAddr]=(0xf<<4)|(0xf<<0); // AR, D1R
        pcmMem[9+insAddr]=0; // DL, D2R
        pcmMem[10+insAddr]=(0xf<<4)|(0xf<<0); // RC, RR
        pcmMem[11+insAddr]=0; // AM
      }
    } else {
      // fill to dummy instrument
      pcmMem[insAddr]=0;
      pcmMem[1+insAddr]=0;
      pcmMem[2+insAddr]=0;
      pcmMem[3+insAddr]=0;
      pcmMem[4+insAddr]=0;
      pcmMem[5+insAddr]=0xff;
      pcmMem[6+insAddr]=0xff;
      pcmMem[7+insAddr]=0; // LFO, VIB
      pcmMem[8+insAddr]=(0xf<<4)|(0xf<<0); // AR, D1R
      pcmMem[9+insAddr]=(0xf<<4)|(0xf<<0); // DL, D2R
      pcmMem[10+insAddr]=(0xf<<4)|(0xf<<0); // RC, RR
      pcmMem[11+insAddr]=0; // AM
    }
  }
}

void DivPlatformMultiPCM::renderSamples(int sysID) {
  memset(pcmMem,0,2097152);
  memset(sampleOff,0,32768*sizeof(unsigned int));
  memset(sampleLoaded,0,32768*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample Memory";

  size_t memPos=0x1800;
  int sampleCount=parent->song.sampleLen;
  if (sampleCount>512) {
    // mark the rest as unavailable
    for (int i=512; i<sampleCount; i++) {
      sampleLoaded[i]=false;
    }
    sampleCount=512;
  }
  for (int i=0; i<sampleCount; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }

    int length;
    int sampleLength;
    unsigned char* src=(unsigned char*)s->getCurBuf();
    switch (s->depth) {
      case DIV_SAMPLE_DEPTH_8BIT:
        sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
        length=MIN(65535,sampleLength+1);
        break;
      case DIV_SAMPLE_DEPTH_12BIT:
        sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_12BIT);
        length=MIN(98303,sampleLength+3);
        break;
      default:
        sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
        length=MIN(65535,sampleLength+1);
        src=(unsigned char*)s->data8;
        break;
    }
    if (sampleLength<1) length=0;
    int actualLength=MIN((int)(getSampleMemCapacity(0)-memPos),length);
    if (actualLength>0) {
        for (int i=0, j=0; i<actualLength; i++, j++) {
          if (j>=sampleLength && s->depth!=DIV_SAMPLE_DEPTH_12BIT) j=sampleLength-1;
          pcmMem[memPos+i]=src[j];
        }
      sampleOff[i]=memPos;
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+length));
      memPos+=length;
    }
    if (actualLength<length) {
      logW("out of MultiPCM memory for sample %d!",i);
      break;
    }
    sampleLoaded[i]=true;
    pcmMemLen=memPos+256;
    memCompo.used=pcmMemLen;
  }
  renderInstruments();
  memCompo.capacity=getSampleMemCapacity(0);
}

int DivPlatformMultiPCM::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<28; i++) {
    isMuted[i]=false;
  }
  for (int i=0; i<28; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  setFlags(flags);

  pcmMem=new unsigned char[2097152];
  pcmMemLen=0;
  pcmMemory.memory=pcmMem;

  reset();
  return 28;
}

void DivPlatformMultiPCM::quit() {
  for (int i=0; i<28; i++) {
    delete oscBuf[i];
  }
  delete[] pcmMem;
}

// initialization of important arrays
DivPlatformMultiPCM::DivPlatformMultiPCM():
  pcmMemory(0x200000),
  pcm(pcmMemory) {
  sampleOff=new unsigned int[32768];
  sampleLoaded=new bool[32768];
}

DivPlatformMultiPCM::~DivPlatformMultiPCM() {
  delete[] sampleOff;
  delete[] sampleLoaded;
}
