#include "ym2610.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "ym2610shared.h"

#include "fmshared_OPN.h"

static unsigned char konOffs[4]={
  1, 2, 5, 6
};

#define CHIP_DIVIDER 32

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
      chan[i].outVol=chan[i].std.vol-(15-(chan[i].vol&15));
      if (chan[i].outVol<0) chan[i].outVol=0;
      if (isMuted[i]) {
        rWrite(0x04+i,0);
      } else {
        rWrite(0x04+i,(chan[i].outVol&15)|((chan[i].psgMode&4)<<2));
      }
    }
    if (chan[i].std.hadArp) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arpMode) {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].std.arp);
        } else {
          chan[i].baseFreq=NOTE_PERIODIC(chan[i].note+chan[i].std.arp);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arpMode && chan[i].std.finishedArp) {
        chan[i].baseFreq=NOTE_PERIODIC(chan[i].note);
        chan[i].freqChanged=true;
      }
    }
    if (chan[i].std.hadDuty) {
      ayNoiseFreq=31-chan[i].std.duty;
      rWrite(0x06,ayNoiseFreq);
    }
    if (chan[i].std.hadWave) {
      chan[i].psgMode=(chan[i].std.wave+1)&7;
      if (isMuted[i]) {
        rWrite(0x04+i,0);
      } else {
        rWrite(0x04+i,(chan[i].outVol&15)|((chan[i].psgMode&4)<<2));
      }
    }
    if (chan[i].std.hadEx2) {
      ayEnvMode=chan[i].std.ex2;
      rWrite(0x0d,ayEnvMode);
    }
    if (chan[i].std.hadEx3) {
      chan[i].autoEnvNum=chan[i].std.ex3;
      chan[i].freqChanged=true;
      if (!chan[i].std.willAlg) chan[i].autoEnvDen=1;
    }
    if (chan[i].std.hadAlg) {
      chan[i].autoEnvDen=chan[i].std.alg;
      chan[i].freqChanged=true;
      if (!chan[i].std.willEx3) chan[i].autoEnvNum=1;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,true);
      if (chan[i].freq>4095) chan[i].freq=4095;
      if (chan[i].keyOn) {
      }
      if (chan[i].keyOff) {
        rWrite(0x04+i,0);
      }
      rWrite((i-4)<<1,chan[i].freq&0xff);
      rWrite(1+((i-4)<<1),chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      if (chan[i].freqChanged && chan[i].autoEnvNum>0 && chan[i].autoEnvDen>0) {
        ayEnvPeriod=(chan[i].freq*chan[i].autoEnvDen/chan[i].autoEnvNum)>>4;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
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

  if (ayEnvSlide!=0) {
    ayEnvSlideLow+=ayEnvSlide;
    while (ayEnvSlideLow>7) {
      ayEnvSlideLow-=8;
      if (ayEnvPeriod<0xffff) {
        ayEnvPeriod++;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
    }
    while (ayEnvSlideLow<-7) {
      ayEnvSlideLow+=8;
      if (ayEnvPeriod>0) {
        ayEnvPeriod--;
        immWrite(0x0b,ayEnvPeriod);
        immWrite(0x0c,ayEnvPeriod>>8);
      }
    }
  }
  
  // FM
  for (int i=0; i<4; i++) {
    if (i==1 && extMode) continue;
    chan[i].std.next();

    if (chan[i].std.hadVol) {
      chan[i].outVol=(chan[i].vol*chan[i].std.vol)/127;
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
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

    if (chan[i].std.hadAlg) {
      chan[i].state.alg=chan[i].std.alg;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    }
    if (chan[i].std.hadFb) {
      chan[i].state.fb=chan[i].std.fb;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    }
    if (chan[i].std.hadFms) {
      chan[i].state.fms=chan[i].std.fms;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].std.hadAms) {
      chan[i].state.ams=chan[i].std.ams;
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      DivMacroInt::IntOp& m=chan[i].std.op[j];
      if (m.hadAm) {
        op.am=m.am;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.hadAr) {
        op.ar=m.ar;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      if (m.hadDr) {
        op.dr=m.dr;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.hadMult) {
        op.mult=m.mult;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      if (m.hadRr) {
        op.rr=m.rr;
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      if (m.hadSl) {
        op.sl=m.sl;
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      if (m.hadTl) {
        op.tl=127-m.tl;
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
      if (m.hadRs) {
        op.rs=m.rs;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      if (m.hadDt) {
        op.dt=m.dt;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      if (m.hadD2r) {
        op.d2r=m.d2r;
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      }
      if (m.hadSsg) {
        op.ssgEnv=m.ssg;
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
      }
    }

    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(0x28,0x00|konOffs[i]);
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<4; i++) {
    if (i==1 && extMode) continue;
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch);
      int freqt=toFreq(chan[i].freq);
      immWrite(chanOffs[i]+ADDR_FREQH,freqt>>8);
      immWrite(chanOffs[i]+ADDR_FREQ,freqt&0xff);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      immWrite(0x28,0xf0|konOffs[i]);
      chan[i].keyOn=false;
    }
  }
}

int DivPlatformYM2610::octave(int freq) {
  if (freq>=622.0f*128) {
    return 128;
  } else if (freq>=622.0f*64) {
    return 64;
  } else if (freq>=622.0f*32) {
    return 32;
  } else if (freq>=622.0f*16) {
    return 16;
  } else if (freq>=622.0f*8) {
    return 8;
  } else if (freq>=622.0f*4) {
    return 4;
  } else if (freq>=622.0f*2) {
    return 2;
  } else {
    return 1;
  }
  return 1;
}

int DivPlatformYM2610::toFreq(int freq) {
  if (freq>=622.0f*128) {
    return 0x3800|((freq>>7)&0x7ff);
  } else if (freq>=622.0f*64) {
    return 0x3000|((freq>>6)&0x7ff);
  } else if (freq>=622.0f*32) {
    return 0x2800|((freq>>5)&0x7ff);
  } else if (freq>=622.0f*16) {
    return 0x2000|((freq>>4)&0x7ff);
  } else if (freq>=622.0f*8) {
    return 0x1800|((freq>>3)&0x7ff);
  } else if (freq>=622.0f*4) {
    return 0x1000|((freq>>2)&0x7ff);
  } else if (freq>=622.0f*2) {
    return 0x800|((freq>>1)&0x7ff);
  } else {
    return freq&0x7ff;
  }
}

int DivPlatformYM2610::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan>6) { // ADPCM
        if (skipRegisterWrites) break;
        if ((12*sampleBank+c.value%12)>=parent->song.sampleLen) {
          immWrite(0x100,0x80|(1<<(c.chan-7)));
          immWrite(0x110+c.chan-7,0);
          immWrite(0x118+c.chan-7,0);
          immWrite(0x120+c.chan-7,0);
          immWrite(0x128+c.chan-7,0);
          break;
        }
        DivSample* s=parent->song.sample[12*sampleBank+c.value%12];
        immWrite(0x110+c.chan-7,(s->rendOff>>8)&0xff);
        immWrite(0x118+c.chan-7,s->rendOff>>16);
        int end=s->rendOff+s->adpcmRendLength-1;
        immWrite(0x120+c.chan-7,(end>>8)&0xff);
        immWrite(0x128+c.chan-7,end>>16);
        immWrite(0x108+(c.chan-7),isMuted[c.chan]?0:((chan[c.chan].pan<<6)|chan[c.chan].vol));
        immWrite(0x100,0x00|(1<<(c.chan-7)));
        break;
      }
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      chan[c.chan].std.init(ins);
      if (c.chan<4) {
        if (!chan[c.chan].std.willVol) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
      }

      if (c.chan>3) { // PSG
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
        chan[c.chan].active=true;
        chan[c.chan].keyOn=true;
        if (isMuted[c.chan]) {
          rWrite(0x04+c.chan,0);
        } else {
          rWrite(0x04+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
        }
        break;
      }

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }
      
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          if (!chan[c.chan].active || chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
          }
        } else {
          if (chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      }
      if (chan[c.chan].insChanged) {
        rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      }
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan>6) {
        immWrite(0x100,0x80|(1<<(c.chan-7)));
        break;
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      chan[c.chan].std.init(NULL);
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.hasVol) {
        chan[c.chan].outVol=c.value;
      }
      if (c.chan>6) { // ADPCM
        immWrite(0x108+(c.chan-7),isMuted[c.chan]?0:((chan[c.chan].pan<<6)|chan[c.chan].vol));
        break;
      }
      if (c.chan>3) { // PSG
        if (isMuted[c.chan]) {
          rWrite(0x04+c.chan,0);
        } else {
          if (chan[c.chan].active) rWrite(0x04+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
        }
        break;
      }
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
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
        immWrite(0x108+(c.chan-7),isMuted[c.chan]?0:((chan[c.chan].pan<<6)|chan[c.chan].vol));
        break;
      }
      if (c.chan>3) break;
      rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (c.chan>3) { // PSG
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

      int destFreq=NOTE_FREQUENCY(c.value2);
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
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      } else {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
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
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>3) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOutput[chan[c.chan].state.alg][c.value]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.chan>3) break;
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_STD_NOISE_MODE:
      if (c.chan<4 || c.chan>6) break;
      chan[c.chan].psgMode=(c.value+1)&7;
      if (isMuted[c.chan]) {
        rWrite(0x04+c.chan,0);
      } else if (chan[c.chan].active) {
        rWrite(0x04+c.chan,(chan[c.chan].outVol&15)|((chan[c.chan].psgMode&4)<<2));
      }
      break;
    case DIV_CMD_STD_NOISE_FREQ:
      if (c.chan<4 || c.chan>6) break;
      ayNoiseFreq=31-c.value;
      rWrite(0x06,ayNoiseFreq);
      break;
    case DIV_CMD_AY_ENVELOPE_SET:
      if (c.chan<4 || c.chan>6) break;
      ayEnvMode=c.value>>4;
      rWrite(0x0d,ayEnvMode);
      if (c.value&15) {
        chan[c.chan].psgMode|=4;
      } else {
        chan[c.chan].psgMode&=~4;
      }
      if (isMuted[c.chan]) {
        rWrite(0x04+c.chan,0);
      } else {
        rWrite(0x04+c.chan,(chan[c.chan].vol&15)|((chan[c.chan].psgMode&4)<<2));
      }
      break;
    case DIV_CMD_AY_ENVELOPE_LOW:
      if (c.chan<4 || c.chan>6) break;
      ayEnvPeriod&=0xff00;
      ayEnvPeriod|=c.value;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_HIGH:
      if (c.chan<4 || c.chan>6) break;
      ayEnvPeriod&=0xff;
      ayEnvPeriod|=c.value<<8;
      immWrite(0x0b,ayEnvPeriod);
      immWrite(0x0c,ayEnvPeriod>>8);
      break;
    case DIV_CMD_AY_ENVELOPE_SLIDE:
      if (c.chan<4 || c.chan>6) break;
      ayEnvSlide=c.value;
      break;
    case DIV_CMD_AY_AUTO_ENVELOPE:
      if (c.chan<4 || c.chan>6) break;
      chan[c.chan].autoEnvNum=c.value>>4;
      chan[c.chan].autoEnvDen=c.value&15;
      chan[c.chan].freqChanged=true;
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
      }
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

void DivPlatformYM2610::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (ch>6) { // ADPCM
    immWrite(0x108+(ch-7),isMuted[ch]?0:((chan[ch].pan<<6)|chan[ch].vol));
    return;
  }
  if (ch>3) { // PSG
    if (isMuted[ch]) {
      rWrite(0x04+ch,0);
    } else {
      rWrite(0x04+ch,(chan[ch].outVol&15)|((chan[ch].psgMode&4)<<2));
    }
    return;
  }
  // FM
  rWrite(chanOffs[ch]+ADDR_LRAF,(isMuted[ch]?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));
}

void DivPlatformYM2610::forceIns() {
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (isOutput[chan[i].state.alg][j]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
    rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
  }
  for (int i=4; i<13; i++) {
    chan[i].insChanged=true;
  }
  immWrite(0x0b,ayEnvPeriod);
  immWrite(0x0c,ayEnvPeriod>>8);
  immWrite(0x0d,ayEnvMode);
}

void* DivPlatformYM2610::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformYM2610::reset() {
  while (!writes.empty()) writes.pop();
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  fm->reset();
  for (int i=0; i<13; i++) {
    chan[i]=DivPlatformYM2610::Channel();
  }
  for (int i=0; i<4; i++) {
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
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
  ayEnvMode=0;
  ayEnvSlide=0;
  ayEnvSlideLow=0;
  ayNoiseFreq=0;

  delay=0;

  extMode=false;

  // AY noise
  immWrite(0x06,ayNoiseFreq);

  // LFO
  immWrite(0x22,0x08);

  // PCM volume
  immWrite(0x101,0x3f);
}

bool DivPlatformYM2610::isStereo() {
  return true;
}

bool DivPlatformYM2610::keyOffAffectsArp(int ch) {
  return (ch>3);
}

void DivPlatformYM2610::notifyInsChange(int ins) {
  for (int i=0; i<13; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformYM2610::notifyInsDeletion(void* ins) {
  for (int i=4; i<7; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

int DivPlatformYM2610::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<13; i++) {
    isMuted[i]=false;
  }
  chipClock=8000000;
  rate=chipClock/16;
  iface.parent=parent;
  iface.sampleBank=0;
  fm=new ymfm::ym2610(iface);
  reset();
  return 10;
}

void DivPlatformYM2610::quit() {
  delete fm;
}

DivPlatformYM2610::~DivPlatformYM2610() {
}