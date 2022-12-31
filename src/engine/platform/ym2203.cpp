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

#include "ym2203.h"
#include "sound/ymfm/ymfm.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

const char* regCheatSheetYM2203[]={
  // SSG
  "SSG_FreqL_A",     "000",
  "SSG_FreqH_A",     "001",
  "SSG_FreqL_B",     "002",
  "SSG_FreqH_B",     "003",
  "SSG_FreqL_C",     "004",
  "SSG_FreqH_C",     "005",
  "SSG_FreqNoise",   "006",
  "SSG_Enable",      "007",
  "SSG_Volume_A",    "008",
  "SSG_Volume_B",    "009",
  "SSG_Volume_C",    "00A",
  "SSG_FreqL_Env",   "00B",
  "SSG_FreqH_Env",   "00C",
  "SSG_Control_Env", "00D",
  // FM (Common)
  "FM_Test",         "021",
  "ClockA1",         "024",
  "ClockA2",         "025",
  "ClockB",          "026",
  "FM_Control",      "027",
  "FM_NoteCtl",      "028",
  // FM (Channel 1-3)
  "FM1_Op1_DT_MULT", "030",
  "FM2_Op1_DT_MULT", "031",
  "FM3_Op1_DT_MULT", "032",
  "FM1_Op2_DT_MULT", "034",
  "FM2_Op2_DT_MULT", "035",
  "FM3_Op2_DT_MULT", "036",
  "FM1_Op3_DT_MULT", "038",
  "FM2_Op3_DT_MULT", "039",
  "FM3_Op3_DT_MULT", "03A",
  "FM1_Op4_DT_MULT", "03C",
  "FM2_Op4_DT_MULT", "03D",
  "FM3_Op4_DT_MULT", "03E",
  "FM1_Op1_TL",      "040",
  "FM2_Op1_TL",      "041",
  "FM3_Op1_TL",      "042",
  "FM1_Op2_TL",      "044",
  "FM2_Op2_TL",      "045",
  "FM3_Op2_TL",      "046",
  "FM1_Op3_TL",      "048",
  "FM2_Op3_TL",      "049",
  "FM3_Op3_TL",      "04A",
  "FM1_Op4_TL",      "04C",
  "FM2_Op4_TL",      "04D",
  "FM3_Op4_TL",      "04E",
  "FM1_Op1_KS_AR",   "050",
  "FM2_Op1_KS_AR",   "051",
  "FM3_Op1_KS_AR",   "052",
  "FM1_Op2_KS_AR",   "054",
  "FM2_Op2_KS_AR",   "055",
  "FM3_Op2_KS_AR",   "056",
  "FM1_Op3_KS_AR",   "058",
  "FM2_Op3_KS_AR",   "059",
  "FM3_Op3_KS_AR",   "05A",
  "FM1_Op4_KS_AR",   "05C",
  "FM2_Op4_KS_AR",   "05D",
  "FM3_Op4_KS_AR",   "05E",
  "FM1_Op1_AM_DR",   "060",
  "FM2_Op1_AM_DR",   "061",
  "FM3_Op1_AM_DR",   "062",
  "FM1_Op2_AM_DR",   "064",
  "FM2_Op2_AM_DR",   "065",
  "FM3_Op2_AM_DR",   "066",
  "FM1_Op3_AM_DR",   "068",
  "FM2_Op3_AM_DR",   "069",
  "FM3_Op3_AM_DR",   "06A",
  "FM1_Op4_AM_DR",   "06C",
  "FM2_Op4_AM_DR",   "06D",
  "FM3_Op4_AM_DR",   "06E",
  "FM1_Op1_SR",      "070",
  "FM2_Op1_SR",      "071",
  "FM3_Op1_SR",      "072",
  "FM1_Op2_SR",      "074",
  "FM2_Op2_SR",      "075",
  "FM3_Op2_SR",      "076",
  "FM1_Op3_SR",      "078",
  "FM2_Op3_SR",      "079",
  "FM3_Op3_SR",      "07A",
  "FM1_Op4_SR",      "07C",
  "FM2_Op4_SR",      "07D",
  "FM3_Op4_SR",      "07E",
  "FM1_Op1_SL_RR",   "080",
  "FM2_Op1_SL_RR",   "081",
  "FM3_Op1_SL_RR",   "082",
  "FM1_Op2_SL_RR",   "084",
  "FM2_Op2_SL_RR",   "085",
  "FM3_Op2_SL_RR",   "086",
  "FM1_Op3_SL_RR",   "088",
  "FM2_Op3_SL_RR",   "089",
  "FM3_Op3_SL_RR",   "08A",
  "FM1_Op4_SL_RR",   "08C",
  "FM2_Op4_SL_RR",   "08D",
  "FM3_Op4_SL_RR",   "08E",
  "FM1_Op1_SSG_EG",  "090",
  "FM2_Op1_SSG_EG",  "091",
  "FM3_Op1_SSG_EG",  "092",
  "FM1_Op2_SSG_EG",  "094",
  "FM2_Op2_SSG_EG",  "095",
  "FM3_Op2_SSG_EG",  "096",
  "FM1_Op3_SSG_EG",  "098",
  "FM2_Op3_SSG_EG",  "099",
  "FM3_Op3_SSG_EG",  "09A",
  "FM1_Op4_SSG_EG",  "09C",
  "FM2_Op4_SSG_EG",  "09D",
  "FM3_Op4_SSG_EG",  "09E",
  "FM1_FNum1",       "0A0",
  "FM2_FNum1",       "0A1",
  "FM3_(Op1)FNum1",  "0A2",
  "FM1_FNum2",       "0A4",
  "FM2_FNum2",       "0A5",
  "FM3_(Op1)FNum2",  "0A6",
  "FM3_Op2_FNum1",   "0A8",
  "FM3_Op3_FNum1",   "0A9",
  "FM3_Op4_FNum1",   "0AA",
  "FM3_Op2_FNum2",   "0AC",
  "FM3_Op3_FNum2",   "0AD",
  "FM3_Op4_FNum2",   "0AE",
  "FM1_FB_ALG",      "0B0",
  "FM2_FB_ALG",      "0B1",
  "FM3_FB_ALG",      "0B2",
  NULL
};

const char** DivPlatformYM2203::getRegisterSheet() {
  return regCheatSheetYM2203;
}

void DivPlatformYM2203::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  if (useCombo) {
    acquire_combo(bufL,bufR,start,len);
  } else {
    acquire_ymfm(bufL,bufR,start,len);
  }
}

void DivPlatformYM2203::acquire_combo(short* bufL, short* bufR, size_t start, size_t len) {
  static int os;
  static short ignored[2];

  for (size_t h=start; h<start+len; h++) {
    os=0;
    // Nuked part
    for (unsigned int i=0; i<nukedMult; i++) {
      if (!writes.empty()) {
        if (--delay<1 && !(fm->read(0)&0x80)) {
          QueuedWrite& w=writes.front();

          if (w.addr<=0x1c || w.addr==0x2d || w.addr==0x2e || w.addr==0x2f) {
            // ymfm write
            fm->write(0x0,w.addr);
            fm->write(0x1,w.val);

            regPool[w.addr&0xff]=w.val;
            writes.pop_front();
            delay=1;
          } else {
            // Nuked write
            if (w.addrOrVal) {
              OPN2_Write(&fm_nuked,0x1,w.val);
              regPool[w.addr&0xff]=w.val;
              writes.pop_front();
            } else {
              lastBusy++;
              if (fm_nuked.write_busy==0) {
                OPN2_Write(&fm_nuked,0x0,w.addr);
                w.addrOrVal=true;
              }
            }
          }
        }
      }

      OPN2_Clock(&fm_nuked,ignored);
    }
    os=(
      (fm_nuked.ch_out[0])+
      (fm_nuked.ch_out[1])+
      (fm_nuked.ch_out[2])
    );

    os&=~3;

    // ymfm part
    fm->generate(&fmout);

    os+=((fmout.data[1]+fmout.data[2]+fmout.data[3])>>1);
    if (os<-32768) os=-32768;
    if (os>32767) os=32767;
  
    bufL[h]=os;
    
    for (int i=0; i<3; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=fm_nuked.ch_out[i];
    }

    for (int i=3; i<6; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=fmout.data[i-2];
    }
  }
}

void DivPlatformYM2203::acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len) {
  static int os;

  ymfm::ym2203::fm_engine* fme=fm->debug_fm_engine();

  ymfm::fm_channel<ymfm::opn_registers_base<false>>* fmChan[3];
  for (int i=0; i<3; i++) {
    fmChan[i]=fme->debug_channel(i);
  }

  for (size_t h=start; h<start+len; h++) {
    os=0;
    if (!writes.empty()) {
      if (--delay<1) {
        QueuedWrite& w=writes.front();
        fm->write(0x0,w.addr);
        fm->write(0x1,w.val);
        regPool[w.addr&0xff]=w.val;
        writes.pop_front();
        delay=6;
      }
    }
    
    fm->generate(&fmout);

    os=fmout.data[0]+((fmout.data[1]+fmout.data[2]+fmout.data[3])>>1);
    if (os<-32768) os=-32768;
    if (os>32767) os=32767;
  
    bufL[h]=os;

    
    for (int i=0; i<3; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(fmChan[i]->debug_output(0)+fmChan[i]->debug_output(1));
    }

    for (int i=3; i<6; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=fmout.data[i-2];
    }
  }
}

void DivPlatformYM2203::tick(bool sysTick) {
  // PSG
  ay->tick(sysTick);
  ay->flushWrites();
  for (DivRegWrite& i: ay->getRegisterWrites()) {
    if (i.addr>15) continue;
    immWrite(i.addr&15,i.val);
  }
  ay->getRegisterWrites().clear();
  
  // FM
  for (int i=0; i<3; i++) {
    if (i==2 && extMode) continue;
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(127,chan[i].std.vol.val),127);
      for (int j=0; j<4; j++) {
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

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FNUM_BLOCK(parent->calcArp(chan[i].note,chan[i].std.arp.val),11);
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-1048576,1048575);
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

    if (chan[i].std.alg.had) {
      chan[i].state.alg=chan[i].std.alg.val;
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
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
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
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

    if (chan[i].keyOn || chan[i].keyOff) {
      if (chan[i].hardReset && chan[i].keyOn) {
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          immWrite(baseAddr+ADDR_SL_RR,0x0f);
          immWrite(baseAddr+ADDR_TL,0x7f);
          oldWrites[baseAddr+ADDR_SL_RR]=-1;
          oldWrites[baseAddr+ADDR_TL]=-1;
          //rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
        }
      }
      immWrite(0x28,0x00|konOffs[i]);
      if (chan[i].hardReset && chan[i].keyOn) {
        for (int j=0; j<4; j++) {
          unsigned short baseAddr=chanOffs[i]|opOffs[j];
          for (int k=0; k<100; k++) {
            immWrite(baseAddr+ADDR_SL_RR,0x0f);
          }
        }
      }
      chan[i].keyOff=false;
    }
  }

  for (int i=16; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<3; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].freqChanged) {
      if (parent->song.linearPitch==2) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,4,chan[i].pitch2,chipClock,CHIP_FREQBASE,11);
      } else {
        int fNum=parent->calcFreq(chan[i].baseFreq&0x7ff,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,4,chan[i].pitch2);
        int block=(chan[i].baseFreq&0xf800)>>11;
        if (fNum<0) fNum=0;
        if (fNum>2047) {
          while (block<7) {
            fNum>>=1;
            block++;
          }
          if (fNum>2047) fNum=2047;
        }
        chan[i].freq=(block<<11)|fNum;
      }
      if (chan[i].freq>0x3fff) chan[i].freq=0x3fff;
      immWrite(chanOffs[i]+ADDR_FREQH,chan[i].freq>>8);
      immWrite(chanOffs[i]+ADDR_FREQ,chan[i].freq&0xff);
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn || chan[i].opMaskChanged) {
      immWrite(0x28,(chan[i].opMask<<4)|konOffs[i]);
      chan[i].opMaskChanged=false;
      chan[i].keyOn=false;
    }
  }
}

int DivPlatformYM2203::dispatch(DivCommand c) {
  if (c.chan>2) {
    c.chan-=3;
    return ay->dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
      chan[c.chan].macroInit(ins);
      if (c.chan<3) {
        if (!chan[c.chan].std.vol.will) {
          chan[c.chan].outVol=chan[c.chan].vol;
        }
      }

      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
        chan[c.chan].opMask=
          (chan[c.chan].state.op[0].enable?1:0)|
          (chan[c.chan].state.op[2].enable?2:0)|
          (chan[c.chan].state.op[1].enable?4:0)|
          (chan[c.chan].state.op[3].enable?8:0);
      }
      
      for (int i=0; i<4; i++) {
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_TL,127);
        } else {
          if (KVS(c.chan,i)) {
            if (!chan[c.chan].active || chan[c.chan].insChanged) {
              rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[c.chan].outVol&0x7f,127));
            }
          } else {
            if (chan[c.chan].insChanged) {
              rWrite(baseAddr+ADDR_TL,op.tl);
            }
          }
        }
        if (chan[c.chan].insChanged) {
          rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
          rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
          rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      }
      if (chan[c.chan].insChanged) {
        rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      }
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
        chan[c.chan].portaPause=false;
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      chan[c.chan].macroInit(NULL);
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
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (c.chan>2 || parent->song.linearPitch==2) { // PSG
        int destFreq=NOTE_FNUM_BLOCK(c.value2,11);
        bool return2=false;
        if (destFreq>chan[c.chan].baseFreq) {
          chan[c.chan].baseFreq+=c.value;
          if (chan[c.chan].baseFreq>=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        } else {
          chan[c.chan].baseFreq-=c.value;
          if (chan[c.chan].baseFreq<=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        }
        chan[c.chan].freqChanged=true;
        if (return2) {
          chan[c.chan].inPorta=false;
          return 2;
        }
        break;
      }
      PLEASE_HELP_ME(chan[c.chan]);
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      if (extSys) {
        extMode=c.value;
        immWrite(0x27,extMode?0x40:0);
      }
      break;
    }
    case DIV_CMD_FM_FB: {
      if (c.chan>2) break;
      chan[c.chan].state.fb=c.value&7;
      rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      if (c.chan>2) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>2) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.tl=c.value2;
      if (isMuted[c.chan]) {
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
      if (c.chan>2) break;
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
    case DIV_CMD_FM_D2R: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.d2r=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.d2r=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
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
    case DIV_CMD_FM_SSG: {
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ssgEnv=8^(c.value2&15);
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ssgEnv=8^(c.value2&15);
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
      }
      break;
    }
    case DIV_CMD_FM_HARD_RESET:
      chan[c.chan].hardReset=c.value;
      break;
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
      if (c.chan>2) return 15;
      return 127;
      break;
    case DIV_CMD_PRE_PORTA:
      if (c.chan>2) {
        if (chan[c.chan].active && c.value2) {
          if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_FM));
        }
      }
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

void DivPlatformYM2203::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (ch>2) { // PSG
    ay->muteChannel(ch-3,mute);
    return;
  }
  for (int j=0; j<4; j++) {
    unsigned short baseAddr=chanOffs[ch]|opOffs[j];
    DivInstrumentFM::Operator& op=chan[ch].state.op[j];
    if (isMuted[ch]) {
      rWrite(baseAddr+ADDR_TL,127);
    } else {
      if (KVS(ch,j)) {
        rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[ch].outVol&0x7f,127));
      } else {
        rWrite(baseAddr+ADDR_TL,op.tl);
      }
    }
  }
}

void DivPlatformYM2203::forceIns() {
  for (int i=0; i<3; i++) {
    for (int j=0; j<4; j++) {
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
  for (int i=3; i<6; i++) {
    chan[i].insChanged=true;
  }

  ay->forceIns();
  ay->flushWrites();
  for (DivRegWrite& i: ay->getRegisterWrites()) {
    if (i.addr>15) continue;
    immWrite(i.addr&15,i.val);
  }
  ay->getRegisterWrites().clear();
}

void* DivPlatformYM2203::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformYM2203::getChanMacroInt(int ch) {
  if (ch>=3) return ay->getChanMacroInt(ch-3);
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformYM2203::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformYM2203::getRegisterPool() {
  return regPool;
}

int DivPlatformYM2203::getRegisterPoolSize() {
  return 256;
}

void DivPlatformYM2203::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformYM2203::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformYM2203::reset() {
  while (!writes.empty()) writes.pop_front();
  memset(regPool,0,256);
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  OPN2_Reset(&fm_nuked);
  OPN2_SetChipType(&fm_nuked,ym3438_mode_opn);
  fm->reset();
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformOPN::OPNChannel();
    chan[i].std.setEngine(parent);
  }
  for (int i=0; i<3; i++) {
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }
  for (int i=3; i<6; i++) {
    chan[i].vol=0x0f;
  }

  for (int i=0; i<256; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  sampleBank=0;

  delay=0;

  extMode=false;

  // set prescaler
  immWrite(0x2d,0xff);
  immWrite(prescale,0xff);

  ay->reset();
  ay->getRegisterWrites().clear();
  ay->flushWrites();
}

bool DivPlatformYM2203::isStereo() {
  return false;
}

bool DivPlatformYM2203::keyOffAffectsArp(int ch) {
  return (ch>2);
}

void DivPlatformYM2203::notifyInsChange(int ins) {
  for (int i=0; i<6; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
  ay->notifyInsChange(ins);
}

void DivPlatformYM2203::notifyInsDeletion(void* ins) {
  ay->notifyInsDeletion(ins);
}

void DivPlatformYM2203::setSkipRegisterWrites(bool value) {
  DivDispatch::setSkipRegisterWrites(value);
  ay->setSkipRegisterWrites(value);
}

void DivPlatformYM2203::setFlags(const DivConfig& flags) {
  // Clock flags
  switch (flags.getInt("clockSel",0)) {
    case 0x01:
      chipClock=COLOR_PAL*4.0/5.0;
      break;
    case 0x02:
      chipClock=4000000.0;
      break;
    case 0x03:
      chipClock=3000000.0;
      break;
    case 0x04:
      chipClock=38400*13*8; // 31948800/8
      break;
    case 0x05:
      chipClock=3000000.0/2.0;
      break;
    default:
      chipClock=COLOR_NTSC;
      break;
  }
  // Prescaler flags
  switch (flags.getInt("prescale",0)) {
    case 0x01: // /3
      prescale=0x2e;
      fmFreqBase=4720270.0/2.0,
      fmDivBase=18,
      ayDiv=8;
      nukedMult=16;
      break;
    case 0x02: // /2
      prescale=0x2f;
      fmFreqBase=4720270.0/3.0,
      fmDivBase=12,
      ayDiv=4;
      nukedMult=24;
      break;
    default: // /6
      prescale=0x2d;
      fmFreqBase=4720270.0,
      fmDivBase=36,
      ayDiv=16;
      nukedMult=8;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  noExtMacros=flags.getBool("noExtMacros",false);
  fbAllOps=flags.getBool("fbAllOps",false);
  rate=fm->sample_rate(chipClock);
  for (int i=0; i<6; i++) {
    oscBuf[i]->rate=rate;
  }
  immWrite(0x2d,0xff);
  immWrite(prescale,0xff);
  ay->setExtClockDiv(chipClock,ayDiv);
  ay->setFlags(ayFlags);
}

int DivPlatformYM2203::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  ayFlags.set("chipType",1);
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  fm=new ymfm::ym2203(iface);
  fm->set_fidelity(ymfm::OPN_FIDELITY_MIN);
  // YM2149, 2MHz
  ay=new DivPlatformAY8910(true,chipClock,ayDiv);
  ay->init(p,3,sugRate,ayFlags);
  ay->toggleRegisterDump(true);
  setFlags(flags);

  reset();
  return 6;
}

void DivPlatformYM2203::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  ay->quit();
  delete ay;
  delete fm;
}

DivPlatformYM2203::~DivPlatformYM2203() {
}
