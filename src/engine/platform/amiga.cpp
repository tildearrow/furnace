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

#define _USE_MATH_DEFINES
#include "amiga.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define AMIGA_DIVIDER 8
#define AMIGA_VPMASK 7
#define CHIP_DIVIDER 16

#define chWrite(c,a,v) rWrite(((c)<<4)+0xa0+(a),(v));

const char* regCheatSheetAmiga[]={
  "DMACON", "96",
  "INTENA", "9A",
  "ADKCON", "9E",

  "AUD0LCH", "A0",
  "AUD0LCL", "A2",
  "AUD0LEN", "A4",
  "AUD0PER", "A6",
  "AUD0VOL", "A8",
  "AUD0DAT", "AA",

  "AUD1LCH", "B0",
  "AUD1LCL", "B2",
  "AUD1LEN", "B4",
  "AUD1PER", "B6",
  "AUD1VOL", "B8",
  "AUD1DAT", "BA",

  "AUD2LCH", "C0",
  "AUD2LCL", "C2",
  "AUD2LEN", "C4",
  "AUD2PER", "C6",
  "AUD2VOL", "C8",
  "AUD2DAT", "CA",

  "AUD3LCH", "D0",
  "AUD3LCL", "D2",
  "AUD3LEN", "D4",
  "AUD3PER", "D6",
  "AUD3VOL", "D8",
  "AUD3DAT", "DA",
  NULL
};

const char** DivPlatformAmiga::getRegisterSheet() {
  return regCheatSheetAmiga;
}

#define writeAudDat(x) \
  chan[i].audDat=x; \
  if (i<3 && chan[i].useV) { \
    chan[i+1].outVol=(unsigned char)chan[i].audDat^0x80; \
    if (chan[i+1].outVol>64) chan[i+1].outVol=64; \
  } \
  if (i<3 && chan[i].useP) { \
    chan[i+1].freq=(unsigned char)chan[i].audDat^0x80; \
    if (chan[i+1].freq<AMIGA_DIVIDER) chan[i+1].freq=AMIGA_DIVIDER; \
  }

void DivPlatformAmiga::acquire(short** buf, size_t len) {
}

void DivPlatformAmiga::acquireDirect(blip_buffer_t** bb, size_t len) {
  thread_local int outL, outR, output;

  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }

  int runCount=1;
  for (size_t h=0; h<len; h++) {
    // skip heuristic
    runCount=len-h;
    //logI("FRAME START - at most %d",runCount);
    if (delay<runCount) {
      if (!writes.empty()) {
        runCount=delay;
      }
    }
    for (int i=0; i<4; i++) {
      if (!amiga.mustDMA[i] && !amiga.audEn[i]) continue;
      if (amiga.audTick[i]<runCount) {
        runCount=amiga.audTick[i];
      }
    }
    if (bypassLimits) {
      for (int i=0; i<4; i++) {
        if (amiga.incLoc[i]) {
          runCount=1;
          break;
        }
      }
    } else {
      if (228-amiga.hPos<runCount) {
        runCount=228-amiga.hPos;
      }
    }
    if (runCount>0) {
      h+=runCount-1;
    } else {
      runCount=1;
    }

    delay-=runCount;
    if (delay<0) delay=0;
    if (!writes.empty() && delay<=0) {
      QueuedWrite w=writes.front();

      //logV("THE WRITE %x = %x",w.addr,w.val);

      if (w.addr==0x96 && !(w.val&0x8000)) delay=6144;

      amiga.write(w.addr,w.val);
      writes.pop();
    }

    bool hsync=bypassLimits;
    outL=0;
    outR=0;

    // TODO:
    // - improve DMA overrun behavior
    // - does V/P mod really work like that?
    if (!bypassLimits) {
      amiga.hPos+=runCount;
      if (amiga.hPos>=228) {
        amiga.hPos-=228;
        hsync=true;
      }
    }
    for (int i=0; i<4; i++) {
      // run DMA
      if (amiga.audEn[i]) amiga.mustDMA[i]=true;
      if (amiga.dmaEn && amiga.mustDMA[i] && !amiga.audIr[i]) {
        amiga.audTick[i]-=runCount;
        if (amiga.audTick[i]<0) {
          amiga.audTick[i]+=MAX(runCount,amiga.audPer[i]);
          if (amiga.audByte[i]) {
            // read next samples
            if (!amiga.incLoc[i]) {
              amiga.audDat[0][i]=sampleMem[(amiga.dmaLoc[i])&chipMask];
              amiga.audDat[1][i]=sampleMem[(amiga.dmaLoc[i]+1)&chipMask];
              amiga.incLoc[i]=true;
            }

            amiga.audWord[i]=!amiga.audWord[i];
          }

          amiga.mustDMA[i]=amiga.audEn[i];

          amiga.audByte[i]=!amiga.audByte[i];
          if (!amiga.audByte[i] && (amiga.useV[i] || amiga.useP[i])) {
            amiga.nextOut2[i]=((unsigned char)amiga.audDat[0][i])<<8|((unsigned char)amiga.audDat[1][i]);
            if (i<3) {
              if (amiga.useV[i] && amiga.useP[i]) {
                if (amiga.audWord[i]) {
                  amiga.audPer[i+1]=amiga.nextOut2[i];
                } else {
                  amiga.audVol[i+1]=amiga.nextOut2[i];
                }
              } else if (amiga.useV[i]) {
                amiga.audVol[i+1]=amiga.nextOut2[i];
              } else {
                amiga.audPer[i+1]=amiga.nextOut2[i];
              }
            }
          } else if (!amiga.useV[i] && !amiga.useP[i]) {
            amiga.nextOut[i]=amiga.audDat[amiga.audByte[i]][i];
          }
        }

        if (hsync) {
          if (amiga.incLoc[i]) {
            amiga.incLoc[i]=false;
            amiga.dmaLoc[i]+=2;
            // check for length
            if ((--amiga.dmaLen[i])==0) {
              if (amiga.audInt[i]) {
                amiga.audIr[i]=true;
                irq(i);
              }
              amiga.dmaLoc[i]=amiga.audLoc[i];
              amiga.dmaLen[i]=amiga.audLen[i];
            }
          }
        }
      }

      // output
      if (!isMuted[i]) {
        if ((amiga.audVol[i]&127)>=64) {
          output=amiga.nextOut[i]<<6;
        } else if ((amiga.audVol[i]&127)==0) {
          output=0;
        } else {
          output=amiga.nextOut[i]*amiga.audVol[i];
        }
        if (i==0 || i==3) {
          outL+=(output*sep1)>>7;
          outR+=(output*sep2)>>7;
        } else {
          outL+=(output*sep2)>>7;
          outR+=(output*sep1)>>7;
        }
        oscBuf[i]->putSample(h,(amiga.nextOut[i]*MIN(64,amiga.audVol[i]&127))<<1);
      } else {
        // TODO: we can remove this!
        oscBuf[i]->putSample(h,0);
      }
    }

    if (outL!=oldOut[0]) {
      blip_add_delta(bb[0],h,outL-oldOut[0]);
      oldOut[0]=outL;
    }
    if (outR!=oldOut[1]) {
      blip_add_delta(bb[1],h,outR-oldOut[1]);
      oldOut[1]=outR;
    }
  }

  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformAmiga::postProcess(short* buf, int outIndex, size_t len, int sampleRate) {
  // filtering
  double filtFreq=100000.0;
  if (filterOn) {
    if (amigaModel) {
      filtFreq=12000.0;
    } else {
      filtFreq=8000.0;
    }
  } else {
    if (!amigaModel) filtFreq=18000.0;
  }
  if (filtFreq>=((double)sampleRate/2)) return;
  filtConst=sin(M_PI*filtFreq/((double)sampleRate*2.0))*4096.0;

  for (size_t i=0; i<len; i++) {
    filter[outIndex][0]+=(filtConst*(buf[i]-filter[outIndex][0]))>>12;
    filter[outIndex][1]+=(filtConst*(filter[outIndex][0]-filter[outIndex][1]))>>12;
    buf[i]=filter[outIndex][1];
  }
}

void DivPlatformAmiga::irq(int ch) {
  // disable interrupt
  rWrite(0x9a,128<<ch);

  if (chan[ch].irLocL==0x400 && chan[ch].irLocH==0 && chan[ch].irLen==1) {
    // turn off DMA
    rWrite(0x96,1<<ch);
  }

  // acknowledge interrupt
  rWrite(0x9c,128<<ch);
}

#define UPDATE_DMA(x) \
  dmaLen[x]=audLen[x]; \
  dmaLoc[x]=audLoc[x]; \
  audByte[x]=true; \
  audTick[x]=0;

void DivPlatformAmiga::Amiga::write(unsigned short addr, unsigned short val) {
  if (addr&1) return;

  switch (addr&0x1fe) {
    case 0x96: { // DMACON
      if (val&32768) {
        if (val&1) audEn[0]=true;
        if (val&2) audEn[1]=true;
        if (val&4) audEn[2]=true;
        if (val&8) audEn[3]=true;
        if (val&512) dmaEn=true;
      } else {
        if (val&1) {
          audEn[0]=false;
        }
        if (val&2) {
          audEn[1]=false;
        }
        if (val&4) {
          audEn[2]=false;
        }
        if (val&8) {
          audEn[3]=false;
        }
        if (val&512) {
          dmaEn=false;
        }
      }
      break;
    }
    case 0x9a: { // INTENA
      if (val&32768) {
        if (val&128) audInt[0]=true;
        if (val&256) audInt[1]=true;
        if (val&512) audInt[2]=true;
        if (val&1024) audInt[3]=true;
      } else {
        if (val&128) audInt[0]=false;
        if (val&256) audInt[1]=false;
        if (val&512) audInt[2]=false;
        if (val&1024) audInt[3]=false;
      }
      break;
    }
    case 0x9c: { // INTREQ
      if (val&32768) {
        if (val&128) {
          audIr[0]=true;
        }
        if (val&256) {
          audIr[1]=true;
        }
        if (val&512) {
          audIr[2]=true;
        }
        if (val&1024) {
          audIr[3]=true;
        }
      } else {
        if (val&128) audIr[0]=false;
        if (val&256) audIr[1]=false;
        if (val&512) audIr[2]=false;
        if (val&1024) audIr[3]=false;
      }
      break;
    }
    case 0x9e: { // ADKCON
      if (val&32768) {
        if (val&1) useV[0]=true;
        if (val&2) useV[1]=true;
        if (val&4) useV[2]=true;
        if (val&8) useV[3]=true;
        if (val&16) useP[0]=true;
        if (val&32) useP[1]=true;
        if (val&64) useP[2]=true;
        if (val&128) useP[3]=true;
      } else {
        if (val&1) useV[0]=false;
        if (val&2) useV[1]=false;
        if (val&4) useV[2]=false;
        if (val&8) useV[3]=false;
        if (val&16) useP[0]=false;
        if (val&32) useP[1]=false;
        if (val&64) useP[2]=false;
        if (val&128) useP[3]=false;
      }
      break;
    }
    default: { // AUDx
      if (addr>=0xa0 && addr<0xe0) {
        const unsigned char ch=((addr-0xa0)>>4)&3;
        bool updateDMA=false;
        switch (addr&15) {
          case 0: // LCH
            audLoc[ch]&=0xffff;
            audLoc[ch]|=val<<16;
            updateDMA=true;
            break;
          case 2: // LCL
            audLoc[ch]&=0xffff0000;
            audLoc[ch]|=val&0xfffe;
            updateDMA=true;
            break;
          case 4: // LEN
            audLen[ch]=val;
            updateDMA=true;
            break;
          case 6: // PER
            audPer[ch]=val;
            break;
          case 8: // VOL
            audVol[ch]=val;
            break;
          case 10: // DAT
            audDat[0][ch]=val&0xff;
            audDat[1][ch]=val>>8;
            break;
        }
        if (updateDMA && !mustDMA[ch]) {
          UPDATE_DMA(ch);
        }
      }
      break;
    }
  }
}

void DivPlatformAmiga::rWrite(unsigned short addr, unsigned short val) {
  if (addr&1) return;

  //logV("%.3x = %.4x",addr,val);
  if (!skipRegisterWrites) {
    writes.push(QueuedWrite(addr,val));
    regPool[addr>>1]=val;

    if (dumpWrites) {
      addWrite(addr,val);
    }
  }
}

void DivPlatformAmiga::updateWave(int ch) {
  for (int i=0; i<MIN(256,(chan[ch].audLen<<1)); i++) {
    sampleMem[(ch<<8)|i]=chan[ch].ws.output[i]^0x80;
  }
}

void DivPlatformAmiga::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=((chan[i].vol%65)*MIN(64,chan[i].std.vol.val))>>6;
      chan[i].writeVol=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      chan[i].baseFreq=round(NOTE_PERIODIC_NOROUND(parent->calcArp(chan[i].note,chan[i].std.arp.val)));
      chan[i].freqChanged=true;
    }
    if (chan[i].useWave && chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        chan[i].updateWave=true;
      }
    }
    if (chan[i].useWave && chan[i].active) {
      if (chan[i].ws.tick()) {
        chan[i].updateWave=true;
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
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].keyOn=true;
      }
    }
  }

  unsigned short dmaOff=0;
  unsigned short dmaOn=0;
  for (int i=0; i<4; i++) {
    if (chan[i].keyOn || chan[i].keyOff) {
      chWrite(i,6,1);
      dmaOff|=1<<i;
    }
  }

  if (dmaOff) rWrite(0x96,dmaOff);

  for (int i=0; i<4; i++) {
    if (chan[i].updateWave) {
      chan[i].updateWave=false;
      updateWave(i);
    }
  }

  for (int i=0; i<4; i++) {
    double off=1.0;
    if (!chan[i].useWave && chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[i].sample);
      if (s->centerRate<1) {
        off=1.0;
      } else {
        off=parent->getCenterRate()/(double)s->centerRate;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_AMIGA);
      chan[i].freq=off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].freq<0) chan[i].freq=0;

      chWrite(i,6,chan[i].freq);

      if (chan[i].keyOn) {
        if (chan[i].useWave) {
          rWrite(0x9a,(128<<i));
          chWrite(i,0,0);
          chWrite(i,2,i<<8);
          chWrite(i,4,chan[i].audLen);
          if (dumpWrites) {
            addWrite(0x200+i,i<<8);
            addWrite(0x204+i,chan[i].audLen);
          }
          dmaOn|=1<<i;
        } else {
          if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
            DivSample* s=parent->getSample(chan[i].sample);
            int start=chan[i].audPos&(~1);
            if (start>s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)) start=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
            int len=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)-start;
            if (len<0) len=0;
            if (len>131070) len=131070;
            len>>=1;

            start+=sampleOff[chan[i].sample];

            if (len<1) {
              chWrite(i,0,0);
              chWrite(i,2,0x400);
              chWrite(i,4,1);
              if (dumpWrites) {
                addWrite(0x200+i,0x400);
                addWrite(0x204+i,1);
              }
            } else {
              chWrite(i,0,start>>16);
              chWrite(i,2,start);
              chWrite(i,4,len);
              if (dumpWrites) {
                addWrite(0x200+i,start);
                addWrite(0x204+i,len);
              }
            }

            dmaOn|=1<<i;
            if (s->isLoopable()) {
              int loopPos=(sampleOff[chan[i].sample]+s->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT))&(~1);
              int loopEnd=(s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT)-s->getLoopStartPosition(DIV_SAMPLE_DEPTH_8BIT))>>1;
              chan[i].irLocH=loopPos>>16;
              chan[i].irLocL=loopPos;
              chan[i].irLen=MIN(65535,loopEnd);
            } else {
              chan[i].irLocH=0;
              chan[i].irLocL=0x400;
              chan[i].irLen=1;
            }
            rWrite(0x9a,0x8000|(128<<i));
          } else {
            chWrite(i,0,0);
            chWrite(i,2,0x400);
            chWrite(i,4,1);
            if (dumpWrites) {
              addWrite(0x200+i,0x400);
              addWrite(0x204+i,1);
            }
          }
        }
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  if (dmaOn) rWrite(0x96,0x8000|dmaOn);

  for (int i=0; i<4; i++) {
    if ((dmaOn&(1<<i)) && !chan[i].useWave) {
      // write latched loc/len
      if (dumpWrites) {
        addWrite(0x200+i,(chan[i].irLocH<<16)|chan[i].irLocL);
        addWrite(0x204+i,chan[i].irLen);
      } else {
        chWrite(i,0,chan[i].irLocH);
        chWrite(i,2,chan[i].irLocL);
        chWrite(i,4,chan[i].irLen);
      }
    }
  }

  for (int i=0; i<4; i++) {
    if (chan[i].writeVol) {
      chan[i].writeVol=false;
      chWrite(i,8,chan[i].outVol);
    }
  }

  if (updateADKCon) {
    updateADKCon=false;
    rWrite(0x9e,0xff);
    rWrite(0x9e,(
      0x8000|
      (chan[0].useV?1:0)|
      (chan[1].useV?2:0)|
      (chan[2].useV?4:0)|
      (chan[3].useV?8:0)|
      (chan[0].useP?16:0)|
      (chan[1].useP?32:0)|
      (chan[2].useP?64:0)|
      (chan[3].useP?128:0)
    ));
  }
}

int DivPlatformAmiga::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      if (ins->amiga.useWave) {
        if (!chan[c.chan].useWave) chan[c.chan].updateWave=true;
        chan[c.chan].useWave=true;
        chan[c.chan].audLen=(ins->amiga.waveLen+1)>>1;
        if (chan[c.chan].insChanged) {
          if (chan[c.chan].wave<0) {
            chan[c.chan].wave=0;
            chan[c.chan].ws.setWidth(chan[c.chan].audLen<<1);
            chan[c.chan].ws.changeWave1(chan[c.chan].wave);
            chan[c.chan].updateWave=true;
          }
        }
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        }
        chan[c.chan].useWave=false;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(NOTE_PERIODIC_NOROUND(c.value));
      }
      if (chan[c.chan].useWave || chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (chan[c.chan].setPos) {
        chan[c.chan].setPos=false;
      } else {
        chan[c.chan].audPos=0;
      }
      chan[c.chan].audSub=0;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
        chan[c.chan].writeVol=true;
      }
      if (chan[c.chan].useWave) {
        chan[c.chan].ws.init(ins,chan[c.chan].audLen<<1,255,chan[c.chan].insChanged);
        chan[c.chan].updateWave=true;
      }
      chan[c.chan].insChanged=false;
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          chan[c.chan].writeVol=true;
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
      if (!chan[c.chan].useWave) break;
      chan[c.chan].wave=c.value;
      chan[c.chan].keyOn=true;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      chan[c.chan].updateWave=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=round(NOTE_PERIODIC_NOROUND(c.value2+chan[c.chan].sampleNoteDelta));
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
      chan[c.chan].baseFreq=round(NOTE_PERIODIC_NOROUND(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0))));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      if (chan[c.chan].useWave) break;
      chan[c.chan].audPos=c.value;
      if (chan[c.chan].active) chan[c.chan].keyOn=true;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_AMIGA_FILTER:
      filterOn=c.value;
      filtConst=filterOn?filtConstOn:filtConstOff;
      break;
    case DIV_CMD_AMIGA_AM:
      chan[c.chan].useV=c.value;
      updateADKCon=true;
      break;
    case DIV_CMD_AMIGA_PM:
      chan[c.chan].useP=c.value;
      updateADKCon=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 64;
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

void DivPlatformAmiga::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformAmiga::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].writeVol=true;
    /*chan[i].keyOn=false;
    chan[i].keyOff=false;
    chan[i].sample=-1;*/
    if (!chan[i].useWave) {
      rWrite(0x96,1<<i);
    }
  }
}

void* DivPlatformAmiga::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformAmiga::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformAmiga::reset() {
  writes.clear();
  memset(regPool,0,256*sizeof(unsigned short));
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformAmiga::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,255);
    filter[0][i]=0;
    filter[1][i]=0;
  }
  filterOn=false;
  filtConst=filterOn?filtConstOn:filtConstOff;
  updateADKCon=true;
  delay=0;
  oldOut[0]=0;
  oldOut[1]=0;

  amiga=Amiga();
  // enable DMA
  rWrite(0x96,0x8200);
}

int DivPlatformAmiga::getOutputCount() {
  return 2;
}

bool DivPlatformAmiga::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformAmiga::hasAcquireDirect() {
  return true;
}

DivMacroInt* DivPlatformAmiga::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivSamplePos DivPlatformAmiga::getSamplePos(int ch) {
  if (ch>=4) return DivSamplePos();
  if (chan[ch].sample<0 || chan[ch].sample>=parent->song.sampleLen) return DivSamplePos();
  int audPer=amiga.audPer[ch];
  if (audPer<1) audPer=1;
  return DivSamplePos(
    chan[ch].sample,
    amiga.dmaLoc[ch]-sampleOff[chan[ch].sample],
    chipClock/audPer
  );
}

void DivPlatformAmiga::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformAmiga::notifyWaveChange(int wave) {
  for (int i=0; i<4; i++) {
    if (chan[i].useWave && chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      chan[i].updateWave=true;
    }
  }
}

void DivPlatformAmiga::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAmiga::setFlags(const DivConfig& flags) {
  if (flags.getInt("clockSel",0)) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  CHECK_CUSTOM_CLOCK;
  
  rate=chipClock;
  for (int i=0; i<4; i++) {
    oscBuf[i]->setRate(rate);
  }
  int sep=flags.getInt("stereoSep",0)&127;
  sep1=sep+127;
  sep2=127-sep;
  amigaModel=flags.getInt("chipType",0);
  chipMem=flags.getInt("chipMem",21);
  if (chipMem<18) chipMem=18;
  if (chipMem>21) chipMem=21;
  chipMask=(1<<chipMem)-1;
  bypassLimits=flags.getBool("bypassLimits",false);
  if (amigaModel) {
    filtConstOff=4000;
    filtConstOn=sin(M_PI*8000.0/(double)rate)*4096.0;
  } else {
    filtConstOff=sin(M_PI*16000.0/(double)rate)*4096.0;
    filtConstOn=sin(M_PI*5500.0/(double)rate)*4096.0;
  }
}

void DivPlatformAmiga::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformAmiga::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

unsigned char* DivPlatformAmiga::getRegisterPool() {
  // update DMACONR
  regPool[1]=(
    (amiga.audEn[0]?1:0)|
    (amiga.audEn[1]?2:0)|
    (amiga.audEn[2]?4:0)|
    (amiga.audEn[3]?8:0)|
    (amiga.dmaEn?512:0)
  );

  // update ADKCONR
  regPool[0x10>>1]=(
    (amiga.useV[0]?1:0)|
    (amiga.useV[1]?2:0)|
    (amiga.useV[2]?4:0)|
    (amiga.useV[3]?8:0)|
    (amiga.useP[0]?16:0)|
    (amiga.useP[1]?32:0)|
    (amiga.useP[2]?64:0)|
    (amiga.useP[3]?128:0)
  );

  // update INTENAR
  regPool[0x1c>>1]=(
    (amiga.audInt[0]?128:0)|
    (amiga.audInt[1]?256:0)|
    (amiga.audInt[2]?512:0)|
    (amiga.audInt[3]?1024:0)|
    16384 // INTEN
  );

  // update INTREQR
  regPool[0x1e>>1]=(
    (amiga.audIr[0]?128:0)|
    (amiga.audIr[1]?256:0)|
    (amiga.audIr[2]?512:0)|
    (amiga.audIr[3]?1024:0)
  );

  return (unsigned char*)regPool;
}

int DivPlatformAmiga::getRegisterPoolSize() {
  return 128;
}

int DivPlatformAmiga::getRegisterPoolDepth() {
  return 16;
}

const void* DivPlatformAmiga::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformAmiga::getSampleMemCapacity(int index) {
  return index == 0 ? (1<<chipMem) : 0;
}

size_t DivPlatformAmiga::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformAmiga::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformAmiga::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformAmiga::renderSamples(int sysID) {
  memset(sampleMem,0,2097152);
  memset(sampleOff,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Chip Memory";

  memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_WAVE_RAM,"Wave RAM",-1,0,1024));
  memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_RESERVED,"End of Sample",-1,1024,1026));

  // first 1024 bytes reserved for wavetable
  // the next 2 bytes are reserved for end of sample
  size_t memPos=1026;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }

    if (memPos>=getSampleMemCapacity()) {
      logW("out of Amiga memory for sample %d!",i);
      break;
    }

    int length=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_8BIT);
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos),length);
    if (actualLength>0) {
      sampleOff[i]=memPos;
      memcpy(&sampleMem[memPos],s->data8,actualLength);
      memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+actualLength));
      memPos+=actualLength;
    }
    // align memPos to short
    if (memPos&1) memPos++;
    sampleLoaded[i]=true;
  }
  sampleMemLen=memPos;

  memCompo.capacity=1<<chipMem;
  memCompo.used=sampleMemLen;
}

int DivPlatformAmiga::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    isMuted[i]=false;
  }

  // Paula volume is implemented using PWM rather than a multiplication.
  // sources:
  // - https://www.youtube.com/watch?v=xyQlmsD7PAg
  // - https://linusakesson.net/music/paulimba/index.php
  memset(volTable,0,64*64);
  for (int i=0; i<64; i++) {
    for (int j=0; j<64; j++) {
      volTable[i][j/AMIGA_DIVIDER]+=(j<i)*(64/AMIGA_DIVIDER);
    }
  }

  sampleMem=new unsigned char[2097152];
  sampleMemLen=0;

  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformAmiga::quit() {
  delete[] sampleMem;
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
}
