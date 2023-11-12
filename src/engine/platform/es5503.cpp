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

#include "es5503.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>

const int ES5503_wave_lengths[DivInstrumentES5503::DIV_ES5503_WAVE_LENGTH_MAX] = {256, 512, 1024, 2048, 4096, 8192, 16384, 32768};

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_DIVIDER (chipClock / rate)

#define CHIP_FREQBASE (894886U)

const char* regCheatSheetES5503[]={
  "CHx_FreqL", "x",
  "CHx_FreqH", "20+x",
  "CHx_Vol", "40+x",
  "CHx_CurrSample", "60+x",
  "CHx_WaveAddress", "80+x",
  "CHx_Control", "A0+x",
  "CHx_WaveSize", "C0+x",
  "Osc_interrupt", "E0",
  "Osc_enable", "E1",
  "A/D_conversion_result", "E2",
  NULL
};


const char** DivPlatformES5503::getRegisterSheet() {
  return regCheatSheetES5503;
}

void DivPlatformES5503::acquire(short** buf, size_t len) { //the function where we actually fill the fucking audio buffer!!!!!
  es5503.fill_audio_buffer(buf[0], buf[1], len);

  while (!writes.empty()) { //do register writes
      QueuedWrite w=writes.front();
      es5503.write(w.addr,w.val);
      regPool[w.addr]=w.val;
      writes.pop();
    }
}

void DivPlatformES5503::setFlags(const DivConfig& flags) {
  chipClock=894886U; //894886 Hz on Apple IIGS

  CHECK_CUSTOM_CLOCK;
  rate=29410; //29.41 kHz for Apple IIGS card with all oscillators enabled

  for (int i=0; i<32; i++) {
    oscBuf[i]->rate=rate;
  }

  es5503.es5503_core_init(chipClock, this->oscBuf);
}

void DivPlatformES5503::writeSampleMemoryByte(int address, unsigned char value)
{
  if(es5503.sampleMem)
  {
    //int8_t actual_val = (int8_t)value - 128;
    es5503.sampleMem[address] = value;//actual_val; //signed???
  }
}

void DivPlatformES5503::updateWave(int ch) {
  if (chan[ch].pcm) {
    chan[ch].deferredWaveUpdate=true;
    return;
  }

  //chWrite(ch,0x04,0x5f);
  //chWrite(ch,0x04,0x1f);

  for (int i=0; i<chan[ch].wave_size; i++) {
    //chWrite(ch,0x06,chan[ch].ws.output[(i+chan[ch].antiClickWavePos)&31]);
    writeSampleMemoryByte(chan[ch].wave_pos + i, chan[ch].ws.output[i&255]); //if using synthesized wavetable
  }
  //chan[ch].antiClickWavePos&=31;
  if (chan[ch].active) {
    //chWrite(ch,0x04,0x80|chan[ch].outVol);
  }
  
  if (chan[ch].deferredWaveUpdate) {
    chan[ch].deferredWaveUpdate=false;
  }
}

void DivPlatformES5503::tick(bool sysTick) {
  for(int i = 0; i < 32; i++)
  {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=chan[i].std.vol.val;
      if (chan[i].furnaceDac && chan[i].pcm) {
        // ignore for now
      } else {
        rWrite(0x40 + i, isMuted[i] ? 0 : chan[i].outVol);
      }
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        int noiseSeek=parent->calcArp(chan[i].note,chan[i].std.arp.val);
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.wave.had && !chan[i].pcm) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }

    if (chan[i].active) {
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) || chan[i].deferredWaveUpdate) {
        updateWave(i);
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_ES5503);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,0,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].furnaceDac && chan[i].pcm) {
        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=8363.0/(double)s->centerRate;
          }
        }
        chan[i].dacRate=((double)chipClock/2)/MAX(1,off*chan[i].freq);
        //if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dacRate);
      }

      rWrite(i, chan[i].freq&0xff);
      rWrite(0x20+i, chan[i].freq>>8);
      
      //chWrite(i,0x02,chan[i].freq&0xff);
      //chWrite(i,0x03,chan[i].freq>>8);

      if (chan[i].keyOn) {
        rWrite(0xA0+i, 0 | (ins->es5503.initial_osc_mode << 1)); //reset halt bit and set oscillator mode
        rWrite(0x40+i, isMuted[i] ? 0 : chan[i].vol); //set volume
      }
      if (chan[i].keyOff) {
        rWrite(0xA0+i, 1); //halt oscillator
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformES5503::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_ES5503);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:255;
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
        chan[c.chan].pcm=true;
      } else if (chan[c.chan].furnaceDac) {
        chan[c.chan].pcm=false;
      }
      if (chan[c.chan].pcm) {
        /*if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
          chan[c.chan].furnaceDac=true;
          if (skipRegisterWrites) break;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].dacSample=ins->amiga.getSample(c.value);
            c.value=ins->amiga.getFreq(c.value);
          }
          if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
             if (dumpWrites) {
               //chWrite(c.chan,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[c.chan].vol));
               //addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
             }
          }
          chan[c.chan].dacPos=0;
          chan[c.chan].dacPeriod=0;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
            chan[c.chan].freqChanged=true;
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].macroInit(ins);
          if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
          }
          //chan[c.chan].keyOn=true;
        } else {
          chan[c.chan].furnaceDac=false;
          if (skipRegisterWrites) break;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          chan[c.chan].dacSample=12*sampleBank+chan[c.chan].note%12;
          if (chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            if (dumpWrites) addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
          }
          chan[c.chan].dacPos=0;
          chan[c.chan].dacPeriod=0;
          chan[c.chan].dacRate=parent->getSample(chan[c.chan].dacSample)->rate;
          if (dumpWrites) {
            chWrite(c.chan,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[c.chan].vol));
            addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dacRate);
          }
        }*/
        break;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      //chWrite(c.chan,0x04,0x80|chan[c.chan].vol);
      rWrite(0x40+c.chan, chan[c.chan].vol); //set volume
      rWrite(0x80+c.chan, ins->es5503.wavePos); //set wave pos
      rWrite(0xc0+c.chan, ins->es5503.waveLen << 3 | 0b001 /*lowest acc resolution*/); //set wave len
      chan[c.chan].wave_pos = ins->es5503.wavePos << 8;
      chan[c.chan].wave_size = ES5503_wave_lengths[ins->es5503.waveLen&7];
      chan[c.chan].macroInit(ins);
      chan[c.chan].outVol=chan[c.chan].vol;

      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }

      chan[c.chan].ws.init(ins,256,255,chan[c.chan].insChanged);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dacSample=-1;
      //if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].pcm=false;
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
  }
  return 1;
}

void DivPlatformES5503::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  /*chWrite(ch,0x05,isMuted[ch]?0:chan[ch].pan);
  if (!isMuted[ch] && (chan[ch].pcm && chan[ch].dacSample!=-1)) {
    chWrite(ch,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[ch].outVol));
    chWrite(ch,0x06,chan[ch].dacOut&0x1f);
  }*/
}

void DivPlatformES5503::forceIns() {
  for (int i=0; i<32; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
    rWrite(0x40+i,isMuted[i]?0:chan[i].vol);
  }
}

void* DivPlatformES5503::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformES5503::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivSamplePos DivPlatformES5503::getSamplePos(int ch) {
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformES5503::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformES5503::getRegisterPool() {
  return regPool;
}

int DivPlatformES5503::getRegisterPoolSize() {
  return 256;
}

void DivPlatformES5503::reset() {
  writes.clear();
  memset(regPool,0,256);
  for (int i=0; i<32; i++) {
    chan[i]=DivPlatformES5503::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,256,255,false);
  }

  curChan=-1;
  // set volume to zero
  for(int i = 0; i < 32; i++)
  {
    rWrite(0x40+i,0);
  }
}

int DivPlatformES5503::getOutputCount() {
  return 2;
}

bool DivPlatformES5503::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformES5503::notifyWaveChange(int wave) {
  for (int i=0; i<32; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformES5503::notifyInsChange(int ins) {
  for (int i=0; i<32; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformES5503::notifyInsDeletion(void* ins) {
  for (int i=0; i<32; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformES5503::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformES5503::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformES5503::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  memset(this->regPool, 0, sizeof(this->regPool[0]) * 256);
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<32; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();
  return 32;
}

void DivPlatformES5503::quit() {
  for (int i=0; i<32; i++) {
    delete oscBuf[i];
  }
  es5503.es5503_core_free();
}

DivPlatformES5503::~DivPlatformES5503() {
}
