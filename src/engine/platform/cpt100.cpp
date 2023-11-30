
#include "cpt100.h"
#include "../engine.h"
#include <stdio.h>
#include <math.h>
#include "sound/cpt100/sound.cpp"


#define CHIP_FREQBASE 1000
#define rWrite(a,v) if (!skipRegisterWrites) {doWrite(a,v); regPool[(a-0x10000)%208]=v; if (dumpWrites) {addWrite(a,v);} }

unsigned int chanaddrs_freq[6] = {0x10000,0x10010,0x10020,0x10030,0x10084,0x10086};
unsigned int chanaddrs_volume[6] = {0x10009,0x10019,0x10029,0x10039,0x10088,0x10089};

void DivPlatformCPT100::doWrite(unsigned int addr, unsigned char data) {
  ram_poke(ram,(int)addr,(Byte)data);
}

void DivPlatformCPT100::acquire(short** buf, size_t len) {
  int chanOut;
  std::vector<std::vector<int16_t>> output = AudioCallBack(len);
  for (size_t i=0; i<len; i++) {
    int out=0;
    for (unsigned char j=0; j<chans; j++) {
      if (chan[j].active) {
        if (!isMuted[j]) {
          chanOut=(signed short)(output[j][i])>>12;
          oscBuf[j]->data[oscBuf[j]->needle++]=chanOut<<1;
          out+=chanOut;
        } else {
          oscBuf[j]->data[oscBuf[j]->needle++]=0;
        }
        chan[j].pos+=chan[j].freq;
      } else {
        oscBuf[j]->data[oscBuf[j]->needle++]=0;
      }
    }
    if (out<-32768) out=-32768;
    if (out>32767) out=32767;
    buf[0][i]=out;
  }
}

void DivPlatformCPT100::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformCPT100::tick(bool sysTick) {
  for (unsigned char i=0; i<chans; i++) {
    if (sysTick) {
      chan[i].amp-=3;
      if (chan[i].amp<15) chan[i].amp=15;
      rWrite(chanaddrs_volume[i],chan[i].amp);
    }

    if (chan[i].freqChanged) {
      chan[i].freqChanged=false;
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,0,false,false,0,0,chipClock,CHIP_FREQBASE);
      rWrite(chanaddrs_freq[i],chan[i].freq>>8);
      rWrite(chanaddrs_freq[i]+1,chan[i].freq&0xff);
    }

    if (i>=4 && i<=5)
    {
      for (int j=0;j<32;j++) {
      rWrite(0x10090+32*(i-4)+j,chan[i].ws.output[j]);
    }
    }
 
  }
}

void* DivPlatformCPT100::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformCPT100::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformCPT100::getRegisterPool() {
  return regPool;
}

int DivPlatformCPT100::getRegisterPoolSize() {
  return 192;
}

int DivPlatformCPT100::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      {
      DivInstrument* ins = parent->getIns(chan[c.chan].ins,DIV_INS_CPT100);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].macroInit(ins);
        if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
        if (chan[c.chan].wave<0) {
          chan[c.chan].wave=0;
          chan[c.chan].ws.changeWave1(chan[c.chan].wave);
        }
        chan[c.chan].ws.init(ins,32,255,chan[c.chan].insChanged);
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].active=true;
      chan[c.chan].amp=255;
      break;
      }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      //chan[c.chan].vol=c.value;
      chan[c.chan].vol=255;
      if (chan[c.chan].vol>255) chan[c.chan].vol=255;
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
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
      if (return2) return 2;
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 255;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformCPT100::notifyInsDeletion(void* ins) {
  // nothing
}

void DivPlatformCPT100::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformCPT100::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformCPT100::reset() {
  ram_boot(ram,vram);
  initSound();
  for (int i=0; i<chans; i++) {
    chan[i]=DivPlatformCPT100::Channel();
    chan[i].vol=0xff;
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,255,false);
  }
}

int DivPlatformCPT100::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<channels; i++) {
    isMuted[i]=false;
    if (i<channels) {
      oscBuf[i]=new DivDispatchOscBuffer;
      oscBuf[i]->rate=48000;
      
    }
  }
  rate=48000;
  chipClock=48000;
  chans=channels;
  memset(regPool,0,192);
  reset();
  return channels;
}

void DivPlatformCPT100::quit() {
  for (int i=0; i<chans; i++) {
    delete oscBuf[i];
  }
}

DivPlatformCPT100::~DivPlatformCPT100() {
}
