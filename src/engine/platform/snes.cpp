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

#include "snes.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define CHIP_FREQBASE 131072

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) {rWrite((a)+(c)*16,v)}
#define sampleTableAddr(c) (sampleTableBase+(c)*4)
#define waveTableAddr(c) (sampleTableBase+8*4+(c)*9*16)

const char* regCheatSheetSNESDSP[]={
  "VxVOLL", "x0",
  "VxVOLR", "x1",
  "VxPITCHL", "x2",
  "VxPITCHH", "x3",
  "VxSRCN", "x4",
  "VxADSR1", "x5",
  "VxADSR2", "x6",
  "VxGAIN", "x7",
  "VxENVX", "x8",
  "VxOUTX", "x9",
  "FIRx", "xF",

  "MVOLL", "0C",
  "MVOLR", "1C",
  "EVOLL", "2C",
  "EVOLR", "3C",
  "KON", "4C",
  "KOFF", "5C",
  "FLG", "6C",
  "ENDX", "7C",

  "EFB", "0D",
  "PMON", "2D",
  "NON", "3D",
  "EON", "4D",
  "DIR", "5D",
  "ESA", "6D",
  "EDL", "7D",
  NULL
};

const char** DivPlatformSNES::getRegisterSheet() {
  return regCheatSheetSNESDSP;
}

void DivPlatformSNES::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  short out[2];
  short chOut[16];
  for (size_t h=start; h<start+len; h++) {
    if (--delay<=0) {
      delay=0;
      if (!writes.empty()) {
        QueuedWrite w=writes.front();
        dsp.write(w.addr,w.val);
        regPool[w.addr&0x7f]=w.val;
        writes.pop();
        delay=(w.addr==0x5c)?8:1;
      }
    }
    dsp.set_output(out,1);
    dsp.run(32);
    dsp.get_voice_outputs(chOut);
    bufL[h]=out[0];
    bufR[h]=out[1];
    for (int i=0; i<8; i++) {
      int next=(3*(chOut[i*2]+chOut[i*2+1]))>>2;
      if (next<-32768) next=-32768;
      if (next>32767) next=32767;
      next=(next*254)/MAX(1,globalVolL+globalVolR);
      if (next<-32768) next=-32768;
      if (next>32767) next=32767;
      oscBuf[i]->data[oscBuf[i]->needle++]=next;
    }
  }
}

void DivPlatformSNES::tick(bool sysTick) {
  // KON/KOFF can't be written several times per one sample
  // so they have to be accumulated
  unsigned char kon=0;
  unsigned char koff=0;
  for (int i=0; i<8; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&127,MIN(127,chan[i].std.vol.val),127);
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      noiseFreq=chan[i].std.duty.val;
      writeControl=true;
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
      chan[i].panL=chan[i].std.panL.val&0x7f;
    }
    if (chan[i].std.panR.had) {
      chan[i].panR=chan[i].std.panR.val&0x7f;
    }
    bool hasInverted=false;
    if (chan[i].std.ex1.had) {
      if (chan[i].invertL!=(bool)(chan[i].std.ex1.val&16)) {
        chan[i].invertL=chan[i].std.ex1.val&16;
        hasInverted=true;
      }
      if (chan[i].invertR!=(bool)(chan[i].std.ex1.val&8)) {
        chan[i].invertR=chan[i].std.ex1.val&8;
        hasInverted=true;
      }
      if (chan[i].pitchMod!=(bool)(chan[i].std.ex1.val&4)) {
        chan[i].pitchMod=chan[i].std.ex1.val&4;
        writePitchMod=true;
      }
      if (chan[i].echo!=(bool)(chan[i].std.ex1.val&2)) {
        chan[i].echo=chan[i].std.ex1.val&2;
        writeEcho=true;
      }
      if (chan[i].noise!=(bool)(chan[i].std.ex1.val&1)) {
        chan[i].noise=chan[i].std.ex1.val&1;
        writeNoise=true;
      }
    }
    if (chan[i].std.vol.had || chan[i].std.panL.had || chan[i].std.panR.had || hasInverted) {
      chan[i].shallWriteVol=true;
    }
    if (chan[i].std.ex2.had) {
      if (chan[i].std.ex2.val&0x80) {
        switch (chan[i].std.ex2.val&0x60) {
          case 0x00:
            chan[i].state.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LINEAR;
            break;
          case 0x20:
            chan[i].state.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LOG;
            break;
          case 0x40:
            chan[i].state.gainMode=DivInstrumentSNES::GAIN_MODE_INC_LINEAR;
            break;
          case 0x60:
            chan[i].state.gainMode=DivInstrumentSNES::GAIN_MODE_INC_INVLOG;
            break;
        }
        chan[i].state.gain=chan[i].std.ex2.val&31;
      } else {
        chan[i].state.gainMode=DivInstrumentSNES::GAIN_MODE_DIRECT;
        chan[i].state.gain=chan[i].std.ex2.val&127;
      }
      chan[i].shallWriteEnv=true;
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
  }
  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivSample* s=parent->getSample(chan[i].sample);
      double off=(s->centerRate>=1)?((double)s->centerRate/8363.0):1.0;
      if (chan[i].useWave) off=(double)chan[i].wtLen/32.0;
      chan[i].freq=(unsigned int)(off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE));
      if (chan[i].freq>16383) chan[i].freq=16383;
      if (chan[i].keyOn) {
        unsigned int start, end, loop;
        unsigned short tabAddr=sampleTableAddr(i);
        if (chan[i].useWave) {
          start=waveTableAddr(i);
          loop=start;
        } else if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
          start=sampleOff[chan[i].sample];
          end=MIN(start+MAX(s->lengthBRR+((s->loop && s->depth!=DIV_SAMPLE_DEPTH_BRR)?9:0),1),getSampleMemCapacity());
          loop=MAX(start,end-1);
          if (chan[i].audPos>0) {
            start=start+MIN(chan[i].audPos,s->lengthBRR-1)/16*9;
          }
          if (s->loopStart>=0) {
            loop=((s->depth!=DIV_SAMPLE_DEPTH_BRR)?9:0)+start+((s->loopStart/16)*9);
          }
        } else {
          start=0;
          end=0;
          loop=0;
        }
        sampleMem[tabAddr+0]=start&0xff;
        sampleMem[tabAddr+1]=start>>8;
        sampleMem[tabAddr+2]=loop&0xff;
        sampleMem[tabAddr+3]=loop>>8;
        kon|=(1<<i);
        koff|=(1<<i);
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        if (!chan[i].state.sus) {
          koff|=(1<<i);
        }
        chan[i].keyOff=false;
      }
      if (chan[i].freqChanged) {
        chWrite(i,2,chan[i].freq&0xff);
        chWrite(i,3,chan[i].freq>>8);
        chan[i].freqChanged=false;
      }
    }
  }
  if (koff!=0) {
    rWrite(0x5c,koff);
  }
  if (writeControl) {
    unsigned char control=(noiseFreq&0x1f)|(echoOn?0:0x20);
    rWrite(0x6c,control);
    writeControl=false;
  }
  if (writeNoise) {
    unsigned char noiseBits=(
      (chan[0].noise?1:0)|
      (chan[1].noise?2:0)|
      (chan[2].noise?4:0)|
      (chan[3].noise?8:0)|
      (chan[4].noise?0x10:0)|
      (chan[5].noise?0x20:0)|
      (chan[6].noise?0x40:0)|
      (chan[7].noise?0x80:0)
    );
    rWrite(0x3d,noiseBits);
    writeNoise=false;
  }
  if (writePitchMod) {
    unsigned char pitchModBits=(
      (chan[0].pitchMod?1:0)|
      (chan[1].pitchMod?2:0)|
      (chan[2].pitchMod?4:0)|
      (chan[3].pitchMod?8:0)|
      (chan[4].pitchMod?0x10:0)|
      (chan[5].pitchMod?0x20:0)|
      (chan[6].pitchMod?0x40:0)|
      (chan[7].pitchMod?0x80:0)
    );
    rWrite(0x2d,pitchModBits);
    writePitchMod=false;
  }
  if (writeEcho) {
    unsigned char echoBits=(
      (chan[0].echo?1:0)|
      (chan[1].echo?2:0)|
      (chan[2].echo?4:0)|
      (chan[3].echo?8:0)|
      (chan[4].echo?0x10:0)|
      (chan[5].echo?0x20:0)|
      (chan[6].echo?0x40:0)|
      (chan[7].echo?0x80:0)
    );
    rWrite(0x4d,echoBits);
    writeEcho=false;
  }
  for (int i=0; i<8; i++) {
    if (chan[i].shallWriteEnv) {
      writeEnv(i);
      chan[i].shallWriteEnv=false;
    }
  }
  if (koff!=0) {
    rWrite(0x5c,0);
  }
  if (kon!=0) {
    rWrite(0x4c,kon);
  }
  for (int i=0; i<8; i++) {
    if (chan[i].shallWriteVol) {
      writeOutVol(i);
      chan[i].shallWriteVol=false;
    }
  }
}

int DivPlatformSNES::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_SNES);
      if (ins->amiga.useWave) {
        chan[c.chan].useWave=true;
        chan[c.chan].wtLen=ins->amiga.waveLen+1;
        if (chan[c.chan].insChanged) {
          if (chan[c.chan].wave<0) {
            chan[c.chan].wave=0;
          }
          chan[c.chan].ws.setWidth(chan[c.chan].wtLen);
          chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        }
        chan[c.chan].ws.init(ins,chan[c.chan].wtLen,15,chan[c.chan].insChanged);
      } else {
        if (c.value!=DIV_NOTE_NULL) chan[c.chan].sample=ins->amiga.getSample(c.value);
        chan[c.chan].useWave=false;
      }
      if (chan[c.chan].useWave || chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->snes;
      }
      chan[c.chan].active=true;
      if (chan[c.chan].insChanged || chan[c.chan].state.sus) {
        chan[c.chan].shallWriteEnv=true;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value));
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      if (chan[c.chan].state.sus) {
        chan[c.chan].shallWriteEnv=true;
      } else {
        chan[c.chan].macroInit(NULL);
      }
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      if (chan[c.chan].state.sus) {
        chan[c.chan].shallWriteEnv=true;
      }
      chan[c.chan].std.release();
      break;
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
          chan[c.chan].shallWriteVol=true;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].panL=c.value>>1;
      chan[c.chan].panR=c.value2>>1;
      chan[c.chan].shallWriteVol=true;
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
      int destFreq=round(NOTE_FREQUENCY(c.value2));
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
      chan[c.chan].baseFreq=round(NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0))));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_SNES));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS:
      // may have to remove this
      chan[c.chan].audPos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].noise=c.value;
      writeNoise=true;
      break;
    case DIV_CMD_SNES_PITCH_MOD:
      chan[c.chan].pitchMod=c.value;
      writePitchMod=true;
      break;
    case DIV_CMD_SNES_INVERT:
      chan[c.chan].invertL=(c.value>>4);
      chan[c.chan].invertR=c.chan&15;
      chan[c.chan].shallWriteVol=true;
      break;
    case DIV_CMD_SNES_GAIN_MODE:
      if (c.value) {
        chan[c.chan].state.useEnv=false;
        switch (c.value) {
          case 1:
            chan[c.chan].state.gainMode=DivInstrumentSNES::GAIN_MODE_DIRECT;
            break;
          case 2:
            chan[c.chan].state.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LINEAR;
            break;
          case 3:
            chan[c.chan].state.gainMode=DivInstrumentSNES::GAIN_MODE_DEC_LOG;
            break;
          case 4:
            chan[c.chan].state.gainMode=DivInstrumentSNES::GAIN_MODE_INC_LINEAR;
            break;
          case 5:
            chan[c.chan].state.gainMode=DivInstrumentSNES::GAIN_MODE_INC_INVLOG;
            break;
        }
      } else {
        chan[c.chan].state.useEnv=true;
      }
      chan[c.chan].shallWriteEnv=true;
      break;
    case DIV_CMD_SNES_GAIN:
      if (chan[c.chan].state.gainMode==DivInstrumentSNES::GAIN_MODE_DIRECT) {
        chan[c.chan].state.gain=c.value&0x7f;
      } else {
        chan[c.chan].state.gain=c.value&0x1f;
      }
      if (!chan[c.chan].state.useEnv) chan[c.chan].shallWriteEnv=true;
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      noiseFreq=c.value&0x1f;
      writeControl=true;
      break;
    case DIV_CMD_FM_AR:
      chan[c.chan].state.a=c.value&15;
      if (chan[c.chan].state.useEnv) chan[c.chan].shallWriteEnv=true;
      break;
    case DIV_CMD_FM_DR:
      chan[c.chan].state.d=c.value&7;
      if (chan[c.chan].state.useEnv) chan[c.chan].shallWriteEnv=true;
      break;
    case DIV_CMD_FM_SL:
      chan[c.chan].state.s=c.value&7;
      if (chan[c.chan].state.useEnv) chan[c.chan].shallWriteEnv=true;
      break;
    case DIV_CMD_FM_RR:
      chan[c.chan].state.r=c.value&0x1f;
      if (chan[c.chan].state.useEnv) chan[c.chan].shallWriteEnv=true;
      break;
    case DIV_CMD_SNES_ECHO:
      chan[c.chan].echo=c.value;
      writeEcho=true;
      break;
    case DIV_CMD_SNES_ECHO_DELAY: {
      echoDelay=c.value&15;
      unsigned char esa=0xf8-(echoDelay<<3);
      if (echoOn) {
        rWrite(0x6d,esa);
        rWrite(0x7d,echoDelay);
      }
      break;
    }
    case DIV_CMD_SNES_ECHO_ENABLE:
      echoOn=c.value;
      initEcho();
      break;
    case DIV_CMD_SNES_ECHO_FEEDBACK:
      echoFeedback=c.value;
      if (echoOn) {
        rWrite(0x0d,echoFeedback);
      }
      break;
    case DIV_CMD_SNES_ECHO_FIR:
      echoFIR[c.value&7]=c.value2;
      if (echoOn) {
        rWrite(0x0f+((c.value&7)<<4),echoFIR[c.value&7]);
      }
      break;
    case DIV_CMD_SNES_ECHO_VOL_LEFT:
      echoVolL=c.value;
      if (echoOn) {
        rWrite(0x2c,echoVolL);
      }
      break;
    case DIV_CMD_SNES_ECHO_VOL_RIGHT:
      echoVolR=c.value;
      if (echoOn) {
        rWrite(0x3c,echoVolR);
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
    default:
      break;
  }
  return 1;
}

void DivPlatformSNES::updateWave(int ch) {
  // Due to the overflow bug in hardware's resampler, the written amplitude here is half of maximum
  unsigned short pos=waveTableAddr(ch);
  for (int i=0; i<chan[ch].wtLen/16; i++) {
    sampleMem[pos++]=0xb0;
    for (int j=0; j<8; j++) {
      int nibble1=(chan[ch].ws.output[i*16+j*2]-8)&15;
      int nibble2=(chan[ch].ws.output[i*16+j*2+1]-8)&15;
      sampleMem[pos++]=(nibble1<<4)|nibble2;
    }
  }
  sampleMem[pos-9]=0xb3; // mark loop
}

void DivPlatformSNES::writeOutVol(int ch) {
  int outL=0;
  int outR=0;
  if (!isMuted[ch]) {
    outL=(globalVolL*((chan[ch].outVol*chan[ch].panL)/127))/127;
    outR=(globalVolR*((chan[ch].outVol*chan[ch].panR)/127))/127;
    if (chan[ch].invertL) outL=-outL;
    if (chan[ch].invertR) outR=-outR;
  }
  chWrite(ch,0,outL);
  chWrite(ch,1,outR);
}

void DivPlatformSNES::writeEnv(int ch) {
  if (chan[ch].state.useEnv) {
    if (chan[ch].state.sus) {
      if (chan[ch].active) {
        chWrite(ch,5,chan[ch].state.a|(chan[ch].state.d<<4)|0x80);
        chWrite(ch,6,(chan[ch].state.s<<5)|(chan[ch].state.d2&31));
      } else {
        switch (chan[ch].state.sus) {
          case 1: // dec linear
            chWrite(ch,7,0x80|chan[ch].state.r);
            chWrite(ch,5,0);
            break;
          case 2: // dec exp
            chWrite(ch,7,0xa0|chan[ch].state.r);
            chWrite(ch,5,0);
            break;
          case 3: // update r
            chWrite(ch,6,(chan[ch].state.s<<5)|(chan[ch].state.r&31));
            break;
          default: // what?
            break;
        }
      }
    } else {
      chWrite(ch,5,chan[ch].state.a|(chan[ch].state.d<<4)|0x80);
      chWrite(ch,6,chan[ch].state.r|(chan[ch].state.s<<5));
    }
  } else {
    chWrite(ch,5,0);
    switch (chan[ch].state.gainMode) {
      case DivInstrumentSNES::GAIN_MODE_DIRECT:
        chWrite(ch,7,chan[ch].state.gain&127);
        break;
      case DivInstrumentSNES::GAIN_MODE_DEC_LINEAR:
        chWrite(ch,7,0x80|(chan[ch].state.gain&31));
        break;
      case DivInstrumentSNES::GAIN_MODE_INC_LINEAR:
        chWrite(ch,7,0xc0|(chan[ch].state.gain&31));
        break;
      case DivInstrumentSNES::GAIN_MODE_DEC_LOG:
        chWrite(ch,7,0xa0|(chan[ch].state.gain&31));
        break;
      case DivInstrumentSNES::GAIN_MODE_INC_INVLOG:
        chWrite(ch,7,0xe0|(chan[ch].state.gain&31));
        break;
    }
  }
}

void DivPlatformSNES::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].shallWriteVol=true;
}

void DivPlatformSNES::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].sample=-1;
    if (chan[i].active && chan[i].useWave) {
      updateWave(i);
    }
    writeOutVol(i);
  }
  writeControl=true;
  writeNoise=true;
  writePitchMod=true;
  writeEcho=true;
  initEcho();
}

void* DivPlatformSNES::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformSNES::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformSNES::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSNES::getRegisterPool() {
  // get states from emulator
  for (int i=0; i<0x80; i+=0x10) {
    regPool[i+8]=dsp.read(i+8);
    regPool[i+9]=dsp.read(i+9);
  }
  regPool[0x7c]=dsp.read(0x7c); // ENDX
  return regPool;
}

int DivPlatformSNES::getRegisterPoolSize() {
  return 128;
}

void DivPlatformSNES::initEcho() {
  unsigned char esa=0xf8-(echoDelay<<3);
  if (echoOn) {
    rWrite(0x6d,esa);
    rWrite(0x7d,echoDelay);
    rWrite(0x0d,echoFeedback);
    rWrite(0x2c,echoVolL);
    rWrite(0x3c,echoVolR);
    for (int i=0; i<8; i++) {
      rWrite(0x0f+(i<<4),echoFIR[i]);
    }
  } else {
    rWrite(0x6d,0);
    rWrite(0x7d,0);
    rWrite(0x2c,0);
    rWrite(0x3c,0);
  }
  writeControl=true;
}

void DivPlatformSNES::reset() {
  memcpy(sampleMem,copyOfSampleMem,65536);
  dsp.init(sampleMem);
  dsp.set_output(NULL,0);
  memset(regPool,0,128);
  // this can't be 0 or channel 1 won't play
  // this can't be 0x100 either as that's used by SPC700 page 1 and the stack
  // this may not even be 0x200 as some space will be taken by the playback routine and variables
  sampleTableBase=0x200;
  rWrite(0x5d,sampleTableBase>>8);
  rWrite(0x0c,127); // global volume left
  rWrite(0x1c,127); // global volume right
  rWrite(0x6c,0); // get DSP out of reset
  for (int i=0; i<8; i++) {
    chan[i]=Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,15);
    writeOutVol(i);
    chWrite(i,4,i); // source number
  }
  writeControl=false;
  writeNoise=false;
  writePitchMod=false;
  writeEcho=true;

  echoDelay=initEchoDelay;
  echoFeedback=initEchoFeedback;
  echoFIR[0]=initEchoFIR[0];
  echoFIR[1]=initEchoFIR[1];
  echoFIR[2]=initEchoFIR[2];
  echoFIR[3]=initEchoFIR[3];
  echoFIR[4]=initEchoFIR[4];
  echoFIR[5]=initEchoFIR[5];
  echoFIR[6]=initEchoFIR[6];
  echoFIR[7]=initEchoFIR[7];
  echoVolL=initEchoVolL;
  echoVolR=initEchoVolR;
  echoOn=initEchoOn;
  
  for (int i=0; i<8; i++) {
    if (initEchoMask&(1<<i)) {
      chan[i].echo=true;
    }
  }

  initEcho();
}

bool DivPlatformSNES::isStereo() {
  return true;
}

void DivPlatformSNES::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformSNES::notifyWaveChange(int wave) {
  for (int i=0; i<8; i++) {
    if (chan[i].useWave && chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      if (chan[i].active) {
        updateWave(i);
      }
    }
  }
}

void DivPlatformSNES::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSNES::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformSNES::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const void* DivPlatformSNES::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformSNES::getSampleMemCapacity(int index) {
  // TODO change it based on current echo buffer size
  return index == 0 ? 65536 : 0;
}

size_t DivPlatformSNES::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

bool DivPlatformSNES::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

void DivPlatformSNES::renderSamples(int sysID) {
  memset(copyOfSampleMem,0,getSampleMemCapacity());
  memset(sampleOff,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  // skip past sample table and wavetable buffer
  size_t memPos=sampleTableBase+8*4+8*9*16;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }

    int length=s->lengthBRR+((s->loop && s->depth!=DIV_SAMPLE_DEPTH_BRR)?9:0);
    int actualLength=MIN((int)(getSampleMemCapacity()-memPos)/9*9,length);
    if (actualLength>0) {
      sampleOff[i]=memPos;
      memcpy(&copyOfSampleMem[memPos],s->dataBRR,actualLength);
      // inject loop if needed
      if (s->loop) {
        copyOfSampleMem[memPos+actualLength-9]|=3;
      }
      memPos+=actualLength;
    }
    if (actualLength<length) {
      // terminate the sample
      copyOfSampleMem[memPos-9]=1;
      logW("out of BRR memory for sample %d!",i);
      break;
    }
    sampleLoaded[i]=true;
  }
  sampleMemLen=memPos;
  memcpy(sampleMem,copyOfSampleMem,65536);
}

void DivPlatformSNES::setFlags(const DivConfig& flags) {
  globalVolL=127-flags.getInt("volScaleL",0);
  globalVolR=127-flags.getInt("volScaleR",0);

  initEchoOn=flags.getBool("echo",false);
  initEchoVolL=flags.getInt("echoVolL",127);
  initEchoVolR=flags.getInt("echoVolR",127);
  initEchoDelay=flags.getInt("echoDelay",0)&15;
  initEchoFeedback=flags.getInt("echoFeedback",0);

  initEchoFIR[0]=flags.getInt("echoFilter0",127);
  initEchoFIR[1]=flags.getInt("echoFilter1",0);
  initEchoFIR[2]=flags.getInt("echoFilter2",0);
  initEchoFIR[3]=flags.getInt("echoFilter3",0);
  initEchoFIR[4]=flags.getInt("echoFilter4",0);
  initEchoFIR[5]=flags.getInt("echoFilter5",0);
  initEchoFIR[6]=flags.getInt("echoFilter6",0);
  initEchoFIR[7]=flags.getInt("echoFilter7",0);

  initEchoMask=flags.getInt("echoMask",0);
}

int DivPlatformSNES::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  sampleMemLen=0;
  chipClock=1024000;
  rate=chipClock/32;
  for (int i=0; i<8; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->rate=rate;
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();
  return 8;
}

void DivPlatformSNES::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}
