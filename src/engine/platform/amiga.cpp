#include "amiga.h"
#include "../engine.h"
#include <math.h>

#define FREQ_BASE 6848.0f
#define AMIGA_DIVIDER 8

void DivPlatformAmiga::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    bufL[h]=0;
    bufR[h]=0;
    for (int i=0; i<4; i++) {
      if (chan[i].sample!=-1) {
        chan[i].audSub-=AMIGA_DIVIDER;
        if (chan[i].audSub<0) {
          DivSample* s=parent->song.sample[chan[i].sample];
          if (s->depth==8) {
            chan[i].audDat=s->rendData[chan[i].audPos++];
          } else {
            chan[i].audDat=s->rendData[chan[i].audPos++]>>8;
          }
          if (chan[i].audPos>=s->rendLength) {
            if (s->loopStart>=0 && s->loopStart<=(int)s->rendLength) {
              chan[i].audPos=s->loopStart;
            } else {
              chan[i].sample=-1;
            }
          }
          /*if (chan[i].freq<124) {
            // ???
          }*/
          chan[i].audSub+=MAX(114,chan[i].freq);
        }
      }
      if (!isMuted[i]) {
        if (i==0 || i==3) {
          bufL[h]+=(chan[i].audDat*chan[i].outVol);
          bufR[h]+=(chan[i].audDat*chan[i].outVol)>>2;
        } else {
          bufL[h]+=(chan[i].audDat*chan[i].outVol)>>2;
          bufR[h]+=(chan[i].audDat*chan[i].outVol);
        }
      }
    }
  }
}

void DivPlatformAmiga::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=(chan[i].vol*chan[i].std.vol)>>6;
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].std.arp)/12.0f)));
        } else {
          chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].note+chan[i].std.arp-12)/12.0f)));
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=round(FREQ_BASE/pow(2.0f,((float)(chan[i].note)/12.0f)));
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadWave) {
      if (chan[i].wave!=chan[i].std.wave) {
        chan[i].wave=chan[i].std.wave;
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].note>0x5d) chan[i].freq=0x01;
      if (chan[i].keyOn) {
        if (chan[i].wave<0) {
          chan[i].wave=0;
        }
      }
      if (chan[i].keyOff) {
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformAmiga::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      chan[c.chan].baseFreq=round(FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].sample=ins->amiga.initSample;
      if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
        chan[c.chan].sample=-1;
      }
      chan[c.chan].audPos=0;
      chan[c.chan].audSub=0;
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
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
      return 64;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformAmiga::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformAmiga::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

void DivPlatformAmiga::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformAmiga::Channel();
  }
}

bool DivPlatformAmiga::isStereo() {
  return true;
}

bool DivPlatformAmiga::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformAmiga::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAmiga::setPAL(bool pal) {
  if (pal) {
    rate=3546895/AMIGA_DIVIDER;
  } else {
    rate=3579545/AMIGA_DIVIDER;
  }
}

int DivPlatformAmiga::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
  }
  setPAL(pal);
  reset();
  return 6;
}

void DivPlatformAmiga::quit() {
}