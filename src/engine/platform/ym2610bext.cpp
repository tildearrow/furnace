/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ym2610bext.h"
#include "../engine.h"
#include <math.h>

#include "ym2610shared.h"

int DivPlatformYM2610BExt::dispatch(DivCommand c) {
  if (c.chan<2) {
    return DivPlatformYM2610B::dispatch(c);
  }
  if (c.chan>5) {
    c.chan-=3;
    return DivPlatformYM2610B::dispatch(c);
  }
  int ch=c.chan-2;
  int ordch=orderedOps[ch];
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      
      unsigned short baseAddr=chanOffs_b[2]|opOffs[ordch];
      DivInstrumentFM::Operator op=ins->fm.op[ordch];
      // TODO: how does this work?!
      if (isOpMuted[ch]) {
        rWrite(baseAddr+0x40,127);
      } else {
        if (opChan[ch].insChanged) {
          rWrite(baseAddr+0x40,127-(((127-op.tl)*(opChan[ch].vol&0x7f))/127));
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
        rWrite(chanOffs_b[2]+0xb0,(ins->fm.alg&7)|(ins->fm.fb<<3));
        rWrite(chanOffs_b[2]+0xb4,(opChan[ch].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      }
      opChan[ch].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        opChan[ch].baseFreq=NOTE_FREQUENCY(c.value);
        opChan[ch].freqChanged=true;
      }
      opChan[ch].keyOn=true;
      opChan[ch].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      opChan[ch].keyOff=true;
      opChan[ch].keyOn=false;
      opChan[ch].active=false;
      break;
    case DIV_CMD_VOLUME: {
      opChan[ch].vol=c.value;
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      unsigned short baseAddr=chanOffs_b[2]|opOffs[ordch];
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
      rWrite(chanOffs_b[2]+0xb4,(opChan[ch].pan<<6)|(ins->fm.fms&7)|((ins->fm.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      opChan[ch].pitch=c.value;
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
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
    case DIV_CMD_LEGATO: {
      opChan[ch].baseFreq=NOTE_FREQUENCY(c.value);
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_MULT: { // TODO
      unsigned short baseAddr=chanOffs_b[2]|opOffs[orderedOps[c.value]];
      DivInstrument* ins=parent->getIns(opChan[ch].ins);
      DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
      rWrite(baseAddr+0x30,(c.value2&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: { // TODO
      unsigned short baseAddr=chanOffs_b[2]|opOffs[orderedOps[c.value]];
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
          unsigned short baseAddr=chanOffs_b[2]|opOffs[i];
          rWrite(baseAddr+0x50,(c.value2&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator op=ins->fm.op[orderedOps[c.value]];
        unsigned short baseAddr=chanOffs_b[2]|opOffs[orderedOps[c.value]];
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

static int opChanOffsL[4]={
  0xa9, 0xaa, 0xa8, 0xa2
};

static int opChanOffsH[4]={
  0xad, 0xae, 0xac, 0xa6
};

void DivPlatformYM2610BExt::tick() {
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

  DivPlatformYM2610B::tick();

  bool writeNoteOn=false;
  unsigned char writeMask=2;
  if (extMode) for (int i=0; i<4; i++) {
    if (opChan[i].freqChanged) {
      opChan[i].freq=parent->calcFreq(opChan[i].baseFreq,opChan[i].pitch);
      if (opChan[i].freq>262143) opChan[i].freq=262143;
      int freqt=toFreq(opChan[i].freq);
      opChan[i].freqH=freqt>>8;
      opChan[i].freqL=freqt&0xff;
      immWrite(opChanOffsH[i],opChan[i].freqH);
      immWrite(opChanOffsL[i],opChan[i].freqL);
      opChan[i].freqChanged=false;
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

void DivPlatformYM2610BExt::muteChannel(int ch, bool mute) {
  if (ch<2) {
    DivPlatformYM2610B::muteChannel(ch,mute);
    return;
  }
  if (ch>5) {
    DivPlatformYM2610B::muteChannel(ch-3,mute);
    return;
  }
  isOpMuted[ch-2]=mute;
  
  int ordch=orderedOps[ch-2];
  DivInstrument* ins=parent->getIns(opChan[ch-2].ins);
  unsigned short baseAddr=chanOffs_b[2]|opOffs[ordch];
  DivInstrumentFM::Operator op=ins->fm.op[ordch];
  if (isOpMuted[ch-2]) {
    rWrite(baseAddr+0x40,127);
  } else if (isOutput[ins->fm.alg][ordch]) {
    rWrite(baseAddr+0x40,127-(((127-op.tl)*(opChan[ch-2].vol&0x7f))/127));
  } else {
    rWrite(baseAddr+0x40,op.tl);
  }
}

void DivPlatformYM2610BExt::forceIns() {
  DivPlatformYM2610B::forceIns();
  for (int i=0; i<4; i++) {
    opChan[i].insChanged=true;
    if (opChan[i].active) {
      opChan[i].keyOn=true;
      opChan[i].freqChanged=true;
    }
  }
}


void* DivPlatformYM2610BExt::getChanState(int ch) {
  if (ch>=6) return &chan[ch-3];
  if (ch>=2) return &opChan[ch-2];
  return &chan[ch];
}

void DivPlatformYM2610BExt::reset() {
  DivPlatformYM2610B::reset();

  for (int i=0; i<4; i++) {
    opChan[i]=DivPlatformYM2610BExt::OpChannel();
    opChan[i].vol=127;
  }

  // channel 2 mode
  immWrite(0x27,0x40);
  extMode=true;
}

bool DivPlatformYM2610BExt::keyOffAffectsArp(int ch) {
  return (ch>8);
}

void DivPlatformYM2610BExt::notifyInsChange(int ins) {
  DivPlatformYM2610B::notifyInsChange(ins);
  for (int i=0; i<4; i++) {
    if (opChan[i].ins==ins) {
      opChan[i].insChanged=true;
    }
  }
}

int DivPlatformYM2610BExt::init(DivEngine* parent, int channels, int sugRate, unsigned int flags) {
  DivPlatformYM2610B::init(parent,channels,sugRate,flags);
  for (int i=0; i<4; i++) {
    isOpMuted[i]=false;
  }

  reset();
  return 19;
}

void DivPlatformYM2610BExt::quit() {
  DivPlatformYM2610B::quit();
}

DivPlatformYM2610BExt::~DivPlatformYM2610BExt() {
}