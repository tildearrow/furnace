#include "ym2610.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "ym2610shared.h"

#define FM_FREQ_BASE 622.0f
#define PSG_FREQ_BASE 7640.0f

static unsigned char konOffs[4]={
  1, 2, 5, 6
};

void DivPlatformYM2610::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      if (--delay<1) {
        QueuedWrite& w=writes.front();
        fm->write(0x0+((w.addr>>8)<<1),w.addr);
        fm->write(0x1+((w.addr>>8)<<1),w.val);
        writes.pop();
        delay=4;
      }
    }
    
    fm->generate(&fmout);

    os[0]=fmout.data[0]+(fmout.data[2]>>1);
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=fmout.data[1]+(fmout.data[2]>>1);
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformYM2610::tick() {
  // PSG
  for (int i=4; i<7; i++) {
    chan[i].std.next();
    if (chan[i].std.hadVol) {
      chan[i].outVol=chan[i].std.vol-(15-chan[i].vol);
      if (chan[i].outVol<0) chan[i].outVol=0;
      rWrite(0x04+i,(chan[i].outVol&15)|((chan[i].psgMode&4)<<2));
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
      DivInstrument* ins=parent->getIns(chan[i].ins);
      chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE-chan[i].pitch))/ONE_SEMITONE;
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
        //rWrite(16+i*5+1,((chan[i].duty&3)<<6)|(63-(ins->gb.soundLen&63)));
        //rWrite(16+i*5+2,((chan[i].vol<<4))|(ins->gb.envLen&7)|((ins->gb.envDir&1)<<3));
      }
      if (chan[i].keyOff) {
        rWrite(0x04+i,0);
      }
      rWrite((i-4)<<1,chan[i].freq&0xff);
      rWrite(1+((i-4)<<1),chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  rWrite(0x07,
         ~((chan[4].psgMode&1)|
         ((chan[5].psgMode&1)<<1)|
         ((chan[6].psgMode&1)<<2)|
         ((chan[4].psgMode&2)<<2)|
         ((chan[5].psgMode&2)<<3)|
         ((chan[6].psgMode&2)<<4)));
  
  // FM
  for (int i=0; i<4; i++) {
    if (i==1 && extMode) continue;
    if (chan[i].keyOn || chan[i].keyOff) {
      writes.emplace(0x28,0x00|konOffs[i]);
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      writes.emplace(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<4; i++) {
    if (i==1 && extMode) continue;
    if (chan[i].freqChanged) {
      chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE+chan[i].pitch))/ONE_SEMITONE;
      int freqt=toFreq(chan[i].freq);
      writes.emplace(chanOffs[i]+0xa4,freqt>>8);
      writes.emplace(chanOffs[i]+0xa0,freqt&0xff);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      writes.emplace(0x28,0xf0|konOffs[i]);
      chan[i].keyOn=false;
    }
  }
}

int DivPlatformYM2610::octave(int freq) {
  if (freq>=FM_FREQ_BASE*128) {
    return 128;
  } else if (freq>=FM_FREQ_BASE*64) {
    return 64;
  } else if (freq>=FM_FREQ_BASE*32) {
    return 32;
  } else if (freq>=FM_FREQ_BASE*16) {
    return 16;
  } else if (freq>=FM_FREQ_BASE*8) {
    return 8;
  } else if (freq>=FM_FREQ_BASE*4) {
    return 4;
  } else if (freq>=FM_FREQ_BASE*2) {
    return 2;
  } else {
    return 1;
  }
  return 1;
}

int DivPlatformYM2610::toFreq(int freq) {
  if (freq>=FM_FREQ_BASE*128) {
    return 0x3800|((freq>>7)&0x7ff);
  } else if (freq>=FM_FREQ_BASE*64) {
    return 0x3000|((freq>>6)&0x7ff);
  } else if (freq>=FM_FREQ_BASE*32) {
    return 0x2800|((freq>>5)&0x7ff);
  } else if (freq>=FM_FREQ_BASE*16) {
    return 0x2000|((freq>>4)&0x7ff);
  } else if (freq>=FM_FREQ_BASE*8) {
    return 0x1800|((freq>>3)&0x7ff);
  } else if (freq>=FM_FREQ_BASE*4) {
    return 0x1000|((freq>>2)&0x7ff);
  } else if (freq>=FM_FREQ_BASE*2) {
    return 0x800|((freq>>1)&0x7ff);
  } else {
    return freq&0x7ff;
  }
}

int DivPlatformYM2610::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan>6) { // ADPCM
        if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
          writes.emplace(0x100,0x80|(1<<(c.chan-7)));
          writes.emplace(0x110+c.chan-7,0);
          writes.emplace(0x118+c.chan-7,0);
          writes.emplace(0x120+c.chan-7,0);
          writes.emplace(0x128+c.chan-7,0);
          break;
        }
        DivSample* s=parent->song.sample[12*sampleBank+c.value%12];
        writes.emplace(0x110+c.chan-7,(s->rendOff>>8)&0xff);
        writes.emplace(0x118+c.chan-7,s->rendOff>>16);
        int end=s->rendOff+s->adpcmRendLength-1;
        writes.emplace(0x120+c.chan-7,(end>>8)&0xff);
        writes.emplace(0x128+c.chan-7,end>>16);
        writes.emplace(0x108+(c.chan-7),(chan[c.chan].pan<<6)|chan[c.chan].vol);
        writes.emplace(0x100,0x00|(1<<(c.chan-7)));
        break;
      }
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);

      if (c.chan>3) { // PSG
        chan[c.chan].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].active=true;
        chan[c.chan].keyOn=true;
        chan[c.chan].std.init(ins);
        rWrite(0x04+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
        break;
      }
      
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=ins->fm.op[i];
        if (isOutput[ins->fm.alg][i]) {
          if (!chan[c.chan].active || chan[c.chan].insChanged) {
            rWrite(baseAddr+0x40,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
          }
        } else {
          if (chan[c.chan].insChanged) {
            rWrite(baseAddr+0x40,op.tl);
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+0x60,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+0x70,op.d2r&31);
          rWrite(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+0x90,op.ssgEnv&15);
        }
      }
      if (chan[c.chan].insChanged) {
        rWrite(chanOffs[c.chan]+0xb0,(ins->fm.alg&7)|(ins->fm.fb<<3));
        rWrite(chanOffs[c.chan]+0xb4,(chan[c.chan].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      }
      chan[c.chan].insChanged=false;

      chan[c.chan].baseFreq=FM_FREQ_BASE*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan>6) {
        writes.emplace(0x100,0x80|(1<<(c.chan-7)));
        break;
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.chan>6) { // ADPCM
        writes.emplace(0x108+(c.chan-7),(chan[c.chan].pan<<6)|chan[c.chan].vol);
        break;
      }
      if (c.chan>3) { // PSG
        if (!chan[c.chan].std.hasVol) {
          chan[c.chan].outVol=c.value;
        }
        rWrite(0x04+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
        break;
      }
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=ins->fm.op[i];
        if (isOutput[ins->fm.alg][i]) {
          rWrite(baseAddr+0x40,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
        } else {
          rWrite(baseAddr+0x40,op.tl);
        }
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
    case DIV_CMD_PANNING: {
      switch (c.value) {
        case 0x01:
          chan[c.chan].pan=1;
          break;
        case 0x10:
          chan[c.chan].pan=2;
          break;
        default:
          chan[c.chan].pan=3;
          break;
      }
      if (c.chan>6) {
        writes.emplace(0x108+(c.chan-7),(chan[c.chan].pan<<6)|chan[c.chan].vol);
        break;
      }
      if (c.chan>3) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      rWrite(chanOffs[c.chan]+0xb4,(chan[c.chan].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (c.chan>3) { // PSG
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

      int destFreq=FM_FREQ_BASE*pow(2.0f,((float)c.value2/12.0f));
      int newFreq;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*octave(chan[c.chan].baseFreq);
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*octave(chan[c.chan].baseFreq);
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      if (!chan[c.chan].portaPause) {
        if (octave(chan[c.chan].baseFreq)!=octave(newFreq)) {
          chan[c.chan].portaPause=true;
          break;
        }
      }
      chan[c.chan].baseFreq=newFreq;
      chan[c.chan].portaPause=false;
      chan[c.chan].freqChanged=true;
      if (return2) return 2;
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      iface.sampleBank=sampleBank;
      break;
    case DIV_CMD_LEGATO: {
      if (c.chan>3) { // PSG
        chan[c.chan].baseFreq=round(PSG_FREQ_BASE/pow(2.0f,((float)c.value/12.0f)));
      } else {
        chan[c.chan].baseFreq=FM_FREQ_BASE*pow(2.0f,((float)c.value/12.0f));
      }
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      rWrite(0x22,(c.value&7)|((c.value>>4)<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      if (c.chan>3) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
      rWrite(baseAddr+0x30,(c.value2&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>3) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (isOutput[ins->fm.alg][c.value]) {
        rWrite(baseAddr+0x40,127-(((127-c.value2)*(chan[c.chan].vol&0x7f))/127));
      } else {
        rWrite(baseAddr+0x40,c.value2);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.chan>3) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator op=ins->fm.op[i];
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+0x50,(c.value2&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+0x50,(c.value2&31)|(op.rs<<6));
      }
      
      break;
    }
    case DIV_CMD_STD_NOISE_FREQ:
      if (c.chan<4 || c.chan>6) break;
      rWrite(0x06,31-c.value);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      if (c.chan<4 || c.chan>6) break;
      rWrite(0x0d,c.value>>4);
      if (c.value&15) {
        chan[c.chan].psgMode|=4;
      } else {
        chan[c.chan].psgMode&=~4;
      }
      rWrite(0x04+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      if (c.chan<4 || c.chan>6) break;
      ayEnvPeriod&=0xff00;
      ayEnvPeriod|=c.value;
      writes.emplace(0x0b,ayEnvPeriod);
      writes.emplace(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      if (c.chan<4 || c.chan>6) break;
      ayEnvPeriod&=0xff;
      ayEnvPeriod|=c.value<<8;
      writes.emplace(0x0b,ayEnvPeriod);
      writes.emplace(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      if (c.chan>6) return 31;
      if (c.chan>3) return 15;
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      if (c.chan>3) {
        chan[c.chan].std.init(parent->getIns(chan[c.chan].ins));
        chan[c.chan].inPorta=c.value;
      }
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformYM2610::reset() {
  fm->reset();
  for (int i=0; i<13; i++) {
    chan[i]=DivPlatformYM2610::Channel();
  }
  for (int i=0; i<4; i++) {
    chan[i].vol=0x7f;
  }
  for (int i=4; i<7; i++) {
    chan[i].vol=0x0f;
  }
  for (int i=7; i<13; i++) {
    chan[i].vol=0x1f;
  }

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  dacMode=0;
  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;
  sampleBank=0;
  ayEnvPeriod=0;

  delay=0;

  extMode=false;

  // LFO
  writes.emplace(0x22,0x08);

  // PCM volume
  writes.emplace(0x101,0x3f);
}

bool DivPlatformYM2610::isStereo() {
  return true;
}

bool DivPlatformYM2610::keyOffAffectsArp(int ch) {
  return (ch>3);
}

int DivPlatformYM2610::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  if (pal) {
    rate=500000;
  } else {
    rate=500000;
  }
  iface.parent=parent;
  iface.sampleBank=0;
  fm=new ymfm::ym2610(iface);
  reset();
  return 10;
}
