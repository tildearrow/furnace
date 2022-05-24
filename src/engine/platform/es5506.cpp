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

#include "es5506.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>
#include <map>

#define PITCH_OFFSET ((double)(16*2048*(chanMax+1)))
#define NOTE_ES5506(c,note) (parent->calcBaseFreq(chipClock,chan[c].pcm.freqOffs,note,false))

#define rWrite(a,...) {if(!skipRegisterWrites) {hostIntf32.emplace(4,(a),__VA_ARGS__); }}
#define rRead(a,...) {hostIntf32.emplace(4,(a),__VA_ARGS__);}
#define immWrite(a,...) {hostIntf32.emplace(4,(a),__VA_ARGS__);}
#define pageWrite(p,a,...) \
  if (!skipRegisterWrites) { \
    if (curPage!=(p)) { \
      curPage=(p); \
      rWrite(0xf,curPage); \
    } \
    rWrite((a),__VA_ARGS__); \
  }

#define pageWriteMask(p,pm,a,...) \
  if (!skipRegisterWrites) { \
    if ((curPage&(pm))!=((p)&(pm))) { \
      curPage=(curPage&~(pm))|((p)&(pm)); \
      rWrite(0xf,curPage,(pm)); \
    } \
    rWrite((a),__VA_ARGS__); \
  }

#define pageReadMask(p,pm,a,...) \
  if (!skipRegisterWrites) { \
    if ((curPage&(pm))!=((p)&(pm))) { \
      curPage=(curPage&~(pm))|((p)&(pm)); \
      rWrite(0xf,curPage,(pm)); \
    } \
    rRead((a),__VA_ARGS__); \
  }


const char* regCheatSheetES5506[]={
	"CR", "00|00",
	"FC", "00|01",
	"LVOL", "00|02",
	"LVRAMP", "00|03",
	"RVOL", "00|04",
	"RVRAMP", "00|05",
	"ECOUNT", "00|06",
	"K2", "00|07",
	"K2RAMP", "00|08",
	"K1", "00|09",
	"K1RAMP", "00|0A",
	"ACTV", "00|0B",
	"MODE", "00|0C",
	"POT", "00|0D",
	"IRQV", "00|0E",
	"PAGE", "00|0F",
	"CR", "20|00",
	"START", "20|01",
	"END", "20|02",
	"ACCUM", "20|03",
	"O4(n-1)", "20|04",
	"O3(n-2)", "20|05",
	"O3(n-1)", "20|06",
	"O2(n-2)", "20|07",
	"O2(n-1)", "20|08",
	"O1(n-1)", "20|09",
	"W_ST", "20|0A",
	"W_END", "20|0B",
	"LR_END", "20|0C",
	"POT", "20|0D",
	"IRQV", "20|0E",
	"PAGE", "20|0F",
	"CH0L", "40|00",
	"CH0R", "40|01",
	"CH1L", "40|02",
	"CH1R", "40|03",
	"CH2L", "40|04",
	"CH2R", "40|05",
	"CH3L", "40|06",
	"CH3R", "40|07",
	"CH4L", "40|08",
	"CH4R", "40|09",
	"CH5L", "40|0A",
	"CH5R", "40|0B",
	"POT", "40|0D",
	"IRQV", "40|0E",
	"PAGE", "40|0F",
  NULL
};

const char** DivPlatformES5506::getRegisterSheet() {
  return regCheatSheetES5506;
}

const char* DivPlatformES5506::getEffectName(unsigned char effect) {
  switch (effect) {
    case 0x10:
      return "10xx: Change waveform or sample, transwave index";
      break;
    case 0x11:
      return "11xx: Set filter mode (00 to 03)";
      break;
    case 0x12:
      return "120x: Set pause (bit 0)";
      break;
    case 0x13:
      return "130x: Set transwave slice mode (bit 0)";
      break;
    case 0x14:
      return "14xx: Set filter coefficient K1 low byte";
      break;
    case 0x15:
      return "15xx: Set filter coefficient K1 high byte";
      break;
    case 0x16:
      return "16xx: Set filter coefficient K2 low byte";
      break;
    case 0x17:
      return "17xx: Set filter coefficient K2 high byte";
      break;
    case 0x18:
      return "18xx: Set filter coefficient K1 slide up";
      break;
    case 0x19:
      return "19xx: Set filter coefficient K1 slide down";
      break;
    case 0x1a:
      return "1axx: Set filter coefficient K2 slide up";
      break;
    case 0x1b:
      return "1bxx: Set filter coefficient K2 slide down";
      break;
    case 0x20:
      return "20xx: Set envelope count (000 to 0FF)";
      break;
    case 0x21:
      return "21xx: Set envelope count (100 to 1FF)";
      break;
    case 0x22:
      return "22xx: Set envelope left volume ramp (signed)";
      break;
    case 0x23:
      return "23xx: Set envelope right volume ramp (signed)";
      break;
    case 0x24:
      return "24xx: Set envelope filter coefficient k1 ramp (signed)";
      break;
    case 0x25:
      return "25xx: Set envelope filter coefficient k1 ramp (signed, slower)";
      break;
    case 0x26:
      return "26xx: Set envelope filter coefficient k2 ramp (signed)";
      break;
    case 0x27:
      return "27xx: Set envelope filter coefficient k2 ramp (signed, slower)";
      break;
    default:
      if ((effect&0xf0)==0x30) {
        return "3xxx: Set filter coefficient K1";
      } else if ((effect&0xf0)==0x40) {
        return "4xxx: Set filter coefficient K2";
      } else if ((effect&0xf0)==0x50) {
        return "5xxx: Set transwave slice point";
      }
      break;
  }
  return NULL;
}
void DivPlatformES5506::acquire(short* bufL, short* bufR, size_t start, size_t len) {
  for (size_t h=start; h<start+len; h++) {
    // convert 32 bit access to 8 bit host interface
    while (!hostIntf32.empty()) {
      QueuedHostIntf w=hostIntf32.front();
      if (w.isRead && (w.read!=NULL)) {
        hostIntf8.emplace(0,w.addr,w.read,w.mask);
        hostIntf8.emplace(1,w.addr,w.read,w.mask);
        hostIntf8.emplace(2,w.addr,w.read,w.mask);
        hostIntf8.emplace(3,w.addr,w.read,w.mask,w.delay);
      } else {
        hostIntf8.emplace(0,w.addr,w.val,w.mask);
        hostIntf8.emplace(1,w.addr,w.val,w.mask);
        hostIntf8.emplace(2,w.addr,w.val,w.mask);
        hostIntf8.emplace(3,w.addr,w.val,w.mask,w.delay);
      }
      hostIntf32.pop();
    }
    prevChanCycle=es5506.voice_cycle();
    es5506.tick_perf();
    bufL[h]=es5506.lout(0);
    bufR[h]=es5506.rout(0);
    for (int i=0; i<32; i++) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(short)(chan[i].oscOut&0xffff);
    }
  }
}

void DivPlatformES5506::e_pin(bool state)
{
  // get channel outputs
  if (es5506.e_falling_edge()) {
    if (es5506.voice_update()) {
      chan[prevChanCycle].lOut=es5506.voice_lout(prevChanCycle);
      chan[prevChanCycle].rOut=es5506.voice_rout(prevChanCycle);
      chan[prevChanCycle].oscOut=CLAMP_VAL((chan[prevChanCycle].lOut+chan[prevChanCycle].rOut)>>5,-32768,32767);
      if (es5506.voice_end()) {
        if (prevChanCycle<31) {
          for (int c=31; c>prevChanCycle; c--) {
            chan[c].lOut=chan[c].rOut=chan[c].oscOut=0;
          }
        }
      }
    }
  }
  // host interface
  if (es5506.e_rising_edge()) {
    if (cycle) { // wait until delay
      cycle--;
    } else if (!hostIntf8.empty()) {
      QueuedHostIntf w=hostIntf8.front();
      unsigned char shift=24-(w.step<<3);
      if (w.isRead) {
        *w.read=((*w.read)&(~((0xff<<shift)&w.mask)))|((es5506.host_r((w.addr<<2)+w.step)<<shift)&w.mask);
        if (w.step==3) {
          if (w.delay>0) {
            cycle+=w.delay;
          }
          isReaded=true;
        } else {
          isReaded=false;
        }
        hostIntf8.pop();
      } else {
        isReaded=false;
        unsigned int mask=(w.mask>>shift)&0xff;
        if ((mask==0xff) || isMasked) {
          if (mask==0xff) {
            maskedVal=(w.val>>shift)&0xff;
          }
          es5506.host_w((w.addr<<2)+w.step,maskedVal);
          if(dumpWrites) {
            addWrite((w.addr<<2)+w.step,maskedVal);
          }
          isMasked=false;
          if ((w.step==3) && (w.delay>0)) {
            cycle+=w.delay;
          }
          hostIntf8.pop();
        } else if (!isMasked) {
          maskedVal=((w.val>>shift)&mask)|(es5506.host_r((w.addr<<2)+w.step)&~mask);
          isMasked=true;
        }
      }
    }
  }
  if (isReaded) {
    isReaded=false;
    if (irqTrigger) {
      irqTrigger=false;
      if ((irqv&0x80)==0) {
        unsigned char ch=irqv&0x1f;
        if (chan[ch].isReverseLoop) { // Reversed loop
          pageWriteMask(0x00|ch,0x5f,0x00,(chan[ch].pcm.reversed?0x0000:0x0040)|0x08,0x78);
          chan[ch].isReverseLoop=false;
        }
        if (chan[ch].transwaveIRQ) {
          if ((chan[ch].cr&0x37)==0x34) { // IRQE = 1, BLE = 1, LPE = 0, LEI = 1
            DivInstrument* ins=parent->getIns(chan[i].ins);
            if (!ins->amiga.useNoteMap && ins->amiga.transWave.enable) {
              const int next=chan[ch].pcm.next;
              if (next>=0 && next<ins->amiga.transWaveMap.size()) {
                DivInstrumentAmiga::TransWaveMap& transWaveInd=ins->amiga.transWaveMap[next];
                int sample=transWaveInd.ind;
                if (sample>=0 && sample<parent->song.sampleLen) {
                  chan[ch].pcm.index=sample;
                  chan[ch].transWave.ind=next;
                  DivSample* s=parent->getSample(sample);
                  // get frequency offset
                  double off=1.0;
                  double center=s->centerRate;
                  if (center<1) {
                    off=1.0;
                  } else {
                    off=(double)center/8363.0;
                  }
                  // get loop mode, transwave loop
                  double loopStart=(double)s->loopStart;
                  double loopEnd=(double)s->loopEnd;
                  DivSampleLoopMode loopMode=s->isLoopable()?s->loopMode:DIV_SAMPLE_LOOPMODE_ONESHOT;
                  if (transWaveInd.loopMode!=DIV_SAMPLE_LOOPMODE_ONESHOT) {
                    loopMode=transWaveInd.loopMode;
                  } else if ((chan[ch].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT) || (!s->isLoopable())) { // default
                    loopMode=DIV_SAMPLE_LOOPMODE_PINGPONG;
                  }
                  // get loop position
                  loopStart=(double)transWaveInd.loopStart;
                  loopEnd=(double)transWaveInd.loopEnd;
                  if (ins->amiga.transWave.sliceEnable) { // sliced loop position?
                    chan[ch].transWave.updateSize(s->samples,loopStart,loopEnd);
                    chan[ch].transWave.slice=transWaveInd.slice;
                    chan[ch].transWave.slicePos(chan[ch].transWave.slice);
                    loopStart=transWaveInd.sliceStart;
                    loopEnd=transWaveInd.sliceEnd;
                  }
                  // get reversed
                  bool reversed=ins->amiga.reversed;
                  if (transWaveInd.reversed!=2) {
                    reversed=transWaveInd.reversed;
                  }
                  const unsigned int start=s->offES5506<<10;
                  const unsigned int length=s->samples-1;
                  const unsigned int end=start+(length<<11);
                  const double nextFreqOffs=PITCH_OFFSET*off;
                  chan[ch].pcm.loopMode=loopMode;
                  chan[ch].pcm.reversed=reversed;
                  chan[ch].pcm.bank=(s->offES5506>>22)&3;
                  chan[ch].pcm.start=start;
                  chan[ch].pcm.loopStart=(start+(unsigned int)(loopStart*2048.0))&0xfffff800;
                  chan[ch].pcm.loopEnd=(start+(unsigned int)((loopEnd-1.0)*2048.0))&0xffffff80;
                  chan[ch].pcm.end=end;
                  chan[ch].pcm.length=length;
                  pageWrite(0x20|ch,0x01,chan[ch].pcm.loopStart);
                  pageWrite(0x20|ch,0x02,chan[ch].pcm.loopEnd);
                  pageWrite(0x20|ch,0x03,(chan[ch].pcm.reversed)?chan[ch].pcm.loopEnd:chan[ch].pcm.loopStart);
                  unsigned int loopFlag=(chan[ch].pcm.bank<<14)|(chan[ch].pcm.reversed?0x0040:0x0000);
                  chan[ch].isReverseLoop=false;
                  switch (chan[ch].pcm.loopMode) {
                    case DIV_SAMPLE_LOOPMODE_ONESHOT: // One shot (no loop)
                    default:
                      break;
                    case DIV_SAMPLE_LOOPMODE_FORWARD: // Foward loop
                      loopFlag|=0x0008;
                      break;
                    case DIV_SAMPLE_LOOPMODE_BACKWARD: // Backward loop: IRQ enable
                      loopFlag|=0x0038;
                      chan[ch].isReverseLoop=true;
                      break;
                    case DIV_SAMPLE_LOOPMODE_PINGPONG: // Pingpong loop: Hardware support
                      loopFlag|=0x0018;
                      break;
                  }
                  // Set loop mode & Bank
                  pageWriteMask(0x00|ch,0x5f,0x00,loopFlag,0xfcfc);
                  if (chan[ch].pcm.nextFreqOffs!=nextFreqOffs) {
                    chan[ch].pcm.nextFreqOffs=nextFreqOffs;
                    chan[ch].noteChanged.offs=1;
                  }
                }
              }
              chan[ch].pcmChanged.changed=0;
            }
          }
          chan[ch].transwaveIRQ=false;
        }
        if (chan[ch].isTransWave) {
          pageReadMask(0x00|ch,0x5f,0x00,&chan[ch].cr);
          chan[ch].transwaveIRQ=true;
          chan[ch].isTransWave=false;
        }
      }
    }
  }
}

void DivPlatformES5506::irqb(bool state) {
  rRead(0x0e,&irqv,0x9f);
  irqTrigger=true;
}

void DivPlatformES5506::tick(bool sysTick) {
  for (int i=0; i<=chanMax; i++) {
    chan[i].std.next();
    DivInstrument* ins=parent->getIns(chan[i].ins);
    signed int k1=chan[i].k1Prev,k2=chan[i].k2Prev;
    // volume/panning macros
    if (chan[i].std.vol.had) {
      const unsigned int nextVol=((chan[i].vol&0xff)*MIN(0xffff,(0xffff*chan[i].std.vol.val)/chan[i].volMacroMax))/0xff;
      if (chan[i].outVol!=nextVol) {
        chan[i].outVol=nextVol;
        if (!isMuted[i]) {
          chan[i].volChanged.changed=0xff;
        }
      }
    }
    if (chan[i].std.panL.had) {
      const unsigned int nextLVol=(((ins->es5506.lVol*(chan[i].lVol&0xff))/0xff)*MIN(0xffff,(0xffff*chan[i].std.panL.val)/chan[i].panMacroMax))/0xffff;
      if (chan[i].outLVol!=nextLVol) {
        chan[i].outLVol=nextLVol;
        if (!isMuted[i]) {
          chan[i].volChanged.lVol=1;
        }
      }
    }
    if (chan[i].std.panR.had) {
      const unsigned int nextRVol=(((ins->es5506.rVol*(chan[i].rVol&0xff))/0xff)*MIN(0xffff,(0xffff*chan[i].std.panR.val)/chan[i].panMacroMax))/0xffff;
      if (chan[i].outRVol!=nextRVol) {
        chan[i].outRVol=nextRVol;
        if (!isMuted[i]) {
          chan[i].volChanged.rVol=1;
        }
      }
    }
    // arpeggio/pitch macros, frequency related
    if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        if (chan[i].std.arp.mode) {
          chan[i].nextNote=chan[i].std.arp.val;
        } else {
          chan[i].nextNote=chan[i].note+chan[i].std.arp.val;
        }
      }
      chan[i].noteChanged.note=1;
    } else {
      if (chan[i].std.arp.mode && chan[i].std.arp.finished) {
        chan[i].nextNote=chan[i].note;
        chan[i].noteChanged.note=1;
      }
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-2048,2048);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    // phase reset macro
    if (chan[i].std.phaseReset.had) {
      if (chan[i].std.phaseReset.val==1) {
        chan[i].keyOn=true;
      }
    }
    // filter macros
    if (chan[i].std.duty.had) {
      if (chan[i].filter.mode!=DivInstrumentES5506::Filter::FilterMode(chan[i].std.duty.val&3)) {
        chan[i].filter.mode=DivInstrumentES5506::Filter::FilterMode(chan[i].std.duty.val&3);
        chan[i].filterChanged.mode=1;
      }
    }
    if (chan[i].std.ex1.had) {
      switch (chan[i].std.ex1.mode) {
        case 0: // relative
          if (chan[i].k1Offs!=chan[i].std.ex1.val) {
            chan[i].k1Offs=chan[i].std.ex1.val;
            chan[i].filterChanged.k1=1;
          }
          break;
        case 1: // absolute
          if (chan[i].filter.k1!=(chan[i].std.ex1.val&0xffff)) {
            chan[i].filter.k1=chan[i].std.ex1.val&0xffff;
            chan[i].filterChanged.k1=1;
          }
          break;
        case 2: { // delta
          const signed int next_k1=CLAMP_VAL(chan[i].k1Offs+chan[i].std.ex1.val,-65535,65535);
          if (chan[i].k1Offs!=next_k1) {
            chan[i].k1Offs=next_k1;
            chan[i].filterChanged.k1=1;
          }
          break;
        }
        default:
          break;
      }
    }
    if (chan[i].std.ex2.had) {
      switch (chan[i].std.ex2.mode) {
        case 0: // relative
          if (chan[i].k2Offs!=chan[i].std.ex1.val) {
            chan[i].k2Offs=chan[i].std.ex1.val;
            chan[i].filterChanged.k2=1;
          }
          break;
        case 1: // absolute
          if (chan[i].filter.k2!=(chan[i].std.ex2.val&0xffff)) {
            chan[i].filter.k2=chan[i].std.ex2.val&0xffff;
            chan[i].filterChanged.k2=1;
          }
          break;
        case 2: { // delta
          const signed int next_k2=CLAMP_VAL(chan[i].k2Offs+chan[i].std.ex2.val,-65535,65535);
          if (chan[i].k2Offs!=next_k2) {
            chan[i].k2Offs=next_k2;
            chan[i].filterChanged.k2=1;
          }
          break;
        }
        default:
          break;
      }
    }
    // envelope macros
    if (chan[i].std.ex3.had) {
      if (chan[i].envelope.ecount!=(chan[i].std.ex3.val&0x1ff)) {
        chan[i].envelope.ecount=chan[i].std.ex3.val&0x1ff;
        chan[i].envChanged.ecount=1;
      }
    }
    if (chan[i].std.ex4.had) {
      if (chan[i].envelope.lVRamp!=chan[i].std.ex4.val) {
        chan[i].envelope.lVRamp=chan[i].std.ex4.val;
        chan[i].envChanged.lVRamp=1;
      }
    }
    if (chan[i].std.ex5.had) {
      if (chan[i].envelope.rVRamp!=chan[i].std.ex5.val) {
        chan[i].envelope.rVRamp=chan[i].std.ex5.val;
        chan[i].envChanged.rVRamp=1;
      }
    }
    if (chan[i].std.ex6.had) {
      if (chan[i].envelope.k1Ramp!=chan[i].std.ex6.val) {
        chan[i].envelope.k1Ramp=chan[i].std.ex6.val;
        chan[i].envChanged.k1Ramp=1;
      }
    }
    if (chan[i].std.ex7.had) {
      if (chan[i].envelope.k2Ramp!=chan[i].std.ex7.val) {
        chan[i].envelope.k2Ramp=chan[i].std.ex7.val;
        chan[i].envChanged.k2Ramp=1;
      }
    }
    if (chan[i].std.ex8.had) {
      if (chan[i].envelope.k1Slow!=(bool)(chan[i].std.ex8.val&1)) {
        chan[i].envelope.k1Slow=chan[i].std.ex8.val&1;
        chan[i].envChanged.k1Ramp=1;
      }
      if (chan[i].envelope.k2Slow!=(bool)(chan[i].std.ex8.val&2)) {
        chan[i].envelope.k2Slow=chan[i].std.ex8.val&2;
        chan[i].envChanged.k2Ramp=1;
      }
    }
    // filter slide
    if (!chan[i].keyOn) {
      if (chan[i].k1Slide!=0 && chan[i].filter.k1>0 && chan[i].filter.k1<65535) {
        signed int next=CLAMP_VAL(chan[i].filter.k1+chan[i].k1Slide,0,65535);
        if (chan[i].filter.k1!=next) {
          chan[i].filter.k1=next;
          chan[i].filterChanged.k1=1;
        }
      }
      if (chan[i].k2Slide!=0 && chan[i].filter.k2>0 && chan[i].filter.k2<65535) {
        signed int next=CLAMP_VAL(chan[i].filter.k2+chan[i].k2Slide,0,65535);
        if (chan[i].filter.k2!=next) {
          chan[i].filter.k2=next;
          chan[i].filterChanged.k2=1;
        }
      }
    }
    // control macros
    if (chan[i].active && chan[i].std.alg.had) {
      if (chan[i].pcm.pause!=(bool)(chan[i].std.alg.val&1)) {
        chan[i].pcm.pause=chan[i].std.alg.val&1;
        if (!chan[i].keyOn) {
          pageWriteMask(0x00|i,0x5f,0x00,chan[i].pcm.pause?0x0002:0x0000,0x0002);
        }
      }
    }
    // transwave macros
    if (chan[i].transWave.enable) {
      if (chan[i].std.wave.had) {
        if (chan[i].std.wave.val>=0 && chan[i].std.wave.val<ins->amiga.transWaveMap.size()) {
          if (chan[i].pcm.next!=chan[i].std.wave.val) {
            chan[i].pcm.next=chan[i].std.wave.val;
            chan[i].pcmChanged.transwaveInd=1;
          }
        }
      }
      if (chan[i].std.fb.had) {
        if (chan[i].transWave.sliceEnable!=(bool)(chan[i].std.fb.val&1)) {
          chan[i].transWave.sliceEnable=chan[i].std.fb.val&1;
          chan[i].pcmChanged.slice=1;
        }
      }
      if (chan[i].std.fms.had) {
        if (chan[i].transWave.slice!=(unsigned short)(chan[i].std.fms.val&0xfff)) {
          chan[i].transWave.slice=chan[i].std.fms.val&0xfff;
          chan[i].pcmChanged.slice=1;
        }
      }
    } else if (chan[i].pcm.isNoteMap) {
    // note map macros
      if (chan[i].std.wave.had) {
        if (chan[i].std.wave.val>=0 && chan[i].std.wave.val<120) {
          if (chan[i].pcm.next!=chan[i].std.wave.val) {
            chan[i].pcm.next=chan[i].std.wave.val;
            chan[i].pcmChanged.index=1;
          }
        }
      }
    } else if (!chan[i].transWave.enable && !chan[i].pcm.isNoteMap) {
      if (chan[i].std.wave.had) {
        if (chan[i].std.wave.val>=0 && chan[i].std.wave.val<parent->song.sampleLen) {
          if (chan[i].pcm.next!=chan[i].std.wave.val) {
            chan[i].pcm.next=chan[i].std.wave.val;
            chan[i].pcmChanged.index=1;
          }
        }
      }
    }
    // update registers
    if (chan[i].volChanged.changed) {
      if (!isMuted[i]) { // calculate volume (16 bit)
        if (chan[i].volChanged.lVol) {
          chan[i].resLVol=(chan[i].outVol*chan[i].outLVol)/0xffff;
          if (!chan[i].keyOn) {
            pageWrite(0x00|i,0x02,chan[i].resLVol);
          }
        }
        if (chan[i].volChanged.rVol) {
          chan[i].resRVol=(chan[i].outVol*chan[i].outRVol)/0xffff;
          if (!chan[i].keyOn) {
            pageWrite(0x00|i,0x04,chan[i].resRVol);
          }
        }
      } else { // mute
        pageWrite(0x00|i,0x02,0);
        pageWrite(0x00|i,0x04,0);
      }
      chan[i].volChanged.changed=0;
    }
    if (chan[i].pcmChanged.changed) {
      if (!chan[i].isTransWave) {
        if (chan[i].pcmChanged.transwaveInd && (!ins->amiga.useNoteMap && ins->amiga.transWave.enable)) {
          const int next=chan[i].pcm.next;
          if (next>=0 && next<ins->amiga.transWaveMap.size()) {
            DivInstrumentAmiga::TransWaveMap& transWaveInd=ins->amiga.transWaveMap[next];
            int sample=transWaveInd.ind;
            if (sample>=0 && sample<parent->song.sampleLen) {
              if (chan[i].pcm.index!=sample) {
                pageWriteMask(0x00|i,0x5f,0x00,0x0034,0x00ff); // Set IRQ
                chan[i].isTranswave=true;
              } else {
                chan[i].transWave.ind=next;
                DivSample* s=parent->getSample(sample);
                // get loop mode, transwave loop
                double loopStart=(double)s->loopStart;
                double loopEnd=(double)s->loopEnd;
                DivSampleLoopMode loopMode=s->isLoopable()?s->loopMode:DIV_SAMPLE_LOOPMODE_ONESHOT;
                if (transWaveInd.loopMode!=DIV_SAMPLE_LOOPMODE_ONESHOT) {
                  loopMode=transWaveInd.loopMode;
                } else if ((chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT) || (!s->isLoopable())) { // default
                  loopMode=DIV_SAMPLE_LOOPMODE_PINGPONG;
                }
                // get loop position
                loopStart=(double)transWaveInd.loopStart;
                loopEnd=(double)transWaveInd.loopEnd;
                if (ins->amiga.transWave.sliceEnable) { // sliced loop position?
                  chan[i].transWave.updateSize(s->samples,loopStart,loopEnd);
                  chan[i].transWave.slice=transWaveInd.slice;
                  chan[i].transWave.slicePos(transWaveInd.slice);
                  loopStart=transWaveInd.sliceStart;
                  loopEnd=transWaveInd.sliceEnd;
                }
                // get reversed
                bool reversed=ins->amiga.reversed;
                if (transWaveInd.reversed!=2) {
                  reversed=transWaveInd.reversed;
                }
                chan[i].pcmChanged.slice=1;
                if ((chan[i].pcm.loopMode!=loopMode) || (chan[i].pcm.reversed!=reversed)) {
                  chan[i].pcm.loopMode=loopMode;
                  chan[i].pcm.reversed=reversed;
                  chan[i].pcmChanged.loopBank=1;
                }
              }
            }
          }
          chan[i].pcmChanged.transwaveInd=0;
        }
        if ((!chan[i].pcmChanged.transwaveInd) && (!chan[i].isTransWave)) {
          if (chan[i].pcmChanged.index) {
            const int next=chan[i].pcm.next;
            bool sampleVaild=false;
            if (((ins->amiga.useNoteMap && !ins->amiga.transWave.enable) && (next>=0 && next<120)) ||
                ((!ins->amiga.useNoteMap && ins->amiga.transWave.enable) && (next>=0 && next<ins->amiga.transWaveMap.size())) ||
                ((!ins->amiga.useNoteMap && !ins->amiga.transWave.enable) && (next>=0 && next<parent->song.sampleLen))) {
              DivInstrumentAmiga::NoteMap& noteMapind=ins->amiga.noteMap[next];
              DivInstrumentAmiga::TransWaveMap& transWaveInd=ins->amiga.transWaveMap[next];
              int sample=next;
              if (ins->amiga.transWave.enable) {
                sample=transWaveInd.ind;
              } else if (ins->amiga.useNoteMap) {
                sample=noteMapind.ind;
              }
              if (sample>=0 && sample<parent->song.sampleLen) {
                sampleVaild=true;
                chan[i].pcm.index=sample;
                chan[i].pcm.isNoteMap=ins->amiga.useNoteMap && !ins->amiga.transWave.enable;
                chan[i].transWave.enable=!ins->amiga.useNoteMap && ins->amiga.transWave.enable;
                chan[i].transWave.ind=next;
                DivSample* s=parent->getSample(sample);
                // get frequency offset
                double off=1.0;
                double center=s->centerRate;
                if (center<1) {
                  off=1.0;
                } else {
                  off=(double)center/8363.0;
                }
                if (ins->amiga.useNoteMap) {
                  off*=(double)noteMapind.freq/((double)MAX(1,center)*pow(2.0,((double)next-48.0)/12.0));
                  chan[i].pcm.note=next;
                }
                // get loop mode, transwave loop
                double loopStart=(double)s->loopStart;
                double loopEnd=(double)s->loopEnd;
                DivSampleLoopMode loopMode=s->isLoopable()?s->loopMode:DIV_SAMPLE_LOOPMODE_ONESHOT;
                if (ins->amiga.transWave.enable) {
                  if (transWaveInd.loopMode!=DIV_SAMPLE_LOOPMODE_ONESHOT) {
                    loopMode=transWaveInd.loopMode;
                  } else if ((chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT) || (!s->isLoopable())) { // default
                    loopMode=DIV_SAMPLE_LOOPMODE_PINGPONG;
                  }
                  // get loop position
                  loopStart=(double)transWaveInd.loopStart;
                  loopEnd=(double)transWaveInd.loopEnd;
                  if (ins->amiga.transWave.sliceEnable) { // sliced loop position?
                    chan[i].transWave.updateSize(s->samples,loopStart,loopEnd);
                    chan[i].transWave.slice=transWaveInd.slice;
                    chan[i].transWave.slicePos(transWaveInd.slice);
                    loopStart=transWaveInd.sliceStart;
                    loopEnd=transWaveInd.sliceEnd;
                  }
                }
                // get reversed
                bool reversed=ins->amiga.reversed;
                if (ins->amiga.transWave.enable&&transWaveInd.reversed!=2) {
                  reversed=transWaveInd.reversed;
                } else if (ins->amiga.useNoteMap&&noteMapind.reversed!=2) {
                  reversed=noteMapind.reversed;
                }
                const unsigned int start=s->offES5506<<10;
                const unsigned int length=s->samples-1;
                const unsigned int end=start+(length<<11);
                const unsigned int nextBank=(s->offES5506>>22)&3;
                const double nextFreqOffs=PITCH_OFFSET*off;
                chan[i].pcm.loopMode=loopMode;
                chan[i].pcm.reversed=reversed;
                chan[i].pcm.bank=nextBank;
                chan[i].pcm.start=start;
                chan[i].pcm.end=end;
                chan[i].pcm.length=length;
                if ((chan[i].pcm.loopMode!=loopMode) || (chan[i].pcm.reversed!=reversed) || (chan[i].pcm.bank!=nextBank)) {
                  chan[i].pcm.loopMode=loopMode;
                  chan[i].pcm.reversed=reversed;
                  chan[i].pcm.bank=nextBank;
                  chan[i].pcmChanged.loopBank=1;
                }
                if (chan[i].pcm.nextFreqOffs!=nextFreqOffs) {
                  chan[i].pcm.nextFreqOffs=nextFreqOffs;
                  chan[i].noteChanged.offs=1;
                }
              }
            }
            if (sampleVaild) {
              if (!chan[i].keyOn) {
                pageWrite(0x20|i,0x03,(chan[i].pcm.reversed)?chan[i].pcm.end:chan[i].pcm.start);
              }
              chan[i].pcmChanged.slice=1;
            }
            chan[i].pcmChanged.index=0;
          }
          if (chan[i].pcmChanged.slice) {
            if (!chan[i].keyOn) {
              if (chan[i].pcm.index>=0 && chan[i].pcm.index<parent->song.sampleLen) {
                // get loop mode, transwave loop
                DivSample* s=parent->getSample(chan[i].pcm.index);
                double loopStart=(double)s->loopStart;
                double loopEnd=(double)s->loopEnd;
                if (ins->amiga.transWave.sliceEnable) { // sliced loop position?
                  chan[i].transWave.updateSize(s->samples,loopStart,loopEnd);
                  chan[i].transWave.slicePos(chan[i].transWave.slice);
                  loopStart=chan[i].transWave.sliceStart;
                  loopEnd=chan[i].transWave.sliceEnd;
                }
                const unsigned int start=s->offES5506<<10;
                const unsigned int nextLoopStart=(start+(unsigned int)(loopStart*2048.0))&0xfffff800;
                const unsigned int nextLoopEnd=(start+(unsigned int)((loopEnd-1.0)*2048.0))&0xffffff80;
                if ((chan[i].pcm.loopStart!=nextLoopStart) || (chan[i].pcm.loopEnd!=nextLoopEnd)) {
                  chan[i].pcm.loopStart=(start+(unsigned int)(loopStart*2048.0))&0xfffff800;
                  chan[i].pcm.loopEnd=(start+(unsigned int)((loopEnd-1.0)*2048.0))&0xffffff80;
                  chan[i].pcmChanged.position=1;
                }
              }
            }
            chan[i].pcmChanged.slice=0;
          }
          if (chan[i].pcmChanged.position) {
            if (!chan[i].keyOn) {
              pageWrite(0x20|i,0x01,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT)?chan[i].pcm.start:chan[i].pcm.loopStart);
              pageWrite(0x20|i,0x02,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT)?chan[i].pcm.end:chan[i].pcm.loopEnd);
            }
            chan[i].pcmChanged.position=0;
          }
          if (chan[i].pcmChanged.loopBank) {
            if (!chan[i].keyOn) {
              unsigned int loopFlag=(chan[i].pcm.bank<<14)|(chan[i].pcm.reversed?0x0040:0x0000);
              chan[i].isReverseLoop=false;
              switch (chan[i].pcm.loopMode) {
                case DIV_SAMPLE_LOOPMODE_ONESHOT: // One shot (no loop)
                default:
                  break;
                case DIV_SAMPLE_LOOPMODE_FORWARD: // Foward loop
                  loopFlag|=0x0008;
                  break;
                case DIV_SAMPLE_LOOPMODE_BACKWARD: // Backward loop: IRQ enable
                  loopFlag|=0x0038;
                  chan[i].isReverseLoop=true;
                  break;
                case DIV_SAMPLE_LOOPMODE_PINGPONG: // Pingpong loop: Hardware support
                  loopFlag|=0x0018;
                  break;
              }
              // Set loop mode & Bank
              pageWriteMask(0x00|i,0x5f,0x00,loopFlag,0xfcfd);
            }
            chan[i].pcmChanged.loopBank=0;
          }
          chan[i].pcmChanged.dummy=0;
        }
      }
    }
    if (chan[i].filterChanged.changed) {
      if (!chan[i].keyOn) {
        if (chan[i].filterChanged.mode) {
          pageWriteMask(0x00|i,0x5f,0x00,(chan[i].filter.mode<<8),0x0300);
        }
        if (chan[i].filterChanged.k2) {
          if (chan[i].std.ex2.mode!=1) { // Relative
            k2=CLAMP_VAL(chan[i].filter.k2+chan[i].k2Offs,0,65535);
          } else {
            k2=chan[i].filter.k2;
          }
        }
        if (chan[i].filterChanged.k1) {
          if (chan[i].std.ex1.mode!=1) { // Relative
            k1=CLAMP_VAL(chan[i].filter.k1+chan[i].k1Offs,0,65535);
          } else {
            k1=chan[i].filter.k1;
          }
        }
      }
      chan[i].filterChanged.changed=0;
    }
    if (chan[i].envChanged.changed) {
      if (!chan[i].keyOn) {
          if (chan[i].envChanged.lVRamp) {
            pageWrite(0x00|i,0x03,((unsigned char)chan[i].envelope.lVRamp)<<8);
          }
          if (chan[i].envChanged.rVRamp) {
            pageWrite(0x00|i,0x05,((unsigned char)chan[i].envelope.rVRamp)<<8);
          }
          if (chan[i].envChanged.ecount) {
            pageWrite(0x00|i,0x06,chan[i].envelope.ecount);
          }
          if (chan[i].envChanged.k2Ramp) {
            pageWrite(0x00|i,0x08,(((unsigned char)chan[i].envelope.k2Ramp)<<8)|(chan[i].envelope.k2Slow?1:0));
          }
          if (chan[i].envChanged.k1Ramp) {
            pageWrite(0x00|i,0x0a,(((unsigned char)chan[i].envelope.k1Ramp)<<8)|(chan[i].envelope.k1Slow?1:0));
          }
      }
      chan[i].envChanged.changed=0;
    }
    if (chan[i].noteChanged.changed) { // note value changed or frequency offset is changed
      if (chan[i].noteChanged.offs) {
        if (chan[i].pcm.freqOffs!=chan[i].pcm.nextFreqOffs) {
          chan[i].pcm.freqOffs=chan[i].pcm.nextFreqOffs;
          const int nextFreq=NOTE_ES5506(i,chan[i].prevNote);
          if (chan[i].nextFreq!=nextFreq) {
            chan[i].nextFreq=nextFreq;
            chan[i].noteChanged.freq=1;
          }
        }
        chan[i].noteChanged.offs=0;
      }
      if (chan[i].noteChanged.note) {
        if (chan[i].prevNote!=chan[i].nextNote) {
          chan[i].prevNote=chan[i].nextNote;
          const int nextFreq=NOTE_ES5506(i,chan[i].nextNote);
          if (chan[i].nextFreq!=nextFreq) {
            chan[i].nextFreq=nextFreq;
            chan[i].noteChanged.freq=1;
          }
        }
        chan[i].noteChanged.note=0;
      }
      if (chan[i].noteChanged.freq) {
        if (chan[i].baseFreq!=chan[i].nextFreq) {
          chan[i].baseFreq=chan[i].nextFreq;
          chan[i].freqChanged=true;
        }
        chan[i].noteChanged.freq=0;
      }
      chan[i].noteChanged.dummy=0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=CLAMP_VAL(parent->calcFreq(chan[i].baseFreq,chan[i].pitch,false,2,chan[i].pitch2,chipClock,chan[i].pcm.freqOffs),0,0x1ffff);
      if (chan[i].keyOn) {
        if (chan[i].pcm.index>=0 && chan[i].pcm.index<parent->song.sampleLen) {
          chan[i].k1Prev=0xffff;
          chan[i].k2Prev=0xffff;
          pageWriteMask(0x00|i,0x5f,0x00,0x0303); // Wipeout CR
          pageWrite(0x00|i,0x06,0); // Clear ECOUNT
          pageWrite(0x20|i,0x03,chan[i].pcm.reversed?chan[i].pcm.end:chan[i].pcm.start); // Set ACCUM to start address
          pageWrite(0x00|i,0x07,0xffff); // Set K1 and K2 to 0xffff
          pageWrite(0x00|i,0x09,0xffff,~0,(chanMax+1)*4*2); // needs to 4 sample period delay
          pageWrite(0x00|i,0x01,chan[i].freq);
          pageWrite(0x20|i,0x01,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT)?chan[i].pcm.start:chan[i].pcm.loopStart);
          pageWrite(0x20|i,0x02,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT)?chan[i].pcm.end:chan[i].pcm.loopEnd);
          // initialize envelope
          pageWrite(0x00|i,0x03,((unsigned char)chan[i].envelope.lVRamp)<<8);
          pageWrite(0x00|i,0x05,((unsigned char)chan[i].envelope.rVRamp)<<8);
          pageWrite(0x00|i,0x0a,(((unsigned char)chan[i].envelope.k1Ramp)<<8)|(chan[i].envelope.k1Slow?1:0));
          pageWrite(0x00|i,0x08,(((unsigned char)chan[i].envelope.k2Ramp)<<8)|(chan[i].envelope.k2Slow?1:0));
          // initialize filter
          pageWriteMask(0x00|i,0x5f,0x00,(chan[i].pcm.bank<<14)|(chan[i].filter.mode<<8),0xc300);
          if ((chan[i].std.ex2.mode!=1) && (chan[i].std.ex2.had)) {
            k2=CLAMP_VAL(chan[i].filter.k2+chan[i].k2Offs,0,65535);
          } else {
            k2=chan[i].filter.k2;
          }
          pageWrite(0x00|i,0x07,k2);
          chan[i].k2Prev=k2;
          if ((chan[i].std.ex1.mode!=1) && (chan[i].std.ex1.had)) {
            k1=CLAMP_VAL(chan[i].filter.k1+chan[i].k1Offs,0,65535);
          } else {
            k1=chan[i].filter.k1;
          }
          pageWrite(0x00|i,0x09,k1);
          chan[i].k1Prev=k1;
          pageWrite(0x00|i,0x02,chan[i].resLVol);
          pageWrite(0x00|i,0x04,chan[i].resRVol);
          unsigned int loopFlag=chan[i].pcm.reversed?0x0040:0x0000;
          chan[i].isReverseLoop=false;
          switch (chan[i].pcm.loopMode) {
            case DIV_SAMPLE_LOOPMODE_ONESHOT: // One shot (no loop)
            default:
              break;
            case DIV_SAMPLE_LOOPMODE_FORWARD: // Foward loop
              loopFlag|=0x0008;
              break;
            case DIV_SAMPLE_LOOPMODE_BACKWARD: // Backward loop: IRQ enable
              loopFlag|=0x0038;
              chan[i].isReverseLoop=true;
              break;
            case DIV_SAMPLE_LOOPMODE_PINGPONG: // Pingpong loop: Hardware support
              loopFlag|=0x0018;
              break;
          }
          if (chan[i].pcm.pause) {
            loopFlag|=0x0002;
          }
          // Run sample
          pageWrite(0x00|i,0x06,chan[i].envelope.ecount); // Clear ECOUNT
          pageWriteMask(0x00|i,0x5f,0x00,loopFlag,0x3cff);
        }
      }
      if (chan[i].keyOff) {
        pageWriteMask(0x00|i,0x5f,0x00,0x0303); // Wipeout CR
      } else if (chan[i].active) {
        pageWrite(0x00|i,0x01,chan[i].freq);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
    if (!chan[i].keyOn) {
      if (chan[i].k2Prev!=k2) {
        pageWrite(0x00|i,0x07,k2);
        chan[i].k2Prev=k2;
      }
      if (chan[i].k1Prev!=k1) {
        pageWrite(0x00|i,0x09,k1);
        chan[i].k1Prev=k1;
      }
    }
  }
}

int DivPlatformES5506::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      bool sampleVaild=false;
      if (((ins->amiga.useNoteMap && !ins->amiga.transWave.enable) && (c.value>=0 && c.value<120)) ||
          ((!ins->amiga.useNoteMap && ins->amiga.transWave.enable) && (ins->amiga.transWave.ind>=0 && ins->amiga.transWave.ind<ins->amiga.transWaveMap.size())) ||
          ((!ins->amiga.useNoteMap && !ins->amiga.transWave.enable) && (ins->amiga.initSample>=0 && ins->amiga.initSample<parent->song.sampleLen))) {
        DivInstrument* ins=parent->getIns(chan[c.chan].ins);
        DivInstrumentAmiga::NoteMap& noteMapind=ins->amiga.noteMap[c.value];
        DivInstrumentAmiga::TransWaveMap& transWaveInd=ins->amiga.transWaveMap[ins->amiga.transWave.ind];
        int sample=ins->amiga.initSample;
        if (ins->amiga.transWave.enable) {
          sample=transWaveInd.ind;
        } else if (ins->amiga.useNoteMap) {
          sample=noteMapind.ind;
        }
        if (sample>=0 && sample<parent->song.sampleLen) {
          sampleVaild=true;
          chan[c.chan].pcm.index=chan[c.chan].pcm.next=sample;
          chan[c.chan].pcm.pause=(chan[c.chan].std.alg.will)?(chan[c.chan].std.alg.val&1):false;
          chan[c.chan].pcm.isNoteMap=ins->amiga.useNoteMap && !ins->amiga.transWave.enable;
          chan[c.chan].transWave.enable=!ins->amiga.useNoteMap && ins->amiga.transWave.enable;
          chan[c.chan].transWave.sliceEnable=ins->amiga.transWave.sliceEnable;
          chan[c.chan].transWave.ind=ins->amiga.transWave.ind;
          DivSample* s=parent->getSample(sample);
          // get frequency offset
          double off=1.0;
          double center=s->centerRate;
          if (center<1) {
            off=1.0;
          } else {
            off=(double)center/8363.0;
          }
          if (ins->amiga.useNoteMap) {
            off*=(double)noteMapind.freq/((double)MAX(1,center)*pow(2.0,((double)c.value-48.0)/12.0));
            chan[c.chan].pcm.note=c.value;
          }
          // get loop mode, transwave loop
          double loopStart=(double)s->loopStart;
          double loopEnd=(double)s->loopEnd;
          DivSampleLoopMode loopMode=s->isLoopable()?s->loopMode:DIV_SAMPLE_LOOPMODE_ONESHOT;
          if (ins->amiga.transWave.enable) {
            if (transWaveInd.loopMode!=DIV_SAMPLE_LOOPMODE_ONESHOT) {
              loopMode=transWaveInd.loopMode;
            } else if ((chan[i].pcm.loopMode==DIV_SAMPLE_LOOPMODE_ONESHOT) || (!s->isLoopable())) { // default
              loopMode=DIV_SAMPLE_LOOPMODE_PINGPONG;
            }
            // get loop position
            loopStart=(double)transWaveInd.loopStart;
            loopEnd=(double)transWaveInd.loopEnd;
            if (ins->amiga.transWave.sliceEnable) { // sliced loop position?
              chan[c.chan].transWave.updateSize(s->samples,loopStart,loopEnd);
              chan[c.chan].transWave.slice=transWaveInd.slice;
              chan[c.chan].transWave.slicePos(transWaveInd.slice);
              loopStart=transWaveInd.sliceStart;
              loopEnd=transWaveInd.sliceEnd;
            }
          }
          // get reversed
          bool reversed=ins->amiga.reversed;
          if (ins->amiga.transWave.enable&&transWaveInd.reversed!=2) {
            reversed=transWaveInd.reversed;
          } else if (ins->amiga.useNoteMap&&noteMapind.reversed!=2) {
            reversed=noteMapind.reversed;
          }
          const unsigned int start=s->offES5506<<10;
          const unsigned int length=s->samples-1;
          const unsigned int end=start+(length<<11);
          chan[c.chan].pcm.loopMode=loopMode;
          chan[c.chan].pcm.freqOffs=PITCH_OFFSET*off;
          chan[c.chan].pcm.reversed=reversed;
          chan[c.chan].pcm.bank=(s->offES5506>>22)&3;
          chan[c.chan].pcm.start=start;
          chan[c.chan].pcm.end=end;
          chan[c.chan].pcm.length=length;
          chan[c.chan].pcm.loopStart=(start+(unsigned int)(loopStart*2048.0))&0xfffff800;
          chan[c.chan].pcm.loopEnd=(start+(unsigned int)((loopEnd-1.0)*2048.0))&0xffffff80;
          chan[c.chan].volMacroMax=ins->type==DIV_INS_AMIGA?64:0xffff;
          chan[c.chan].panMacroMax=ins->type==DIV_INS_AMIGA?127:0xffff;
          chan[c.chan].filter=ins->es5506.filter;
          chan[c.chan].envelope=ins->es5506.envelope;
        }
      }
      if (!sampleVaild) {
        chan[c.chan].pcm.index=chan[c.chan].pcm.next=-1;
        chan[c.chan].filter=DivInstrumentES5506::Filter();
        chan[c.chan].envelope=DivInstrumentES5506::Envelope();
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].note=c.value;
        chan[c.chan].nextNote=chan[c.chan].note;
        chan[c.chan].freqChanged=true;
        chan[c.chan].noteChanged.changed=0xff;
        chan[c.chan].volChanged.changed=0xff;
      }
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=(0xffff*chan[c.chan].vol)/0xff;
      }
      if (!chan[c.chan].std.panL.will) {
        chan[c.chan].outLVol=(ins->es5506.lVol*chan[c.chan].lVol)/0xff;
      }
      if (!chan[c.chan].std.panR.will) {
        chan[c.chan].outRVol=(ins->es5506.rVol*chan[c.chan].rVol)/0xff;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(ins);
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].filter=DivInstrumentES5506::Filter();
      chan[c.chan].envelope=DivInstrumentES5506::Envelope();
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=(unsigned int)(c.value)) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=(0xffff*c.value)/0xff;
          if (!isMuted[c.chan]) {
            chan[c.chan].volChanged.changed=0xff;
          }
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      if (chan[c.chan].std.vol.has) {
        return chan[c.chan].vol;
      }
      return chan[c.chan].outVol;
      break;
    case DIV_CMD_PANNING: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins);
      // Left volume
      if (chan[c.chan].lVol!=(unsigned int)(c.value)) {
        chan[c.chan].lVol=c.value;
        if (!chan[c.chan].std.panL.has) {
          chan[c.chan].outLVol=(ins->es5506.lVol*c.value)/0xff;
          if (!isMuted[c.chan]) {
            chan[c.chan].volChanged.lVol=1;
          }
        }
      }
      // Right volume
      if (chan[c.chan].rVol!=(unsigned int)(c.value2)) {
        chan[c.chan].rVol=c.value2;
        if (!chan[c.chan].std.panR.has) {
          chan[c.chan].outRVol=(ins->es5506.rVol*c.value2)/0xff;
          if (!isMuted[c.chan]) {
            chan[c.chan].volChanged.rVol=1;
          }
        }
      }
      break;
    }
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    // sample commands
    case DIV_CMD_WAVE:
      if (!chan[c.chan].useWave) {
        if (chan[c.chan].active) {
          if (((ins->amiga.useNoteMap && !ins->amiga.transWave.enable) && (c.value>=0 && c.value<120)) ||
              ((!ins->amiga.useNoteMap && ins->amiga.transWave.enable) && (c.value>=0 && c.value<ins->amiga.transWaveMap.size())) ||
              ((!ins->amiga.useNoteMap && !ins->amiga.transWave.enable) && (c.value>=0 && c.value<parent->song.sampleLen))) {
            chan[c.chan].pcm.next=c.value;
            if (!ins->amiga.useNoteMap && ins->amiga.transWave.enable) {
              chan[c.chan].pcmChanged.transwaveInd=1;
            } else {
              chan[c.chan].pcmChanged.index=1;
            }
          }
        }
      }
      // reserved for useWave
      break;
    case DIV_CMD_SAMPLE_TRANSWAVE_SLICE_MODE:
      if (chan[c.chan].transWave.sliceEnable!=(bool)(c.value&1)) {
        chan[c.chan].transWave.sliceEnable=c.value&1;
        chan[c.chan].pcmChanged.slice=1;
      }
      break;
    case DIV_CMD_SAMPLE_TRANSWAVE_SLICE_POS:
      if (chan[c.chan].transWave.sliceEnable && (chan[c.chan].transWave.slice!=(unsigned short)(c.value&0xfff))) {
        chan[c.chan].transWave.slice=c.value&0xfff;
        chan[c.chan].pcmChanged.slice=1;
      }
      break;
    // Filter commands
    case DIV_CMD_ES5506_FILTER_MODE:
      chan[c.chan].filter.mode=DivInstrumentES5506::Filter::FilterMode(c.value&3);
      chan[c.chan].filterChanged.mode=1;
      break;
    case DIV_CMD_ES5506_FILTER_K1:
      chan[c.chan].filter.k1=(chan[c.chan].filter.k1&~c.value2)|(c.value&c.value2);
      chan[c.chan].filterChanged.k1=1;
      break;
    case DIV_CMD_ES5506_FILTER_K2:
      chan[c.chan].filter.k2=(chan[c.chan].filter.k2&~c.value2)|(c.value&c.value2);
      chan[c.chan].filterChanged.k2=1;
      break;
    case DIV_CMD_ES5506_FILTER_K1_SLIDE:
      chan[c.chan].k1Slide=c.value2?(-c.value):c.value;
      break;
    case DIV_CMD_ES5506_FILTER_K2_SLIDE:
      chan[c.chan].k2Slide=c.value2?(-c.value):c.value;
      break;
    // Envelope commands
    case DIV_CMD_ES5506_ENVELOPE_COUNT:
      chan[c.chan].envelope.ecount=c.value&0x1ff;
      chan[c.chan].envChanged.ecount=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_LVRAMP:
      chan[c.chan].envelope.lVRamp=(signed char)(c.value&0xff);
      chan[c.chan].envChanged.lVRamp=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_RVRAMP:
      chan[c.chan].envelope.rVRamp=(signed char)(c.value&0xff);
      chan[c.chan].envChanged.rVRamp=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_K1RAMP:
      chan[c.chan].envelope.k1Ramp=(signed char)(c.value&0xff);
      chan[c.chan].envelope.k1Slow=c.value2&1;
      chan[c.chan].envChanged.k1Ramp=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_K2RAMP:
      chan[c.chan].envelope.k2Ramp=(signed char)(c.value&0xff);
      chan[c.chan].envelope.k2Slow=c.value2&1;
      chan[c.chan].envChanged.k2Ramp=1;
      break;
    // controls
    case DIV_CMD_ES5506_PAUSE:
      if (chan[c.chan].active) {
        if (chan[c.chan].pcm.pause!=(bool)(c.value&1)) {
          chan[c.chan].pcm.pause=c.value&1;
          pageWriteMask(0x00|i,0x5f,0x00,chan[c.chan].pcm.pause?0x0002:0x0000,0x0002);
        }
      }
      break;
    case DIV_CMD_NOTE_PORTA: {
      int nextFreq=chan[c.chan].baseFreq;
      const int destFreq=NOTE_ES5506(c.chan,c.value2);
      bool return2=false;
      if (destFreq>nextFreq) {
        nextFreq+=c.value;
        if (nextFreq>=destFreq) {
          nextFreq=destFreq;
          return2=true;
        }
      } else {
        nextFreq-=c.value;
        if (nextFreq<=destFreq) {
          nextFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].nextFreq=nextFreq;
      chan[c.chan].noteChanged.freq=1;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO: {
      chan[c.chan].note=c.value;
      chan[c.chan].nextNote=chan[c.chan].note+((chan[c.chan].std.arp.will && !chan[c.chan].std.arp.mode)?(chan[c.chan].std.arp.val-12):(0));
      chan[c.chan].noteChanged.note=1;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins));
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS: {
      if (chan[c.chan].useWave) break;
      if (chan[c.chan].active) {
        const unsigned int start=chan[c.chan].transWave.enable?chan[c.chan].pcm.loopStart:chan[c.chan].pcm.start;
        const unsigned int end=chan[c.chan].transWave.enable?chan[c.chan].pcm.loopEnd:chan[c.chan].pcm.length;
        const unsigned int pos=chan[c.chan].pcm.reversed?(end-c.value):c.value;
        if ((chan[c.chan].pcm.reversed && pos>0) || ((!chan[c.chan].pcm.reversed) && pos<end)) {
          pageWrite(0x20|c.chan,0x03,start+(pos<<11));
        }
        break;
      }
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      return 255;
      break;
    case DIV_ALWAYS_SET_VOLUME:
      return 1;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformES5506::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  es5506.set_mute(ch,mute);
}

void DivPlatformES5506::forceIns() {
  for (int i=0; i<=chanMax; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    chan[i].noteChanged.changed=0xff;
    chan[i].volChanged.changed=0xff;
    chan[i].filterChanged.changed=0xff;
    chan[i].envChanged.changed=0xff;
    chan[i].sample=-1;
  }
}

void* DivPlatformES5506::getChanState(int ch) {
  return &chan[ch];
}

void DivPlatformES5506::reset() {
  while (!hostIntf32.empty()) hostIntf32.pop();
  while (!hostIntf8.empty()) hostIntf8.pop();
  for (int i=0; i<32; i++) {
    chan[i]=DivPlatformES5506::Channel();
    chan[i].std.setEngine(parent);
  }
  es5506.reset();
  for (int i=0; i<32; i++) {
    es5506.set_mute(i,isMuted[i]);
  }

  cycle=0;
  curPage=0;
  maskedVal=0;
  irqv=0x80;
  isMasked=false;
  isReaded=false;
  irqTrigger=false;
  transwaveCh=0;
  prevChanCycle=0;
  chanMax=initChanMax;

  pageWriteMask(0x00,0x60,0x0b,chanMax);
  pageWriteMask(0x00,0x60,0x0b,0x1f);
  // set serial output to I2S-ish, 16 bit
  pageWriteMask(0x20,0x60,0x0a,0x01);
  pageWriteMask(0x20,0x60,0x0b,0x11);
  pageWriteMask(0x20,0x60,0x0c,0x20);
  pageWriteMask(0x00,0x60,0x0c,0x08); // Reset serial output
}

bool DivPlatformES5506::isStereo() {
  return true;
}

bool DivPlatformES5506::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformES5506::notifyInsChange(int ins) {
  for (int i=0; i<32; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
}

void DivPlatformES5506::notifyWaveChange(int wave) {
  // TODO when wavetables are added
  // TODO they probably won't be added unless the samples reside in RAM
}

void DivPlatformES5506::notifyInsDeletion(void* ins) {
  for (int i=0; i<32; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformES5506::setFlags(unsigned int flags) {
  initChanMax=MAX(4,flags&0x1f);
  chanMax=initChanMax;
  pageWriteMask(0x00,0x60,0x0b,chanMax);
}

void DivPlatformES5506::poke(unsigned int addr, unsigned short val) {
  immWrite(addr, val);
}

void DivPlatformES5506::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) immWrite(i.addr,i.val);
}

DivDispatchOscBuffer* DivPlatformES5506::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformES5506::getRegisterPool() {
  unsigned char* regPoolPtr = regPool;
  for (unsigned char p=0; p<128; p++) {
    for (unsigned char r=0; r<16; r++) {
      unsigned int reg=es5506.regs_r(p,r,false);
      for (int b=0; b<4; b++) {
        *regPoolPtr++ = reg>>(24-(b<<3));
      }
    }
  }
  return regPool;
}

int DivPlatformES5506::getRegisterPoolSize() {
  return 4*16*128; // 7 bit page x 16 registers per page x 32 bit per registers
}
const void* DivPlatformES5506::getSampleMem(int index) {
  return index == 0 ? sampleMem : NULL;
}

size_t DivPlatformES5506::getSampleMemCapacity(int index) {
  return index == 0 ? 16777216 : 0; // 2Mword x 16bit * 4 banks
}

size_t DivPlatformES5506::getSampleMemUsage(int index) {
  return index == 0 ? sampleMemLen : 0;
}

void DivPlatformES5506::renderSamples() {
  memset(sampleMem,0,getSampleMemCapacity());

  size_t memPos=128;
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    unsigned int length=s->length16;
    // fit sample size to single bank size
    if (length>(4194304-128)) {
      length=4194304-128;
    }
    if ((memPos&0xc00000)!=((memPos+length+128)&0xc00000)) {
      memPos=((memPos+0x3fffff)&0xc00000)+128;
    }
    if (memPos>=(getSampleMemCapacity()-128)) {
      logW("out of ES5506 memory for sample %d!",i);
      break;
    }
    if (memPos+length>=(getSampleMemCapacity()-128)) {
      memcpy(sampleMem+(memPos/sizeof(short)),s->data16,(getSampleMemCapacity()-128)-memPos);
      logW("out of ES5506 memory for sample %d!",i);
    } else {
      memcpy(sampleMem+(memPos/sizeof(short)),s->data16,length);
    }
    s->offES5506=memPos;
    memPos+=length;
  }
  sampleMemLen=memPos+256;
}

int DivPlatformES5506::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  sampleMem=new signed short[getSampleMemCapacity()/sizeof(short)];
  sampleMemLen=0;
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  chipClock=16000000;
  rate=chipClock/16; // 2 E clock tick (16 CLKIN tick) per voice
  for (int i=0; i<32; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->rate=rate;
  }
  setFlags(flags);

  reset();
  return 32;
}

void DivPlatformES5506::quit() {
  delete[] sampleMem;
  for (int i=0; i<32; i++) {
    delete oscBuf[i];
  }
}
