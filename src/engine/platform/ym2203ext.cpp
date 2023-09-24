/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#include "ym2203ext.h"
#include "../engine.h"
#include <math.h>

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

void DivPlatformYM2203Ext::commitStateExt(int ch, DivInstrument* ins) {
  int ordch=orderedOps[ch];
  
  if (opChan[ch].insChanged) {
    chan[2].state.alg=ins->fm.alg;
    if (ch==0 || fbAllOps) {
      chan[2].state.fb=ins->fm.fb;
    }
    chan[2].state.op[ordch]=ins->fm.op[ordch];
  }
  
  unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
  DivInstrumentFM::Operator& op=chan[2].state.op[ordch];
  // TODO: how does this work?!
  if (isOpMuted[ch] || !op.enable) {
    rWrite(baseAddr+0x40,127);
  } else {
    if (opChan[ch].insChanged) {
      rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[ch].outVol&0x7f,127));
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
  }
}

int DivPlatformYM2203Ext::dispatch(DivCommand c) {
  if (c.chan<2) {
    return DivPlatformYM2203::dispatch(c);
  }
  if (c.chan>5) {
    c.chan-=3;
    return DivPlatformYM2203::dispatch(c);
  }
  int ch=c.chan-2;
  int ordch=orderedOps[ch];
  if (!extMode) {
    c.chan=2;
    return DivPlatformYM2203::dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins,DIV_INS_FM);

      if (noExtMacros) {
        opChan[ch].macroInit(NULL);
      } else {
        opChan[ch].macroInit(ins);       
      }
      if (!opChan[ch].std.vol.will) {
        opChan[ch].outVol=opChan[ch].vol;
      }

      commitStateExt(ch,ins);
      opChan[ch].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        opChan[ch].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
        opChan[ch].portaPause=false;
        opChan[ch].note=c.value;
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
    case DIV_CMD_NOTE_OFF_ENV:
      if (noExtMacros) break;
      opChan[ch].keyOff=true;
      opChan[ch].keyOn=false;
      opChan[ch].active=false;
      opChan[ch].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      if (noExtMacros) break;
      opChan[ch].std.release();
      break;
    case DIV_CMD_VOLUME: {
      opChan[ch].vol=c.value;
      if (!opChan[ch].std.vol.has) {
        opChan[ch].outVol=c.value;
      }
      unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
      DivInstrumentFM::Operator& op=chan[2].state.op[ordch];
      if (isOpMuted[ch] || !op.enable) {
        rWrite(baseAddr+0x40,127);
      } else {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[ch].outVol&0x7f,127));
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
      chan[extChanOffs].ins=opChan[ch].ins;
      break;
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
    case DIV_CMD_LEGATO: {
      if (opChan[ch].insChanged) {
        DivInstrument* ins=parent->getIns(opChan[ch].ins,DIV_INS_FM);
        commitStateExt(ch,ins);
        opChan[ch].insChanged=false;
      }
      opChan[ch].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      if (extMode==(bool)c.value) break;
      extMode=c.value;
      immWrite(0x27,extMode?0x40:0);
      if (!extMode) {
        for (int i=0; i<4; i++) {
          opChan[i].insChanged=true;
        }
        chan[extChanOffs].insChanged=true;
      }
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[2].state.fb=c.value&7;
      rWrite(chanOffs[2]+ADDR_FB_ALG,(chan[2].state.alg&7)|(chan[2].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOpMuted[ch] || !op.enable) {
        rWrite(baseAddr+0x40,127);
      } else if (KVS(2,c.value)) {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[ch].outVol&0x7f,127));
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
    case DIV_CMD_FM_HARD_RESET:
      opChan[ch].hardReset=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_MACRO_OFF:
      opChan[ch].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      opChan[ch].std.mask(c.value,false);
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

void DivPlatformYM2203Ext::tick(bool sysTick) {
  int hardResetElapsed=0;
  bool mustHardReset=false;

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
      if (opChan[i].hardReset && opChan[i].keyOn) {
        mustHardReset=true;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
        immWrite(baseAddr+ADDR_SL_RR,0x0f);
        hardResetElapsed++;
      }
    }
    if (writeSomething) {
      immWrite(0x28,writeMask);
    }
  }

  if (extMode && !noExtMacros) for (int i=0; i<4; i++) {
    opChan[i].std.next();

    if (opChan[i].std.vol.had) {
      opChan[i].outVol=VOL_SCALE_LOG_BROKEN(opChan[i].vol,MIN(127,opChan[i].std.vol.val),127);
      unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[i]];
      DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[i]];
      if (isOpMuted[i]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[i].outVol&0x7f,127));
      }
    }

    if (opChan[i].std.arp.had) {
      if (!opChan[i].inPorta) {
        opChan[i].baseFreq=NOTE_FNUM_BLOCK(parent->calcArp(opChan[i].note,opChan[i].std.arp.val),11);
      }
      opChan[i].freqChanged=true;
    }

    if (opChan[i].std.pitch.had) {
      if (opChan[i].std.pitch.mode) {
        opChan[i].pitch2+=opChan[i].std.pitch.val;
        CLAMP_VAR(opChan[i].pitch2,-1048576,1048575);
      } else {
        opChan[i].pitch2=opChan[i].std.pitch.val;
      }
      opChan[i].freqChanged=true;
    }

    // channel macros
    if (opChan[i].std.alg.had) {
      chan[extChanOffs].state.alg=opChan[i].std.alg.val;
      rWrite(chanOffs[extChanOffs]+ADDR_FB_ALG,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
      if (!parent->song.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[j];
        if (isOpMuted[j] || !op.enable) {
          rWrite(baseAddr+0x40,127);
        } else {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[j].outVol&0x7f,127));
        }
      }
    }
    if (i==0 || fbAllOps) {
      if (opChan[i].std.fb.had) {
        chan[extChanOffs].state.fb=opChan[i].std.fb.val;
        rWrite(chanOffs[extChanOffs]+ADDR_FB_ALG,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
      }
    }

    // param macros
    unsigned short baseAddr=chanOffs[2]|opOffs[orderedOps[i]];
    DivInstrumentFM::Operator& op=chan[2].state.op[orderedOps[i]];
    DivMacroInt::IntOp& m=opChan[i].std.op[orderedOps[i]];
    if (m.am.had) {
      op.am=m.am.val;
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
    }
    if (m.ar.had) {
      op.ar=m.ar.val;
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
    }
    if (m.dr.had) {
      op.dr=m.dr.val;
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
    }
    if (m.mult.had) {
      op.mult=m.mult.val;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
    }
    if (m.rr.had) {
      op.rr=m.rr.val;
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
    }
    if (m.sl.had) {
      op.sl=m.sl.val;
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
    }
    if (m.tl.had) {
      op.tl=m.tl.val;
      if (isOpMuted[i]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[i].outVol&0x7f,127));
      }
    }
    if (m.rs.had) {
      op.rs=m.rs.val;
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
    }
    if (m.dt.had) {
      op.dt=m.dt.val;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
    }
    if (m.d2r.had) {
      op.d2r=m.d2r.val;
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
    }
    if (m.ssg.had) {
      op.ssgEnv=m.ssg.val;
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
  }


  DivPlatformYM2203::tick(sysTick);

  bool writeNoteOn=false;
  unsigned char writeMask=2;
  unsigned char hardResetMask=0;
  if (extMode) for (int i=0; i<4; i++) {
    if (opChan[i].freqChanged) {
      if (parent->song.linearPitch==2) {
        opChan[i].freq=parent->calcFreq(opChan[i].baseFreq,opChan[i].pitch,opChan[i].fixedArp?opChan[i].baseNoteOverride:opChan[i].arpOff,opChan[i].fixedArp,false,4,opChan[i].pitch2,chipClock,CHIP_FREQBASE,11);
      } else {
        int fNum=parent->calcFreq(opChan[i].baseFreq&0x7ff,opChan[i].pitch,opChan[i].fixedArp?opChan[i].baseNoteOverride:opChan[i].arpOff,opChan[i].fixedArp,false,4,opChan[i].pitch2);
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
        if (opChan[i].hardReset) {
          hardResetMask|=1<<(4+i);
        }
      }
      if (!opChan[i].hardReset) {
        opChan[i].keyOn=false;
      }
    }
  }
  if (writeNoteOn) {
    writeMask^=hardResetMask;
    immWrite(0x28,writeMask);
    writeMask^=hardResetMask;

    // hard reset handling
    if (mustHardReset) {
      for (unsigned int i=hardResetElapsed; i<hardResetCycles; i++) {
        immWrite(0xf0,i&0xff);
      }
      for (int i=0; i<4; i++) {
        if (opChan[i].keyOn && opChan[i].hardReset) {
          // restore SL/RR
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          immWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          opChan[i].keyOn=false;
        }
      }
      immWrite(0x28,writeMask);
    }
  }
}

void DivPlatformYM2203Ext::muteChannel(int ch, bool mute) {
  if (ch<2) {
    DivPlatformYM2203::muteChannel(ch,mute);
    return;
  }
  if (ch>5) {
    DivPlatformYM2203::muteChannel(ch-3,mute);
    return;
  }
  isOpMuted[ch-2]=mute;
  
  DivPlatformYM2203::muteChannel(extChanOffs,IS_EXTCH_MUTED);
  
  if (extMode) {
    for (int i=0; i<4; i++) {
      int ordch=orderedOps[i];
      unsigned short baseAddr=chanOffs[2]|opOffs[ordch];
      DivInstrumentFM::Operator op=chan[2].state.op[ordch];
      if (isOpMuted[i] || !op.enable) {
        rWrite(baseAddr+0x40,127);
      } else {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[i].outVol&0x7f,127));
      }
    }
  }
}

void DivPlatformYM2203Ext::forceIns() {
  for (int i=0; i<3; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (i==2 && extMode) { // extended channel
        if (isOpMuted[orderedOps[j]] || !op.enable) {
          rWrite(baseAddr+0x40,127);
        } else {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[orderedOps[j]].outVol&0x7f,127));
        }
      } else {
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (KVS(i,j)) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[i].outVol&0x7f,127));
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
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }

  ay->forceIns();
  ay->flushWrites();
  for (DivRegWrite& i: ay->getRegisterWrites()) {
    if (i.addr>15) continue;
    immWrite(i.addr&15,i.val);
  }
  ay->getRegisterWrites().clear();
  for (int i=0; i<4; i++) {
    opChan[i].insChanged=true;
    if (opChan[i].active) {
      opChan[i].keyOn=true;
      opChan[i].freqChanged=true;
    }
  }
  if (!extMode) {
    immWrite(0x27,0x00);
  }
}

void* DivPlatformYM2203Ext::getChanState(int ch) {
  if (ch>=6) return &chan[ch-3];
  if (ch>=2) return &opChan[ch-2];
  return &chan[ch];
}

DivMacroInt* DivPlatformYM2203Ext::getChanMacroInt(int ch) {
  if (ch>=6) return ay->getChanMacroInt(ch-6);
  if (ch>=2) return &opChan[ch-2].std;
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformYM2203Ext::getOscBuffer(int ch) {
  if (ch>=6) return oscBuf[ch-3];
  if (ch<3) return oscBuf[ch];
  return NULL;
}

void DivPlatformYM2203Ext::reset() {
  DivPlatformYM2203::reset();

  for (int i=0; i<4; i++) {
    opChan[i]=DivPlatformOPN::OPNOpChannel();
    opChan[i].std.setEngine(parent);
    opChan[i].vol=127;
    opChan[i].outVol=127;
  }

  // channel 2 mode
  immWrite(0x27,0x40);
  extMode=true;
}

bool DivPlatformYM2203Ext::keyOffAffectsArp(int ch) {
  return (ch>8);
}

void DivPlatformYM2203Ext::notifyInsChange(int ins) {
  DivPlatformYM2203::notifyInsChange(ins);
  for (int i=0; i<4; i++) {
    if (opChan[i].ins==ins) {
      opChan[i].insChanged=true;
    }
  }
}

void DivPlatformYM2203Ext::notifyInsDeletion(void* ins) {
  DivPlatformYM2203::notifyInsDeletion(ins);
  for (int i=0; i<4; i++) {
    opChan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

int DivPlatformYM2203Ext::init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) {
  DivPlatformYM2203::init(parent,channels,sugRate,flags);
  for (int i=0; i<4; i++) {
    isOpMuted[i]=false;
  }
  extSys=true;

  reset();
  return 9;
}

void DivPlatformYM2203Ext::quit() {
  DivPlatformYM2203::quit();
}

DivPlatformYM2203Ext::~DivPlatformYM2203Ext() {
}
