#include "ay8930.h"
#include "../engine.h"
#include "sound/ay8910.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define PSG_FREQ_BASE 6848.0f

void DivPlatformAY8930::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (ayBufLen<len) {
    ayBufLen=len;
    for (int i=0; i<3; i++) {
      delete[] ayBuf[i];
      ayBuf[i]=new short[ayBufLen];
    }
  }
  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    if (bank!=(w.addr>>4)) {
      bank=w.addr>>4;
      ay->address_w(0x0d);
      ay->data_w(0xa0|(bank<<4)|ayEnvMode[0]);
    }
    ay->address_w(w.addr&15);
    if (w.addr==0x0d) {
      ay->data_w(0xa0|(bank<<4)|(w.val&15));
    } else {
      ay->data_w(w.val);
    }
    writes.pop();
  }
  ay->sound_stream_update(ayBuf,len);
  for (size_t i=0; i<len; i++) {
    bufL[i+start]=ayBuf[0][i]+ayBuf[1][i]+ayBuf[2][i];
  }
}

const unsigned char regPeriodL[3]={
  0x0b, 0x10, 0x12
};

const unsigned char regPeriodH[3]={
  0x0c, 0x11, 0x13
};

const unsigned char regMode[3]={
  0x0d, 0x14, 0x15
};

void DivPlatformAY8930::tick() {
  // PSG
  for (int i=0; i<3; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=chan[i].std.vol-(31-(chan[i].vol&31));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(0x08+i,0);
      } else {
        rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].psgMode&4)<<3));
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
      rWrite(0x06,chan[i].std.duty);
    }
    if (chan[i].std.hadWave) {
      chan[i].psgMode=(chan[i].std.wave+1)&7;
      if (isMuted[i]) {
        rWrite(0x08+i,0);
      } else {
        rWrite(0x08+i,(chan[i].outVol&31)|((chan[i].psgMode&4)<<3));
      }
    }
    if (chan[i].std.hadEx1) { // duty
      rWrite(0x16+i,chan[i].std.ex1);
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
        if (chan[i].insChanged) {
          if (!chan[i].std.willEx1) immWrite(0x16+i,chan[i].duty);
          chan[i].insChanged=false;
        }
      }
      if (chan[i].keyOff) {
        rWrite(0x08+i,0);
      }
      rWrite((i)<<1,chan[i].freq&0xff);
      rWrite(1+((i)<<1),chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
        ayEnvPeriod[i]=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>4;
        immWrite(regPeriodL[i],ayEnvPeriod[i]);
        immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
      }
      chan[i].freqChanged=false;
    }

    if (ayEnvSlide[i]!=0) {
      ayEnvSlideLow[i]+=ayEnvSlide[i];
      while (ayEnvSlideLow[i]>7) {
        ayEnvSlideLow[i]-=8;
        if (ayEnvPeriod[i]<0xffff) {
          ayEnvPeriod[i]++;
          immWrite(regPeriodL[i],ayEnvPeriod[i]);
          immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
        }
      }
      while (ayEnvSlideLow[i]<-7) {
        ayEnvSlideLow[i]+=8;
        if (ayEnvPeriod[i]>0) {
          ayEnvPeriod[i]--;
          immWrite(regPeriodL[i],ayEnvPeriod[i]);
          immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
        }
      }
    }
  }

  rWrite(0x07,
         ~((chan[0].psgMode&1)|
         ((chan[1].psgMode&1)<<1)|
         ((chan[2].psgMode&1)<<2)|
         ((chan[0].psgMode&2)<<2)|
         ((chan[1].psgMode&2)<<3)|
         ((chan[2].psgMode&2)<<4)));
  
  for (int i=0; i<32; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }
}

int DivPlatformAY8930::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].std.init(ins);
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0);
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].psgMode&4)<<3));
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
        rWrite(0x08+c.chan,0);
      } else {
        if (chan[c.chan].active) rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].psgMode&4)<<3));
      }
      break;
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return chan[c.chan].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
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
    case DIV_CMD_STD_NOISE_MODE:
      if (c.value<0x10) {
        chan[c.chan].psgMode=(c.value+1)&7;
        if (isMuted[c.chan]) {
          rWrite(0x08+c.chan,0);
        } else if (chan[c.chan].active) {
          rWrite(0x08+c.chan,(chan[c.chan].outVol&31)|((chan[c.chan].psgMode&4)<<3));
        }
      } else {
        chan[c.chan].duty=c.value&15;
        immWrite(0x16,chan[c.chan].duty);
      }
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      rWrite(0x06,c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      ayEnvMode[c.chan]=c.value>>4;
      rWrite(regMode[c.chan],ayEnvMode[c.chan]);
      if (c.value&15) {
        chan[c.chan].psgMode|=4;
      } else {
        chan[c.chan].psgMode&=~4;
      }
      if (isMuted[c.chan]) {
        rWrite(0x08+c.chan,0);
      } else {
        rWrite(0x08+c.chan,(chan[c.chan].vol&31)|((chan[c.chan].psgMode&4)<<3));
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      ayEnvPeriod[c.chan]&=0xff00;
      ayEnvPeriod[c.chan]|=c.value;
      immWrite(regPeriodL[c.chan],ayEnvPeriod[c.chan]);
      immWrite(regPeriodH[c.chan],ayEnvPeriod[c.chan]>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      ayEnvPeriod[c.chan]&=0xff;
      ayEnvPeriod[c.chan]|=c.value<<8;
      immWrite(regPeriodL[c.chan],ayEnvPeriod[c.chan]);
      immWrite(regPeriodH[c.chan],ayEnvPeriod[c.chan]>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      ayEnvSlide[c.chan]=c.value;
      break;
    case DIV_CMD_AY_NOISE_MASK_AND:
      ayNoiseAnd=c.value;
      immWrite(0x19,ayNoiseAnd);
      break;
    case DIV_CMD_AY_NOISE_MASK_OR:
      ayNoiseOr=c.value;
      immWrite(0x1a,ayNoiseOr);
      break;
    case DIV_CMD_AY_AUTO_ENVELOPE:
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 31;
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

void DivPlatformAY8930::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(0x08+ch,0);
  } else {
    rWrite(0x08+ch,(chan[ch].outVol&31)|((chan[ch].psgMode&4)<<3));
  }
}

void DivPlatformAY8930::forceIns() {
  for (int i=0; i<3; i++) {
    chan[i].insChanged=true;
    immWrite(regPeriodL[i],ayEnvPeriod[i]);
    immWrite(regPeriodH[i],ayEnvPeriod[i]>>8);
    immWrite(regMode[i],ayEnvMode[i]);
  }
}

void DivPlatformAY8930::reset() {
  while (!writes.empty()) writes.pop();
  ay->device_reset();
  for (int i=0; i<3; i++) {
    chan[i]=DivPlatformAY8930::Channel();
    chan[i].vol=31;
    ayEnvPeriod[i]=0;
    ayEnvMode[i]=0;
    ayEnvSlide[i]=0;
    ayEnvSlideLow[i]=0;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }

  for (int i=0; i<32; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  ayNoiseAnd=0;
  ayNoiseOr=0;
  delay=0;

  extMode=false;
  bank=false;

  immWrite(0x0d,0xa0);
}

bool DivPlatformAY8930::isStereo() {
  return false;
}

bool DivPlatformAY8930::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformAY8930::notifyInsDeletion(void* ins) {
  for (int i=0; i<3; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformAY8930::setPAL(bool pal) {
  if (pal) {
    rate=221681;
  } else {
    rate=223722;
  }
}

int DivPlatformAY8930::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<3; i++) {
    isMuted[i]=false;
  }
  setPAL(pal);
  ay=new ay8930_device(rate);
  ay->device_start();
  ayBufLen=65536;
  for (int i=0; i<3; i++) ayBuf[i]=new short[ayBufLen];
  reset();
  return 3;
}

void DivPlatformAY8930::quit() {
  for (int i=0; i<3; i++) delete[] ayBuf[i];
  delete ay;
}
