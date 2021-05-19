#include "genesis.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "genesisshared.h"

void DivPlatformGenesis::acquire(int& l, int& r) {
  static short o[2];

  if (dacMode && dacSample!=-1) {
    if (--dacPeriod<1) {
      DivSample* s=parent->song.sample[dacSample];
      if (s->depth==8) {
        writes.emplace(0x2a,(unsigned char)s->rendData[dacPos++]+0x80);
      } else {
        writes.emplace(0x2a,((unsigned short)s->rendData[dacPos++]+0x8000)>>8);
      }
      if (dacPos>=s->rendLength) {
        dacSample=-1;
      }
      dacPeriod=dacRate;
    }
  }

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
  OPN2_Clock(&fm,o);
  //OPN2_Write(&fm,0,0);
  
  psgClocks+=223722;
  if (psgClocks>=rate) {
    psg.acquire(psgOut,psgOut);
    psgClocks-=rate;
    psgOut=(psgOut>>2)+(psgOut>>3);
  }

  l=(o[0]<<7)+psgOut;
  r=(o[1]<<7)+psgOut;
}

void DivPlatformGenesis::tick() {
  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
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

  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].freqChanged) {
      chan[i].freq=(chan[i].baseFreq*(ONE_SEMITONE+chan[i].pitch))/ONE_SEMITONE;
      int freqt=toFreq(chan[i].freq);
      writes.emplace(chanOffs[i]+0xa4,freqt>>8);
      writes.emplace(chanOffs[i]+0xa0,freqt&0xff);
    }
    if (chan[i].keyOn) {
      writes.emplace(0x28,0xf0|konOffs[i]);
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

int DivPlatformGenesis::dispatch(DivCommand c) {
  if (c.chan>5) {
    c.chan-=6;
    return psg.dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan==5 && dacMode) {
        dacSample=c.value%12;
        if (dacSample>=parent->song.sampleLen) {
          dacSample=-1;
          break;
        }
        dacPos=0;
        dacPeriod=0;
        dacRate=dacRates[parent->song.sample[dacSample]->rate];
        break;
      }
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      
      
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

      chan[c.chan].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
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
      if (c.chan==5) {
        dacMode=c.value;
        rWrite(0x2b,c.value<<7);
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
      rWrite(baseAddr+0x30,(c.value2&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
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
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      break;
    default:
      printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

int DivPlatformGenesis::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=1278409;
  OPN2_Reset(&fm);
  for (int i=0; i<10; i++) {
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

  extMode=false;

  // LFO
  writes.emplace(0x22,0x08);
  
  delay=0;
  
  // PSG
  psg.init(p,4,sugRate);
  psgClocks=0;
  psgOut=0;
  return 10;
}
