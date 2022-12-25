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

#include "opll.h"
#include "../engine.h"
#include <string.h>
#include <math.h>

#define rWrite(a,v) if (!skipRegisterWrites) {pendingWrites[a]=v;}
#define immWrite(a,v) if (!skipRegisterWrites) {writes.emplace(a,v); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 1180068

const unsigned char cycleMapOPLL[18]={
  8, 7, 6, 7, 8, 7, 8, 6, 0, 1, 2, 7, 8, 9, 3, 4, 5, 9
};

const unsigned char drumSlot[11]={
  0, 0, 0, 0, 0, 0, 6, 7, 8, 8, 7
};

const unsigned char visMapOPLL[9]={
  6, 7, 8, 3, 4, 5, 0, 1, 2
};

void DivPlatformOPLL::acquire_nuked(short* bufL, short* bufR, size_t start, size_t len) {
  static int o[2];
  static int os;

  for (size_t h=start; h<start+len; h++) {
    os=0;
    for (int i=0; i<9; i++) {
      if (!writes.empty() && --delay<0) {
        // 84 is safe value
        QueuedWrite& w=writes.front();
        if (w.addrOrVal) {
          OPLL_Write(&fm,1,w.val);
          //printf("write: %x = %.2x\n",w.addr,w.val);
          regPool[w.addr&0xff]=w.val;
          writes.pop();
          delay=21;
        } else {
          //printf("busycounter: %d\n",lastBusy);
          OPLL_Write(&fm,0,w.addr);
          w.addrOrVal=true;
          delay=3;
        }
      }
      
      OPLL_Clock(&fm,o);
      unsigned char nextOut=cycleMapOPLL[fm.cycles];
      if ((nextOut>=6 && properDrums) || !isMuted[nextOut]) {
        os+=(o[0]+o[1]);
        if (vrc7 || (fm.rm_enable&0x20)) oscBuf[nextOut]->data[oscBuf[nextOut]->needle++]=(o[0]+o[1])<<6;
      } else {
        if (vrc7 || (fm.rm_enable&0x20)) oscBuf[nextOut]->data[oscBuf[nextOut]->needle++]=0;
      }
    }
    if (!(vrc7 || (fm.rm_enable&0x20))) for (int i=0; i<9; i++) {
      unsigned char ch=visMapOPLL[i];
      if ((i>=6 && properDrums) || !isMuted[ch]) {
        oscBuf[ch]->data[oscBuf[ch]->needle++]=(fm.output_ch[i])<<6;
      } else {
        oscBuf[ch]->data[oscBuf[ch]->needle++]=0;
      }
    }
    os*=50;
    if (os<-32768) os=-32768;
    if (os>32767) os=32767;
    bufL[h]=os;
  }
}

void DivPlatformOPLL::acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len) {
}

void DivPlatformOPLL::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  acquire_nuked(bufL,bufR,start,len);
}

void DivPlatformOPLL::tick(bool sysTick) {
  for (int i=0; i<11; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(15,chan[i].std.vol.val),15);
      if (i<9) {
        rWrite(0x30+i,((15-VOL_SCALE_LOG_BROKEN(chan[i].outVol,15-chan[i].state.op[1].tl,15))&15)|(chan[i].state.opllPreset<<4));
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

    if (chan[i].std.wave.had && chan[i].state.opllPreset!=16) {
      chan[i].state.opllPreset=chan[i].std.wave.val;
      if (i<9) {
        rWrite(0x30+i,((15-VOL_SCALE_LOG_BROKEN(chan[i].outVol,15-chan[i].state.op[1].tl,15))&15)|(chan[i].state.opllPreset<<4));
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

    if (chan[i].state.opllPreset==0) {
      if (chan[i].std.alg.had) { // SUS
        chan[i].state.alg=chan[i].std.alg.val;
        chan[i].freqChanged=true;
      }
      if (chan[i].std.fb.had) {
        chan[i].state.fb=chan[i].std.fb.val;
        rWrite(0x03,(chan[i].state.op[1].ksl<<6)|((chan[i].state.fms&1)<<4)|((chan[i].state.ams&1)<<3)|chan[i].state.fb);
      }
      if (chan[i].std.fms.had) {
        chan[i].state.fms=chan[i].std.fms.val;
        rWrite(0x03,(chan[i].state.op[1].ksl<<6)|((chan[i].state.fms&1)<<4)|((chan[i].state.ams&1)<<3)|chan[i].state.fb);
      }
      if (chan[i].std.ams.had) {
        chan[i].state.ams=chan[i].std.ams.val;
        rWrite(0x03,(chan[i].state.op[1].ksl<<6)|((chan[i].state.fms&1)<<4)|((chan[i].state.ams&1)<<3)|chan[i].state.fb);
      }

      for (int j=0; j<2; j++) {
        DivInstrumentFM::Operator& op=chan[i].state.op[j];
        DivMacroInt::IntOp& m=chan[i].std.op[j];

        if (m.am.had) {
          op.am=m.am.val;
          rWrite(0x00+j,(op.am<<7)|(op.vib<<6)|((op.ssgEnv&8)<<2)|(op.ksr<<4)|(op.mult));
        }
        if (m.ar.had) {
          op.ar=m.ar.val;
          rWrite(0x04+j,(op.ar<<4)|(op.dr));
        }
        if (m.dr.had) {
          op.dr=m.dr.val;
          rWrite(0x04+j,(op.ar<<4)|(op.dr));
        }
        if (m.mult.had) {
          op.mult=m.mult.val;
          rWrite(0x00+j,(op.am<<7)|(op.vib<<6)|((op.ssgEnv&8)<<2)|(op.ksr<<4)|(op.mult));
        }
        if (m.rr.had) {
          op.rr=m.rr.val;
          rWrite(0x06+j,(op.sl<<4)|(op.rr));
        }
        if (m.sl.had) {
          op.sl=m.sl.val;
          rWrite(0x06+j,(op.sl<<4)|(op.rr));
        }
        if (m.tl.had) {
          op.tl=((j==1)?15:63)-m.tl.val;
          if (j==1) {
            if (i<9) {
              rWrite(0x30+i,((15-VOL_SCALE_LOG_BROKEN(chan[i].outVol,15-chan[i].state.op[1].tl,15))&15)|(chan[i].state.opllPreset<<4));
            }
          } else {
            rWrite(0x02,(chan[i].state.op[0].ksl<<6)|(op.tl&63));
          }
        }

        if (m.egt.had) {
          op.ssgEnv=(m.egt.val&1)?8:0;
          rWrite(0x00+j,(op.am<<7)|(op.vib<<6)|((op.ssgEnv&8)<<2)|(op.ksr<<4)|(op.mult));
        }
        if (m.ksl.had) {
          op.ksl=m.ksl.val;
          if (j==1) {
            rWrite(0x02,(chan[i].state.op[0].ksl<<6)|(chan[i].state.op[0].tl&63));
          } else {
            rWrite(0x03,(chan[i].state.op[1].ksl<<6)|((chan[i].state.fms&1)<<4)|((chan[i].state.ams&1)<<3)|chan[i].state.fb);
          }
        }
        if (m.ksr.had) {
          op.ksr=m.ksr.val;
          rWrite(0x00+j,(op.am<<7)|(op.vib<<6)|((op.ssgEnv&8)<<2)|(op.ksr<<4)|(op.mult));
        }
        if (m.vib.had) {
          op.vib=m.vib.val;
          rWrite(0x00+j,(op.am<<7)|(op.vib<<6)|((op.ssgEnv&8)<<2)|(op.ksr<<4)|(op.mult));
        }
      }
    }

    if (chan[i].keyOn || chan[i].keyOff) {
      if (i>=6 && properDrums) {
        drumState&=~(0x10>>(i-6));
        immWrite(0x0e,0x20|drumState);
      } else if (i>=6 && drums) {
        drumState&=~(0x10>>(chan[i].note%12));
        immWrite(0x0e,0x20|drumState);
      } else {
        if (i<9) {
          immWrite(0x20+i,(chan[i].freqH)|(chan[i].state.alg?0x20:0));
        }
      }
      //chan[i].keyOn=false;
      chan[i].keyOff=false;
    }
  }

  for (int i=0; i<256; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  for (int i=0; i<11; i++) {
    if (chan[i].freqChanged) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,octave(chan[i].baseFreq)*2,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].fixedFreq>0) chan[i].freq=chan[i].fixedFreq;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;
      int freqt=toFreq(chan[i].freq);
      if (freqt>4095) freqt=4095;
      chan[i].freqL=freqt&0xff;
      if (i>=6 && properDrums && (i<9 || !noTopHatFreq)) {
        immWrite(0x10+drumSlot[i],freqt&0xff);
        immWrite(0x20+drumSlot[i],freqt>>8);
      } else if (i<6 || !drums) {
        if (i<9) {
          immWrite(0x10+i,freqt&0xff);
        }
      }
      chan[i].freqH=(freqt>>8)&15;
    }
    if (chan[i].keyOn && i>=6 && properDrums) {
      if (!isMuted[i]) {
        drumState|=(0x10>>(i-6));
        immWrite(0x0e,0x20|drumState);
      }
      chan[i].keyOn=false;
    } else if (chan[i].keyOn && i>=6 && drums) {
      //printf("%d\n",chan[i].note%12);
      drumState|=(0x10>>(chan[i].note%12));
      immWrite(0x0e,0x20|drumState);
      chan[i].keyOn=false;
    } else if ((chan[i].keyOn || chan[i].freqChanged) && i<9) {
      //immWrite(0x28,0xf0|konOffs[i]);
      if (!(i>=6 && properDrums)) {
        if (i<9) {
          immWrite(0x20+i,(chan[i].freqH)|(chan[i].active<<4)|(chan[i].state.alg?0x20:0));
        }
      }
      chan[i].keyOn=false;
    }
    chan[i].freqChanged=false;
  }
}

#define OPLL_C_NUM 343

int DivPlatformOPLL::octave(int freq) {
  if (freq>=OPLL_C_NUM*64) {
    return 128;
  } else if (freq>=OPLL_C_NUM*32) {
    return 64;
  } else if (freq>=OPLL_C_NUM*16) {
    return 32;
  } else if (freq>=OPLL_C_NUM*8) {
    return 16;
  } else if (freq>=OPLL_C_NUM*4) {
    return 8;
  } else if (freq>=OPLL_C_NUM*2) {
    return 4;
  } else if (freq>=OPLL_C_NUM) {
    return 2;
  } else {
    return 1;
  }
  return 1;
}

int DivPlatformOPLL::toFreq(int freq) {
  if (freq>=OPLL_C_NUM*64) {
    return 0xe00|((freq>>7)&0x1ff);
  } else if (freq>=OPLL_C_NUM*32) {
    return 0xc00|((freq>>6)&0x1ff);
  } else if (freq>=OPLL_C_NUM*16) {
    return 0xa00|((freq>>5)&0x1ff);
  } else if (freq>=OPLL_C_NUM*8) {
    return 0x800|((freq>>4)&0x1ff);
  } else if (freq>=OPLL_C_NUM*4) {
    return 0x600|((freq>>3)&0x1ff);
  } else if (freq>=OPLL_C_NUM*2) {
    return 0x400|((freq>>2)&0x1ff);
  } else if (freq>=OPLL_C_NUM) {
    return 0x200|((freq>>1)&0x1ff);
  } else {
    return freq&0x1ff;
  }
}

void DivPlatformOPLL::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

int DivPlatformOPLL::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_OPLL);
      if (chan[c.chan].insChanged) {
        chan[c.chan].state=ins->fm;
      }

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      if (c.chan>=6 && properDrums) { // drums mode
        chan[c.chan].insChanged=false;
        if (c.value!=DIV_NOTE_NULL) {
          if (chan[c.chan].state.opllPreset==16 && chan[c.chan].state.fixedDrums) {
            switch (c.chan) {
              case 6:
                chan[c.chan].fixedFreq=(chan[c.chan].state.kickFreq&511)<<(chan[c.chan].state.kickFreq>>9);
                break;
              case 7: case 10:
                chan[c.chan].fixedFreq=(chan[c.chan].state.snareHatFreq&511)<<(chan[c.chan].state.snareHatFreq>>9);
                break;
              case 8: case 9:
                chan[c.chan].fixedFreq=(chan[c.chan].state.tomTopFreq&511)<<(chan[c.chan].state.tomTopFreq>>9);
                break;
              default:
                chan[c.chan].fixedFreq=0;
                chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
                break;
            }
          } else {
            chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
          }
          chan[c.chan].note=c.value;
          chan[c.chan].freqChanged=true;
        }
        chan[c.chan].keyOn=true;
        chan[c.chan].active=true;
        break;
      }
      
      if (chan[c.chan].insChanged) {
        // update custom preset
        if (chan[c.chan].state.opllPreset==0) {
          DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
          DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
          rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
          rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
          rWrite(0x02,(mod.ksl<<6)|(mod.tl&63));
          rWrite(0x03,(car.ksl<<6)|((chan[c.chan].state.fms&1)<<4)|((chan[c.chan].state.ams&1)<<3)|chan[c.chan].state.fb);
          rWrite(0x04,(mod.ar<<4)|(mod.dr));
          rWrite(0x05,(car.ar<<4)|(car.dr));
          rWrite(0x06,(mod.sl<<4)|(mod.rr));
          rWrite(0x07,(car.sl<<4)|(car.rr));
          lastCustomMemory=c.chan;
        }
        if (chan[c.chan].state.opllPreset==16) { // compatible drums mode
          if (c.chan>=6) {
            drums=true;
            immWrite(0x16,0x20);
            immWrite(0x26,0x05);
            immWrite(0x16,0x20);
            immWrite(0x26,0x05);
            immWrite(0x17,0x50);
            immWrite(0x27,0x05);
            immWrite(0x17,0x50);
            immWrite(0x27,0x05);
            immWrite(0x18,0xC0);
            immWrite(0x28,0x01);
          }
        } else {
          if (c.chan>=6) {
            if (drums) {
              drums=false;
              immWrite(0x0e,0);
              drumState=0;
            }
          }
          if (c.chan<9) {
            rWrite(0x30+c.chan,((15-VOL_SCALE_LOG_BROKEN(chan[c.chan].outVol,15-chan[c.chan].state.op[1].tl,15))&15)|(chan[c.chan].state.opllPreset<<4));
          }
        }
      }

      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].note=c.value;

        if (c.chan>=6 && drums) {
          switch (chan[c.chan].note%12) {
            case 0: // kick
              drumVol[0]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            case 1: // snare
              drumVol[1]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            case 2: // tom
              drumVol[2]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            case 3: // top
              drumVol[3]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
            default: // hi-hat
              drumVol[4]=(15-(chan[c.chan].outVol*(15-chan[c.chan].state.op[1].tl))/15);
              break;
          }
          rWrite(0x36,drumVol[0]);
          rWrite(0x37,drumVol[1]|(drumVol[4]<<4));
          rWrite(0x38,drumVol[3]|(drumVol[2]<<4));
        }
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan>=9 && !properDrums) return 0;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].freqChanged=true;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      if (c.chan>=9 && !properDrums) return 0;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].freqChanged=true;
      chan[c.chan].active=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      if (c.chan>=9 && !properDrums) return 0;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_VOLUME: {
      if (c.chan>=9 && !properDrums) return 0;
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (c.chan>=6 && properDrums) {
        drumVol[c.chan-6]=15-chan[c.chan].outVol;
        rWrite(0x36,drumVol[0]);
        rWrite(0x37,drumVol[1]|(drumVol[4]<<4));
        rWrite(0x38,drumVol[3]|(drumVol[2]<<4));
        break;
      } else if (c.chan<6 || !drums) {
        if (c.chan<9) {
          rWrite(0x30+c.chan,((15-VOL_SCALE_LOG_BROKEN(chan[c.chan].outVol,15-chan[c.chan].state.op[1].tl,15))&15)|(chan[c.chan].state.opllPreset<<4));
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
      if (c.chan>=9 && !properDrums) return 0;
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (c.chan>=9 && !properDrums) return 0;
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
      /*if (!chan[c.chan].portaPause) {
        if (octave(chan[c.chan].baseFreq)!=octave(newFreq)) {
          chan[c.chan].portaPause=true;
          break;
        }
      }*/
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
      if (c.chan>=9 && !properDrums) return 0;
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_FB: {
      if (c.chan>=9 && !properDrums) return 0;
      //DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      chan[c.chan].state.fb=c.value&7;
      rWrite(0x03,(car.ksl<<6)|((chan[c.chan].state.fms&1)<<4)|((chan[c.chan].state.ams&1)<<3)|chan[c.chan].state.fb);
      break;
    }
    
    case DIV_CMD_FM_MULT: {
      if (c.chan>=9 && !properDrums) return 0;
      if (c.value==0) {
        DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
        mod.mult=c.value2&15;
        rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      } else {
        DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
        car.mult=c.value2&15;
        rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      }
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>=9 && !properDrums) return 0;
      if (c.value==0) {
        DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
        //DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
        mod.tl=c.value2&63;
        rWrite(0x02,(mod.ksl<<6)|(mod.tl&63));
      } else {
        DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
        car.tl=c.value2&15;
        if (c.chan<9) {
          rWrite(0x30+c.chan,((15-VOL_SCALE_LOG_BROKEN(chan[c.chan].outVol,15-chan[c.chan].state.op[1].tl,15))&15)|(chan[c.chan].state.opllPreset<<4));
        }
      }
      break;
    }
    case DIV_CMD_FM_AR: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.ar=c.value2&15;
        car.ar=c.value2&15;
      } else {
        if (c.value==0) {
          mod.ar=c.value2&15;
        } else {
          car.ar=c.value2&15;
        }
      }
      rWrite(0x04,(mod.ar<<4)|(mod.dr));
      rWrite(0x05,(car.ar<<4)|(car.dr));
      break;
    }
    case DIV_CMD_FM_DR: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.dr=c.value2&15;
        car.dr=c.value2&15;
      } else {
        if (c.value==0) {
          mod.dr=c.value2&15;
        } else {
          car.dr=c.value2&15;
        }
      }
      rWrite(0x04,(mod.ar<<4)|(mod.dr));
      rWrite(0x05,(car.ar<<4)|(car.dr));
      break;
    }
    case DIV_CMD_FM_SL: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.sl=c.value2&15;
        car.sl=c.value2&15;
      } else {
        if (c.value==0) {
          mod.sl=c.value2&15;
        } else {
          car.sl=c.value2&15;
        }
      }
      rWrite(0x06,(mod.sl<<4)|(mod.rr));
      rWrite(0x07,(car.sl<<4)|(car.rr));
      break;
    }
    case DIV_CMD_FM_RR: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.rr=c.value2&15;
        car.rr=c.value2&15;
      } else {
        if (c.value==0) {
          mod.rr=c.value2&15;
        } else {
          car.rr=c.value2&15;
        }
      }
      rWrite(0x06,(mod.sl<<4)|(mod.rr));
      rWrite(0x07,(car.sl<<4)|(car.rr));
      break;
    }
    case DIV_CMD_FM_AM: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.am=c.value2&1;
        car.am=c.value2&1;
      } else {
        if (c.value==0) {
          mod.am=c.value2&1;
        } else {
          car.am=c.value2&1;
        }
      }
      rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      break;
    }
    case DIV_CMD_FM_VIB: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.vib=c.value2&1;
        car.vib=c.value2&1;
      } else {
        if (c.value==0) {
          mod.vib=c.value2&1;
        } else {
          car.vib=c.value2&1;
        }
      }
      rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      break;
    }
    case DIV_CMD_FM_KSR: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.ksr=c.value2&1;
        car.ksr=c.value2&1;
      } else {
        if (c.value==0) {
          mod.ksr=c.value2&1;
        } else {
          car.ksr=c.value2&1;
        }
      }
      rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      break;
    }
    case DIV_CMD_FM_SUS: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.ssgEnv=c.value2?8:0;
        car.ssgEnv=c.value2?8:0;
      } else {
        if (c.value==0) {
          mod.ssgEnv=c.value2?8:0;
        } else {
          car.ssgEnv=c.value2?8:0;
        }
      }
      rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.chan>=9 && !properDrums) return 0;
      DivInstrumentFM::Operator& mod=chan[c.chan].state.op[0];
      DivInstrumentFM::Operator& car=chan[c.chan].state.op[1];
      if (c.value<0)  {
        mod.ksl=c.value2&3;
        car.ksl=c.value2&3;
      } else {
        if (c.value==0) {
          mod.ksl=c.value2&3;
        } else {
          car.ksl=c.value2&3;
        }
      }
      rWrite(0x02,(mod.ksl<<6)|(mod.tl&63));
      rWrite(0x03,(car.ksl<<6)|((chan[c.chan].state.fms&1)<<4)|((chan[c.chan].state.ams&1)<<3)|chan[c.chan].state.fb);
      break;
    }
    case DIV_CMD_FM_EXTCH:
      if (!properDrumsSys) break;
      if ((int)properDrums==c.value) break;
      if (c.value) {
        properDrums=true;
        immWrite(0x0e,0x20);
      } else {
        properDrums=false;
        immWrite(0x0e,0x00);
        drumState=0;
      }
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
      return 15;
      break;
    case DIV_CMD_PRE_PORTA:
      if (c.chan>=9 && !properDrums) return 0;
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
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

void DivPlatformOPLL::forceIns() {
  for (int i=0; i<9; i++) {
    if (i>=6 && properDrums) continue;
    // update custom preset
    if (chan[i].state.opllPreset==0 && i==lastCustomMemory) {
      DivInstrumentFM::Operator& mod=chan[i].state.op[0];
      DivInstrumentFM::Operator& car=chan[i].state.op[1];
      rWrite(0x00,(mod.am<<7)|(mod.vib<<6)|((mod.ssgEnv&8)<<2)|(mod.ksr<<4)|(mod.mult));
      rWrite(0x01,(car.am<<7)|(car.vib<<6)|((car.ssgEnv&8)<<2)|(car.ksr<<4)|(car.mult));
      rWrite(0x02,(mod.ksl<<6)|(mod.tl&63));
      rWrite(0x03,(car.ksl<<6)|((chan[i].state.fms&1)<<4)|((chan[i].state.ams&1)<<3)|chan[i].state.fb);
      rWrite(0x04,(mod.ar<<4)|(mod.dr));
      rWrite(0x05,(car.ar<<4)|(car.dr));
      rWrite(0x06,(mod.sl<<4)|(mod.rr));
      rWrite(0x07,(car.sl<<4)|(car.rr));
    }
    if (i<9) {
      rWrite(0x30+i,((15-VOL_SCALE_LOG_BROKEN(chan[i].outVol,15-chan[i].state.op[1].tl,15))&15)|(chan[i].state.opllPreset<<4));
    }
    if (!(i>=6 && properDrums)) {
      if (chan[i].active) {
        chan[i].keyOn=true;
        chan[i].freqChanged=true;
        chan[i].insChanged=true;
      }
    }
  }
  if (drums) { // WHAT?! FIX THIS!
    immWrite(0x16,0x20);
    immWrite(0x26,0x05);
    immWrite(0x16,0x20);
    immWrite(0x26,0x05);
    immWrite(0x17,0x50);
    immWrite(0x27,0x05);
    immWrite(0x17,0x50);
    immWrite(0x27,0x05);
    immWrite(0x18,0xC0);
    immWrite(0x28,0x01);
  }
  // restore drum volumes
  if (properDrums) {
    rWrite(0x36,drumVol[0]);
    rWrite(0x37,drumVol[1]|(drumVol[4]<<4));
    rWrite(0x38,drumVol[3]|(drumVol[2]<<4));
  }
  drumState=0;
}

void DivPlatformOPLL::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void DivPlatformOPLL::setVRC7(bool vrc) {
  vrc7=vrc;
}

void DivPlatformOPLL::setProperDrums(bool pd) {
  properDrums=pd;
  properDrumsSys=pd;
}


void* DivPlatformOPLL::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformOPLL::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformOPLL::getOscBuffer(int ch) {
  if (ch>=9) return NULL;
  return oscBuf[ch];
}

unsigned char* DivPlatformOPLL::getRegisterPool() {
  return regPool;
}

int DivPlatformOPLL::getRegisterPoolSize() {
  return 64;
}

void DivPlatformOPLL::reset() {
  while (!writes.empty()) writes.pop();
  memset(regPool,0,256);
  if (vrc7) {
    OPLL_Reset(&fm,opll_type_ds1001);
  } else {
    OPLL_Reset(&fm,opll_type_ym2413);
    switch (patchSet) {
      case 0:
        fm.patchrom=OPLL_GetPatchROM(opll_type_ym2413);
        break;
      case 1:
        fm.patchrom=OPLL_GetPatchROM(opll_type_ymf281);
        break;
      case 2:
        fm.patchrom=OPLL_GetPatchROM(opll_type_ym2423);
        break;
      case 3:
        fm.patchrom=OPLL_GetPatchROM(opll_type_ds1001);
        break;
    }
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<11; i++) {
    chan[i]=DivPlatformOPLL::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=15;
    chan[i].outVol=15;
  }

  for (int i=0; i<256; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lastBusy=60;
  drumState=0;
  lastCustomMemory=-1;

  drumVol[0]=0;
  drumVol[1]=0;
  drumVol[2]=0;
  drumVol[3]=0;
  drumVol[4]=0;
  
  delay=0;
  drums=false;
  properDrums=properDrumsSys;

  if (properDrums) {
    immWrite(0x0e,0x20);
  }
}

bool DivPlatformOPLL::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformOPLL::keyOffAffectsPorta(int ch) {
  return false;
}

void DivPlatformOPLL::notifyInsChange(int ins) {
  for (int i=0; i<11; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformOPLL::notifyInsDeletion(void* ins) {
}

void DivPlatformOPLL::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformOPLL::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformOPLL::getPortaFloor(int ch) {
  return (ch>5)?12:0;
}

void DivPlatformOPLL::setYMFM(bool use) {
  useYMFM=use;
}

void DivPlatformOPLL::setFlags(const DivConfig& flags) {
  int clockSel=flags.getInt("clockSel",0);
  if (clockSel==3) {
    chipClock=COLOR_NTSC/2.0;
  } else if (clockSel==2) {
    chipClock=4000000.0;
  } else if (clockSel==1) {
    chipClock=COLOR_PAL*4.0/5.0;
  } else {
    chipClock=COLOR_NTSC;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock/36;
  patchSet=flags.getInt("patchSet",0);
  for (int i=0; i<11; i++) {
    oscBuf[i]->rate=rate/2;
  }
  noTopHatFreq=flags.getBool("noTopHatFreq",false);
}

int DivPlatformOPLL::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  patchSet=0;
  for (int i=0; i<11; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  reset();
  return 11;
}

void DivPlatformOPLL::quit() {
  for (int i=0; i<11; i++) {
    delete oscBuf[i];
  }
}

DivPlatformOPLL::~DivPlatformOPLL() {
}
