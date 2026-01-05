/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "ym2610xext.h"
#include <math.h>

void DivPlatformYM2610XExt::commitStateExt(int ch, DivInstrument* ins) {
  int ordch=orderedOps[ch];

  if (opChan[ch].insChanged) {
    chan[extChanOffs].state.alg=ins->fm.alg;
    if (ch==0) {
      chan[extChanOffs].state.fb=ins->fm.fb;
    }
    chan[extChanOffs].state.fms=ins->fm.fms;
    chan[extChanOffs].state.ams=ins->fm.ams;
    chan[extChanOffs].state.fms2=ins->fm.fms2;
    chan[extChanOffs].state.ams2=ins->fm.ams2;
    chan[extChanOffs].state.fms3=ins->fm.fms3;
    chan[extChanOffs].state.ams3=ins->fm.ams3;
    chan[extChanOffs].state.lfoRate=ins->fm.lfoRate;
    chan[extChanOffs].state.lfoFmDepth=ins->fm.lfoFmDepth;
    chan[extChanOffs].state.lfoAmDepth=ins->fm.lfoAmDepth;
    chan[extChanOffs].state.lfoWs=ins->fm.lfoWs;
    chan[extChanOffs].state.lfoNoise=ins->fm.lfoNoise;
    chan[extChanOffs].state.lfoSync=ins->fm.lfoSync;
    chan[extChanOffs].state.lfoRate2=ins->fm.lfoRate2;
    chan[extChanOffs].state.lfoFmDepth2=ins->fm.lfoFmDepth2;
    chan[extChanOffs].state.lfoAmDepth2=ins->fm.lfoAmDepth2;
    chan[extChanOffs].state.lfoWs2=ins->fm.lfoWs2;
    chan[extChanOffs].state.lfoNoise2=ins->fm.lfoNoise2;
    chan[extChanOffs].state.lfoSync2=ins->fm.lfoSync2;
    chan[extChanOffs].state.block=ins->fm.block;
    chan[extChanOffs].state.op[ordch]=ins->fm.op[ordch];
  }
  
  unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[ordch];
  DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[ordch];
  // TODO: how does this work?!
  if (isOpMuted[ch] || !op.enable) {
    rWrite(baseAddr+0x40,127);
  } else {
    if (opChan[ch].insChanged) {
      rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch].outVol&0x7f,127));
    }
  }
  if (opChan[ch].insChanged) {
    rWrite(baseAddr+0x30,(op.mult&15)|(dtTable[op.dt&7]<<4));
    rWrite(baseAddr+0x50,(op.ar&31)|(op.rs<<6));
    rWrite(baseAddr+0x60,(op.dr&31)|(op.am<<7));
    rWrite(baseAddr+0x70,op.d2r&31);
    rWrite(baseAddr+0x80,(op.rr&15)|(op.sl<<4));
    rWrite(baseAddr+0x90,(op.ssgEnv&15)|((op.ws&15)<<4));
    opChan[ch].mask=op.enable;
  }
  if (opChan[ch].insChanged) { // TODO how does this work?
    rWrite(chanOffs[extChanOffs]+0xb0,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
    rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_R,chan[extChanOffs].state.lfoRate);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_FMD,chan[extChanOffs].state.lfoFmDepth);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_AMD,chan[extChanOffs].state.lfoAmDepth);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_NOI,chan[extChanOffs].state.lfoNoise);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_R,chan[extChanOffs].state.lfoRate2);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_FMD,chan[extChanOffs].state.lfoFmDepth2);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_AMD,chan[extChanOffs].state.lfoAmDepth2);
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
    rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_NOI,chan[extChanOffs].state.lfoNoise2);
  }
}

int DivPlatformYM2610XExt::dispatch(DivCommand c) {
  if (c.chan<extChanOffs) {
    return DivPlatformYM2610X::dispatch(c);
  }
  if (c.chan>(extChanOffs+3)) {
    c.chan-=3;
    return DivPlatformYM2610X::dispatch(c);
  }
  int ch=c.chan-extChanOffs;
  int ordch=orderedOps[ch];
  if (!extMode) {
    c.chan=extChanOffs;
    return DivPlatformYM2610X::dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(opChan[ch].ins,DIV_INS_OPNX);

      opChan[ch].macroInit(ins);
      if (!opChan[ch].std.vol.will) {
        opChan[ch].outVol=opChan[ch].vol;
      }

      commitStateExt(ch,ins);
      opChan[ch].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        opChan[ch].baseFreq=NOTE_FNUM_BLOCK(c.value,11,chan[extChanOffs].state.block);
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
      opChan[ch].keyOff=true;
      opChan[ch].keyOn=false;
      opChan[ch].active=false;
      opChan[ch].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      opChan[ch].std.release();
      break;
    case DIV_CMD_VOLUME: {
      opChan[ch].vol=c.value;
      if (!opChan[ch].std.vol.has) {
        opChan[ch].outVol=c.value;
      }
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[ordch];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[ordch];
      if (isOpMuted[ch] || !op.enable) {
        rWrite(baseAddr+0x40,127);
      } else {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch].outVol&0x7f,127));
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
    case DIV_CMD_PANNING: {
      if (c.value==0 && c.value2==0) {
        opChan[ch].pan=3;
      } else {
        opChan[ch].pan=(c.value2>0)|((c.value>0)<<1);
      }
      if (parent->song.compatFlags.sharedExtStat) {
        for (int i=0; i<4; i++) {
          if (ch==i) continue;
          opChan[i].pan=opChan[ch].pan;
        }
      }
      rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
      lastExtChPan=opChan[ch].pan;
      break;
    }
    case DIV_CMD_PITCH: {
      opChan[ch].pitch=c.value;
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (parent->song.compatFlags.linearPitch) {
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
      PLEASE_HELP_ME(opChan[ch],chan[extChanOffs].state.block);
      break;
    }
    case DIV_CMD_LEGATO: {
      if (opChan[ch].insChanged) {
        DivInstrument* ins=parent->getIns(opChan[ch].ins,DIV_INS_OPNX);
        commitStateExt(ch,ins);
        opChan[ch].insChanged=false;
      }
      opChan[ch].baseFreq=NOTE_FNUM_BLOCK(c.value,11,chan[extChanOffs].state.block);
      opChan[ch].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      if (extMode==(bool)c.value) break;
      extMode=c.value;
      immWriteBanked(0x27,extMode?0x40:0);
      if (!extMode) {
        for (int i=0; i<4; i++) {
          opChan[i].insChanged=true;
        }
        chan[extChanOffs].insChanged=true;
      }
      break;
    }
    case DIV_CMD_FM_LFO: {
      lfoValue=(c.value&7)|((c.value>>4)<<3);
      rWrite(0x22,lfoValue);
      break;
    }
    case DIV_CMD_FM_ALG: {
      chan[extChanOffs].state.alg=c.value&7;
      // TODO: TL compensation?
      rWrite(ADDR_FB_ALG+chanOffs[extChanOffs],(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[extChanOffs].state.fb=c.value&7;
      rWrite(chanOffs[extChanOffs]+ADDR_FB_ALG,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_FMS: {
      chan[extChanOffs].state.fms=c.value&7;
      rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_FM_AMS: {
      chan[extChanOffs].state.ams=c.value&3;
      rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_FM_FMS2: {
      chan[extChanOffs].state.fms2=c.value&7;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
      break;
    }
    case DIV_CMD_FM_AMS2: {
      chan[extChanOffs].state.ams2=c.value&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
      break;
    }
    case DIV_CMD_FM_LFOA_WAVEFORM: {
      chan[extChanOffs].state.lfoWs=c.value&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
      break;
    }
    case DIV_CMD_FM_LFOA_SYNC: {
      chan[extChanOffs].state.lfoSync=c.value&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
      break;
    }
    case DIV_CMD_FM_FMS3: {
      chan[extChanOffs].state.fms3=c.value&7;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
      break;
    }
    case DIV_CMD_FM_AMS3: {
      chan[extChanOffs].state.ams3=c.value&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
      break;
    }
    case DIV_CMD_FM_LFOB_WAVEFORM: {
      chan[extChanOffs].state.lfoWs2=c.value&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
      break;
    }
    case DIV_CMD_FM_LFOB_SYNC: {
      chan[extChanOffs].state.lfoSync2=c.value&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
      break;
    }
    case DIV_CMD_FM_LFOA_RATE: {
      chan[extChanOffs].state.lfoRate=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_R,chan[extChanOffs].state.lfoRate);
      break;
    }
    case DIV_CMD_FM_LFOA_FMDEPTH: {
      chan[extChanOffs].state.lfoFmDepth=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_FMD,chan[extChanOffs].state.lfoFmDepth);
      break;
    }
    case DIV_CMD_FM_LFOA_AMDEPTH: {
      chan[extChanOffs].state.lfoAmDepth=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_AMD,chan[extChanOffs].state.lfoAmDepth);
      break;
    }
    case DIV_CMD_FM_LFOA_NOISE: {
      chan[extChanOffs].state.lfoNoise=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_NOI,chan[extChanOffs].state.lfoNoise);
      break;
    }
    case DIV_CMD_FM_LFOB_RATE: {
      chan[extChanOffs].state.lfoRate2=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_R,chan[extChanOffs].state.lfoRate2);
      break;
    }
    case DIV_CMD_FM_LFOB_FMDEPTH: {
      chan[extChanOffs].state.lfoFmDepth2=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_FMD,chan[extChanOffs].state.lfoFmDepth2);
      break;
    }
    case DIV_CMD_FM_LFOB_AMDEPTH: {
      chan[extChanOffs].state.lfoAmDepth2=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_AMD,chan[extChanOffs].state.lfoAmDepth2);
      break;
    }
    case DIV_CMD_FM_LFOB_NOISE: {
      chan[extChanOffs].state.lfoNoise2=c.value&0xff;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_NOI,chan[extChanOffs].state.lfoNoise2);
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
      if (isOpMuted[ch] || !op.enable) {
        rWrite(baseAddr+0x40,127);
      } else if (KVS(2,c.value)) {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[ch].outVol&0x7f,127));
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
          rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.ssgEnv=8^(c.value2&15);
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
      }
      break;
    }
    case DIV_CMD_FM_WS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          op.ws=c.value2&15;
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[c.value]];
        op.ws=c.value2&15;
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
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

void DivPlatformYM2610XExt::tick(bool sysTick) {
  int hardResetElapsed=0;
  bool mustHardReset=false;

  if (extMode) for (int i=0; i<4; i++) {
    opChan[i].std.next();

    if (opChan[i].std.phaseReset.had) {
      if (opChan[i].std.phaseReset.val==1 && opChan[i].active) {
        opChan[i].keyOn=true;
      }
    }
  }

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
        immWriteBanked(baseAddr+ADDR_SL_RR,0x0f);
        hardResetElapsed++;
      }
    }
    if (writeSomething) {
      if (chan[csmChan].active) { // CSM
        writeMask^=0xf0;
      }
      immWriteBanked(0x28,writeMask);
    }
  }

  if (extMode) for (int i=0; i<4; i++) {
    if (opChan[i].std.vol.had) {
      opChan[i].outVol=VOL_SCALE_LOG(opChan[i].vol,MIN(127,opChan[i].std.vol.val),127);
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[orderedOps[i]];
      DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[orderedOps[i]];
      if (isOpMuted[i]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,opChan[i].outVol&0x7f,127));
      }
    }

    if (NEW_ARP_STRAT) {
      opChan[i].handleArp();
    } else if (opChan[i].std.arp.had) {
      if (!opChan[i].inPorta) {
        opChan[i].baseFreq=NOTE_FNUM_BLOCK(parent->calcArp(opChan[i].note,opChan[i].std.arp.val),11,chan[extChanOffs].state.block);
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
      if (!parent->song.compatFlags.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[j];
        if (isOpMuted[j] || !op.enable) {
          rWrite(baseAddr+0x40,127);
        } else {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[j].outVol&0x7f,127));
        }
      }
    }
    if (i==0) {
      if (opChan[i].std.fb.had) {
        chan[extChanOffs].state.fb=opChan[i].std.fb.val;
        rWrite(chanOffs[extChanOffs]+ADDR_FB_ALG,(chan[extChanOffs].state.alg&7)|(chan[extChanOffs].state.fb<<3));
      }
    }
    if (opChan[i].std.fms.had) {
      chan[extChanOffs].state.fms=opChan[i].std.fms.val;
      rWrite(chanOffs[extChanOffs]+ADDR_LRAF,(IS_EXTCH_MUTED?0:(opChan[i].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
    }
    if (opChan[i].std.ams.had) {
      chan[extChanOffs].state.ams=opChan[i].std.ams.val;
      rWrite(chanOffs[extChanOffs]+ADDR_LRAF,(IS_EXTCH_MUTED?0:(opChan[i].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
    }
    if (opChan[i].std.ex3.had) {
      lfoValue=(opChan[i].std.ex3.val>7)?0:(8|(opChan[i].std.ex3.val&7));
      rWrite(0x22,lfoValue);
    }

    if (opChan[i].std.ex5.had) { // per-channel LFOA rate
      chan[extChanOffs].state.lfoRate=opChan[i].std.ex5.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_R,chan[extChanOffs].state.lfoRate);
    }
    if (opChan[i].std.ex6.had) { // per-channel LFOA FM depth
      chan[extChanOffs].state.lfoFmDepth=opChan[i].std.ex6.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_FMD,chan[extChanOffs].state.lfoFmDepth);
    }
    if (opChan[i].std.ex7.had) { // per-channel LFOA AM depth
      chan[extChanOffs].state.lfoAmDepth=opChan[i].std.ex7.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_AMD,chan[extChanOffs].state.lfoAmDepth);
    }
    if (opChan[i].std.ex8.had) { // per-channel LFOA waveform
      chan[extChanOffs].state.lfoWs=opChan[i].std.ex8.val&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
    }
    if (opChan[i].std.ex9.had) { // per-channel LFOA Noise frequency
      chan[extChanOffs].state.lfoNoise=opChan[i].std.ex9.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_NOI,chan[extChanOffs].state.lfoNoise);
    }
    if (opChan[i].std.ex10.had) { // per-channel LFOA FMS
      chan[extChanOffs].state.fms2=opChan[i].std.ex10.val&7;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
    }
    if (opChan[i].std.ex11.had) { // per-channel LFOA AMS
      chan[extChanOffs].state.ams2=opChan[i].std.ex11.val&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
    }
    if (opChan[i].std.ex12.had) { // per-channel LFOA Sync
      chan[extChanOffs].state.lfoSync=opChan[i].std.ex12.val&1;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO_CTL,(chan[extChanOffs].state.lfoSync?128:0)|((chan[extChanOffs].state.fms2&7)<<4)|((chan[extChanOffs].state.lfoWs&3)<<2)|(chan[extChanOffs].state.ams2&3));
    }

    if (opChan[i].std.ex13.had) { // per-channel LFOB rate
      chan[extChanOffs].state.lfoRate2=opChan[i].std.ex13.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_R,chan[extChanOffs].state.lfoRate2);
    }
    if (opChan[i].std.ex14.had) { // per-channel LFOB FM depth
      chan[extChanOffs].state.lfoFmDepth2=opChan[i].std.ex14.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_FMD,chan[extChanOffs].state.lfoFmDepth2);
    }
    if (opChan[i].std.ex15.had) { // per-channel LFOB AM depth
      chan[extChanOffs].state.lfoAmDepth2=opChan[i].std.ex15.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_AMD,chan[extChanOffs].state.lfoAmDepth2);
    }
    if (opChan[i].std.ex16.had) { // per-channel LFOB waveform
      chan[extChanOffs].state.lfoWs2=opChan[i].std.ex16.val&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
    }
    if (opChan[i].std.ex17.had) { // per-channel LFOB Noise frequency
      chan[extChanOffs].state.lfoNoise2=opChan[i].std.ex17.val;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_NOI,chan[extChanOffs].state.lfoNoise2);
    }
    if (opChan[i].std.ex18.had) { // per-channel LFOB FMS
      chan[extChanOffs].state.fms3=opChan[i].std.ex18.val&7;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
    }
    if (opChan[i].std.ex19.had) { // per-channel LFOB AMS
      chan[extChanOffs].state.ams3=opChan[i].std.ex19.val&3;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
    }
    if (opChan[i].std.ex20.had) { // per-channel LFOB Sync
      chan[extChanOffs].state.lfoSync2=opChan[i].std.ex20.val&1;
      rWrite(chanOffs[extChanOffs]+ADDR_PCLFO2_CTL,(chan[extChanOffs].state.lfoSync2?128:0)|((chan[extChanOffs].state.fms3&7)<<4)|((chan[extChanOffs].state.lfoWs2&3)<<2)|(chan[extChanOffs].state.ams3&3));
    }
    if (opChan[i].std.panL.had) {
      opChan[i].pan=opChan[i].std.panL.val&3;
      if (parent->song.compatFlags.sharedExtStat) {
        for (int j=0; j<4; j++) {
          if (i==j) continue;
          opChan[j].pan=opChan[i].pan;
        }
      }
      rWrite(chanOffs[extChanOffs]+ADDR_LRAF,(IS_EXTCH_MUTED?0:(opChan[i].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
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
      op.tl=m.tl.val;
      if (isOpMuted[i]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG(127-op.tl,opChan[i].outVol&0x7f,127));
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
      rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
    }
    if (m.ws.had) {
      op.ws=m.ws.val;
      rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
    }
  }

  DivPlatformYM2610X::tick(sysTick);

  bool writeNoteOn=false;
  unsigned char writeMask=2;
  unsigned char hardResetMask=0;
  if (extMode) for (int i=0; i<4; i++) {
    if (opChan[i].freqChanged) {
      if (parent->song.compatFlags.linearPitch) {
        opChan[i].freq=parent->calcFreq(opChan[i].baseFreq,opChan[i].pitch,opChan[i].fixedArp?opChan[i].baseNoteOverride:opChan[i].arpOff,opChan[i].fixedArp,false,4,opChan[i].pitch2,chipClock,CHIP_FREQBASE,11,chan[extChanOffs].state.block);
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
      immWriteBanked(opChanOffsH[i],opChan[i].freq>>8);
      immWriteBanked(opChanOffsL[i],opChan[i].freq&0xff);
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
  if (extMode) {
    if (chan[csmChan].freqChanged) {
      chan[csmChan].freq=parent->calcFreq(chan[csmChan].baseFreq,chan[csmChan].pitch,chan[csmChan].fixedArp?chan[csmChan].baseNoteOverride:chan[csmChan].arpOff,chan[csmChan].fixedArp,true,0,chan[csmChan].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[csmChan].freq<1) chan[csmChan].freq=1;
      if (chan[csmChan].freq>1024) chan[csmChan].freq=1024;
      int wf=0x400-chan[csmChan].freq;
      immWriteBanked(0x24,wf>>2);
      immWriteBanked(0x25,wf&3);
      chan[csmChan].freqChanged=false;
    }

    if (chan[csmChan].keyOff || chan[csmChan].keyOn) {
      writeNoteOn=true;
      for (int i=0; i<4; i++) {
        writeMask|=opChan[i].active<<(4+i);
      }
    }
  }
  if (writeNoteOn) {
    if (chan[csmChan].active) { // CSM
      writeMask^=0xf0;
    }
    writeMask^=hardResetMask;
    immWriteBanked(0x28,writeMask);
    writeMask^=hardResetMask;

    // hard reset handling
    if (mustHardReset) {
      immWriteBanked(0xfffffffe,hardResetCycles-hardResetElapsed);
      for (int i=0; i<4; i++) {
        if (opChan[i].keyOn && opChan[i].hardReset) {
          // restore SL/RR
          unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[i];
          DivInstrumentFM::Operator& op=chan[extChanOffs].state.op[i];
          immWriteBanked(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          opChan[i].keyOn=false;
        }
      }
      immWriteBanked(0x28,writeMask);
    }
  }

  if (extMode) {
    if (chan[csmChan].keyOn) {
      immWriteBanked(0x27,0x81);
      chan[csmChan].keyOn=false;
    }
    if (chan[csmChan].keyOff) {
      immWriteBanked(0x27,0x40);
      chan[csmChan].keyOff=false;
    }
  }
}

void DivPlatformYM2610XExt::muteChannel(int ch, bool mute) {
  if (ch<extChanOffs) {
    DivPlatformYM2610X::muteChannel(ch,mute);
    return;
  }
  if (ch>(extChanOffs+3)) {
    DivPlatformYM2610X::muteChannel(ch-3,mute);
    return;
  }
  isOpMuted[ch-extChanOffs]=mute;
  
  DivPlatformYM2610X::muteChannel(extChanOffs,IS_EXTCH_MUTED);
  
  if (extMode) {
    for (int i=0; i<4; i++) {
      int ordch=orderedOps[i];
      unsigned short baseAddr=chanOffs[extChanOffs]|opOffs[ordch];
      DivInstrumentFM::Operator op=chan[extChanOffs].state.op[ordch];
      if (isOpMuted[i] || !op.enable) {
        rWrite(baseAddr+0x40,127);
      } else {
        rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[i].outVol&0x7f,127));
      }
    }

    rWrite(chanOffs[extChanOffs]+0xb4,(IS_EXTCH_MUTED?0:(opChan[ch-extChanOffs].pan<<6))|(chan[extChanOffs].state.fms&7)|((chan[extChanOffs].state.ams&3)<<4));
  }
}

void DivPlatformYM2610XExt::forceIns() {
  for (int i=0; i<(psgChanOffs-isCSM); i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      if (i==extChanOffs && extMode) { // extended channel
        if (isOpMuted[orderedOps[j]] || !op.enable) {
          rWrite(baseAddr+0x40,127);
        } else {
          rWrite(baseAddr+0x40,127-VOL_SCALE_LOG(127-op.tl,opChan[orderedOps[j]].outVol&0x7f,127));
        }
      } else {
        if (isMuted[i] || !op.enable) {
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
      rWrite(baseAddr+ADDR_SSG,(op.ssgEnv&15)|((op.ws&15)<<4));
    }
    rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    if (i==extChanOffs) {
      rWrite(chanOffs[i]+ADDR_LRAF,(IS_EXTCH_MUTED?0:(lastExtChPan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    } else {
      rWrite(chanOffs[i]+ADDR_LRAF,(isMuted[i]?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    rWrite(chanOffs[i]+ADDR_PCLFO_R,chan[i].state.lfoRate);
    rWrite(chanOffs[i]+ADDR_PCLFO_FMD,chan[i].state.lfoFmDepth);
    rWrite(chanOffs[i]+ADDR_PCLFO_AMD,chan[i].state.lfoAmDepth);
    rWrite(chanOffs[i]+ADDR_PCLFO_CTL,(chan[i].state.lfoSync?128:0)|((chan[i].state.fms2&7)<<4)|((chan[i].state.lfoWs&3)<<2)|(chan[i].state.ams2&3));
    rWrite(chanOffs[i]+ADDR_PCLFO_NOI,chan[i].state.lfoNoise);
    rWrite(chanOffs[i]+ADDR_PCLFO2_R,chan[i].state.lfoRate2);
    rWrite(chanOffs[i]+ADDR_PCLFO2_FMD,chan[i].state.lfoFmDepth2);
    rWrite(chanOffs[i]+ADDR_PCLFO2_AMD,chan[i].state.lfoAmDepth2);
    rWrite(chanOffs[i]+ADDR_PCLFO2_CTL,(chan[i].state.lfoSync2?128:0)|((chan[i].state.fms3&7)<<4)|((chan[i].state.lfoWs2&3)<<2)|(chan[i].state.ams3&3));
    rWrite(chanOffs[i]+ADDR_PCLFO2_NOI,chan[i].state.lfoNoise2);

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
    if (i.addr>31) continue;
    immWriteBanked(((i.addr&0x10)<<5)|(i.addr&15),i.val);
  }
  ay->getRegisterWrites().clear();
  immWriteBanked(0x22,lfoValue);
  for (int i=0; i<4; i++) {
    opChan[i].insChanged=true;
    if (opChan[i].active) {
      opChan[i].keyOn=true;
      opChan[i].freqChanged=true;
    }
  }
  if (extMode && chan[csmChan].active) { // CSM
    chan[csmChan].insChanged=true;
    chan[csmChan].freqChanged=true;
    chan[csmChan].keyOn=true;
  }
  if (!extMode) {
    immWriteBanked(0x27,0x00);
  }
}

void* DivPlatformYM2610XExt::getChanState(int ch) {
  if (ch>=(extChanOffs+4)) return &chan[ch-3];
  if (ch>=extChanOffs) return &opChan[ch-extChanOffs];
  return &chan[ch];
}

DivMacroInt* DivPlatformYM2610XExt::getChanMacroInt(int ch) {
  if (ch>=(psgChanOffs+3) && ch<(adpcmAChanOffs+3)) return ay->getChanMacroInt(ch-psgChanOffs-3);
  if (ch>=(extChanOffs+4)) return &chan[ch-3].std;
  if (ch>=extChanOffs) return &opChan[ch-extChanOffs].std;
  return &chan[ch].std;
}

unsigned short DivPlatformYM2610XExt::getPan(int ch) {
  if (ch==4+csmChan) return 0;
  if (ch>=4+extChanOffs) return DivPlatformYM2610X::getPan(ch-3);
  if (ch>=extChanOffs) {
    if (extMode) {
      return ((lastExtChPan&2)<<7)|(lastExtChPan&1);
    } else {
      return DivPlatformYM2610X::getPan(extChanOffs);
    }
  }
  return DivPlatformYM2610X::getPan(ch);
}

DivDispatchOscBuffer* DivPlatformYM2610XExt::getOscBuffer(int ch) {
  if (ch>=(extChanOffs+4)) return oscBuf[ch-3];
  if (ch<(extChanOffs+1)) return oscBuf[ch];
  return NULL;
}

int DivPlatformYM2610XExt::mapVelocity(int ch, float vel) {
  if (ch>=extChanOffs+4) return DivPlatformOPN::mapVelocity(ch-3,vel);
  if (ch>=extChanOffs) return DivPlatformOPN::mapVelocity(extChanOffs,vel);
  return DivPlatformOPN::mapVelocity(ch,vel);
}

void DivPlatformYM2610XExt::reset() {
  DivPlatformYM2610X::reset();

  for (int i=0; i<4; i++) {
    opChan[i]=DivPlatformOPN::OPNOpChannelStereo();
    opChan[i].std.setEngine(parent);
    opChan[i].vol=127;
    opChan[i].outVol=127;
  }

  lastExtChPan=3;

  // channel 3 mode
  immWriteBanked(0x27,0x40);
  extMode=true;
}

bool DivPlatformYM2610XExt::keyOffAffectsArp(int ch) {
  return (ch>=(psgChanOffs+3));
}

void DivPlatformYM2610XExt::notifyInsChange(int ins) {
  DivPlatformYM2610X::notifyInsChange(ins);
  for (int i=0; i<4; i++) {
    if (opChan[i].ins==ins) {
      opChan[i].insChanged=true;
    }
  }
}

void DivPlatformYM2610XExt::notifyInsDeletion(void* ins) {
  DivPlatformYM2610X::notifyInsDeletion(ins);
  for (int i=0; i<4; i++) {
    opChan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

int DivPlatformYM2610XExt::init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) {
  DivPlatformYM2610X::init(parent,channels,sugRate,flags);
  for (int i=0; i<4; i++) {
    isOpMuted[i]=false;
  }
  extSys=true;

  reset();
  return 22;
}

void DivPlatformYM2610XExt::quit() {
  DivPlatformYM2610X::quit();
}

DivPlatformYM2610XExt::~DivPlatformYM2610XExt() {
}
