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

#include "genesis.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <string.h>
#include <math.h>

#define CHIP_FREQBASE fmFreqBase
#define CHIP_DIVIDER fmDivBase

#define IS_REALLY_MUTED(x) (isMuted[x] && (x<5 || !softPCM || (isMuted[5] && isMuted[6])))

void DivYM2612Interface::ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) {
  if (tnum==1) {
    setB=duration_in_clocks;
  } else if (tnum==0) {
    setA=duration_in_clocks;
  }
}

void DivYM2612Interface::clock() {
  if (setA>=0) {
    countA-=144;
    if (countA<0) {
      m_engine->engine_timer_expired(0);
      countA+=setA;
    }
  }
  if (setB>=0) {
    countB-=144;
    if (countB<0) {
      m_engine->engine_timer_expired(1);
      countB+=setB;
    }
  }
}

void DivPlatformGenesis::processDAC(int iRate) {
  if (softPCM) {
    softPCMTimer+=chipClock/576;
    if (softPCMTimer>iRate) {
      softPCMTimer-=iRate;

      int sample=0;
      for (int i=5; i<7; i++) {
        if (chan[i].dacSample!=-1) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (!isMuted[i] && s->samples>0 && chan[i].dacPos<s->samples) {
            if (parent->song.noOPN2Vol) {
              chan[i].dacOutput=s->data8[chan[i].dacDirection?(s->samples-chan[i].dacPos-1):chan[i].dacPos];
            } else {
              chan[i].dacOutput=(s->data8[chan[i].dacDirection?(s->samples-chan[i].dacPos-1):chan[i].dacPos]*dacVolTable[chan[i].outVol])>>7;
            }
            sample+=chan[i].dacOutput;
          } else {
            chan[i].dacOutput=0;
          }
          chan[i].dacPeriod+=chan[i].dacRate;
          if (chan[i].dacPeriod>=(chipClock/576)) {
            if (s->samples>0) {
              while (chan[i].dacPeriod>=(chipClock/576)) {
                ++chan[i].dacPos;
                if (!chan[i].dacDirection && (s->isLoopable() && chan[i].dacPos>=(unsigned int)s->loopEnd)) {
                  chan[i].dacPos=s->loopStart;
                } else if (chan[i].dacPos>=s->samples) {
                  chan[i].dacSample=-1;
                  chan[i].dacPeriod=0;
                  break;
                }
                chan[i].dacPeriod-=(chipClock/576);
              }
            } else {
              chan[i].dacSample=-1;
            }
          }
        }
      }
      //sample>>=1;
      if (sample<-128) sample=-128;
      if (sample>127) sample=127;
      dacWrite=(unsigned char)(sample+0x80);
    }
  } else {
    if (chan[5].dacMode && chan[5].dacSample!=-1) {
      chan[5].dacPeriod+=chan[5].dacRate;
      if (chan[5].dacPeriod>=iRate) {
        DivSample* s=parent->getSample(chan[5].dacSample);
        if (s->samples>0 && chan[5].dacPos<s->samples) {
          if (!isMuted[5]) {
            int sample;
            if (parent->song.noOPN2Vol) {
              sample=s->data8[chan[5].dacDirection?(s->samples-chan[5].dacPos-1):chan[5].dacPos];
            } else {
              sample=(s->data8[chan[5].dacDirection?(s->samples-chan[5].dacPos-1):chan[5].dacPos]*dacVolTable[chan[5].outVol])>>7;
            }
            dacWrite=(unsigned char)(sample+0x80);
          }
          chan[5].dacPos++;
          if (!chan[5].dacDirection && (s->isLoopable() && chan[5].dacPos>=(unsigned int)s->loopEnd)) {
            chan[5].dacPos=s->loopStart;
          } else if (chan[5].dacPos>=s->samples) {
            chan[5].dacSample=-1;
            if (parent->song.brokenDACMode) {
              rWrite(0x2b,0);
            }
          }
          while (chan[5].dacPeriod>=iRate) chan[5].dacPeriod-=iRate;
        } else {
          chan[5].dacSample=-1;
        }
      }
    }
  }
}

void DivPlatformGenesis::acquire_nuked(short** buf, size_t len) {
  thread_local short o[2];
  thread_local int os[2];

  for (size_t h=0; h<len; h++) {
    processDAC(rate);

    os[0]=0; os[1]=0;
    for (int i=0; i<6; i++) {
      if (!writes.empty()) {
        QueuedWrite& w=writes.front();
        if (w.addrOrVal) {
          //logV("%.3x = %.2x",w.addr,w.val);
          OPN2_Write(&fm,0x1+((w.addr>>8)<<1),w.val);
          regPool[w.addr&0x1ff]=w.val;
          writes.pop_front();

          if (dacWrite>=0) {
            if (!canWriteDAC) {
              canWriteDAC=true;
            } else {
              urgentWrite(0x2a,dacWrite);
              dacWrite=-1;
              canWriteDAC=writes.empty();
            }
          }
        } else {
          if (fm.write_busy==0) {
            OPN2_Write(&fm,0x0+((w.addr>>8)<<1),w.addr);
            w.addrOrVal=true;
          }
        }
      } else {
        canWriteDAC=true;
        if (dacWrite>=0) {
          urgentWrite(0x2a,dacWrite);
          dacWrite=-1;
        }
        flushFirst=false;
      }
      
      OPN2_Clock(&fm,o);
      if (chipType==2) {
        os[0]+=CLAMP(o[0],-8192,8191);
        os[1]+=CLAMP(o[1],-8192,8191);
      } else {
        os[0]+=o[0];
        os[1]+=o[1];
      }
      //OPN2_Write(&fm,0,0);
      if (i==5) {
        if (fm.dacen) {
          if (softPCM) {
            oscBuf[5]->data[oscBuf[5]->needle++]=chan[5].dacOutput<<6;
            oscBuf[6]->data[oscBuf[6]->needle++]=chan[6].dacOutput<<6;
          } else {
            oscBuf[i]->data[oscBuf[i]->needle++]=((fm.dacdata^0x100)-0x100)<<6;
            oscBuf[6]->data[oscBuf[6]->needle++]=0;
          }
        } else {
          oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fm.ch_out[i]<<(chipType==2?1:6),-32768,32767);
          oscBuf[6]->data[oscBuf[6]->needle++]=0;
        }
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=CLAMP(fm.ch_out[i]<<(chipType==2?1:6),-32768,32767);
      }
    }
    
    if (chipType!=2) os[0]=(os[0]<<5);
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    if (chipType!=2) os[1]=(os[1]<<5);
    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    buf[0][h]=os[0];
    buf[1][h]=os[1];
  }
}

void DivPlatformGenesis::acquire_ymfm(short** buf, size_t len) {
  thread_local int os[2];

  ymfm::ym2612::fm_engine* fme=fm_ymfm->debug_engine();

  for (size_t h=0; h<len; h++) {
    processDAC(rate);
  
    os[0]=0; os[1]=0;
    if (!writes.empty()) {
      QueuedWrite& w=writes.front();
      fm_ymfm->write(0x0+((w.addr>>8)<<1),w.addr);
      fm_ymfm->write(0x1+((w.addr>>8)<<1),w.val);
      regPool[w.addr&0x1ff]=w.val;
      writes.pop_front();

      if (dacWrite>=0) {
        if (!canWriteDAC) {
          canWriteDAC=true;
        } else {
          urgentWrite(0x2a,dacWrite);
          dacWrite=-1;
          canWriteDAC=writes.empty();
        }
      }
    } else {
      canWriteDAC=true;
      if (dacWrite>=0) {
        urgentWrite(0x2a,dacWrite);
        dacWrite=-1;
      }
      flushFirst=false;
    }
    
    if (chipType==1) {
      fm_ymfm->generate(&out_ymfm);
    } else {
      ((ymfm::ym3438*)fm_ymfm)->generate(&out_ymfm);
    }
    iface.clock();
    os[0]=out_ymfm.data[0];
    os[1]=out_ymfm.data[1];
    //OPN2_Write(&fm,0,0);

    for (int i=0; i<6; i++) {
      int chOut=(fme->debug_channel(i)->debug_output(0)+fme->debug_channel(i)->debug_output(1))<<5;
      if (chOut<-32768) chOut=-32768;
      if (chOut>32767) chOut=32767;
      if (i==5) {
        if (fm_ymfm->debug_dac_enable()) {
          if (softPCM) {
            oscBuf[5]->data[oscBuf[5]->needle++]=chan[5].dacOutput<<6;
            oscBuf[6]->data[oscBuf[6]->needle++]=chan[6].dacOutput<<6;
          } else {
            oscBuf[i]->data[oscBuf[i]->needle++]=((fm_ymfm->debug_dac_data()^0x100)-0x100)<<6;
            oscBuf[6]->data[oscBuf[6]->needle++]=0;
          }
        } else {
          oscBuf[i]->data[oscBuf[i]->needle++]=chOut;
          oscBuf[6]->data[oscBuf[6]->needle++]=0;
        }
      } else {
        oscBuf[i]->data[oscBuf[i]->needle++]=chOut;
      }
    }
    
    if (os[0]<-32768) os[0]=-32768;
    if (os[0]>32767) os[0]=32767;

    if (os[1]<-32768) os[1]=-32768;
    if (os[1]>32767) os[1]=32767;
  
    buf[0][h]=os[0];
    buf[1][h]=os[1];
  }
}

void DivPlatformGenesis::acquire_nuked276(short** buf, size_t len) {
  // TODO
}

void DivPlatformGenesis::acquire(short** buf, size_t len) {
  if (useYMFM==2) {
    acquire_nuked276(buf,len);
  } else if (useYMFM==1) {
    acquire_ymfm(buf,len);
  } else {
    acquire_nuked(buf,len);
  }
}

void DivPlatformGenesis::fillStream(std::vector<DivDelayedWrite>& stream, int sRate, size_t len) {
  writes.clear();
  for (size_t i=0; i<len; i++) {
    processDAC(sRate);

    if (dacWrite>=0) {
      urgentWrite(0x2a,dacWrite);
      dacWrite=-1;
    }

    while (!writes.empty()) {
      QueuedWrite& w=writes.front();
      stream.push_back(DivDelayedWrite(i,w.addr,w.val));
      writes.pop_front();
    }
  }
  regWrites.clear();
}

void DivPlatformGenesis::tick(bool sysTick) {
  for (int i=0; i<(softPCM?7:6); i++) {
    if (i==2 && extMode) continue;
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      int inVol=chan[i].std.vol.val;
      if (chan[i].furnaceDac && inVol>0) {
        inVol+=63;
      }
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol,MIN(127,inVol),127);
      if (i<6) for (int j=0; j<4; j++) {
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

    if (i>=5 && chan[i].furnaceDac) {
      if (NEW_ARP_STRAT) {
        chan[i].handleArp();
      } else if (chan[i].std.arp.had) {
        if (!chan[i].inPorta) {
          chan[i].baseFreq=parent->calcBaseFreq(1,1,parent->calcArp(chan[i].note,chan[i].std.arp.val),false);
        }
        chan[i].freqChanged=true;
      }
    } else {
      if (NEW_ARP_STRAT) {
        chan[i].handleArp();
      } else if (chan[i].std.arp.had) {
        if (!chan[i].inPorta) {
          chan[i].baseFreq=NOTE_FNUM_BLOCK(parent->calcArp(chan[i].note,chan[i].std.arp.val),11);
        }
        chan[i].freqChanged=true;
      }
    }

    if (i>=5 && chan[i].furnaceDac) {
      if (chan[i].std.panL.had) {
        chan[5].pan&=1;
        chan[5].pan|=chan[i].std.panL.val?2:0;
      }
      if (chan[i].std.panR.had) {
        chan[5].pan&=2;
        chan[5].pan|=chan[i].std.panR.val?1:0;
      }
      if (chan[i].std.panL.had || chan[i].std.panR.had) {
        rWrite(chanOffs[5]+ADDR_LRAF,(IS_REALLY_MUTED(i)?0:(chan[5].pan<<6))|(chan[5].state.fms&7)|((chan[5].state.ams&3)<<4));
      }
    } else {
      if (chan[i].std.panL.had) {
        chan[i].pan=chan[i].std.panL.val&3;
        if (i<6) {
          rWrite(chanOffs[i]+ADDR_LRAF,(IS_REALLY_MUTED(i)?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
        }
      }
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

    if (i>=5 && chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1 && chan[i].furnaceDac) {
        chan[i].dacPos=0;
      }
    }

    if (i>=6) continue;

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
      rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    }
    if (chan[i].std.fms.had) {
      chan[i].state.fms=chan[i].std.fms.val;
      rWrite(chanOffs[i]+ADDR_LRAF,(IS_REALLY_MUTED(i)?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].std.ams.had) {
      chan[i].state.ams=chan[i].std.ams.val;
      rWrite(chanOffs[i]+ADDR_LRAF,(IS_REALLY_MUTED(i)?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    }
    if (chan[i].std.ex3.had) {
      lfoValue=(chan[i].std.ex3.val>7)?0:(8|(chan[i].std.ex3.val&7));
      rWrite(0x22,lfoValue);
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
  }

  for (int i=0; i<512; i++) {
    if (pendingWrites[i]!=oldWrites[i]) {
      if (i==0x2b && pendingWrites[i]!=0 && !parent->song.brokenDACMode) {
        if (chan[5].keyOn) chan[5].keyOn=false;
        chan[5].keyOff=true;
      }
      immWrite(i,pendingWrites[i]&0xff);
      oldWrites[i]=pendingWrites[i];
    }
  }

  int hardResetElapsed=0;
  bool mustHardReset=false;

  for (int i=0; i<6; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].keyOn || chan[i].keyOff) {
      immWrite(0x28,0x00|konOffs[i]);
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

  for (int i=0; i<csmChan; i++) {
    if (i==2 && extMode) continue;
    if (chan[i].freqChanged) {
      if (parent->song.linearPitch==2) {
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE,11);
      } else {
        int fNum=parent->calcFreq(chan[i].baseFreq&0x7ff,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,CHIP_FREQBASE,11);
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
      if (i<6) {
        immWrite(chanOffs[i]+ADDR_FREQH,chan[i].freq>>8);
        immWrite(chanOffs[i]+ADDR_FREQ,chan[i].freq&0xff);
         hardResetElapsed+=2;
      }
      if (chan[i].furnaceDac && chan[i].dacMode) {
        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=(double)s->centerRate/8363.0;
          }
        }
        chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,1,1);
        chan[i].dacRate=chan[i].freq*off;
        if (chan[i].dacRate<1) chan[i].dacRate=1;
        if (dumpWrites) addWrite(0xffff0001,chan[i].dacRate);
      }
      chan[i].freqChanged=false;
    }
    if ((chan[i].keyOn || chan[i].opMaskChanged) && !chan[i].hardReset) {
      if (i<6) {
        immWrite(0x28,(chan[i].opMask<<4)|konOffs[i]);
        hardResetElapsed++;
      }
      chan[i].opMaskChanged=false;
      chan[i].keyOn=false;
    }
  }

  // hard reset handling
  if (mustHardReset) {
    for (unsigned int i=hardResetElapsed; i<hardResetCycles; i++) {
      immWrite(0xf0,i&0xff);
    }
    for (int i=0; i<csmChan; i++) {
      if (i==2 && extMode) continue;
      if ((chan[i].keyOn || chan[i].opMaskChanged) && chan[i].hardReset) {
        if (i<6) {
          // restore SL/RR
          for (int j=0; j<4; j++) {
            unsigned short baseAddr=chanOffs[i]|opOffs[j];
            DivInstrumentFM::Operator& op=chan[i].state.op[j];
            immWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
          }
          immWrite(0x28,(chan[i].opMask<<4)|konOffs[i]);
        }
        chan[i].opMaskChanged=false;
        chan[i].keyOn=false;
      }
    }
  }
}

void DivPlatformGenesis::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (ch>6) return;
  if (ch<6) {
    if (ch==5) immWrite(0x2a,0x80);
    for (int j=0; j<4; j++) {
      unsigned short baseAddr=chanOffs[ch]|opOffs[j];
      DivInstrumentFM::Operator& op=chan[ch].state.op[j];
      if (isMuted[ch] || !op.enable) {
        rWrite(baseAddr+ADDR_TL,127);
      } else {
        if (KVS(ch,j)) {
          rWrite(baseAddr+ADDR_TL,127-VOL_SCALE_LOG_BROKEN(127-op.tl,chan[ch].outVol&0x7f,127));
        } else {
          rWrite(baseAddr+ADDR_TL,op.tl);
        }
      }
    }
  } else {
    ch--;
  }
  rWrite(chanOffs[ch]+ADDR_LRAF,(IS_REALLY_MUTED(ch)?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));
}

void DivPlatformGenesis::commitState(int ch, DivInstrument* ins) {
  if (chan[ch].insChanged) {
    chan[ch].state=ins->fm;
    chan[ch].opMask=
      (chan[ch].state.op[0].enable?1:0)|
      (chan[ch].state.op[2].enable?2:0)|
      (chan[ch].state.op[1].enable?4:0)|
      (chan[ch].state.op[3].enable?8:0);
  }
  
  for (int i=0; i<4; i++) {
    unsigned short baseAddr=chanOffs[ch]|opOffs[i];
    DivInstrumentFM::Operator& op=chan[ch].state.op[i];
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
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
  }
  if (chan[ch].insChanged) {
    rWrite(chanOffs[ch]+ADDR_FB_ALG,(chan[ch].state.alg&7)|(chan[ch].state.fb<<3));
    rWrite(chanOffs[ch]+ADDR_LRAF,(IS_REALLY_MUTED(ch)?0:(chan[ch].pan<<6))|(chan[ch].state.fms&7)|((chan[ch].state.ams&3)<<4));
  }
}

int DivPlatformGenesis::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
      if (c.chan==csmChan && extMode) { // CSM
        chan[c.chan].macroInit(ins);
        chan[c.chan].insChanged=false;

        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
          chan[c.chan].portaPause=false;
          chan[c.chan].note=c.value;
          chan[c.chan].freqChanged=true;
        }
        chan[c.chan].keyOn=true;
        chan[c.chan].active=true;
        break;
      }
      if (c.chan>=5) {
        if (ins->type==DIV_INS_AMIGA) {
          chan[c.chan].dacMode=1;
          rWrite(0x2b,1<<7);
        } else if (chan[c.chan].furnaceDac) {
          chan[c.chan].dacMode=0;
          rWrite(0x2b,0<<7);
        } else if (!chan[c.chan].dacMode) {
          rWrite(0x2b,0<<7);
        }
      }
      if (c.chan>=5 && chan[c.chan].dacMode) {
        //if (skipRegisterWrites) break;
        if (ins->type==DIV_INS_AMIGA) { // Furnace mode
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].dacSample=ins->amiga.getSample(c.value);
            c.value=ins->amiga.getFreq(c.value);
          }
          if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002,0);
            break;
          } else {
            rWrite(0x2b,1<<7);
            if (dumpWrites) {
              addWrite(0xffff0000,chan[c.chan].dacSample);
              addWrite(0xffff0003,chan[c.chan].dacDirection);
            }
          }
          if (chan[c.chan].setPos) {
            chan[c.chan].setPos=false;
          } else {
            chan[c.chan].dacPos=0;
          }
          chan[c.chan].dacPeriod=0;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value,false);
            chan[c.chan].portaPause=false;
            chan[c.chan].note=c.value;
            chan[c.chan].freqChanged=true;
          }
          chan[c.chan].furnaceDac=true;

          chan[c.chan].macroInit(ins);
          if (!chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
          }

          // ???
          //chan[c.chan].keyOn=true;
          chan[c.chan].active=true;
        } else { // compatible mode
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          chan[c.chan].dacSample=12*chan[c.chan].sampleBank+chan[c.chan].note%12;
          if (chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002,0);
            break;
          } else {
            rWrite(0x2b,1<<7);
            if (dumpWrites) addWrite(0xffff0000,chan[c.chan].dacSample);
          }
          chan[c.chan].dacPos=0;
          chan[c.chan].dacPeriod=0;
          chan[c.chan].dacRate=MAX(1,parent->getSample(chan[c.chan].dacSample)->rate);
          if (dumpWrites) addWrite(0xffff0001,parent->getSample(chan[c.chan].dacSample)->rate);
          chan[c.chan].furnaceDac=false;
        }
        break;
      }
      if (c.chan>=6) break;

      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }

      commitState(c.chan,ins);
      chan[c.chan].insChanged=false;

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
        chan[c.chan].portaPause=false;
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].keyOn=true;
      chan[c.chan].active=true;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      if (c.chan>=5 && c.chan<csmChan) {
        chan[c.chan].dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
        if (parent->song.brokenDACMode) {
          rWrite(0x2b,0);
          if (chan[c.chan].dacMode) break;
        }
      }
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].active=false;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      if (c.chan>=5) {
        chan[c.chan].dacSample=-1;
        if (dumpWrites) addWrite(0xffff0002,0);
      }
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
      if (c.chan>=6) break;
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
      if (c.chan>5) c.chan=5;
      if (c.value==0 && c.value2==0) {
        chan[c.chan].pan=3;
      } else {
        chan[c.chan].pan=(c.value2>0)|((c.value>0)<<1);
      }
      rWrite(chanOffs[c.chan]+ADDR_LRAF,(IS_REALLY_MUTED(c.chan)?0:(chan[c.chan].pan<<6))|(chan[c.chan].state.fms&7)|((chan[c.chan].state.ams&3)<<4));
      break;
    }
    case DIV_CMD_PITCH: {
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_NOTE_PORTA: {
      if (parent->song.linearPitch==2) {
        int destFreq=NOTE_FREQUENCY(c.value2);
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
      if (c.chan==csmChan) {
        int destFreq=NOTE_PERIODIC(c.value2);
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
      if (c.chan>=5 && chan[c.chan].furnaceDac && chan[c.chan].dacMode) {
        int destFreq=parent->calcBaseFreq(1,1,c.value2,false);
        bool return2=false;
        if (destFreq>chan[c.chan].baseFreq) {
          chan[c.chan].baseFreq+=c.value*16;
          if (chan[c.chan].baseFreq>=destFreq) {
            chan[c.chan].baseFreq=destFreq;
            return2=true;
          }
        } else {
          chan[c.chan].baseFreq-=c.value*16;
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
    case DIV_CMD_SAMPLE_MODE: {
      if (c.chan<5) c.chan=5;
      chan[c.chan].dacMode=c.value;
      rWrite(0x2b,c.value<<7);
      break;
    }
    case DIV_CMD_SAMPLE_BANK:
      if (c.chan<5) c.chan=5;
      chan[c.chan].sampleBank=c.value;
      if (chan[c.chan].sampleBank>(parent->song.sample.size()/12)) {
        chan[c.chan].sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_SAMPLE_DIR: {
      if (c.chan<5) c.chan=5;
      chan[c.chan].dacDirection=c.value;
      if (dumpWrites) addWrite(0xffff0003,chan[c.chan].dacDirection);
      break;
    }
    case DIV_CMD_SAMPLE_POS:
      if (c.chan<5) c.chan=5;
      chan[c.chan].dacPos=c.value;
      chan[c.chan].setPos=true;
      if (dumpWrites) addWrite(0xffff0005,chan[c.chan].dacPos);
      break;
    case DIV_CMD_LEGATO: {
      if (c.chan==csmChan) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
      } else if (c.chan>=5 && chan[c.chan].furnaceDac && chan[c.chan].dacMode) {
        chan[c.chan].baseFreq=parent->calcBaseFreq(1,1,c.value,false);
      } else {
        if (chan[c.chan].insChanged) {
          DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_FM);
          commitState(c.chan,ins);
          chan[c.chan].insChanged=false;
        }
        chan[c.chan].baseFreq=NOTE_FNUM_BLOCK(c.value,11);
      }
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
      break;
    }
    case DIV_CMD_FM_EXTCH: {
      if (extSys) {
        if (extMode==(bool)c.value) break;
        extMode=c.value;
        immWrite(0x27,extMode?0x40:0);
      }
      break;
    }
    case DIV_CMD_FM_LFO: {
      if (c.chan>=6) break;
      lfoValue=(c.value&7)|((c.value>>4)<<3);
      rWrite(0x22,lfoValue);
      break;
    }
    case DIV_CMD_FM_FB: {
      if (c.chan>=6) break;
      chan[c.chan].state.fb=c.value&7;
      rWrite(chanOffs[c.chan]+ADDR_FB_ALG,(chan[c.chan].state.alg&7)|(chan[c.chan].state.fb<<3));
      break;
    }
    case DIV_CMD_FM_MULT: {
      if (c.chan>=6) break;
      unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
      DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
      op.mult=c.value2&15;
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      break;
    }
    case DIV_CMD_FM_TL: {
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
      if (c.value<0)  {
        for (int i=0; i<4; i++) {
          DivInstrumentFM::Operator& op=chan[c.chan].state.op[i];
          op.ar=c.value2&31;
          unsigned short baseAddr=chanOffs[c.chan]|opOffs[i];
          rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
        }
      } else if (c.value<4) {
        DivInstrumentFM::Operator& op=chan[c.chan].state.op[orderedOps[c.value]];
        op.ar=c.value2&31;
        unsigned short baseAddr=chanOffs[c.chan]|opOffs[orderedOps[c.value]];
        rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      }
      break;
    }
    case DIV_CMD_FM_RS: {
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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
      if (c.chan>=6) break;
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

#define DRSLD2R(x) \
  if (chan[i].state.op[x].dr<dr) dr=chan[i].state.op[x].dr; \
  if (chan[i].state.op[x].sl<sl) sl=chan[i].state.op[x].sl; \
  if (chan[i].state.op[x].d2r<d2r) d2r=chan[i].state.op[x].d2r;

void DivPlatformGenesis::forceIns() {
  for (int i=0; i<6; i++) {
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
      rWrite(baseAddr+ADDR_MULT_DT,(op.mult&15)|(dtTable[op.dt&7]<<4));
      rWrite(baseAddr+ADDR_RS_AR,(op.ar&31)|(op.rs<<6));
      rWrite(baseAddr+ADDR_AM_DR,(op.dr&31)|(op.am<<7));
      rWrite(baseAddr+ADDR_DT2_D2R,op.d2r&31);
      rWrite(baseAddr+ADDR_SL_RR,(op.rr&15)|(op.sl<<4));
      rWrite(baseAddr+ADDR_SSG,op.ssgEnv&15);
    }
    rWrite(chanOffs[i]+ADDR_FB_ALG,(chan[i].state.alg&7)|(chan[i].state.fb<<3));
    rWrite(chanOffs[i]+ADDR_LRAF,(IS_REALLY_MUTED(i)?0:(chan[i].pan<<6))|(chan[i].state.fms&7)|((chan[i].state.ams&3)<<4));
    if (chan[i].active) {
      bool sustained=false;
      unsigned char dr=chan[i].state.op[3].dr;
      unsigned char sl=chan[i].state.op[3].sl;
      unsigned char d2r=chan[i].state.op[3].d2r;

      switch (chan[i].state.alg&7) {
        case 4:
          DRSLD2R(2);
          break;
        case 5:
        case 6:
          DRSLD2R(2);
          DRSLD2R(1);
          break;
        case 7:
          DRSLD2R(2);
          DRSLD2R(1);
          DRSLD2R(3);
          break;
      }

      if (dr<2 || (sl<15 && d2r<2)) sustained=true;
      if ((i<5 || !chan[i].dacMode) && sustained) {
        chan[i].keyOn=true;
        chan[i].freqChanged=true;
      }
    }
  }
  immWrite(0x2b,0x00);
  //rWrite(0x2a,0x00);
  if (chan[5].dacMode) {
    chan[5].dacSample=-1;
    chan[6].dacSample=-1;
  }
  immWrite(0x22,lfoValue);
  flushFirst=true;
}

void DivPlatformGenesis::toggleRegisterDump(bool enable) {
  DivDispatch::toggleRegisterDump(enable);
}

void* DivPlatformGenesis::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformGenesis::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformGenesis::getPan(int ch) {
  if (ch>5) ch=5;
  return ((chan[ch].pan&2)<<7)|(chan[ch].pan&1);
}

DivSamplePos DivPlatformGenesis::getSamplePos(int ch) {
  if (!chan[5].dacMode) return DivSamplePos();
  if (ch<5) return DivSamplePos();
  if (ch>5 && !softPCM) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformGenesis::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformGenesis::getRegisterPool() {
  return regPool;
}

int DivPlatformGenesis::getRegisterPoolSize() {
  return 512;
}

float DivPlatformGenesis::getPostAmp() {
  return 2.0f;
}

void DivPlatformGenesis::reset() {
  writes.clear();
  memset(regPool,0,512);
  if (useYMFM==2) {
    memset(&fm_276,0,sizeof(fmopn2_t));
  } else if (useYMFM==1) {
    fm_ymfm->reset();
  }
  OPN2_Reset(&fm);
  switch (chipType) {
    case 1: // YM2612
      OPN2_SetChipType(&fm,ym3438_mode_ym2612);
      break;
    case 2: // YMF276
      OPN2_SetChipType(&fm,ym3438_mode_opn);
      break;
    default: // YM3438
      OPN2_SetChipType(&fm,0);
      break;
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  for (int i=0; i<10; i++) {
    chan[i]=DivPlatformGenesis::Channel();
    chan[i].std.setEngine(parent);
    chan[i].vol=0x7f;
    chan[i].outVol=0x7f;
  }

  for (int i=0; i<512; i++) {
    oldWrites[i]=-1;
    pendingWrites[i]=-1;
  }

  lfoValue=8;
  softPCMTimer=0;
  extMode=false;
  flushFirst=false;
  dacWrite=-1;
  canWriteDAC=true;

  if (softPCM) {
    chan[5].dacMode=true;
    chan[6].dacMode=true;
  }

  // normal sample direction
  if (dumpWrites) addWrite(0xffff0003,0);

  // LFO
  immWrite(0x22,lfoValue);
}

int DivPlatformGenesis::getOutputCount() {
  return 2;
}

bool DivPlatformGenesis::keyOffAffectsArp(int ch) {
  return (ch>5);
}

bool DivPlatformGenesis::keyOffAffectsPorta(int ch) {
  return (ch>5);
}

void DivPlatformGenesis::notifyInsChange(int ins) {
  for (int i=0; i<10; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformGenesis::notifyInsDeletion(void* ins) {
  for (int i=0; i<10; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformGenesis::poke(unsigned int addr, unsigned short val) {
  immWrite(addr,val);
}

void DivPlatformGenesis::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

int DivPlatformGenesis::getPortaFloor(int ch) {
  return 0;
}

void DivPlatformGenesis::setYMFM(unsigned char use) {
  useYMFM=use;
}

void DivPlatformGenesis::setSoftPCM(bool value) {
  softPCM=value;
}

void DivPlatformGenesis::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 1:
      chipClock=COLOR_PAL*12.0/7.0;
      break;
    case 2:
      chipClock=8000000.0;
      break;
    case 3:
      chipClock=COLOR_NTSC*12.0/7.0;
      break;
    case 4:
      chipClock=COLOR_NTSC*9.0/4.0;
      break;
    default:
      chipClock=COLOR_NTSC*15.0/7.0;
      break;
  }
  if (flags.has("chipType")) {
    chipType=flags.getInt("chipType",0);
  } else {
    chipType=flags.getBool("ladderEffect",false)?1:0;
  }
  noExtMacros=flags.getBool("noExtMacros",false);
  fbAllOps=flags.getBool("fbAllOps",false);
  switch (chipType) {
    case 1: // YM2612
      OPN2_SetChipType(&fm,ym3438_mode_ym2612);
      break;
    case 2: // YMF276
      OPN2_SetChipType(&fm,ym3438_mode_opn);
      break;
    default: // YM3438
      OPN2_SetChipType(&fm,0);
      break;
  }
  CHECK_CUSTOM_CLOCK;
  if (useYMFM==1) {
    if (fm_ymfm!=NULL) delete fm_ymfm;
    if (chipType==1) {
      fm_ymfm=new ymfm::ym2612(iface);
    } else {
      fm_ymfm=new ymfm::ym3438(iface);
    }
    rate=chipClock/144;
  } else {
    rate=chipClock/36;
  }
  for (int i=0; i<10; i++) {
    oscBuf[i]->rate=rate;
  }
}

int DivPlatformGenesis::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  chipType=0;
  skipRegisterWrites=false;
  flushFirst=false;
  for (int i=0; i<10; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  fm_ymfm=NULL;
  setFlags(flags);

  for (int i=0; i<128; i++) {
    dacVolTable[127-i]=128*pow(10.0f,(float)(-i)*0.75f/20.0f);
  }
  dacVolTable[0]=0;

  reset();
  return 10;
}

void DivPlatformGenesis::quit() {
  for (int i=0; i<10; i++) {
    delete oscBuf[i];
  }
  if (fm_ymfm!=NULL) delete fm_ymfm;
}

DivPlatformGenesis::~DivPlatformGenesis() {
}
