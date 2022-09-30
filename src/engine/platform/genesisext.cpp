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

#include "genesisext.h"
#include "../engine.h"
#include <math.h>

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

#define IS_REALLY_MUTED(x) (isMuted[x] && (x<5 || !softPCM || (isMuted[5] && isMuted[6])))
#define IS_EXTCH_MUTED (isOpMuted[0] && isOpMuted[1] && isOpMuted[2] && isOpMuted[3])

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
  if (!extMode) {
    c.chan=2;
    return DivPlatformGenesis::dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins,DIV_INS_FM);

      if (opChan[ch].insChanged) {
        chan[2].state.alg=ins->fm.alg;
        chan[2].state.fb=ins->fm.fb;
        chan[2].state.fms=ins->fm.fms;
        chan[2].state.ams=ins->fm.ams;
        chan[2].state.op[ordch]=ins->fm.op[ordch];
      }
      
      unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
      DivInstrumentFM::Operator& op=chan[2].state.op[ordch];
      // TODO: how does this work?!
      if (isOpMuted[ch]) {
        rWrite(baseAddr+0x40,127);
      } else {
        if (opChan[ch].insChanged) {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch].vol&0x7f,127));
        }
      }
      if (opChan[ch].insChanged) {
        rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
        rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
        rWrite(baseAddr+0x60,(op.dr&31)|(op.am<<7));
        rWrite(baseAddr+0x70,op.d2r&31);
        rWrite(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
        rWrite(baseAddr+0x90,op.ssgEnv&15);
        opChan[ch].mask=op.enable;
      }
      if (opChan[ch].insChanged) { // TODO how does this work?
        rWrite(chanOffs[2]+0xb0,(chan[2].state.alg&7)|(chan[2].state.fb<<3));
        rWrite(chanOffs[2]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[2].state.fms&7)|((chan[2].state.ams&3)<<4));
      }
      opChan[ch].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        opChan[ch].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
        opChan[ch].portaPause=false;
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
      unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
      DivInstrumentFM::Operator& op=chan[2].state.op[ordch];
      if (isOpMuted[ch]) {
        rWrite(baseAddr+0x40,127);
      } else {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch].vol&0x7f,127));
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
      if (c.value==0 && c.value2==0) {
        opChan[ch].pan=3;
      } else {
        opChan[ch].pan=(c.value2>0)|((c.value>0)<<1);
      }
      if (parent->song.sharedExtStat) {
        for (int i=0; i<4; i++) {
          if (ch==i) continue;
          opChan[i].pan=opChan[ch].pan;
        }
      }
      rWrite(chanOffs[2]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[2].state.fms&7)|((chan[2].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      opChan[ch].pitch=c.value;
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (parent->song.linearPitch==2) {
        int destFreq=NOTE_FREQUENCY(c.value2);
        bool return2=false;
        if (destFreq>opChan[ch].baseFreq) {
          opChan[ch].baseFreq+=c.value;
          if (opChan[ch].baseFreq>=destFreq) {
            opChan[ch].baseFreq=destFreq;
            return2=true;
          }
        } else {
          opChan[ch].baseFreq-=c.value;
          if (opChan[ch].baseFreq<=destFreq) {
            opChan[ch].baseFreq=destFreq;
            return2=true;
          }
        }
        opChan[ch].freqChanged=true;
        if (return2) {
          //opChan[ch].inPorta=false;
          return 2;
        }
        break;
      }
      PLEASE_HELP_ME(opChan[ch]);
      break;
    }
    case DIV_CMD_SAMPLE_MODE: {
      // not ignored actually!
      if (!parent->song.ignoreDACModeOutsideIntendedChannel) {
        chan[5].dacMode=c.value;
        rWrite(0x2b,c.value<<7);
      }
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      if (!parent->song.ignoreDACModeOutsideIntendedChannel) {
        chan[5].sampleBank=c.value;
        if (chan[5].sampleBank>(parent->song.sample.size()/12)) {
          chan[5].sampleBank=parent->song.sample.size()/12;
        }
      }
      break;
    case DIV_CMD_LEGATO: {
      opChan[ch].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      extMode=c.value;
      immWrite(0x27,extMode?0x40:0);
      break;
    }
    case DIV_CMD_FM_LFO: {
      lfoValue=(c.value&7)|((c.value>>4)<<3);
      rWrite(0x22,lfoValue);
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[2].state.fb=c.value&7;
      rWrite(chanOffs[2]+ADDR_FB_ALG,(chan[2].state.alg&7)|(chan[2].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: { // TODO
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: { // TODO
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOpMuted[ch]) {
        rWrite(baseAddr+0x40,127);
      } else if (KVS(2,c.value)) {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch].vol&0x7f,127));
      } else {
        rWrite(baseAddr+0x40,op.tl);
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.rs=c.value2&3;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.rs=c.value2&3;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.am=c.value2&1;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.am=c.value2&1;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.dr=c.value2&31;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.dr=c.value2&31;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.sl=c.value2&15;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.sl=c.value2&15;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.rr=c.value2&15;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.rr=c.value2&15;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_D2R: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.d2r=c.value2&31;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.d2r=c.value2&31;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      }
      break;
    }
    case DIV_CMD_FM_DT: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.dt=c.value&7;
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.dt=c.value2&7;
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      break;
    }
    case DIV_CMD_FM_SSG: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[2].state.op[i];
          op.ssgEnv=8^(c.value2&15);
          unsigned short baseAddr=chanOffs[2]|opOffs[i];
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
        op.ssgEnv=8^(c.value2&15);
        unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
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
  unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
  DivInstrumentFM::Operator op=chan[2].state.op[ordch];
  if (isOpMuted[ch-2]) {
    rWrite(baseAddr+0x40,127);
    immWrite(baseAddr+0x40,127);
  } else if (KVS(2,ordch)) {
    rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch-2].vol&0x7f,127));
    immWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch-2].vol&0x7f,127));
  } else {
    rWrite(baseAddr+0x40,op.tl);
    immWrite(baseAddr+0x40,op.tl);
  }

  rWrite(chanOffs[2]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch-2].pan<<6))|(chan[2].state.fms&7)|((chan[2].state.ams&3)<<4));
}

static int opChanOffsL[4]={
  0xa9, 0xaa, 0xa8, 0xa2
};

static int opChanOffsH[4]={
  0xad, 0xae, 0xac, 0xa6
};

void DivPlatformGenesisExt::tick(bool sysTick) {
  if (extMode) {
    bool writeSomething=false;
    unsigned char writeMask=2;
    for (int i=0; i<4; i++) {
      writeMask|=(unsigned char)(opChan[i].mask && opChan[i].active)<<(4+i);
      if (opChan[i].keyOn || opChan[i].keyOff) {
        writeSomething=true;
        writeMask&=~(1<<(4+i));
        opChan[i].keyOff=false;
      }
    }
    if (writeSomething) {
      if (chan[7].active) { // CSM
        writeMask^=0xf0;
      }
      /*printf(
        "Mask: %c %c %c %c\n",
        (writeMask&0x10)?'1':'-',
        (writeMask&0x20)?'2':'-',
        (writeMask&0x40)?'3':'-',
        (writeMask&0x80)?'4':'-'
      );*/
      immWrite(0x28,writeMask);
    }
  }

  DivPlatformGenesis::tick(sysTick);

  bool writeNoteOn=false;
  unsigned char writeMask=2;
  if (extMode) for (int i=0; i<4; i++) {
    if (opChan[i].freqChanged) {
      if (parent->song.linearPitch==2) {
        opChan[i].freq=parent->calcFreq(opChan[i].baseFreq,opChan[i].pitch,false,2,opChan[i].pitch2,chipClock,CHIP_FREQBASE,11);
      } else {
        int fNum=parent->calcFreq(opChan[i].baseFreq&0x7ff,opChan[i].pitch,false,2,opChan[i].pitch2);
        int block=(opChan[i].baseFreq&0xf800)>>11;
        if (fNum<0) fNum=0;
        if (fNum>2047) {
          while (block<7) {
            fNum>>=1;
            block++;
          }
          if (fNum>2047) fNum=2047;
        }
        opChan[i].freq=(block<<11)|fNum;
      }
      if (opChan[i].freq>0x3fff) opChan[i].freq=0x3fff;
      immWrite(opChanOffsH[i],opChan[i].freq>>8);
      immWrite(opChanOffsL[i],opChan[i].freq&0xff);
    }
    writeMask|=(unsigned char)(opChan[i].mask && opChan[i].active)<<(4+i);
    if (opChan[i].keyOn) {
      writeNoteOn=true;
      if (opChan[i].mask) {
        writeMask|=1<<(4+i);
      }
      opChan[i].keyOn=false;
    }
  }

  if (extMode && softPCM) {
    if (chan[7].freqChanged) {
      chan[7].freq=parent->calcFreq(chan[7].baseFreq,chan[7].pitch,true,0,chan[7].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[7].freq<1) chan[7].freq=1;
      if (chan[7].freq>1024) chan[7].freq=1024;
      int wf=0x400-chan[7].freq;
      immWrite(0x24,wf>>2);
      immWrite(0x25,wf&3);
      chan[7].freqChanged=false;
    }

    if (chan[7].keyOff || chan[7].keyOn) {
      writeNoteOn=true;
      for (int i=0; i<4; i++) {
        writeMask|=opChan[i].active<<(4+i);
      }
    }
  }

  if (writeNoteOn) {
    if (chan[7].active) { // CSM
      writeMask^=0xf0;
    }
    /*printf(
        "Mask: %c %c %c %c\n",
        (writeMask&0x10)?'1':'-',
        (writeMask&0x20)?'2':'-',
        (writeMask&0x40)?'3':'-',
        (writeMask&0x80)?'4':'-'
      );*/
    immWrite(0x28,writeMask);
  }

  if (extMode && softPCM) {
    if (chan[7].keyOn) {
      immWrite(0x27,0x81);
      chan[7].keyOn=false;
    }
    if (chan[7].keyOff) {
      immWrite(0x27,0x40);
      chan[7].keyOff=false;
    }
  }
}

void DivPlatformGenesisExt::forceIns() {
  for (int i=0; i<6; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (i==2 && extMode) { // extended channel
        if (isOpMuted[j]) {
          rWrite(baseAddr+0x40,127);
        } else if (KVS(i,j)) {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[j].vol&0x7f,127));
        } else {
          rWrite(baseAddr+0x40,op.tl);
        }
      } else {
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (KVS(i,j)) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,chan[i].outVol&0x7f,127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
    rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    if (i==2) {
      rWrite(chanOffs[i]+ADDR_LRAF,(IS_EXTCH_MUTED?0:(opChan[0].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    } else {
      rWrite(chanOffs[i]+ADDR_LRAF,(IS_REALLY_MUTED(i)?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  if (chan[5].dacMode) {
    rWrite(0x2b,0x80);
  }
  immWrite(0x22,lfoValue);
  for (int i=0; i<4; i++) {
    opChan[i].insChanged=true;
    if (opChan[i].active) {
      opChan[i].keyOn=true;
      opChan[i].freqChanged=true;
    }
  }
  if (extMode && softPCM && chan[7].active) { // CSM
    chan[7].insChanged=true;
    chan[7].freqChanged=true;
    chan[7].keyOn=true;
  }
}

void* DivPlatformGenesisExt::getChanState(int ch) {
  if (ch>=6) return &chan[ch-3];
  if (ch>=2) return &opChan[ch-2];
  return &chan[ch];
}

DivMacroInt* DivPlatformGenesisExt::getChanMacroInt(int ch) {
  if (ch>=6) return &chan[ch-3].std;
  if (ch>=2) return NULL; // currently not implemented
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformGenesisExt::getOscBuffer(int ch) {
  if (ch>=6) return oscBuf[ch-3];
  if (ch<3) return oscBuf[ch];
  return NULL;
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

int DivPlatformGenesisExt::getPortaFloor(int ch) {
  return (ch>8)?12:0;
}

int DivPlatformGenesisExt::init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) {
  DivPlatformGenesis::init(parent,channels,sugRate,flags);
  for (int i=0; i<4; i++) {
    isOpMuted[i]=false;
  }
  extSys=true;

  reset();
  return 13;
}

void DivPlatformGenesisExt::quit() {
  DivPlatformGenesis::quit();
}

DivPlatformGenesisExt::~DivPlatformGenesisExt() {
}
