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

#include "c140.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define CHIP_FREQBASE (is219?74448896:12582912)

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

const char* regCheatSheetC219[]={
  "CHx_RVol", "00+x*10",
  "CHx_LVol", "01+x*10",
  "CHx_FreqH", "02+x*10",
  "CHx_FreqL", "03+x*10",
  "CHx_Ctrl", "05+x*10",
  "CHx_StartH", "06+x*10",
  "CHx_StartL", "07+x*10",
  "CHx_EndH", "08+x*10",
  "CHx_EndL", "09+x*10",
  "CHx_LoopH", "0A+x*10",
  "CHx_LoopL", "0B+x*10",
  "BankA", "1F7",
  "BankB", "1F1",
  "BankC", "1F3",
  "BankD", "1F5",
  NULL
};

const char** DivPlatformC140::getRegisterSheet() {
  return is219?regCheatSheetC219:regCheatSheetC140;
}

void DivPlatformC140::acquire_219(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      c219_write(&c219,w.addr,w.val);
      regPool[w.addr&0x1ff]=w.val;
      writes.pop();
    }

    c219_tick(&c219,1);
    // scale as 16bit
    c219.lout>>=10;
    c219.rout>>=10;

    if (c219.lout<-32768) c219.lout=-32768;
    if (c219.lout>32767) c219.lout=32767;

    if (c219.rout<-32768) c219.rout=-32768;
    if (c219.rout>32767) c219.rout=32767;
  
    buf[0][h]=c219.lout;
    buf[1][h]=c219.rout;

    for (int i=0; i<totalChans; i++) {
      if (c219.voice[i].inv_lout) {
        oscBuf[i]->data[oscBuf[i]->needle++]=(c219.voice[i].lout-c219.voice[i].rout)>>10;
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=(c219.voice[i].lout+c219.voice[i].rout)>>10;
      }
    }
  }
}

void DivPlatformC140::acquire_140(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      c140_write(&c140,w.addr,w.val);
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

    for (int i=0; i<totalChans; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(c140.voice[i].lout+c140.voice[i].rout)>>10;
    }
  }
}

void DivPlatformC140::acquire(short** buf, size_t len) {
  if (is219) {
    acquire_219(buf,len);
  } else {
    acquire_140(buf,len);
  }
}

void DivPlatformC140::tick(bool sysTick) {
  for (int i=0; i<totalChans; i++) {
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
    if (is219) {
      if (chan[i].std.duty.had) {
        unsigned char singleByte=(
          (chan[i].noise?1:0)|
          (chan[i].invert?2:0)|
          (chan[i].surround?4:0)
        );
        if (singleByte!=(chan[i].std.duty.val&7)) {
          chan[i].noise=chan[i].std.duty.val&1;
          chan[i].invert=chan[i].std.duty.val&2;
          chan[i].surround=chan[i].std.duty.val&4;
          chan[i].freqChanged=true;
          chan[i].writeCtrl=true;
        }
      }
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
      DivSample* s=parent->getSample(chan[i].sample);
      unsigned char ctrl=0;
      double off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
      chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      if (is219) {
        ctrl|=(chan[i].active?0x80:0)|((s->isLoopable() || chan[i].noise)?0x10:0)|((s->depth==DIV_SAMPLE_DEPTH_C219)?1:0)|(chan[i].invert?0x40:0)|(chan[i].surround?8:0)|(chan[i].noise?4:0);
      } else {
        ctrl|=(chan[i].active?0x80:0)|((s->isLoopable())?0x10:0)|((s->depth==DIV_SAMPLE_DEPTH_MULAW)?0x08:0);
      }
      if (chan[i].keyOn) {
        unsigned int bank=0;
        unsigned int start=0;
        unsigned int loop=0;
        unsigned int end=0;
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          if (is219) {
            bank=(sampleOff[chan[i].sample]>>16)&3;
            start=sampleOff[chan[i].sample]&0xffff;
            end=MIN(start+(s->length8>>1)-1,65535);
          } else {
            bank=(sampleOff[chan[i].sample]>>16)&0xff;
            start=sampleOff[chan[i].sample]&0xffff;
            end=MIN(start+s->length8-1,65535);
          }
        } else if (chan[i].noise && is219) {
          bank=groupBank[i>>2];
          start=0;
          end=1;
        }
        if (chan[i].audPos>0) {
          start=MIN(start+(MIN(chan[i].audPos,s->length8)>>1),65535);
        }
        if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen && s->isLoopable()) {
          if (is219) {
            loop=MIN(start+(s->loopStart>>1),65535);
            end=MIN(start+(s->loopEnd>>1),65535);
          } else {
            loop=MIN(start+s->loopStart+1,65535);
            end=MIN(start+s->loopEnd+1,65535);
          }
        } else if (chan[i].noise && is219) {
          loop=0;
        }
        rWrite(0x05+(i<<4),0); // force keyoff first
        if (is219) {
          if (groupBank[i>>2]!=bank) {
            groupBank[i>>2]=bank;
            rWrite(0x1f1+(((3+(i>>2))&3)<<1),groupBank[i>>2]);
            // shut everyone else up
            for (int j=0; j<4; j++) {
              int ch=(i&(~3))|j;
              if (chan[ch].active && !chan[ch].keyOn && (i&3)!=j) {
                chan[ch].sample=-1;
                chan[ch].active=false;
                chan[ch].keyOff=true;
                chan[ch].macroInit(NULL);
                rWrite(0x05+(ch<<4),ctrl);
              }
            }
          }
        } else {
          switch (bankType) {
            case 0:
              bank=((bank&8)<<2)|(bank&7);
              break;
            case 1:
              bank=((bank&0x18)<<1)|(bank&7);
              break;
          }
          rWrite(0x04+(i<<4),bank);
        }
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
        chan[i].writeCtrl=true;
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chan[i].writeCtrl=true;
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        rWrite(0x02+(i<<4),chan[i].freq>>8);
        rWrite(0x03+(i<<4),chan[i].freq&0xff);
        chan[i].freqChanged=false;
      }
      if (chan[i].writeCtrl) {
        rWrite(0x05+(i<<4),ctrl);
        chan[i].writeCtrl=false;
      }
    }
  }

  for (int i=0; i<4; i++) {
    bankLabel[i][0]='0'+groupBank[i];
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
    case DIV_CMD_STD_NOISE_MODE:
      if (!is219) break;
      chan[c.chan].noise=c.value;
      chan[c.chan].writeCtrl=true;
      break;
    case DIV_CMD_SNES_INVERT:
      if (!is219) break;
      chan[c.chan].invert=c.value&15;
      chan[c.chan].surround=c.value>>4;
      chan[c.chan].writeCtrl=true;
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformC140::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (is219) {
    c219.voice[ch].muted=mute;
  } else {
    c140.voice[ch].muted=mute;
  }
}

void DivPlatformC140::forceIns() {
  for (int i=0; i<totalChans; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].volChangedL=true;
    chan[i].volChangedR=true;
    chan[i].sample=-1;
  }
  if (is219) {
    // restore banks
    for (int i=0; i<4; i++) {
      rWrite(0x1f1+(((3+i)&3)<<1),groupBank[i]);
    }
  }
}

void* DivPlatformC140::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformC140::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformC140::getPan(int ch) {
  return (chan[ch].chPanL<<8)|(chan[ch].chPanR);
}

DivDispatchOscBuffer* DivPlatformC140::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformC140::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,512);
  if (is219) {
    c219_reset(&c219);
  } else {
    c140_reset(&c140);
  }
  for (int i=0; i<totalChans; i++) {
    chan[i]=DivPlatformC140::Channel();
    chan[i].std.setEngine(parent);
    rWrite(0x05+(i<<4),0);
  }
  for (int i=0; i<4; i++) {
    groupBank[i]=0;
  }
}

int DivPlatformC140::getOutputCount() {
  return 2;
}

void DivPlatformC140::notifyInsChange(int ins) {
  for (int i=0; i<totalChans; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformC140::notifyWaveChange(int wave) {

}

void DivPlatformC140::notifyInsDeletion(void* ins) {
  for (int i=0; i<totalChans; i++) {
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

void DivPlatformC140::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (!is219) return;
  if ((ch&3)==0) {
    ret.push_back(DivChannelPair(bankLabel[ch>>2],ch+1,ch+2,ch+3,-1,-1,-1,-1,-1));
  }
}

const void* DivPlatformC140::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformC140::getSampleMemCapacity(int index) {
  if (index!=0) return 0;
  if (is219) return 524288;
  switch (bankType) {
    case 0:
      return 2097152;
    case 1:
      return 4194304;
  }
  return 16777216;
}

size_t DivPlatformC140::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformC140::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformC140::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformC140::renderSamples(int sysID) {
  memset(sampleMem,0,is219?524288:16777216);
  memset(sampleOff,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample ROM";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }

    if (is219) { // C219 (8-bit)
      unsigned int length=s->length8+4;
      // fit sample size to single bank size
      if (length>131072) {
        length=131072;
      }
      if (length&1) length++;
      if ((memPos&0xfe0000)!=((memPos+length)&0xfe0000)) {
        memPos=((memPos+0x1ffff)&0xfe0000);
      }
      logV("%d",length);
      if (memPos>=(getSampleMemCapacity())) {
        logW("out of C219 memory for sample %d!",i);
        break;
      }
      if (memPos+length>=(getSampleMemCapacity())) {
        length=getSampleMemCapacity()-memPos;
        logW("out of C219 memory for sample %d!",i);
      }
      if (s->depth==DIV_SAMPLE_DEPTH_C219) {
        unsigned char next=0;
        unsigned int sPos=0;
        for (unsigned int i=0; i<length; i++) {
          if (sPos<s->lengthC219) {
            next=s->dataC219[sPos++];
            if (s->isLoopable()) {
              if ((int)sPos>=s->loopEnd) {
                sPos=s->loopStart;
              }
            }
          }
          sampleMem[(memPos+i)^1]=next;
        }
      } else {
        signed char next=0;
        unsigned int sPos=0;
        for (unsigned int i=0; i<length; i++) {
          if (sPos<s->length8) {
            next=s->data8[sPos++];
            if (s->isLoopable()) {
              if ((int)sPos>=s->loopEnd) {
                sPos=s->loopStart;
              }
            }
          }
          sampleMem[(memPos+i)^1]=next;
        }
      }
      sampleOff[i]=memPos>>1;
      sampleLoaded[i]=true;
      memCompo.entries.push_back(DivMemoryEntry((DivMemoryEntryType)(DIV_MEMORY_BANK0+((memPos>>17)&3)),"Sample",i,memPos,memPos+length));
      memPos+=length;
    } else { // C140 (16-bit)
      unsigned int length=s->length16+4;
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
        length=getSampleMemCapacity()-memPos;
        logW("out of C140 memory for sample %d!",i);
      }
      if (s->depth==DIV_SAMPLE_DEPTH_MULAW) {
        for (unsigned int i=0; i<length; i+=2) {
          if ((i>>1)>=s->lengthMuLaw) break;
          unsigned char x=s->dataMuLaw[i>>1]^0xff;
          if (x&0x80) x^=15;
          unsigned char c140Mu=(x&0x80)|((x&15)<<3)|((x&0x70)>>4);
          sampleMem[i+memPos]=0;
          sampleMem[1+i+memPos]=c140Mu;
        }
      } else {
        short next=0;
        unsigned int sPos=0;
        for (unsigned int i=0; i<length; i+=2) {
          if (sPos<s->samples) {
            next=s->data16[sPos++];
            if (s->isLoopable()) {
              if ((int)sPos>=s->loopEnd) {
                sPos=s->loopStart;
              }
            }
          }
          sampleMem[memPos+i]=((unsigned short)next);
          sampleMem[memPos+i+1]=((unsigned short)next)>>8;
        }
      }
      sampleOff[i]=memPos>>1;
      sampleLoaded[i]=true;
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+length));
      memPos+=length;
    }
  }
  sampleMemLen=memPos+256;

  memCompo.used=sampleMemLen;
  memCompo.capacity=getSampleMemCapacity(0);
}

void DivPlatformC140::set219(bool is_219) {
  is219=is_219;
  totalChans=is219?16:24;
}

int DivPlatformC140::getClockRangeMin() {
  if (is219) return 1000000;
  return MIN_CUSTOM_CLOCK;
}

int DivPlatformC140::getClockRangeMax() {
  if (is219) return 100000000;
  return MAX_CUSTOM_CLOCK;
}

void DivPlatformC140::setFlags(const DivConfig& flags) {
  if (is219) {
    chipClock=50113000; // 50.113MHz clock input in Namco NA-1/NA-2 PCB
    CHECK_CUSTOM_CLOCK;
    rate=chipClock/1136; // assumed as ~44100hz
  } else {
    chipClock=32000*256; // 8.192MHz and 12.288MHz input, verified from Assault Schematics
    CHECK_CUSTOM_CLOCK;
    rate=chipClock/192;
  }
  bankType=flags.getInt("bankType",0);
  if (!is219) {
    c140_bank_type(&c140,bankType);
  }
  for (int i=0; i<totalChans; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformC140::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  bankType=0;

  memset(bankLabel,0,16);

  for (int i=0; i<totalChans; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new unsigned char[is219?524288:16777216];
  sampleMemLen=0;
  if (is219) {
    c219_init(&c219);
    c219.sample_mem=(signed char*)sampleMem;
  } else {
    c140_init(&c140);
    c140.sample_mem=(short*)sampleMem;
  }
  setFlags(flags);
  reset();

  return totalChans;
}

void DivPlatformC140::quit() {
  delete[] sampleMem;
  for (int i=0; i<totalChans; i++) {
    delete oscBuf[i];
  }
}
