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

#include "esfm.h"
#include "../engine.h"
#include "../bsr.h"
#include "../../ta-log.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define CHIP_FREQBASE (32768*288)

#define ADDR_AM_VIB_SUS_KSR_MULT 0x00
#define ADDR_KSL_TL 0x01
#define ADDR_AR_DR 0x02
#define ADDR_SL_RR 0x03
#define ADDR_FREQL 0x04
#define ADDR_FREQH_BLOCK_DELAY 0x05
#define ADDR_DAM_DVB_LEFT_RIGHT_MODIN 0x06
#define ADDR_OUTLVL_NOISE_WS 0x07

#define KEY_ON_REGS_START (18*8*4)

void DivPlatformESFM::acquire(short** buf, size_t len) {
  thread_local short o[2];
  for (int i=0; i<18; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t h=0; h<len; h++) {
    if (!writes.empty()) {
      QueuedWrite& w=writes.front();
      ESFM_write_reg_buffered_fast(&chip,w.addr,w.val);
      if (w.addr<ESFM_REG_POOL_SIZE) {
        regPool[w.addr]=w.val;
      }
      writes.pop();
    }

    ESFM_generate(&chip,o);
    for (int c=0; c<18; c++) {
      oscBuf[c]->putSample(h,ESFM_get_channel_output_native(&chip,c));
    }

    buf[0][h]=o[0];
    buf[1][h]=o[1];
  }
  for (int i=0; i<18; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformESFM::acquireDirect(blip_buffer_t** bb, size_t off, size_t len) {
  thread_local short o[2];
  unsigned int sharedNeedlePos=oscBuf[0]->needle;
  for (int i=0; i<18; i++) {
    oscBuf[i]->begin(len);
  }
  size_t pos=off;
  for (size_t h=0; h<len; h++) {
    if (!writes.empty()) {
      QueuedWrite& w=writes.front();
      ESFM_write_reg_buffered_fast(&chip,w.addr,w.val);
      if (w.addr<ESFM_REG_POOL_SIZE) {
        regPool[w.addr]=w.val;
      }
      writes.pop();
    }

    ESFM_generate(&chip,o);
    const unsigned int shiftedNeedlePos=sharedNeedlePos>>OSCBUF_PREC;
    for (int c=0; c<18; c++) {
      putSampleIKnowWhatIAmDoing(oscBuf[c],shiftedNeedlePos,ESFM_get_channel_output_native(&chip,c));
    }
    sharedNeedlePos+=oscBuf[0]->rateMul;

    if (o[0]!=oldOut[0]) {
      blip_add_delta(bb[0],pos,oldOut[0]-o[0]);
      oldOut[0]=o[0];
    }
    if (o[1]!=oldOut[1]) {
      blip_add_delta(bb[1],pos,oldOut[1]-o[1]);
      oldOut[1]=o[1];
    }

    pos++;
  }
  for (int i=0; i<18; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformESFM::tick(bool sysTick) {
  for (int i=0; i<18; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(63,chan[i].std.vol.val),63);
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=i*32+o*8;
        DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[o];
        unsigned char noise=chan[i].state.esfm.noise&3;

        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
        } else {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
          if (KVS_ES(i,o)) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
          }
        }
      }
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.panL.had) {
      chan[i].globalPan=((chan[i].std.panL.val&1)<<1)|((chan[i].std.panL.val&2)>>1);
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=i*32+o*8;
        DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[o];
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[i].globalPan)&1)<<4)|(((opE.right&(chan[i].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }
    }

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-131071,131071);
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

    if (chan[i].std.duty.had) {
      unsigned short baseAddr=i*32+3*8;
      DivInstrumentESFM& ins=chan[i].state.esfm;
      DivInstrumentFM::Operator& op=chan[i].state.fm.op[3];
      DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[3];
      ins.noise=chan[i].std.duty.val;

      if (isMuted[i]) {
        rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((ins.noise&3)<<3)|0);
      } else {
        rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((ins.noise&3)<<3)|((opE.outLvl&7)<<5));
      }
    }

    for (int o=0; o<4; o++) {
      unsigned short baseAddr=i*32+o*8;
      DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
      DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[o];
      DivMacroInt::IntOp& m=chan[i].std.op[o];

      if (m.am.had) {
        op.am=m.am.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.vib.had) {
        op.vib=m.vib.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.sus.had) {
        op.sus=m.sus.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.ksr.had) {
        op.ksr=m.ksr.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.mult.had) {
        op.mult=m.mult.val;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }

      if (m.ar.had) {
        op.ar=m.ar.val;
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      if (m.dr.had) {
        op.dr=m.dr.val;
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      if (m.sl.had) {
        op.sl=m.sl.val;
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }
      if (m.rr.had) {
        op.rr=m.rr.val;
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }

      if (m.tl.had || m.ksl.had) {
        if (m.tl.had) {
          op.tl=m.tl.val&63;
        }
        if (m.ksl.had) {
          op.ksl=m.ksl.val;
        }

        if (KVS_ES(i,o)) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
      }

      if (m.dam.had) {
        op.dam=m.dam.val;
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[i].globalPan)&1)<<4)|(((opE.right&(chan[i].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }
      if (m.dvb.had) {
        op.dvb=m.dvb.val;
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[i].globalPan)&1)<<4)|(((opE.right&(chan[i].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }
      if (m.rs.had) {
        // operator panning
        opE.left=(m.rs.val&2)!=0;
        opE.right=(m.rs.val&1)!=0;
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[i].globalPan)&1)<<4)|(((opE.right&(chan[i].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }
      if (m.d2r.had) {
        // modIn
        opE.modIn=m.d2r.val;
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[i].globalPan)&1)<<4)|(((opE.right&(chan[i].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }

      if (m.egt.had | m.ws.had) {
        unsigned char noise=chan[i].state.esfm.noise&3;
        if (m.egt.had) {
          // outLvl
          opE.outLvl=m.egt.val;
        }
        if (m.ws.had) {
          op.ws=m.ws.val;
        }

        if (isMuted[i]) {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
        } else {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
        }
      }



      // detune/fixed pitch
      if (opE.fixed) {
        if (m.ssg.had) {
          opE.ct=(opE.ct&(~(7<<2)))|((m.ssg.val&7)<<2);
          chan[i].freqChanged=true;
        }
        if (m.dt.had) {
          opE.dt=m.dt.val&0xff;
          opE.ct=(opE.ct&(~3))|((m.dt.val>>8)&3);
          chan[i].freqChanged=true;
        }
      } else {
        chan[i].handleArpFmOp(0, o);
        chan[i].handlePitchFmOp(o);
      }

      if (m.dt2.had) {
        opE.delay=m.dt2.val;
        rWrite(baseAddr+ADDR_FREQH_BLOCK_DELAY,chan[i].freqH[o]|(opE.delay<<5));
      }
    }
  }



  for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  int hardResetElapsed=0;
  bool mustHardReset=false;

  for (int i=0; i<18; i++) {
    if (chan[i].keyOn || chan[i].keyOff) {
      // logI("chan[%d] key off", i);
      if (i<16) {
        immWrite(KEY_ON_REGS_START+i, 0);
      } else {
        // Handle writing to the split key-on registers of channels 16 and 17
        immWrite(KEY_ON_REGS_START+16+(i-16)*2, 0);
        immWrite(KEY_ON_REGS_START+16+1+(i-16)*2, 0);
      }
      chan[i].keyOff=false;
    }
    if (chan[i].hardReset && chan[i].keyOn) {
      mustHardReset=true;
      // logI("chan[%d] hard reset, slrr := 0x0f", i);
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=i*32+o*8;
        immWrite(baseAddr+ADDR_SL_RR,0x0f);
        hardResetElapsed++;
      }
    }
  }

  for (int i=0; i<18; i++) {
    if (chan[i].freqChanged) {
      int mul=2;
      int fixedBlock=chan[i].state.fm.block;
      if (parent->song.linearPitch!=2) {
        mul=octave(chan[i].baseFreq,fixedBlock)*2;
      }
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,mul,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>131071) chan[i].freq=131071;

      for (int o=0; o<4; o++) {
        unsigned short baseAddr=i*32+o*8;
        DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[o];
        int ct=(int)opE.ct;
        int dt=(int)opE.dt;
        if (opE.fixed) {
          chan[i].freqL[o]=opE.dt;
          chan[i].freqH[o]=opE.ct&0x1f;
        } else {
          int arp=chan[i].fixedArp?chan[i].baseNoteOverride+ct:chan[i].arpOff+ct;
          int pitch2=chan[i].pitch2+dt;
          int fixedArp=chan[i].fixedArp;
          if(chan[i].opsState[o].hasOpArp) {
            arp=chan[i].opsState[o].fixedArp?chan[i].opsState[o].baseNoteOverride+ct:chan[i].opsState[o].arpOff+ct;
            fixedArp=chan[i].opsState[o].fixedArp;
          }
          if(chan[i].opsState[o].hasOpPitch) {
            pitch2=chan[i].opsState[o].pitch2+dt;
          }
          int opFreq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,arp,fixedArp,false,mul,pitch2,chipClock,CHIP_FREQBASE);
          if (opFreq<0) opFreq=0;
          if (opFreq>131071) opFreq=131071;
          int freqt=toFreq(opFreq,fixedBlock);
          chan[i].freqL[o]=freqt&0xff;
          chan[i].freqH[o]=freqt>>8;
        }
        immWrite(baseAddr+ADDR_FREQL,chan[i].freqL[o]);
        immWrite(baseAddr+ADDR_FREQH_BLOCK_DELAY,chan[i].freqH[o]|(opE.delay<<5));
        hardResetElapsed+=2;
      }
      chan[i].freqChanged=false;
    }
    if (chan[i].keyOn && !chan[i].hardReset) {
      // logI("chan[%d] soft key on", i);
      if (i<16) {
        immWrite(KEY_ON_REGS_START+i, 1);
      } else {
        // Handle writing to the split key-on registers of channels 16 and 17
        immWrite(KEY_ON_REGS_START+16+(i-16)*2, 1);
        immWrite(KEY_ON_REGS_START+16+1+(i-16)*2, 1);
      }
      chan[i].keyOn=false;
    }
  }

  if (mustHardReset) {
    for (unsigned int i=hardResetElapsed; i<128; i++) {
      immWrite(0x25f, i&0xff);
    }
    for (int i=0; i<18; i++) {
      if (chan[i].hardReset && chan[i].keyOn) {
        // logI("chan[%d] hard reset key on, writing original slrr back", i);
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=i*32+o*8;
          DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
          immWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
        }
        if (i<16) {
          immWrite(KEY_ON_REGS_START+i, 1);
        } else {
          // Handle writing to the split key-on registers of channels 16 and 17
          immWrite(KEY_ON_REGS_START+16+(i-16)*2, 1);
          immWrite(KEY_ON_REGS_START+16+1+(i-16)*2, 1);
        }
        chan[i].keyOn=false;
      }
    }
  }
}

int DivPlatformESFM::octave(int freq, int fixedBlock) {
  if (fixedBlock>0) {
    return 1<<(fixedBlock-1);
  }
  if (freq>0x3ff) {
    return 1<<(bsr32(freq)-10);
  }
  return 1;
}

int DivPlatformESFM::toFreq(int freq, int fixedBlock) {
  int block=0;
  if (fixedBlock>0) {
    block=fixedBlock-1;
    freq>>=block;
    if (freq>0x3ff) freq=0x3ff;
  } else if (freq>0x3ff) {
    block=bsr32(freq)-10;
    freq>>=block;
  }
  return (block<<10)|(freq&0x3ff);
}

void DivPlatformESFM::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;

  for (int o=0; o<4; o++) {
    unsigned short baseAddr=ch*32+o*8;
    DivInstrumentFM::Operator& op=chan[ch].state.fm.op[o];
    DivInstrumentESFM::Operator& opE=chan[ch].state.esfm.op[o];
    unsigned char noise=chan[ch].state.esfm.noise&3;

    if (isMuted[ch]) {
      rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
    } else {
      rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
      if (KVS_ES(ch,o)) {
        rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
      } else {
        rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
      }
    }
  }
}

void DivPlatformESFM::commitState(int ch, DivInstrument* ins) {
  if (chan[ch].insChanged) {
    chan[ch].state.fm=ins->fm;
    chan[ch].state.esfm=ins->esfm;
    for (int o=0; o<4; o++) {
      unsigned short baseAddr=ch*32+o*8;
      DivInstrumentFM::Operator& op=chan[ch].state.fm.op[o];
      DivInstrumentESFM::Operator& opE=chan[ch].state.esfm.op[o];
      unsigned char noise=chan[ch].state.esfm.noise&3;

      if (isMuted[ch]) {
        rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
      } else {
        rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
        if (KVS_ES(ch,o)) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
      }

      rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
      rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
      rWrite(baseAddr+ADDR_FREQH_BLOCK_DELAY,chan[ch].freqH[o]|(opE.delay<<5));
      rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[ch].globalPan)&1)<<4)|(((opE.right&(chan[ch].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
    }
  }
}

int DivPlatformESFM::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_ESFM);

      chan[c.chan].macroInit(ins);
      memset(chan[c.chan].opsState, 0, sizeof(chan[c.chan].opsState));
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      commitState(c.chan,ins);
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
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
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
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
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        if (KVS_ES(c.chan,o)) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
      }
      break;
    }
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].globalPan=(c.value>0)|((c.value2>0)<<1);
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }
      break;
    }
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
      int newFreq;
      bool return2=false;
      int mul=1;
      int fixedBlock=0;
      if (parent->song.linearPitch!=2) {
        fixedBlock=chan[c.chan].state.fm.block;
        mul=octave(chan[c.chan].baseFreq,fixedBlock);
      }
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*mul;
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*mul;
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      if (!chan[c.chan].portaPause && parent->song.linearPitch!=2) {
        if (mul!=octave(newFreq,fixedBlock)) {
          chan[c.chan].portaPause=true;
          break;
        }
      }
      chan[c.chan].baseFreq=newFreq;
      chan[c.chan].portaPause=false;
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      if (chan[c.chan].insChanged) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_ESFM);
        commitState(c.chan,ins);
        chan[c.chan].insChanged=false;
      }
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_MULT: {
      unsigned int o=c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32+o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      break;
    }
    case DIV_CMD_FM_TL: {
      unsigned int o=c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32+o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      op.tl=c.value2&63;
      if (KVS_ES(c.chan,o)) {
        rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
      } else {
        rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.ar=c.value2&15;
          rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.ar=c.value2&15;
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.dr=c.value2&15;
          rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.dr=c.value2&15;
        rWrite(baseAddr+ADDR_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.sl=c.value2&15;
          rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.sl=c.value2&15;
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.rr=c.value2&15;
          rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.rr=c.value2&15;
        rWrite(baseAddr+ADDR_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.am=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.am=c.value2&1;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_VIB: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.vib=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.vib=c.value2&1;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_SUS: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.sus=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.sus=c.value2&1;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_KSR: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.ksr=c.value2&1;
          rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.ksr=c.value2&1;
        rWrite(baseAddr+ADDR_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_WS: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
          unsigned char noise=chan[c.chan].state.esfm.noise&3;
          op.ws=c.value2&7;
          if (isMuted[c.chan]) {
            rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
          } else {
            rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
          }
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        unsigned char noise=chan[c.chan].state.esfm.noise&3;
        op.ws=c.value2&7;
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
        } else {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
        }
      }
      break;
    }
    // KSL
    case DIV_CMD_FM_RS: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.ksl=c.value2&3;
          if (KVS_ES(c.chan,o)) {
            rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
          }
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.ksl=c.value2&3;
        if (KVS_ES(c.chan,o)) {
          rWrite(baseAddr+ADDR_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+ADDR_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
      }
      break;
    }
    case DIV_CMD_FM_AM_DEPTH: {
      unsigned int o=c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32+o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
      op.dam=c.value2&1;
      rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      break;
    }
    case DIV_CMD_FM_PM_DEPTH: {
      unsigned int o=c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32+o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
      op.dvb=c.value2&1;
      rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      break;
    }
    case DIV_CMD_FM_FIXFREQ: {
      unsigned int o=c.value&3;
      bool isFNum=c.value&4;
      DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
      if (!opE.fixed) break;
      if (isFNum) {
        opE.dt=(c.value2)&0xff;
        opE.ct=(opE.ct&(~3))|((c.value2>>8)&3);
        chan[c.chan].freqChanged=true;
      } else {
        opE.ct=(opE.ct&(~(7<<2)))|((c.value2&7)<<2);
        chan[c.chan].freqChanged=true;
      }
      break;
    }
    case DIV_CMD_ESFM_OP_PANNING: {
      unsigned int o=c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32+o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
      opE.left=(c.value2&0xf0)!=0;
      opE.right=(c.value2&0x0f)!=0;
      rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      break;
    }
    case DIV_CMD_ESFM_OUTLVL: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
          unsigned char noise=chan[c.chan].state.esfm.noise&3;
          opE.outLvl=c.value2&7;
          if (isMuted[c.chan]) {
            rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
          } else {
            rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
          }
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        unsigned char noise=chan[c.chan].state.esfm.noise&3;
        opE.outLvl=c.value2&7;
        if (isMuted[c.chan]) {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
        } else {
          rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
        }
      }
      break;
    }
    case DIV_CMD_ESFM_MODIN: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
          opE.modIn=c.value2&7;
          rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        opE.modIn=c.value2&7;
        rWrite(baseAddr+ADDR_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
      }
      break;
    }
    case DIV_CMD_ESFM_ENV_DELAY: {
      if (c.value<0) {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32+o*8;
          DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
          opE.delay=c.value2&7;
          rWrite(baseAddr+ADDR_FREQH_BLOCK_DELAY,chan[c.chan].freqH[o]|(opE.delay<<5));
        }
      } else {
        unsigned int o=c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32+o*8;
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        opE.delay=c.value2&7;
        rWrite(baseAddr+ADDR_FREQH_BLOCK_DELAY,chan[c.chan].freqH[o]|(opE.delay<<5));
      }
      break;
    }
    case DIV_CMD_STD_NOISE_MODE: {
      unsigned short baseAddr=c.chan*32+3*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[3];
      DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[3];
      DivInstrumentESFM insE=chan[c.chan].state.esfm;
      insE.noise=c.value&3;
      if (isMuted[c.chan]) {
        rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|(insE.noise<<3)|0);
      } else {
        rWrite(baseAddr+ADDR_OUTLVL_NOISE_WS,(op.ws&7)|(insE.noise<<3)|((opE.outLvl&7)<<5));
      }
      break;
    }
    case DIV_CMD_FM_DT: {
      unsigned int o=c.value;
      if (o >= 4) break;
      DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
      if (opE.fixed) break;
      opE.dt=c.value2-0x80;
      chan[c.chan].freqChanged=true;
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
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    case DIV_CMD_GET_VOLMAX:
      return 63;
      break;
    case DIV_CMD_PRE_PORTA:
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      }
      chan[c.chan].inPorta=c.value;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformESFM::forceIns() {
  for (int i=0; i<18; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
  for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
    oldWrites[i]=-1;
  }
}

void DivPlatformESFM::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void* DivPlatformESFM::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformESFM::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformESFM::getPan(int ch) {
  return ((chan[ch].globalPan&1)<<8)|((chan[ch].globalPan&2)>>1);
}

DivDispatchOscBuffer* DivPlatformESFM::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformESFM::getRegisterPool() {
  // // Uncomment this for debugging weird behavior
  // for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
  //   regPool[i]=ESFM_readback_reg(&chip, i);
  // }
  return regPool;
}

int DivPlatformESFM::getRegisterPoolSize() {
  return ESFM_REG_POOL_SIZE;
}

void DivPlatformESFM::reset() {
  while (!writes.empty()) writes.pop();

  ESFM_init(&chip,isFast?1:0);
  // set chip to native mode
  ESFM_write_reg(&chip, 0x105, 0x80);
  // ensure NTS bit in register 0x408 is reset, for smooth envelope rate scaling
  ESFM_write_reg(&chip, 0x408, 0x00);

  for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
    regPool[i]=ESFM_readback_reg(&chip, i);
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  for (int i=0; i<18; i++) {
    chan[i]=DivPlatformESFM::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x3f;
    chan[i].outVol=0x3f;
  }

  oldOut[0]=0;
  oldOut[1]=0;
}

int DivPlatformESFM::getOutputCount() {
  return 2;
}

bool DivPlatformESFM::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformESFM::keyOffAffectsPorta(int ch) {
  return false;
}

bool DivPlatformESFM::hasAcquireDirect() {
  return true;
}

bool DivPlatformESFM::getLegacyAlwaysSetVolume() {
  return false;
}

void DivPlatformESFM::notifyInsChange(int ins) {
  for (int i=0; i<18; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformESFM::notifyInsDeletion(void* ins) {
  for (int i=0; i<18; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

int DivPlatformESFM::mapVelocity(int ch, float vel) {
  const int volMax=MAX(1,dispatch(DivCommand(DIV_CMD_GET_VOLMAX,MAX(ch,0))));
  double attenDb=20*log10(vel); // 20dB/decade for a linear mapping
  double attenUnits=attenDb/0.75; // 0.75dB/unit
  return MAX(0,volMax+attenUnits);
}

void DivPlatformESFM::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformESFM::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformESFM::setFlags(const DivConfig& flags) {
  chipClock=COLOR_NTSC*4.0;
  rate=chipClock/288.0;
  for (int i=0; i<18; i++) {
    oscBuf[i]->setRate(rate);
  }
}

void DivPlatformESFM::setFast(bool fast) {
  isFast=fast;
}

int DivPlatformESFM::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<18; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);
  reset();

  return 18;
}

void DivPlatformESFM::quit() {
  for (int i=0; i<18; i++) {
    delete oscBuf[i];
  }
}

DivPlatformESFM::~DivPlatformESFM() {
}
