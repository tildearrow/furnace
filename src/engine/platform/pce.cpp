#include "pce.h"
#include "../engine.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) \
  if (!skipRegisterWrites) { \
    if (curChan!=c) { \
      curChan=c; \
      rWrite(0,curChan); \
    } \
    rWrite(a,v); \
  }

#define CHIP_DIVIDER 32

const char* regCheatSheetPCE[]={
  "Select", "0",
  "MasterVol", "1",
  "FreqL", "2",
  "FreqH", "3",
  "DataCtl", "4",
  "ChanVol", "5",
  "WaveCtl", "6",
  "NoiseCtl", "7",
  "LFOFreq", "8",
  "LFOCtl", "9",
  NULL
};

const char** DivPlatformPCE::getRegisterSheet() {
  return regCheatSheetPCE;
}

void DivPlatformPCE::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    // PCM part
    for (int i=0; i<6; i++) {
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        chan[i].dacPeriod+=chan[i].dacRate;
        if (chan[i].dacPeriod>rate) {
          DivSample* s=parent->song.sample[chan[i].dacSample];
          if (s->rendLength<=0) {
            chan[i].dacSample=-1;
            continue;
          }
          chWrite(i,0x07,0);
          if (s->depth==8) {
            chWrite(i,0x04,0xdf);
            chWrite(i,0x06,(((unsigned char)s->rendData[chan[i].dacPos++]+0x80)>>3));
          } else {
            chWrite(i,0x04,0xdf);
            chWrite(i,0x06,(((unsigned short)s->rendData[chan[i].dacPos++]+0x8000)>>11));
          }
          if (chan[i].dacPos>=s->rendLength) {
            if (s->loopStart>=0 && s->loopStart<=(int)s->rendLength) {
              chan[i].dacPos=s->loopStart;
            } else {
              chan[i].dacSample=-1;
            }
          }
          chan[i].dacPeriod-=rate;
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

    tempL[0]=(tempL[0]>>1)+(tempL[0]>>2);
    tempR[0]=(tempR[0]>>1)+(tempR[0]>>2);

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
      chan[i].outVol=((chan[i].vol&31)*MIN(31,chan[i].std.vol))>>5;
      if (chan[i].furnaceDac) {
        // ignore for now
      } else {
        chWrite(i,0x04,0x80|chan[i].outVol);
      }
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp);
          // noise
          int noiseSeek=chan[i].std.arp;
          if (noiseSeek<0) noiseSeek=0;
          chWrite(i,0x07,chan[i].noise?(0x80|(parent->song.properNoiseLayout?(noiseSeek&31):noiseFreq[noiseSeek%12])):0);
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp);
          int noiseSeek=chan[i].note+chan[i].std.arp;
          if (noiseSeek<0) noiseSeek=0;
          chWrite(i,0x07,chan[i].noise?(0x80|(parent->song.properNoiseLayout?(noiseSeek&31):noiseFreq[noiseSeek%12])):0);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        int noiseSeek=chan[i].note;
        if (noiseSeek<0) noiseSeek=0;
        chWrite(i,0x07,chan[i].noise?(0x80|(parent->song.properNoiseLayout?(noiseSeek&31):noiseFreq[noiseSeek%12])):0);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadWave && !chan[i].pcm) {
      if (chan[i].wave!=chan[i].std.wave) {
        chan[i].wave=chan[i].std.wave;
        updateWave(i);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].furnaceDac) {
        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->song.sample[chan[i].dacSample];
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=8363.0/(double)s->centerRate;
          }
        }
        chan[i].dacRate=((double)chipClock/2)/(off*chan[i].freq);
        if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dacRate);
      }
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
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (ins->type==DIV_INS_AMIGA) {
        chan[c.chan].pcm=true;
      } else if (chan[c.chan].furnaceDac) {
        chan[c.chan].pcm=false;
      }
      if (chan[c.chan].pcm) {
        if (ins->type==DIV_INS_AMIGA) {
          chan[c.chan].dacSample=ins->amiga.initSample;
          if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
             if (dumpWrites) {
               chWrite(c.chan,0x04,0xdf);
               addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
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
          chan[c.chan].std.init(ins);
          //chan[c.chan].keyOn=true;
          chan[c.chan].furnaceDac=true;
        } else {
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
          chan[c.chan].dacRate=parent->song.sample[chan[c.chan].dacSample]->rate;
          if (dumpWrites) {
            chWrite(c.chan,0x04,0xdf);
            addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dacRate);
          }
          chan[c.chan].furnaceDac=false;
        }
        break;
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        int noiseSeek=chan[c.chan].note;
        if (noiseSeek<0) noiseSeek=0;
        chWrite(c.chan,0x07,chan[c.chan].noise?(0x80|(parent->song.properNoiseLayout?(noiseSeek&31):noiseFreq[noiseSeek%12])):0);
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chWrite(c.chan,0x04,0x80|chan[c.chan].vol);
      chan[c.chan].std.init(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dacSample=-1;
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].pcm=false;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
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
      if (c.value==0) {
        lfoMode=0;
      } else {
        lfoMode=c.value;
      }
      rWrite(0x08,lfoSpeed);
      rWrite(0x09,lfoMode);
      break;
    case DIV_CMD_PCE_LFO_SPEED:
      lfoSpeed=255-c.value;
      rWrite(0x08,lfoSpeed);
      rWrite(0x09,lfoMode);
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
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp):(0)));
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

void* DivPlatformPCE::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformPCE::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformPCE::Channel();
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  pce->Power(0);
  lastPan=0xff;
  memset(tempL,0,32*sizeof(int));
  memset(tempR,0,32*sizeof(int));
  cycles=0;
  curChan=-1;
  sampleBank=0;
  lfoMode=0;
  lfoSpeed=255;
  // set global volume
  rWrite(0,0);
  rWrite(0x01,0xff);
  // set LFO
  rWrite(0x08,lfoSpeed);
  rWrite(0x09,lfoMode);
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

void DivPlatformPCE::notifyWaveChange(int wave) {
  for (int i=0; i<6; i++) {
    if (chan[i].wave==wave) {
      updateWave(i);
    }
  }
}

void DivPlatformPCE::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPCE::setFlags(unsigned int flags) {
  if (flags&1) { // technically there is no PAL PC Engine but oh well...
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  rate=chipClock/12;
}

void DivPlatformPCE::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformPCE::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

int DivPlatformPCE::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);
  pce=new PCE_PSG(tempL,tempR,PCE_PSG::REVISION_HUC6280);
  reset();
  return 6;
}

void DivPlatformPCE::quit() {
  delete pce;
}

DivPlatformPCE::~DivPlatformPCE() {
}
