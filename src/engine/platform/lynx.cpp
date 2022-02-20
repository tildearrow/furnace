#include "lynx.h"
#include "../engine.h"
#include <math.h>

#define rWrite(a,v) {if (!skipRegisterWrites) {mikey->write(a,v); if (dumpWrites) {addWrite(a,v);}}}

#define WRITE_VOLUME(ch,v) rWrite(0x20+(ch<<3),(v))
#define WRITE_FEEDBACK(ch,v) rWrite(0x21+(ch<<3),(v))
#define WRITE_LFSR(ch,v) rWrite(0x23+(ch<<3),(v))
#define WRITE_BACKUP(ch,v) rWrite(0x24+(ch<<3),(v))
#define WRITE_CONTROL(ch,v) rWrite(0x25+(ch<<3),(v))
#define WRITE_OTHER(ch,v) rWrite(0x27+(ch<<3),(v))

#define CHIP_DIVIDER 64

#if defined( _MSC_VER )

#include <intrin.h>

static int bsr(uint16_t v) {
  unsigned long idx;
  if (_BitScanReverse(&idx,(unsigned long)v)) {
    return idx;
  }
  else {
    return -1;
  }
}

#elif defined( __GNUC__ )

static int bsr(uint16_t v)
{
  if (v) {
    return 16 - __builtin_clz(v);
  }
  else{
    return -1;
  }
}

#else

static int bsr(uint16_t v)
{
  uint16_t mask = 0x8000;
  for (int i = 31; i >= 0; --i) {
    if (v&mask)
      return (int)i;
    mask>>=1;
  }

  return -1;
}

#endif

static int32_t clamp(int32_t v, int32_t lo, int32_t hi)
{
  return v<lo?lo:(v>hi?hi:v);
}

const char* regCheatSheetLynx[]={
  "DATA", "0",
  NULL
};


const char** DivPlatformLynx::getRegisterSheet() {
  return regCheatSheetLynx;
}

void DivPlatformLynx::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  mikey->sampleAudio( bufL + start, bufR + start, len );
}

void DivPlatformLynx::tick() {
  for (int i=0; i<4; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=((chan[i].vol&127)*MIN(127,chan[i].std.vol))>>7;
      WRITE_VOLUME(i,(isMuted[i]?0:(chan[i].outVol&127)));
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

    if (chan[i].freqChanged) {
      chan[i].fd=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].resetLFSR) {
        chan[i].resetLFSR=false;
        WRITE_LFSR(i, 0);
        WRITE_OTHER(i, 0);
      }
      if (chan[i].std.hadDuty) {
        chan[i].duty = chan[i].std.duty;
        WRITE_FEEDBACK(i, chan[i].duty.feedback);
      }
      WRITE_BACKUP(i, chan[i].fd.backup);
      WRITE_CONTROL(i, (chan[i].fd.clockDivider|0x18|chan[i].duty.int_feedback7));
    }
    else if (chan[i].std.hadDuty)
    {
      chan[i].duty = chan[i].std.duty;
      WRITE_FEEDBACK(i, chan[i].duty.feedback);
      WRITE_CONTROL(i, (chan[i].fd.clockDivider|0x18|chan[i].duty.int_feedback7));
    }
  }
}

int DivPlatformLynx::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].resetLFSR=true;
        chan[c.chan].note=c.value;
        chan[c.chan].actualNote=c.value;
      }
      chan[c.chan].active=true;
      WRITE_VOLUME(c.chan,(isMuted[c.chan]?0:(chan[c.chan].vol&127)));
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      WRITE_VOLUME(c.chan, 0);
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
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
        if (chan[c.chan].active) WRITE_VOLUME(c.chan,(isMuted[c.chan]?0:(chan[c.chan].vol&127)));
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
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((chan[c.chan].std.willArp && !chan[c.chan].std.arpMode)?(chan[c.chan].std.arp):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].actualNote=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformLynx::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (chan[ch].active) WRITE_VOLUME(ch,(isMuted[ch]?0:(chan[ch].outVol&127)));
}

void DivPlatformLynx::forceIns() {
  for (int i=0; i<4; i++) {
    if (chan[i].active) {
      chan[i].insChanged=true;
      chan[i].freqChanged=true;
    }
  }
}

void* DivPlatformLynx::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformLynx::reset() {
  for (int i=0; i<4; i++) {
    chan[i]= DivPlatformLynx::Channel();
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
}

bool DivPlatformLynx::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformLynx::keyOffAffectsPorta(int ch) {
  return true;
}

//int DivPlatformLynx::getPortaFloor(int ch) {
//  return 12;
//}

void DivPlatformLynx::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformLynx::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformLynx::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr, i.val);
}

int DivPlatformLynx::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<4; i++) {
    isMuted[i]=false;
  }

  chipClock = 16000000;
  rate = chipClock/128;

  mikey = std::make_unique<Lynx::Mikey>(rate);
  reset();
  return 4;
}

void DivPlatformLynx::quit() {
  mikey.reset();
}

DivPlatformLynx::~DivPlatformLynx() {
}

DivPlatformLynx::MikeyFreqDiv::MikeyFreqDiv(int frequency) {

  int clamped=clamp(frequency, 36, 16383);

  auto top=bsr(clamped);
  
  if (top>7)
  {
    clockDivider=top-7;
    backup=frequency>>(top-7);
  }
  else
  {
    clockDivider=0;
    backup=frequency;
  }
}

DivPlatformLynx::MikeyDuty::MikeyDuty(int duty) {

  //duty:
  //9: int
  //8: f11
  //7: f10
  //6: f7
  //5: f5
  //4: f4
  //3: f3
  //2: f2
  //1: f1
  //0: f0

  //f7 moved to bit 7 and int moved to bit 5
  int_feedback7=((duty&0x40)<<1)|((duty&0x200)>>4);
  //f11 and f10 moved to bits 7 & 6
  feedback=(duty&0x3f)|((duty&0x180)>>1);
}
