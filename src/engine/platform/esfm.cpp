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

#include "esfm.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define CHIP_FREQBASE (32768*288)

#define OFFSET_AM_VIB_SUS_KSR_MULT 0x00
#define OFFSET_KSL_TL 0x01
#define OFFSET_AR_DR 0x02
#define OFFSET_SL_RR 0x03
#define OFFSET_FREQL 0x04
#define OFFSET_FREQH_BLOCK_DELAY 0x05
#define OFFSET_DAM_DVB_LEFT_RIGHT_MODIN 0x06
#define OFFSET_OUTLVL_NOISE_WS 0x07

#define KEY_ON_REGS_START (18 * 8 * 4)

void DivPlatformESFM::acquire(short** buf, size_t len) {
  thread_local short o[2];
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
      oscBuf[c]->data[oscBuf[c]->needle++]=ESFM_get_channel_output_native(&chip,c);
    }

    buf[0][h]=o[0];
    buf[1][h]=o[1];
  }
}

void DivPlatformESFM::tick(bool sysTick) {
  for (int i=0; i<18; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(63,chan[i].std.vol.val),63);
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=i*32 + o*8;
        DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[o];
        unsigned char noise=chan[i].state.esfm.noise&3;

        if (isMuted[i]) {
          rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
        } else {
          rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
          if (KVS(i, o)) {
            rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
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

    // TODO: check why I disabled globalPan here?
#if 0
    if (chan[i].std.panL.had) {
      chan[i].globalPan=((chan[i].std.panL.val&1)<<1)|((chan[i].std.panL.val&2)>>1);
    }
#endif

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
    
    for (int o=0; o<4; o++) {
      unsigned short baseAddr=i*32 + o*8;
      DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
      DivMacroInt::IntOp& m=chan[i].std.op[o];
      
      if (m.am.had) {
        op.am=m.am.val;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.vib.had) {
        op.vib=m.vib.val;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.sus.had) {
        op.sus=m.sus.val;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.ksr.had) {
        op.ksr=m.ksr.val;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      if (m.mult.had) {
        op.mult=m.mult.val;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }

      if (m.ar.had) {
        op.ar=m.ar.val;
        rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      if (m.dr.had) {
        op.dr=m.dr.val;
        rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      if (m.sl.had) {
        op.sl=m.sl.val;
        rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }
      if (m.rr.had) {
        op.rr=m.rr.val;
        rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }

      if (m.tl.had || m.ksl.had) {
        if (m.tl.had) {
          op.tl=m.tl.val&63;
        }
        if (m.ksl.had) {
          op.ksl=m.ksl.val;
        }

        if (KVS(i, o)) {
          rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[i].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
      }
    }
  }

  for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

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
        unsigned short baseAddr=i*32 + o*8;
        immWrite(baseAddr+OFFSET_SL_RR,0x0f);
      }
    }
  }

  for (int i=0; i<18; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,octave(chan[i].baseFreq)*2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>131071) chan[i].freq=131071;

      for (int o=0; o<4; o++) {
        unsigned short baseAddr=i*32 + o*8;
        DivInstrumentESFM::Operator& opE=chan[i].state.esfm.op[o];
        int ct=(int)opE.ct;
        int dt=(int)opE.dt;
        int opFreq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride+ct:chan[i].arpOff+ct,chan[i].fixedArp,false,octave(chan[i].baseFreq)*2,chan[i].pitch2+dt,chipClock,CHIP_FREQBASE);
        if (opFreq<0) opFreq=0;
        if (opFreq>131071) opFreq=131071;
        int freqt=toFreq(opFreq);
        chan[i].freqL[o]=freqt&0xff;
        chan[i].freqH[o]=freqt>>8;
        immWrite(baseAddr+OFFSET_FREQL,chan[i].freqL[o]);
        immWrite(baseAddr+OFFSET_FREQH_BLOCK_DELAY,chan[i].freqH[o]|(opE.delay<<5));
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
    for (int i=0; i<18; i++) {
      if (chan[i].hardReset && chan[i].keyOn) {
        // logI("chan[%d] hard reset key on, writing original slrr back", i);
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=i*32 + o*8;
          DivInstrumentFM::Operator& op=chan[i].state.fm.op[o];
          immWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|(op.rr&0xf));
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

int DivPlatformESFM::octave(int freq) {
  if (freq>=0x3ff<<6) {
    return 1<<7;
  } else if (freq>=0x3ff<<5) {
    return 1<<6;
  } else if (freq>=0x3ff<<4) {
    return 1<<5;
  } else if (freq>=0x3ff<<3) {
    return 1<<4;
  } else if (freq>=0x3ff<<2) {
    return 1<<3;
  } else if (freq>=0x3ff<<1) {
    return 1<<2;
  } else if (freq>=0x3ff) {
    return 1<<1;
  } else {
    return 1<<0;
  }
  return 1<<0;
}

int DivPlatformESFM::toFreq(int freq) {
  if (freq>=0x3ff<<6) {
    return 0x1c00|((freq>>7)&0x3ff);
  } else if (freq>=0x3ff<<5) {
    return 0x1800|((freq>>6)&0x3ff);
  } else if (freq>=0x3ff<<4) {
    return 0x1400|((freq>>5)&0x3ff);
  } else if (freq>=0x3ff<<3) {
    return 0x1000|((freq>>4)&0x3ff);
  } else if (freq>=0x3ff<<2) {
    return 0xc00|((freq>>3)&0x3ff);
  } else if (freq>=0x3ff<<1) {
    return 0x800|((freq>>2)&0x3ff);
  } else if (freq>=0x3ff<<0) {
    return 0x400|((freq>>1)&0x3ff);
  } else {
    return freq&0x3ff;
  }
}

void DivPlatformESFM::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  
  for (int o=0; o<4; o++) {
    unsigned short baseAddr=ch*32 + o*8;
    DivInstrumentFM::Operator& op=chan[ch].state.fm.op[o];
    DivInstrumentESFM::Operator& opE=chan[ch].state.esfm.op[o];
    unsigned char noise=chan[ch].state.esfm.noise&3;
    
    if (isMuted[ch]) {
      rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
    } else {
      rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
      if (KVS(ch, o)) {
        rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
      } else {
        rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
      }
    }
  }
}

void DivPlatformESFM::commitState(int ch, DivInstrument* ins) {
  if (chan[ch].insChanged) {
    chan[ch].state.fm=ins->fm;
    chan[ch].state.esfm=ins->esfm;
    for (int o=0; o<4; o++) {
      unsigned short baseAddr=ch*32 + o*8;
      DivInstrumentFM::Operator& op=chan[ch].state.fm.op[o];
      DivInstrumentESFM::Operator& opE=chan[ch].state.esfm.op[o];
      unsigned char noise=chan[ch].state.esfm.noise&3;

      if (isMuted[ch]) {
        rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
      } else {
        rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
        if (KVS(ch, o)) {
          rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[ch].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
      }

      rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
      rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|(op.rr&0xf));
      rWrite(baseAddr+OFFSET_FREQH_BLOCK_DELAY,chan[ch].freqH[o]|(opE.delay<<5));
      rWrite(baseAddr+OFFSET_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[ch].globalPan)&1)<<4)|(((opE.right&(chan[ch].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
    }
  }
}

int DivPlatformESFM::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_ESFM);

      chan[c.chan].macroInit(ins);
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
      for (int o=0; o<4; o++) {
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        if (KVS(c.chan, o)) {
          rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
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
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        
        rWrite(baseAddr+OFFSET_DAM_DVB_LEFT_RIGHT_MODIN,((opE.modIn&7)<<1)|(((opE.left&chan[c.chan].globalPan)&1)<<4)|(((opE.right&(chan[c.chan].globalPan>>1))&1)<<5)|((op.dvb&1)<<6)|(op.dam<<7));
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
      if (destFreq>chan[c.chan].baseFreq) {
        newFreq=chan[c.chan].baseFreq+c.value*((parent->song.linearPitch==2)?1:octave(chan[c.chan].baseFreq));
        if (newFreq>=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      } else {
        newFreq=chan[c.chan].baseFreq-c.value*((parent->song.linearPitch==2)?1:octave(chan[c.chan].baseFreq));
        if (newFreq<=destFreq) {
          newFreq=destFreq;
          return2=true;
        }
      }
      if (!chan[c.chan].portaPause && parent->song.linearPitch!=2) {
        if (octave(chan[c.chan].baseFreq)!=octave(newFreq)) {
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
      unsigned int o = c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32 + o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      op.mult=c.value2&15;
      rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      break;
    }
    case DIV_CMD_FM_TL: {
      unsigned int o = c.value;
      if (o >= 4) break;
      unsigned short baseAddr=c.chan*32 + o*8;
      DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
      op.tl=c.value2&63;
      if (KVS(c.chan, o)) {
        rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
      } else {
        rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.ar=c.value2&15;
          rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.ar=c.value2&15;
        rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.dr=c.value2&15;
          rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.dr=c.value2&15;
        rWrite(baseAddr+OFFSET_AR_DR,(op.ar<<4)|(op.dr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.sl=c.value2&15;
          rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|(op.rr&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.sl=c.value2&15;
        rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|(op.rr&0xf));
      }
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.rr=c.value2&15;
          rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|op.rr);
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.rr=c.value2&15;
        rWrite(baseAddr+OFFSET_SL_RR,(op.sl<<4)|op.rr);
      }
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.am=c.value2&1;
          rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.am=c.value2&1;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_VIB: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.vib=c.value2&1;
          rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.vib=c.value2&1;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_SUS: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.sus=c.value2&1;
          rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.sus=c.value2&1;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_KSR: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.ksr=c.value2&1;
          rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.ksr=c.value2&1;
        rWrite(baseAddr+OFFSET_AM_VIB_SUS_KSR_MULT,((op.am&1)<<7)|((op.vib&1)<<6)|((op.sus&1)<<5)|((op.ksr&1)<<4)|(op.mult&0xf));
      }
      break;
    }
    case DIV_CMD_FM_WS: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
          unsigned char noise=chan[c.chan].state.esfm.noise&3;
          op.ws=c.value2&7;
          if (isMuted[c.chan]) {
            rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
          } else {
            rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
          }
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        DivInstrumentESFM::Operator& opE=chan[c.chan].state.esfm.op[o];
        unsigned char noise=chan[c.chan].state.esfm.noise&3;
        op.ws=c.value2&7;
        if (isMuted[c.chan]) {
          rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|0);
        } else {
          rWrite(baseAddr+OFFSET_OUTLVL_NOISE_WS,(op.ws&7)|((o==3?noise:0)<<3)|((opE.outLvl&7)<<5));
        }
      }
      break;
    }
    // KSL
    case DIV_CMD_FM_RS: {
      if (c.value<0)  {
        for (int o=0; o<4; o++) {
          unsigned short baseAddr=c.chan*32 + o*8;
          DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
          op.ksl=c.value2&3;
          if (KVS(c.chan, o)) {
            rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
          } else {
            rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
          }
        }
      } else {
        unsigned int o = c.value;
        if (o >= 4) break;
        unsigned short baseAddr=c.chan*32 + o*8;
        DivInstrumentFM::Operator& op=chan[c.chan].state.fm.op[o];
        op.ksl=c.value2&3;
        if (KVS(c.chan, o)) {
          rWrite(baseAddr+OFFSET_KSL_TL,(63-VOL_SCALE_LOG_BROKEN(63-op.tl,chan[c.chan].outVol&0x3f,63))|(op.ksl<<6));
        } else {
          rWrite(baseAddr+OFFSET_KSL_TL,(op.tl&0x3f)|(op.ksl<<6));
        }
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
  // TODO: DEBUG, remove this, it impacts performance
  for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
    regPool[i] = ESFM_readback_reg(&chip, i);
  }
  return regPool;
}

int DivPlatformESFM::getRegisterPoolSize() {
  return ESFM_REG_POOL_SIZE;
}

void DivPlatformESFM::reset() {
  while (!writes.empty()) writes.pop();

  ESFM_init(&chip);
  // set chip to native mode
  ESFM_write_reg(&chip, 0x105, 0x80);
  // ensure NTS bit in register 0x408 is reset, for smooth envelope rate scaling
  ESFM_write_reg(&chip, 0x408, 0x00);

  for (int i=0; i<ESFM_REG_POOL_SIZE; i++) {
    regPool[i] = ESFM_readback_reg(&chip, i);
    oldWrites[i] = -1;
    pendingWrites[i] = -1;
  }

  for (int i=0; i<18; i++) {
    chan[i]=DivPlatformESFM::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x3f;
    chan[i].outVol=0x3f;
  }
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

void DivPlatformESFM::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformESFM::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

void DivPlatformESFM::setFlags(const DivConfig& flags) {
  rate=49716;
  chipClock=COLOR_NTSC*4.0;
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
