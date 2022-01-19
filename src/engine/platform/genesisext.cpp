#include "genesisext.h"
#include "../engine.h"
#include <math.h>

#include "genesisshared.h"

int DivPlatformGenesisExt::dispatch(DivCommand c) {
  if (c.chan<2) {
    return DivPlatformGenesis::dispatch(c);
  }
  if (c.chan>5) {
    c.chan-=3;
    return DivPlatformGenesis::dispatch(c);
  }
  int ch=c.chan-2;
  int ordch=orderedOps[ch];
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      
      unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
      DivInstrumentFM::Operator op=ins->fm.op[ordch];
      if (isOpMuted[ch]) {
        rWrite(baseAddr+0x40,127);
      } else if (isOutput[ins->fm.alg][ordch]) {
        if (!opChan[ch].active || opChan[ch].insChanged) {
          rWrite(baseAddr+0x40,127-(((127-op.tl)*(opChan[ch].vol&0x7f))/127));
        }
      } else {
        if (opChan[ch].insChanged) {
          rWrite(baseAddr+0x40,op.tl);
        }
      }
      if (opChan[ch].insChanged) {
        rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
        rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
        rWrite(baseAddr+0x60,(op.dr&31)|(op.am<<7));
        rWrite(baseAddr+0x70,op.d2r&31);
        rWrite(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
        rWrite(baseAddr+0x90,op.ssgEnv&15);
      }
      if (opChan[ch].insChanged) { // TODO how does this work?
        rWrite(chanOffs[2]+0xb0,(ins->fm.alg&7)|(ins->fm.fb<<3));
        rWrite(chanOffs[2]+0xb4,(opChan[ch].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      }
      opChan[ch].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        opChan[ch].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
        opChan[ch].freqChanged=true;
      }
      opChan[ch].keyOn=true;
      opChan[ch].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      opChan[ch].keyOff=true;
      opChan[ch].active=false;
      break;
    case DIV_CMD_VOLUME: {
      opChan[ch].vol=c.value;
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
      DivInstrumentFM::Operator op=ins->fm.op[ordch];
      if (isOpMuted[ch]) {
        rWrite(baseAddr+0x40,127);
      } else {
        rWrite(baseAddr+0x40,127-(((127-op.tl)*(opChan[ch].vol&0x7f))/127));
      }
      break;
    }
    case DIV_CMD_GET_VOLUME: {
      return opChan[ch].vol;
      break;
    }
    case DIV_CMD_INSTRUMENT:
      if (opChan[ch].ins!=c.value || c.value2==1) {
        opChan[ch].insChanged=true;
      }
      opChan[ch].ins=c.value;
      break;
    case DIV_CMD_PANNING: {
      switch (c.value) {
        case 0x01:
          opChan[ch].pan=1;
          break;
        case 0x10:
          opChan[ch].pan=2;
          break;
        default:
          opChan[ch].pan=3;
          break;
      }
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      // TODO: ???
      rWrite(chanOffs[2]+0xb4,(opChan[ch].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      opChan[ch].pitch=c.value;
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=644.0f*pow(2.0f,((float)c.value2/12.0f));
      int newFreq;
      bool return2=false;
      if (destFreq>opChan[ch].baseFreq) {
        newFreq=opChan[ch].baseFreq+c.value*octave(opChan[ch].baseFreq);
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=opChan[ch].baseFreq-c.value*octave(opChan[ch].baseFreq);
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      if (!opChan[ch].portaPause) {
        if (octave(opChan[ch].baseFreq)!=octave(newFreq)) {
          opChan[ch].portaPause=true;
          break;
        }
      }
      opChan[ch].baseFreq=newFreq;
      opChan[ch].portaPause=false;
      opChan[ch].freqChanged=true;
      if (return2) return 2;
      break;
    }
    case DIV_CMD_SAMPLE_MODE: {
      // ignored on extended channel 3 mode.
      break;
    }
    case DIV_CMD_LEGATO: {
      opChan[ch].baseFreq=644.0f*pow(2.0f,((float)c.value/12.0f));
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_MULT: { // TODO
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
      rWrite(baseAddr+0x30,(c.value2&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: { // TODO
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      if (isOutput[ins->fm.alg][c.value]) {
        rWrite(baseAddr+0x40,127-(((127-c.value2)*(opChan[ch].vol&0x7f))/127));
      } else {
        rWrite(baseAddr+0x40,c.value2);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator op=ins->fm.op[i];
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+0x50,(c.value2&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+0x50,(c.value2&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_PRE_PORTA:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformGenesisExt::muteChannel(int ch, bool mute) {
  if (ch<2) {
    DivPlatformGenesis::muteChannel(ch,mute);
    return;
  }
  if (ch>5) {
    DivPlatformGenesis::muteChannel(ch-3,mute);
    return;
  }
  isOpMuted[ch-2]=mute;
  
  int ordch=orderedOps[ch-2];
  DivInstrument* ins=parent->getIns(opChan[ch].ins);
  unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
  DivInstrumentFM::Operator op=ins->fm.op[ordch];
  if (isOpMuted[ch]) {
    rWrite(baseAddr+0x40,127);
  } else if (isOutput[ins->fm.alg][ordch]) {
    rWrite(baseAddr+0x40,127-(((127-op.tl)*(opChan[ch].vol&0x7f))/127));
  } else {
    rWrite(baseAddr+0x40,op.tl);
  }
}

static int opChanOffsL[4]={
  0xa9, 0xaa, 0xa8, 0xa2
};

static int opChanOffsH[4]={
  0xad, 0xae, 0xac, 0xa6
};

void DivPlatformGenesisExt::tick() {
  if (extMode) {
    bool writeSomething=false;
    unsigned char writeMask=2;
    for (int i=0; i<4; i++) {
      writeMask|=opChan[i].active<<(4+i);
      if (opChan[i].keyOn || opChan[i].keyOff) {
        writeSomething=true;
        writeMask&=~(1<<(4+i));
        opChan[i].keyOff=false;
      }
    }
    if (writeSomething) {
      immWrite(0x28,writeMask);
    }
  }

  DivPlatformGenesis::tick();

  bool writeNoteOn=false;
  unsigned char writeMask=2;
  if (extMode) for (int i=0; i<4; i++) {
    if (opChan[i].freqChanged) {
      opChan[i].freq=parent->calcFreq(opChan[i].baseFreq,opChan[i].pitch);
      if (opChan[i].freq>=82432) {
        opChan[i].freqH=((opChan[i].freq>>15)&7)|0x38;
        opChan[i].freqL=(opChan[i].freq>>7)&0xff;
      } else if (opChan[i].freq>=41216) {
        opChan[i].freqH=((opChan[i].freq>>14)&7)|0x30;
        opChan[i].freqL=(opChan[i].freq>>6)&0xff;
      } else if (opChan[i].freq>=20608) {
        opChan[i].freqH=((opChan[i].freq>>13)&7)|0x28;
        opChan[i].freqL=(opChan[i].freq>>5)&0xff;
      } else if (opChan[i].freq>=10304) {
        opChan[i].freqH=((opChan[i].freq>>12)&7)|0x20;
        opChan[i].freqL=(opChan[i].freq>>4)&0xff;
      } else if (opChan[i].freq>=5152) {
        opChan[i].freqH=((opChan[i].freq>>11)&7)|0x18;
        opChan[i].freqL=(opChan[i].freq>>3)&0xff;
      } else if (opChan[i].freq>=2576) {
        opChan[i].freqH=((opChan[i].freq>>10)&7)|0x10;
        opChan[i].freqL=(opChan[i].freq>>2)&0xff;
      } else if (opChan[i].freq>=1288) {
        opChan[i].freqH=((opChan[i].freq>>9)&7)|0x08;
        opChan[i].freqL=(opChan[i].freq>>1)&0xff;
      } else {
        opChan[i].freqH=(opChan[i].freq>>8)&7;
        opChan[i].freqL=opChan[i].freq&0xff;
      }
      immWrite(opChanOffsH[i],opChan[i].freqH);
      immWrite(opChanOffsL[i],opChan[i].freqL);
    }
    writeMask|=opChan[i].active<<(4+i);
    if (opChan[i].keyOn) {
      writeNoteOn=true;
      writeMask|=1<<(4+i);
      opChan[i].keyOn=false;
    }
  }
  if (writeNoteOn) {
    immWrite(0x28,writeMask);
  }
}

void DivPlatformGenesisExt::forceIns() {
  DivPlatformGenesis::forceIns();
  for (int i=0; i<4; i++) {
    opChan[i].insChanged=true;
  }
}

void DivPlatformGenesisExt::reset() {
  DivPlatformGenesis::reset();

  for (int i=0; i<4; i++) {
    opChan[i]=DivPlatformGenesisExt::OpChannel();
    opChan[i].vol=127;
  }

  // channel 3 mode
  immWrite(0x27,0x40);
  extMode=true;
}

bool DivPlatformGenesisExt::keyOffAffectsArp(int ch) {
  return (ch>8);
}

bool DivPlatformGenesisExt::keyOffAffectsPorta(int ch) {
  return (ch>8);
}

void DivPlatformGenesisExt::notifyInsChange(int ins) {
  DivPlatformGenesis::notifyInsChange(ins);
  for (int i=0; i<4; i++) {
    if (opChan[i].ins==ins) {
      opChan[i].insChanged=true;
    }
  }
}

int DivPlatformGenesisExt::init(DivEngine* parent, int channels, int sugRate, bool pal) {
  DivPlatformGenesis::init(parent,channels,sugRate,pal);
  for (int i=0; i<4; i++) {
    isOpMuted[i]=false;
  }

  reset();
  return 13;
}

void DivPlatformGenesisExt::quit() {
  DivPlatformGenesis::quit();
}

DivPlatformGenesisExt::~DivPlatformGenesisExt() {
}