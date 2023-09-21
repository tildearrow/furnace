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

#include "msm6295.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }
#define rWriteDelay(a,v,d) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v,d)); if (dumpWrites) {addWrite(a,v);} }

const char** DivPlatformMSM6295::getRegisterSheet() {
  return NULL;
}

u8 DivPlatformMSM6295::read_byte(u32 address) {
  if (adpcmMem==NULL || address>=getSampleMemCapacity(0)) {
    return 0;
  }
  if (isBanked) {
    if (address<0x400) {
      return adpcmMem[(bank[(address>>8)&0x3]<<16)|(address&0x3ff)];
    }
    return adpcmMem[(bank[(address>>16)&0x3]<<16)|(address&0xffff)];
  }
  return adpcmMem[address&0x3ffff];
}

void DivPlatformMSM6295::acquire(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    if (delay<=0) {
      if (!writes.empty()) {
        QueuedWrite& w=writes.front();
        switch (w.addr) {
          case 0: // command
            msm.command_w(w.val);
            break;
          case 8: // chip clock select (VGM)
          case 9:
          case 10:
          case 11:
            break;
          case 12: // rate select
            msm.ss_w(!w.val);
            break;
          case 14: // enable bankswitch
            break;
          case 15: // set bank base
            break;
          case 16: // switch bank
          case 17:
          case 18:
          case 19:
            bank[w.addr-16]=w.val;
            break;
        }
        writes.pop();
        delay=w.delay;
      }
    } else {
      delay-=3;
    }
    
    msm.tick();
    msm.tick();
    msm.tick();
  
    buf[0][h]=msm.out()<<4;

    if (++updateOsc>=22) {
      updateOsc=0;
      for (int i=0; i<4; i++) {
        oscBuf[i]->data[oscBuf[i]->needle++]=msm.voice_out(i)<<5;
      }
    }
  }
}

void DivPlatformMSM6295::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    if (!parent->song.disableSampleMacro) {
      chan[i].std.next();
      if (chan[i].std.vol.had) {
        chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].std.vol.val,chan[i].vol,8);
      }
      if (chan[i].std.duty.had) {
        if (rateSel!=(chan[i].std.duty.val&1)) {
          rateSel=chan[i].std.duty.val&1;
          rWrite(12,!rateSel);
        }
      }
      if (chan[i].std.phaseReset.had) {
        if (chan[i].std.phaseReset.val && chan[i].active) {
          chan[i].keyOn=true;
        }
      }
    }
    if (chan[i].keyOn || chan[i].keyOff) {
      rWriteDelay(0,(8<<i),60); // turn off
      if (chan[i].active && !chan[i].keyOff) {
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          if (isBanked) {
            rWrite(16+i,bankedPhrase[chan[i].sample].bank);
            rWrite(0,0x80|(i<<5)|bankedPhrase[chan[i].sample].phrase);
          } else {
            rWrite(0,0x80|chan[i].sample); // set phrase
          }
          rWrite(0,(16<<i)|(8-chan[i].outVol)); // turn on
        } else {
          chan[i].sample=-1;
        }
      } else {
        chan[i].sample=-1;
      }
      chan[i].keyOn=false;
      chan[i].keyOff=false;
    }
  }
}

int DivPlatformMSM6295::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
      if (ins->type==DIV_INS_MSM6295 || ins->type==DIV_INS_AMIGA) {
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
        if (c.value!=DIV_NOTE_NULL) chan[c.chan].sample=ins->amiga.getSample(c.value);
        if (chan[c.chan].sample>=0 && chan[c.chan].sample<parent->song.sampleLen) {
          //DivSample* s=parent->getSample(chan[c.chan].sample);
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].keyOn=true;
          rWriteDelay(0,(8<<c.chan),180); // turn off
          if (isBanked) {
            rWrite(16+c.chan,bankedPhrase[chan[c.chan].sample].bank);
            rWrite(0,0x80|(c.chan<<5)|bankedPhrase[chan[c.chan].sample].phrase);
          } else {
            rWrite(0,0x80|chan[c.chan].sample); // set phrase
          }
          rWrite(0,(16<<c.chan)|(8-chan[c.chan].outVol)); // turn on
        } else {
          break;
        }
      } else {
        chan[c.chan].sample=-1;
        chan[c.chan].macroInit(NULL);
        chan[c.chan].outVol=chan[c.chan].vol;
        if ((12*sampleBank+c.value%12)<0 || (12*sampleBank+c.value%12)>=parent->song.sampleLen) {
          break;
        }
        //DivSample* s=parent->getSample(12*sampleBank+c.value%12);
        chan[c.chan].sample=12*sampleBank+c.value%12;
        rWriteDelay(0,(8<<c.chan),180); // turn off
        if (isBanked) {
          rWrite(16+c.chan,bankedPhrase[chan[c.chan].sample].bank);
          rWrite(0,0x80|(c.chan<<5)|bankedPhrase[chan[c.chan].sample].phrase);
        } else {
          rWrite(0,0x80|chan[c.chan].sample); // set phrase
        }
        rWrite(0,(16<<c.chan)|(8-chan[c.chan].outVol)); // turn on
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      rWriteDelay(0,(8<<c.chan),180); // turn off
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      rWriteDelay(0,(8<<c.chan),180); // turn off
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=MIN(8,c.value);
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
    case DIV_CMD_SAMPLE_FREQ:
      rateSel=c.value;
      rWrite(12,!rateSel);
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_LEGATO: {
      break;
    }
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
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

void DivPlatformMSM6295::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  msm.voice_mute(ch,mute);
}

void DivPlatformMSM6295::forceIns() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
  }
  rWrite(12,!rateSel);
}

void* DivPlatformMSM6295::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformMSM6295::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformMSM6295::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformMSM6295::getRegisterPool() {
  return NULL;
}

int DivPlatformMSM6295::getRegisterPoolSize() {
  return 0;
}

void DivPlatformMSM6295::poke(unsigned int addr, unsigned short val) {
  //immWrite(addr,val);
}

void DivPlatformMSM6295::poke(std::vector<DivRegWrite>& wlist) {
  //for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformMSM6295::reset() {
  while (!writes.empty()) writes.pop();
  msm.reset();
  msm.ss_w(rateSelInit);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformMSM6295::Channel();
    chan[i].std.setEngine(parent);
    msm.voice_mute(i,isMuted[i]);
  }
  for (int i=0; i<4; i++) {
    chan[i].vol=8;
    chan[i].outVol=8;
  }

  sampleBank=0;
  rateSel=rateSelInit;
  rWrite(12,!rateSelInit);
  if (isBanked) {
    rWrite(14,1);
  }

  delay=0;
}

bool DivPlatformMSM6295::keyOffAffectsArp(int ch) {
  return false;
}

float DivPlatformMSM6295::getPostAmp() {
  return 3.0f;
}

void DivPlatformMSM6295::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformMSM6295::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

const void* DivPlatformMSM6295::getSampleMem(int index) {
  return index == 0 ? adpcmMem : NULL;
}

size_t DivPlatformMSM6295::getSampleMemCapacity(int index) {
  return index == 0 ? (isBanked?16777216:262144) : 0;
}

size_t DivPlatformMSM6295::getSampleMemUsage(int index) {
  return index == 0 ? adpcmMemLen : 0;
}

bool DivPlatformMSM6295::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformMSM6295::renderSamples(int sysID) {
  unsigned int sampleOffVOX[256];

  memset(adpcmMem,0,16777216);
  memset(sampleOffVOX,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));
  for (int i=0; i<256; i++) {
    bankedPhrase[i].bank=0;
    bankedPhrase[i].phrase=0;
  }

  // sample data
  size_t memPos=128*8;
  if (isBanked) {
    int bankInd=0;
    int phraseInd=0;
    for (int i=0; i<parent->song.sampleLen; i++) {
      DivSample* s=parent->song.sample[i];
      if (!s->renderOn[0][sysID]) {
        sampleOffVOX[i]=0;
        continue;
      }

      int paddedLen=s->lengthVOX;
      // fit to single bank size
      if (paddedLen>65536-0x400) {
        paddedLen=65536-0x400;
      }
      // 32 phrase per bank
      if ((phraseInd>=32)||((memPos&0xff0000)!=((memPos+paddedLen)&0xff0000))) {
        memPos=((memPos+0xffff)&0xff0000)+0x400;
        bankInd++;
        phraseInd=0;
      }
      if (memPos>=getSampleMemCapacity(0)) {
        logW("out of ADPCM memory for sample %d!",i);
        break;
      }
      if (memPos+paddedLen>=getSampleMemCapacity(0)) {
        memcpy(adpcmMem+memPos,s->dataVOX,getSampleMemCapacity(0)-memPos);
        logW("out of ADPCM memory for sample %d!",i);
      } else {
        memcpy(adpcmMem+memPos,s->dataVOX,paddedLen);
        sampleLoaded[i]=true;
      }
      sampleOffVOX[i]=memPos;
      bankedPhrase[i].bank=bankInd;
      bankedPhrase[i].phrase=phraseInd;
      memPos+=paddedLen;
      phraseInd++;
    }
    adpcmMemLen=memPos+256;

    // phrase book
    for (int i=0; i<parent->song.sampleLen; i++) {
      DivSample* s=parent->song.sample[i];
      int endPos=sampleOffVOX[i]+s->lengthVOX;
      for (int b=0; b<4; b++) {
        unsigned int bankedAddr=((unsigned int)bankedPhrase[i].bank<<16)+(b<<8)+(bankedPhrase[i].phrase*8);
        adpcmMem[bankedAddr]=b;
        adpcmMem[bankedAddr+1]=(sampleOffVOX[i]>>8)&0xff;
        adpcmMem[bankedAddr+2]=(sampleOffVOX[i])&0xff;
        adpcmMem[bankedAddr+3]=b;
        adpcmMem[bankedAddr+4]=(endPos>>8)&0xff;
        adpcmMem[bankedAddr+5]=(endPos)&0xff;
      }
    }
  } else {
    int sampleCount=parent->song.sampleLen;
    if (sampleCount>127) sampleCount=127;
    for (int i=0; i<sampleCount; i++) {
      DivSample* s=parent->song.sample[i];
      if (!s->renderOn[0][sysID]) {
        sampleOffVOX[i]=0;
        continue;
      }

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
        sampleLoaded[i]=true;
      }
      sampleOffVOX[i]=memPos;
      memPos+=paddedLen;
    }
    adpcmMemLen=memPos+256;

    // phrase book
    for (int i=0; i<sampleCount; i++) {
      DivSample* s=parent->song.sample[i];
      int endPos=sampleOffVOX[i]+s->lengthVOX;
      adpcmMem[i*8]=(sampleOffVOX[i]>>16)&0xff;
      adpcmMem[1+i*8]=(sampleOffVOX[i]>>8)&0xff;
      adpcmMem[2+i*8]=(sampleOffVOX[i])&0xff;
      adpcmMem[3+i*8]=(endPos>>16)&0xff;
      adpcmMem[4+i*8]=(endPos>>8)&0xff;
      adpcmMem[5+i*8]=(endPos)&0xff;
    }
  }
}

void DivPlatformMSM6295::setFlags(const DivConfig& flags) {
  rateSelInit=flags.getBool("rateSel",false);
  isBanked=flags.getBool("isBanked",false);
  switch (flags.getInt("clockSel",0)) {
    case 1:
      chipClock=4224000/4;
      break;
    case 2:
      chipClock=4000000;
      break;
    case 3:
      chipClock=4224000;
      break;
    case 4:
      chipClock=COLOR_NTSC;
      break;
    case 5:
      chipClock=COLOR_NTSC/2.0;
      break;
    case 6:
      chipClock=COLOR_NTSC*2.0/7.0;
      break;
    case 7:
      chipClock=COLOR_NTSC/4.0;
      break;
    case 8:
      chipClock=4000000/2;
      break;
    case 9:
      chipClock=4224000/2;
      break;
    case 10:
      chipClock=875000;
      break;
    case 11:
      chipClock=937500;
      break;
    case 12:
      chipClock=1500000;
      break;
    case 13:
      chipClock=3000000;
      break;
    case 14:
      chipClock=COLOR_NTSC/3.0;
      break;
    default:
      chipClock=4000000/4;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/3;
  for (int i=0; i<4; i++) {
    oscBuf[i]->rate=rate/22;
  }
  if (rateSel!=rateSelInit) {
    rWrite(12,!rateSelInit);
    rateSel=rateSelInit;
  }
  rWrite(14,isBanked);
}

int DivPlatformMSM6295::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  adpcmMem=new unsigned char[16777216];
  adpcmMemLen=0;
  dumpWrites=false;
  skipRegisterWrites=false;
  updateOsc=0;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformMSM6295::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  delete[] adpcmMem;
}

DivPlatformMSM6295::~DivPlatformMSM6295() {
}
