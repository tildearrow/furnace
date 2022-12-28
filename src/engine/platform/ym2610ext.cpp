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

#include "ym2610ext.h"
#include <math.h>

int DivPlatformYM2610Ext::dispatch(DivCommand c) {
  if (c.chan<extChanOffs) {
    return DivPlatformYM2610::dispatch(c);
  }
  if (c.chan>(extChanOffs+3)) {
    c.chan-=3;
    return DivPlatformYM2610::dispatch(c);
  }
  int ch=c.chan-extChanOffs;
  int ordch=orderedOps[ch];
  if (!extMode) {
    c.chan=extChanOffs;
    return DivPlatformYM2610::dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins,DIV_INS_FM);

      if (opChan[ch].insChanged) {
        chan[extChanOffs].state.alg=ins->fm.alg;
        if (ch==0 || fbAllOps) {
          chan[extChanOffs].state.fb=ins->fm.fb;
        }
        chan[extChanOffs].state.fms=ins->fm.fms;
        chan[extChanOffs].state.ams=ins->fm.ams;
        chan[extChanOffs].state.op[ordch]=ins->fm.op[ordch];
      }

      if (noExtMacros) {
        opChan[ch].macroInit(NULL);
      } else {
        opChan[ch].macroInit(ins);       
      }
      if (!opChan[ch].std.vol.will) {
        opChan[ch].outVol=opChan[ch].vol;
      }
      
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[ordch];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[ordch];
      // TODO: how does this work?!
      if (isOpMuted[ch]) {
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
        rWrite(chanOffs[extChanOffs]+0xb0,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
        rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
      }
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
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[ordch];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[ordch];
      if (isOpMuted[ch]) {
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
      rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
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
      chan[extChanOffs].state.fb=c.value&7;
      rWrite(chanOffs[extChanOffs]+ADDR_FB_ALG,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isOpMuted[ch]) {
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
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.rs=c.value2&3;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.rs=c.value2&3;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.am=c.value2&1;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.am=c.value2&1;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.dr=c.value2&31;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.dr=c.value2&31;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.sl=c.value2&15;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.sl=c.value2&15;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.rr=c.value2&15;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.rr=c.value2&15;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_D2R: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.d2r=c.value2&31;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.d2r=c.value2&31;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      }
      break;
    }
    case DIV_CMD_FM_DT: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.dt=c.value&7;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.dt=c.value2&7;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      break;
    }
    case DIV_CMD_FM_SSG: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.ssgEnv=8^(c.value2&15);
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.ssgEnv=8^(c.value2&15);
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
      }
      break;
    }
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

void DivPlatformYM2610Ext::tick(bool sysTick) {
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
      immWrite(0x28,writeMask);
    }
  }

  if (extMode && !noExtMacros) for (int i=0; i<4; i++) {
    opChan[i].std.next();

    if (opChan[i].std.vol.had) {
      opChan[i].outVol=VOL_SCALE_LOG_BROKEN(opChan[i].vol,MIN(127,opChan[i].std.vol.val),127);
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[i]];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[i]];
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

    // param macros
    unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[i]];
    DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[i]];
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
      op.tl=127-m.tl.val;
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

  DivPlatformYM2610::tick(sysTick);

  bool writeNoteOn=false;
  unsigned char writeMask=2;
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
      }
      opChan[i].keyOn=false;
    }
  }
  if (writeNoteOn) {
    immWrite(0x28,writeMask);
  }
}

void DivPlatformYM2610Ext::muteChannel(int ch, bool mute) {
  if (ch<extChanOffs) {
    DivPlatformYM2610::muteChannel(ch,mute);
    return;
  }
  if (ch>(extChanOffs+3)) {
    DivPlatformYM2610::muteChannel(ch-3,mute);
    return;
  }
  isOpMuted[ch-extChanOffs]=mute;
  
  int ordch=orderedOps[ch-extChanOffs];
  unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[ordch];
  DivInstrumentFM::Operator op=chan[extChanOffs].state.op[ordch];
  if (isOpMuted[ch-extChanOffs]) {
    rWrite(baseAddr+0x40,127);
    immWrite(baseAddr+0x40,127);
  } else if (KVS(2,ordch)) {
    rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[ch-extChanOffs].outVol&0x7f,127));
    immWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[ch-extChanOffs].outVol&0x7f,127));
  } else {
    rWrite(baseAddr+0x40,op.tl);
    immWrite(baseAddr+0x40,op.tl);
  }

  rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch-extChanOffs].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
}

void DivPlatformYM2610Ext::forceIns() {
  for (int i=0; i<psgChanOffs; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (i==extChanOffs && extMode) { // extended channel
        if (isOpMuted[j]) {
          rWrite(baseAddr+0x40,127);
        } else if (KVS(i,j)) {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG_BROKEN(127-op.tl,opChan[j].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+0x40,op.tl);
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
    if (i==extChanOffs) {
      rWrite(chanOffs[i]+ADDR_LRAF,(IS_EXTCH_MUTED?0:(opChan[0].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    } else {
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  for (int i=adpcmAChanOffs; i<=adpcmBChanOffs; i++) {
    chan[i].insChanged=true;
  }
  ay->forceIns();
  ay->flushWrites();
  for (DivRegWrite& i: ay->getRegisterWrites()) {
    if (i.addr>15) continue;
    immWrite(i.addr&15,i.val);
  }
  ay->getRegisterWrites().clear();
  immWrite(0x22,lfoValue);
  for (int i=0; i<4; i++) {
    opChan[i].insChanged=true;
    if (opChan[i].active) {
      opChan[i].keyOn=true;
      opChan[i].freqChanged=true;
    }
  }
}

void* DivPlatformYM2610Ext::getChanState(int ch) {
  if (ch>=(extChanOffs+4)) return &chan[ch-3];
  if (ch>=extChanOffs) return &opChan[ch-extChanOffs];
  return &chan[ch];
}

DivMacroInt* DivPlatformYM2610Ext::getChanMacroInt(int ch) {
  if (ch>=(psgChanOffs+3) && ch<(adpcmAChanOffs+3)) return ay->getChanMacroInt(ch-psgChanOffs-3);
  if (ch>=(extChanOffs+4)) return &chan[ch-3].std;
  if (ch>=extChanOffs) return &opChan[ch-extChanOffs].std;
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformYM2610Ext::getOscBuffer(int ch) {
  if (ch>=(extChanOffs+4)) return oscBuf[ch-3];
  if (ch<(extChanOffs+1)) return oscBuf[ch];
  return NULL;
}

void DivPlatformYM2610Ext::reset() {
  DivPlatformYM2610::reset();

  for (int i=0; i<4; i++) {
    opChan[i]=DivPlatformOPN::OPNOpChannelStereo();
    opChan[i].std.setEngine(parent);
    opChan[i].vol=127;
    opChan[i].outVol=127;
  }

  // channel 2 mode
  immWrite(0x27,0x40);
  extMode=true;
}

bool DivPlatformYM2610Ext::keyOffAffectsArp(int ch) {
  return (ch>=(psgChanOffs+3));
}

void DivPlatformYM2610Ext::notifyInsChange(int ins) {
  DivPlatformYM2610::notifyInsChange(ins);
  for (int i=0; i<4; i++) {
    if (opChan[i].ins==ins) {
      opChan[i].insChanged=true;
    }
  }
}

int DivPlatformYM2610Ext::init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) {
  DivPlatformYM2610::init(parent,channels,sugRate,flags);
  for (int i=0; i<4; i++) {
    isOpMuted[i]=false;
  }
  extSys=true;

  reset();
  return 17;
}

void DivPlatformYM2610Ext::quit() {
  DivPlatformYM2610::quit();
}

DivPlatformYM2610Ext::~DivPlatformYM2610Ext() {
}
