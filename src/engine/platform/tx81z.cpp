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

#include "tx81z.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#include "fmshared_OPM.h"

// actually 0x40 but the upper bit of data selects address
#define ADDR_WS_FINE 0x100
// actually 0xc0 but bit 5 of data selects address
#define ADDR_EGS_REV 0x120

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
  7,6,5,0,1,2,3,4
};

static int orderedOps[4]={
  0,2,1,3
};

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define NOTE_LINEAR(x) (((x)<<6)+baseFreqOff+log2(parent->song.tuning/440.0)*12.0*64.0)

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

const char* DivPlatformTX81Z::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Set noise frequency (xx: value; 0 disables noise)";
      break;
    case 0x11:
      return "11xx: Set feedback (0 to 7)";
      break;
    case 0x12:
      return "12xx: Set level of operator 1 (0 highest, 7F lowest)";
      break;
    case 0x13:
      return "13xx: Set level of operator 2 (0 highest, 7F lowest)";
      break;
    case 0x14:
      return "14xx: Set level of operator 3 (0 highest, 7F lowest)";
      break;
    case 0x15:
      return "15xx: Set level of operator 4 (0 highest, 7F lowest)";
      break;
    case 0x16:
      return "16xy: Set operator multiplier (x: operator from 1 to 4; y: multiplier)";
      break;
    case 0x17:
      return "17xx: Set LFO speed";
      break;
    case 0x18:
      return "18xx: Set LFO waveform (0 saw, 1 square, 2 triangle, 3 noise)";
      break;
    case 0x19:
      return "19xx: Set attack of all operators (0 to 1F)";
      break;
    case 0x1a:
      return "1Axx: Set attack of operator 1 (0 to 1F)";
      break;
    case 0x1b:
      return "1Bxx: Set attack of operator 2 (0 to 1F)";
      break;
    case 0x1c:
      return "1Cxx: Set attack of operator 3 (0 to 1F)";
      break;
    case 0x1d:
      return "1Dxx: Set attack of operator 4 (0 to 1F)";
      break;
    case 0x1e:
      return "1Exx: Set AM depth (0 to 7F)";
      break;
    case 0x1f:
      return "1Fxx: Set PM depth (0 to 7F)";
      break;
    case 0x28:
      return "28xy: Set reverb (x: operator from 1 to 4 (0 for all ops); y: reverb from 0 to 7)";
      break;
    case 0x2a:
      return "2Axy: Set waveform (x: operator from 1 to 4 (0 for all ops); y: waveform from 0 to 7)";
      break;
    case 0x2b:
      return "2Bxy: Set envelope generator shift (x: operator from 1 to 4 (0 for all ops); y: shift from 0 to 3)";
      break;
    case 0x2c:
      return "2Cxy: Set fine multiplier (x: operator from 1 to 4 (0 for all ops); y: fine)";
      break;
    case 0x2f:
      return "2Fxx: Toggle hard envelope reset on new notes";
      break;
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x34: case 0x35: case 0x36: case 0x37:
      return "3xyy: Set fixed frequency of operator 1 (x: octave from 0 to 7; y: frequency)";
      break;
    case 0x38: case 0x39: case 0x3a: case 0x3b:
    case 0x3c: case 0x3d: case 0x3e: case 0x3f:
      return "3xyy: Set fixed frequency of operator 2 (x: octave from 8 to F; y: frequency)";
      break;
    case 0x40: case 0x41: case 0x42: case 0x43:
    case 0x44: case 0x45: case 0x46: case 0x47:
      return "4xyy: Set fixed frequency of operator 3 (x: octave from 0 to 7; y: frequency)";
      break;
    case 0x48: case 0x49: case 0x4a: case 0x4b:
    case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      return "4xyy: Set fixed frequency of operator 4 (x: octave from 8 to F; y: frequency)";
      break;
    case 0x50:
      return "50xy: Set AM (x: operator from 1 to 4 (0 for all ops); y: AM)";
      break;
    case 0x51:
      return "51xy: Set sustain level (x: operator from 1 to 4 (0 for all ops); y: sustain)";
      break;
    case 0x52:
      return "52xy: Set release (x: operator from 1 to 4 (0 for all ops); y: release)";
      break;
    case 0x53:
      return "53xy: Set detune (x: operator from 1 to 4 (0 for all ops); y: detune where 3 is center)";
      break;
    case 0x54:
      return "54xy: Set envelope scale (x: operator from 1 to 4 (0 for all ops); y: scale from 0 to 3)";
      break;
    case 0x55:
      return "55xy: Set detune 2 (x: operator from 1 to 4 (0 for all ops); y: detune from 0 to 3)";
      break;
    case 0x56:
      return "56xx: Set decay of all operators (0 to 1F)";
      break;
    case 0x57:
      return "57xx: Set decay of operator 1 (0 to 1F)";
      break;
    case 0x58:
      return "58xx: Set decay of operator 2 (0 to 1F)";
      break;
    case 0x59:
      return "59xx: Set decay of operator 3 (0 to 1F)";
      break;
    case 0x5a:
      return "5Axx: Set decay of operator 4 (0 to 1F)";
      break;
    case 0x5b:
      return "5Bxx: Set decay 2 of all operators (0 to 1F)";
      break;
    case 0x5c:
      return "5Cxx: Set decay 2 of operator 1 (0 to 1F)";
      break;
    case 0x5d:
      return "5Dxx: Set decay 2 of operator 2 (0 to 1F)";
      break;
    case 0x5e:
      return "5Exx: Set decay 2 of operator 3 (0 to 1F)";
      break;
    case 0x5f:
      return "5Fxx: Set decay 2 of operator 4 (0 to 1F)";
      break;
  }
  return NULL;
}

void DivPlatformTX81Z::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  static int os[2];

  ymfm::ym2414::fm_engine* fme=fm_ymfm->debug_engine();

  for (size_t h=start; h<start+len; h++) {
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      if (--delay<1) {
        QueuedWrite& w=writes.front();
        fm_ymfm->write(0x0+((w.addr>>8)<<1),w.addr);
        fm_ymfm->write(0x1+((w.addr>>8)<<1),w.val);
        regPool[w.addr&0xff]=w.val;
        writes.pop();
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

static unsigned char noteMap[12]={
  0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14
};

inline int hScale(int note) {
  return ((note/12)<<4)+(noteMap[note%12]);
}

void DivPlatformTX81Z::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol*MIN(127,chan[i].std.vol.val))/127;
      for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
    }

    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].baseFreq=NOTE_LINEAR(chan[i].std.arp.val);
        } else {
          chan[i].baseFreq=NOTE_LINEAR(chan[i].note+(signed char)chan[i].std.arp.val);
        }
      }
      chan[i].freqChanged=true;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].baseFreq=NOTE_LINEAR(chan[i].note);
        chan[i].freqChanged=true;
      }
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

    if (chan[i].std.alg.had) {
      chan[i].state.alg=chan[i].std.alg.val;
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|(chan[i].active?0:0x40)|(chan[i].chVolR<<7));
      if (!parent->song.algMacroBehavior) for (int j=0; j<4; j++) {
        unsigned short baseAddr=chanOffs[i]|opOffs[j];
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
          } else {
            rWrite(baseAddr+ADDR_TL,op.tl);
          }
        }
      }
    }
    if (chan[i].std.fb.had) {
      chan[i].state.fb=chan[i].std.fb.val;
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|(chan[i].active?0:0x40)|(chan[i].chVolR<<7));
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
        op.tl=127-m.tl.val;
        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[i].state.alg][j]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
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
      //if (chan[i].keyOn) immWrite(0x08,i);
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|0x00|(chan[i].chVolR<<7));
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

  for (int i=0; i<8; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=chan[i].baseFreq+(chan[i].pitch>>1)-64+chan[i].pitch2;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>=(95<<6)) chan[i].freq=(95<<6)-1;
      immWrite(i+0x28,hScale(chan[i].freq>>6));
      immWrite(i+0x30,(chan[i].freq<<2)|(chan[i].chVolL==chan[i].chVolR));
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn) {
      //immWrite(0x08,i);
      immWrite(chanOffs[i]+ADDR_LR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3)|0x40|(chan[i].chVolR<<7));
      chan[i].keyOn=false;
    }
  }
}

void DivPlatformTX81Z::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  for (int i=0; i<4; i++) {
    unsigned short baseAddr=chanOffs[ch]|opOffs[i];
    DivInstrumentFM::Operator op=chan[ch].state.op[i];
    if (isMuted[ch]) {
      rWrite(baseAddr+ADDR_TL,127);
    } else {
      if (isOutput[chan[ch].state.alg][i]) {
        rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[ch].outVol&0x7f))/127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
    }
  }
}

int DivPlatformTX81Z::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_OPZ);

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator op=chan[c.chan].state.op[i];
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[c.chan].state.alg][i]) {
            if (!chan[c.chan].active || chan[c.chan].insChanged) {
              rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
            }
          } else {
            if (chan[c.chan].insChanged) {
              rWrite(baseAddr+ADDR_TL,op.tl);
            }
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|((op.egt?(op.dt&7):dtTable[op.dt&7])<<4));
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.egt<<5)|(op.rs<<6));
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+ADDR_DT2_D2R,(op.d2r&31)|(op.dt2<<6));
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+ADDR_WS_FINE,(op.dvb&15)|(op.ws<<4));
          rWrite(baseAddr+ADDR_EGS_REV,(op.dam&7)|(op.ksl<<6));
        }
      }
      if (chan[c.chan].insChanged) {
        /*
        if (isMuted[c.chan]) {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
        } else {
          rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
        }*/
        rWrite(chanOffs[c.chan]+ADDR_FMS_AMS,((chan[c.chan].state.fms&7)<<4)|(chan[c.chan].state.ams&3));
        //rWrite(chanOffs[c.chan]+ADDR_FMS_AMS,0x84|((chan[c.chan].state.fms2&7)<<4)|(chan[c.chan].state.ams2&3));
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
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (isOutput[chan[c.chan].state.alg][i]) {
            rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
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
      /*
      if (isMuted[c.chan]) {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      } else {
        rWrite(chanOffs[c.chan]+ADDR_LR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3)|((chan[c.chan].chVolL&1)<<6)|((chan[c.chan].chVolR&1)<<7));
      }*/
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
      rWrite(0x18,c.value);
      break;
    }
    case DIV_CMD_FM_LFO_WAVE: {
      rWrite(0x1b,c.value&3);
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
      if (isMuted[c.chan]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        if (isOutput[chan[c.chan].state.alg][c.value]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[c.chan].outVol&0x7f))/127));
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
    case DIV_ALWAYS_SET_VOLUME:
      return 0;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
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
      if (isMuted[i]) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        if (isOutput[chan[i].state.alg][j]) {
          rWrite(baseAddr+ADDR_TL,127-(((127-op.tl)*(chan[i].outVol&0x7f))/127));
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
    //rWrite(chanOffs[i]+ADDR_FMS_AMS,0x84|((chan[i].state.fms2&7)<<4)|(chan[i].state.ams2&3));
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
}

void DivPlatformTX81Z::notifyInsChange(int ins) {
  for (int i=0; i<8; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void* DivPlatformTX81Z::getChanState(int ch) {
  return &chan[ch];
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
  while (!writes.empty()) writes.pop();
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

  //rWrite(0x18,0x10);
  immWrite(0x19,amDepth);
  immWrite(0x19,0x80|pmDepth);
  //rWrite(0x1b,0x00);

  extMode=false;
}

void DivPlatformTX81Z::setFlags(unsigned int flags) {
  if (flags==2) {
    chipClock=4000000.0;
    baseFreqOff=-122;
  } else if (flags==1) {
    chipClock=COLOR_PAL*4.0/5.0;
    baseFreqOff=12;
  } else {
    chipClock=COLOR_NTSC;
    baseFreqOff=0;
  }
  rate=chipClock/64;
  for (int i=0; i<8; i++) {
    oscBuf[i]->rate=rate;
  }
}

bool DivPlatformTX81Z::isStereo() {
  return true;
}

int DivPlatformTX81Z::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
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
