#include "c64.h"
#include "../engine.h"
#include <math.h>

#define FREQ_BASE 277.0f

void DivPlatformC64::acquire(int& l, int& r) {
  sid.clock();
  l=sid.output();
  r=l;
}

void DivPlatformC64::tick() {
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      // ok, why are the volumes like that?
      chan[i].outVol=chan[i].std.vol-(15-chan[i].vol);
      if (chan[i].outVol<0) chan[i].outVol=0;
    }
    if (chan[i].std.hadArp) {
      if (i==3) { // noise
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=chan[i].std.arp;
        } else {
          chan[i].baseFreq=chan[i].note+chan[i].std.arp-12;
        }
        if (chan[i].baseFreq>255) chan[i].baseFreq=255;
        if (chan[i].baseFreq<0) chan[i].baseFreq=0;
      } else {
        if (!chan[i].inPorta) {
          if (chan[i].std.arpMode) {
            chan[i].baseFreq=round(FREQ_BASE*pow(2.0f,((float)(chan[i].std.arp)/12.0f)));
          } else {
            chan[i].baseFreq=round(FREQ_BASE*pow(2.0f,((float)(chan[i].note+chan[i].std.arp-12)/12.0f)));
          }
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=round(FREQ_BASE*pow(2.0f,((float)(chan[i].note)/12.0f)));
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadDuty) {
      chan[i].duty=chan[i].std.duty;
      if (i==3 && chan[i].duty>1) chan[i].duty=1;
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (i!=2) {
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
    if (chan[i].testWhen>0) {
      if (--chan[i].testWhen<1) {
        sid.write(i*7+4,8);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE-chan[i].pitch))/ONE_SEMITONE;
      if (chan[i].keyOn) {
        sid.write(i*7+5,(ins->c64.a<<4)|(ins->c64.d));
        sid.write(i*7+6,(ins->c64.s<<4)|(ins->c64.r));
        sid.write(i*7+4,
                  (ins->c64.noiseOn<<7)|
                  (ins->c64.pulseOn<<6)|
                  (ins->c64.sawOn<<5)|
                  (ins->c64.triOn<<4)|
                  1
                  );
      }
      if (chan[i].keyOff) {
        sid.write(i*7+5,(ins->c64.a<<4)|(ins->c64.d));
        sid.write(i*7+6,(ins->c64.s<<4)|(ins->c64.r));
        sid.write(i*7+4,
                  (ins->c64.noiseOn<<7)|
                  (ins->c64.pulseOn<<6)|
                  (ins->c64.sawOn<<5)|
                  (ins->c64.triOn<<4)|
                  0
                  );
      }
      sid.write(i*7,chan[i].freq&0xff);
      sid.write(i*7+1,chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformC64::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].baseFreq=round(FREQ_BASE*pow(2.0f,((float)c.value/12.0f)));
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
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
        }
        if (c.chan==2) {
        } else {
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
      int destFreq=round(FREQ_BASE*pow(2.0f,((float)c.value2/12.0f)));
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
    case DIV_CMD_PRE_NOTE:
      chan[c.chan].testWhen=c.value;
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

void DivPlatformC64::setChipModel(bool is6581) {
  if (is6581) {
    sid.set_chip_model(MOS6581);
  } else {
    sid.set_chip_model(MOS8580);
  }
}

int DivPlatformC64::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=985248;

  sid.reset();

  sid.write(0x18,0x0f);

  return 3;
}
