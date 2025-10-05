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

#include "n163.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }
#define rWriteMask(a,v,m) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v,m)); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) \
  if (c<=chanMax) { \
    rWrite(0x78-(c<<3)+(a&7),v) \
  }

#define chWriteMask(c,a,v,m) \
  if (c<=chanMax) { \
    rWriteMask(0x78-(c<<3)+(a&7),v,m) \
  }

#define CHIP_FREQBASE (15*524288)

const char* regCheatSheetN163[]={
  "FreqL7", "40",
  "AccL7", "41",
  "FreqM7", "42",
  "AccM7", "43",
  "WavLen_FreqH7", "44",
  "AccH7", "45",
  "WavPos7", "46",
  "Vol7", "47",
  "FreqL6", "48",
  "AccL6", "49",
  "FreqM6", "4A",
  "AccM6", "4B",
  "WavLen_FreqH6", "4C",
  "AccH6", "4D",
  "WavPos6", "4E",
  "Vol6", "4F",
  "FreqL5", "50",
  "AccL5", "51",
  "FreqM5", "52",
  "AccM5", "53",
  "WavLen_FreqH5", "54",
  "AccH5", "55",
  "WavPos5", "56",
  "Vol5", "57",
  "FreqL4", "58",
  "AccL4", "59",
  "FreqM4", "5A",
  "AccM4", "5B",
  "WavLen_FreqH4", "5C",
  "AccH4", "5D",
  "WavPos4", "5E",
  "Vol4", "5F",
  "FreqL3", "60",
  "AccL3", "61",
  "FreqM3", "62",
  "AccM3", "63",
  "WavLen_FreqH3", "64",
  "AccH3", "65",
  "WavPos3", "66",
  "Vol3", "67",
  "FreqL2", "68",
  "AccL2", "69",
  "FreqM2", "6A",
  "AccM2", "6B",
  "WavLen_FreqH2", "6C",
  "AccH2", "6D",
  "WavPos2", "6E",
  "Vol2", "6F",
  "FreqL1", "70",
  "AccL1", "71",
  "FreqM1", "72",
  "AccM1", "73",
  "WavLen_FreqH1", "74",
  "AccH1", "75",
  "WavPos1", "76",
  "Vol1", "77",
  "FreqL0", "78",
  "AccL0", "79",
  "FreqM0", "7A",
  "AccM0", "7B",
  "WavLen_FreqH0", "7C",
  "AccH0", "7D",
  "WavPos0", "7E",
  "ChanMax_Vol0", "7F",
  NULL
};

const char** DivPlatformN163::getRegisterSheet() {
  return regCheatSheetN163;
}

void DivPlatformN163::acquire(short** buf, size_t len) {
  for (int i=0; i<8; i++) {
    oscBuf[i]->begin(len);
  }
  
  for (size_t i=0; i<len; i++) {
    n163.tick();
    int out=(n163.out()<<6)*2; // scale to 16 bit
    if (out>32767) out=32767;
    if (out<-32768) out=-32768;
    buf[0][i]=out;

    if (n163.voice_cycle()==0x78) for (int j=0; j<8; j++) {
      oscBuf[j]->putSample(i,n163.voice_out(j)<<7);
    }

    // command queue
    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      n163.addr_w(w.addr);
      n163.data_w((n163.data_r()&~w.mask)|(w.val&w.mask));
      writes.pop();
    }
  }

  for (int i=0; i<8; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformN163::updateWave(int ch, int wave, int pos, int len) {
  len&=0xfc; // 4 nibble boundary
  if (wave<0) {
    // load from wave synth
    if (ch>=0) {
      for (int i=0; i<len; i++) {
        unsigned char addr=(pos+i); // address (nibble each)
        if (addr>=((0x78-(chanMax<<3))<<1)) { // avoid conflict with channel register area
          break;
        }
        unsigned char mask=(addr&1)?0xf0:0x0f;
        int data=chan[ch].ws.output[i];
        rWriteMask(addr>>1,(addr&1)?(data<<4):(data&0xf),mask);
      }
    }
  } else {
    // load from custom
    DivWavetable* wt=parent->getWave(wave);
    for (int i=0; i<wt->len; i++) {
      unsigned char addr=(pos+i); // address (nibble each)
      if (addr>=((0x78-(chanMax<<3))<<1)) { // avoid conflict with channel register area
        break;
      }
      unsigned char mask=(addr&1)?0xf0:0x0f;
      if (wt->max<1 || wt->len<1) {
        rWriteMask(addr>>1,0,mask);
      } else {
        int data=wt->data[i]*15/wt->max;
        if (data<0) data=0;
        if (data>15) data=15;
        rWriteMask(addr>>1,(addr&1)?(data<<4):(data&0xf),mask);
      }
    }
  }
}

void DivPlatformN163::updateWaveCh(int ch) {
  if (ch<=chanMax) {
    //logV("updateWave with pos %d and len %d",chan[ch].wavePos,chan[ch].waveLen);
    updateWave(ch,-1,chan[ch].wavePos,chan[ch].waveLen);
    if (chan[ch].active && !isMuted[ch]) {
      chan[ch].volumeChanged=true;
    }
  }
}

void DivPlatformN163::tick(bool sysTick) {
  for (int i=0; i<=chanMax; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(MIN(15,chan[i].std.vol.val)*(chan[i].vol&15))/15;
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (chan[i].outVol>15) chan[i].outVol=15;
      if (chan[i].resVol!=chan[i].outVol) {
        chan[i].resVol=chan[i].outVol;
        if (!isMuted[i]) {
          chan[i].volumeChanged=true;
        }
      }
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
      if (chan[i].curWavePos!=chan[i].std.duty.val) {
        chan[i].curWavePos=chan[i].std.duty.val;
        chan[i].waveChanged=true;
      }
    }
    if (chan[i].std.wave.had) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (chan[i].waveMode) {
          chan[i].waveUpdated=true;
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
    if (chan[i].std.ex1.had) {
      if (chan[i].curWaveLen!=(chan[i].std.ex1.val&0xfc)) {
        chan[i].curWaveLen=chan[i].std.ex1.val&0xfc;
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].volumeChanged) {
      if (chan[i].active && !isMuted[i]) {
        chWriteMask(i,0x7,chan[i].resVol&0xf,0xf);
      } else {
        chWriteMask(i,0x7,0,0xf);
      }
      chan[i].volumeChanged=false;
    }
    if (chan[i].waveChanged) {
      chWrite(i,0x6,chan[i].curWavePos);
      if (chan[i].active) {
        chan[i].freqChanged=true;
      }
      chan[i].waveChanged=false;
    }
    if (chan[i].active) {
      if (chan[i].ws.tick()) {
        chan[i].waveUpdated=true;
      }
    }
    if (chan[i].waveUpdated) {
      updateWaveCh(i);
      if (chan[i].active) {
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
      chan[i].waveUpdated=false;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      // TODO: what is this mess?
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (lenCompensate) {
        chan[i].freq=(((chan[i].freq*chan[i].curWaveLen)*(chanMax+1))/256);
      } else {
        chan[i].freq*=(chanMax+1);
        chan[i].freq>>=3;
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0x3ffff) chan[i].freq=0x3ffff;
      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff && !isMuted[i]) {
        chWriteMask(i,0x7,0,0xf);
      }
      chWrite(i,0x0,chan[i].freq&0xff);
      chWrite(i,0x2,chan[i].freq>>8);
      chWrite(i,0x4,((256-chan[i].curWaveLen)&0xfc)|((chan[i].freq>>16)&3));
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  // update memory composition positions
  for (int i=0; i<=chanMax; i++) {
    memCompo.entries[i].begin=chan[i].wavePos>>1;
    memCompo.entries[i].end=(chan[i].wavePos+chan[i].waveLen)>>1;
    memCompo.entries[i+8].begin=chan[i].curWavePos>>1;
    memCompo.entries[i+8].end=(chan[i].curWavePos+chan[i].curWaveLen)>>1;
  }

  // update register pool
  for (int i=0; i<128; i++) {
    regPool[i]=n163.reg(i);
  }
}

int DivPlatformN163::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_N163);
      if (chan[c.chan].insChanged) {
        if (ins->n163.wave>=0) {
          chan[c.chan].wave=ins->n163.wave;
        }
        chan[c.chan].wavePos=ins->n163.perChanPos?ins->n163.wavePosCh[c.chan&7]:ins->n163.wavePos;
        chan[c.chan].waveLen=ins->n163.perChanPos?ins->n163.waveLenCh[c.chan&7]:ins->n163.waveLen;
        chan[c.chan].waveMode=ins->n163.waveMode;
        chan[c.chan].curWavePos=chan[c.chan].wavePos;
        chan[c.chan].curWaveLen=chan[c.chan].waveLen;
        chan[c.chan].ws.init(NULL,chan[c.chan].waveLen,15,true);
        if (chan[c.chan].wave<0) {
          chan[c.chan].wave=0;
        }
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        chan[c.chan].waveChanged=true;
        if (chan[c.chan].waveMode) {
          chan[c.chan].waveUpdated=true;
        }
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].resVol=chan[c.chan].vol;
      if (!isMuted[c.chan]) {
        chan[c.chan].volumeChanged=true;
      }
      chan[c.chan].macroInit(ins);
      chan[c.chan].ws.init(ins,chan[c.chan].waveLen,15,chan[c.chan].insChanged);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          chan[c.chan].resVol=chan[c.chan].outVol;
        } else {
          chan[c.chan].resVol=chan[c.chan].vol;
        }
        if (!isMuted[c.chan]) {
          chan[c.chan].volumeChanged=true;
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
      double destFreqD=NOTE_FREQUENCY(c.value2);
      if (destFreqD>2000000000.0) destFreqD=2000000000.0;
      int destFreq=destFreqD;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value*((parent->song.linearPitch==2)?1:16);
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value*((parent->song.linearPitch==2)?1:16);
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
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      if (chan[c.chan].waveMode) {
        chan[c.chan].waveUpdated=true;
      }
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_N163_WAVE_POSITION:
      chan[c.chan].curWavePos=c.value;
      chan[c.chan].waveChanged=true;
      break;
    case DIV_CMD_N163_WAVE_LENGTH:
      chan[c.chan].curWaveLen=c.value&0xfc;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_N163_WAVE_LOADPOS:
      chan[c.chan].wavePos=c.value;
      if (chan[c.chan].waveMode) {
        chan[c.chan].waveUpdated=true;
      }
      break;
    case DIV_CMD_N163_WAVE_LOADLEN:
      chan[c.chan].waveLen=c.value&0xfc;
      if (chan[c.chan].waveMode) {
        chan[c.chan].waveUpdated=true;
      }
      break;
    case DIV_CMD_N163_GLOBAL_WAVE_LOAD:
      loadWave=c.value;
      if (loadWave>=0 && loadWave<parent->song.waveLen) {
        updateWave(-1,loadWave,loadPos,-1);
      }
      break;
    case DIV_CMD_N163_GLOBAL_WAVE_LOADPOS:
      loadPos=c.value;
      break;
    case DIV_CMD_N163_CHANNEL_LIMIT:
      if (chanMax!=(c.value&0x7)) {
        chanMax=c.value&0x7;
        rWriteMask(0x7f,chanMax<<4,0x70);
        forceIns();
      }
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_N163));
          chan[c.chan].keyOn=true;
        }
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
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

void DivPlatformN163::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chan[ch].volumeChanged=true;
}

void DivPlatformN163::forceIns() {
  for (int i=0; i<=chanMax; i++) {
    chan[i].insChanged=true;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
      chan[i].volumeChanged=true;
      chan[i].waveChanged=true;
    }
  }
  memCompo.entries[16].begin=120-chanMax*8;
}

void DivPlatformN163::notifyWaveChange(int wave) {
  for (int i=0; i<8; i++) {
    if (chan[i].wave==wave) {
      if (chan[i].waveMode) {
        chan[i].ws.changeWave1(wave);
        chan[i].waveUpdated=true;
      }
    }
  }
}

void DivPlatformN163::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformN163::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformN163::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformN163::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformN163::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformN163::getRegisterPool() {
  return regPool;
}

int DivPlatformN163::getRegisterPoolSize() {
  return 128;
}

void DivPlatformN163::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformN163::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,15,false);
  }

  n163.reset();
  memset(regPool,0,128);

  n163.set_disable(false);
  n163.set_multiplex(multiplex);
  chanMax=initChanMax;
  loadWave=-1;
  loadPos=0;
  rWrite(0x7f,initChanMax<<4);

  memCompo.entries[16].begin=120-chanMax*8;
}

void DivPlatformN163::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformN163::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

const DivMemoryComposition* DivPlatformN163::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformN163::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 1: // PAL
      chipClock=COLOR_PAL*3.0/8.0;
      break;
    case 2: // Dendy
      chipClock=COLOR_PAL*2.0/5.0;
      break;
    default: // NTSC
      chipClock=COLOR_NTSC/2.0;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  initChanMax=chanMax=flags.getInt("channels",0)&7;
  multiplex=!flags.getBool("multiplex",false); // not accurate in real hardware
  rate=chipClock;
  rate/=15;
  n163.set_multiplex(multiplex);
  rWrite(0x7f,initChanMax<<4);
  for (int i=0; i<8; i++) {
    oscBuf[i]->setRate(rate);//=rate/(initChanMax+1);
  }

  lenCompensate=flags.getBool("lenCompensate",false);

  // needed to make sure changing channel count won't trigger glitches
  reset();
}

int DivPlatformN163::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  memCompo.used=0;
  memCompo.capacity=128;
  memCompo.memory=regPool;
  memCompo.waveformView=DIV_MEMORY_WAVE_4BIT;

  for (int i=0; i<8; i++) {
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_N163_LOAD,fmt::sprintf("Channel %d (load)",i),-1,0,0));
  }
  for (int i=0; i<8; i++) {
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_N163_PLAY,fmt::sprintf("Channel %d (play)",i),-1,0,0));
  }
  memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_RESERVED,"Registers",-1,127,128));

  setFlags(flags);

  reset();

  return 8;
}

void DivPlatformN163::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
}

DivPlatformN163::~DivPlatformN163() {
}
