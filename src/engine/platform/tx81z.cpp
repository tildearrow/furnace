/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

#include "tx81z.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

// actually 0x40 but the upper bit of data selects address
#define ADDR_WS_FINE 0x100
// actually 0xc0 but bit 5 of data selects address
#define ADDR_EGS_REV 0x120
// actually 0x38 but bits 7 and 2 select address
#define ADDR_FMS2_AMS2 0x140

const char* regCheatSheetOPZ[]={
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

const char** DivPlatformTX81Z::getRegisterSheet() {
  return regCheatSheetOPZ;
}

void DivPlatformTX81Z::acquire(short** buf, size_t len) {
  thread_local int os[2];

  ymfm::ym2414::fm_engine* fme=fm_ymfm->debug_engine();

  for (size_t h=0; h<len; h++) {
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
      oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fme->debug_channel(i)->debug_output(0)+fme->debug_channel(i)->debug_output(1),-32768,32767);
    }

    os[0]=out_ymfm.data[0];
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    os[1]=out_ymfm.data[1];
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    buf[0][h]=os[0];
    buf[1][h]=os[1];
  }
}

static unsigned char noteMap[12]={
  0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14
};

inline int hScale(int note) {
  return ((note/12)<<4)+(noteMap[note%12]);
}

int DivPlatformTX81Z::toFreq(int freq) {
  int block=0;
  while (freq>0xff) {
    freq>>=1;
    block++;
  }
  return ((block&7)<<8)|(freq&0xff);
}

void DivPlatformTX81Z::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(127,chan[i].std.vol.val),127);
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i] || !op.enable) {
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
        rWrite(0x0f,0x80|(chan[i].std.duty.val-1));
      } else {
        rWrite(0x0f,0);
      }
    }

    if (chan[i].std.wave.had) {
      lfoShape=chan[i].std.wave.val&3;
      immWrite(0x1b,lfoShape|(lfoShape2<<2));
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val*(brokenPitch?2:1);
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val*(brokenPitch?2:1);
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.panL.had) {
      chan[i].chVolL=(chan[i].std.panL.val&2)>>1;
      chan[i].chVolR=chan[i].std.panL.val&1;
      chan[i].freqChanged=true;

      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|(chan[i].active?0x40:0)|(chan[i].chVolR<<7));
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
      lfoValue=chan[i].std.ex3.val;
      immWrite(0x18,lfoValue);
    }

    if (chan[i].std.ex5.had) {
      amDepth2=chan[i].std.ex5.val;
      immWrite(0x17,amDepth2);
    }

    if (chan[i].std.ex6.had) {
      pmDepth2=chan[i].std.ex6.val;
      immWrite(0x17,0x80|pmDepth2);
    }

    if (chan[i].std.ex7.had) {
      lfoValue2=chan[i].std.ex7.val;
      immWrite(0x16,lfoValue2);
    }

    if (chan[i].std.ex8.had) {
      lfoShape2=chan[i].std.ex8.val&3;
      immWrite(0x1b,lfoShape|(lfoShape2<<2));
    }

    if (chan[i].std.alg.had) {
      chan[i].state.alg=chan[i].std.alg.val;
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|(chan[i].active?0x40:0)|(chan[i].chVolR<<7));
      if (!parent->song.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i] || !op.enable) {
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
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|(chan[i].active?0x40:0)|(chan[i].chVolR<<7));
    }
    if (chan[i].std.fms.had) {
      chan[i].state.fms=chan[i].std.fms.val;
      rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    }
    if (chan[i].std.ams.had) {
      chan[i].state.ams=chan[i].std.ams.val;
      rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
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
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      }
      if (m.dr.had) {
        op.dr=m.dr.val;
        rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      }
      if (m.mult.had) {
        op.mult=m.mult.val;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
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
        if (isMuted[i] || !op.enable) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (KVS(i,j)) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[i].outVol&0x7f,127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
      if (m.rs.had) {
        op.rs=m.rs.val;
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      }
      if (m.dt.had) {
        op.dt=m.dt.val;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
      }
      if (m.d2r.had) {
        op.d2r=m.d2r.val;
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }
      if (m.dt2.had) {
        op.dt2=m.dt2.val;
        rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      }

      /*if (ImGui::InputInt(_("Block"),&block,1,1)) {
                  if (block<0) block=0;
                  if (block>7) block=7;
                  op.dt=block;
                }
                if (ImGui::InputInt(_("FreqNum"),&freqNum,1,16)) {
                  if (freqNum<0) freqNum=0;
                  if (freqNum>255) freqNum=255;
                  op.mult=freqNum>>4;
                  op.dvb=freqNum&15;
                }*/

      // fixed pitch
      bool freqChangeOp = false;

      if(op.egt)
      {
        if (op.sus) {
          chan[i].handleArpFmOp(freqChangeOp, 0, j); //arp and pitch macros
          chan[i].handlePitchFmOp(freqChangeOp, j);
        } else {
          if (m.ssg.had) { //block and f-num macros
            op.dt=m.ssg.val&7;
            rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
          }
          if (m.sus.had) {
            op.mult=(m.sus.val & 0xff) >> 4;
            op.dvb=(m.sus.val & 0xf);
            rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
            rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
          }
        }
      }

      if(freqChangeOp)
      {
        int arp=chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff;
        int pitch2=chan[i].pitch2;
        int fixedArp=chan[i].fixedArp;
        if(chan[i].opsState[j].hasOpArp) {
          arp=chan[i].opsState[j].fixedArp?chan[i].opsState[j].baseNoteOverride:chan[i].opsState[j].arpOff;
          fixedArp=chan[i].opsState[j].fixedArp;
        }
        if(chan[i].opsState[j].hasOpPitch) {
          pitch2=chan[i].opsState[j].pitch2;
        }
        int opFreq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,arp,fixedArp,false,2, pitch2,chipClock,(524288.0 / 4.0) * 2093.0 / 2192.0 * chipClock / 4000000.0,0);
        if (opFreq<0) opFreq=0;
        if (opFreq>65280) opFreq=65280;
        int freqt=toFreq(opFreq);

        op.dt=(freqt >> 8) & 7;

        op.mult=(freqt & 0xff) >> 4;
        op.dvb=(freqt & 0xf);

        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
        rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
      }
    }
  }

  for (int i=0; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }
  for (int i=256; i<288; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(0x40+(i&0x1f),0x80|(pendingWrites[i]&0x7f));
      oldWrites[i]=pendingWrites[i];
    }
  }
  for (int i=288; i<320; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(0xc0+(i&0x1f),0x20|(pendingWrites[i]&0xdf));
      oldWrites[i]=pendingWrites[i];
    }
  }
  for (int i=320; i<328; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(0x38+(i&7),(0x84|pendingWrites[i]));
      oldWrites[i]=pendingWrites[i];
    }
  }

  int hardResetElapsed=0;
  bool mustHardReset=false;

  for (int i=0; i<8; i++) {
    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|0x00|(chan[i].chVolR<<7));
      if (chan[i].hardReset && chan[i].keyOn) {
        mustHardReset=true;
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          immWrite(baseAddr+ADDR_SL_RR,0x0f);
          hardResetElapsed++;
        }
      }
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+chan[i].pitch-128+chan[i].pitch2;
      if (!parent->song.oldArpStrategy) {
        if (chan[i].fixedArp) {
          chan[i].freq=(chan[i].baseNoteOverride<<7)+chan[i].pitch-128+chan[i].pitch2;
        } else {
          chan[i].freq+=chan[i].arpOff<<7;
        }
      }
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>=(95<<7)) chan[i].freq=(95<<7)-1;
      immWrite(i+0x28,hScale(chan[i].freq>>7));
      immWrite(i+0x30,((chan[i].freq<<1)&0xfc)|(chan[i].chVolL==chan[i].chVolR));
      hardResetElapsed+=2;
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn && !chan[i].hardReset) {
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|0x40|(chan[i].chVolR<<7));
      chan[i].keyOn=false;
    }
  }

  // hard reset handling
  if (mustHardReset) {
    for (unsigned int i=hardResetElapsed; i<hardResetCycles; i++) {
      immWrite(0x1f,i&0xff);
    }
    for (int i=0; i<8; i++) {
      if (chan[i].keyOn && chan[i].hardReset) {
        // restore SL/RR
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          DivInstrumentFM::Operator& op=chan[i].state.op[j];
          immWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }

        immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|0x40|(chan[i].chVolR<<7));
        chan[i].keyOn=false;
      }
    }
  }
}

void DivPlatformTX81Z::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  for (int i=0; i<4; i++) {
    unsigned short baseAddr=chanOffs[ch]|opOffs[i];
    DivInstrumentFM::Operator op=chan[ch].state.op[i];
    if (isMuted[ch] || !op.enable) {
      rWrite(baseAddr+ADDR_TL,127);
    } else {
      if (KVS(ch,i)) {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[ch].outVol&0x7f,127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
    }
  }
}

void DivPlatformTX81Z::commitState(int ch, DivInstrument* ins) {
  if (chan[ch].insChanged) {
    chan[ch].state=ins->fm;
  }

  for (int i=0; i<4; i++) {
    unsigned short baseAddr=chanOffs[ch]|opOffs[i];
    DivInstrumentFM::Operator op=chan[ch].state.op[i];
    if (isMuted[ch] || !op.enable) {
      rWrite(baseAddr+ADDR_TL,127);
    } else {
      if (KVS(ch,i)) {
        if (!chan[ch].active || chan[ch].insChanged) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[ch].outVol&0x7f,127));
        }
      } else {
        if (chan[ch].insChanged) {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
    }
    if (chan[ch].insChanged) {
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
      rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
    }
  }
  if (chan[ch].insChanged) {
    /*
    if (isMuted[ch]) {
      rWrite(chanOffs[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3));
    } else {
      rWrite(chanOffs[ch]+ADDR_LR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3)|((chan[ch].chVolL&1)<<6)|((chan[ch].chVolR&1)<<7));
    }*/
    rWrite(chanOffs[ch]+ADDR_FMS_AMS,((chan[ch].state.fms&7)<<4)|(chan[ch].state.ams&3));
    rWrite(chanOffs[ch]+ADDR_FMS2_AMS2,((chan[ch].state.fms2&7)<<4)|(chan[ch].state.ams2&3));
  }
}

int DivPlatformTX81Z::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_OPZ);

      memset(chan[c.chan].opsState, 0, sizeof(chan[c.chan].opsState));
      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      commitState(c.chan,ins);
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
        if (isMuted[c.chan] || !op.enable) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (KVS(c.chan,i)) {
            rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[c.chan].outVol&0x7f,127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
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
      chan[c.chan].freqChanged=true;

      immWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|(chan[c.chan].active?0x40:0)|(chan[c.chan].chVolR<<7));
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
        newFreq=chan[c.chan].baseFreq+c.value*(brokenPitch?2:1);
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*(brokenPitch?2:1);
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
      if (chan[c.chan].insChanged) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_OPZ);
        commitState(c.chan,ins);
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].baseFreq=NOTE_LINEAR(c.value);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_LFO: {
      lfoValue=c.value;
      immWrite(0x18,lfoValue);
      break;
    }
    case DIV_CMD_FM_LFO_WAVE: {
      lfoShape=c.value&3;
      immWrite(0x1b,lfoShape|(lfoShape2<<2));
      break;
    }
    case DIV_CMD_FM_LFO2: {
      lfoValue2=c.value;
      immWrite(0x16,lfoValue2);
      break;
    }
    case DIV_CMD_FM_LFO2_WAVE: {
      lfoShape2=c.value&3;
      immWrite(0x1b,lfoShape|(lfoShape2<<2));
      break;
    }
    case DIV_CMD_FM_FB: {
      chan[c.chan].state.fb=c.value&7;
      /*
      if (isMuted[c.chan]) {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      } else {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
      }*/
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      if (!op.egt) {
        op.mult=c.value2&15;
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
      }
      break;
    }
    case DIV_CMD_FM_TL: {
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isMuted[c.chan] || !op.enable) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        if (KVS(c.chan,c.value)) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[c.chan].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
        }
      } else {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.rs=c.value2&3;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.rs=c.value2&3;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
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
          if (!op.egt) {
            op.dt=c.value&7;
            unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
            rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
          }
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        if (!op.egt) {
          op.dt=c.value2&7;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
        }
      }
      break;
    }
    case DIV_CMD_FM_EG_SHIFT: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ksl=c.value2&3;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ksl=c.value2&3;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
      }
      break;
    }
    case DIV_CMD_FM_REV: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.dam=c.value2&7;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.dam=c.value2&7;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
      }
      break;
    }
    case DIV_CMD_FM_WS: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ws=c.value2&7;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ws=c.value2&7;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
      }
      break;
    }
    case DIV_CMD_FM_FINE: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          if (!op.egt) {
            op.dvb=c.value2&15;
            unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
            rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
          }
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        if (!op.egt) {
          op.dvb=c.value2&15;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
          rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
        }
      }
      break;
    }
    case DIV_CMD_FM_FIXFREQ: {
      if (c.value<0 || c.value>3) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.egt=(c.value2>0);
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      if (op.egt) {
        rWrite(baseAddr+ADDR_MULT_DT,((c.value2>>4)&15)|(((c.value2>>8)&7)<<4));
        rWrite(baseAddr+ADDR_WS_FINE,(c.value2&15)|(op.ws<<4));
      } else {
        rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
        rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
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
    case DIV_CMD_FM_AM2_DEPTH: {
      amDepth2=c.value;
      immWrite(0x17,amDepth);
      break;
    }
    case DIV_CMD_FM_PM2_DEPTH: {
      pmDepth2=c.value;
      immWrite(0x17,0x80|pmDepth);
      break;
    }
    case DIV_CMD_FM_HARD_RESET:
      chan[c.chan].hardReset=c.value;
      break;
    case DIV_CMD_STD_NOISE_FREQ: {
      if (c.chan!=7) break;
      if (c.value) {
        if (c.value>0x1f) {
          rWrite(0x0f,0x80|0x1f);
        } else {
          rWrite(0x0f,0x80|(c.value-1));
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
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

void DivPlatformTX81Z::forceIns() {
  for (int i=0; i<8; i++) {
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[i]|opOffs[j];
      DivInstrumentFM::Operator op=chan[i].state.op[j];
      if (isMuted[i] || !op.enable) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        if (KVS(i,j)) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[i].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
      rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
    }
    /*
    if (isMuted[i]) {
      rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    } else {
      rWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|((chan[i].chVolL&1)<<6)|((chan[i].chVolR&1)<<7));
    }*/
    rWrite(chanOffs[i]+ADDR_FMS_AMS,((chan[i].state.fms&7)<<4)|(chan[i].state.ams&3));
    rWrite(chanOffs[i]+ADDR_FMS2_AMS2,((chan[i].state.fms2&7)<<4)|(chan[i].state.ams2&3));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
  immWrite(0x17,amDepth2);
  immWrite(0x17,0x80|pmDepth2);
  immWrite(0x18,lfoValue);
  immWrite(0x16,lfoValue2);
  immWrite(0x1b,lfoShape|(lfoShape2<<2));
}

void DivPlatformTX81Z::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformTX81Z::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformTX81Z::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformTX81Z::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformTX81Z::getPan(int ch) {
  return (chan[ch].chVolL<<8)|(chan[ch].chVolR);
}

DivDispatchOscBuffer* DivPlatformTX81Z::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformTX81Z::getRegisterPool() {
  return regPool;
}

int DivPlatformTX81Z::getRegisterPoolSize() {
  return 330;
}

void DivPlatformTX81Z::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformTX81Z::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformTX81Z::reset() {
  writes.clear();
  memset(regPool,0,330);
  fm_ymfm->reset();
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformTX81Z::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }

  for (int i=0; i<330; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  pcmCycles=0;
  pcmL=0;
  pcmR=0;
  delay=0;
  amDepth=0x7f;
  pmDepth=0x7f;
  amDepth2=0x7f;
  pmDepth2=0x7f;
  lfoValue=0;
  lfoValue2=0;
  lfoShape=0;
  lfoShape2=0;

  immWrite(0x18,0x00); // LFO Freq Off
  immWrite(0x16,0x00);
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
  immWrite(0x17,amDepth2);
  immWrite(0x17,0x80|pmDepth2);

  extMode=false;
}

void DivPlatformTX81Z::setFlags(const DivConfig& flags) {
  int clockSel=flags.getInt("clockSel",0);
  if (clockSel==2) {
    chipClock=4000000.0;
  } else if (clockSel==1) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  CHECK_CUSTOM_CLOCK;

  baseFreqOff=round(1536.0*(log((COLOR_NTSC/(double)chipClock))/log(2.0)));

  brokenPitch=flags.getBool("brokenPitch",false);

  rate=chipClock/64;
  for (int i=0; i<8; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformTX81Z::getOutputCount() {
  return 2;
}

int DivPlatformTX81Z::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  fm_ymfm=new ymfm::ym2414(iface);
  reset();

  return 8;
}

void DivPlatformTX81Z::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
  delete fm_ymfm;
}

DivPlatformTX81Z::~DivPlatformTX81Z() {
}
