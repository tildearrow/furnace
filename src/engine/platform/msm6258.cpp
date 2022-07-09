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

#include "msm6258.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "sound/oki/okim6258.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

const char** DivPlatformMSM6258::getRegisterSheet() {
  return NULL;
}

const char* DivPlatformMSM6258::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x20:
      return "20xx: Set frequency divider (0-2)";
      break;
    case 0x21:
      return "21xx: Select clock rate (0: full; 1: half)";
      break;
  }
  return NULL;
}

void DivPlatformMSM6258::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  short* outs[2]={
    &msmOut,
    NULL
  };
  for (size_t h=start; h<start+len; h++) {
    if (--msmClockCount<0) {
      if (--msmDividerCount<=0) {
        if (!writes.empty()) {
          QueuedWrite& w=writes.front();
          switch (w.addr) {
            case 0:
              msm->ctrl_w(w.val);
              break;
            case 2:
              msmPan=w.val;
              break;
            case 8:
              msmClock=w.val;
              break;
            case 12:
              msmDivider=4-(w.val&3);
              if (msmDivider<2) msmDivider=2;
              break;
          }
          writes.pop();
        }

        if (sample>=0 && sample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(sample);
          unsigned char nextData=(s->dataVOX[samplePos]>>4)|(s->dataVOX[samplePos]<<4);
          if (msm->data_w(nextData)) {
            samplePos++;
            if (samplePos>=(int)s->lengthVOX) {
              sample=-1;
              samplePos=0;
              msm->ctrl_w(1);
            }
          }
        }
        
        msm->sound_stream_update(outs,1);
        msmDividerCount=msmDivider;
      }
      msmClockCount=msmClock;
    }
    
    if (isMuted[0]) {
      bufL[h]=0;
      bufR[h]=0;
      oscBuf[0]->data[oscBuf[0]->needle++]=0;
    } else {
      bufL[h]=(msmPan&2)?msmOut:0;
      bufR[h]=(msmPan&1)?msmOut:0;
      oscBuf[0]->data[oscBuf[0]->needle++]=msmPan?msmOut:0;
    }
  }
}

void DivPlatformMSM6258::tick(bool sysTick) {
  // nothing
}

int DivPlatformMSM6258::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
      if (ins->type==DIV_INS_AMIGA) {
        chan[c.chan].furnacePCM=true;
      } else {
        chan[c.chan].furnacePCM=false;
      }
      if (skipRegisterWrites) break;
      if (chan[c.chan].furnacePCM) {
        chan[c.chan].macroInit(ins);
        if (!chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
        sample=ins->amiga.getSample(c.value);
        samplePos=0;
        if (sample>=0 && sample<parent->song.sampleLen) {
          //DivSample* s=parent->getSample(chan[c.chan].sample);
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
            chan[c.chan].freqChanged=true;
          }
          chan[c.chan].active=true;
          chan[c.chan].keyOn=true;
          rWrite(0,1);
          rWrite(0,2);
        } else {
          break;
        }
      } else {
        chan[c.chan].sample=-1;
        chan[c.chan].macroInit(NULL);
        chan[c.chan].outVol=chan[c.chan].vol;
        if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
          break;
        }
        //DivSample* s=parent->getSample(12*sampleBank+c.value%12);
        sample=12*sampleBank+c.value%12;
        samplePos=0;
        rWrite(0,1);
        rWrite(0,2);
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      rWrite(0,1); // turn off
      sample=-1;
      samplePos=0;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      rWrite(0,1); // turn off
      sample=-1;
      samplePos=0;
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
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PITCH: {
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      return 2;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_SAMPLE_FREQ:
      rateSel=c.value&3;
      rWrite(12,rateSel);
      break;
    case DIV_CMD_SAMPLE_MODE:
      clockSel=c.value&1;
      rWrite(8,clockSel);
      break;
    case DIV_CMD_PANNING: {
      if (c.value==0 && c.value2==0) {
        chan[c.chan].pan=3;
      } else {
        chan[c.chan].pan=(c.value2>0)|((c.value>0)<<1);
      }
      rWrite(2,chan[c.chan].pan);
      break;
    }
    case DIV_CMD_LEGATO: {
      break;
    }
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 8;
      break;
    case DIV_CMD_PRE_PORTA:
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformMSM6258::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformMSM6258::forceIns() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<1; i++) {
    chan[i].insChanged=true;
  }
  rWrite(12,rateSel);
  rWrite(8,clockSel);
  rWrite(2,chan[0].pan);
}

void* DivPlatformMSM6258::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformMSM6258::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformMSM6258::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformMSM6258::getRegisterPool() {
  return NULL;
}

int DivPlatformMSM6258::getRegisterPoolSize() {
  return 0;
}

void DivPlatformMSM6258::poke(unsigned int addr, unsigned short val) {
  //immWrite(addr,val);
}

void DivPlatformMSM6258::poke(std::vector<DivRegWrite>& wlist) {
  //for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformMSM6258::reset() {
  while (!writes.empty()) writes.pop();
  msm->device_reset();
  msmClock=chipClock;
  msmDivider=2;
  msmDividerCount=0;
  msmClock=0;
  msmClockCount=0;
  msmPan=3;
  rateSel=2;
  clockSel=0;
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<1; i++) {
    chan[i]=DivPlatformMSM6258::Channel();
    chan[i].std.setEngine(parent);
  }
  for (int i=0; i<1; i++) {
    chan[i].vol=8;
    chan[i].outVol=8;
  }

  sampleBank=0;
  sample=-1;
  samplePos=0;

  delay=0;
}

bool DivPlatformMSM6258::isStereo() {
  return true;
}

bool DivPlatformMSM6258::keyOffAffectsArp(int ch) {
  return false;
}

void DivPlatformMSM6258::notifyInsChange(int ins) {
  for (int i=0; i<1; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformMSM6258::notifyInsDeletion(void* ins) {
}

const void* DivPlatformMSM6258::getSampleMem(int index) {
  return index == 0 ? adpcmMem : NULL;
}

size_t DivPlatformMSM6258::getSampleMemCapacity(int index) {
  return index == 0 ? 262144 : 0;
}

size_t DivPlatformMSM6258::getSampleMemUsage(int index) {
  return index == 0 ? adpcmMemLen : 0;
}

void DivPlatformMSM6258::renderSamples() {
  memset(adpcmMem,0,getSampleMemCapacity(0));

  // sample data
  size_t memPos=0;
  int sampleCount=parent->song.sampleLen;
  if (sampleCount>128) sampleCount=128;
  for (int i=0; i<sampleCount; i++) {
    DivSample* s=parent->song.sample[i];
    int paddedLen=s->lengthVOX;
    if (memPos>=getSampleMemCapacity(0)) {
      logW("out of ADPCM memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=getSampleMemCapacity(0)) {
      memcpy(adpcmMem+memPos,s->dataVOX,getSampleMemCapacity(0)-memPos);
      logW("out of ADPCM memory for sample %d!",i);
    } else {
      memcpy(adpcmMem+memPos,s->dataVOX,paddedLen);
    }
    s->offVOX=memPos;
    memPos+=paddedLen;
  }
  adpcmMemLen=memPos+256;
}

void DivPlatformMSM6258::setFlags(unsigned int flags) {
  switch (flags) {
    case 3:
      chipClock=8192000;
      break;
    case 2:
      chipClock=8000000;
      break;
    case 1:
      chipClock=4096000;
      break;
    default:
      chipClock=4000000;
      break;
  }
  rate=chipClock/256;
  for (int i=0; i<1; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformMSM6258::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  adpcmMem=new unsigned char[getSampleMemCapacity(0)];
  adpcmMemLen=0;
  dumpWrites=false;
  skipRegisterWrites=false;
  updateOsc=0;
  for (int i=0; i<1; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  msm=new okim6258_device(4000000);
  msm->device_start();
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformMSM6258::quit() {
  for (int i=0; i<1; i++) {
    delete oscBuf[i];
  }
  delete msm;
  delete[] adpcmMem;
}

DivPlatformMSM6258::~DivPlatformMSM6258() {
}
