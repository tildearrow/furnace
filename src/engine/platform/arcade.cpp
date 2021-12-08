#include "arcade.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

static unsigned short chanOffs[8]={
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};
static unsigned short opOffs[4]={
  0x00, 0x08, 0x10, 0x18
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

static int orderedOps[4]={
  0,2,1,3
};

#define rWrite(a,v) pendingWrites[a]=v;

void DivPlatformArcade::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static int o[2];

  for (size_t h=start; h<start+len; h++) {
    if (!writes.empty() && !fm.write_busy) {
      QueuedWrite& w=writes.front();
      if (w.addrOrVal) {
        OPM_Write(&fm,1,w.val);
        //printf("write: %x = %.2x\n",w.addr,w.val);
        writes.pop();
      } else {
        OPM_Write(&fm,0,w.addr);
        w.addrOrVal=true;
      }
    }
    
    OPM_Clock(&fm,o,NULL,NULL,NULL);
    OPM_Clock(&fm,o,NULL,NULL,NULL);
    OPM_Clock(&fm,o,NULL,NULL,NULL);
    OPM_Clock(&fm,o,NULL,NULL,NULL);
    
    //if (o[0]<-32768) o[0]=-32768;
    //if (o[0]>32767) o[0]=32767;

    //if (o[1]<-32768) o[1]=-32768;
    //if (o[1]>32767) o[1]=32767;
  
    bufL[h]=o[0];
    bufR[h]=o[1];
  }
}

unsigned char noteMap[12]={
  0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14
};

int hScale(int note) {
  return ((note/12)<<4)+(noteMap[note%12]);
}

void DivPlatformArcade::tick() {
  for (int i=0; i<8; i++) {
    if (chan[i].keyOn || chan[i].keyOff) {
      writes.emplace(0x08,i);
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      writes.emplace(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+(chan[i].pitch>>2)-64;
      //writes.emplace(chanOffs[i]+0xa4,freqt>>8);
      //writes.emplace(chanOffs[i]+0xa0,freqt&0xff);
      writes.emplace(i+0x28,hScale(chan[i].freq>>6));
      writes.emplace(i+0x30,chan[i].freq<<2);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      writes.emplace(0x08,0x78|i);
      chan[i].keyOn=false;
    }
  }
}

int DivPlatformArcade::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);

      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=ins->fm.op[i];
        if (isOutput[ins->fm.alg][i]) {
          if (!chan[c.chan].active || chan[c.chan].insChanged) {
            rWrite(baseAddr+0x60,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
          }
        } else {
          if (chan[c.chan].insChanged) {
            rWrite(baseAddr+0x60,op.tl);
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+0x40,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+0x80,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+0xa0,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+0xc0,(op.d2r&31)|(op.dt2<<6));
          rWrite(baseAddr+0xe0,(op.rr&15)|(op.sl<<4));
        }
      }
      if (chan[c.chan].insChanged) {
        rWrite(chanOffs[c.chan]+0x20,(ins->fm.alg&7)|(ins->fm.fb<<3)|0xc0);
        //rWrite(chanOffs[c.chan]+0xb4,(chan[c.chan].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      }
      chan[c.chan].insChanged=false;

      chan[c.chan].baseFreq=c.value<<6;
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
          rWrite(baseAddr+0x60,127-(((127-op.tl)*(chan[c.chan].vol&0x7f))/127));
        } else {
          rWrite(baseAddr+0x60,op.tl);
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
      // TODO
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=c.value2<<6;
      int newFreq;
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value;
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value;
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].baseFreq=newFreq;
      chan[c.chan].freqChanged=true;
      if (return2) return 2;
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=c.value<<6;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      rWrite(0x22,(c.value&7)|((c.value>>4)<<3));
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

bool DivPlatformArcade::isStereo() {
  return true;
}

int DivPlatformArcade::init(DivEngine* p, int channels, int sugRate, bool pal) {
  parent=p;
  rate=447443;
  memset(&fm,0,sizeof(opm_t));
  OPM_Reset(&fm);
  for (int i=0; i<13; i++) {
    chan[i].vol=0x7f;
  }

  for (int i=0; i<256; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;

  extMode=false;

  return 13;
}
