#include "gb.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) GB_apu_write(gb,a,v);

#define FREQ_BASE 8015.85f

void DivPlatformGB::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    GB_advance_cycles(gb,16);
    bufL[i]=gb->apu_output.final_sample.left<<2;
    bufR[i]=gb->apu_output.final_sample.right<<2;
  }
}

void DivPlatformGB::updateWave() {
  DivWavetable* wt=parent->getWave(chan[2].wave);
  rWrite(0x1a,0);
  for (int i=0; i<16; i++) {
    unsigned char next=((wt->data[i*2]&15)<<4)|(wt->data[1+i*2]&15);
    rWrite(0x30+i,next);
  }
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

void DivPlatformGB::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
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
            chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].std.arp+24)/12.0f)));
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
        rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
      }
    }
    if (chan[i].std.hadWave) {
      if (chan[i].wave!=chan[i].std.wave) {
        chan[i].wave=chan[i].std.wave;
        if (i==2) {
          updateWave();
          if (!chan[i].keyOff) chan[i].keyOn=true;
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
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (i==3) { // noise
        chan[i].freq=noiseTable[chan[i].baseFreq];
      } else {
        chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE-chan[i].pitch))/ONE_SEMITONE;
        if (chan[i].freq>2047) chan[i].freq=2047;
      }
      if (chan[i].note>0x5d) chan[i].freq=0x01;
      if (chan[i].keyOn) {
        if (i==2) { // wave
          if (chan[i].wave<0) {
            chan[i].wave=0;
            updateWave();
          }
          rWrite(16+i*5,0x80);
          rWrite(16+i*5+2,gbVolMap[chan[i].vol]);
        } else {
          rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
          rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
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
        rWrite(16+i*5+4,((chan[i].keyOn||chan[i].keyOff)?0x80:0x00)|((ins->gb.soundLen<64)<<6));
      } else {
        rWrite(16+i*5+3,(2048-chan[i].freq)&0xff);
        rWrite(16+i*5+4,(((2048-chan[i].freq)>>8)&7)|((chan[i].keyOn||chan[i].keyOff)?0x80:0x00)|((ins->gb.soundLen<63)<<6));
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformGB::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.chan==3) { // noise
        chan[c.chan].baseFreq=c.value;
      } else {
        chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      }
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value) {
        chan[c.chan].ins=c.value;
        if (c.chan!=2) {
          chan[c.chan].vol=parent->getIns(chan[c.chan].ins)->gb.envVol;
        }
      }
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      if (c.chan==2) {
        rWrite(16+c.chan*5+2,gbVolMap[chan[c.chan].vol]);
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
      updateWave();
      chan[c.chan].keyOn=true;
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
      if (c.chan!=2) {
        chan[c.chan].freqChanged=true;
        rWrite(16+c.chan*5+1,((chan[c.chan].duty&3)<<6)|(63-(parent->getIns(chan[c.chan].ins)->gb.soundLen&63)));
      }
      break;
    case DIV_CMD_PANNING: {
      lastPan&=~(0x11<<c.chan);
      if (c.value==0) c.value=0x11;
      lastPan|=c.value<<c.chan;
      rWrite(0x25,lastPan);
      break;
    }
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp-12):(0)))/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GB_SWEEP_DIR:
      chan[c.chan].sweep&=0xf7;
      if (c.value&1) {
        chan[c.chan].sweep|=8;
      }
      chan[c.chan].sweepChanged=true;
      break;
    case DIV_CMD_GB_SWEEP_TIME:
      chan[c.chan].sweep&=8;
      chan[c.chan].sweep|=c.value&0x77;
      chan[c.chan].sweepChanged=true;
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

void DivPlatformGB::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformGB::Channel();
  }
  memset(gb,0,sizeof(GB_gameboy_t));
  gb->model=GB_MODEL_DMG_B;
  GB_apu_init(gb);
  GB_set_sample_rate(gb,rate);
  // enable all channels
  GB_apu_write(gb,0x26,0x80);
  GB_apu_write(gb,0x25,0xff);
  lastPan=0xff;
}

bool DivPlatformGB::isStereo() {
  return true;
}

int DivPlatformGB::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  rate=262144;
  gb=new GB_gameboy_t;
  reset();
  return 4;
}

void DivPlatformGB::quit() {
  delete gb;
}

DivPlatformGB::~DivPlatformGB() {
}