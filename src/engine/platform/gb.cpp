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

#include "gb.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }
#define immWrite(a,v) {writes.emplace(a,v); regPool[(a)&0x7f]=v; if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER 16

const char* regCheatSheetGB[]={
  "NR10_Sweep", "10",
  "NR11_DutyLen", "11",
  "NR12_VolEnv", "12",
  "NR13_FreqL", "13",
  "NR14_FreqH", "14",

  "NR21_DutyLen", "16",
  "NR22_VolEnv", "17",
  "NR23_FreqL", "18",
  "NR24_FreqH", "19",

  "NR30_WaveOn", "1A",
  "NR31_Len", "1B",
  "NR32_Vol", "1C",
  "NR33_FreqL", "1D",
  "NR34_FreqH", "1E",

  "NR41_Len", "20",
  "NR42_VolEnv", "21",
  "NR43_Freq", "22",
  "NR44_Control", "23",

  "NR50_MasterVol", "24",
  "NR51_Toggle", "25",
  "NR52_PowerStat", "26",

  "Wave", "30",
  NULL
};

const char** DivPlatformGB::getRegisterSheet() {
  return regCheatSheetGB;
}

void DivPlatformGB::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    if (!writes.empty()) {
      QueuedWrite& w=writes.front();
      GB_apu_write(gb,w.addr,w.val);
      writes.pop();
    }

    GB_advance_cycles(gb,16);
    bufL[i]=gb->apu_output.final_sample.left;
    bufR[i]=gb->apu_output.final_sample.right;

    for (int i=0; i<4; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(gb->apu_output.current_sample[i].left+gb->apu_output.current_sample[i].right)<<6;
    }
  }
}

void DivPlatformGB::updateWave() {
  rWrite(0x1a,0);
  for (int i=0; i<16; i++) {
    int nibble1=15-ws.output[((i<<1)+antiClickWavePos-1)&31];
    int nibble2=15-ws.output[((1+(i<<1))+antiClickWavePos-1)&31];
    rWrite(0x30+i,(nibble1<<4)|nibble2);
  }
  antiClickWavePos&=31;
}

static unsigned char chanMuteMask[4]={
  0xee, 0xdd, 0xbb, 0x77
};

unsigned char DivPlatformGB::procMute() {
  return lastPan&(isMuted[0]?chanMuteMask[0]:0xff)
                &(isMuted[1]?chanMuteMask[1]:0xff)
                &(isMuted[2]?chanMuteMask[2]:0xff)
                &(isMuted[3]?chanMuteMask[3]:0xff);
}

static unsigned char gbVolMap[16]={
  0x00, 0x00, 0x00, 0x00,
  0x60, 0x60, 0x60, 0x60,
  0x40, 0x40, 0x40, 0x40,
  0x20, 0x20, 0x20, 0x20
};

static unsigned char noiseTable[256]={
  0,
  0xf7, 0xf6, 0xf5, 0xf4,
  0xe7, 0xe6, 0xe5, 0xe4,
  0xd7, 0xd6, 0xd5, 0xd4,
  0xc7, 0xc6, 0xc5, 0xc4,
  0xb7, 0xb6, 0xb5, 0xb4,
  0xa7, 0xa6, 0xa5, 0xa4,
  0x97, 0x96, 0x95, 0x94,
  0x87, 0x86, 0x85, 0x84,
  0x77, 0x76, 0x75, 0x74,
  0x67, 0x66, 0x65, 0x64,
  0x57, 0x56, 0x55, 0x54,
  0x47, 0x46, 0x45, 0x44,
  0x37, 0x36, 0x35, 0x34,
  0x27, 0x26, 0x25, 0x24,
  0x17, 0x16, 0x15, 0x14,
  0x07, 0x06, 0x05, 0x04,
  0x03, 0x02, 0x01, 0x00,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void DivPlatformGB::tick(bool sysTick) {
  if (antiClickEnabled && sysTick && chan[2].freq>0) {
    antiClickPeriodCount+=((chipClock>>1)/MAX(parent->getCurHz(),1.0f));
    antiClickWavePos+=antiClickPeriodCount/chan[2].freq;
    antiClickPeriodCount%=chan[2].freq;
  }

  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].softEnv) {
      if (chan[i].std.vol.had) {
        chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
        if (chan[i].outVol<0) chan[i].outVol=0;

        if (i==2) {
          rWrite(16+i*5+2,gbVolMap[chan[i].outVol]);
          chan[i].soundLen=64;
        } else {
          chan[i].envLen=0;
          chan[i].envDir=1;
          chan[i].envVol=chan[i].outVol;
          chan[i].soundLen=64;

          if (!chan[i].keyOn) chan[i].killIt=true;
        }
      }
    }
    if (NEW_ARP_STRAT && i!=3) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (i==3) { // noise
        chan[i].baseFreq=parent->calcArp(chan[i].note,chan[i].std.arp.val,24);
        if (chan[i].baseFreq>255) chan[i].baseFreq=255;
        if (chan[i].baseFreq<0) chan[i].baseFreq=0;
      } else {
        if (!chan[i].inPorta) {
          chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val,24));
        }
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      chan[i].duty=chan[i].std.duty.val;
      if (i!=2) {
        rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(chan[i].soundLen&63)));
      } else if (!chan[i].softEnv) {
        if (parent->song.waveDutyIsVol) {
          rWrite(16+i*5+2,gbVolMap[(chan[i].std.duty.val&3)<<2]);
        }
      }
    }
    if (i==2 && chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].std.panL.had) {
      lastPan&=~(0x11<<i);
      lastPan|=((chan[i].std.panL.val&1)<<i)|((chan[i].std.panL.val&2)<<(i+3));
      rWrite(0x25,procMute());
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
      if (chan[i].std.phaseReset.val==1) {
        chan[i].keyOn=true;
        if (i==2) {
          antiClickWavePos=0;
          antiClickPeriodCount=0;
        }
      }
    }
    if (i==2) {
      if (chan[i].active) {
        if (ws.tick()) {
          updateWave();
          if (!chan[i].keyOff) chan[i].keyOn=true;
        }
      }
    }
    // run hardware sequence
    if (chan[i].active) {
      if (--chan[i].hwSeqDelay<=0) {
        chan[i].hwSeqDelay=0;
        DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_GB);
        int hwSeqCount=0;
        while (chan[i].hwSeqPos<ins->gb.hwSeqLen && hwSeqCount<4) {
          bool leave=false;
          unsigned short data=ins->gb.hwSeq[chan[i].hwSeqPos].data;
          switch (ins->gb.hwSeq[chan[i].hwSeqPos].cmd) {
            case DivInstrumentGB::DIV_GB_HWCMD_ENVELOPE:
              if (!chan[i].softEnv) {
                chan[i].envLen=data&7;
                chan[i].envDir=(data&8)?1:0;
                chan[i].envVol=(data>>4)&15;
                chan[i].soundLen=data>>8;
                chan[i].keyOn=true;
              }
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_SWEEP:
              chan[i].sweep=data;
              chan[i].sweepChanged=true;
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_WAIT:
              chan[i].hwSeqDelay=data+1;
              leave=true;
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_WAIT_REL:
              if (!chan[i].released) {
                chan[i].hwSeqPos--;
                leave=true;
              }
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_LOOP:
              chan[i].hwSeqPos=data-1;
              break;
            case DivInstrumentGB::DIV_GB_HWCMD_LOOP_REL:
              if (!chan[i].released) {
                chan[i].hwSeqPos=data-1;
              }
              break;
          }

          chan[i].hwSeqPos++;
          if (leave) break;
          hwSeqCount++;
        }
      }
    }

    if (chan[i].sweepChanged) {
      chan[i].sweepChanged=false;
      if (i==0) {
        rWrite(16+i*5,chan[i].sweep);
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      if (i==3) { // noise
        int ntPos=chan[i].baseFreq;
        if (ntPos<0) ntPos=0;
        if (ntPos>255) ntPos=255;
        chan[i].freq=noiseTable[ntPos];
      } else {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
        if (chan[i].freq>2047) chan[i].freq=2047;
        if (chan[i].freq<0) chan[i].freq=0;
      }
      if (chan[i].keyOn) {
        if (i==2) { // wave
          rWrite(16+i*5,0x80);
          rWrite(16+i*5+2,gbVolMap[chan[i].outVol]);
        } else {
          rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(chan[i].soundLen&63)));
          rWrite(16+i*5+2,((chan[i].envVol<<4))|(chan[i].envLen&7)|((chan[i].envDir&1)<<3));
          chan[i].lastKill=chan[i].envVol;
        }
      }
      if (chan[i].keyOff) {
        if (i==2) {
          rWrite(16+i*5+2,0);
        } else {
          rWrite(16+i*5+2,8);
        }
      }
      if (i==3) { // noise
        rWrite(16+i*5+3,(chan[i].freq&0xff)|(chan[i].duty?8:0));
        rWrite(16+i*5+4,((chan[i].keyOn||chan[i].keyOff)?0x80:0x00)|((chan[i].soundLen<64)<<6));
      } else {
        rWrite(16+i*5+3,(2048-chan[i].freq)&0xff);
        rWrite(16+i*5+4,(((2048-chan[i].freq)>>8)&7)|((chan[i].keyOn||chan[i].keyOff)?0x80:0x00)|((chan[i].soundLen<63)<<6));
      }
      if (enoughAlready) { // more compat garbage
        rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(chan[i].soundLen&63)));
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
    if (chan[i].killIt) {
      if (i!=2) {
        //rWrite(16+i*5+2,8);
        int killDelta=chan[i].lastKill-chan[i].outVol+1;
        if (killDelta<0) killDelta+=16;
        chan[i].lastKill=chan[i].outVol;

        if (killDelta!=1) {
          rWrite(16+i*5+2,((chan[i].envVol<<4))|8);
          for (int j=0; j<killDelta; j++) {
            rWrite(16+i*5+2,0x09);
            rWrite(16+i*5+2,0x11);
            rWrite(16+i*5+2,0x08);
          }
        }
      }
      chan[i].killIt=false;
    }

    chan[i].soManyHacksToMakeItDefleCompatible=false;
  }
}

void DivPlatformGB::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  rWrite(0x25,procMute());
}

int DivPlatformGB::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_GB);
      if (c.value!=DIV_NOTE_NULL) {
        if (c.chan==3) { // noise
          chan[c.chan].baseFreq=c.value;
        } else {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        }
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].hwSeqPos=0;
      chan[c.chan].hwSeqDelay=0;
      chan[c.chan].released=false;
      chan[c.chan].softEnv=ins->gb.softEnv;
      chan[c.chan].macroInit(ins);
      if (c.chan==2) {
        if (chan[c.chan].wave<0) {
          chan[c.chan].wave=0;
          ws.changeWave1(chan[c.chan].wave);
        }
        ws.init(ins,32,15,chan[c.chan].insChanged);
      }
      if ((chan[c.chan].insChanged || ins->gb.alwaysInit) && !chan[c.chan].softEnv) {
        if (!chan[c.chan].soManyHacksToMakeItDefleCompatible && c.chan!=2) {
          chan[c.chan].envVol=ins->gb.envVol;
        }
        chan[c.chan].envLen=ins->gb.envLen;
        chan[c.chan].envDir=ins->gb.envDir;
        chan[c.chan].soundLen=ins->gb.soundLen;
        if (!chan[c.chan].soManyHacksToMakeItDefleCompatible && c.chan!=2) {
          chan[c.chan].vol=chan[c.chan].envVol;
          chan[c.chan].outVol=chan[c.chan].envVol;
        }
      }
      if (c.chan==2 && chan[c.chan].softEnv) {
        chan[c.chan].soundLen=64;
      }
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].hwSeqPos=0;
      chan[c.chan].hwSeqDelay=0;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      chan[c.chan].released=true;
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
        if (c.chan!=2) {
          DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_GB);
          if (!ins->gb.softEnv) {
            chan[c.chan].envVol=ins->gb.envVol;
            chan[c.chan].envLen=ins->gb.envLen;
            chan[c.chan].envDir=ins->gb.envDir;
            chan[c.chan].soundLen=ins->gb.soundLen;
            chan[c.chan].vol=chan[c.chan].envVol;
            chan[c.chan].outVol=chan[c.chan].vol;
            if (parent->song.gbInsAffectsEnvelope) {
              rWrite(16+c.chan*5+2,((chan[c.chan].vol<<4))|(chan[c.chan].envLen&7)|((chan[c.chan].envDir&1)<<3));
            }
          }
        }
      }
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      chan[c.chan].outVol=c.value;
      if (c.chan==2) {
        rWrite(16+c.chan*5+2,gbVolMap[chan[c.chan].outVol]);
      }
      if (!chan[c.chan].softEnv) {
        chan[c.chan].envVol=chan[c.chan].vol;
        chan[c.chan].soManyHacksToMakeItDefleCompatible=true;
      } else if (c.chan!=2) {
        if (chan[c.chan].std.vol.will && !chan[c.chan].std.vol.finished) {
          chan[c.chan].outVol=VOL_SCALE_LINEAR(chan[c.chan].vol&15,MIN(15,chan[c.chan].std.vol.val),15);
        }
        chan[c.chan].envVol=chan[c.chan].outVol;
        
        if (!chan[c.chan].keyOn) chan[c.chan].killIt=true;
        chan[c.chan].freqChanged=true;
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      if (c.chan!=2) break;
      chan[c.chan].wave=c.value;
      ws.changeWave1(chan[c.chan].wave);
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
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
      if (c.chan!=2) {
        chan[c.chan].freqChanged=true;
        rWrite(16+c.chan*5+1,((chan[c.chan].duty&3)<<6)|(63-(parent->getIns(chan[c.chan].ins,DIV_INS_GB)->gb.soundLen&63)));
      }
      break;
    case DIV_CMD_PANNING: {
      lastPan&=~(0x11<<c.chan);
      int pan=0;
      if (c.value>0) pan|=0x10;
      if (c.value2>0) pan|=0x01;
      if (pan==0) pan=0x11;
      lastPan|=pan<<c.chan;
      rWrite(0x25,procMute());
      break;
    }
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_GB));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GB_SWEEP_DIR:
      if (c.chan>0) break;
      chan[c.chan].sweep&=0xf7;
      if (c.value&1) {
        chan[c.chan].sweep|=8;
      }
      chan[c.chan].sweepChanged=true;
      break;
    case DIV_CMD_GB_SWEEP_TIME:
      if (c.chan>0) break;
      chan[c.chan].sweep&=8;
      chan[c.chan].sweep|=c.value&0x77;
      chan[c.chan].sweepChanged=true;
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
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformGB::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
  }
  immWrite(0x25,procMute());
  updateWave();
}

void* DivPlatformGB::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformGB::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformGB::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformGB::getRegisterPool() {
  return regPool;
}

int DivPlatformGB::getRegisterPoolSize() {
  return 64;
}

void DivPlatformGB::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformGB::Channel();
    chan[i].std.setEngine(parent);
  }
  ws.setEngine(parent);
  ws.init(NULL,32,15,false);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  memset(gb,0,sizeof(GB_gameboy_t));
  memset(regPool,0,128);
  gb->model=model;
  GB_apu_init(gb);
  GB_set_sample_rate(gb,rate);
  // enable all channels
  immWrite(0x10,0);
  immWrite(0x26,0x8f);
  lastPan=0xff;
  immWrite(0x25,procMute());
  immWrite(0x24,0x77);

  antiClickPeriodCount=0;
  antiClickWavePos=0;
}

int DivPlatformGB::getPortaFloor(int ch) {
  return 24;
}

bool DivPlatformGB::isStereo() {
  return true;
}

bool DivPlatformGB::getDCOffRequired() {
  return (model==GB_MODEL_AGB);
}

void DivPlatformGB::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGB::notifyWaveChange(int wave) {
  if (chan[2].wave==wave) {
    ws.changeWave1(wave);
    updateWave();
    if (!chan[2].keyOff && chan[2].active) chan[2].keyOn=true;
  }
}

void DivPlatformGB::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformGB::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformGB::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformGB::setFlags(const DivConfig& flags) {
  antiClickEnabled=!flags.getBool("noAntiClick",false);
  switch (flags.getInt("chipType",0)) {
    case 0:
      model=GB_MODEL_DMG_B;
      break;
    case 1:
      model=GB_MODEL_CGB_C;
      break;
    case 2:
      model=GB_MODEL_CGB_E;
      break;
    case 3:
      model=GB_MODEL_AGB;
      break;
  }
  enoughAlready=flags.getBool("enoughAlready",false);
}

int DivPlatformGB::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  chipClock=4194304;
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/16;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->rate=rate;
  }
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  model=GB_MODEL_DMG_B;
  gb=new GB_gameboy_t;
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformGB::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  delete gb;
}

DivPlatformGB::~DivPlatformGB() {
}
