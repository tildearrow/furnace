#include "pce.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v);}
#define chWrite(c,a,v) \
  if (!skipRegisterWrites) { \
    if (curChan!=c) { \
      curChan=c; \
      rWrite(0,curChan); \
    } \
    rWrite(a,v); \
  }

#define FREQ_BASE 1712.0f*2

void DivPlatformPCE::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    // PCM part
    for (int i=0; i<6; i++) {
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        chan[i].dacPeriod-=6;
        if (chan[i].dacPeriod<1) {
          DivSample* s=parent->song.sample[chan[i].dacSample];
          chWrite(i,0x07,0);
          if (s->depth==8) {
            chWrite(i,0x04,0xdf);
            chWrite(i,0x06,(((unsigned char)s->rendData[chan[i].dacPos++]+0x80)>>3));
          } else {
            chWrite(i,0x04,0xdf);
            chWrite(i,0x06,(((unsigned short)s->rendData[chan[i].dacPos++]+0x8000)>>11));
          }
          if (chan[i].dacPos>=s->rendLength) {
            chan[i].dacSample=-1;
          }
          chan[i].dacPeriod+=chan[i].dacRate;
        }
      }
    }
  
    // PCE part
    cycles=0;
    while (!writes.empty() && cycles<24) {
      QueuedWrite w=writes.front();
      pce->Write(cycles,w.addr,w.val);
      //cycles+=2;
      writes.pop();
    }
    memset(tempL,0,24*sizeof(int));
    memset(tempR,0,24*sizeof(int));
    pce->Update(24);
    pce->ResetTS(0);

    if (tempL[0]<-32768) tempL[0]=-32768;
    if (tempL[0]>32767) tempL[0]=32767;
    if (tempR[0]<-32768) tempR[0]=-32768;
    if (tempR[0]>32767) tempR[0]=32767;
    
    //printf("tempL: %d tempR: %d\n",tempL,tempR);
    bufL[h]=tempL[0];
    bufR[h]=tempR[0];
  }
}

void DivPlatformPCE::updateWave(int ch) {
  DivWavetable* wt=parent->getWave(chan[ch].wave);
  chWrite(ch,0x04,0x5f);
  chWrite(ch,0x04,0x1f);
  for (int i=0; i<32; i++) {
    if (wt->max<1 || wt->len<1) {
      chWrite(ch,0x06,0);
    } else {
      chWrite(ch,0x06,wt->data[i*wt->len/32]*31/wt->max);
    }
  }
  if (chan[ch].active) {
    chWrite(ch,0x04,0x80|chan[ch].outVol);
  }
}

// TODO: in octave 6 the noise table changes to a tonal one
static unsigned char noiseFreq[12]={
  4,13,15,18,21,23,25,27,29,31,0,2  
};

void DivPlatformPCE::tick() {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=(chan[i].vol*chan[i].std.vol)>>5;
      chWrite(i,0x04,0x80|chan[i].outVol);
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].std.arp)/12.0f)));
          // noise
          chWrite(i,0x07,chan[i].noise?(0x80|noiseFreq[(chan[i].std.arp)%12]):0);
        } else {
          chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].note+chan[i].std.arp-12)/12.0f)));
          chWrite(i,0x07,chan[i].noise?(0x80|noiseFreq[(chan[i].note+chan[i].std.arp-12)%12]):0);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].note)/12.0f)));
        chWrite(i,0x07,chan[i].noise?(0x80|noiseFreq[chan[i].note%12]):0);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadWave) {
      if (chan[i].wave!=chan[i].std.wave) {
        chan[i].wave=chan[i].std.wave;
        updateWave(i);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].note>0x5d) chan[i].freq=0x01;
      chWrite(i,0x02,chan[i].freq&0xff);
      chWrite(i,0x03,chan[i].freq>>8);
      if (chan[i].keyOn) {
        if (chan[i].wave<0) {
          chan[i].wave=0;
          updateWave(i);
        }
        //rWrite(16+i*5,0x80);
        //chWrite(i,0x04,0x80|chan[i].vol);
      }
      if (chan[i].keyOff) {
        chWrite(i,0x04,0);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformPCE::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (chan[c.chan].pcm) {
        chan[c.chan].dacSample=12*sampleBank+c.value%12;
        if (chan[c.chan].dacSample>=parent->song.sampleLen) {
          chan[c.chan].dacSample=-1;
          break;
        }
        chan[c.chan].dacPos=0;
        chan[c.chan].dacPeriod=0;
        chan[c.chan].dacRate=1789773/parent->song.sample[chan[c.chan].dacSample]->rate;
        break;
      }
      chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chWrite(c.chan,0x07,chan[c.chan].noise?(0x80|noiseFreq[chan[c.chan].note%12]):0);
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chWrite(c.chan,0x04,0x80|chan[c.chan].vol);
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dacSample=-1;
      chan[c.chan].pcm=false;
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
          if (chan[c.chan].active) chWrite(c.chan,0x04,0x80|chan[c.chan].outVol);
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.hasVol) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      updateWave(c.chan);
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_PCE_LFO_MODE:
      rWrite(0x09,c.value);
      break;
    case DIV_CMD_PCE_LFO_SPEED:
      rWrite(0x08,c.value);
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
      chan[c.chan].noise=c.value;
      chWrite(c.chan,0x07,chan[c.chan].noise?(0x80|chan[c.chan].note):0);
      break;
    case DIV_CMD_SAMPLE_MODE:
      chan[c.chan].pcm=c.value;
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=c.value;
      chWrite(c.chan,0x05,isMuted[c.chan]?0:chan[c.chan].pan);
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp-12):(0)))/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 31;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformPCE::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chWrite(ch,0x05,isMuted[ch]?0:chan[ch].pan);
}

void DivPlatformPCE::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
    chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
}

void DivPlatformPCE::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformPCE::Channel();
  }
  pce->Power(0);
  lastPan=0xff;
  memset(tempL,0,32*sizeof(int));
  memset(tempR,0,32*sizeof(int));
  cycles=0;
  curChan=-1;
  sampleBank=0;
  // set global volume
  rWrite(0,0);
  rWrite(0x01,0xff);
  // set per-channel initial panning
  for (int i=0; i<6; i++) {
    chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
  delay=500;
}

bool DivPlatformPCE::isStereo() {
  return true;
}

bool DivPlatformPCE::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformPCE::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPCE::setPAL(bool pal) {
  if (pal) { // technically there is no PAL PC Engine but oh well...
    rate=1773448/6;
  } else {
    rate=1789773/6;
  }
}

int DivPlatformPCE::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
  }
  setPAL(pal);
  pce=new PCE_PSG(tempL,tempR,PCE_PSG::REVISION_HUC6280);
  reset();
  return 6;
}

void DivPlatformPCE::quit() {
  delete pce;
}

DivPlatformPCE::~DivPlatformPCE() {
}