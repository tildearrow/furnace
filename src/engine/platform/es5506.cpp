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

#include "es5506.h"
#include "../engine.h"
#include "../../ta-log.h"
#include <math.h>

#define PITCH_OFFSET ((double)(16*2048*(chanMax+1)))
#define NOTE_ES5506(c,note) (parent->calcBaseFreq(chipClock,chan[c].pcm.freqOffs,note,false))

#define rWrite(a,...) {if(!skipRegisterWrites) {hostIntf32.push_back(QueuedHostIntf(4,(a),__VA_ARGS__)); }}
#define immWrite(a,...) {hostIntf32.push_back(QueuedHostIntf(4,(a),__VA_ARGS__));}
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

#define pageReadMask(p,pm,a,st,...) \
  if (!skipRegisterWrites) { \
    if ((curPage&(pm))!=((p)&(pm))) { \
      curPage=(curPage&~(pm))|((p)&(pm)); \
      rWrite(0xf,curPage,(pm)); \
    } \
    rRead(st,(a),__VA_ARGS__); \
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

void DivPlatformES5506::acquire(short** buf, size_t len) {
  for (size_t h=0; h<len; h++) {
    // convert 32 bit access to 8 bit host interface
    while (!hostIntf32.empty()) {
      QueuedHostIntf w=hostIntf32.front();
      if (w.isRead && (w.read!=NULL)) {
        hostIntf8.push(QueuedHostIntf(w.state,0,w.addr,w.read,w.mask));
        hostIntf8.push(QueuedHostIntf(w.state,1,w.addr,w.read,w.mask));
        hostIntf8.push(QueuedHostIntf(w.state,2,w.addr,w.read,w.mask));
        hostIntf8.push(QueuedHostIntf(w.state,3,w.addr,w.read,w.mask,w.delay));
      } else {
        hostIntf8.push(QueuedHostIntf(0,w.addr,w.val,w.mask));
        hostIntf8.push(QueuedHostIntf(1,w.addr,w.val,w.mask));
        hostIntf8.push(QueuedHostIntf(2,w.addr,w.val,w.mask));
        hostIntf8.push(QueuedHostIntf(3,w.addr,w.val,w.mask,w.delay));
      }
      hostIntf32.pop();
    }

    es5506.tick_perf();
    if (cycle>0) { // wait until delay
      cycle-=2;
    } else while (!hostIntf8.empty()) {
      QueuedHostIntf w=hostIntf8.front();
      unsigned char shift=24-(w.step<<3);
      if (w.isRead) {
        logE("READING?!");
        hostIntf8.pop();
      } else {
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
        if (cycle>0) break;
      }
    }
    
    for (int o=0; o<6; o++) {
      buf[(o<<1)|0][h]=es5506.lout(o);
      buf[(o<<1)|1][h]=es5506.rout(o);
    }
    for (int i=chanMax; i>=0; i--) {
      oscBuf[i]->data[oscBuf[i]->needle++]=(es5506.voice_lout(i)+es5506.voice_rout(i))>>5;
    }
  }
}

void DivPlatformES5506::e_pin(bool state) {
}

void DivPlatformES5506::irqb(bool state) {
  /*
  rRead(0x0e,0x80,&irqv,0x9f);
  irqTrigger=true;
  */
}

void DivPlatformES5506::tick(bool sysTick) {
  for (int i=0; i<=chanMax; i++) {
    chan[i].std.next();
    DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_ES5506);
    signed int k1=chan[i].k1Prev,k2=chan[i].k2Prev;
    // volume/panning macros
    if (chan[i].std.vol.had) {
      const int nextVol=VOL_SCALE_LOG((0xfff*chan[i].vol)/0xff,(0xfff*chan[i].std.vol.val)/chan[i].volMacroMax,0xfff);
      if (chan[i].outVol!=nextVol) {
        chan[i].outVol=nextVol;
        chan[i].volChanged.lVol=1;
        chan[i].volChanged.rVol=1;
      }
    }
    if (chan[i].std.panL.had) {
      const int nextLVol=VOL_SCALE_LOG((0xfff*chan[i].lVol)/0xff,(0xfff*chan[i].std.panL.val)/chan[i].panMacroMax,0xfff);
      if (chan[i].outLVol!=nextLVol) {
        chan[i].outLVol=nextLVol;
        chan[i].volChanged.lVol=1;
      }
    }
    if (chan[i].std.panR.had) {
      const int nextRVol=VOL_SCALE_LOG((0xfff*chan[i].rVol)/0xff,(0xfff*chan[i].std.panR.val)/chan[i].panMacroMax,0xfff);
      if (chan[i].outRVol!=nextRVol) {
        chan[i].outRVol=nextRVol;
        chan[i].volChanged.rVol=1;
      }
    }
    // arpeggio/pitch macros, frequency related
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].nextNote=parent->calcArp(chan[i].note,chan[i].std.arp.val);
      }
      chan[i].noteChanged.note=1;
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
        case 0: // absolute
          if (chan[i].filter.k1!=(chan[i].std.ex1.val&0xffff)) {
            chan[i].filter.k1=chan[i].std.ex1.val&0xffff;
            chan[i].filterChanged.k1=1;
          }
          break;
        case 1: // relative
          if (chan[i].k1Offs!=chan[i].std.ex1.val) {
            chan[i].k1Offs=chan[i].std.ex1.val;
            chan[i].filterChanged.k1=1;
          }
          break;
        default:
          break;
      }
    }
    if (chan[i].std.ex2.had) {
      switch (chan[i].std.ex2.mode) {
        case 0: // absolute
          if (chan[i].filter.k2!=(chan[i].std.ex2.val&0xffff)) {
            chan[i].filter.k2=chan[i].std.ex2.val&0xffff;
            chan[i].filterChanged.k2=1;
          }
          break;
        case 1: // relative
          if (chan[i].k2Offs!=chan[i].std.ex1.val) {
            chan[i].k2Offs=chan[i].std.ex1.val;
            chan[i].filterChanged.k2=1;
          }
          break;
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
        signed int next=CLAMP(chan[i].filter.k1+chan[i].k1Slide,0,65535);
        if (chan[i].filter.k1!=next) {
          chan[i].filter.k1=next;
          chan[i].filterChanged.k1=1;
        }
      }
      if (chan[i].k2Slide!=0 && chan[i].filter.k2>0 && chan[i].filter.k2<65535) {
        signed int next=CLAMP(chan[i].filter.k2+chan[i].k2Slide,0,65535);
        if (chan[i].filter.k2!=next) {
          chan[i].filter.k2=next;
          chan[i].filterChanged.k2=1;
        }
      }
    }
    // channel assignment
    if (chan[i].active && chan[i].std.fb.had) {
      const unsigned char ca=CLAMP(chan[i].std.fb.val,0,5);
      if (chan[i].ca!=ca) {
        chan[i].ca=ca;
        if (!chan[i].keyOn) {
          chan[i].volChanged.ca=1;
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
      if (chan[i].pcm.direction!=(bool)(chan[i].std.alg.val&2)) {
        chan[i].pcm.direction=chan[i].std.alg.val&2;
        if (!chan[i].keyOn) {
          pageWriteMask(0x00|i,0x5f,0x00,chan[i].pcm.direction?0x0040:0x0000,0x0040);
        }
      }
    }
    // update registers
    if (chan[i].volChanged.changed) {
      // calculate volume (16 bit)
      if (chan[i].volChanged.lVol) {
        chan[i].resLVol=VOL_SCALE_LOG(chan[i].outVol,chan[i].outLVol,0xfff);
        chan[i].resLVol-=volScale;
        if (chan[i].resLVol<0) chan[i].resLVol=0;
        chan[i].resLVol<<=4;
        if (!chan[i].keyOn && chan[i].active) {
          pageWrite(0x00|i,0x02,chan[i].resLVol);
        }
      }
      if (chan[i].volChanged.rVol) {
        chan[i].resRVol=VOL_SCALE_LOG(chan[i].outVol,chan[i].outRVol,0xfff);
        chan[i].resRVol-=volScale;
        if (chan[i].resRVol<0) chan[i].resRVol=0;
        chan[i].resRVol<<=4;
        if (!chan[i].keyOn && chan[i].active) {
          pageWrite(0x00|i,0x04,chan[i].resRVol);
        }
      }
      if (chan[i].volChanged.ca) {
        pageWriteMask(0x00|i,0x5f,0x00,(chan[i].ca<<10),0x1c00);
      }
      chan[i].volChanged.changed=0;
    }
    if (chan[i].pcmChanged.changed) {
      if (chan[i].pcmChanged.index) {
        const int next=chan[i].pcm.next;
        bool sampleValid=false;
        if (((ins->amiga.useNoteMap) && (next>=0 && next<120)) ||
            ((!ins->amiga.useNoteMap) && (next>=0 && next<parent->song.sampleLen))) {
          DivInstrumentAmiga::SampleMap& noteMapind=ins->amiga.noteMap[next];
          int sample=next;
          if (ins->amiga.useNoteMap) {
            sample=noteMapind.map;
          }
          if (sample>=0 && sample<parent->song.sampleLen) {
            const unsigned int offES5506=sampleOffES5506[sample];
            sampleValid=true;
            chan[i].pcm.index=sample;
            chan[i].pcm.isNoteMap=ins->amiga.useNoteMap;
            DivSample* s=parent->getSample(sample);
            // get frequency offset
            double off=1.0;
            double center=(double)s->centerRate;
            if (center<1) {
              off=1.0;
            } else {
              off=(double)center/8363.0;
            }
            if (ins->amiga.useNoteMap) {
              //chan[i].pcm.note=next;
            }
            // get loop mode
            DivSampleLoopMode loopMode=s->isLoopable()?s->loopMode:DIV_SAMPLE_LOOP_MAX;
            const unsigned int start=offES5506<<10;
            const unsigned int length=s->samples-1;
            const unsigned int end=start+(length<<11);
            const unsigned int nextBank=(offES5506>>22)&3;
            const double nextFreqOffs=PITCH_OFFSET*off;
            chan[i].pcm.loopMode=loopMode;
            chan[i].pcm.bank=nextBank;
            chan[i].pcm.start=start;
            chan[i].pcm.end=end;
            chan[i].pcm.length=length;
            if ((chan[i].pcm.loopMode!=loopMode) || (chan[i].pcm.bank!=nextBank)) {
              chan[i].pcm.loopMode=loopMode;
              chan[i].pcm.bank=nextBank;
              chan[i].pcmChanged.loopBank=1;
            }
            if (chan[i].pcm.nextFreqOffs!=nextFreqOffs) {
              chan[i].pcm.nextFreqOffs=nextFreqOffs;
              chan[i].noteChanged.offs=1;
            }
          }
        }
        if (sampleValid) {
          if (!chan[i].keyOn) {
            pageWrite(0x20|i,0x03,(chan[i].pcm.direction)?chan[i].pcm.end:chan[i].pcm.start);
          }
          chan[i].pcmChanged.slice=1;
        }
        chan[i].pcmChanged.index=0;
      }
      if (chan[i].pcmChanged.slice) {
        if (!chan[i].keyOn) {
          if (chan[i].pcm.index>=0 && chan[i].pcm.index<parent->song.sampleLen) {
            // get loop mode
            DivSample* s=parent->getSample(chan[i].pcm.index);
            const unsigned int start=sampleOffES5506[chan[i].pcm.index]<<10;
            const unsigned int nextLoopStart=(start+(s->loopStart<<11))&0xfffff800;
            const unsigned int nextLoopEnd=(start+((s->loopEnd-1)<<11))&0xffffff80;
            if ((chan[i].pcm.loopStart!=nextLoopStart) || (chan[i].pcm.loopEnd!=nextLoopEnd)) {
              chan[i].pcm.loopStart=nextLoopStart;
              chan[i].pcm.loopEnd=nextLoopEnd;
              chan[i].pcmChanged.position=1;
            }
          }
        }
        chan[i].pcmChanged.slice=0;
      }
      if (chan[i].pcmChanged.position) {
        if (!chan[i].keyOn) {
          pageWrite(0x20|i,0x01,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOP_MAX)?chan[i].pcm.start:chan[i].pcm.loopStart);
          pageWrite(0x20|i,0x02,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOP_MAX)?chan[i].pcm.end:chan[i].pcm.loopEnd);
        }
        chan[i].pcmChanged.position=0;
      }
      if (chan[i].pcmChanged.loopBank) {
        if (!chan[i].keyOn) {
          unsigned int loopFlag=(chan[i].pcm.bank<<14)|(chan[i].pcm.direction?0x0040:0x0000);
          chan[i].isReverseLoop=false;
          switch (chan[i].pcm.loopMode) {
            case DIV_SAMPLE_LOOP_FORWARD: // Forward loop
              loopFlag|=0x0008;
              break;
            /*
            case DIV_SAMPLE_LOOP_BACKWARD: // Backward loop: IRQ enable
              loopFlag|=0x0038;
              chan[i].isReverseLoop=true;
              break;
            */
            case DIV_SAMPLE_LOOP_PINGPONG: // Pingpong loop: Hardware support
              loopFlag|=0x0018;
              break;
            case DIV_SAMPLE_LOOP_MAX: // no loop
            default:
              break;
          }
          // Set loop mode & Bank
          pageWriteMask(0x00|i,0x5f,0x00,loopFlag,0xe0fd);
        }
        chan[i].pcmChanged.loopBank=0;
      }
      chan[i].pcmChanged.dummy=0;
    }
    if (chan[i].filterChanged.changed) {
      if (!chan[i].keyOn) {
        if (chan[i].filterChanged.mode) {
          pageWriteMask(0x00|i,0x5f,0x00,(chan[i].filter.mode<<8),0x0300);
        }
        if (chan[i].filterChanged.k2) {
          if (chan[i].std.ex2.mode!=0) { // Relative
            k2=CLAMP(chan[i].filter.k2+chan[i].k2Offs,0,65535);
          } else {
            k2=chan[i].filter.k2;
          }
        }
        if (chan[i].filterChanged.k1) {
          if (chan[i].std.ex1.mode!=0) { // Relative
            k1=CLAMP(chan[i].filter.k1+chan[i].k1Offs,0,65535);
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
          chan[i].nextFreq=NOTE_ES5506(i,chan[i].currNote);
          chan[i].noteChanged.freq=1;
          chan[i].freqChanged=true;
        }
      }
      if (chan[i].noteChanged.note) {
        if (chan[i].currNote!=chan[i].nextNote) {
          chan[i].currNote=chan[i].nextNote;
          const int nextFreq=NOTE_ES5506(i,chan[i].nextNote);
          if (chan[i].nextFreq!=nextFreq) {
            chan[i].nextFreq=nextFreq;
            chan[i].noteChanged.freq=1;
          }
        }
      }
      if (chan[i].noteChanged.freq) {
        if (chan[i].baseFreq!=chan[i].nextFreq) {
          chan[i].baseFreq=chan[i].nextFreq;
          chan[i].freqChanged=true;
        }
      }
      chan[i].noteChanged.changed=0;
    }
    if (chan[i].pcm.setPos) {
      if (chan[i].active) {
        const unsigned int start=chan[i].pcm.start;
        const unsigned int end=chan[i].pcm.length;
        const unsigned int pos=chan[i].pcm.direction?(end-chan[i].pcm.nextPos):chan[i].pcm.nextPos;
        if ((chan[i].pcm.direction && pos>0) || ((!chan[i].pcm.direction) && pos<end)) {
          pageWrite(0x20|i,0x03,start+(pos<<11));
        }
      } else {
        // force keyon
        chan[i].keyOn=true;
      }
      chan[i].pcm.setPos=false;
    } else {
      chan[i].pcm.nextPos=0;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=CLAMP(parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,2,chan[i].pitch2,chipClock,chan[i].pcm.freqOffs),0,0x1ffff);
      if (chan[i].keyOn) {
        if (chan[i].pcm.index>=0 && chan[i].pcm.index<parent->song.sampleLen) {
          const int ind=chan[i].pcm.index;
          DivSample* s=parent->getSample(ind);
          // get frequency offset
          double off=1.0;
          double center=(double)s->centerRate;
          if (center<1) {
            off=1.0;
          } else {
            off=(double)center/8363.0;
          }
          chan[i].pcm.loopStart=(chan[i].pcm.start+(s->loopStart<<11))&0xfffff800;
          chan[i].pcm.loopEnd=(chan[i].pcm.start+((s->loopEnd-1)<<11))&0xffffff80;
          chan[i].pcm.freqOffs=PITCH_OFFSET*off;
          unsigned int startPos=chan[i].pcm.direction?chan[i].pcm.end:chan[i].pcm.start;
          if (chan[i].pcm.nextPos) {
            const unsigned int start=chan[i].pcm.start;
            const unsigned int end=chan[i].pcm.length;
            startPos=start+((chan[i].pcm.direction?(end-chan[i].pcm.nextPos):(chan[i].pcm.nextPos))<<11);
            chan[i].pcm.nextPos=0;
          }
          chan[i].k1Prev=0xffff;
          chan[i].k2Prev=0xffff;
          pageWriteMask(0x00|i,0x5f,0x00,0x0303); // Wipeout CR
          pageWrite(0x00|i,0x06,0); // Clear ECOUNT
          pageWrite(0x20|i,0x03,startPos); // Set ACCUM to start address
          pageWrite(0x00|i,0x07,0xffff); // Set K1 and K2 to 0xffff
          pageWrite(0x00|i,0x09,0xffff,~0,(chanMax+1)*4*2); // needs to 4 sample period delay
          pageWrite(0x00|i,0x01,chan[i].freq);
          pageWrite(0x20|i,0x01,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOP_MAX)?chan[i].pcm.start:chan[i].pcm.loopStart);
          pageWrite(0x20|i,0x02,(chan[i].pcm.loopMode==DIV_SAMPLE_LOOP_MAX)?chan[i].pcm.end:chan[i].pcm.loopEnd);
          // initialize overwrite
          if (chan[i].overwrite.state.overwrited) {
            // Filter
            if (chan[i].overwrite.state.mode) {
              chan[i].filter.mode=chan[i].overwrite.filter.mode;
            }
            if (chan[i].overwrite.state.k1) {
              chan[i].filter.k1=chan[i].overwrite.filter.k1;
            }
            if (chan[i].overwrite.state.k2) {
              chan[i].filter.k2=chan[i].overwrite.filter.k2;
            }
            // Envelope
            if (chan[i].overwrite.state.ecount) {
              chan[i].envelope.ecount=chan[i].overwrite.envelope.ecount;
            }
            if (chan[i].overwrite.state.lVRamp) {
              chan[i].envelope.lVRamp=chan[i].overwrite.envelope.lVRamp;
            }
            if (chan[i].overwrite.state.rVRamp) {
              chan[i].envelope.rVRamp=chan[i].overwrite.envelope.rVRamp;
            }
            if (chan[i].overwrite.state.k1Ramp) {
              chan[i].envelope.k1Ramp=chan[i].overwrite.envelope.k1Ramp;
              chan[i].envelope.k1Slow=chan[i].overwrite.envelope.k1Slow;
            }
            if (chan[i].overwrite.state.k2Ramp) {
              chan[i].envelope.k2Ramp=chan[i].overwrite.envelope.k2Ramp;
              chan[i].envelope.k2Slow=chan[i].overwrite.envelope.k2Slow;
            }
            chan[i].overwrite.state.overwrited=0;
          }
          // initialize envelope
          pageWrite(0x00|i,0x03,(unsigned char)(chan[i].envelope.lVRamp)<<8);
          pageWrite(0x00|i,0x05,(unsigned char)(chan[i].envelope.rVRamp)<<8);
          pageWrite(0x00|i,0x0a,((unsigned char)(chan[i].envelope.k1Ramp)<<8)|(chan[i].envelope.k1Slow?1:0));
          pageWrite(0x00|i,0x08,((unsigned char)(chan[i].envelope.k2Ramp)<<8)|(chan[i].envelope.k2Slow?1:0));
          // initialize filter
          pageWriteMask(0x00|i,0x5f,0x00,(chan[i].pcm.bank<<14)|(chan[i].filter.mode<<8),0xc300);
          if ((chan[i].std.ex2.mode!=0) && (chan[i].std.ex2.had)) {
            k2=CLAMP(chan[i].filter.k2+chan[i].k2Offs,0,65535);
          } else {
            k2=chan[i].filter.k2;
          }
          pageWrite(0x00|i,0x07,k2);
          chan[i].k2Prev=k2;
          if ((chan[i].std.ex1.mode!=0) && (chan[i].std.ex1.had)) {
            k1=CLAMP(chan[i].filter.k1+chan[i].k1Offs,0,65535);
          } else {
            k1=chan[i].filter.k1;
          }
          pageWrite(0x00|i,0x09,k1);
          chan[i].k1Prev=k1;
          pageWrite(0x00|i,0x02,chan[i].resLVol);
          pageWrite(0x00|i,0x04,chan[i].resRVol);
          unsigned int loopFlag=(chan[i].ca<<10)|(chan[i].pcm.direction?0x0040:0x0000);
          chan[i].isReverseLoop=false;
          switch (chan[i].pcm.loopMode) {
            case DIV_SAMPLE_LOOP_FORWARD: // Forward loop
              loopFlag|=0x0008;
              break;
            /*
            case DIV_SAMPLE_LOOP_BACKWARD: // Backward loop: IRQ enable
              loopFlag|=0x0038;
              chan[i].isReverseLoop=true;
              break;
            */
            case DIV_SAMPLE_LOOP_PINGPONG: // Pingpong loop: Hardware support
              loopFlag|=0x0018;
              break;
            case DIV_SAMPLE_LOOP_MAX: // no loop
            default:
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
      } else if (!chan[i].keyOn && chan[i].active) {
        pageWrite(0x00|i,0x01,chan[i].freq);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
    if (!chan[i].keyOn && chan[i].active) {
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

// man this code
// part of the reason why it's so messy is because the chip is
// overly complex and because when this pull request was made,
// Furnace still was in an early state with no support for sample
// maps or whatever...
// one day I'll come back and clean this up
int DivPlatformES5506::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_ES5506);
      bool sampleValid=false;
      if (c.value!=DIV_NOTE_NULL) {
        int sample=ins->amiga.getSample(c.value);
        chan[c.chan].sampleNote=c.value;
        if (sample>=0 && sample<parent->song.sampleLen) {
          sampleValid=true;
          chan[c.chan].volMacroMax=ins->type==DIV_INS_AMIGA?64:0xfff;
          chan[c.chan].panMacroMax=ins->type==DIV_INS_AMIGA?127:0xfff;
          chan[c.chan].pcm.next=ins->amiga.useNoteMap?c.value:sample;
          c.value=ins->amiga.getFreq(c.value);
          chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
          chan[c.chan].pcm.note=c.value;
          chan[c.chan].filter=ins->es5506.filter;
          chan[c.chan].envelope=ins->es5506.envelope;
        } else {
          chan[c.chan].sampleNoteDelta=0;
        }
      } else {
        int sample=ins->amiga.getSample(chan[c.chan].sampleNote);
        if (sample>=0 && sample<parent->song.sampleLen) {
          sampleValid=true;
          chan[c.chan].volMacroMax=ins->type==DIV_INS_AMIGA?64:0xfff;
          chan[c.chan].panMacroMax=ins->type==DIV_INS_AMIGA?127:0xfff;
          chan[c.chan].pcm.next=ins->amiga.useNoteMap?chan[c.chan].sampleNote:sample;
          c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
          chan[c.chan].pcm.note=c.value;
          chan[c.chan].filter=ins->es5506.filter;
          chan[c.chan].envelope=ins->es5506.envelope;
        } else {
          chan[c.chan].sampleNoteDelta=0;
        }
      }
      if (!sampleValid) {
        chan[c.chan].pcm.index=chan[c.chan].pcm.next=-1;
        chan[c.chan].sampleNoteDelta=0;
        chan[c.chan].filter=DivInstrumentES5506::Filter();
        chan[c.chan].envelope=DivInstrumentES5506::Envelope();
      }
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].note=c.value;
        chan[c.chan].nextNote=chan[c.chan].note;
        chan[c.chan].pcm.nextFreqOffs=chan[c.chan].pcm.freqOffs;
        chan[c.chan].freqChanged=true;
        chan[c.chan].pcmChanged.changed=0xff;
        chan[c.chan].noteChanged.changed=0xff;
        chan[c.chan].volChanged.changed=0xff;
      }
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=(0xfff*chan[c.chan].vol)/0xff;
      }
      if (!chan[c.chan].std.panL.will) {
        chan[c.chan].outLVol=(0xfff*chan[c.chan].lVol)/0xff;
      }
      if (!chan[c.chan].std.panR.will) {
        chan[c.chan].outRVol=(0xfff*chan[c.chan].rVol)/0xff;
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
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=(0xfff*c.value)/0xff;
          chan[c.chan].volChanged.changed=0xff;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PANNING: {
      if (chan[c.chan].ca!=0) {
        chan[c.chan].ca=0;
        chan[c.chan].volChanged.ca=1;
      }
      // Left volume
      if (chan[c.chan].lVol!=c.value) {
        chan[c.chan].lVol=c.value;
        if (!chan[c.chan].std.panL.has) {
          chan[c.chan].outLVol=(0xfff*c.value)/0xff;
          chan[c.chan].volChanged.lVol=1;
        }
      }
      // Right volume
      if (chan[c.chan].rVol!=c.value2) {
        chan[c.chan].rVol=c.value2;
        if (!chan[c.chan].std.panR.has) {
          chan[c.chan].outRVol=(0xfff*c.value2)/0xff;
          chan[c.chan].volChanged.rVol=1;
        }
      }
      break;
    }
    case DIV_CMD_SURROUND_PANNING: {
      unsigned char ca=CLAMP(c.value>>1,0,5);
      if (chan[c.chan].ca!=ca) {
        chan[c.chan].ca=ca;
        chan[c.chan].volChanged.ca=1;
      }
      if ((c.value&1)==0) {
        // Left volume
        if (chan[c.chan].lVol!=c.value2) {
          chan[c.chan].lVol=c.value2;
          if (!chan[c.chan].std.panL.has) {
            chan[c.chan].outLVol=(0xfff*c.value2)/0xff;
            chan[c.chan].volChanged.lVol=1;
          }
        }
      }
      else if ((c.value&1)==1) {
        // Right volume
        if (chan[c.chan].rVol!=c.value2) {
          chan[c.chan].rVol=c.value2;
          if (!chan[c.chan].std.panR.has) {
            chan[c.chan].outRVol=(0xfff*c.value2)/0xff;
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
    // Filter commands
    case DIV_CMD_ES5506_FILTER_MODE:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.mode) {
          chan[c.chan].overwrite.filter.mode=chan[c.chan].filter.mode;
          chan[c.chan].overwrite.state.mode=1;
        }
        chan[c.chan].overwrite.filter.mode=DivInstrumentES5506::Filter::FilterMode(c.value&3);
      }
      chan[c.chan].filter.mode=DivInstrumentES5506::Filter::FilterMode(c.value&3);
      chan[c.chan].filterChanged.mode=1;
      break;
    case DIV_CMD_ES5506_FILTER_K1:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.k1) {
          chan[c.chan].overwrite.filter.k1=chan[c.chan].filter.k1;
          chan[c.chan].overwrite.state.k1=1;
        }
        chan[c.chan].overwrite.filter.k1=(chan[c.chan].overwrite.filter.k1&~c.value2)|(c.value&c.value2);
      }
      chan[c.chan].filter.k1=(chan[c.chan].filter.k1&~c.value2)|(c.value&c.value2);
      chan[c.chan].filterChanged.k1=1;
      break;
    case DIV_CMD_ES5506_FILTER_K2:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.k2) {
          chan[c.chan].overwrite.filter.k2=chan[c.chan].filter.k2;
          chan[c.chan].overwrite.state.k2=1;
        }
        chan[c.chan].overwrite.filter.k2=(chan[c.chan].overwrite.filter.k2&~c.value2)|(c.value&c.value2);
      }
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
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.ecount) {
          chan[c.chan].overwrite.envelope.ecount=chan[c.chan].envelope.ecount;
          chan[c.chan].overwrite.state.ecount=1;
        }
        chan[c.chan].overwrite.envelope.ecount=c.value&0x1ff;
      }
      chan[c.chan].envelope.ecount=c.value&0x1ff;
      chan[c.chan].envChanged.ecount=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_LVRAMP:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.lVRamp) {
          chan[c.chan].overwrite.envelope.lVRamp=chan[c.chan].envelope.lVRamp;
          chan[c.chan].overwrite.state.lVRamp=1;
        }
        chan[c.chan].overwrite.envelope.lVRamp=(signed char)(c.value&0xff);
      }
      chan[c.chan].envelope.lVRamp=(signed char)(c.value&0xff);
      chan[c.chan].envChanged.lVRamp=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_RVRAMP:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.rVRamp) {
          chan[c.chan].overwrite.envelope.rVRamp=chan[c.chan].envelope.rVRamp;
          chan[c.chan].overwrite.state.rVRamp=1;
        }
        chan[c.chan].overwrite.envelope.rVRamp=(signed char)(c.value&0xff);
      }
      chan[c.chan].envelope.rVRamp=(signed char)(c.value&0xff);
      chan[c.chan].envChanged.rVRamp=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_K1RAMP:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.k1Ramp) {
          chan[c.chan].overwrite.envelope.k1Ramp=chan[c.chan].envelope.k1Ramp;
          chan[c.chan].overwrite.envelope.k1Slow=chan[c.chan].envelope.k1Slow;
          chan[c.chan].overwrite.state.k1Ramp=1;
        }
        chan[c.chan].overwrite.envelope.k1Ramp=(signed char)(c.value&0xff);
        chan[c.chan].overwrite.envelope.k1Slow=c.value2&1;
      }
      chan[c.chan].envelope.k1Ramp=(signed char)(c.value&0xff);
      chan[c.chan].envelope.k1Slow=c.value2&1;
      chan[c.chan].envChanged.k1Ramp=1;
      break;
    case DIV_CMD_ES5506_ENVELOPE_K2RAMP:
      if (!chan[c.chan].active) {
        if (!chan[c.chan].overwrite.state.k2Ramp) {
          chan[c.chan].overwrite.envelope.k2Ramp=chan[c.chan].envelope.k2Ramp;
          chan[c.chan].overwrite.envelope.k2Slow=chan[c.chan].envelope.k2Slow;
          chan[c.chan].overwrite.state.k2Ramp=1;
        }
        chan[c.chan].overwrite.envelope.k2Ramp=(signed char)(c.value&0xff);
        chan[c.chan].overwrite.envelope.k2Slow=c.value2&1;
      }
      chan[c.chan].envelope.k2Ramp=(signed char)(c.value&0xff);
      chan[c.chan].envelope.k2Slow=c.value2&1;
      chan[c.chan].envChanged.k2Ramp=1;
      break;
    // controls
    case DIV_CMD_ES5506_PAUSE:
      if (chan[c.chan].active) {
        if (chan[c.chan].pcm.pause!=(bool)(c.value&1)) {
          chan[c.chan].pcm.pause=c.value&1;
          pageWriteMask(0x00|c.chan,0x5f,0x00,chan[c.chan].pcm.pause?0x0002:0x0000,0x0002);
        }
      }
      break;
    case DIV_CMD_NOTE_PORTA: {
      int nextFreq=chan[c.chan].baseFreq;
      const int destFreq=NOTE_ES5506(c.chan,c.value2+chan[c.chan].sampleNoteDelta);
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
      chan[c.chan].nextNote=chan[c.chan].note+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val-12):(0));
      chan[c.chan].noteChanged.note=1;
      break;
    }
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_ES5506));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) {
        chan[c.chan].nextNote=chan[c.chan].note;
        chan[c.chan].noteChanged.note=1;
      }
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_SAMPLE_POS: {
      chan[c.chan].pcm.nextPos=c.value;
      chan[c.chan].pcm.setPos=true;
      break;
    }
    case DIV_CMD_SAMPLE_DIR: {
      if (chan[c.chan].pcm.direction!=(bool)(c.value&1)) {
        chan[c.chan].pcm.direction=c.value&1;
        pageWriteMask(0x00|c.chan,0x5f,0x00,chan[c.chan].pcm.direction?0x0040:0x0000,0x0040);
      }
      break;
    }
    case DIV_CMD_GET_VOLMAX:
      return 255;
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
    chan[i].pcmChanged.changed=0xff;
  }
}

void* DivPlatformES5506::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformES5506::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformES5506::getPan(int ch) {
  float expL=255.0f*pow(((float)(chan[ch].resLVol>>4))/4095.0f,4.0f);
  float expR=255.0f*pow(((float)(chan[ch].resRVol>>4))/4095.0f,4.0f);
  if (expL>255.0f) expL=255.0f;
  if (expR>255.0f) expR=255.0f;
  return (((unsigned int)expL)<<8)|((unsigned int)expR);
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
  irqTrigger=false;
  chanMax=initChanMax;

  pageWriteMask(0x00,0x60,0x0b,chanMax);
  pageWriteMask(0x00,0x60,0x0b,0x1f);
  // set serial output to I2S-ish, 16 bit
  pageWriteMask(0x20,0x60,0x0a,0x01);
  pageWriteMask(0x20,0x60,0x0b,0x11);
  pageWriteMask(0x20,0x60,0x0c,0x20);
  pageWriteMask(0x00,0x60,0x0c,0x08); // Reset serial output
}

int DivPlatformES5506::getOutputCount() {
  return 12;
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
}

void DivPlatformES5506::notifyInsDeletion(void* ins) {
  for (int i=0; i<32; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformES5506::setFlags(const DivConfig& flags) {
  chipClock=16000000;
  CHECK_CUSTOM_CLOCK;

  initChanMax=MAX(4,flags.getInt("channels",0x1f)&0x1f);
  volScale=4095-flags.getInt("volScale",4095);
  chanMax=initChanMax;
  pageWriteMask(0x00,0x60,0x0b,chanMax);

  rate=chipClock/(16*(initChanMax+1)); // 2 E clock tick (16 CLKIN tick) per voice / 4
  for (int i=0; i<32; i++) {
    oscBuf[i]->rate=rate;
  }
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

bool DivPlatformES5506::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>255) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformES5506::getMemCompo(int index) {
  if (index!=0) return NULL;
  return &memCompo;
}

void DivPlatformES5506::renderSamples(int sysID) {
  memset(sampleMem,0,getSampleMemCapacity());
  memset(sampleOffES5506,0,256*sizeof(unsigned int));
  memset(sampleLoaded,0,256*sizeof(bool));

  memCompo=DivMemoryComposition();
  memCompo.name="Sample Memory";

  size_t memPos=128; // add silent at begin and end of each bank for reverse playback
  for (int i=0; i<parent->song.sampleLen; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOffES5506[i]=0;
      continue;
    }

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
    sampleOffES5506[i]=memPos;
    sampleLoaded[i]=true;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+length));
    memPos+=length;
  }
  sampleMemLen=memPos+256;

  memCompo.used=sampleMemLen;
  memCompo.capacity=16777216;
}

int DivPlatformES5506::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  sampleMem=new signed short[getSampleMemCapacity()/sizeof(short)];
  sampleMemLen=0;
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  volScale=0;
  curPage=0;

  for (int i=0; i<32; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
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
