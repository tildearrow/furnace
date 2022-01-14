#include "saa.h"
#include "../engine.h"
#include "sound/saa1099.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v);}

#define PSG_FREQ_BASE 122240.0f

void DivPlatformSAA1099::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (ayBufLen<len) {
    ayBufLen=len;
    for (int i=0; i<2; i++) {
      delete[] ayBuf[i];
      ayBuf[i]=new short[ayBufLen];
    }
  }
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    saa.control_w(w.addr);
    saa.data_w(w.val);
    writes.pop();
  }
  saa.sound_stream_update(ayBuf,len);
  for (size_t i=0; i<len; i++) {
    bufL[i+start]=ayBuf[0][i];
    bufR[i+start]=ayBuf[1][i];
  }
}

void DivPlatformSAA1099::tick() {
  for (int i=0; i<6; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=chan[i].std.vol-(15-chan[i].vol);
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(i,0);
      } else {
        rWrite(i,(chan[i].outVol&15));
      }
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)(chan[i].std.arp)/12.0f)));
        } else {
          chan[i].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)(chan[i].note+chan[i].std.arp-12)/12.0f)));
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)(chan[i].note)/12.0f)));
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadDuty) {
      rWrite(0x06,31-chan[i].std.duty);
    }
    if (chan[i].std.hadWave) {
      chan[i].psgMode&=4;
      chan[i].psgMode|=(chan[i].std.wave+1)&3;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>=32768) {
        chan[i].freqH=7;
      } else if (chan[i].freq>=16384) {
        chan[i].freqH=6;
      } else if (chan[i].freq>=8192) {
        chan[i].freqH=5;
      } else if (chan[i].freq>=4096) {
        chan[i].freqH=4;
      } else if (chan[i].freq>=2048) {
        chan[i].freqH=3;
      } else if (chan[i].freq>=1024) {
        chan[i].freqH=2;
      } else if (chan[i].freq>=512) {
        chan[i].freqH=1;
      } else {
        chan[i].freqH=0;
      }
      chan[i].freqL=0xff-(chan[i].freq>>chan[i].freqH);
      chan[i].freqH=7-chan[i].freqH;
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
      }
      if (chan[i].keyOff) {
        rWrite(i,0);
      }
      rWrite(0x08+i,chan[i].freqL);
      rWrite(0x10+(i>>1),chan[i&2].freqH|(chan[1+(i&2)].freqH<<4));
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  rWrite(0x14,(chan[0].psgMode&1)|
              ((chan[1].psgMode&1)<<1)|
              ((chan[2].psgMode&1)<<2)|
              ((chan[3].psgMode&1)<<3)|
              ((chan[4].psgMode&1)<<4)|
              ((chan[5].psgMode&1)<<5)
  );

  rWrite(0x15,((chan[0].psgMode&2)>>1)|
              (chan[1].psgMode&2)|
              ((chan[2].psgMode&2)<<1)|
              ((chan[3].psgMode&2)<<2)|
              ((chan[4].psgMode&2)<<3)|
              ((chan[5].psgMode&2)<<4)
  );

  if (ayEnvSlide!=0) {
    ayEnvSlideLow+=ayEnvSlide;
    while (ayEnvSlideLow>7) {
      ayEnvSlideLow-=8;
      if (ayEnvPeriod<0xffff) {
        ayEnvPeriod++;
        rWrite(0x0b,ayEnvPeriod);
        rWrite(0x0c,ayEnvPeriod>>8);
      }
    }
    while (ayEnvSlideLow<-7) {
      ayEnvSlideLow+=8;
      if (ayEnvPeriod>0) {
        ayEnvPeriod--;
        rWrite(0x0b,ayEnvPeriod);
        rWrite(0x0c,ayEnvPeriod>>8);
      }
    }
  }
}

int DivPlatformSAA1099::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      chan[c.chan].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(ins);
      if (isMuted[c.chan]) {
        rWrite(c.chan,0);
      } else {
        rWrite(c.chan,(chan[c.chan].vol&15));
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.hasVol) {
        chan[c.chan].outVol=c.value;
      }
      if (isMuted[c.chan]) {
        rWrite(c.chan,0);
      } else {
        if (chan[c.chan].active) rWrite(c.chan,(chan[c.chan].vol&15));
      }
      break;
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value2/12.0f)));
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
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_STD_NOISE_FREQ:
      rWrite(0x06,31-c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      ayEnvMode=c.value>>4;
      rWrite(0x0d,ayEnvMode);
      if (c.value&15) {
        chan[c.chan].psgMode|=4;
      } else {
        chan[c.chan].psgMode&=~4;
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      ayEnvPeriod&=0xff00;
      ayEnvPeriod|=c.value;
      rWrite(0x0b,ayEnvPeriod);
      rWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      ayEnvPeriod&=0xff;
      ayEnvPeriod|=c.value<<8;
      rWrite(0x0b,ayEnvPeriod);
      rWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      ayEnvSlide=c.value;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformSAA1099::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(ch,0);
  } else {
    rWrite(ch,(chan[ch].outVol&15));
  }
}

void DivPlatformSAA1099::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
  }
  rWrite(0x0b,ayEnvPeriod);
  rWrite(0x0c,ayEnvPeriod>>8);
  rWrite(0x0d,ayEnvMode);
}

void DivPlatformSAA1099::reset() {
  while (!writes.empty()) writes.pop();
  saa=saa1099_device();
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformSAA1099::Channel();
    chan[i].vol=0x0f;
  }

  lastBusy=60;
  dacMode=0;
  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;
  ayEnvPeriod=0;
  ayEnvMode=0;
  ayEnvSlide=0;
  ayEnvSlideLow=0;

  delay=0;

  extMode=false;

  rWrite(0x1c,1);
}

bool DivPlatformSAA1099::isStereo() {
  return true;
}

bool DivPlatformSAA1099::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformSAA1099::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSAA1099::setPAL(bool pal) {
  if (pal) {
    rate=250000;
  } else {
    rate=250000;
  }
}

int DivPlatformSAA1099::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
  }
  setPAL(pal);
  ayBufLen=65536;
  for (int i=0; i<2; i++) ayBuf[i]=new short[ayBufLen];
  reset();
  return 3;
}

void DivPlatformSAA1099::quit() {
  for (int i=0; i<2; i++) delete[] ayBuf[i];
}