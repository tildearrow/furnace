/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "powernoise.h"
#include "../engine.h"
#include "../../ta-log.h"
#include "furIcons.h"
#include <math.h>
#include "../bsr.h"

#define rWrite(a,v) if (!skipRegisterWrites) {regPool[a]=(v); pwrnoise_write(&pn,(unsigned char)(a),(unsigned char)(v)); if (dumpWrites) {addWrite(a,v);}}
#define chWrite(c,a,v) rWrite((c<<3)|((a)+1),(v))
#define noiseCtl(enable,am,tapB) (((enable)?0x80:0x00)|((am)?0x02:0x00)|((tapB)?0x01:0x00))
#define slopeCtl(enable,rst,a,b) (((enable)?0x80:0x00)| \
  (rst?0x40:0x00)| \
  (a.clip?0x20:0x00)| \
  (b.clip?0x10:0x00)| \
  (a.reset?0x08:0x00)| \
  (b.reset?0x04:0x00)| \
  (a.dir?0x02:0x00)| \
  (b.dir?0x01:0x00))
#define volPan(v,p) (((v*(p>>4)/15)<<4)|((v*(p&0xf)/15)&0xf))
// TODO: optimize!
#define mapAmp(a) ((((a)*65535/63-32768)*(pn.flags&0x7)/7)>>1)

const char* regCheatSheetPowerNoise[]={
  "ACTL", "00",
  "N1CTL", "01",
  "N1FL", "02",
  "N1FH", "03",
  "N1SRL", "04",
  "N1SRH", "05",
  "N1TAP", "06",
  "N1V", "07",
  "IOA", "08",
  "N2CTL", "09",
  "N2FL", "0A",
  "N2FH", "0B",
  "N2SRL", "0C",
  "N2SRH", "0D",
  "N2TAP", "0E",
  "N2V", "0F",
  "IOB", "10",
  "N3CTL", "11",
  "N3FL", "12",
  "N3FH", "13",
  "N3SRL", "14",
  "N3SRH", "15",
  "N3TAP", "16",
  "N3V", "17",
  "SLACC", "18",
  "SLCTL", "19",
  "SLFL", "1A",
  "SLFH", "1B",
  "SLPA", "1C",
  "SLPB", "1D",
  "SLPO", "1E",
  "SLV", "1F",
  NULL
};

const char** DivPlatformPowerNoise::getRegisterSheet() {
  return regCheatSheetPowerNoise;
}

void DivPlatformPowerNoise::acquire(short** buf, size_t len) {
  short left, right;

  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    pwrnoise_step(&pn,coreQuality,&left,&right);

    oscBuf[0]->putSample(h,mapAmp((pn.n1.out_latch&0xf)+(pn.n1.out_latch>>4)));
    oscBuf[1]->putSample(h,mapAmp((pn.n2.out_latch&0xf)+(pn.n2.out_latch>>4)));
    oscBuf[2]->putSample(h,mapAmp((pn.n3.out_latch&0xf)+(pn.n3.out_latch>>4)));
    oscBuf[3]->putSample(h,mapAmp((pn.s.out_latch&0xf)+(pn.s.out_latch>>4)));

    buf[0][h]=left;
    buf[1][h]=right;
  }

  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
}

/* macros:
 *  EX1 - control (0-63)
 *  EX2 - portion A length (0-255) - slope only
 *  EX3 - portion B length (0-255) - slope only
 *  EX4 - tap A location (0-15) - noise only
 *  EX5 - tap B location (0-15) - noise only
 *  EX6 - portion A offset (0-15) - slope only
 *  EX7 - portion B offset (0-15) - slope only
 *  EX8 - load LFSR (0-65535) - noise only
 */

void DivPlatformPowerNoise::tick(bool sysTick) {
  for (int i=0; i<4; i++) {
    int CHIP_DIVIDER=2;
    if (i==3) CHIP_DIVIDER=128;
    chan[i].std.next();

    if (chan[i].std.ex1.had) {
      int val=chan[i].std.ex1.val;

      if (chan[i].slope) {
        chan[i].slopeA.clip=(val&0x20);
        chan[i].slopeB.clip=(val&0x10);
        chan[i].slopeA.reset=(val&0x08);
        chan[i].slopeB.reset=(val&0x04);
        chan[i].slopeA.dir=(val&0x02);
        chan[i].slopeB.dir=(val&0x01);
        chWrite(i,0x00,slopeCtl(chan[i].active,false,chan[i].slopeA,chan[i].slopeB));
      } else {
        chan[i].am=(val&0x02);
        chan[i].tapBEnable=(val&0x01);
        chWrite(i,0x00,noiseCtl(chan[i].active, chan[i].am, chan[i].tapBEnable));
      }
    }
    if (chan[i].slope) {
      if (chan[i].std.ex2.had) {
        chan[i].slopeA.len=chan[i].std.ex2.val;
        chWrite(i,0x03,chan[i].std.ex2.val);
      }
      if (chan[i].std.ex3.had) {
        chan[i].slopeB.len=chan[i].std.ex3.val;
        chWrite(i,0x04,chan[i].std.ex3.val);
      }
      if (chan[i].std.ex6.had) {
        chan[i].slopeA.offset=chan[i].std.ex6.val;
      }
      if (chan[i].std.ex7.had) {
        chan[i].slopeB.offset=chan[i].std.ex7.val;
      }
      if (chan[i].std.ex6.had || chan[i].std.ex7.had) {
        chWrite(i,0x05,(chan[i].slopeA.offset<<4)|chan[i].slopeB.offset);
      }
    } else {
      if (chan[i].std.ex4.had) {
        chan[i].tapA=chan[i].std.ex4.val;
      }
      if (chan[i].std.ex5.had) {
        chan[i].tapB=chan[i].std.ex5.val;
      }
      if (chan[i].std.ex4.had || chan[i].std.ex5.had) {
        chWrite(i,0x05,(chan[i].tapA<<4)|chan[i].tapB);
      }
      if (chan[i].std.ex8.had) {
        if (chan[i].initLFSR!=(chan[i].std.ex8.val&0xffff)) {
          chan[i].initLFSR=chan[i].std.ex8.val&0xffff;
          chWrite(i,0x03,chan[i].std.ex8.val&0xff);
          chWrite(i,0x04,chan[i].std.ex8.val>>8);
        }
      }
    }

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LINEAR(chan[i].vol&15,MIN(15,chan[i].std.vol.val),15);
      if (chan[i].outVol<0) chan[i].outVol=0;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.panL.had) {
      chan[i].pan&=0x0f;
      chan[i].pan|=(chan[i].std.panL.val&15)<<4;
    }
    if (chan[i].std.panR.had) {
      chan[i].pan&=0xf0;
      chan[i].pan|=chan[i].std.panR.val&15;
    }

    if (chan[i].std.vol.had || chan[i].std.panL.had || chan[i].std.panR.had) {
      chWrite(i,0x06,isMuted[i]?0:volPan(chan[i].outVol,chan[i].pan));
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      if (chan[i].slope) {
        chWrite(i,0x00,slopeCtl(chan[i].active,true,chan[i].slopeA,chan[i].slopeB));
        chan[i].keyOn=true;
      } else {
        chWrite(i,0x03,chan[i].initLFSR&0xff);
        chWrite(i,0x04,chan[i].initLFSR>>8);
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      chan[i].freq>>=chan[i].octaveOff;

      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0x7ffffff) chan[i].freq=0x7ffffff;
      int bsr32Val=bsr32(chan[i].freq);
      chan[i].octave=MAX(bsr32Val-12,0);
      if (chan[i].octave>15) chan[i].octave=15;
      chan[i].fNum=0x1000-(chan[i].freq>>chan[i].octave);
      if (chan[i].fNum<0) chan[i].fNum=0;
      if (chan[i].fNum>4095) chan[i].fNum=4095;

      chWrite(i,0x01,chan[i].fNum&0xff);
      chWrite(i,0x02,(chan[i].fNum>>8)|(chan[i].octave<<4));

      if (chan[i].keyOn) {
        if (chan[i].slope) {
          chWrite(i,0x00,slopeCtl(true,false,chan[i].slopeA,chan[i].slopeB));
        } else {
          chWrite(i,0x00,noiseCtl(true,chan[i].am,chan[i].tapBEnable));
        }
      }
      if (chan[i].keyOff) {
        if (chan[i].slope) {
          chWrite(i,0x00,slopeCtl(false,false,chan[i].slopeA,chan[i].slopeB));
        } else {
          chWrite(i,0x00,noiseCtl(false,chan[i].am,chan[i].tapBEnable));
        }
      }

      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }

    if (chan[i].slope) {
      unsigned char counter=pn.s.accum;
      regPool[0x18]=counter;
    } else {
      unsigned short lfsr;
      if (i==0) {
        lfsr=pn.n1.lfsr;
      } else if (i==1) {
        lfsr=pn.n2.lfsr;
      } else {
        lfsr=pn.n3.lfsr;
      }
      regPool[(i<<3)+0x4]=lfsr&0xff;
      regPool[(i<<3)+0x5]=lfsr>>8;
    }
  }
}

int DivPlatformPowerNoise::dispatch(DivCommand c) {
  int CHIP_DIVIDER=2;
  if (c.chan==3) CHIP_DIVIDER=128;

  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_POWERNOISE);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].octaveOff=ins->powernoise.octave;
      }
      chan[c.chan].active=true;
      chan[c.chan].macroInit(ins);
      if (!parent->song.compatFlags.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          chWrite(c.chan,0x06,isMuted[c.chan]?0:volPan(chan[c.chan].outVol,chan[c.chan].pan));
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
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
        if (chan[c.chan].baseFreq>0x7ffffff) {
          chan[c.chan].baseFreq=0x7ffffff;
        }
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<0) {
          chan[c.chan].baseFreq=0;
        }
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
    case DIV_CMD_PANNING:
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      break;
    case DIV_CMD_LEGATO: {
      int whatAMess=c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0));

      chan[c.chan].baseFreq=NOTE_PERIODIC(whatAMess);
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    }
    case DIV_CMD_PRE_PORTA: {
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_POWERNOISE));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      }
      chan[c.chan].inPorta=c.value;
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_POWERNOISE_COUNTER_LOAD: {
      if (chan[c.chan].slope && c.value==0) {
        rWrite(0x18,c.value2&0x7f);
      } else if (!chan[c.chan].slope) {
        chWrite(c.chan,0x03+c.value,c.value2);
      }
      break;
    }
    case DIV_CMD_POWERNOISE_IO_WRITE:
      rWrite(0x08+(c.value<<3),c.value2);
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformPowerNoise::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chWrite(ch,0x06,isMuted[ch]?0:volPan(chan[ch].outVol,chan[ch].pan));
}

void DivPlatformPowerNoise::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    if (i<3) {
      chWrite(i,0x03,chan[i].initLFSR&0xff);
      chWrite(i,0x04,chan[i].initLFSR>>8);
    }
  }
}

void* DivPlatformPowerNoise::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformPowerNoise::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformPowerNoise::getPan(int ch) {
  return ((chan[ch].pan&0xf0)<<4)|(chan[ch].pan&15);
}

DivChannelModeHints DivPlatformPowerNoise::getModeHints(int ch) {
  DivChannelModeHints ret;
  ret.count=0;
  return ret;
}

bool DivPlatformPowerNoise::getDCOffRequired() {
  return true;
}

DivDispatchOscBuffer* DivPlatformPowerNoise::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformPowerNoise::getRegisterPool() {
  return regPool;
}

int DivPlatformPowerNoise::getRegisterPoolSize() {
  return 32;
}

void DivPlatformPowerNoise::reset() {
  memset(regPool,0,32);
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformPowerNoise::Channel();
    chan[i].std.setEngine(parent);
    chan[i].slope=(i==3);
  }

  pwrnoise_reset(&pn);

  // set master volume to full
  rWrite(0,0x87);
  // set per-channel panning
  for (int i=0; i<4; i++) {
    chWrite(i,0x06,isMuted[i]?0:volPan(chan[i].outVol,chan[i].pan));
  }
  // set default params so we have sound
  // noise
  for (int i=0; i<3; i++) {
    chWrite(i,0x03,chan[i].initLFSR&0xff);
    chWrite(i,0x04,chan[i].initLFSR>>8);
    chWrite(i,0x05,(chan[i].tapA<<4)|chan[i].tapB);
  }
  // slope
  chWrite(3,0x03,chan[3].slopeA.len);
  chWrite(3,0x04,chan[3].slopeB.len);
  chWrite(3,0x05,(chan[3].slopeA.offset<<4)|chan[3].slopeB.offset);

  addWrite(0xffffffff,0);
}

int DivPlatformPowerNoise::getOutputCount() {
  return 2;
}

bool DivPlatformPowerNoise::hasSoftPan(int ch) {
  return true;
}

bool DivPlatformPowerNoise::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformPowerNoise::notifyWaveChange(int wave) {
}

void DivPlatformPowerNoise::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPowerNoise::setFlags(const DivConfig& flags) {
  chipClock=16000000;

  CHECK_CUSTOM_CLOCK;
  rate=chipClock/coreQuality;

  for (int i=0; i<4; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformPowerNoise::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformPowerNoise::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformPowerNoise::setCoreQuality(unsigned char q) {
  switch (q) {
    case 0:
      coreQuality=256;
      break;
    case 1:
      coreQuality=128;
      break;
    case 2:
      coreQuality=64;
      break;
    case 3:
      coreQuality=32;
      break;
    case 4:
      coreQuality=8;
      break;
    case 5:
      coreQuality=1;
      break;
    default:
      coreQuality=32;
      break;
  }
}

int DivPlatformPowerNoise::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<4; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    isMuted[i]=false;
  }
  setFlags(flags);
  reset();
  return 4;
}

void DivPlatformPowerNoise::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
}

DivPlatformPowerNoise::~DivPlatformPowerNoise() {
}
