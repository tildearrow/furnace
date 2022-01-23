#include "genesis.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "genesisshared.h"

static unsigned char konOffs[6]={
  0, 1, 2, 4, 5, 6
};

void DivPlatformGenesis::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static short o[2];
  static int os[2];

  for (size_t h=start; h<start+len; h++) {
    if (dacMode && dacSample!=-1) {
      dacPeriod-=6;
      if (dacPeriod<1) {
        DivSample* s=parent->song.sample[dacSample];
        if (!isMuted[5]) {
          if (s->depth==8) {
            immWrite(0x2a,(unsigned char)s->rendData[dacPos]+0x80);
          } else {
            immWrite(0x2a,((unsigned short)s->rendData[dacPos]+0x8000)>>8);
          }
        }
        if (++dacPos>=s->rendLength) {
          if (s->loopStart>=0 && s->loopStart<=(int)s->rendLength) {
            dacPos=s->loopStart;
          } else {
            dacSample=-1;
          }
        }
        dacPeriod+=MAX(40,dacRate);
      }
    }
  
    os[0]=0; os[1]=0;
    for (int i=0; i<6; i++) {
      if (!writes.empty() && --delay<0) {
        delay=0;
        QueuedWrite& w=writes.front();
        if (w.addrOrVal) {
          OPN2_Write(&fm,0x1+((w.addr>>8)<<1),w.val);
          //printf("write: %x = %.2x\n",w.addr,w.val);
          lastBusy=0;
          writes.pop();
        } else {
          lastBusy++;
          if (fm.write_busy==0) {
            //printf("busycounter: %d\n",lastBusy);
            OPN2_Write(&fm,0x0+((w.addr>>8)<<1),w.addr);
            w.addrOrVal=true;
          }
        }
      }
      
      OPN2_Clock(&fm,o); os[0]+=o[0]; os[1]+=o[1];
      //OPN2_Write(&fm,0,0);
      }
    
    psgClocks+=psg.rate;
    while (psgClocks>=rate) {
      psgOut=(psg.acquireOne()*3)>>3;
      psgClocks-=rate;
    }

    os[0]=(os[0]<<5)+psgOut;
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=(os[1]<<5)+psgOut;
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformGenesis::tick() {
  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    chan[i].std.next();
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
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].vol&0x7f))/127));
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

  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch);
      int freqt=toFreq(chan[i].freq);
      immWrite(chanOffs[i]+ADDR_FREQH,freqt>>8);
      immWrite(chanOffs[i]+ADDR_FREQ,freqt&0xff);
      if (chan[i].furnaceDac) {
        dacRate=(1280000*1.25)/chan[i].baseFreq;
      }
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      immWrite(0x28,0xf0|konOffs[i]);
      chan[i].keyOn=false;
    }
  }

  psg.tick();
}

int DivPlatformGenesis::octave(int freq) {
  if (freq>=82432) {
    return 128;
  } else if (freq>=41216) {
    return 64;
  } else if (freq>=20608) {
    return 32;
  } else if (freq>=10304) {
    return 16;
  } else if (freq>=5152) {
    return 8;
  } else if (freq>=2576) {
    return 4;
  } else if (freq>=1288) {
    return 2;
  } else {
    return 1;
  }
  return 1;
}

int DivPlatformGenesis::toFreq(int freq) {
  if (freq>=82432) {
    return 0x3800|((freq>>7)&0x7ff);
  } else if (freq>=41216) {
    return 0x3000|((freq>>6)&0x7ff);
  } else if (freq>=20608) {
    return 0x2800|((freq>>5)&0x7ff);
  } else if (freq>=10304) {
    return 0x2000|((freq>>4)&0x7ff);
  } else if (freq>=5152) {
    return 0x1800|((freq>>3)&0x7ff);
  } else if (freq>=2576) {
    return 0x1000|((freq>>2)&0x7ff);
  } else if (freq>=1288) {
    return 0x800|((freq>>1)&0x7ff);
  } else {
    return freq&0x7ff;
  }
}

void DivPlatformGenesis::muteChannel(int ch, bool mute) {
  if (ch>5) {
    psg.muteChannel(ch-6,mute);
    return;
  }
  isMuted[ch]=mute;
  rWrite(chanOffs[ch]+ADDR_LRAF,(isMuted[ch]?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));
}

int DivPlatformGenesis::dispatch(DivCommand c) {
  if (c.chan>5) {
    c.chan-=6;
    return psg.dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      if (c.chan==5) {
        if (ins->type==DIV_INS_AMIGA) {
          dacMode=1;
          rWrite(0x2b,1<<7);
        } else if (chan[c.chan].furnaceDac) {
          dacMode=0;
          rWrite(0x2b,0<<7);
        }
      }
      if (c.chan==5 && dacMode) {
        if (skipRegisterWrites) break;
        if (ins->type==DIV_INS_AMIGA) { // Furnace mode
          dacSample=ins->amiga.initSample;
          if (dacSample<0 || dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            break;
          }
          dacPos=0;
          dacPeriod=0;
          chan[c.chan].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
          chan[c.chan].freqChanged=true;
          chan[c.chan].furnaceDac=true;
        } else { // compatible mode
          dacSample=12*sampleBank+c.value%12;
          if (dacSample>=parent->song.sampleLen) {
            dacSample=-1;
            break;
          }
          dacPos=0;
          dacPeriod=0;
          dacRate=1280000/parent->song.sample[dacSample]->rate;
          chan[c.chan].furnaceDac=false;
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
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
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
        chan[c.chan].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      chan[c.chan].std.init(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan==5) {
        dacSample=-1;
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isOutput[chan[c.chan].state.alg][i]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
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
      rWrite(chanOffs[c.chan]+ADDR_LRAF,(isMuted[c.chan]?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=644.0f*pow(2.0f,((float)c.value2/12.0f));
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
    case DIV_CMD_SAMPLE_MODE: {
      dacMode=c.value;
      rWrite(0x2b,c.value<<7);
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      lfoValue=(c.value&7)|((c.value>>4)<<3);
      rWrite(0x22,lfoValue);
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOutput[chan[c.chan].state.alg][c.value]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
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
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformGenesis::forceIns() {
  for (int i=0; i<6; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (isOutput[chan[i].state.alg][j]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].vol&0x7f))/127));
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
  if (dacMode) {
    rWrite(0x2b,0x80);
  }
  immWrite(0x22,lfoValue);
}

void DivPlatformGenesis::reset() {
  while (!writes.empty()) writes.pop();
  OPN2_Reset(&fm);
  for (int i=0; i<10; i++) {
    chan[i]=DivPlatformGenesis::Channel();
    chan[i].vol=0x7f;
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
  lfoValue=8;

  extMode=false;

  // LFO
  immWrite(0x22,lfoValue);
  
  delay=0;
  
  // PSG
  psg.reset();
  psgClocks=0;
  psgOut=0;
}

bool DivPlatformGenesis::isStereo() {
  return true;
}

bool DivPlatformGenesis::keyOffAffectsArp(int ch) {
  return (ch>5);
}

bool DivPlatformGenesis::keyOffAffectsPorta(int ch) {
  return (ch>5);
}

void DivPlatformGenesis::notifyInsChange(int ins) {
  for (int i=0; i<10; i++) {
    if (i>5) {
      psg.notifyInsChange(ins);
    } else if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGenesis::notifyInsDeletion(void* ins) {
  psg.notifyInsDeletion(ins);
}

void DivPlatformGenesis::setPAL(bool pal) {
  if (pal) {
    rate=211125;
  } else {
    rate=213068;
  }
}

int DivPlatformGenesis::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<10; i++) {
    isMuted[i]=false;
  }
  setPAL(pal);
  // PSG
  psg.init(p,4,sugRate,pal);

  reset();
  return 10;
}

void DivPlatformGenesis::quit() {
  psg.quit();
}

DivPlatformGenesis::~DivPlatformGenesis() {
}
