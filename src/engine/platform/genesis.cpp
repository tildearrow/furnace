#include "genesis.h"
#include "../engine.h"
#include <string.h>

// TODO fix all the writes.
// i think there is no wait for data writes, just for ON/OFF writes
void DivPlatformGenesis::acquire(short& l, short& r) {
  short o[2];

  if (dacMode && dacSample!=-1) {
    if (--dacPeriod<1) {
      DivSample* s=parent->song.sample[dacSample];
      writes.emplace(0x2a,((unsigned short)s->rendData[dacPos++]+0x8000)>>8);
      if (dacPos>s->rendLength) {
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
  l=o[0]<<7;
  r=o[1]<<7;
}

static unsigned short chanOffs[6]={
  0x00, 0x01, 0x02, 0x100, 0x101, 0x102
};
static unsigned short opOffs[4]={
  0x00, 0x04, 0x08, 0x0c
};
static unsigned char konOffs[6]={
  0, 1, 2, 4, 5, 6
};
static unsigned short notes[12]={
  644,681,722,765,810,858,910,964,1021,1081,1146,1214
};
static bool isOutput[8][4]={
  // 1     3     2    4
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,false,false,true},
  {false,false,true ,true},
  {false,true ,true ,true},
  {false,true ,true ,true},
  {true ,true ,true ,true},
};
static unsigned char dtTable[8]={
  7,6,5,0,1,2,3,0
};
static int dacRates[6]={
  160,160,116,80,58,40
};

void DivPlatformGenesis::tick() {
  for (int i=0; i<6; i++) {
    if (chan[i].keyOn || chan[i].keyOff) {
      writes.emplace(0x28,0x00|konOffs[i]);
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=-1) {
      writes.emplace(i,pendingWrites[i]&0xff);
      pendingWrites[i]=-1;
    }
  }

  for (int i=0; i<6; i++) {
    if (chan[i].keyOn) {
      writes.emplace(chanOffs[i]+0xa4,chan[i].freqH);
      writes.emplace(chanOffs[i]+0xa0,chan[i].freqL);
      writes.emplace(0x28,0xf0|konOffs[i]);
      chan[i].keyOn=false;
    }
  }
}

#define rWrite(a,v) pendingWrites[a]=v;

int DivPlatformGenesis::dispatch(DivCommand c) {
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
      if (c.chan>5) break;
      //chan[c.chan].freq=16.4f*pow(2.0f,((float)c.value/12.0f));
      DivInstrument* ins=parent->song.ins[chan[c.chan].ins];
      
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
        for (int i=0; i<4; i++) {
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          DivInstrumentFM::Operator op=ins->fm.op[i];
          rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
          if (isOutput[ins->fm.alg][i]) {
            rWrite(baseAddr+0x40,127-(((127-op.tl)*chan[c.chan].vol)/127));
          } else {
            rWrite(baseAddr+0x40,op.tl);
          }
          rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+0x60,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+0x70,op.d2r&31);
          rWrite(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+0x90,op.ssgEnv&15);
        }
        rWrite(chanOffs[c.chan]+0xb0,(ins->fm.alg&7)|(ins->fm.fb<<3));
        rWrite(chanOffs[c.chan]+0xb4,(chan[c.chan].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      }
      chan[c.chan].freqH=((c.value/12)<<3)|(notes[c.value%12]>>8);
      chan[c.chan].freqL=notes[c.value%12];
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
      DivInstrument* ins=parent->song.ins[chan[c.chan].ins];
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=ins->fm.op[i];
        if (isOutput[ins->fm.alg][i]) {
          rWrite(baseAddr+0x40,127-(((127-op.tl)*chan[c.chan].vol)/127));
        } else {
          rWrite(baseAddr+0x40,op.tl);
        }
      }
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
      DivInstrument* ins=parent->song.ins[chan[c.chan].ins];
      rWrite(chanOffs[c.chan]+0xb4,(chan[c.chan].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      break;
    }
    case DIV_CMD_SAMPLE_MODE: {
      if (c.chan==5) {
        dacMode=c.value;
        rWrite(0x2b,c.value<<7);
      }
      break;
    }
    default:
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
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  dacMode=0;
  dacPeriod=0;
  dacPos=0;
  dacRate=0;
  dacSample=-1;

  // LFO
  writes.emplace(0x22,0x08);
  
  delay=0;
  return 10;
}
