/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include "scsp.h"
#include "../engine.h"
#include <math.h>

extern "C" {
#include "../../../extern/scsp/scsp_bridge.h"
#include "../../../extern/scsp/scsp_waveforms.h"
}

#define CHIP_FREQBASE 4096

// SCSP samples are stored as little-endian 16-bit signed PCM in sound RAM.
// On x86/ARM (LE hosts) we can write through reinterpret_cast<short*>;
// on BE hosts we'd need byte-swap. Furnace's primary targets are LE so
// we assume that here.

// SCSP playback rate is determined by OCT (signed 4-bit) and FNS (unsigned
// 10-bit) packed into register 0x8. From scsp.c SCSP_Step():
//   step = FNS_Table[FNS] << OCT  (OCT positive)  / 44100
//   step = FNS_Table[FNS] >> -OCT (OCT negative)  / 44100
// where FNS_Table[i] = (1<<SHIFT) * 44100 * (1024+i)/1024.
// Therefore native-rate playback (step = 1<<SHIFT) corresponds to OCT=0, FNS=0;
// each integer increase in OCT doubles the rate; FNS interpolates linearly
// from 1.0× to ~2.0× within the octave.
static void computeOctFnsFromHz(double targetHz, int* outOct, int* outFns) {
  if (targetHz<=0.0) targetHz=1.0;
  double octaves=log2(targetHz/44100.0);
  int oct=(int)floor(octaves);
  double frac=octaves-oct;
  int fns=(int)((pow(2.0,frac)-1.0)*1024.0+0.5);
  if (fns>=1024) { fns-=1024; oct+=1; }
  if (fns<0) fns=0;
  if (oct<-8) { oct=-8; fns=0; }
  if (oct>7)  { oct=7;  fns=0x3FF; }
  *outOct=oct & 0xF;
  *outFns=fns & 0x3FF;
}

int DivPlatformSCSP::midiNoteAtNativeRate(int sampleRate) {
  if (sampleRate<=0) return 60;
  double octaves=log2((double)sampleRate/44100.0);
  return 60+(int)(octaves*12.0+0.5);
}

void DivPlatformSCSP::writeSlotPitch(int slot, int midiNote, int baseMidiNote) {
  // Legacy entry point kept for source compatibility; converts MIDI note
  // delta to a Hz target then defers to computeOctFnsFromHz.
  double targetHz=44100.0*pow(2.0,(double)(midiNote-baseMidiNote)/12.0);
  int oct, fns;
  computeOctFnsFromHz(targetHz,&oct,&fns);
  unsigned short val=((oct&0xF)<<11)|(fns&0x3FF);
  scsp_write_slot(slot,0x8,val);
}

void DivPlatformSCSP::writeSlotEnvelope(int slot, unsigned char ar, unsigned char d1r,
                                       unsigned char d2r, unsigned char rr,
                                       unsigned char dl, unsigned char krs) {
  // reg 0x4: D2R[15:11] | D1R[10:6] | EGHOLD[5] | AR[4:0]
  unsigned short r4=(((unsigned short)(d2r&0x1F))<<11)
                  | (((unsigned short)(d1r&0x1F))<<6)
                  |  ((unsigned short)(ar &0x1F));
  scsp_write_slot(slot,0x4,r4);
  // reg 0x5: LPSLNK[14] | KRS[13:10] | DL[9:5] | RR[4:0]
  unsigned short r5=(((unsigned short)(krs&0xF))<<10)
                  | (((unsigned short)(dl &0x1F))<<5)
                  |  ((unsigned short)(rr &0x1F));
  scsp_write_slot(slot,0x5,r5);
}

void DivPlatformSCSP::writeSlotTotalLevel(int slot, unsigned char tl) {
  // reg 0x6: STWINH[9] | SDIR[8] | TL[7:0]
  scsp_write_slot(slot,0x6,(unsigned short)(tl&0xFF));
}

void DivPlatformSCSP::writeSlotPan(int slot, unsigned char disdl, unsigned char dipan) {
  // reg 0xB upper byte: DISDL[15:13] | DIPAN[12:8]; lower byte (EFSDL/EFPAN) preserved by helper
  scsp_slot_set_direct_output(slot,disdl,dipan);
}

int DivPlatformSCSP::findFreeRun(int numSlots) {
  if (numSlots<1 || numSlots>32) return -1;
  for (int s=0; s<=32-numSlots; s++) {
    bool ok=true;
    for (int j=0; j<numSlots; j++) {
      if (slotInUse[s+j]) { ok=false; break; }
    }
    if (ok) return s;
  }
  return -1;
}

int DivPlatformSCSP::findLruVoice() {
  int best=-1;
  unsigned long long bestAge=~(unsigned long long)0;
  for (int i=0; i<8; i++) {
    if (voices[i].active && voices[i].age<=bestAge) {
      bestAge=voices[i].age;
      best=i;
    }
  }
  return best;
}

void DivPlatformSCSP::releaseVoice(int chanIdx) {
  for (int i=0; i<8; i++) {
    if (voices[i].active && voices[i].chan==chanIdx) {
      for (int s=0; s<voices[i].slotCount; s++) {
        int slot=voices[i].firstSlot+s;
        scsp_key_off(slot);
        slotInUse[slot]=false;
      }
      voices[i].active=false;
      return;
    }
  }
}

void DivPlatformSCSP::releaseAllVoices() {
  for (int i=0; i<8; i++) {
    if (voices[i].active) {
      for (int s=0; s<voices[i].slotCount; s++) {
        scsp_key_off(voices[i].firstSlot+s);
      }
      voices[i].active=false;
    }
  }
  for (int s=0; s<32; s++) slotInUse[s]=false;
}

// Allocate `numSlots` contiguous hardware slots for a tracker channel.
// First-fit (matches bebhionn — keeps allocations packed at low indices,
// improving FM ring-buffer locality). On full, steal LRU voices and retry.
// Returns first-slot index or -1 if allocation failed.
int DivPlatformSCSP::allocateVoice(int chanIdx, int note, int numSlots) {
  releaseVoice(chanIdx);

  int start=findFreeRun(numSlots);

  for (int attempt=0; attempt<8 && start<0; attempt++) {
    int lru=findLruVoice();
    if (lru<0) break;
    for (int s=0; s<voices[lru].slotCount; s++) {
      int slot=voices[lru].firstSlot+s;
      scsp_key_off(slot);
      slotInUse[slot]=false;
    }
    voices[lru].active=false;
    start=findFreeRun(numSlots);
  }

  if (start<0) return -1;

  int rec=-1;
  for (int i=0; i<8; i++) {
    if (!voices[i].active) { rec=i; break; }
  }
  if (rec<0) return -1;

  voices[rec].chan=chanIdx;
  voices[rec].note=note;
  voices[rec].firstSlot=start;
  voices[rec].slotCount=numSlots;
  voices[rec].age=allocCounter++;
  voices[rec].active=true;

  for (int s=0; s<numSlots; s++) slotInUse[start+s]=true;
  return start;
}

void DivPlatformSCSP::programSlot(int slot, int chanIdx) {
  Channel& c=chan[chanIdx];
  if (c.sample<0 || c.sample>=parent->song.sampleLen || !sampleLoaded[c.sample]) {
    return;
  }
  DivSample* s=parent->song.sample[c.sample];
  DivInstrument* ins=parent->getIns(c.ins,DIV_INS_YMF292);
  bool isScspIns=(ins->type==DIV_INS_YMF292);

  unsigned int sampleByte=USER_SAMPLE_BASE+sampleOff[c.sample];

  unsigned int loopStart=s->isLoopable()?(unsigned int)s->loopStart:0;
  unsigned int loopEnd=s->isLoopable()?(unsigned int)s->loopEnd:(unsigned int)s->samples;
  if (loopEnd<1) loopEnd=1;
  if (loopEnd>0xFFFF) loopEnd=0xFFFF;
  if (loopStart>=loopEnd) loopStart=0;

  unsigned char lpctl=0;
  if (s->isLoopable()) {
    switch (s->loopMode) {
      case DIV_SAMPLE_LOOP_FORWARD:  lpctl=1; break;
      case DIV_SAMPLE_LOOP_BACKWARD: lpctl=2; break;
      case DIV_SAMPLE_LOOP_PINGPONG: lpctl=3; break;
      default: lpctl=1; break;
    }
  }
  // Instrument can override loop control
  if (isScspIns && ins->scsp.lpctl!=0) lpctl=ins->scsp.lpctl&0x3;

  unsigned char eghold=0, lpslnk=0, sdir=0, stwinh=0;
  if (isScspIns) {
    eghold=ins->scsp.eghold?1:0;
    lpslnk=ins->scsp.lpslnk?1:0;
    sdir  =ins->scsp.sdir?1:0;
    stwinh=ins->scsp.stwinh?1:0;
  }

  // reg 0x0: bits 5..6 LPCTL, bit 4 PCM8B (we always use 16-bit), bits 0..3 SA hi
  unsigned short r0=((lpctl&0x3)<<5)|((sampleByte>>16)&0xF);
  scsp_write_slot(slot,0x0,r0);

  // reg 0x1: SA low 16 bits
  scsp_write_slot(slot,0x1,(unsigned short)(sampleByte&0xFFFF));
  // reg 0x2: LSA — loop start in samples
  scsp_write_slot(slot,0x2,(unsigned short)(loopStart&0xFFFF));
  // reg 0x3: LEA — loop end in samples
  scsp_write_slot(slot,0x3,(unsigned short)(loopEnd&0xFFFF));

  // Envelope from instrument, fall back to instant attack
  unsigned char ar  = isScspIns? ins->scsp.ar  : 31;
  unsigned char d1r = isScspIns? ins->scsp.d1r : 0;
  unsigned char d2r = isScspIns? ins->scsp.d2r : 0;
  unsigned char rr  = isScspIns? ins->scsp.rr  : 31;
  unsigned char dl  = isScspIns? ins->scsp.dl  : 0;
  unsigned char krs = isScspIns? ins->scsp.krs : 0xF;

  // reg 0x4: D2R[15:11] | D1R[10:6] | EGHOLD[5] | AR[4:0]
  unsigned short r4=(((unsigned short)(d2r&0x1F))<<11)
                  | (((unsigned short)(d1r&0x1F))<<6)
                  | ((eghold&1)<<5)
                  |  ((unsigned short)(ar &0x1F));
  scsp_write_slot(slot,0x4,r4);

  // reg 0x5: LPSLNK[14] | KRS[13:10] | DL[9:5] | RR[4:0]
  unsigned short r5=((lpslnk&1)<<14)
                  | (((unsigned short)(krs&0xF))<<10)
                  | (((unsigned short)(dl &0x1F))<<5)
                  |  ((unsigned short)(rr &0x1F));
  scsp_write_slot(slot,0x5,r5);

  // reg 0x6: STWINH[9] | SDIR[8] | TL[7:0] — TL from channel volume
  unsigned char tl=(unsigned char)(127-(c.outVol&0x7F));
  if (isScspIns && ins->scsp.tl>tl) tl=ins->scsp.tl;
  unsigned short r6=((stwinh&1)<<9)|((sdir&1)<<8)|(tl&0xFF);
  scsp_write_slot(slot,0x6,r6);

  // reg 0x7: MDL[15:12] | MDXSL[11:6] | MDYSL[5:0] — FM only, zero for PCM
  scsp_write_slot(slot,0x7,0);

  // reg 0x9: LFOF[14:10] | PLFOWS[9:8] | PLFOS[7:5] | ALFOWS[4:3] | ALFOS[2:0]
  unsigned short r9=0;
  if (isScspIns) {
    r9=(((unsigned short)(ins->scsp.lfof   &0x1F))<<10)
     | (((unsigned short)(ins->scsp.plfows&0x3))<<8)
     | (((unsigned short)(ins->scsp.plfos &0x7))<<5)
     | (((unsigned short)(ins->scsp.alfows&0x3))<<3)
     |  ((unsigned short)(ins->scsp.alfos &0x7));
    if (ins->scsp.lforeset) r9|=0x8000;
  }
  scsp_write_slot(slot,0x9,r9);

  // DSP send (reg 0xA): ISEL[6:3] | IMXL[2:0]
  if (isScspIns) {
    scsp_slot_set_effect_send(slot,ins->scsp.isel,ins->scsp.imxl);
  } else {
    scsp_slot_set_effect_send(slot,0,0);
  }

  // EFSDL/EFPAN (lower byte of reg 0xB)
  if (isScspIns) {
    scsp_slot_set_effect_output(slot,ins->scsp.efsdl,ins->scsp.efpan);
  } else {
    scsp_slot_set_effect_output(slot,0,0);
  }

  // DISDL/DIPAN (upper byte of reg 0xB) — direct mix output
  unsigned char disdl=isMuted[chanIdx]?0:(isScspIns?(ins->scsp.disdl&0x7):7);
  unsigned char dipan=(unsigned char)(c.pan&0x1F);
  writeSlotPan(slot,disdl,dipan);

  c.sampleSet=true;
}

void DivPlatformSCSP::acquire(short** buf, size_t len) {
  for (int i=0; i<8; i++) {
    oscBuf[i]->begin(len);
  }

  // Render in chunks bounded by the bridge buffer (8192 stereo pairs).
  size_t off=0;
  while (off<len) {
    size_t chunk=len-off;
    if (chunk>4096) chunk=4096;
    int16_t* rendered=scsp_render((int)chunk);
    for (size_t i=0; i<chunk; i++) {
      buf[0][off+i]=rendered[i*2+0];
      buf[1][off+i]=rendered[i*2+1];
    }
    off+=chunk;
  }

  // No per-slot oscilloscope yet — emit zeros for now. Phase A2+ will
  // tap individual slot output via SCSP_DoMasterSample's slot accumulator.
  for (size_t i=0; i<len; i++) {
    for (int j=0; j<8; j++) {
      oscBuf[j]->putSample(i,0);
    }
  }

  for (int i=0; i<8; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSCSP::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  // Re-program direct-output level: mute forces DISDL=0
  if (chan[ch].slot>=0) {
    unsigned char disdl=mute?0:7;
    unsigned char dipan=(unsigned char)(chan[ch].pan&0x1F);
    writeSlotPan(chan[ch].slot,disdl,dipan);
  }
}

void DivPlatformSCSP::tick(bool sysTick) {
  for (int i=0; i<8; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG((chan[i].vol&0x7F),(0x7F*chan[i].std.vol.val)/0x7F,0x7F);
      if (chan[i].slot>=0) {
        unsigned char tl=(unsigned char)(127-(chan[i].outVol&0x7F));
        writeSlotTotalLevel(chan[i].slot,tl);
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

    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-131071,131071);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }

    if (chan[i].std.panL.had) {
      chan[i].pan=chan[i].std.panL.val&0x1F;
      if (chan[i].slot>=0) {
        unsigned char disdl=isMuted[i]?0:7;
        writeSlotPan(chan[i].slot,disdl,(unsigned char)(chan[i].pan&0x1F));
      }
    }
  }

  for (int i=0; i<8; i++) {
    if (chan[i].keyOn || chan[i].keyOff || chan[i].freqChanged) {
      if (chan[i].keyOn) {
        // PCM mode = 1 slot. FM mode (Phase B) will pass opCount here.
        int firstSlot=allocateVoice(i,chan[i].note,1);
        chan[i].slot=firstSlot;
        if (firstSlot>=0) {
          programSlot(firstSlot,i);
        }
      }
      if (chan[i].sample>=0 && chan[i].slot>=0 && sampleLoaded[chan[i].sample]) {
        DivSample* s=parent->song.sample[chan[i].sample];
        // Use Furnace's pitch model end-to-end (handles arp, portamento,
        // pitch macros, fine pitch, linear-vs-period mode).
        double freqUnits=(double)parent->calcFreq(
          chan[i].baseFreq,chan[i].pitch,
          chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,
          chan[i].fixedArp,false,2,chan[i].pitch2,
          chipClock,CHIP_FREQBASE);
        // calcFreq returns: tuning * 2^((note-60+3)/12) * CHIP_FREQBASE / chipClock
        // Convert to Hz at the note: noteHz = freqUnits * chipClock / CHIP_FREQBASE
        double noteHz=freqUnits*(double)chipClock/(double)CHIP_FREQBASE;
        // Apply the sample's centerRate offset (Furnace convention: a
        // centerRate equal to parent->getCenterRate() means "play at note's
        // tuning frequency"; higher means upshift).
        double off=(s->centerRate>=1.0)?((double)s->centerRate/parent->getCenterRate()):1.0;
        double targetHz=noteHz*off;
        int oct, fns;
        computeOctFnsFromHz(targetHz,&oct,&fns);
        unsigned short val=((oct&0xF)<<11)|(fns&0x3FF);
        scsp_write_slot(chan[i].slot,0x8,val);
        chan[i].freqChanged=false;
      }
      if (chan[i].keyOn) {
        scsp_key_on(chan[i].slot);
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        releaseVoice(i);
        chan[i].slot=-1;
        chan[i].keyOff=false;
      }
    }
  }
}

SharedChannel* DivPlatformSCSP::getChanState(int ch) {
  return &chan[ch];
}

DivDispatchOscBuffer* DivPlatformSCSP::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformSCSP::getRegisterPool() {
  return regPool;
}

int DivPlatformSCSP::getRegisterPoolSize() {
  return 1024+48;
}

int DivPlatformSCSP::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].sample=ins->amiga.getSample(c.value);
        chan[c.chan].sampleNote=ins->amiga.getFreq(c.value);
        chan[c.chan].sampleNoteDelta=c.value-chan[c.chan].sampleNote;
        chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
        chan[c.chan].note=c.value;
        chan[c.chan].freqChanged=true;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].keyOff=false;
      chan[c.chan].macroInit(ins);
      if (!chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
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
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      if (!chan[c.chan].std.vol.has) {
        chan[c.chan].outVol=c.value;
      }
      if (chan[c.chan].slot>=0) {
        unsigned char tl=(unsigned char)(127-(chan[c.chan].outVol&0x7F));
        writeSlotTotalLevel(chan[c.chan].slot,tl);
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].insChanged=true;
      }
      chan[c.chan].ins=c.value;
      break;
    case DIV_CMD_PANNING:
      chan[c.chan].pan=parent->convertPanSplitToLinearLR(c.value,c.value2,30);
      if (chan[c.chan].slot>=0) {
        unsigned char disdl=isMuted[c.chan]?0:7;
        writeSlotPan(chan[c.chan].slot,disdl,(unsigned char)(chan[c.chan].pan&0x1F));
      }
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
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
    case DIV_CMD_LEGATO:
      chan[c.chan].baseFreq=NOTE_FREQUENCY(c.value);
      chan[c.chan].note=c.value;
      chan[c.chan].freqChanged=true;
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
      return 127;
    case DIV_CMD_SCSP_LFO_FREQ:
    case DIV_CMD_SCSP_PLFO_DEPTH:
    case DIV_CMD_SCSP_ALFO_DEPTH: {
      if (chan[c.chan].slot<0) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      bool isScspIns=(ins->type==DIV_INS_YMF292);
      unsigned char lfof  =isScspIns?ins->scsp.lfof  :0;
      unsigned char plfows=isScspIns?ins->scsp.plfows:0;
      unsigned char plfos =isScspIns?ins->scsp.plfos :0;
      unsigned char alfows=isScspIns?ins->scsp.alfows:0;
      unsigned char alfos =isScspIns?ins->scsp.alfos :0;
      if (c.cmd==DIV_CMD_SCSP_LFO_FREQ)   lfof =c.value&0x1F;
      if (c.cmd==DIV_CMD_SCSP_PLFO_DEPTH) plfos=c.value&0x07;
      if (c.cmd==DIV_CMD_SCSP_ALFO_DEPTH) alfos=c.value&0x07;
      unsigned short r9=(((unsigned short)(lfof  &0x1F))<<10)
                      | (((unsigned short)(plfows&0x3))<<8)
                      | (((unsigned short)(plfos &0x7))<<5)
                      | (((unsigned short)(alfows&0x3))<<3)
                      |  ((unsigned short)(alfos &0x7));
      scsp_write_slot(chan[c.chan].slot,0x9,r9);
      break;
    }
    case DIV_CMD_SCSP_KRS: {
      if (chan[c.chan].slot<0) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      bool isScspIns=(ins->type==DIV_INS_YMF292);
      unsigned char dl =isScspIns?ins->scsp.dl :0;
      unsigned char rr =isScspIns?ins->scsp.rr :31;
      unsigned char krs=c.value&0xF;
      writeSlotEnvelope(chan[c.chan].slot,
                        isScspIns?ins->scsp.ar :31,
                        isScspIns?ins->scsp.d1r:0,
                        isScspIns?ins->scsp.d2r:0,
                        rr, dl, krs);
      break;
    }
    case DIV_CMD_SCSP_DSP_SEND: {
      if (chan[c.chan].slot<0) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      unsigned char efpan=(ins->type==DIV_INS_YMF292)?ins->scsp.efpan:0;
      scsp_slot_set_effect_output(chan[c.chan].slot,c.value&0x7,efpan);
      break;
    }
    case DIV_CMD_SCSP_DSP_PAN: {
      if (chan[c.chan].slot<0) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      unsigned char efsdl=(ins->type==DIV_INS_YMF292)?ins->scsp.efsdl:0;
      scsp_slot_set_effect_output(chan[c.chan].slot,efsdl,c.value&0x1F);
      break;
    }
    case DIV_CMD_SCSP_DIRECT_SEND: {
      if (chan[c.chan].slot<0) break;
      writeSlotPan(chan[c.chan].slot,c.value&0x7,(unsigned char)(chan[c.chan].pan&0x1F));
      break;
    }
    case DIV_CMD_SCSP_DIRECT_PAN: {
      if (chan[c.chan].slot<0) break;
      chan[c.chan].pan=c.value&0x1F;
      writeSlotPan(chan[c.chan].slot,isMuted[c.chan]?0:7,(unsigned char)(chan[c.chan].pan&0x1F));
      break;
    }
    default:
      break;
  }
  return 1;
}

void DivPlatformSCSP::notifyInsDeletion(void* ins) {
  for (int i=0; i<8; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformSCSP::notifySampleChange(int sample) {
  // Engine calls renderSamples on sample edits; this is just a hook for
  // chips that maintain per-sample state outside of RAM. We have none.
}

void DivPlatformSCSP::notifyInsAddition(int sysID) {
  // No persistent instrument table; nothing to do.
}

void DivPlatformSCSP::notifyPitchTable(int sample) {
  pitchTable.init(parent->song.tuning,chipClock,CHIP_FREQBASE,0xffff,false,parent->song.compatFlags.linearPitch);
}

void DivPlatformSCSP::reset() {
  scsp_init();
  for (int i=0; i<8; i++) {
    chan[i]=DivPlatformSCSP::Channel(parent->song.compatFlags.linearPitch);
    chan[i].pitchTable=&pitchTable;
    chan[i].vol=0x7F;
    chan[i].outVol=0x7F;
  }
  for (int i=0; i<32; i++) {
    slotInUse[i]=false;
  }
  for (int i=0; i<8; i++) {
    voices[i]=Voice();
  }
  allocCounter=0;
  memset(regPool,0,sizeof(regPool));

  // Re-upload sample memory: scsp_init zeroed RAM, so we need to refill it.
  uint8_t* ram=scsp_get_ram_ptr();
  if (sampleMem!=NULL && ram!=NULL) {
    memcpy(ram,sampleMem,(sampleMemLen<RAM_SIZE)?sampleMemLen:RAM_SIZE);
  }

  // Load the 10 built-in FM waveforms into RAM[0x0000..0x4FFF].
  // Done after the user-sample memcpy (which writes zeros over this region
  // from the always-zero head of sampleMem) so the builtins win.
  for (int i=0; i<10; i++) builtinOffsets[i]=0;
  if (ram!=NULL) {
    scsp_load_builtins(ram,builtinOffsets);
  }
}

void DivPlatformSCSP::forceIns() {
  for (int i=0; i<8; i++) {
    chan[i].insChanged=true;
    chan[i].freqChanged=true;
  }
}

int DivPlatformSCSP::getOutputCount() {
  return 2;
}

bool DivPlatformSCSP::hasSoftPan(int ch) {
  return true;
}

bool DivPlatformSCSP::keyOffAffectsArp(int ch) {
  return false;
}

bool DivPlatformSCSP::keyOffAffectsPorta(int ch) {
  return false;
}

void DivPlatformSCSP::setFlags(const DivConfig& flags) {
  // SCSP master clock is 22.5792 MHz on Saturn; output rate is 44100 Hz.
  chipClock=22579200;
  rate=44100;
  for (int i=0; i<8; i++) {
    if (oscBuf[i]) oscBuf[i]->setRate(rate);
  }
}

void DivPlatformSCSP::poke(unsigned int addr, unsigned short val) {
  scsp_write_reg(addr,val);
}

void DivPlatformSCSP::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite& w: wlist) {
    scsp_write_reg(w.addr,w.val);
  }
}

const void* DivPlatformSCSP::getSampleMem(int index) {
  return (index==0)?sampleMem:NULL;
}

size_t DivPlatformSCSP::getSampleMemCapacity(int index) {
  return (index==0)?(RAM_SIZE-USER_SAMPLE_BASE):0;
}

size_t DivPlatformSCSP::getSampleMemUsage(int index) {
  return (index==0)?(sampleMemLen>USER_SAMPLE_BASE?sampleMemLen-USER_SAMPLE_BASE:0):0;
}

bool DivPlatformSCSP::isSampleLoaded(int index, int sample) {
  if (index!=0) return false;
  if (sample<0 || sample>=65536) return false;
  return sampleLoaded[sample];
}

const DivMemoryComposition* DivPlatformSCSP::getMemCompo(int index) {
  return (index==0)?&memCompo:NULL;
}

void DivPlatformSCSP::renderSamples(int sysID) {
  if (sampleMem==NULL) return;
  memset(sampleMem,0,RAM_SIZE);
  memset(sampleOff,0,65536*sizeof(unsigned int));
  memset(sampleLoaded,0,65536*sizeof(bool));
  sampleMemLen=USER_SAMPLE_BASE;

  memCompo=DivMemoryComposition();
  memCompo.name="Sound RAM";

  size_t memPos=USER_SAMPLE_BASE;
  int sampleCount=parent->song.sampleLen;
  for (int i=0; i<sampleCount; i++) {
    DivSample* s=parent->song.sample[i];
    if (!s->renderOn[0][sysID]) {
      sampleOff[i]=0;
      continue;
    }
    // store as 16-bit signed PCM (SCSP native format)
    int sampleLength=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_16BIT);
    int byteLength=sampleLength*2;
    if (byteLength<2) continue;
    if (memPos+byteLength>RAM_SIZE) {
      logW("out of SCSP sound RAM for sample %d!",i);
      break;
    }
    short* src=s->data16;
    if (src==NULL) continue;
    memcpy(sampleMem+memPos,src,byteLength);
    sampleOff[i]=(unsigned int)(memPos-USER_SAMPLE_BASE);
    sampleLoaded[i]=true;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+byteLength));
    memPos+=byteLength;
  }
  sampleMemLen=memPos;
  memCompo.used=memPos;
  memCompo.capacity=RAM_SIZE;

  // Push to live SCSP RAM
  uint8_t* ram=scsp_get_ram_ptr();
  if (ram!=NULL) {
    memcpy(ram,sampleMem,(sampleMemLen<RAM_SIZE)?sampleMemLen:RAM_SIZE);
  }
}

int DivPlatformSCSP::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<8; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  setFlags(flags);

  sampleMem=new unsigned char[RAM_SIZE];
  sampleMemLen=USER_SAMPLE_BASE;
  memset(sampleMem,0,RAM_SIZE);
  sampleOff=new unsigned int[65536];
  memset(sampleOff,0,65536*sizeof(unsigned int));
  sampleLoaded=new bool[65536];
  memset(sampleLoaded,0,65536*sizeof(bool));

  notifyPitchTable();
  reset();
  return 8;
}

void DivPlatformSCSP::quit() {
  for (int i=0; i<8; i++) {
    delete oscBuf[i];
  }
  delete[] sampleMem;
  delete[] sampleOff;
  delete[] sampleLoaded;
  sampleMem=NULL;
  sampleOff=NULL;
  sampleLoaded=NULL;
}

DivPlatformSCSP::~DivPlatformSCSP() {
}
