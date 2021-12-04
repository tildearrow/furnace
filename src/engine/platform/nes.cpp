#include "nes.h"
#include "sound/nes/cpu_inline.h"
#include "../engine.h"
#include <math.h>

#define FREQ_BASE 3424.0f

void DivPlatformNES::acquire(int& l, int& r) {
  if (dacSample!=-1) {
    dacPeriod+=dacRate;
    if (dacPeriod>=rate) {
      DivSample* s=parent->song.sample[dacSample];
      if (s->depth==8) {
        apu_wr_reg(0x4011,((unsigned char)s->rendData[dacPos++]+0x80)>>1);
      } else {
        apu_wr_reg(0x4011,((unsigned short)s->rendData[dacPos++]+0x8000)>>9);
      }
      if (dacPos>=s->rendLength) {
        dacSample=-1;
      }
      dacPeriod-=rate;
    }
  }

  apu_tick(NULL);
  apu.odd_cycle=!apu.odd_cycle;
  if (apu.clocked) {
    apu.clocked=false;
    l=(pulse_output()+tnd_output())*30;
    r=l;
    //printf("output value: %d\n",l);
  }
}

static int dacRates[6]={
  4000, 8000, 11025, 16000, 22050, 32000
};

static unsigned char noiseTable[256]={
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

void DivPlatformNES::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      // ok, why are the volumes like that?
      chan[i].outVol=chan[i].std.vol-(15-chan[i].vol);
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (i==2) { // triangle
        apu_wr_reg(0x4000+i*4,(chan[i].outVol==0)?0:255);
        chan[i].freqChanged=true;
      } else {
        apu_wr_reg(0x4000+i*4,0x30|chan[i].outVol|((chan[i].duty&3)<<6));
      }
    }
    if (chan[i].std.hadArp) {
      if (i==3) { // noise
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=chan[i].std.arp+24;
        } else {
          chan[i].baseFreq=chan[i].note+chan[i].std.arp-12;
        }
        if (chan[i].baseFreq>255) chan[i].baseFreq=255;
        if (chan[i].baseFreq<0) chan[i].baseFreq=0;
      } else {
        if (!chan[i].inPorta) {
          if (chan[i].std.arpMode) {
            chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].std.arp)/12.0f)));
          } else {
            chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].note+chan[i].std.arp-12)/12.0f)));
          }
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].note)/12.0f)));
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadDuty) {
      chan[i].duty=chan[i].std.duty;
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (i!=2) {
        apu_wr_reg(0x4000+i*4,0x30|chan[i].outVol|((chan[i].duty&3)<<6));
      }
      if (i==3) { // noise
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].sweepChanged) {
      chan[i].sweepChanged=false;
      if (i==0) {
        //rWrite(16+i*5,chan[i].sweep);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (i==3) { // noise
        chan[i].freq=noiseTable[chan[i].baseFreq];
      } else {
        chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE-chan[i].pitch))/ONE_SEMITONE;
        if (chan[i].freq>2047) chan[i].freq=2047;
      }
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
      }
      if (chan[i].keyOff) {
        //rWrite(16+i*5+2,8);
        if (i==2) { // triangle
          apu_wr_reg(0x4000+i*4,0x00);
        } else {
          apu_wr_reg(0x4000+i*4,0x30);
        }
      }
      if (i==3) { // noise
        apu_wr_reg(0x4002+i*4,(chan[i].duty<<7)|chan[i].freq);
        apu_wr_reg(0x4003+i*4,0xf0);
      } else {
        apu_wr_reg(0x4002+i*4,chan[i].freq&0xff);
        if ((chan[i].prevFreq>>8)!=(chan[i].freq>>8) || i==2) {
          apu_wr_reg(0x4003+i*4,0xf8|(chan[i].freq>>8));
        }
        chan[i].prevFreq=chan[i].freq;
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformNES::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.chan==4) { // PCM
        dacSample=c.value%12;
        if (dacSample>=parent->song.sampleLen) {
          dacSample=-1;
          break;
        }
        dacPos=0;
        dacPeriod=0;
        dacRate=dacRates[parent->song.sample[dacSample]->rate];
        break;
      } else if (c.chan==3) { // noise
        chan[c.chan].baseFreq=c.value;
      } else {
        chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      }
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      if (c.chan==2) {
        apu_wr_reg(0x4000+c.chan*4,0xff);
      } else {
        apu_wr_reg(0x4000+c.chan*4,0x30|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
        }
        if (c.chan==2) {
          apu_wr_reg(0x4000+c.chan*4,0xff);
        } else {
          apu_wr_reg(0x4000+c.chan*4,0x30|chan[c.chan].vol|((chan[c.chan].duty&3)<<6));
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
      int destFreq=round(FREQ_BASE/pow(2.0f,((float)c.value2/12.0f)));
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
      }
      break;
    case DIV_CMD_PANNING: {
      lastPan&=~(0x11<<c.chan);
      if (c.value==0) c.value=0x11;
      lastPan|=c.value<<c.chan;
      break;
    }
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

int DivPlatformNES::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=1789773;

  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;

  init_nla_table(500,500);

  apu_turn_on();
  apu.addrSpace=new unsigned char[65536];
  apu.cpu_cycles=0;
  apu.cpu_opcode_cycle=0;

  apu_wr_reg(0x4015,0x1f);
  apu_wr_reg(0x4001,0x08);
  apu_wr_reg(0x4005,0x08);
  /*apu_wr_reg(0x4000,0x3f);
  apu_wr_reg(0x4001,0x00);
  apu_wr_reg(0x4002,0xff);
  apu_wr_reg(0x4003,0x10);
  apu_wr_reg(0x4004,0x3f);
  apu_wr_reg(0x4005,0x00);
  apu_wr_reg(0x4006,0xfe);
  apu_wr_reg(0x4007,0x10);*/

  lastPan=0xff;
  return 5;
}
