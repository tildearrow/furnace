#include "sms.h"
#include "../engine.h"
#include <math.h>

#define rWrite(v) {if (!skipRegisterWrites) {sn->write(v); if (dumpWrites) {addWrite(0x200,v);}}}

#define CHIP_DIVIDER 64

const char* regCheatSheetSN[]={
  "DATA", "0",
  NULL
};

const char** DivPlatformSMS::getRegisterSheet() {
  return regCheatSheetSN;
}

void DivPlatformSMS::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  sn->sound_stream_update(bufL+start,len);
}

int DivPlatformSMS::acquireOne() {
  short v;
  sn->sound_stream_update(&v,1);
  return v;
}

void DivPlatformSMS::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=((chan[i].vol&15)*MIN(15,chan[i].std.vol))>>4;
      rWrite(0x90|(i<<5)|(isMuted[i]?15:(15-(chan[i].outVol&15))));
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp);
          chan[i].actualNote=chan[i].std.arp;
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp);
          chan[i].actualNote=chan[i].note+chan[i].std.arp;
        }
        chan[i].freqChanged=true;
      }
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        chan[i].actualNote=chan[i].note;
        chan[i].freqChanged=true;
      }
    }
    if (i==3) if (chan[i].std.hadDuty) {
      snNoiseMode=chan[i].std.duty;
      if (chan[i].std.duty<2) {
        chan[3].freqChanged=false;
      }
      updateSNMode=true;
    }
  }
  for (int i=0; i<3; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>1023) chan[i].freq=1023;
      if (chan[i].actualNote>0x5d) chan[i].freq=0x01;
      rWrite(0x80|i<<5|(chan[i].freq&15));
      rWrite(chan[i].freq>>4);
      if (i==2 && snNoiseMode&2) {
        chan[3].baseFreq=chan[2].baseFreq;
        chan[3].actualNote=chan[2].actualNote;
      }
      chan[i].freqChanged=false;
    }
  }
  if (chan[3].freqChanged || updateSNMode) {
    updateSNMode=false;
    // seems arbitrary huh?
    chan[3].freq=parent->calcFreq(chan[3].baseFreq,chan[3].pitch-1-(isRealSN?127:0),true);
    if (chan[3].freq>1023) chan[3].freq=1023;
    if (chan[3].actualNote>0x5d) chan[3].freq=0x01;
    chan[3].freqChanged=false;
    if (snNoiseMode&2) { // take period from channel 3
      if (snNoiseMode&1) {
        rWrite(0xe7);
      } else {
        rWrite(0xe3);
      }
      rWrite(0xdf);
      rWrite(0xc0|(chan[3].freq&15));
      rWrite(chan[3].freq>>4);
    } else { // 3 fixed values
      unsigned char value;
      if (chan[3].std.hadArp) {
        if (chan[3].std.arpMode) {
          value=chan[3].std.arp%12;
        } else {
          value=(chan[3].note+chan[3].std.arp)%12;
        }
      } else {
        value=chan[3].note%12;
      }
      if (value<3) {
        value=2-value;
        rWrite(0xe0|value|((snNoiseMode&1)<<2));
      }
    }
  }
}

int DivPlatformSMS::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].actualNote=c.value;
      }
      chan[c.chan].active=true;
      rWrite(0x90|c.chan<<5|(isMuted[c.chan]?15:(15-(chan[c.chan].vol&15))));
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      rWrite(0x9f|c.chan<<5);
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=c.value;
      //chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) rWrite(0x90|c.chan<<5|(isMuted[c.chan]?15:(15-(chan[c.chan].vol&15))));
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
      snNoiseMode=(c.value&1)|((c.value&16)>>3);
      updateSNMode=true;
      break;
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].actualNote=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformSMS::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (chan[ch].active) rWrite(0x90|ch<<5|(isMuted[ch]?15:(15-(chan[ch].outVol&15))));
}

void DivPlatformSMS::forceIns() {
  for (int i=0; i<4; i++) {
    if (chan[i].active) {
      chan[i].insChanged=true;
      chan[i].freqChanged=true;
    }
  }
}

void* DivPlatformSMS::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformSMS::reset() {
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformSMS::Channel();
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  sn->device_start();
  snNoiseMode=3;
  updateSNMode=false;
}

bool DivPlatformSMS::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformSMS::keyOffAffectsPorta(int ch) {
  return true;
}

int DivPlatformSMS::getPortaFloor(int ch) {
  return 12;
}

void DivPlatformSMS::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSMS::poke(unsigned int addr, unsigned short val) {
  rWrite(val);
}

void DivPlatformSMS::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.val);
}

void DivPlatformSMS::setFlags(unsigned int flags) {
  if ((flags&3)==2) {
    chipClock=4000000;
  } else if ((flags&3)==1) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  if (sn!=NULL) delete sn;
  switch (flags>>2) {
    case 1: // TI
      sn=new sn76496_base_device(0x4000, 0x4000, 0x01, 0x02, true, 1, false, true);
      isRealSN=true;
      break;
    case 2: // TI+Atari
      sn=new sn76496_base_device(0x4000, 0x0f35, 0x01, 0x02, true, 1, false, true);
      isRealSN=true;
      break;
    case 3: // Game Gear (not fully emulated yet!)
      sn=new sn76496_base_device(0x8000, 0x8000, 0x01, 0x08, false, 1, false, false);
      isRealSN=false;
      break;
    default: // Sega
      sn=new sn76496_base_device(0x8000, 0x8000, 0x01, 0x08, false, 1, false, false);
      isRealSN=false;
      break;
  }
  rate=chipClock/16;
}

int DivPlatformSMS::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
  }
  sn=NULL;
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformSMS::quit() {
  if (sn!=NULL) delete sn;
}

DivPlatformSMS::~DivPlatformSMS() {
}
