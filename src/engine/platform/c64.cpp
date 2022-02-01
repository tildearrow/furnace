#include "c64.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {sid.write(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 524288

void DivPlatformC64::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t i=start; i<start+len; i++) {
    sid.clock();
    bufL[i]=sid.output();
  }
}

void DivPlatformC64::updateFilter() {
  rWrite(0x15,filtCut&7);
  rWrite(0x16,filtCut>>3);
  rWrite(0x17,(filtRes<<4)|(chan[2].filter<<2)|(chan[1].filter<<1)|(chan[0].filter));
  rWrite(0x18,(filtControl<<4)|vol);
}

void DivPlatformC64::tick() {
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (ins->c64.volIsCutoff) {
        if (ins->c64.filterIsAbs) {
          filtCut=MIN(2047,chan[i].std.vol);
        } else {
          filtCut-=((signed char)chan[i].std.vol-18)*7;
          if (filtCut>2047) filtCut=2047;
          if (filtCut<0) filtCut=0;
        }
        updateFilter();
      } else {
        vol=MIN(15,chan[i].std.vol);
        updateFilter();
      }
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].std.arp);
        } else {
          chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note+(signed char)chan[i].std.arp);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=NOTE_FREQUENCY(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadDuty) {
      DivInstrument* ins=parent->getIns(chan[i].ins);
      if (ins->c64.dutyIsAbs) {
        chan[i].duty=chan[i].std.duty;
      } else {
        chan[i].duty-=((signed char)chan[i].std.duty-12)*4;
      }
      rWrite(i*7+2,chan[i].duty&0xff);
      rWrite(i*7+3,chan[i].duty>>8);
    }
    if (chan[i].testWhen>0) {
      if (--chan[i].testWhen<1) {
        if (!chan[i].resetMask && !isMuted[i]) {
          rWrite(i*7+5,0);
          rWrite(i*7+6,0);
          rWrite(i*7+4,(isMuted[i]?0:(chan[i].wave<<4))|8|(chan[i].ring<<2)|(chan[i].sync<<1));
        }
      }
    }
    if (chan[i].std.hadWave) {
      chan[i].wave=chan[i].std.wave;
      rWrite(i*7+4,(isMuted[i]?0:(chan[i].wave<<4))|(chan[i].ring<<2)|(chan[i].sync<<1)|chan[i].active);
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch);
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;
      if (chan[i].keyOn) {
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(isMuted[i]?0:(chan[i].wave<<4))|(chan[i].ring<<2)|(chan[i].sync<<1)|1);
      }
      if (chan[i].keyOff && !isMuted[i]) {
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(isMuted[i]?0:(chan[i].wave<<4))|(chan[i].ring<<2)|(chan[i].sync<<1)|0);
      }
      rWrite(i*7,chan[i].freq&0xff);
      rWrite(i*7+1,chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformC64::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      if (chan[c.chan].insChanged || chan[c.chan].resetDuty || ins->std.waveMacroLen>0) {
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].wave=(ins->c64.noiseOn<<3)|(ins->c64.pulseOn<<2)|(ins->c64.sawOn<<1)|(ins->c64.triOn);
        chan[c.chan].attack=ins->c64.a;
        chan[c.chan].decay=(ins->c64.s==15)?0:ins->c64.d;
        chan[c.chan].sustain=ins->c64.s;
        chan[c.chan].release=ins->c64.r;
        chan[c.chan].ring=ins->c64.ringMod;
        chan[c.chan].sync=ins->c64.oscSync;
      }
      if (chan[c.chan].insChanged || chan[c.chan].resetFilter) {
        chan[c.chan].filter=ins->c64.toFilter;
        if (ins->c64.initFilter) {
          filtCut=ins->c64.cut;
          filtRes=ins->c64.res;
          filtControl=ins->c64.lp|(ins->c64.bp<<1)|(ins->c64.hp<<2)|(ins->c64.ch3off<<3);
          updateFilter();
        }
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].std.init(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      //chan[c.chan].std.init(NULL);
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
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
          vol=chan[c.chan].outVol;
        } else {
          vol=chan[c.chan].vol;
        }
        updateFilter();
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
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].duty=(c.value*4095)/100;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      break;
    case DIV_CMD_C64_FINE_DUTY:
      chan[c.chan].duty=c.value;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      break;
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|chan[c.chan].active);
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      chan[c.chan].keyOn=true;
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      if (resetTime) chan[c.chan].testWhen=c.value-resetTime+1;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_C64_CUTOFF:
      if (c.value>100) c.value=100;
      filtCut=c.value*2047/100;
      updateFilter();
      break;
    case DIV_CMD_C64_FINE_CUTOFF:
      filtCut=c.value;
      updateFilter();
      break;
    case DIV_CMD_C64_RESONANCE:
      if (c.value>15) c.value=15;
      filtRes=c.value;
      updateFilter();
      break;
    case DIV_CMD_C64_FILTER_MODE:
      filtControl=c.value&7;
      updateFilter();
      break;
    case DIV_CMD_C64_RESET_TIME:
      resetTime=c.value;
      break;
    case DIV_CMD_C64_RESET_MASK:
      chan[c.chan].resetMask=c.value;
      break;
    case DIV_CMD_C64_FILTER_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins);
        if (ins->c64.initFilter) {
          filtCut=ins->c64.cut;
          updateFilter();
        }
      }
      chan[c.chan].resetFilter=c.value>>4;
      break;
    case DIV_CMD_C64_DUTY_RESET:
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins);
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      }
      chan[c.chan].resetDuty=c.value>>4;
      break;
    case DIV_CMD_C64_EXTENDED:
      switch (c.value>>4) {
        case 0:
          chan[c.chan].attack=c.value&15;
          break;
        case 1:
          chan[c.chan].decay=c.value&15;
          break;
        case 2:
          chan[c.chan].sustain=c.value&15;
          break;
        case 3:
          chan[c.chan].release=c.value&15;
          break;
        case 4:
          chan[c.chan].ring=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|chan[c.chan].active);
          break;
        case 5:
          chan[c.chan].sync=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|chan[c.chan].active);
          break;
        case 6:
          filtControl&=7;
          filtControl|=(!!c.value)<<3;
          break;
      }
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformC64::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  rWrite(ch*7+4,(isMuted[ch]?0:(chan[ch].wave<<4))|(chan[ch].ring<<2)|(chan[ch].sync<<1)|chan[ch].active);
}

void DivPlatformC64::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    chan[i].testWhen=0;
  }
  updateFilter();
}

void DivPlatformC64::notifyInsChange(int ins) {
  for (int i=0; i<3; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformC64::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformC64::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformC64::reset() {
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformC64::Channel();
  }

  sid.reset();

  rWrite(0x18,0x0f);

  filtControl=0;
  filtRes=0;
  filtCut=2047;
  resetTime=1;
  vol=15;
}

void DivPlatformC64::setChipModel(bool is6581) {
  if (is6581) {
    sid.set_chip_model(MOS6581);
  } else {
    sid.set_chip_model(MOS8580);
  }
}

void DivPlatformC64::setFlags(unsigned int flags) {
  if (flags&1) {
    rate=COLOR_PAL*2.0/9.0;
  } else {
    rate=COLOR_NTSC*2.0/7.0;
  }
  chipClock=rate;
}

int DivPlatformC64::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
  }
  setFlags(flags);

  reset();

  return 3;
}

void DivPlatformC64::quit() {
}

DivPlatformC64::~DivPlatformC64() {
}
