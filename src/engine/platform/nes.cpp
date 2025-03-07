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

#include "nes.h"
#include "sound/nes/cpu_inline.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <stddef.h>
#include <math.h>

struct _nla_table nla_table;

#define CHIP_DIVIDER 16

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite((a),v)); if (dumpWrites) {addWrite((a),v);} }

const char* regCheatSheetNES[]={
  "S0Volume", "4000",
  "S0Sweep", "4001",
  "S0PeriodL", "4002",
  "S0PeriodH", "4003",
  "S1Volume", "4004",
  "S1Sweep", "4005",
  "S1PeriodL", "4006",
  "S1PeriodH", "4007",
  "TRVolume", "4008",
  "TRPeriodL", "400A",
  "TRPeriodH", "400B",
  "NSVolume", "400C",
  "NSPeriod", "400E",
  "NSLength", "400F",
  "DMCControl", "4010",
  "DMCLoad", "4011",
  "DMCAddr", "4012",
  "DMCLength", "4013",
  "APUControl", "4015",
  "APUFrameCtl", "4017",
  NULL
};

unsigned char _readDMC(void* user, unsigned short addr) {
  return ((DivPlatformNES*)user)->readDMC(addr);
}

const char** DivPlatformNES::getRegisterSheet() {
  return regCheatSheetNES;
}

void DivPlatformNES::doWrite(int ts, unsigned short addr, unsigned char data) {
  if (useNP) {
    if (isE) {
      e1_NP->Write(addr,data);
      e2_NP->Write(addr,data);
    } else {
      nes1_NP->Write(addr,data);
      nes2_NP->Write(addr,data);
    }
  } else {
    apu_wr_reg(nes,ts,addr,data);
  }
}

#define doPCM \
  if (!dpcmMode && dacSample!=-1) { \
    dacPeriod+=dacRate*pcmAdvance; \
    if (dacPeriod>=rate) { \
      DivSample* s=parent->getSample(dacSample); \
      if (s->samples>0 && dacPos<s->samples) { \
        if (!isMuted[4]) { \
          unsigned char next=((unsigned char)s->data8[dacPos]+0x80)>>1; \
          if (dacAntiClickOn && dacAntiClick<next) { \
            dacAntiClick+=8; \
            doWrite(i,0x4011,dacAntiClick); \
          } else { \
            dacAntiClickOn=false; \
            doWrite(i,0x4011,next); \
          } \
        } \
        dacPos++; \
        if (s->isLoopable() && dacPos>=(unsigned int)s->loopEnd) { \
          dacPos=s->loopStart; \
        } else if (dacPos>=s->samples) { \
          dacSample=-1; \
        } \
        dacPeriod-=rate; \
      } else { \
        dacSample=-1; \
      } \
    } \
  }

void DivPlatformNES::acquire_puNES(blip_buffer_t** bb, size_t len) {
  for (int i=0; i<5; i++) {
    oscBuf[i]->begin(len);
    nes->oscBuf[i]=oscBuf[i];
  }

  nes->timestamp=0;
  nes->bb=bb[0];

  for (size_t i=0; i<len; i++) {
    // heuristic
    int pcmAdvance=1;
    if (writes.empty()) {
      if (dpcmMode || dacSample==-1) {
        break;
      } else {
        pcmAdvance=len-i;
        if (dacPeriod>0) {
          int remainTime=(rate-dacPeriod+dacRate-1)/dacRate;
          if (remainTime<pcmAdvance) pcmAdvance=remainTime;
          if (remainTime<1) pcmAdvance=1;
        }
      }
    }

    if (!writes.empty()) {
      pcmAdvance=1;
      QueuedWrite w=writes.front();
      doWrite(i,w.addr,w.val);
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }

    i+=pcmAdvance-1;

    doPCM;
  }
  apu_tick(nes,len);

  for (int i=0; i<5; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformNES::acquire_NSFPlay(short** buf, size_t len) {
  int out1[2];
  int out2[2];
  const int pcmAdvance=1;

  for (int i=0; i<5; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t i=0; i<len; i++) {
    doPCM;

    if (!writes.empty()) {
      QueuedWrite w=writes.front();
      doWrite(i,w.addr,w.val);
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }
  
    nes1_NP->Tick(8);
    nes2_NP->TickFrameSequence(8);
    nes2_NP->Tick(8);
    nes1_NP->Render(out1);
    nes2_NP->Render(out2);

    int sample=(out1[0]+out1[1]+out2[0]+out2[1])<<1;
    if (sample>32767) sample=32767;
    if (sample<-32768) sample=-32768;
    buf[0][i]=sample;
    if (++writeOscBuf>=4) {
      writeOscBuf=0;
      oscBuf[0]->putSample(i,nes1_NP->out[0]<<11);
      oscBuf[1]->putSample(i,nes1_NP->out[1]<<11);
      oscBuf[2]->putSample(i,nes2_NP->out[0]<<11);
      oscBuf[3]->putSample(i,nes2_NP->out[1]<<11);
      oscBuf[4]->putSample(i,nes2_NP->out[2]<<8);
    }
  }

  for (int i=0; i<5; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformNES::acquire_NSFPlayE(short** buf, size_t len) {
  int out1[2];
  int out2[2];
  const int pcmAdvance=1;

  for (int i=0; i<5; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t i=0; i<len; i++) {
    doPCM;

    if (!writes.empty()) {
      QueuedWrite w=writes.front();
      doWrite(i,w.addr,w.val);
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }
  
    e1_NP->Tick(8);
    e2_NP->TickFrameSequence(8);
    e2_NP->Tick(8);
    e1_NP->Render(out1);
    e2_NP->Render(out2);

    int sample=(out1[0]+out1[1]+out2[0]+out2[1])<<1;
    if (sample>32767) sample=32767;
    if (sample<-32768) sample=-32768;
    buf[0][i]=sample;
    if (++writeOscBuf>=4) {
      writeOscBuf=0;
      oscBuf[0]->putSample(i,e1_NP->out[0]<<11);
      oscBuf[1]->putSample(i,e1_NP->out[1]<<11);
      oscBuf[2]->putSample(i,e2_NP->out[0]<<11);
      oscBuf[3]->putSample(i,e2_NP->out[1]<<11);
      oscBuf[4]->putSample(i,e2_NP->out[2]<<8);
    }
  }

  for (int i=0; i<5; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformNES::acquire(short** buf, size_t len) {
  if (!useNP) return;
  if (isE) {
    acquire_NSFPlayE(buf,len);
  } else {
    acquire_NSFPlay(buf,len);
  }
}

void DivPlatformNES::acquireDirect(blip_buffer_t** bb, size_t len) {
  if (useNP) return;
  acquire_puNES(bb,len);
}

static unsigned char noiseTable[253]={
  6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4,
  3, 2, 1, 0, 11, 10, 9, 8, 7, 6, 5, 4,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  15
};

unsigned char DivPlatformNES::calcDPCMRate(int inRate) {
  if (inRate<4450) return 0;
  if (inRate<5000) return 1;
  if (inRate<5400) return 2;
  if (inRate<5900) return 3;
  if (inRate<6650) return 4;
  if (inRate<7450) return 5;
  if (inRate<8100) return 6;
  if (inRate<8800) return 7;
  if (inRate<10200) return 8;
  if (inRate<11700) return 9;
  if (inRate<13300) return 10;
  if (inRate<15900) return 11;
  if (inRate<18900) return 12;
  if (inRate<23500) return 13;
  if (inRate<29000) return 14;
  return 15;
}

void DivPlatformNES::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      // ok, why are the volumes like that?
      chan[i].outVol=VOL_SCALE_LINEAR_BROKEN(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (i==2) { // triangle
        rWrite(0x4000+i*4,(chan[i].outVol==0)?0:linearCount);
        chan[i].freqChanged=true;
      } else {
        rWrite(0x4000+i*4,(chan[i].envMode<<4)|chan[i].outVol|((chan[i].duty&3)<<6));
      }
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (i==3) { // noise
        chan[i].baseFreq=parent->calcArp(chan[i].note,chan[i].std.arp.val);
        if (chan[i].baseFreq>255) chan[i].baseFreq=255;
        if (chan[i].baseFreq<0) chan[i].baseFreq=0;
      } else {
        if (!chan[i].inPorta) {
          chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
        }
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      if (i==3) {
        if (parent->song.properNoiseLayout) {
          chan[i].duty&=1;
        } else if (chan[i].duty>1) {
          chan[i].duty=1;
        }
      }
      if (i!=2) {
        rWrite(0x4000+i*4,(chan[i].envMode<<4)|chan[i].outVol|((chan[i].duty&3)<<6));
      } else if (isE) {
        rWrite(0x4000+9,chan[i].duty);
      }
      if (i==3) { // noise
        chan[i].freqChanged=true;
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
    if (chan[i].sweepChanged) {
      chan[i].sweepChanged=false;
      if (i==0) {
        // rWrite(16+i*5,chan[i].sweep);
      }
    }
    if (i<3) if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].freqChanged=true;
        chan[i].prevFreq=-1;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (i==3) { // noise
        int ntPos=chan[i].baseFreq;
        if (NEW_ARP_STRAT) {
          if (chan[i].fixedArp) {
            ntPos=chan[i].baseNoteOverride;
          } else {
            ntPos+=chan[i].arpOff;
          }
        }
        ntPos+=chan[i].pitch2;
        if (isE) {
          chan[i].freq=31-(ntPos&31);
        } else if (parent->song.properNoiseLayout) {
          chan[i].freq=15-(ntPos&15);
        } else {
          if (ntPos<0) ntPos=0;
          if (ntPos>252) ntPos=252;
          chan[i].freq=noiseTable[ntPos];
        }
      } else {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
        if (chan[i].freq>2047) chan[i].freq=2047;
        if (chan[i].freq<0) chan[i].freq=0;
      }
      if (chan[i].keyOn) {
        // retrigger if sweep is on
        if (chan[i].sweep!=0x08) {
          chan[i].prevFreq=-1;
        }
      }
      if (chan[i].keyOff) {
        // rWrite(16+i*5+2,8);
        if (i==2) { // triangle
          rWrite(0x4000+i*4,0x00);
        } else {
          rWrite(0x4000+i*4,0x30);
        }
      }
      if (i==3) { // noise
        rWrite(0x4002+i*4,(chan[i].duty<<7)|chan[i].freq);
        rWrite(0x4003+i*4,(chan[i].len<<3));
      } else {
        rWrite(0x4002+i*4,chan[i].freq&0xff);
        if ((chan[i].prevFreq>>8)!=(chan[i].freq>>8) || i==2) {
          rWrite(0x4003+i*4,(chan[i].len<<3)|(chan[i].freq>>8));
        }
        if (chan[i].freq!=65535 && chan[i].freq!=0) {
          chan[i].prevFreq=chan[i].freq;
        }
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  // PCM
  if (chan[4].freqChanged || chan[4].keyOn) {
    chan[4].freq=parent->calcFreq(chan[4].baseFreq,chan[4].pitch,chan[4].fixedArp?chan[4].baseNoteOverride:chan[4].arpOff,chan[4].fixedArp,false);
    if (chan[4].furnaceDac) {
      double off=1.0;
      if (dacSample>=0 && dacSample<parent->song.sampleLen) {
        DivSample* s=parent->getSample(dacSample);
        off=(double)s->centerRate/parent->getCenterRate();
      }
      dacRate=MIN(chan[4].freq*off,32000);
      if (chan[4].keyOn) {
        if (dpcmMode && !skipRegisterWrites && dacSample>=0 && dacSample<parent->song.sampleLen) {
          unsigned int dpcmAddr=sampleOffDPCM[dacSample]+(dacPos>>3);
          int dpcmLen=(parent->getSample(dacSample)->lengthDPCM-(dacPos>>3))>>4;
          if (dpcmLen<0) dpcmLen=0;
          if (dpcmLen>255) dpcmLen=255;
          goingToLoop=parent->getSample(dacSample)->isLoopable();
          // write DPCM
          rWrite(0x4015,15);
          if (nextDPCMFreq>=0) {
            rWrite(0x4010,nextDPCMFreq|(goingToLoop?0x40:0));
            nextDPCMFreq=-1;
          } else {
            rWrite(0x4010,calcDPCMRate(dacRate)|(goingToLoop?0x40:0));
          }
          if (nextDPCMDelta>=0) {
            rWrite(0x4011,nextDPCMDelta);
            nextDPCMDelta=-1;
          }
          rWrite(0x4012,(dpcmAddr>>6)&0xff);
          rWrite(0x4013,dpcmLen&0xff);
          rWrite(0x4015,31);
          if (dpcmBank!=(dpcmAddr>>14)) {
            dpcmBank=dpcmAddr>>14;
            logV("switching bank to %d",dpcmBank);
            if (dumpWrites) addWrite(0xffff0004,dpcmBank);
          }

          // sample custom loop point...
          DivSample* lsamp=parent->getSample(dacSample);

          // how it works:
          // when the initial sample info is written (see above) and playback is launched,
          // the parameters (start point in memory and length) are locked until sample end
          // is reached.

          // thus, if we write new data after just several APU clock cycles, it will be used only when
          // sample finishes one full loop.

          // thus we can write sample's loop point as "start address" and sample's looped part length
          // as "full sample length".

          // APU will play full sample once and then repeatedly cycle through the looped part.

          // sources:
          // https://www.nesdev.org/wiki/APU_DMC
          // https://www.youtube.com/watch?v=vB4P8x2Am6Y

          if (lsamp->loopEnd>lsamp->loopStart && goingToLoop) {
            int loopStartAddr=sampleOffDPCM[dacSample]+(lsamp->loopStart>>3);
            int loopLen=(lsamp->loopEnd-lsamp->loopStart)>>3;

            rWrite(0x4012,(loopStartAddr>>6)&0xff);
            rWrite(0x4013,(loopLen>>4)&0xff);
          }
        }
      } else {
        if (nextDPCMFreq>=0) {
          rWrite(0x4010,nextDPCMFreq|(goingToLoop?0x40:0));
          nextDPCMFreq=-1;
        } else {
          rWrite(0x4010,calcDPCMRate(dacRate)|(goingToLoop?0x40:0));
        }
      }
      if (dumpWrites && !dpcmMode) addWrite(0xffff0001,dacRate);
    }
    if (chan[4].keyOn) chan[4].keyOn=false;
    chan[4].freqChanged=false;
  }

  nextDPCMFreq=-1;
}

int DivPlatformNES::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.chan==4) { // PCM
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_NES);
        if (ins->type==DIV_INS_AMIGA || (ins->type==DIV_INS_NES && !parent->song.oldDPCM)) {
          if (ins->type==DIV_INS_NES) {
            if (!dpcmMode) {
              dpcmMode=true;
              if (dumpWrites) addWrite(0xffff0002,0);
              dacSample=-1;
              rWrite(0x4015,15);
              rWrite(0x4010,0);
              rWrite(0x4012,0);
              rWrite(0x4013,0);
              rWrite(0x4015,31);
            }

            if (ins->amiga.useNoteMap) {
              nextDPCMFreq=ins->amiga.getDPCMFreq(c.value);
              if (nextDPCMFreq<0 || nextDPCMFreq>15) nextDPCMFreq=lastDPCMFreq;
              lastDPCMFreq=nextDPCMFreq;
              nextDPCMDelta=ins->amiga.getDPCMDelta(c.value);
            } else {
              if (c.value==DIV_NOTE_NULL) {
                nextDPCMFreq=lastDPCMFreq;
              } else {
                nextDPCMFreq=c.value&15;
              }
            }
          }
          if (c.value!=DIV_NOTE_NULL) {
            dacSample=(int)ins->amiga.getSample(c.value);
            if (ins->type==DIV_INS_AMIGA) {
              chan[c.chan].sampleNote=c.value;
              c.value=ins->amiga.getFreq(c.value);
              chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
            }
          } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
            dacSample=(int)ins->amiga.getSample(chan[c.chan].sampleNote);
            if (ins->type==DIV_INS_AMIGA) {
              c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
            }
          }
          if (dacSample<0 || dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            if (dumpWrites && !dpcmMode) addWrite(0xffff0002,0);
            break;
          } else {
            if (dumpWrites && !dpcmMode) addWrite(0xffff0000,dacSample);
          }
          if (chan[c.chan].setPos) {
            chan[c.chan].setPos=false;
          } else {
            dacPos=0;
          }
          dacPeriod=0;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value,false);
            chan[c.chan].freqChanged=true;
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].keyOn=true;
          chan[c.chan].furnaceDac=true;
        } else {
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          dacSample=12*sampleBank+chan[c.chan].note%12;
          if (dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            if (dumpWrites && !dpcmMode) addWrite(0xffff0002,0);
            break;
          } else {
            if (dumpWrites && !dpcmMode) addWrite(0xffff0000,dacSample);
          }
          if (chan[c.chan].setPos) {
            chan[c.chan].setPos=false;
          } else {
            dacPos=0;
          }
          dacPeriod=0;
          dacRate=parent->getSample(dacSample)->rate;
          if (dumpWrites && !dpcmMode) addWrite(0xffff0001,dacRate);
          chan[c.chan].furnaceDac=false;
          if (dpcmMode && !skipRegisterWrites) {
            unsigned int dpcmAddr=sampleOffDPCM[dacSample]+(dacPos>>3);
            int dpcmLen=(parent->getSample(dacSample)->lengthDPCM-(dacPos>>3))>>4;
            if (dpcmLen<0) dpcmLen=0;
            if (dpcmLen>255) dpcmLen=255;
            goingToLoop=parent->getSample(dacSample)->isLoopable();
            // write DPCM
            rWrite(0x4015,15);
            if (nextDPCMFreq>=0) {
              rWrite(0x4010,nextDPCMFreq|(goingToLoop?0x40:0));
              nextDPCMFreq=-1;
            } else {
              rWrite(0x4010,calcDPCMRate(dacRate)|(goingToLoop?0x40:0));
            }
            rWrite(0x4012,(dpcmAddr>>6)&0xff);
            rWrite(0x4013,dpcmLen&0xff);
            rWrite(0x4015,31);
            if (dpcmBank!=(dpcmAddr>>14)) {
              dpcmBank=dpcmAddr>>14;
              logV("switching bank to %d",dpcmBank);
              if (dumpWrites) addWrite(0xffff0004,dpcmBank);
            }
          }
        }
        break;
      } else if (c.chan==3) { // noise
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=c.value;
        }
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_NES));
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (c.chan==2) {
        rWrite(0x4000+c.chan*4,linearCount);
      } else if (!parent->song.brokenOutVol2) {
        rWrite(0x4000+c.chan*4,(chan[c.chan].envMode<<4)|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
      }
      break;
    case DIV_CMD_NOTE_OFF:
      if (c.chan==4) {
        dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
        if (dpcmMode && !skipRegisterWrites) rWrite(0x4015,15);
      }
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
        }
        if (chan[c.chan].active) {
          if (c.chan==2) {
            rWrite(0x4000+c.chan*4,linearCount);
          } else {
            rWrite(0x4000+c.chan*4,(chan[c.chan].envMode<<4)|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=(c.chan==4)?(parent->calcBaseFreq(1,1,c.value2+chan[c.chan].sampleNoteDelta,false)):(NOTE_PERIODIC(c.value2));
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
      if (c.chan==3) { // noise
        chan[c.chan].freqChanged=true;
      } else if (c.chan==2 && isE) {
        rWrite(0x4000+9,chan[c.chan].duty);
      } else if (c.chan<2) {
        rWrite(0x4000+c.chan*4,(chan[c.chan].active?((chan[c.chan].envMode<<4)|chan[c.chan].outVol):0x30)|((chan[c.chan].duty&3)<<6));
      }
      break;
    case DIV_CMD_NES_SWEEP:
      if (c.chan>1) break;
      if (c.value2==0) {
        chan[c.chan].sweep=0x08;
      } else {
        if (!c.value) { // down
          chan[c.chan].sweep=0x88|(c.value2&0x77);
        } else { // up
          chan[c.chan].sweep=0x80|(c.value2&0x77);
        }
      }
      rWrite(0x4001+(c.chan*4),chan[c.chan].sweep);
      break;
    case DIV_CMD_NES_ENV_MODE:
      chan[c.chan].envMode=c.value&3;
      if (c.chan==3) { // noise
        chan[c.chan].freqChanged=true;
      } else if (c.chan<2) {
        rWrite(0x4000+c.chan*4,(chan[c.chan].active?((chan[c.chan].envMode<<4)|chan[c.chan].outVol):0x30)|((chan[c.chan].duty&3)<<6));
      }
      break;
    case DIV_CMD_NES_LENGTH:
      if (c.chan>=4) break;
      chan[c.chan].len=c.value&0x1f;
      chan[c.chan].freqChanged=true;
      chan[c.chan].prevFreq=-1;
      break;
    case DIV_CMD_NES_COUNT_MODE:
      countMode=c.value;
      rWrite(0x4017,countMode?0x80:0);
      break;
    case DIV_CMD_NES_LINEAR_LENGTH:
      if (c.chan==2) {
        linearCount=c.value;
        if (chan[c.chan].active) {
          rWrite(0x4000+c.chan*4,(chan[c.chan].outVol==0)?0:linearCount);
          chan[c.chan].freqChanged=true;
          chan[c.chan].prevFreq=-1;
        }
      }
      break;
    case DIV_CMD_NES_DMC:
      rWrite(0x4011,c.value&0x7f);
      break;
    case DIV_CMD_SAMPLE_MODE:
      dpcmMode=c.value;
      if (dumpWrites && dpcmMode) addWrite(0xffff0002,0);
      dacSample=-1;
      rWrite(0x4015,15);
      rWrite(0x4010,0);
      rWrite(0x4012,0);
      rWrite(0x4013,0);
      rWrite(0x4015,31);
      break;
    case DIV_CMD_SAMPLE_FREQ: {
      bool goingToLoop=parent->getSample(dacSample)->isLoopable();
      if (dpcmMode) {
        nextDPCMFreq=c.value&15;
        rWrite(0x4010,(c.value&15)|(goingToLoop?0x40:0));
      }
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_SAMPLE_POS:
      if (c.chan!=4) break;
      dacPos=c.value;
      dpcmPos=c.value;
      chan[c.chan].setPos=true;
      if (chan[c.chan].active) {
        chan[c.chan].keyOn=true;
      }
      break;
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      if (c.chan==4) {
        chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)),false);
      } else {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      }
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_NES));
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

void DivPlatformNES::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (useNP) {
    if (isE) {
      e1_NP->SetMask(((int)isMuted[0])|(isMuted[1]<<1));
      e2_NP->SetMask(((int)isMuted[2])|(isMuted[3]<<1)|(isMuted[4]<<2));
    } else {
      nes1_NP->SetMask(((int)isMuted[0])|(isMuted[1]<<1));
      nes2_NP->SetMask(((int)isMuted[2])|(isMuted[3]<<1)|(isMuted[4]<<2));
    }
  } else {
    nes->muted[ch]=mute;
  }
}

void DivPlatformNES::forceIns() {
  for (int i=0; i<5; i++) {
    chan[i].insChanged=true;
    chan[i].prevFreq=65535;
  }
  rWrite(0x4001,chan[0].sweep);
  rWrite(0x4005,chan[1].sweep);
  rWrite(0x4017,countMode?0x80:0);
}

void* DivPlatformNES::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformNES::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformNES::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformNES::getRegisterPool() {
  return regPool;
}

int DivPlatformNES::getRegisterPoolSize() {
  return 32;
}

float DivPlatformNES::getPostAmp() {
  return 2.0f;
}

void DivPlatformNES::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<5; i++) {
    chan[i]=DivPlatformNES::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  dacPeriod=0;
  dacPos=0;
  dpcmPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;
  dpcmBank=0;
  dpcmMode=dpcmModeDefault;
  goingToLoop=false;
  countMode=false;
  nextDPCMFreq=-1;
  nextDPCMDelta=-1;
  lastDPCMFreq=15;
  linearCount=255;

  if (useNP) {
    if (isE) {
      e1_NP->Reset();
      e2_NP->Reset();
      e1_NP->SetMask(((int)isMuted[0])|(isMuted[1]<<1));
      e2_NP->SetMask(((int)isMuted[2])|(isMuted[3]<<1)|(isMuted[4]<<2));
    } else {
      nes1_NP->Reset();
      nes2_NP->Reset();
      nes1_NP->SetMask(((int)isMuted[0])|(isMuted[1]<<1));
      nes2_NP->SetMask(((int)isMuted[2])|(isMuted[3]<<1)|(isMuted[4]<<2));
    }
  } else {
    apu_turn_on(nes,apuType);
    nes->apu.cpu_cycles=0;
    nes->apu.cpu_opcode_cycle=0;
  }
  memset(regPool,0,128);

  rWrite(0x4015,0x1f);
  rWrite(0x4001,chan[0].sweep);
  rWrite(0x4005,chan[1].sweep);

  dacAntiClickOn=true;
  dacAntiClick=0;
}

bool DivPlatformNES::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformNES::hasAcquireDirect() {
  return (!useNP && !isE);
}

void DivPlatformNES::setFlags(const DivConfig& flags) {
  int clockSel=flags.getInt("clockSel",0);
  if (clockSel==2) { // Dendy
    chipClock=COLOR_PAL*2.0/5.0;
    apuType=2;
  } else if (clockSel==1) { // PAL
    chipClock=COLOR_PAL*3.0/8.0;
    apuType=1;
  } else { // NTSC
    chipClock=COLOR_NTSC/2.0;
    apuType=0;
  }
  if (useNP) {
    if (isE) {
      e1_NP->SetClock(rate);
      e1_NP->SetRate(rate);
      e2_NP->SetClock(rate);
      e2_NP->SetRate(rate);
      e2_NP->SetPal(apuType==1);
    } else {
      nes1_NP->SetClock(rate);
      nes1_NP->SetRate(rate);
      nes2_NP->SetClock(rate);
      nes2_NP->SetRate(rate);
      nes2_NP->SetPal(apuType==1);
    }
  } else {
    nes->apu.type=apuType;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  if (useNP) {
    rate/=8;
  }
  for (int i=0; i<5; i++) {
    oscBuf[i]->setRate(rate);
  }
  
  dpcmModeDefault=flags.getBool("dpcmMode",true);
}

void DivPlatformNES::notifyInsDeletion(void* ins) {
  for (int i=0; i<5; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformNES::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformNES::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformNES::setNSFPlay(bool use) {
  useNP=use;
}

void DivPlatformNES::set5E01(bool use) {
  isE=use;
  // for now
  if (isE) useNP=true;
}

unsigned char DivPlatformNES::readDMC(unsigned short addr) {
  return dpcmMem[(addr&0x3fff)|((dpcmBank&15)<<14)];
}

const void* DivPlatformNES::getSampleMem(int index) {
  return index==0?dpcmMem:NULL;
}

size_t DivPlatformNES::getSampleMemCapacity(int index) {
  return index==0?262144:0;
}

size_t DivPlatformNES::getSampleMemUsage(int index) {
  return index==0?dpcmMemLen:0;
}

bool DivPlatformNES::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformNES::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformNES::renderSamples(int sysID) {
  memset(dpcmMem,0,getSampleMemCapacity(0));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="DPCM";

  size_t memPos=0;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOffDPCM[i]=0;
      continue;
    }

    unsigned int paddedLen=(s->lengthDPCM+63)&(~0x3f);
    logV("%d padded length: %d",i,paddedLen);
    if ((memPos&(~0x3fff))!=((memPos+paddedLen)&(~0x3fff))) {
      memPos=(memPos+0x3fff)&(~0x3fff);
    }
    if (paddedLen>4081) {
      paddedLen=4096;
    }
    if (memPos>=getSampleMemCapacity(0)) {
      logW("out of DPCM memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=getSampleMemCapacity(0)) {
      memcpy(dpcmMem+memPos,s->dataDPCM,getSampleMemCapacity(0)-memPos);
      logW("out of DPCM memory for sample %d!",i);
    } else {
      memcpy(dpcmMem+memPos,s->dataDPCM,MIN(s->lengthDPCM,paddedLen));
      sampleLoaded[i]=true;
    }
    sampleOffDPCM[i]=memPos;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+paddedLen));
    memPos+=paddedLen;
  }
  dpcmMemLen=memPos;

  memCompo.capacity=262144;
  memCompo.used=dpcmMemLen;
}

int DivPlatformNES::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  if (useNP) {
    if (isE) {
      e1_NP=new xgm::I5E01_APU;
      e1_NP->SetOption(xgm::I5E01_APU::OPT_NONLINEAR_MIXER,1);
      e2_NP=new xgm::I5E01_DMC;
      e2_NP->SetOption(xgm::I5E01_DMC::OPT_NONLINEAR_MIXER,1);
      e2_NP->SetMemory([this](unsigned short addr, unsigned int& data) {
        data=readDMC(addr);
      });
      e2_NP->SetAPU(e1_NP);
    } else {
      nes1_NP=new xgm::NES_APU;
      nes1_NP->SetOption(xgm::NES_APU::OPT_NONLINEAR_MIXER,1);
      nes2_NP=new xgm::NES_DMC;
      nes2_NP->SetOption(xgm::NES_DMC::OPT_NONLINEAR_MIXER,1);
      nes2_NP->SetMemory([this](unsigned short addr, unsigned int& data) {
        data=readDMC(addr);
      });
      nes2_NP->SetAPU(nes1_NP);
    }
  } else {
    nes=new struct NESAPU;
    nes->readDMC=_readDMC;
    nes->readDMCUser=this;
  }
  writeOscBuf=0;
  for (int i=0; i<5; i++) {
    isMuted[i]=false;
    if (!useNP) nes->muted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  dpcmMem=new unsigned char[262144];
  dpcmMemLen=0;
  dpcmBank=0;
  if (dumpWrites) addWrite(0xffff0004,dpcmBank);

  init_nla_table(500,500);
  reset();
  return 5;
}

void DivPlatformNES::quit() {
  for (int i=0; i<5; i++) {
    delete oscBuf[i];
  }
  if (useNP) {
    if (isE) {
      delete e1_NP;
      delete e2_NP;
    } else {
      delete nes1_NP;
      delete nes2_NP;
    }
  } else {
    delete nes;
  }
}

DivPlatformNES::~DivPlatformNES() {
}
