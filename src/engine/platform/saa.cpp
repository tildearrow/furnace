#include "saa.h"
#include "../engine.h"
#include "sound/saa1099.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define PSG_FREQ_BASE 122240.0f

void DivPlatformSAA1099::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (saaBufLen<len) {
    saaBufLen=len;
    for (int i=0; i<2; i++) {
      delete[] saaBuf[i];
      saaBuf[i]=new short[saaBufLen];
    }
  }
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    saa.control_w(w.addr);
    saa.data_w(w.val);
    writes.pop();
  }
  saa.sound_stream_update(saaBuf,len);
  for (size_t i=0; i<len; i++) {
    bufL[i+start]=saaBuf[0][i];
    bufR[i+start]=saaBuf[1][i];
  }
}

inline unsigned char applyPan(unsigned char vol, unsigned char pan) {
  return ((vol*(pan>>4))/15)|(((vol*(pan&15))/15)<<4);
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
        rWrite(i,applyPan(chan[i].outVol&15,chan[i].pan));
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
      saaNoise[i/3]=chan[i].std.duty&3;
      rWrite(0x16,saaNoise[0]|(saaNoise[1]<<4));
    }
    if (chan[i].std.hadWave) {
      chan[i].psgMode=chan[i].std.wave&3;
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
      }
      if (chan[i].keyOff) {
        rWrite(i,0);
      }
      rWrite(0x08+i,chan[i].freqL);
      rWrite(0x10+(i>>1),chan[i&6].freqH|(chan[1+(i&6)].freqH<<4));
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
        rWrite(c.chan,applyPan(chan[c.chan].vol&15,chan[c.chan].pan));
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
        if (chan[c.chan].active) rWrite(c.chan,applyPan(chan[c.chan].vol&15,chan[c.chan].pan));
      }
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
        chan[c.chan].baseFreq+=c.value*(8-chan[c.chan].freqH);
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value*(8-chan[c.chan].freqH);
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
      chan[c.chan].pan=c.value;
      if (isMuted[c.chan]) {
        rWrite(c.chan,0);
      } else {
        if (chan[c.chan].active) rWrite(c.chan,applyPan(chan[c.chan].vol&15,chan[c.chan].pan));
      }
      break;
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      chan[c.chan].psgMode=(c.value&1)|((c.value&16)>>3);
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      saaNoise[c.chan/3]=(c.value&1)|((c.value&16)>>3);
      rWrite(0x16,saaNoise[0]|(saaNoise[1]<<4));
      break;
    case DIV_CMD_SAA_ENVELOPE:
      saaEnv[c.chan/3]=c.value;
      rWrite(0x18+(c.chan/3),c.value);
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
    if (chan[ch].active) rWrite(ch,applyPan(chan[ch].outVol&15,chan[ch].pan));
  }
}

void DivPlatformSAA1099::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
  rWrite(0x18,saaEnv[0]);
  rWrite(0x19,saaEnv[1]);
  rWrite(0x16,saaNoise[0]|(saaNoise[1]<<4));
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
  saaEnv[0]=0;
  saaEnv[1]=0;
  saaNoise[0]=0;
  saaNoise[1]=0;

  delay=0;

  extMode=false;

  rWrite(0x1c,1);
}

bool DivPlatformSAA1099::isStereo() {
  return true;
}

int DivPlatformSAA1099::getPortaFloor(int ch) {
  return 12;
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
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
  }
  setPAL(pal);
  saaBufLen=65536;
  for (int i=0; i<2; i++) saaBuf[i]=new short[saaBufLen];
  reset();
  return 3;
}

void DivPlatformSAA1099::quit() {
  for (int i=0; i<2; i++) delete[] saaBuf[i];
}
