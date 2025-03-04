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

#include "pce.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>

//#define rWrite(a,v) pendingWrites[a]=v;
#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }
#define chWrite(c,a,v) \
  if (!skipRegisterWrites) { \
    if (curChan!=c) { \
      curChan=c; \
      rWrite(0,curChan); \
    } \
    regPool[16+((c)<<4)+((a)&0x0f)]=v; \
    rWrite(a,v); \
  }

#define CHIP_DIVIDER 64

const char* regCheatSheetPCE[]={
  "Select", "0",
  "MasterVol", "1",
  "FreqL", "2",
  "FreqH", "3",
  "DataCtl", "4",
  "ChanVol", "5",
  "WaveCtl", "6",
  "NoiseCtl", "7",
  "LFOFreq", "8",
  "LFOCtl", "9",
  NULL
};

const char** DivPlatformPCE::getRegisterSheet() {
  return regCheatSheetPCE;
}

void DivPlatformPCE::acquire(short** buf, size_t len) {
}

void DivPlatformPCE::acquireDirect(blip_buffer_t** bb, size_t off, size_t len) {
  for (int i=0; i<6; i++) {
    oscBuf[i]->begin(len);
    pce->channel[i].oscBuf=oscBuf[i];
  }

  pce->bb[0]=bb[0];
  pce->bb[1]=bb[1];

  size_t pos=off;
  pce->ResetTS(pos);

  while (!writes.empty()) {
    QueuedWrite w=writes.front();
    pce->Write(pos,w.addr,w.val);
    regPool[w.addr&0x0f]=w.val;
    writes.pop();
  }

  for (size_t h=0; h<len;) {
    int advance=len-h;
    // heuristic
    int remainTime=9;
    for (int i=0; i<6; i++) {
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        if (chan[i].dacRate<=0) continue;
        remainTime=(rate-chan[i].dacPeriod+chan[i].dacRate-1)/chan[i].dacRate;
        if (remainTime<advance) advance=remainTime;
        if (remainTime<1) advance=1;
      }
    }

    // PCM part
    for (int i=0; i<6; i++) {
      if (chan[i].pcm && chan[i].dacSample!=-1) {
        chan[i].dacPeriod+=chan[i].dacRate*advance;
        if (chan[i].dacPeriod>rate) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->samples<=0 || chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
            continue;
          }
          chWrite(i,0x07,0);
          signed char dacData=((signed char)((unsigned char)s->data8[chan[i].dacPos]^0x80))>>3;
          chan[i].dacOut=CLAMP(dacData,-16,15);
          if (!isMuted[i]) {
            chWrite(i,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[i].outVol));
            chWrite(i,0x06,chan[i].dacOut&0x1f);
          } else {
            chWrite(i,0x04,0xc0);
            chWrite(i,0x06,0x10);
          }
          chan[i].dacPos++;
          if (s->isLoopable() && chan[i].dacPos>=(unsigned int)s->loopEnd) {
            chan[i].dacPos=s->loopStart;
          } else if (chan[i].dacPos>=s->samples) {
            chan[i].dacSample=-1;
          }
          chan[i].dacPeriod-=rate;
        }
      }
    }
  
    // PCE part
    // WHAT?????????
    pos+=advance;

    while (!writes.empty()) {
      QueuedWrite w=writes.front();
      pce->Write(pos,w.addr,w.val);
      regPool[w.addr&0x0f]=w.val;
      writes.pop();
    }

    h+=advance;
  }
  pce->Update(pos);

  for (int i=0; i<6; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformPCE::updateWave(int ch) {
  if (chan[ch].pcm) {
    chan[ch].deferredWaveUpdate=true;
    return;
  }
  chWrite(ch,0x04,0x5f);
  chWrite(ch,0x04,0x1f);
  for (int i=0; i<32; i++) {
    chWrite(ch,0x06,chan[ch].ws.output[(i+chan[ch].antiClickWavePos)&31]);
  }
  chan[ch].antiClickWavePos&=31;
  if (chan[ch].active) {
    chWrite(ch,0x04,0x80|chan[ch].outVol);
  }
  if (chan[ch].deferredWaveUpdate) {
    chan[ch].deferredWaveUpdate=false;
  }
}

// TODO: in octave 6 the noise table changes to a tonal one
static unsigned char noiseFreq[12]={
  4,13,15,18,21,23,25,27,29,31,0,2  
};

void DivPlatformPCE::tick(bool sysTick) {
  for (int i=0; i<6; i++) {
    // anti-click
    if (antiClickEnabled && sysTick && chan[i].freq>0) {
      chan[i].antiClickPeriodCount+=(chipClock/MAX(parent->getCurHz(),1.0f));
      chan[i].antiClickWavePos+=chan[i].antiClickPeriodCount/chan[i].freq;
      chan[i].antiClickPeriodCount%=chan[i].freq;
    }

    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG_BROKEN(chan[i].vol&31,MIN(31,chan[i].std.vol.val),31);
      if (chan[i].pcm) {
        chWrite(i,0x04,0xc0|chan[i].outVol);
      } else {
        chWrite(i,0x04,0x80|chan[i].outVol);
      }
    }
    if (chan[i].std.duty.had && i>=4) {
      chan[i].noise=chan[i].std.duty.val;
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        int noiseSeek=parent->calcArp(chan[i].note,chan[i].std.arp.val);
        chan[i].baseFreq=NOTE_PERIODIC(noiseSeek);
        if (noiseSeek<0) noiseSeek=0;
        chan[i].noiseSeek=noiseSeek;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.wave.had && !chan[i].pcm) {
      if (chan[i].wave!=chan[i].std.wave.val || chan[i].ws.activeChanged()) {
        chan[i].wave=chan[i].std.wave.val;
        chan[i].ws.changeWave1(chan[i].wave);
        if (!chan[i].keyOff) chan[i].keyOn=true;
      }
    }
    if (chan[i].std.panL.had) {
      chan[i].pan&=0x0f;
      chan[i].pan|=(chan[i].std.panL.val&15)<<4;
    }
    if (chan[i].std.panR.had) {
      chan[i].pan&=0xf0;
      chan[i].pan|=chan[i].std.panR.val&15;
    }
    if (chan[i].std.panL.had || chan[i].std.panR.had) {
      chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
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
    if (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) {
      if (chan[i].furnaceDac && chan[i].pcm) {
        if (chan[i].active && chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          if (chan[i].setPos) {
            chan[i].setPos=false;
          } else {
            chan[i].dacPos=0;
          }
          chan[i].dacPeriod=0;
          chWrite(i,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[i].vol));
          addWrite(0xffff0000+(i<<8),chan[i].dacSample);
          chan[i].keyOn=true;
        }
      }
      chan[i].antiClickWavePos=0;
      chan[i].antiClickPeriodCount=0;
    }
    if (chan[i].active) {
      if (chan[i].ws.tick() || (chan[i].std.phaseReset.had && chan[i].std.phaseReset.val==1) || chan[i].deferredWaveUpdate) {
        updateWave(i);
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      //DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_PCE);
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER);
      if (chan[i].furnaceDac && chan[i].pcm) {
        double off=1.0;
        if (chan[i].dacSample>=0 && chan[i].dacSample<parent->song.sampleLen) {
          DivSample* s=parent->getSample(chan[i].dacSample);
          if (s->centerRate<1) {
            off=1.0;
          } else {
            off=parent->getCenterRate()/(double)s->centerRate;
          }
        }
        chan[i].dacRate=(double)chipClock/(4*MAX(1,off*chan[i].freq));
        if (dumpWrites) addWrite(0xffff0001+(i<<8),chan[i].dacRate);
      }
      if (chan[i].freq<1) chan[i].freq=1;
      if (chan[i].freq>4095) chan[i].freq=4095;
      chWrite(i,0x02,chan[i].freq&0xff);
      chWrite(i,0x03,chan[i].freq>>8);

      if (i>=4) {
        int noiseSeek=(chan[i].fixedArp?chan[i].baseNoteOverride:(chan[i].note+chan[i].arpOff))+chan[i].pitch2;
        if (!parent->song.properNoiseLayout && noiseSeek<0) noiseSeek=0;
        if (!NEW_ARP_STRAT) {
          noiseSeek=chan[i].noiseSeek;
        }
        chWrite(i,0x07,chan[i].noise?(0x80|(parent->song.properNoiseLayout?(noiseSeek&31):noiseFreq[noiseSeek%12])):0);
      }
      if (chan[i].keyOn) {
        //rWrite(16+i*5,0x80);
        //chWrite(i,0x04,0x80|chan[i].vol);
      }
      if (chan[i].keyOff) {
        chWrite(i,0x04,0);
      }
      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
  if (updateLFO) {
    rWrite(0x08,lfoSpeed);
    rWrite(0x09,lfoMode);
    updateLFO=false;
  }
}

int DivPlatformPCE::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_PCE);
      chan[c.chan].macroVolMul=ins->type==DIV_INS_AMIGA?64:31;
      if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
        chan[c.chan].pcm=true;
      } else if (chan[c.chan].furnaceDac) {
        chan[c.chan].pcm=false;
        chan[c.chan].sampleNote=DIV_NOTE_NULL;
        chan[c.chan].sampleNoteDelta=0;
        if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      }
      if (chan[c.chan].pcm) {
        if (ins->type==DIV_INS_AMIGA || ins->amiga.useSample) {
          chan[c.chan].furnaceDac=true;
          if (skipRegisterWrites) break;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].dacSample=ins->amiga.getSample(c.value);
            chan[c.chan].sampleNote=c.value;
            c.value=ins->amiga.getFreq(c.value);
            chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
          } else if (chan[c.chan].sampleNote!=DIV_NOTE_NULL) {
            chan[c.chan].dacSample=ins->amiga.getSample(chan[c.chan].sampleNote);
            c.value=ins->amiga.getFreq(chan[c.chan].sampleNote);
          }
          if (chan[c.chan].dacSample<0 || chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
             if (dumpWrites) {
               chWrite(c.chan,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[c.chan].vol));
               addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
             }
          }
          if (chan[c.chan].setPos) {
            chan[c.chan].setPos=false;
          } else {
            chan[c.chan].dacPos=0;
          }
          chan[c.chan].dacPeriod=0;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
            chan[c.chan].freqChanged=true;
            chan[c.chan].note=c.value;
          }
          chan[c.chan].active=true;
          chan[c.chan].macroInit(ins);
          if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
            chan[c.chan].outVol=chan[c.chan].vol;
          }
          //chan[c.chan].keyOn=true;
        } else {
          chan[c.chan].furnaceDac=false;
          chan[c.chan].sampleNote=DIV_NOTE_NULL;
          chan[c.chan].sampleNoteDelta=0;
          if (skipRegisterWrites) break;
          if (c.value!=DIV_NOTE_NULL) {
            chan[c.chan].note=c.value;
          }
          chan[c.chan].dacSample=12*sampleBank+chan[c.chan].note%12;
          if (chan[c.chan].dacSample>=parent->song.sampleLen) {
            chan[c.chan].dacSample=-1;
            if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
            break;
          } else {
            if (dumpWrites) addWrite(0xffff0000+(c.chan<<8),chan[c.chan].dacSample);
          }
          if (chan[c.chan].setPos) {
            chan[c.chan].setPos=false;
          } else {
            chan[c.chan].dacPos=0;
          }
          chan[c.chan].dacPeriod=0;
          chan[c.chan].dacRate=parent->getSample(chan[c.chan].dacSample)->rate;
          if (dumpWrites) {
            chWrite(c.chan,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[c.chan].vol));
            addWrite(0xffff0001+(c.chan<<8),chan[c.chan].dacRate);
          }
        }
        break;
      }
      chan[c.chan].sampleNote=DIV_NOTE_NULL;
      chan[c.chan].sampleNoteDelta=0;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
        chan[c.chan].noiseSeek=c.value;
        if (chan[c.chan].noiseSeek<0) chan[c.chan].noiseSeek=0;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chWrite(c.chan,0x04,0x80|chan[c.chan].vol);
      chan[c.chan].macroInit(ins);
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      if (chan[c.chan].wave<0) {
        chan[c.chan].wave=0;
        chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      }
      chan[c.chan].ws.init(ins,32,31,chan[c.chan].insChanged);
      chan[c.chan].insChanged=false;
      break;
    }
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].dacSample=-1;
      if (dumpWrites) addWrite(0xffff0002+(c.chan<<8),0);
      chan[c.chan].pcm=false;
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
        chan[c.chan].insChanged=true;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
          if (chan[c.chan].active) {
            if (chan[c.chan].pcm) {
              chWrite(c.chan,0x04,0xc0|chan[c.chan].outVol);
            } else {
              chWrite(c.chan,0x04,0x80|chan[c.chan].outVol);
            }
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
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_WAVE:
      chan[c.chan].wave=c.value;
      chan[c.chan].ws.changeWave1(chan[c.chan].wave);
      chan[c.chan].keyOn=true;
      break;
    case DIV_CMD_PCE_LFO_MODE:
      if (c.value==0) {
        lfoMode=0;
      } else {
        lfoMode=c.value;
      }
      updateLFO=true;
      break;
    case DIV_CMD_PCE_LFO_SPEED:
      lfoSpeed=255-c.value;
      updateLFO=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2+chan[c.chan].sampleNoteDelta);
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
      chan[c.chan].noise=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_SAMPLE_MODE:
      chan[c.chan].pcm=c.value;
      break;
    case DIV_CMD_SAMPLE_BANK:
      sampleBank=c.value;
      if (sampleBank>(parent->song.sample.size()/12)) {
        sampleBank=parent->song.sample.size()/12;
      }
      break;
    case DIV_CMD_SAMPLE_POS:
      chan[c.chan].dacPos=c.value;
      chan[c.chan].setPos=true;
      break;
    case DIV_CMD_PANNING: {
      chan[c.chan].pan=(c.value&0xf0)|(c.value2>>4);
      chWrite(c.chan,0x05,isMuted[c.chan]?0:chan[c.chan].pan);
      break;
    }
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+chan[c.chan].sampleNoteDelta+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_PCE));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 31;
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

void DivPlatformPCE::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  chWrite(ch,0x05,isMuted[ch]?0:chan[ch].pan);
  if (!isMuted[ch] && (chan[ch].pcm && chan[ch].dacSample!=-1)) {
    chWrite(ch,0x04,parent->song.disableSampleMacro?0xdf:(0xc0|chan[ch].outVol));
    chWrite(ch,0x06,chan[ch].dacOut&0x1f);
  }
}

void DivPlatformPCE::forceIns() {
  for (int i=0; i<6; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
    updateWave(i);
    chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
}

void* DivPlatformPCE::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformPCE::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

unsigned short DivPlatformPCE::getPan(int ch) {
  return ((chan[ch].pan&0xf0)<<4)|(chan[ch].pan&15);
}

void DivPlatformPCE::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (ch==1 && lfoMode>0) {
    ret.push_back(DivChannelPair(_("mod"),0));
  }
}

DivChannelModeHints DivPlatformPCE::getModeHints(int ch) {
  DivChannelModeHints ret;
  if (ch<4) return ret;
  ret.count=1;
  ret.hint[0]=ICON_FUR_NOISE;
  ret.type[0]=0;

  if (chan[ch].noise) ret.type[0]=4;
  
  return ret;
}

DivSamplePos DivPlatformPCE::getSamplePos(int ch) {
  if (ch>=6) return DivSamplePos();
  if (!chan[ch].pcm) return DivSamplePos();
  return DivSamplePos(
    chan[ch].dacSample,
    chan[ch].dacPos,
    chan[ch].dacRate
  );
}

DivDispatchOscBuffer* DivPlatformPCE::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformPCE::mapVelocity(int ch, float vel) {
  return round(31.0*pow(vel,0.22));
}

float DivPlatformPCE::getGain(int ch, int vol) {
  if (vol==0) return 0;
  return 1.0/pow(10.0,(float)(31-vol)*3.0/20.0);
}

unsigned char* DivPlatformPCE::getRegisterPool() {
  return regPool;
}

int DivPlatformPCE::getRegisterPoolSize() {
  return 112;
}

void DivPlatformPCE::reset() {
  writes.clear();
  memset(regPool,0,128);
  for (int i=0; i<6; i++) {
    chan[i]=DivPlatformPCE::Channel();
    chan[i].std.setEngine(parent);
    chan[i].ws.setEngine(parent);
    chan[i].ws.init(NULL,32,31,false);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
  pce->Power(0);
  lastPan=0xff;
  curChan=-1;
  sampleBank=0;
  lfoMode=0;
  lfoSpeed=255;
  // set global volume
  rWrite(0,0);
  rWrite(0x01,0xff);
  // set LFO
  updateLFO=true;
  // set per-channel initial panning
  for (int i=0; i<6; i++) {
    chWrite(i,0x05,isMuted[i]?0:chan[i].pan);
  }
}

int DivPlatformPCE::getOutputCount() {
  return 2;
}

bool DivPlatformPCE::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformPCE::hasAcquireDirect() {
  return true;
}

void DivPlatformPCE::notifyWaveChange(int wave) {
  for (int i=0; i<6; i++) {
    if (chan[i].wave==wave) {
      chan[i].ws.changeWave1(wave);
      updateWave(i);
    }
  }
}

void DivPlatformPCE::notifyInsDeletion(void* ins) {
  for (int i=0; i<6; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformPCE::setFlags(const DivConfig& flags) {
  if (flags.getInt("clockSel",0)) { // technically there is no PAL PC Engine but oh well...
    chipClock=COLOR_PAL*8.0/5.0;
  } else {
    chipClock=COLOR_NTSC*2.0;
  }
  CHECK_CUSTOM_CLOCK;
  antiClickEnabled=!flags.getBool("noAntiClick",false);
  rate=chipClock;
  for (int i=0; i<6; i++) {
    oscBuf[i]->setRate(rate);
  }

  if (pce!=NULL) {
    delete pce;
    pce=NULL;
  }
  pce=new PCE_PSG(flags.getInt("chipType",0)?PCE_PSG::REVISION_HUC6280A:PCE_PSG::REVISION_HUC6280);
}

void DivPlatformPCE::poke(unsigned int addr, unsigned short val) {
  rWrite(addr,val);
}

void DivPlatformPCE::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& i: wlist) rWrite(i.addr,i.val);
}

void DivPlatformPCE::setCoreQuality(unsigned char q) {
  switch (q) {
    case 0:
      coreQuality=192;
      break;
    case 1:
      coreQuality=96;
      break;
    case 2:
      coreQuality=48;
      break;
    case 3:
      coreQuality=24;
      break;
    case 4:
      coreQuality=6;
      break;
    case 5:
      coreQuality=2;
      break;
    default:
      coreQuality=24;
      break;
  }
}

int DivPlatformPCE::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  updateLFO=false;
  for (int i=0; i<6; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  pce=NULL;
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformPCE::quit() {
  for (int i=0; i<6; i++) {
    delete oscBuf[i];
  }
  if (pce!=NULL) {
    delete pce;
    pce=NULL;
  }
}

DivPlatformPCE::~DivPlatformPCE() {
}
