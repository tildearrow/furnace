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

#include "arcade.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

const char* regCheatSheetOPM[]={
  "Test", "00",
  "NoteCtl", "08",
  "NoiseCtl", "0F",
  "ClockA1", "10",
  "ClockA2", "11",
  "ClockB", "12",
  "Control", "14",
  "LFOFreq", "18",
  "AMD_PMD", "19",
  "LFOWave", "1B",
  "L_R_FB_ALG", "20",
  "KC", "28",
  "KF", "30",
  "PMS_AMS", "38",
  "DT_MULT", "40",
  "TL", "60",
  "KS_AR", "80",
  "AM_DR", "A0",
  "DT2_SR", "C0",
  "SL_RR", "E0",
  NULL
};

const char** DivPlatformArcade::getRegisterSheet() {
  return regCheatSheetOPM;
}

void DivPlatformArcade::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  static int o[2];

  for (size_t h=start; h<start+len; h++) {
    for (int i=0; i<8; i++) {
      if (!writes.empty() && !fm.write_busy) {
        QueuedWrite& w=writes.front();
        if (w.addrOrVal) {
          OPM_Write(&fm,1,w.val);
          regPool[w.addr&0xff]=w.val;
          //printf("write: %x = %.2x\n",w.addr,w.val);
          writes.pop_front();
        } else {
          OPM_Write(&fm,0,w.addr);
          w.addrOrVal=true;
        }
      }

      OPM_Clock(&fm,NULL,NULL,NULL,NULL);
      OPM_Clock(&fm,NULL,NULL,NULL,NULL);
      OPM_Clock(&fm,NULL,NULL,NULL,NULL);
      OPM_Clock(&fm,o,NULL,NULL,NULL);
    }

    for (int i=0; i<8; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=fm.ch_out[i];
    }

    if (o[0]<-32768) o[0]=-32768;
    if (o[0]>32767) o[0]=32767;

    if (o[1]<-32768) o[1]=-32768;
    if (o[1]>32767) o[1]=32767;

    bufL[h]=o[0];
    bufR[h]=o[1];
  }
}

void DivPlatformArcade::acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  ymfm::ym2151::fm_engine* fme=fm_ymfm->debug_engine();

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      if (--delay<1) {
        QueuedWrite& w=writes.front();
        fm_ymfm->write(0x0+((w.addr>>8)<<1),w.addr);
        fm_ymfm->write(0x1+((w.addr>>8)<<1),w.val);
        regPool[w.addr&0xff]=w.val;
        writes.pop_front();
        delay=1;
      }
    }

    fm_ymfm->generate(&out_ymfm);

    for (int i=0; i<8; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(fme->debug_channel(i)->debug_output(0)+fme->debug_channel(i)->debug_output(1));
    }

    os[0]=out_ymfm.data[0];
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=out_ymfm.data[1];
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;

    bufL[h]=os[0];
    bufR[h]=os[1];
  }
}

void DivPlatformArcade::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (useYMFM) {
    acquire_ymfm(bufL,bufR,start,len);
  } else {
    acquire_nuked(bufL,bufR,start,len);
  }
}

static unsigned char noteMap[12]={
  0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14
};

inline int hScale(int note) {
  return ((note/12)<<4)+(noteMap[note%12]);
}

void DivPlatformArcade::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(127,chan[i].std.vol.val),127);
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (KVS(i,j)) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[i].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_LINEAR(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.duty.had) {
      if (chan[i].std.duty.val>0) {
        rWrite(0x0f,0x80|(0x20-chan[i].std.duty.val));
      } else {
        rWrite(0x0f,0);
      }
    }

    if (chan[i].std.wave.had) {
      rWrite(0x1b,chan[i].std.wave.val&3);
    }

    if (chan[i].std.panL.had) {
      chan[i].chVolL=(chan[i].std.panL.val&2)>>1;
      chan[i].chVolR=chan[i].std.panL.val&1;
      if (isMuted[i]) {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      } else {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
      }
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].active) {
        chan[i].keyOn=true;
      }
    }

    if (chan[i].std.ex1.had) {
      amDepth=chan[i].std.ex1.val;
      immWrite(0x19,amDepth);
    }

    if (chan[i].std.ex2.had) {
      pmDepth=chan[i].std.ex2.val;
      immWrite(0x19,0x80|pmDepth);
    }

    if (chan[i].std.ex3.had) {
      immWrite(0x18,chan[i].std.ex3.val);
    }

    if (chan[i].std.ex1.had || chan[i].std.ex2.had || chan[i].std.ex3.had) {
      immWrite(0x01,0x00); // LFO On
    }

    if (chan[i].std.alg.had) {
      chan[i].state.alg=chan[i].std.alg.val;
      if (isMuted[i]) {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      } else {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
      }
      if (!parent->song.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
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
    }
    if (chan[i].std.fb.had) {
      chan[i].state.fb=chan[i].std.fb.val;
      if (isMuted[i]) {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
      } else {
        rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
      }
    }
    if (chan[i].std.fms.had) {
      chan[i].state.fms=chan[i].std.fms.val;
      rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    }
    if (chan[i].std.ams.had) {
      chan[i].state.ams=chan[i].std.ams.val;
      rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    }
    if (chan[i].std.ex4.had && chan[i].active) {
      chan[i].opMask=chan[i].std.ex4.val&15;
      chan[i].opMaskChanged=true;
    }
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[i].state.op[j];
      DivMacroInt::IntOp& m=chan[i].std.op[j];
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
        if (KVS(i,j)) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[i].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
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
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
      if (m.dt2.had) {
        op.dt2=m.dt2.val;
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
    }
    if (chan[i].keyOn || chan[i].keyOff) {
      if (chan[i].hardReset && chan[i].keyOn) {
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          immWrite(baseAddr+ADDR_SL_RR,0x0f);
          immWrite(baseAddr+ADDR_TL,0x7f);
          oldWrites[baseAddr+ADDR_SL_RR]=-1;
          oldWrites[baseAddr+ADDR_TL]=-1;
        }
      }
      immWrite(0x08,i);
      if (chan[i].hardReset && chan[i].keyOn) {
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          for (int k=0; k<9; k++) {
            immWrite(baseAddr+ADDR_SL_RR,0x0f);
          }
        }
      }
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+(chan[i].pitch>>1)-64+chan[i].pitch2;
      if (!parent->song.oldArpStrategy) {
        if (chan[i].fixedArp) {
          chan[i].freq=(chan[i].baseNoteOverride<<6)+(chan[i].pitch>>1)-64+chan[i].pitch2;
        } else {
          chan[i].freq+=chan[i].arpOff<<6;
        }
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>=(95<<6)) chan[i].freq=(95<<6)-1;
      immWrite(i+0x28,hScale(chan[i].freq>>6));
      immWrite(i+0x30,chan[i].freq<<2);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn || chan[i].opMaskChanged) {
      immWrite(0x08,(chan[i].opMask<<3)|i);
      chan[i].opMaskChanged=false;
      chan[i].keyOn=false;
    }
  }
}

void DivPlatformArcade::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (isMuted[ch]) {
    rWrite(chanOffs[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3));
  } else {
    rWrite(chanOffs[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3)|((chan[ch].chVolL&1)<<6)|((chan[ch].chVolR&1)<<7));
  }
}

int DivPlatformArcade::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
        chan[c.chan].opMask=
          (chan[c.chan].state.op[0].enable?1:0)|
          (chan[c.chan].state.op[2].enable?2:0)|
          (chan[c.chan].state.op[1].enable?4:0)|
          (chan[c.chan].state.op[3].enable?8:0);
      }

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=chan[c.chan].state.op[i];
        if (KVS(c.chan,i)) {
          if (!chan[c.chan].active || chan[c.chan].insChanged) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[c.chan].outVol&0x7f,127));
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
          rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      }
      if (chan[c.chan].insChanged) {
        if (isMuted[c.chan]) {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        } else {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
        }
        rWrite(chanOffs[c.chan]+ADDR_FMS_AMS,((chan[c.chan].state.fms&7)<<4)|(chan[c.chan].state.ams&3));
      }
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_LINEAR(c.value);
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (KVS(c.chan,i)) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[c.chan].outVol&0x7f,127));
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
      chan[c.chan].chVolL=(c.value>0);
      chan[c.chan].chVolR=(c.value2>0);
      if (isMuted[c.chan]) {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      } else {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
      }
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_LINEAR(c.value2);
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
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_LINEAR(c.value);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      if(c.value==0) {
        rWrite(0x01,0x02);
      }
      else {
        rWrite(0x01,0x00);
      }
      rWrite(0x18,c.value);
      break;
    }
    case DIV_CMD_FM_LFO_WAVE: {
      rWrite(0x1b,c.value&3);
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[c.chan].state.fb=c.value&7;
      if (isMuted[c.chan]) {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      } else {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
      }
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
      if (KVS(c.chan,c.value)) {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[c.chan].outVol&0x7f,127));
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
    case DIV_CMD_FM_RS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.rs=c.value2&3;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.rs=c.value2&3;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.am=c.value2&1;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.am=c.value2&1;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.dr=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.dr=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.sl=c.value2&15;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.sl=c.value2&15;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.rr=c.value2&15;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.rr=c.value2&15;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      }
      break;
    }
    case DIV_CMD_FM_DT2: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.dt2=c.value2&3;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.dt2=c.value2&3;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
      break;
    }
    case DIV_CMD_FM_D2R: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.d2r=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.d2r=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
      break;
    }
    case DIV_CMD_FM_DT: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.dt=c.value&7;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.dt=c.value2&7;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      }
      break;
    }
    case DIV_CMD_FM_AM_DEPTH: {
      amDepth=c.value;
      immWrite(0x19,amDepth);
      break;
    }
    case DIV_CMD_FM_PM_DEPTH: {
      pmDepth=c.value;
      immWrite(0x19,0x80|pmDepth);
      break;
    }
    case DIV_CMD_FM_HARD_RESET:
      chan[c.chan].hardReset=c.value;
      break;
    case DIV_CMD_STD_NOISE_FREQ: {
      if (c.chan!=7) break;
      if (c.value) {
        if (c.value>0x1f) {
          rWrite(0x0f,0x80);
        } else {
          rWrite(0x0f,0x80|(0x1f-c.value));
        }
      } else {
        rWrite(0x0f,0);
      }
      break;
    }
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_LINEAR(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      break;
    default:
      //printf("WARNING: unimplemented command %d\n",c.cmd);
      break;
  }
  return 1;
}

void DivPlatformArcade::forceIns() {
  for (int i=0; i<8; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator op=chan[i].state.op[j];
      if (KVS(i,j)) {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[i].outVol&0x7f,127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
    }
    if (isMuted[i]) {
      rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    } else {
      rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
    }
    rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
}

void DivPlatformArcade::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void* DivPlatformArcade::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformArcade::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformArcade::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformArcade::getRegisterPool() {
  return regPool;
}

int DivPlatformArcade::getRegisterPoolSize() {
  return 256;
}

void DivPlatformArcade::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformArcade::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformArcade::reset() {
  while (!writes.empty()) writes.pop_front();
  memset(regPool,0,256);
  if (useYMFM) {
    fm_ymfm->reset();
  } else {
    memset(&fm,0,sizeof(opm_t));
    OPM_Reset(&fm);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformArcade::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }

  for (int i=0; i<256; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  delay=0;
  amDepth=0x7f;
  pmDepth=0x7f;

  //rWrite(0x18,0x10);
  immWrite(0x01,0x02); // LFO Off
  immWrite(0x18,0x00); // LFO Freq Off
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
  //rWrite(0x1b,0x00);
}

void DivPlatformArcade::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 1:
      chipClock=COLOR_PAL*4.0/5.0;
      baseFreqOff=12;
      break;
    case 2:
      chipClock=4000000.0;
      baseFreqOff=-122;
      break;
    default:
      chipClock=COLOR_NTSC;
      baseFreqOff=0;
      break;
  }
  rate=chipClock/64;
  for (int i=0; i<8; i++) {
    oscBuf[i]->rate=rate;
  }
}

bool DivPlatformArcade::isStereo() {
  return true;
}

void DivPlatformArcade::setYMFM(bool use) {
  useYMFM=use;
}

int DivPlatformArcade::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  if (useYMFM) fm_ymfm=new ymfm::ym2151(iface);
  reset();

  return 8;
}

void DivPlatformArcade::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
  if (useYMFM) {
    delete fm_ymfm;
  }
}

DivPlatformArcade::~DivPlatformArcade() {
}
