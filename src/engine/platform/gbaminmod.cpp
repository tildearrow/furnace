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

#include "gbaminmod.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define CHIP_FREQBASE 16777216

#define rWrite(a,v) {regPool[a]=v;}

const char* regCheatSheetMinMod[]={
  "CHx_Counter", "x0",
  "CHx_Address", "x2",
  "CHx_LastLeft", "x4",
  "CHx_LastRight", "x6",
  "CHx_Freq", "x8",
  "CHx_LoopEnd", "xA",
  "CHx_LoopStart", "xC",
  "CHx_VolumeLeft", "xE",
  "CHx_VolumeRight", "xF",
  NULL
};

const char** DivPlatformGBAMinMod::getRegisterSheet() {
  return regCheatSheetMinMod;
}

void DivPlatformGBAMinMod::acquire(short** buf, size_t len) {
  size_t sampPos=mixBufReadPos&3;
  bool newSamp=false;
  // cache channel registers that might change
  struct {
    uint64_t address;
    unsigned int freq, loopEnd, loopStart;
    short volL, volR;
  } chState[16];
  for (int i=0; i<chanMax; i++) {
    unsigned short* chReg=&regPool[i*16];
    chState[i].address=chReg[0]|((uint64_t)chReg[1]<<16)|((uint64_t)chReg[2]<<32)|((uint64_t)chReg[3]<<48);
    chState[i].freq=chReg[8]|((unsigned int)chReg[9]<<16);
    chState[i].loopEnd=chReg[10]|((unsigned int)chReg[11]<<16);
    chState[i].loopStart=chReg[12]|((unsigned int)chReg[13]<<16);
    chState[i].volL=(short)chReg[14];
    chState[i].volR=(short)chReg[15];
  }
  for (size_t h=0; h<len; h++) {
    while (sampTimer>=sampCycles) {
      // the driver generates 4 samples at a time and can be start-offset
      sampPos=mixBufReadPos&3;
      if (sampPos==mixBufOffset) {
        for (size_t j=mixBufOffset; j<4; j++) {
          mixOut[0][j]=0;
          mixOut[1][j]=0;
        }
        for (int i=0; i<chanMax; i++) {
          for (size_t j=mixBufOffset; j<4; j++) {
            unsigned int lastAddr=chState[i].address>>32;
            chState[i].address+=((uint64_t)chState[i].freq)<<8;
            unsigned int newAddr=chState[i].address>>32;
            if (newAddr!=lastAddr) {
              if (newAddr>=chState[i].loopEnd) {
                newAddr=newAddr-chState[i].loopEnd+chState[i].loopStart;
                chState[i].address=(chState[i].address&0xffffffff)|((uint64_t)newAddr<<32);
              }
              int newSamp=0;
              switch (newAddr>>24) {
                case 2: // wavetable
                  newAddr&=0x0003ffff;
                  if (newAddr<sizeof(wtMem)) {
                    newSamp=wtMem[newAddr];
                  }
                  break;
                case 3: // echo
                  newAddr&=0x00007fff;
                  if (newAddr>0x800) {
                    newSamp=mixBuf[(newAddr-0x800)/1024][newAddr&1023];
                  }
                  break;
                case 8: // sample
                case 9:
                case 10:
                case 11:
                case 12:
                  newSamp=sampleMem[newAddr&0x01ffffff];
                  break;
              }
              chanOut[i][0]=newSamp*chState[i].volL;
              chanOut[i][1]=newSamp*chState[i].volR;
            }
            int outL=chanOut[i][0];
            int outR=chanOut[i][1];
            int outA=(chan[i].invertL==chan[i].invertR)?outL+outR:outL-outR;
            mixOut[0][j]+=(unsigned char)(outL>>15);
            mixOut[1][j]+=(unsigned char)(outR>>15);
            oscOut[i][j]=volScale>0?outA*64/volScale:0;
          }
        }
        for (size_t j=mixBufOffset; j<4; j++) {
          mixBuf[mixBufPage][mixBufWritePos]=mixOut[0][j];
          mixBuf[mixBufPage+1][mixBufWritePos]=mixOut[1][j];
          mixBufWritePos++;
        }
        mixBufOffset=0;
      }
      newSamp=true;
      mixBufReadPos++;
      sampTimer-=sampCycles;
    }
    if (newSamp) {
      // assuming max PCM FIFO volume
      sampL=((short)mixOut[0][sampPos]<<8)&(0xff80<<(9-dacDepth));
      sampR=((short)mixOut[1][sampPos]<<8)&(0xff80<<(9-dacDepth));
      newSamp=false;
    }
    buf[0][h]=sampL;
    buf[1][h]=sampR;
    for (int i=0; i<chanMax; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=oscOut[i][sampPos];
    }
    for (int i=chanMax; i<16; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=0;
    }
    while (updTimer>=updCycles) {
      // flip buffer
      // logV("ut=%d,pg=%d,w=%d,r=%d,sc=%d,st=%d",updTimer,mixBufPage,mixBufWritePos,mixBufReadPos,sampCycles,sampTimer);
      mixMemCompo.entries[mixBufPage].end=mixMemCompo.entries[mixBufPage].begin+mixBufWritePos;
      mixMemCompo.entries[mixBufPage+1].end=mixMemCompo.entries[mixBufPage+1].begin+mixBufWritePos;
      mixBufPage=(mixBufPage+2)%(mixBufs*2);
      memset(mixBuf[mixBufPage],0,sizeof(mixBuf[mixBufPage]));
      memset(mixBuf[mixBufPage+1],0,sizeof(mixBuf[mixBufPage+1]));
      // emulate buffer loss prevention and buffer copying
      sampsRendered+=mixBufReadPos;
      mixBufOffset=mixBufWritePos-mixBufReadPos;
      for (size_t j=0; j<mixBufOffset; j++) {
        mixOut[0][j]=mixOut[0][(mixBufReadPos&3)+j];
        mixOut[1][j]=mixOut[1][(mixBufReadPos&3)+j];
        mixBuf[mixBufPage][j]=mixOut[0][j];
        mixBuf[mixBufPage+1][j]=mixOut[1][j];
      }
      mixBufReadPos=0;
      mixBufWritePos=mixBufOffset;
      // check for echo channels and give them proper addresses
      for (int i=0; i<chanMax; i++) {
        unsigned char echoDelay=MIN(chan[i].echo&0x0f,mixBufs-1);
        if(echoDelay) {
          echoDelay=echoDelay*2-((chan[i].echo&0x10)?0:1);
          size_t echoPage=(mixBufPage+mixBufs*2-echoDelay)%(mixBufs*2);
          chState[i].address=(0x03000800ULL+echoPage*1024)<<32;
          chState[i].loopStart=0-echoDelay;
          chState[i].loopEnd=0-echoDelay;
        }
      }
      updTimer-=updCycles;
      updCyclesTotal+=updCycles;
      // recalculate update timer from a new tick rate
      float hz=parent->getCurHz();
      float updCyclesNew=(hz>=1)?(16777216.f/hz):1;
      // the maximum buffer size in the default multi-rate config is 1024 samples
      // (so 15 left/right buffers + mixer code fit the entire 32k of internal RAM)
      // if the driver determines that the current tick rate is too low, it will
      // internally double the rate until the resulting buffer size fits
      while (true) {
        updCycles=floorf(updCyclesNew);
        // emulate prescaler rounding
        if (updCycles>=65536*256) {
          updCycles&=~1024;
        } else if (updCycles>=65536*64) {
          updCycles&=~256;
        } else if (updCycles>=65536) {
          updCycles&=~64;
        }
        unsigned int bufSize=(updCycles/sampCycles+3)&~3;
        if (bufSize<1024 || updCyclesNew<1) {
          break;
        }
        updCyclesNew/=2;
      }
    }
    updTimer+=1<<dacDepth;
    sampTimer+=1<<dacDepth;
  }
  // write back changed cached channel registers
  for (int i=0; i<chanMax; i++) {
    unsigned short* chReg=&regPool[i*16];
    chReg[0]=chState[i].address&0xffff;
    chReg[1]=(chState[i].address>>16)&0xffff;
    chReg[2]=(chState[i].address>>32)&0xffff;
    chReg[3]=(chState[i].address>>48)&0xffff;
    chReg[4]=(chanOut[i][0]>>7)&0xff00;
    chReg[5]=0;
    chReg[6]=(chanOut[i][1]>>7)&0xff00;
    chReg[7]=0;
    chReg[10]=chState[i].loopEnd&0xffff;
    chReg[11]=(chState[i].loopEnd>>16)&0xffff;
    chReg[12]=chState[i].loopStart&0xffff;
    chReg[13]=(chState[i].loopStart>>16)&0xffff;
  }
}

void DivPlatformGBAMinMod::tick(bool sysTick) {
  // collect stats for display in chip config
  // logV("rendered=%d,updTot=%d",sampsRendered,updCyclesTotal);
  if (sysTick && updCyclesTotal>0) {
    // assuming new sample, L!=R and lowest ROM access wait in all channels
    // this gives 39.5 cycles/sample, rounded up to 40 for loops
    maxCPU=(float)sampsRendered*chanMax*40/(float)updCyclesTotal;
  }
  sampsRendered=0;
  updCyclesTotal=0;

  for (int i=0; i<chanMax; i++) {
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
    if (chan[i].useWave && chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
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
    if (chan[i].std.ex1.had) {
      if (chan[i].invertL!=(bool)(chan[i].std.ex1.val&16)) {
        chan[i].invertL=chan[i].std.ex1.val&2;
        chan[i].volChangedL=true;
      }
      if (chan[i].invertR!=(bool)(chan[i].std.ex1.val&8)) {
        chan[i].invertR=chan[i].std.ex1.val&1;
      chan[i].volChangedR=true;
      }
    }
    if (chan[i].setPos) {
      // force keyon
      chan[i].keyOn=true;
      chan[i].setPos=false;
    } else {
      chan[i].audPos=0;
    }
    if (chan[i].useWave && chan[i].active) {
      if (chan[i].ws.tick()) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivSample* s=parent->getSample(chan[i].sample);
      double off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
      chan[i].freq=(int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].keyOn) {
        unsigned int start, end, loop;
        if ((chan[i].echo&0xf)!=0) {
          // make sure echo channels' frequency can't be faster than the sample rate
          if (chan[i].freq>CHIP_FREQBASE) {
            chan[i].freq=CHIP_FREQBASE;
          }
          // this is only to match the HLE implementation
          // the actual engine will handle mid-frame echo switch differently
          start=loop=0x08000000;
          end=0x08000001;
        } else if (chan[i].useWave) {
          start=(i*256)|0x02000000;
          end=start+chan[i].wtLen;
          loop=start;
        } else {
          size_t maxPos=getSampleMemCapacity();
          start=sampleOff[chan[i].sample];
          if (s->isLoopable()) {
            end=MIN(start+MAX(s->loopEnd,1),maxPos);
            loop=start+s->loopStart;
          } else {
            end=MIN(start+s->length8+16,maxPos);
            loop=MIN(start+s->length8,maxPos);
          }
          if (chan[i].audPos>0) {
            start=start+MIN(chan[i].audPos,end);
          }
          start|=0x08000000;
          end|=0x08000000;
          loop|=0x08000000;
        }
        rWrite(2+i*16,start&0xffff);
        rWrite(3+i*16,start>>16);
        rWrite(10+i*16,end&0xffff);
        rWrite(11+i*16,end>>16);
        rWrite(12+i*16,loop&0xffff);
        rWrite(13+i*16,loop>>16);
        if (!chan[i].std.vol.had) {
          chan[i].outVol=chan[i].vol;
        }
        chan[i].volChangedL=true;
        chan[i].volChangedR=true;
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        chan[i].volChangedL=true;
        chan[i].volChangedR=true;
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        rWrite(8+i*16,chan[i].freq&0xffff);
        rWrite(9+i*16,chan[i].freq>>16);
        chan[i].freqChanged=false;
      }
    }
    // don't scale echo channels
    if (chan[i].volChangedL) {
      int out=chan[i].outVol*chan[i].chPanL;
      if ((chan[i].echo&0xf)==0) out=(out*volScale)>>16;
      else out=out>>1;
      if (chan[i].invertL) out=-out;
      rWrite(14+i*16,(isMuted[i] || !chan[i].active)?0:out);
      chan[i].volChangedL=false;
    }
    if (chan[i].volChangedR) {
      int out=chan[i].outVol*chan[i].chPanR;
      if ((chan[i].echo&0xf)==0) out=(out*volScale)>>16;
      else out=out>>1;
      if (chan[i].invertR) out=-out;
      rWrite(15+i*16,(isMuted[i] || !chan[i].active)?0:out);
      chan[i].volChangedR=false;
    }
  }
}

int DivPlatformGBAMinMod::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_AMIGA);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
      chan[c.chan].macroPanMul=ins->type==DIV_INS_AMIGA?127:255;
      if (ins->amiga.useWave) {
        chan[c.chan].useWave=true;
        chan[c.chan].wtLen=ins->amiga.waveLen+1;
        if (c.chan<chanMax) {
          wtMemCompo.entries[c.chan].end=wtMemCompo.entries[c.chan].begin+chan[c.chan].wtLen;
        }
        if (chan[c.chan].insChanged) {
          if (chan[c.chan].wave<0) {
            chan[c.chan].wave=0;
          }
          chan[c.chan].ws.setWidth(chan[c.chan].wtLen);
          chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        }
        chan[c.chan].ws.init(ins,chan[c.chan].wtLen,255,chan[c.chan].insChanged);
      } else {
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          c.value=ins->amiga.getFreq(c.value);
        }
        chan[c.chan].useWave=false;
      }
      if (chan[c.chan].useWave || chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value));
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
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
    case DIV_CMD_SNES_INVERT:
      chan[c.chan].invertL=(c.value>>4);
      chan[c.chan].invertR=c.value&15;
      chan[c.chan].volChangedL=true;
      chan[c.chan].volChangedR=true;
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
    case DIV_CMD_WAVE:
      if (!chan[c.chan].useWave) break;
      chan[c.chan].wave=c.value;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
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
    case DIV_CMD_MINMOD_ECHO:
      chan[c.chan].echo=c.value;
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

void DivPlatformGBAMinMod::updateWave(int ch) {
  int addr=ch*256;
  for (unsigned int i=0; i<chan[ch].wtLen; i++) {
    wtMem[addr+i]=(signed char)(chan[ch].ws.output[i]-128);
  }
}

void DivPlatformGBAMinMod::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volChangedL=true;
  chan[ch].volChangedR=true;
}

void DivPlatformGBAMinMod::forceIns() {
  for (int i=0; i<chanMax; i++) {
    chan[i].insChanged=true;
    chan[i].volChangedL=true;
    chan[i].volChangedR=true;
    chan[i].sample=-1;
    chan[i].active=false;
  }
}

void* DivPlatformGBAMinMod::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformGBAMinMod::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformGBAMinMod::getPan(int ch) {
  return (chan[ch].chPanL<<8)|(chan[ch].chPanR);
}

DivSamplePos DivPlatformGBAMinMod::getSamplePos(int ch) {
  if (ch>=chanMax ||
    chan[ch].sample<0 || chan[ch].sample>=parent->song.sampleLen ||
    !chan[ch].active || (chan[ch].echo&0xf)!=0
  ) {
    return DivSamplePos();
  }
  return DivSamplePos(
    chan[ch].sample,
    (((int)regPool[ch*16+2]|((int)regPool[ch*16+3]<<16))&0x01ffffff)-sampleOff[chan[ch].sample],
    (int64_t)chan[ch].freq*chipClock/CHIP_FREQBASE
  );
}

DivDispatchOscBuffer* DivPlatformGBAMinMod::getOscBuffer(int ch) {
  return oscBuf[ch];
}

void DivPlatformGBAMinMod::reset() {
  resetMixer();
  memset(regPool,0,sizeof(regPool));
  memset(wtMem,0,sizeof(wtMem));
  for (int i=0; i<16; i++) {
    chan[i]=DivPlatformGBAMinMod::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,255);
  }
}

void DivPlatformGBAMinMod::resetMixer() {
  sampTimer=sampCycles;
  updTimer=0;
  updCycles=0;
  mixBufReadPos=0;
  mixBufWritePos=0;
  mixBufPage=0;
  mixBufOffset=0;
  sampsRendered=0;
  sampL=0;
  sampR=0;
  memset(mixBuf,0,sizeof(mixBuf));
  memset(chanOut,0,sizeof(chanOut));
}

int DivPlatformGBAMinMod::getOutputCount() {
  return 2;
}

void DivPlatformGBAMinMod::notifyInsChange(int ins) {
  for (int i=0; i<16; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGBAMinMod::notifyWaveChange(int wave) {
  for (int i=0; i<16; i++) {
    if (chan[i].useWave && chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformGBAMinMod::notifyInsDeletion(void* ins) {
  for (int i=0; i<16; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformGBAMinMod::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformGBAMinMod::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

unsigned char* DivPlatformGBAMinMod::getRegisterPool() {
  return (unsigned char*)regPool;
}

int DivPlatformGBAMinMod::getRegisterPoolSize() {
  return 256;
}

int DivPlatformGBAMinMod::getRegisterPoolDepth() {
  return 16;
}

const void* DivPlatformGBAMinMod::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformGBAMinMod::getSampleMemCapacity(int index) {
  return index == 0 ? 33554432 : 0;
}

size_t DivPlatformGBAMinMod::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformGBAMinMod::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformGBAMinMod::getMemCompo(int index) {
  switch (index) {
    case 0: return &romMemCompo;
    case 1: return &wtMemCompo;
    case 2: return &mixMemCompo;
  }
  return NULL;
}

void DivPlatformGBAMinMod::renderSamples(int sysID) {
  size_t maxPos=getSampleMemCapacity();
  memset(sampleMem,0,maxPos);
  romMemCompo.entries.clear();
  romMemCompo.capacity=maxPos;

  // dummy zero-length samples are at pos 0 as the engine still outputs them
  size_t memPos=1;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }
    int length=s->length8;
    int actualLength=MIN((int)(maxPos-memPos),length);
    if (actualLength>0) {
      sampleOff[i]=memPos;
      memcpy(&sampleMem[memPos],s->data8,actualLength);
      memPos+=actualLength;
      romMemCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"PCM",i,sampleOff[i],memPos));
      // if it's one-shot, add 16 silent samples for looping area
      // this should be enough for most cases even though the
      // frequency register can make position jump by up to 256 samples
      if (!s->isLoopable()) {
        int oneShotLen=MIN((int)maxPos-memPos,16);
        memset(&sampleMem[memPos],0,oneShotLen);
        memPos+=oneShotLen;
      }
    }
    if (actualLength<length) {
      logW("out of GBA MinMod PCM memory for sample %d!",i);
      break;
    }
    sampleLoaded[i]=true;
  }
  sampleMemLen=memPos;
  romMemCompo.used=sampleMemLen;
}

void DivPlatformGBAMinMod::setFlags(const DivConfig& flags) {
  volScale=flags.getInt("volScale",4096);
  mixBufs=flags.getInt("mixBufs",15);
  dacDepth=flags.getInt("dacDepth",9);
  chanMax=flags.getInt("channels",16);
  rate=16777216>>dacDepth;
  for (int i=0; i<16; i++) {
    oscBuf[i]->rate=rate;
  }
  sampCycles=16777216/flags.getInt("sampRate",21845);
  chipClock=16777216/sampCycles;
  resetMixer();
  wtMemCompo.used=256*chanMax;
  mixMemCompo.used=2048*mixBufs;
  wtMemCompo.entries.clear();
  mixMemCompo.entries.clear();
  for (int i=0; i<chanMax; i++) {
    wtMemCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_WAVE_RAM, fmt::sprintf("Channel %d",i),-1,i*256,i*256));
  }
  for (int i=0; i<(int)mixBufs; i++) {
    mixMemCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_ECHO, fmt::sprintf("Buffer %d Left",i),-1,i*2048,i*2048));
    mixMemCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_ECHO, fmt::sprintf("Buffer %d Right",i),-1,i*2048+1024,i*2048+1024));
  }
}

int DivPlatformGBAMinMod::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<16; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  sampleMem=new signed char[getSampleMemCapacity()];
  sampleMemLen=0;
  romMemCompo=DivMemoryComposition();
  romMemCompo.name="Sample ROM";
  wtMemCompo=DivMemoryComposition();
  wtMemCompo.name="Wavetable RAM";
  wtMemCompo.capacity=256*16;
  wtMemCompo.memory=(unsigned char*)wtMem;
  wtMemCompo.waveformView=DIV_MEMORY_WAVE_8BIT_SIGNED;
  mixMemCompo=DivMemoryComposition();
  mixMemCompo.name="Mix/Echo Buffer";
  mixMemCompo.capacity=2048*15;
  mixMemCompo.memory=(unsigned char*)mixBuf;
  mixMemCompo.waveformView=DIV_MEMORY_WAVE_8BIT_SIGNED;
  setFlags(flags);
  reset();

  maxCPU=0.0; // initialize!

  return 16;
}

void DivPlatformGBAMinMod::quit() {
  delete[] sampleMem;
  for (int i=0; i<16; i++) {
    delete oscBuf[i];
  }
}
