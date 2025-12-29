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

#include "c64.h"
#include "../engine.h"
#include "sound/c64_fp/siddefs-fp.h"
#include "IconsFontAwesome4.h"
#include <math.h>
#include "../../ta-log.h"

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 524288

const char* regCheatSheetSID[]={
  "FreqL0", "00",
  "FreqH0", "01",
  "PWL0", "02",
  "PWH0", "03",
  "Control0", "04",
  "AtkDcy0", "05",
  "StnRis0", "06",
  "FreqL1", "07",
  "FreqH1", "08",
  "PWL1", "09",
  "PWH1", "0A",
  "Control1", "0B",
  "AtkDcy1", "0C",
  "StnRis1", "0D",
  "FreqL2", "0E",
  "FreqH2", "0F",
  "PWL2", "10",
  "PWH2", "11",
  "Control2", "12",
  "AtkDcy2", "13",
  "StnRis2", "14",
  "FCL", "15",
  "FCH", "16",
  "FilterRes", "17",
  "FilterMode", "18",
  "PotX", "19",
  "PotY", "1A",
  "Osc3", "1B",
  "Env3", "1C",
  NULL
};

const char** DivPlatformC64::getRegisterSheet() {
  return regCheatSheetSID;
}

short DivPlatformC64::runFakeFilter(unsigned char ch, int in) {
  if (!(regPool[0x17]&(1<<ch))) {
    if (regPool[0x18]&0x80 && ch==2) return 0;
    float fin=in;
    if (noSoftPCM) {
      fin*=(float)(regPool[0x18]&15)/20.0f;
    }
    return CLAMP(fin,-32768,32767);
  }

  // taken from dSID
  float fin=in;
  float fout=0;
  float ctf=fakeCutTable[((regPool[0x15]&7)|(regPool[0x16]<<3))&0x7ff];
  float reso=(sidIs6581?
    ((regPool[0x17]>0x5F)?8.0/(float)(regPool[0x17]>>4):1.41):
    (pow(2,((float)(4-(float)(regPool[0x17]>>4))/8)))
  );
  float tmp=fin+fakeBand[ch]*reso+fakeLow[ch];
  if (regPool[0x18]&0x40) {
    fout-=tmp;
  }
  tmp=fakeBand[ch]-tmp*ctf;
  fakeBand[ch]=tmp;
  if (regPool[0x18]&0x20) {
    fout-=tmp;
  }
  tmp=fakeLow[ch]+tmp*ctf;
  fakeLow[ch]=tmp;
  if (regPool[0x18]&0x10) {
    fout+=tmp;
  }

  if (noSoftPCM) {
    fout*=(float)(regPool[0x18]&15)/20.0f;
  }
  return CLAMP(fout,-32768,32767);
}

void DivPlatformC64::processDAC(int sRate) {
  bool didWrite=false;
  if (chan[3].sample>=0 && chan[3].sample<parent->song.sampleLen) {
    chan[3].pcmPeriod-=chan[3].pcmRate*4;
    while (chan[3].pcmPeriod<0) {
      chan[3].pcmPeriod+=sRate;
      DivSample* s=parent->getSample(chan[3].sample);
      if (s!=NULL) {
        if (chan[3].pcmPos<0 || chan[3].pcmPos>=(int)s->samples) {
          chan[3].pcmOut=15;
          didWrite=true;
        } else {
          chan[3].pcmOut=(0x80+s->data8[chan[3].pcmPos])>>4;
          didWrite=true;
        }
        chan[3].pcmPos++;

        if (s->isLoopable() && chan[3].pcmPos>=s->loopEnd) {
          chan[3].pcmPos=s->loopStart;
        } else if (chan[3].pcmPos>=(int)s->samples) {
          chan[3].sample=-1;
          didWrite=true;
        }
      } else {
        chan[3].sample=-1;
        didWrite=true;
      }
    }
  }

  if (didWrite && !isMuted[3]) updateVolume();
}

void DivPlatformC64::acquire(short** buf, size_t len) {
  int dcOff=(sidCore)?0:sid->get_dc(0);
  for (int i=0; i<4; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t i=0; i<len; i++) {
    // run PCM
    pcmCycle+=lineRate;
    while (pcmCycle>=(rate*2)) {
      pcmCycle-=(rate*2);
      processDAC(lineRate);
    }

    // the rest
    if (!writes.empty()) {
      QueuedWrite w=writes.front();
      if (sidCore==2) {
        dSID_write(sid_d,w.addr,w.val);
      } else if (sidCore==1) {
        sid_fp->write(w.addr,w.val);
      } else {
        sid->write(w.addr,w.val);
      }
      regPool[w.addr&0x1f]=w.val;
      writes.pop();
    }
    if (sidCore==2) {
      double o=dSID_render(sid_d);
      buf[0][i]=32767*CLAMP(o,-1.0,1.0);
      if (++writeOscBuf>=4) {
        writeOscBuf=0;
        oscBuf[0]->putSample(i,sid_d->lastOut[0]);
        oscBuf[1]->putSample(i,sid_d->lastOut[1]);
        oscBuf[2]->putSample(i,sid_d->lastOut[2]);
        oscBuf[3]->putSample(i,isMuted[3]?0:(chan[3].pcmOut<<11));
      }
    } else if (sidCore==1) {
      sid_fp->clock(4,&buf[0][i]);
      if (++writeOscBuf>=4) {
        writeOscBuf=0;
        oscBuf[0]->putSample(i,runFakeFilter(0,(sid_fp->lastChanOut[0]-dcOff)>>5));
        oscBuf[1]->putSample(i,runFakeFilter(1,(sid_fp->lastChanOut[1]-dcOff)>>5));
        oscBuf[2]->putSample(i,runFakeFilter(2,(sid_fp->lastChanOut[2]-dcOff)>>5));
        oscBuf[3]->putSample(i,isMuted[3]?0:(chan[3].pcmOut<<11));
      }
    } else {
      sid->clock();
      buf[0][i]=sid->output();
      if (++writeOscBuf>=16) {
        writeOscBuf=0;
        oscBuf[0]->putSample(i,runFakeFilter(0,(sid->last_chan_out[0]-dcOff)>>5));
        oscBuf[1]->putSample(i,runFakeFilter(1,(sid->last_chan_out[1]-dcOff)>>5));
        oscBuf[2]->putSample(i,runFakeFilter(2,(sid->last_chan_out[2]-dcOff)>>5));
        oscBuf[3]->putSample(i,isMuted[3]?0:(chan[3].pcmOut<<11));
      }
    }
  }
  for (int i=0; i<4; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformC64::updateFilter() {
  rWrite(0x15,filtCut&7);
  rWrite(0x16,filtCut>>3);
  rWrite(0x17,(filtRes<<4)|(chan[2].filter<<2)|(chan[1].filter<<1)|(int)(chan[0].filter));
  updateVolume();
}

void DivPlatformC64::updateVolume() {
  if (chan[3].sample>=0 && !isMuted[3]) {
    rWrite(0x18,(filtControl<<4)|chan[3].pcmOut);
  } else {
    rWrite(0x18,(filtControl<<4)|vol);
  }
}

void DivPlatformC64::tick(bool sysTick) {
  bool willUpdateFilter=false;
  for (int _i=0; _i<3; _i++) {
    int i=chanOrder[_i];

    chan[i].std.next();

    if (sysTick) {
      if (chan[i].pw_slide!=0) {
        chan[i].duty-=chan[i].pw_slide;
        chan[i].duty=CLAMP(chan[i].duty,0,0xfff);
        rWrite(i*7+2,chan[i].duty&0xff);
        rWrite(i*7+3,(chan[i].duty>>8)|(chan[i].outVol<<4));
      }
      if (cutoff_slide!=0) {
        filtCut+=cutoff_slide;
        filtCut=CLAMP(filtCut,0,0x7ff);
        updateFilter();
      }
    }

    if (chan[i].std.vol.had) {
      vol=MIN(15,chan[i].std.vol.val);
      willUpdateFilter=true;
    }

    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_FREQUENCY(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.duty.had) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_C64);
      if (ins->c64.dutyIsAbs) {
        chan[i].duty=chan[i].std.duty.val;
      } else {
        if (multiplyRel) {
          chan[i].duty-=((signed char)chan[i].std.duty.val)*4;
        } else {
          chan[i].duty-=chan[i].std.duty.val;
        }
      }
      chan[i].duty&=4095;
      rWrite(i*7+2,chan[i].duty&0xff);
      rWrite(i*7+3,chan[i].duty>>8);
    }
    if (chan[i].std.wave.had) {
      chan[i].wave=chan[i].std.wave.val;
      rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active && chan[i].gate));
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-65535,65535);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    bool condition=chan[i].std.alg.will;
    DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_C64);
    if (filterOld) {
      condition=chan[i].std.alg.had && (_i==2||macroRace);
    } else if ((!ins->c64.filterIsAbs) || macroRace) {
      condition=chan[i].std.alg.had;
    }
    if (condition) { // new cutoff macro
      if (ins->c64.filterIsAbs) {
        filtCut=MIN(2047,chan[i].std.alg.val);
      } else {
        if (multiplyRel) {
          filtCut+=((signed char)chan[i].std.alg.val)*7;
        } else {
          filtCut+=chan[i].std.alg.val;
        }
        if (filtCut>2047) filtCut=2047;
        if (filtCut<0) filtCut=0;
      }
      willUpdateFilter=true;
    }
    if (chan[i].std.ex1.had) {
      filtControl=chan[i].std.ex1.val&15;
      willUpdateFilter=true;
    }
    if (chan[i].std.ex2.had) {
      filtRes=chan[i].std.ex2.val&15;
      willUpdateFilter=true;
    }
    if (chan[i].std.ex3.had) {
      chan[i].filter=(chan[i].std.ex3.val&1);
      willUpdateFilter=true;
    }
    if (chan[i].std.ex4.had) {
      chan[i].gate=chan[i].std.ex4.val&1;
      chan[i].sync=chan[i].std.ex4.val&2;
      chan[i].ring=chan[i].std.ex4.val&4;
      chan[i].test=chan[i].std.ex4.val&8;
      chan[i].freqChanged=true;
      rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(int)(chan[i].active && chan[i].gate));
    }

    if (chan[i].std.ex5.had) {
      chan[i].attack=chan[i].std.ex5.val&15;
      rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
    }

    if (chan[i].std.ex6.had) {
      chan[i].decay=chan[i].std.ex6.val&15;
      rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
    }

    if (chan[i].std.ex7.had) {
      chan[i].sustain=chan[i].std.ex7.val&15;
      rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
    }

    if (chan[i].std.ex8.had) {
      chan[i].release=chan[i].std.ex8.val&15;
      rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
    }

    if (sysTick) {
      if (chan[i].testWhen>0) {
        if (--chan[i].testWhen<1) {
          if (!chan[i].resetMask && !chan[i].inPorta) {
            DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_C64);
            rWrite(i*7+5,testAD);
            rWrite(i*7+6,testSR);
            rWrite(i*7+4,(chan[i].wave<<4)|(ins->c64.noTest?0:8)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1));
          }
        }
      }
    }

    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,8,chan[i].pitch2,chipClock,CHIP_FREQBASE);
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>0xffff) chan[i].freq=0xffff;
      if (chan[i].keyOn) {
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|(chan[i].gate?1:0));
      }
      if (chan[i].keyOff) {
        rWrite(i*7+5,(chan[i].attack<<4)|(chan[i].decay));
        rWrite(i*7+6,(chan[i].sustain<<4)|(chan[i].release));
        rWrite(i*7+4,(chan[i].wave<<4)|(chan[i].test<<3)|(chan[i].ring<<2)|(chan[i].sync<<1)|0);
      }
      rWrite(i*7,chan[i].freq&0xff);
      rWrite(i*7+1,chan[i].freq>>8);
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }

  if (chan[3].freqChanged) {
    int i=3;
    double off=1.0;
    if (chan[i].sample>=0 && chan[i].sample<parent->song.sampleLen) {
      DivSample* s=parent->getSample(chan[i].sample);
      if (s->centerRate<1) {
        off=1.0;
      } else {
        off=(double)s->centerRate/parent->getCenterRate();
      }
    }
    chan[i].pcmRate=off*parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,2,1);
    if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].pcmRate);
  }

  if (willUpdateFilter) updateFilter();
}

int DivPlatformC64::dispatch(DivCommand c) {
  if (c.chan>3) return 0;
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_C64);

      if (chan[c.chan].pcm || c.chan>2) {
        if (skipRegisterWrites) break;
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(c.value);
          chan[c.chan].sampleNote=c.value;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
          chan[c.chan].sample=ins->amiga.getSample(chan[c.chan].sampleNote);
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
        }
        if (chan[c.chan].sample<0 || chan[c.chan].sample>=parent->song.sampleLen) {
          chan[c.chan].sample=-1;
          if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
          break;
        }
        if (chan[c.chan].setPos) {
          chan[c.chan].setPos=false;
        } else {
          chan[c.chan].pcmPos=0;
        }
        chan[c.chan].pcmPeriod=0;
        if (c.value!=DIV_NOTE_NULL) {
          chan[c.chan].baseFreq=parent->calcBaseFreq(2,1,c.value,false);
          chan[c.chan].freqChanged=true;
          chan[c.chan].note=c.value;
        }
        chan[c.chan].active=true;
        chan[c.chan].macroInit(ins);
        chan[c.chan].keyOn=true;
        break;
      }

      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].test=false;
      if (((chan[c.chan].insChanged || chan[c.chan].resetDuty || ins->std.waveMacro.len>0) && ins->c64.resetDuty) || chan[c.chan].resetDuty) {
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].wave=(ins->c64.noiseOn<<3)|(ins->c64.pulseOn<<2)|(ins->c64.sawOn<<1)|(int)(ins->c64.triOn);
        chan[c.chan].attack=ins->c64.a;
        chan[c.chan].decay=(ins->c64.s==15)?0:ins->c64.d;
        chan[c.chan].sustain=ins->c64.s;
        chan[c.chan].release=ins->c64.r;
        chan[c.chan].ring=ins->c64.ringMod;
        chan[c.chan].sync=ins->c64.oscSync;
      }
      if (chan[c.chan].insChanged || chan[c.chan].resetFilter) {
        chan[c.chan].filter=ins->c64.toFilter;
        if (ins->c64.initFilter) {
          filtCut=ins->c64.cut;
          filtRes=ins->c64.res;
          filtControl=(int)(ins->c64.lp)|(ins->c64.bp<<1)|(ins->c64.hp<<2)|(ins->c64.ch3off<<3);
        }
        updateFilter();
      }
      if (chan[c.chan].insChanged) {
        chan[c.chan].insChanged=false;
      }
      if (keyPriority) {
        if (chanOrder[1]==c.chan) {
          chanOrder[1]=chanOrder[2];
          chanOrder[2]=c.chan;
        } else if (chanOrder[0]==c.chan) {
          chanOrder[0]=chanOrder[1];
          chanOrder[1]=chanOrder[2];
          chanOrder[2]=c.chan;
        }
      }
      chan[c.chan].macroInit(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].sample=-1;
      chan[c.chan].pcm=false;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      //chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
      if (c.chan>2) break;
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].keyOn=false;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_ENV_RELEASE:
      if (c.chan>2) break;
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (c.chan>2) break;
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          vol=chan[c.chan].outVol;
        } else {
          vol=chan[c.chan].vol;
        }
        updateFilter();
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_FREQUENCY(c.value2);
      if (c.chan>2 || chan[c.chan].pcm) {
        destFreq=parent->calcBaseFreq(2,1,c.value2+chan[c.chan].sampleNoteDelta,false);
      }
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
    case DIV_CMD_STD_NOISE_MODE:
      if (c.chan>2) break;
      chan[c.chan].duty=(c.value*4095)/100;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      break;
    case DIV_CMD_C64_FINE_DUTY:
      if (c.chan>2) break;
      chan[c.chan].duty=c.value;
      rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
      rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      break;
    case DIV_CMD_WAVE:
      if (c.chan>2) break;
      chan[c.chan].wave=c.value;
      rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test<<3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
      break;
    case DIV_CMD_LEGATO:
      if (c.chan>2 || chan[c.chan].pcm) {
        chan[c.chan].baseFreq=parent->calcBaseFreq(2,1,c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)),false);
      } else {
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      }
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.compatFlags.resetMacroOnPorta || parent->song.compatFlags.preNoteNoEffect) {
          chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_C64));
          chan[c.chan].keyOn=true;
        }
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.compatFlags.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_FREQUENCY(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_PRE_NOTE:
      if (c.chan>2) break;
      if (resetTime) chan[c.chan].testWhen=c.value-resetTime+1;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 15;
      break;
    case DIV_CMD_C64_CUTOFF:
      if (c.chan>2) break;
      if (c.value>100) c.value=100;
      filtCut=(c.value+2)*2047/102;
      updateFilter();
      break;
    case DIV_CMD_C64_FINE_CUTOFF:
      if (c.chan>2) break;
      filtCut=c.value;
      updateFilter();
      break;
    case DIV_CMD_C64_RESONANCE:
      if (c.chan>2) break;
      if (c.value>15) c.value=15;
      filtRes=c.value;
      updateFilter();
      break;
    case DIV_CMD_C64_FILTER_MODE:
      if (c.chan>2) break;
      filtControl=c.value&7;
      updateFilter();
      break;
    case DIV_CMD_C64_RESET_TIME:
      if (c.chan>2) break;
      resetTime=c.value;
      break;
    case DIV_CMD_C64_RESET_MASK:
      if (c.chan>2) break;
      chan[c.chan].resetMask=c.value;
      break;
    case DIV_CMD_C64_FILTER_RESET:
      if (c.chan>2) break;
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_C64);
        if (ins->c64.initFilter) {
          filtCut=ins->c64.cut;
          updateFilter();
        }
      }
      chan[c.chan].resetFilter=c.value>>4;
      break;
    case DIV_CMD_C64_DUTY_RESET:
      if (c.chan>2) break;
      if (c.value&15) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_C64);
        chan[c.chan].duty=ins->c64.duty;
        rWrite(c.chan*7+2,chan[c.chan].duty&0xff);
        rWrite(c.chan*7+3,chan[c.chan].duty>>8);
      }
      chan[c.chan].resetDuty=c.value>>4;
      break;
    case DIV_CMD_C64_EXTENDED:
      if (c.chan>2) break;
      switch (c.value>>4) {
        case 0:
          chan[c.chan].attack=c.value&15;
          if (!no1EUpdate) {
            rWrite(c.chan*7+5,(chan[c.chan].attack<<4)|(chan[c.chan].decay));
          }
          break;
        case 1:
          chan[c.chan].decay=c.value&15;
          if (!no1EUpdate) {
            rWrite(c.chan*7+5,(chan[c.chan].attack<<4)|(chan[c.chan].decay));
          }
          break;
        case 2:
          chan[c.chan].sustain=c.value&15;
          if (!no1EUpdate) {
            rWrite(c.chan*7+6,(chan[c.chan].sustain<<4)|(chan[c.chan].release));
          }
          break;
        case 3:
          chan[c.chan].release=c.value&15;
          if (!no1EUpdate) {
            rWrite(c.chan*7+6,(chan[c.chan].sustain<<4)|(chan[c.chan].release));
          }
          break;
        case 4:
          chan[c.chan].ring=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test<<3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          break;
        case 5:
          chan[c.chan].sync=c.value;
          rWrite(c.chan*7+4,(chan[c.chan].wave<<4)|(chan[c.chan].test<<3)|(chan[c.chan].ring<<2)|(chan[c.chan].sync<<1)|(int)(chan[c.chan].active && chan[c.chan].gate));
          break;
        case 6:
          filtControl&=7;
          filtControl|=(!!c.value)<<3;
          break;
      }
      break;
    case DIV_CMD_C64_AD:
      if (c.chan>2) break;
      chan[c.chan].attack=c.value>>4;
      chan[c.chan].decay=c.value&15;
      rWrite(c.chan*7+5,(chan[c.chan].attack<<4)|(chan[c.chan].decay));
      break;
    case DIV_CMD_C64_SR:
      if (c.chan>2) break;
      chan[c.chan].sustain=c.value>>4;
      chan[c.chan].release=c.value&15;
      rWrite(c.chan*7+6,(chan[c.chan].sustain<<4)|(chan[c.chan].release));
      break;
    case DIV_CMD_C64_PW_SLIDE:
      if (c.chan>2) break;
      chan[c.chan].pw_slide=c.value*c.value2;
      break;
    case DIV_CMD_C64_CUTOFF_SLIDE:
      if (c.chan>2) break;
      cutoff_slide=c.value*c.value2;
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
    default:
      break;
  }
  return 1;
}

void DivPlatformC64::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  if (sidCore==2) {
    dSID_setMuteMask(
      sid_d,
      (isMuted[0]?0:1)|
      (isMuted[1]?0:2)|
      (isMuted[2]?0:4)
    );
  } else if (sidCore==1) {
    sid_fp->mute(ch,mute);
  } else {
    sid->set_is_muted(ch,mute);
  }
  if (ch==3) updateVolume();
}

void DivPlatformC64::forceIns() {
  for (int i=0; i<4; i++) {
    chan[i].insChanged=true;
    chan[i].testWhen=0;
    if (chan[i].active) {
      chan[i].keyOn=true;
      chan[i].freqChanged=true;
    }
  }
  updateFilter();
}

void DivPlatformC64::notifyInsChange(int ins) {
  for (int i=0; i<4; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformC64::notifyInsDeletion(void* ins) {
  for (int i=0; i<4; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void* DivPlatformC64::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformC64::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

void DivPlatformC64::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (chan[ch].ring) {
    if (ch==0){
      ret.push_back(DivChannelPair(_("ring"),2));
    } else {
      ret.push_back(DivChannelPair(_("ring"),(ch-1)%3));
    }
  }
  if (chan[ch].sync) {
    if (ch==0) {
      ret.push_back(DivChannelPair(_("sync"),2));
    } else {
      ret.push_back(DivChannelPair(_("sync"),(ch-1)%3));
    }
  }
}

DivChannelModeHints DivPlatformC64::getModeHints(int ch) {
  DivChannelModeHints ret;
  ret.count=1;
  ret.hint[0]=ICON_FA_BELL_SLASH_O;
  ret.type[0]=0;
  if (ch==2 && (filtControl&8)) {
    ret.type[0]=7;
  } else if (chan[ch].test && !chan[ch].gate) {
    ret.type[0]=5;
  } else if (chan[ch].test) {
    ret.type[0]=6;
  } else if (!chan[ch].gate) {
    ret.type[0]=4;
  }

  return ret;
}

DivDispatchOscBuffer* DivPlatformC64::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformC64::getRegisterPool() {
  return regPool;
}

int DivPlatformC64::getRegisterPoolSize() {
  return 32;
}

bool DivPlatformC64::getDCOffRequired() {
  return true;
}

bool DivPlatformC64::getWantPreNote() {
  return true;
}

bool DivPlatformC64::isVolGlobal() {
  return true;
}

float DivPlatformC64::getPostAmp() {
  return (sidCore==1)?3.0f:1.0f;
}

void DivPlatformC64::reset() {
  while (!writes.empty()) writes.pop();
  for (int i=0; i<4; i++) {
    chan[i]=DivPlatformC64::Channel();
    chan[i].std.setEngine(parent);
    fakeLow[i]=0;
    fakeBand[i]=0;
    chan[i].pw_slide=0;
  }

  cutoff_slide=0;
  pcmCycle=0;

  if (sidCore==2) {
    dSID_init(sid_d,chipClock,rate,sidIs6581?6581:8580,needInitTables);
    dSID_setMuteMask(
      sid_d,
      (isMuted[0]?0:1)|
      (isMuted[1]?0:2)|
      (isMuted[2]?0:4)
    );
    needInitTables=false;
  } else if (sidCore==1) {
    sid_fp->reset();
    for (int i=0; i<3; i++) {
      sid_fp->write(i*7+5,testAD);
      sid_fp->write(i*7+6,testSR);
      sid_fp->write(i*7+4,8);
    }
    sid_fp->clockSilent(30000);
    for (int i=0; i<3; i++) {
      sid_fp->write(i*7+5,testAD);
      sid_fp->write(i*7+6,testSR);
      sid_fp->write(i*7+4,0);
    }
    sid_fp->clockSilent(30000);
  } else {
    sid->reset();
  }
  memset(regPool,0,32);

  rWrite(0x18,0x0f);

  filtControl=7;
  filtRes=0;
  filtCut=2047;
  resetTime=initResetTime;
  vol=15;

  chanOrder[0]=0;
  chanOrder[1]=1;
  chanOrder[2]=2;
}

void DivPlatformC64::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformC64::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformC64::setChipModel(bool is6581) {
  sidIs6581=is6581;
}

void DivPlatformC64::setCore(unsigned char which) {
  sidCore=which;
}

void DivPlatformC64::setFlags(const DivConfig& flags) {
  switch (flags.getInt("clockSel",0)) {
    case 0x0: // NTSC C64
      chipClock=COLOR_NTSC*2.0/7.0;
      lineRate=15734;
      break;
    case 0x1: // PAL C64
      chipClock=COLOR_PAL*2.0/9.0;
      lineRate=15625;
      break;
    case 0x2: // SSI 2001
    default:
      chipClock=14318180.0/16.0;
      lineRate=15734;
      break;
  }
  CHECK_CUSTOM_CLOCK;
  rate=chipClock;
  if (sidCore>0) {
    rate/=(sidCore==2)?coreQuality:4;
    if (sidCore==1) sid_fp->setSamplingParameters(chipClock,reSIDfp::DECIMATE,rate,0);
  }
  for (int i=0; i<4; i++) {
    oscBuf[i]->setRate(rate);
  }
  keyPriority=flags.getBool("keyPriority",true);
  no1EUpdate=flags.getBool("no1EUpdate",false);
  multiplyRel=flags.getBool("multiplyRel",false);
  macroRace=flags.getBool("macroRace",false);
  filterOld=flags.getBool("filterOld",false);
  testAD=((flags.getInt("testAttack",0)&15)<<4)|(flags.getInt("testDecay",0)&15);
  testSR=((flags.getInt("testSustain",0)&15)<<4)|(flags.getInt("testRelease",0)&15);
  initResetTime=flags.getInt("initResetTime",2);
  if (initResetTime<0) initResetTime=1;

  // init fake filter table
  // taken from dSID
  double oscBufRate=(double)rate/((sidCore==0)?16.0:4.0);
  double cutRatio=-2.0*3.14*(sidIs6581?((oscBufRate/44100.0)*(20000.0/256.0)):(12500.0/256.0))/oscBufRate;

  for (int i=0; i<2048; i++) {
    double c=(double)i/8.0+0.2;
    if (sidIs6581) {
      if (c<24) {
        c=2.0*sin(771.78/oscBufRate);
      } else {
        c=(44100.0/oscBufRate)-1.263*(44100.0/oscBufRate)*exp(c*cutRatio);
      }
    } else {
      c=1-exp(c*cutRatio);
    }
    fakeCutTable[i]=c;
  }
}

void DivPlatformC64::setCoreQuality(unsigned char q) {
  switch (q) {
    case 0:
      coreQuality=32;
      break;
    case 1:
      coreQuality=16;
      break;
    case 2:
      coreQuality=8;
      break;
    case 3:
      coreQuality=4;
      break;
    case 4:
      coreQuality=2;
      break;
    case 5:
      coreQuality=1;
      break;
    default:
      coreQuality=4;
      break;
  }
}

void DivPlatformC64::setSoftPCM(bool isSoft) {
  noSoftPCM=!isSoft;
}

int DivPlatformC64::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  needInitTables=true;
  writeOscBuf=0;
  for (int i=0; i<4; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  if (sidCore==2) {
    sid=NULL;
    sid_fp=NULL;
    sid_d=new struct SID_chip;
  } else if (sidCore==1) {
    sid=NULL;
    sid_fp=new reSIDfp::SID;
    sid_d=NULL;
  } else {
    sid=new SID;
    sid_fp=NULL;
    sid_d=NULL;
  }

  if (sidIs6581) {
    if (sidCore==2) {
      // do nothing
    } else if (sidCore==1) {
      sid_fp->setChipModel(reSIDfp::MOS6581);
    } else {
      sid->set_chip_model(MOS6581);
    }
  } else {
    if (sidCore==2) {
      // do nothing
    } else if (sidCore==1) {
      sid_fp->setChipModel(reSIDfp::MOS8580);
    } else {
      sid->set_chip_model(MOS8580);
    }
  }

  setFlags(flags);

  reset();

  return 3;
}

void DivPlatformC64::quit() {
  for (int i=0; i<4; i++) {
    delete oscBuf[i];
  }
  if (sid!=NULL) delete sid;
  if (sid_fp!=NULL) delete sid_fp;
  if (sid_d!=NULL) delete sid_d;
}

DivPlatformC64::~DivPlatformC64() {
}
