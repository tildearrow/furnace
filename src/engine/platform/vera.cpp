/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "vera.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

extern "C" {
  #include "sound/vera_psg.h"
  #include "sound/vera_pcm.h"
}

//if (dumpWrites) {addWrite(((c)*4+(a)),(d));}
//#define rWrite(c,a,d) {regPool[(c)*4+(a)]=(d); psg_writereg(psg,((c)*4+(a)),(d));}
#define rWrite(c,a,d) {regPool[(c)*4+(a)]=(d); psg_writereg(psg,((c)*4+(a)),(d));if (dumpWrites) {addWrite(((c)*4+(a)),(d));}}
#define rWriteLo(c,a,d) rWrite(c,a,(regPool[(c)*4+(a)]&(~0x3f))|((d)&0x3f))
#define rWriteHi(c,a,d) rWrite(c,a,(regPool[(c)*4+(a)]&(~0xc0))|(((d)<<6)&0xc0))
#define rWritePCMCtrl(d) {regPool[64]=(d); pcm_write_ctrl(pcm,d);}
#define rWritePCMRate(d) {regPool[65]=(d); pcm_write_rate(pcm,d);}
#define rWritePCMData(d) {regPool[66]=(d); pcm_write_fifo(pcm,d);}
#define rWritePCMVol(d) rWritePCMCtrl((regPool[64]&(~0x3f))|((d)&0x3f))

const char* regCheatSheetVERA[]={
  "CHxFreq",    "00+x*4",
  "CHxVol",     "02+x*4",
  "CHxWave",    "03+x*4",

  "AUDIO_CTRL", "40",
  "AUDIO_RATE", "41",
  "AUDIO_DATA", "42",

  NULL
};

const char** DivPlatformVERA::getRegisterSheet() {
  return regCheatSheetVERA;
}

const char* DivPlatformVERA::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x20:
      return "20xx: Change waveform";
      break;
    case 0x22:
      return "22xx: Set duty cycle (0 to 3F)";
      break;
  }
  return NULL;
}

void DivPlatformVERA::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  // both PSG part and PCM part output a full 16-bit range, putting bufL/R
  // argument right into both could cause an overflow
  short buf[4][128];
  size_t pos=start;
  DivSample* s=parent->getSample(chan[16].pcm.sample);
  while (len>0) {
    if (s->samples>0) {
      while (pcm_is_fifo_almost_empty(pcm)) {
        short tmp_l=0;
        short tmp_r=0;
        if (!isMuted[16]) {
          // TODO stereo samples once DivSample has a support for it
          if (chan[16].pcm.depth16) {
            tmp_l=s->data16[chan[16].pcm.pos];
            tmp_r=tmp_l;
          } else {
            tmp_l=s->data8[chan[16].pcm.pos];
            tmp_r=tmp_l;
          }
          if (!(chan[16].pan&1)) tmp_l=0;
          if (!(chan[16].pan&2)) tmp_r=0;
        }
        if (chan[16].pcm.depth16) {
          rWritePCMData(tmp_l&0xff);
          rWritePCMData((tmp_l>>8)&0xff);
          rWritePCMData(tmp_r&0xff);
          rWritePCMData((tmp_r>>8)&0xff);
        } else {
          rWritePCMData(tmp_l&0xff);
          rWritePCMData(tmp_r&0xff);
        }
        chan[16].pcm.pos++;
        if (chan[16].pcm.pos>=s->samples) {
          if (s->loopStart>=0 && s->loopStart<(int)s->samples) {
            chan[16].pcm.pos=s->loopStart;
          } else {
            chan[16].pcm.sample=-1;
            break;
          }
        }
      }
    } else {
      // just let the buffer run out
      chan[16].pcm.sample=-1;
    }
    int curLen=MIN(len,128);
    memset(buf,0,sizeof(buf));
    psg_render(psg,buf[0],buf[1],curLen);
    pcm_render(pcm,buf[2],buf[3],curLen);
    for (int i=0; i<curLen; i++) {
      bufL[pos]=(short)(((int)buf[0][i]+buf[2][i])/2);
      bufR[pos]=(short)(((int)buf[1][i]+buf[3][i])/2);
      pos++;
    }
    len-=curLen;
  }
}

void DivPlatformVERA::reset() {
  for (int i=0; i<17; i++) {
    chan[i]=Channel();
    chan[i].std.setEngine(parent);
  }
  psg_reset(psg);
  pcm_reset(pcm);
  memset(regPool,0,67);
  for (int i=0; i<16; i++) {
    chan[i].vol=63;
    chan[i].pan=3;
    rWriteHi(i,2,isMuted[i]?0:3);
  }
  chan[16].vol=15;
  chan[16].pan=3;
}

int DivPlatformVERA::calcNoteFreq(int ch, int note) {
  if (ch<16) {
    return parent->calcBaseFreq(chipClock,2097152,note,false);
  } else {
    double off=1.0;
    if (chan[ch].pcm.sample>=0 && chan[ch].pcm.sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[ch].pcm.sample);
      if (s->centerRate<1) {
        off=1.0;
      } else {
        off=s->centerRate/8363.0;
      }
    }
    return (int)(off*parent->calcBaseFreq(chipClock,65536,note,false));
  }
}

void DivPlatformVERA::tick(bool sysTick) {
  for (int i=0; i<16; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=MAX(chan[i].vol+chan[i].std.vol.val-63,0);
      rWriteLo(i,2,chan[i].outVol);
    }
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=calcNoteFreq(0,chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=calcNoteFreq(0,chan[i].note+chan[i].std.arp.val);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=calcNoteFreq(0,chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.duty.had) {
      rWriteLo(i,3,chan[i].std.duty.val);
    }
    if (chan[i].std.wave.had) {
      rWriteHi(i,3,chan[i].std.wave.val);
    }
    if (i<16) {
      if (chan[i].std.panL.had) {
        chan[i].pan=chan[i].std.panL.val&3;
        rWriteHi(i,2,isMuted[i]?0:chan[i].pan);
      }
    }
    if (chan[i].std.pitch.had) {
      chan[i].freqChanged=true;
    }
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,8)+chan[i].std.pitch.val;
      if (chan[i].freq>65535) chan[i].freq=65535;
      rWrite(i,0,chan[i].freq&0xff);
      rWrite(i,1,(chan[i].freq>>8)&0xff);
      chan[i].freqChanged=false;
    }
  }
  // PCM
  chan[16].std.next();
  if (chan[16].std.vol.had) {
    chan[16].outVol=MAX(chan[16].vol+MIN(chan[16].std.vol.val/4,15)-15,0);
    rWritePCMVol(chan[16].outVol&15);
  }
  if (chan[16].std.arp.had) {
    if (!chan[16].inPorta) {
      if (chan[16].std.arp.mode) {
        chan[16].baseFreq=calcNoteFreq(16,chan[16].std.arp.val);
      } else {
        chan[16].baseFreq=calcNoteFreq(16,chan[16].note+chan[16].std.arp.val);
      }
    }
    chan[16].freqChanged=true;
  } else {
    if (chan[16].std.arp.mode && chan[16].std.arp.finished) {
      chan[16].baseFreq=calcNoteFreq(16,chan[16].note);
      chan[16].freqChanged=true;
    }
  }
  if (chan[16].freqChanged) {
    chan[16].freq=parent->calcFreq(chan[16].baseFreq,chan[16].pitch,false,8)+chan[16].std.pitch.val;
    if (chan[16].freq>128) chan[16].freq=128;
    rWritePCMRate(chan[16].freq&0xff);
    chan[16].freqChanged=false;
  }
}

int DivPlatformVERA::dispatch(DivCommand c) {
  int tmp;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if(c.chan<16) {
        rWriteLo(c.chan,2,chan[c.chan].vol)
      } else {
        chan[16].pcm.sample=parent->getIns(chan[16].ins,DIV_INS_VERA)->amiga.initSample;
        if (chan[16].pcm.sample<0 || chan[16].pcm.sample>=parent->song.sampleLen) {
          chan[16].pcm.sample=-1;
        }
        chan[16].pcm.pos=0;
        DivSample* s=parent->getSample(chan[16].pcm.sample);
        unsigned char ctrl=0x90|chan[16].vol; // always stereo
        if (s->depth==16) {
          chan[16].pcm.depth16=true;
          ctrl|=0x20;
        } else {
          chan[16].pcm.depth16=false;
          if (s->depth!=8) chan[16].pcm.sample=-1;
        }
        rWritePCMCtrl(ctrl);
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=calcNoteFreq(c.chan,c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_VERA));
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      if(c.chan<16) {
        rWriteLo(c.chan,2,0)
      } else {
        chan[16].pcm.sample=-1;
        rWritePCMCtrl(0x80);
        rWritePCMRate(0);
      }
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      chan[c.chan].ins=(unsigned char)c.value;
      break;
    case DIV_CMD_VOLUME:
      if (c.chan<16) {
        tmp=c.value&0x3f;
        chan[c.chan].vol=tmp;
        rWriteLo(c.chan,2,tmp);
      } else {
        tmp=c.value&0x0f;
        chan[c.chan].vol=tmp;
        rWritePCMVol(tmp);
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
      int destFreq=calcNoteFreq(c.chan,c.value2);
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
      chan[c.chan].baseFreq=calcNoteFreq(c.chan,c.value+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].std.init(parent->getIns(chan[c.chan].ins,DIV_INS_VERA));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_STD_NOISE_MODE:
      if (c.chan<16) rWriteLo(c.chan,3,c.value);
      break;
    case DIV_CMD_WAVE:
      if (c.chan<16) rWriteHi(c.chan,3,c.value);
      break;
    case DIV_CMD_PANNING: {
      tmp=0;
      tmp|=(c.value&0x10)?1:0;
      tmp|=(c.value&0x01)?2:0;
      chan[c.chan].pan=tmp&3;
      if (c.chan<16) {
        rWriteHi(c.chan,2,isMuted[c.chan]?0:chan[c.chan].pan);
      }
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      if(c.chan<16) {
        return 63;
      } else {
        return 15;
      }
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    default:
      break;
  }
  return 1;
}

void* DivPlatformVERA::getChanState(int ch) {
  return &chan[ch];
}

unsigned char* DivPlatformVERA::getRegisterPool() {
  return regPool;
}

int DivPlatformVERA::getRegisterPoolSize() {
  return 67;
}

void DivPlatformVERA::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (ch<16) {
    rWriteHi(ch,2,mute?0:chan[ch].pan);
  }
}

float DivPlatformVERA::getPostAmp() {
  return 8.0f;
}

bool DivPlatformVERA::isStereo() {
  return true;
}

void DivPlatformVERA::notifyInsDeletion(void* ins) {
  for (int i=0; i<2; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformVERA::poke(unsigned int addr, unsigned short val) {
  switch (addr) {
    case 64:
      rWritePCMCtrl((unsigned char)val);
      break;
    case 65:
      rWritePCMRate((unsigned char)val);
      break;
    case 66:
      rWritePCMData((unsigned char)val);
      break;
    default:
      rWrite(0,addr,(unsigned char)val);
      break;
  }
}

void DivPlatformVERA::poke(std::vector<DivRegWrite>& wlist) {
  for (auto &i: wlist) poke(i.addr,i.val);
}

int DivPlatformVERA::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  for (int i=0; i<17; i++) {
    isMuted[i]=false;
  }
  parent=p;
  psg=new struct VERA_PSG;
  pcm=new struct VERA_PCM;
  dumpWrites=false;
  skipRegisterWrites=false;
  chipClock=25000000;
  rate=chipClock/512;
  reset();
  return 17;
}

void DivPlatformVERA::quit() {
  delete psg;
  delete pcm;
}
DivPlatformVERA::~DivPlatformVERA() {
}
