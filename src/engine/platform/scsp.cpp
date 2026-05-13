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
#include "../scspdspasm.h"
#include "IconsFontAwesome4.h"
#include <math.h>

extern "C" {
#include "../../../extern/scsp/scsp_bridge.h"
}

// Default waveform period assumed by the FM operator math when no sample-
// specific length is available. Matches the SCSP_WAVE_LEN constant the
// (now-removed) built-in waveform generator used.
static const int SCSP_WAVE_LEN = 1024;

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

// ── FM helpers ────────────────────────────────────────────────────────────
//
// Used by the FM dispatch path to translate high-level op parameters into
// SCSP register words. Static file-scope so they don't leak into the class ABI.

// Pack OCT/FNS register bits from a desired MIDI note and the base note
// at which the underlying sample plays unshifted.
static unsigned short computeOctFnsBits(double midiNote, double opBaseNote) {
  double semi=midiNote-opBaseNote;
  int octave=(int)floor(semi/12.0);
  if (octave<-8) octave=-8;
  if (octave>7)  octave=7;
  double frac=semi-(double)octave*12.0;
  int fns=(int)floor(1024.0*(pow(2.0,frac/12.0)-1.0)+0.5);
  if (fns<0) fns=0;
  if (fns>1023) fns=1023;
  return (unsigned short)(((octave&0xF)<<11)|(fns&0x3FF));
}

// Natural base MIDI note of a 1024-sample waveform at 44100 Hz: the note
// at which its natural period plays back at OCT=0 FNS=0.
static double wavBaseNoteFor(int wavLen) {
  if (wavLen<=0) wavLen=1024;
  return 69.0+12.0*log2(44100.0/(double)wavLen/440.0);
}

// Convert a packed TL byte to its linear gain (matches scsp_voice.c).
static double tlToLinear(unsigned char tl) {
  double db=0.0;
  if (tl & 1)   db -= 0.4;
  if (tl & 2)   db -= 0.8;
  if (tl & 4)   db -= 1.5;
  if (tl & 8)   db -= 3.0;
  if (tl & 16)  db -= 6.0;
  if (tl & 32)  db -= 12.0;
  if (tl & 64)  db -= 24.0;
  if (tl & 128) db -= 48.0;
  return pow(10.0, db/20.0);
}

// Derive a feedback-path MDL nibble from a target beta and the carrier TL.
// Mirrors scsp_voice.c:compute_mdl, including the max_safe clamp that
// prevents runaway feedback when the modulator is near full scale.
// `feedback` is 0..127 (mapped to beta 0..π).
static unsigned char computeFeedbackMdl(unsigned char tl, unsigned char feedback) {
  if (feedback==0) return 0;
  double tlLin=tlToLinear(tl);
  double ringPeak=32767.0*4.0*tlLin*0.5;
  if (ringPeak<1.0) return 0;
  double targetBeta=((double)feedback/127.0)*M_PI;
  double needed=targetBeta*1024.0/(ringPeak*2.0*M_PI);
  if (needed<1e-10) needed=1e-10;
  int mdl=(int)floor(16.0+log2(needed)+0.5);
  if (mdl<0) mdl=0;
  if (mdl>15) mdl=15;
  double maxSafe=1024.0/(ringPeak*2.0);
  if (maxSafe<1e-10) maxSafe=1e-10;
  int maxMdl=(int)floor(15.0+log2(maxSafe));
  if (mdl>maxMdl) mdl=maxMdl;
  if (mdl<0) mdl=0;
  return (unsigned char)(mdl&0xF);
}

// Compute d7 (MDL|MDXSL|MDYSL) from a high-level op, resolving modSource
// to an absolute slot via slotBase.
//
// Sound-stack indexing (Sega SCSP doc 4.2/4.3, MAME RINGBUF impl): the chip
// reads two 6-bit indices into a 64-entry ring updated 32×/Fs. For a slot's
// own past output the ring offsets are 0x20 ("1 Fs ago" — the doc's
// "latest" sample) and 0x00 ("2 Fs ago" — "past"). For an external
// modulator at slot M reading from carrier C, J=M-C; (M-C)&63 reads the
// modulator's previous-Fs write (doc's "past sample"), which is the
// standard 1-Fs-latency FM convention.
//
// Self-feedback caveat (p04_fm3): the doc explicitly warns that feeding
// the same self-output sample to BOTH X and Y inputs of the averager can
// oscillate. Self-FB must use one latest (0x20) and one past (0x00).
//
// Critical (vs the JS reference): explicit (unsigned) cast on slot
// subtraction before & 63. JS bitwise ops are 32-bit; in C, signed shift
// on a negative value is implementation-defined.
static unsigned short computeD7FromOp(unsigned char mdl, signed char modSource,
                                      unsigned char feedback, unsigned char tl,
                                      int slot, int slotBase) {
  static const unsigned int SELF_LATEST=0x20u; // ring offset 32 = 1 Fs ago
  static const unsigned int SELF_PAST  =0x00u; // ring offset  0 = 2 Fs ago
  unsigned int regMdl=0, mdxsl=0, mdysl=0;
  if (modSource>=0 && mdl>0) {
    regMdl=(unsigned int)mdl & 0xF;
    int modSlot=slotBase+(int)modSource;
    unsigned int dist=(unsigned int)(modSlot-slot) & 0x3F;
    mdxsl=dist;
    mdysl=dist;
  }
  if (feedback>0) {
    unsigned char fbMdl=computeFeedbackMdl(tl, feedback);
    if (regMdl>0) {
      // External mod already on X (latest of another slot); route self-FB
      // to Y as the past self-sample to avoid same-cycle self-coupling.
      mdysl=SELF_PAST;
      if ((unsigned int)fbMdl>regMdl) regMdl=(unsigned int)fbMdl;
    } else {
      regMdl=(unsigned int)fbMdl;
      mdxsl=SELF_LATEST;
      mdysl=SELF_PAST;
    }
  }
  return (unsigned short)(((regMdl&0xF)<<12)|((mdxsl&0x3F)<<6)|(mdysl&0x3F));
}

// Compute OCT/FNS bits for one FM op given a desired MIDI note. The op's
// effective base note is derived from its freqRatio (Q8.8) or freqFixed.
static unsigned short computeFMOctBitsForOp(const DivInstrumentSCSP::Op& op, double midiNote) {
  int wavLen=SCSP_WAVE_LEN;
  double wavBaseFreq=44100.0/(double)wavLen;
  double wavBaseNote=wavBaseNoteFor(wavLen);
  double opBaseNote;
  if (op.freqFixed>0) {
    opBaseNote=wavBaseNote+12.0*log2((double)op.freqFixed/wavBaseFreq);
  } else {
    double ratio=(double)op.freqRatio/256.0;
    if (ratio<=0.0) ratio=1.0;
    opBaseNote=wavBaseNote-12.0*log2(ratio);
  }
  return computeOctFnsBits(midiNote, opBaseNote);
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

// Release chanIdx's currently-owned slot run (key-off the hardware slots
// and clear the per-channel ownership state). Safe to call when the chan
// is already idle.
void DivPlatformSCSP::releaseChan(int chanIdx) {
  int n=activeOpCount[chanIdx];
  if (n<=0) return;
  for (int s=0; s<n; s++) {
    int slot=chanIdx+s;
    if (slot<0 || slot>=32) continue;
    scsp_key_off(slot);
  }
  activeOpCount[chanIdx]=0;
  chan[chanIdx].slot=-1;
}

// Steal-on-overlap: any other chan whose owned [c', c'+activeOpCount[c']-1]
// intersects [anchorChan, anchorChan+numSlots-1] gets released. The new
// note about to be keyed at anchorChan is the winner.
void DivPlatformSCSP::stealOverlapping(int anchorChan, int numSlots) {
  int aLo=anchorChan;
  int aHi=anchorChan+numSlots-1;
  for (int c=0; c<32; c++) {
    if (c==anchorChan) continue;
    int n=activeOpCount[c];
    if (n<=0) continue;
    int oLo=c;
    int oHi=c+n-1;
    if (oHi<aLo || oLo>aHi) continue;
    // Stolen — silence its slots and clear all pending state so the
    // tick loop doesn't try to act on a corpse.
    releaseChan(c);
    chan[c].keyOn=false;
    chan[c].keyOff=false;
    chan[c].active=false;
    chan[c].freqChanged=false;
  }
}

// Apply DISDL/DIPAN across every owned slot of chanIdx. Mute forces DISDL=0;
// FM modulator ops force DISDL=0 regardless so they stay internal to the
// FM ring.
void DivPlatformSCSP::updateChanDirectOutput(int chanIdx) {
  int n=activeOpCount[chanIdx];
  if (n<=0) return;
  DivInstrument* ins=parent->getIns(chan[chanIdx].ins,DIV_INS_YMF292);
  bool isScspIns=(ins->type==DIV_INS_YMF292);
  bool isFMIns=isScspIns && (ins->scsp.mode==DivInstrumentSCSP::SCSP_MODE_FM);
  unsigned char dipan=(unsigned char)(chan[chanIdx].pan&0x1F);
  for (int s=0; s<n; s++) {
    int slot=chanIdx+s;
    unsigned char disdl;
    if (isMuted[chanIdx]) {
      disdl=0;
    } else if (isFMIns) {
      disdl=ins->scsp.ops[s].isCarrier?7:0;
    } else {
      disdl=isScspIns?(ins->scsp.disdl&0x7):7;
    }
    scsp_slot_set_direct_output(slot,disdl,dipan);
  }
}

// Apply current channel TL across the chan's owned slots. For FM voices,
// only carriers receive the volume — modulators keep the TL set by
// programSlotFM so their FM index is preserved.
void DivPlatformSCSP::updateChanVolume(int chanIdx) {
  int n=activeOpCount[chanIdx];
  if (n<=0) return;
  DivInstrument* ins=parent->getIns(chan[chanIdx].ins,DIV_INS_YMF292);
  bool isFMIns=(ins->type==DIV_INS_YMF292) && (ins->scsp.mode==DivInstrumentSCSP::SCSP_MODE_FM);
  unsigned char chanTl=(unsigned char)(127-(chan[chanIdx].outVol&0x7F));
  for (int s=0; s<n; s++) {
    int slot=chanIdx+s;
    if (isFMIns && !ins->scsp.ops[s].isCarrier) continue;
    writeSlotTotalLevel(slot,chanTl);
  }
}

// Program one slot of an FM voice from a high-level op definition.
void DivPlatformSCSP::programSlotFM(int slot, int chanIdx, int opIdx, int slotBase, double midiNote) {
  Channel& c=chan[chanIdx];
  DivInstrument* ins=parent->getIns(c.ins,DIV_INS_YMF292);
  if (ins->type!=DIV_INS_YMF292) return;
  const DivInstrumentSCSP::Op& op=ins->scsp.ops[opIdx];

  // Resolve the op's waveform source. sampleId>=0 picks a user sample
  // (any length, any loop config); otherwise fall back to the 1024-sample
  // built-in indexed by `waveform`. Pure FM math assumes a 1024-sample
  // modulator, so a long sample as a modulator will alias — that's a
  // documented experimentation hazard, not enforced here.
  unsigned int sa;
  unsigned int lsa, lea;
  unsigned char lpctl;
  bool useSample=(op.sampleId>=0 &&
                  op.sampleId<parent->song.sampleLen &&
                  sampleLoaded[op.sampleId]);
  if (useSample) {
    DivSample* s=parent->song.sample[op.sampleId];
    sa=sampleOff[op.sampleId];
    // sampleStored may be smaller than s->samples if SCSP RAM ran out
    // and renderSamples truncated the upload. Use the stored count so
    // the slot doesn't read past the last uploaded frame into another
    // sample's bytes (or zeroed RAM).
    unsigned int storedFrames=sampleStored[op.sampleId];
    if (storedFrames<1) storedFrames=1;
    bool needsLoop=(!op.isCarrier) || op.feedback>0 ||
                   (op.modSource>=0 && op.mdl>=5);
    if (s->isLoopable() && (unsigned int)s->loopEnd<=storedFrames) {
      lsa=(unsigned int)s->loopStart;
      lea=(unsigned int)s->loopEnd;
      switch (s->loopMode) {
        case DIV_SAMPLE_LOOP_FORWARD:  lpctl=1; break;
        case DIV_SAMPLE_LOOP_BACKWARD: lpctl=2; break;
        case DIV_SAMPLE_LOOP_PINGPONG: lpctl=3; break;
        default: lpctl=1; break;
      }
    } else if (needsLoop) {
      // Modulator/feedback ops MUST keep producing output continuously,
      // even if the user picked a non-looping sample — otherwise the
      // slot reaches end-of-sample and stops, killing the FM modulation.
      lsa=0;
      lea=storedFrames;
      lpctl=1;
    } else {
      lsa=0;
      lea=storedFrames;
      lpctl=0;
    }
  } else {
    // No sample assigned — leave the slot unprogrammed (and the caller
    // skips key-on for this op so it stays silent).
    return;
  }
  if (lea>0xFFFF) lea=0xFFFF;
  if (lsa>=lea) lsa=0;

  unsigned short octBits=computeFMOctBitsForOp(op, midiNote);

  // TL: linear-in-level.
  int tlInt=(int)floor((1.0-(double)op.level/127.0)*128.0+0.5);
  if (tlInt<0) tlInt=0;
  if (tlInt>255) tlInt=255;
  unsigned char tl=(unsigned char)tlInt;

  unsigned short d4=(((unsigned short)(op.d2r&0x1F))<<11)
                  | (((unsigned short)(op.d1r&0x1F))<<6)
                  |  ((unsigned short)(op.ar &0x1F));
  // Force KRS=0xF (key-rate scaling disabled). Without this, the DR_TIMES
  // tables (which assume KRS=0xF) disagree with the hardware envelope and
  // the high-level rate→time mapping is wrong away from ~A3.
  unsigned short d5=((unsigned short)0xF<<10)
                  | (((unsigned short)(op.dl&0x1F))<<5)
                  |  ((unsigned short)(op.rr&0x1F));
  unsigned short d7=computeD7FromOp(op.mdl, op.modSource, op.feedback, tl, slot, slotBase);

  unsigned char disdl=isMuted[chanIdx]?0:(op.isCarrier?7:0);
  unsigned char dipan=(unsigned char)(c.pan&0x1F);

  unsigned short r0=((lpctl&0x3)<<5)|((sa>>16)&0xF);
  scsp_write_slot(slot,0x0,r0);
  scsp_write_slot(slot,0x1,(unsigned short)(sa&0xFFFF));
  scsp_write_slot(slot,0x2,(unsigned short)(lsa&0xFFFF));
  scsp_write_slot(slot,0x3,(unsigned short)(lea&0xFFFF));
  scsp_write_slot(slot,0x4,d4);
  scsp_write_slot(slot,0x5,d5);
  scsp_write_slot(slot,0x6,(unsigned short)(tl&0xFF));
  scsp_write_slot(slot,0x7,d7);
  scsp_write_slot(slot,0x8,octBits);
  scsp_write_slot(slot,0x9,0);
  // DSP send (reg 0xA): only carriers contribute to the FX bus. The
  // modulator's raw output is internal to the FM ring buffer — routing it
  // to MIXS via IMXL would dump an unenveloped/distorted signal alongside
  // the carrier and produce clicks at note-on (sharp AR=31 step) plus a
  // generally wrong wet signal.
  if (op.isCarrier) {
    scsp_slot_set_effect_send(slot,ins->scsp.isel,ins->scsp.imxl);
  } else {
    scsp_slot_set_effect_send(slot,0,0);
  }
  scsp_slot_set_effect_output(slot,0,0);
  scsp_slot_set_direct_output(slot,disdl,dipan);
}

void DivPlatformSCSP::programSlot(int slot, int chanIdx) {
  Channel& c=chan[chanIdx];
  if (c.sample<0 || c.sample>=parent->song.sampleLen || !sampleLoaded[c.sample]) {
    return;
  }
  DivSample* s=parent->song.sample[c.sample];
  DivInstrument* ins=parent->getIns(c.ins,DIV_INS_YMF292);
  bool isScspIns=(ins->type==DIV_INS_YMF292);

  unsigned int sampleByte=sampleOff[c.sample];

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
  for (int i=0; i<32; i++) {
    oscBuf[i]->begin(len);
  }

  // Render in chunks bounded by the bridge buffer (8192 stereo pairs).
  size_t off=0;
  // Per-frame, per-slot post-EG/pan-L capture. The MAME backend writes
  // each slot's audible left-bus contribution here; the aosdk backend's
  // bridge stub leaves the buffer untouched (osc shows zeros). Modulator
  // slots have DISDL=0 so their LPANTABLE entry is zero and they emit
  // nothing through this path — matching what the listener hears for an
  // FM voice (only the carrier is audible).
  static int16_t slotCap[4096*32];
  scsp_set_slot_capture(slotCap);

  while (off<len) {
    size_t chunk=len-off;
    if (chunk>4096) chunk=4096;
    memset(slotCap,0,chunk*32*sizeof(int16_t));
    short* rendered=scsp_render((int)chunk);
    for (size_t i=0; i<chunk; i++) {
      buf[0][off+i]=rendered[i*2+0];
      buf[1][off+i]=rendered[i*2+1];
    }
    // Per-channel oscilloscope. With the 1:1 chan→slot anchor model, an
    // idle chan owns no slots (capture row stays zero from the memset).
    // An active chan owns slots [chIdx, chIdx+activeOpCount-1] — sum them
    // to get the audible output: PCM is one slot direct; FM modulators
    // contribute zero (DISDL=0), so the sum reduces to just the carrier.
    for (int chIdx=0; chIdx<32; chIdx++) {
      int n=activeOpCount[chIdx];
      if (n<=0) {
        for (size_t i=0; i<chunk; i++) oscBuf[chIdx]->putSample(off+i,0);
        continue;
      }
      int slotCount=n;
      if (chIdx+slotCount>32) slotCount=32-chIdx;
      for (size_t i=0; i<chunk; i++) {
        int s=0;
        const int16_t* row=slotCap+i*32+chIdx;
        for (int sl=0; sl<slotCount; sl++) s+=row[sl];
        if (s>32767) s=32767;
        if (s<-32768) s=-32768;
        oscBuf[chIdx]->putSample(off+i,(short)s);
      }
    }
    off+=chunk;
  }

  scsp_set_slot_capture(NULL);

  for (int i=0; i<32; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformSCSP::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  updateChanDirectOutput(ch);
}

void DivPlatformSCSP::tick(bool sysTick) {
  for (int i=0; i<32; i++) {
    chan[i].std.next();

    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG((chan[i].vol&0x7F),(0x7F*chan[i].std.vol.val)/0x7F,0x7F);
      updateChanVolume(i);
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
      updateChanDirectOutput(i);
    }
  }

  for (int i=0; i<32; i++) {
    if (chan[i].keyOn || chan[i].keyOff || chan[i].freqChanged) {
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_YMF292);
      bool isFMIns=(ins->type==DIV_INS_YMF292) && (ins->scsp.mode==DivInstrumentSCSP::SCSP_MODE_FM);

      if (chan[i].keyOn) {
        // Determine slot run: PCM=1, FM=opCount (clamped to fit in 32 slots
        // from the chan anchor). With 1:1 chan→slot mapping, op k of an
        // FM voice on chan i lands on slot i+k.
        int numSlots=1;
        if (isFMIns) {
          numSlots=ins->scsp.opCount;
          if (numSlots<1) numSlots=1;
          if (numSlots>32) numSlots=32;
        }
        if (i+numSlots>32) numSlots=32-i;

        // FLEXIBILITY ROADMAP — DSP-pinned slots. If the chan's required
        // slot run hits a DSP-pinned slot (slots 0/1 when a DSP program is
        // loaded), suppress the note rather than stomping the DSP routing.
        // Future: let the user pick the DSP-out slots, removing this gate.
        bool blocked=false;
        for (int k=0; k<numSlots; k++) {
          if (slotInUse[i+k]) { blocked=true; break; }
        }

        if (blocked) {
          chan[i].keyOn=false;
          chan[i].slot=-1;
          chan[i].active=false;
        } else {
          stealOverlapping(i,numSlots);
          releaseChan(i);
          activeOpCount[i]=numSlots;
          chan[i].slot=i;
          if (isFMIns) {
            for (int op=0; op<numSlots; op++) {
              programSlotFM(i+op, i, op, i, (double)chan[i].note);
            }
          } else {
            programSlot(i,i);
          }
        }
      }

      if (chan[i].slot>=0) {
        if (isFMIns) {
          // Bypass parent->calcFreq: with SCSP's chipClock=22.58 MHz and
          // CHIP_FREQBASE=4096, the divider/clock ratio (~0.000181) makes
          // round(fbase * divider/clock) always truncate to 0. Compute the
          // target MIDI note directly from chan[i].note + arp + fine pitch.
          //
          // Furnace's tracker labels (noteNames[60]="C-0", noteNames[96]="C-3"
          // etc.) align with standard MIDI octaves, but the internal note
          // number has 60 entries of "negative" octaves below: internal
          // note = 60 + 12*octave, while standard MIDI = 12 + 12*octave.
          // So MIDI = internal - 48. (Verified against Genesis playback:
          // tracker "C-3" / internal 96 / MIDI 48 = ~130 Hz.)
          double midiNoteFurnace=(double)chan[i].note;
          if (!parent->song.compatFlags.oldArpStrategy) {
            if (chan[i].fixedArp) {
              midiNoteFurnace=(double)chan[i].baseNoteOverride;
            } else {
              midiNoteFurnace+=(double)chan[i].arpOff;
            }
          }
          midiNoteFurnace+=(double)(chan[i].pitch+chan[i].pitch2)/128.0;
          double midiNote=midiNoteFurnace-48.0;

          int n=activeOpCount[i];
          for (int op=0; op<n; op++) {
            unsigned short octBits=computeFMOctBitsForOp(ins->scsp.ops[op], midiNote);
            scsp_write_slot(i+op, 0x8, octBits);
          }
          chan[i].freqChanged=false;
        } else if (chan[i].sample>=0 && sampleLoaded[chan[i].sample]) {
          // Bypass parent->calcFreq for the same reason as the FM path:
          // with SCSP's CHIP_FREQBASE=4096 / chipClock=22.58 MHz ratio,
          // round(fbase * divider/clock) truncates to 0. Recompute fbase
          // in double precision matching calcFreq's linear-pitch math
          // (calcBaseFreq + nbase + 2^((nbase-7296)/1536) * tuning).
          double semitone;
          if (parent->song.compatFlags.linearPitch) {
            int nbase=chan[i].baseFreq+chan[i].pitch+chan[i].pitch2;
            if (!parent->song.compatFlags.oldArpStrategy) {
              if (chan[i].fixedArp) {
                nbase=(chan[i].baseNoteOverride<<7)+chan[i].pitch+chan[i].pitch2;
              } else {
                nbase+=chan[i].arpOff<<7;
              }
            }
            semitone=(double)nbase/128.0;
          } else {
            // Non-linear pitch has no chip-native freq unit on SCSP, so
            // pitch deltas are best-effort: integer note + arp + (pitch in
            // 128ths) treated as semitone offsets.
            semitone=(double)chan[i].note;
            if (!parent->song.compatFlags.oldArpStrategy) {
              if (chan[i].fixedArp) semitone=(double)chan[i].baseNoteOverride;
              else semitone+=(double)chan[i].arpOff;
            }
            semitone+=(double)(chan[i].pitch+chan[i].pitch2)/128.0;
          }
          double noteHz=(double)parent->song.tuning*
                        pow(2.0,(semitone-60.0+3.0)/12.0);

          DivSample* s=parent->song.sample[chan[i].sample];
          // Apply the sample's centerRate offset (Furnace convention: a
          // centerRate equal to parent->getCenterRate() means "play at the
          // note's tuning frequency"; higher means upshift).
          double off=(s->centerRate>=1.0)?((double)s->centerRate/parent->getCenterRate()):1.0;
          double targetHz=noteHz*off;
          int oct, fns;
          computeOctFnsFromHz(targetHz,&oct,&fns);
          unsigned short val=((oct&0xF)<<11)|(fns&0x3FF);
          scsp_write_slot(chan[i].slot,0x8,val);
          chan[i].freqChanged=false;
        }
      }

      if (chan[i].keyOn) {
        int n=activeOpCount[i];
        for (int s=0; s<n; s++) {
          scsp_key_on(i+s);
        }
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        releaseChan(i);
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

// Surface multi-op FM patches in the channel-pair UI hint so users can see
// which slots are occupied by which patch's ops *right now*. Driven off
// runtime activeOpCount[] (not the loaded instrument), matching Furnace's
// convention that pair hints are runtime indicators (the consumer in
// pattern.cpp gates on e->isRunning() anyway). When a note ends and the
// chan releases its slots, activeOpCount→0 and the indicators clear.
// Conflicts between overlapping anchors don't pile up because the
// runtime steal-on-overlap leaves only one active anchor per slot range.
//
// For a currently-active N-op FM voice anchored at slot A (activeOpCount[A]=N):
//  - the anchor's pair list points forward to A+1..A+N-1 ("op 2".."op N")
//  - each consumed slot's pair list points back to A ("from S<A+1>")
//
// Labels must outlive the call (DivChannelPair stores const char*), so
// they're indexed into static tables instead of formatted per-call.
void DivPlatformSCSP::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  static const char* const opLabels[33]={
    "op 0","op 1","op 2","op 3","op 4","op 5","op 6","op 7",
    "op 8","op 9","op 10","op 11","op 12","op 13","op 14","op 15",
    "op 16","op 17","op 18","op 19","op 20","op 21","op 22","op 23",
    "op 24","op 25","op 26","op 27","op 28","op 29","op 30","op 31",
    "op 32",
  };
  static const char* const fromLabels[32]={
    "from S1","from S2","from S3","from S4","from S5","from S6","from S7","from S8",
    "from S9","from S10","from S11","from S12","from S13","from S14","from S15","from S16",
    "from S17","from S18","from S19","from S20","from S21","from S22","from S23","from S24",
    "from S25","from S26","from S27","from S28","from S29","from S30","from S31","from S32",
  };

  // Forward: ch is currently an anchor of a multi-op voice.
  int n=activeOpCount[ch];
  if (n>1) {
    for (int k=1; k<n && (ch+k)<32; k++) {
      ret.push_back(DivChannelPair(opLabels[k+1],(signed char)(ch+k)));
    }
  }

  // Reverse: find any earlier chan whose active range covers ch.
  for (int a=ch-1; a>=0; a--) {
    int an=activeOpCount[a];
    if (an>1 && a+an-1>=ch) {
      ret.push_back(DivChannelPair(fromLabels[a],(signed char)a));
      break;
    }
  }
}

// Per-slot mode hints: link icon on anchors of currently-active multi-op
// voices, ban icon on slots being consumed by another anchor's active range.
// Driven off runtime activeOpCount[] (clears as soon as the note ends),
// matching pair-arrow semantics. Requires "Pattern channel status" enabled
// in Furnace settings to be visible.
DivChannelModeHints DivPlatformSCSP::getModeHints(int ch) {
  DivChannelModeHints ret;
  if (ch<0 || ch>=32) return ret;

  // Anchor of a currently-active multi-op voice.
  int n=activeOpCount[ch];
  if (n>1) {
    ret.count=1;
    ret.hint[0]=ICON_FA_LINK;
    ret.type[0]=4;  // chip primary
    return ret;
  }

  // Consumed by an earlier anchor's active range.
  for (int a=ch-1; a>=0; a--) {
    int an=activeOpCount[a];
    if (an>1 && a+an-1>=ch) {
      ret.count=1;
      ret.hint[0]=ICON_FA_BAN;
      ret.type[0]=5;  // chip secondary
      return ret;
    }
  }

  return ret;
}

void DivPlatformSCSP::refreshRegPool() {
  // Big-endian halfword pack helper — matches Saturn-driver register
  // dumps and tonview's display order.
  auto putBE = [&](int off, unsigned short v) {
    regPool[off  ] = (unsigned char)((v >> 8) & 0xFF);
    regPool[off+1] = (unsigned char)( v       & 0xFF);
  };
  // Slot register space: 32 slots × 16 regs.
  for (int s=0; s<32; s++) {
    for (int r=0; r<16; r++) {
      putBE(s*32 + r*2, scsp_read_slot_reg(s,r));
    }
  }
  // Common register space (24 words → 48 bytes).
  for (int r=0; r<24; r++) {
    putBE(0x400 + r*2, scsp_read_common_reg(r));
  }
  // DSP MPRO / COEF / MADRS.
  for (int i=0; i<512; i++) putBE(0x430 + i*2, scsp_dsp_get_mpro(i));
  for (int i=0; i<64;  i++) putBE(0x830 + i*2, (unsigned short)scsp_dsp_get_coef(i));
  for (int i=0; i<32;  i++) putBE(0x8B0 + i*2, scsp_dsp_get_madrs(i));
}

unsigned char* DivPlatformSCSP::getRegisterPool() {
  refreshRegPool();
  return regPool;
}

int DivPlatformSCSP::getRegisterPoolSize() {
  return REG_POOL_SIZE;
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
      updateChanVolume(c.chan);
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
      updateChanDirectOutput(c.chan);
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
    // ── FM-mode performance effects (20xx-43xx). Patch the affected slot
    // registers in-flight without re-running programSlotFM, so a row that
    // carries multiple effects accumulates correctly. State is reset on the
    // next note-on. Not preserved by Saturn SEQ export — Furnace-only.
    case DIV_CMD_SCSP_OP_TL: {
      int n=activeOpCount[c.chan];
      if (n<=0) break;
      int opIdx=c.value;
      if (opIdx<0 || opIdx>=n) break;
      scsp_write_slot(c.chan+opIdx,0x6,(unsigned short)(c.value2&0xFF));
      break;
    }
    case DIV_CMD_SCSP_OP_MDL: {
      int n=activeOpCount[c.chan];
      if (n<=0) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      if (ins->type!=DIV_INS_YMF292 || ins->scsp.mode!=DivInstrumentSCSP::SCSP_MODE_FM) break;
      int opIdx=c.value;
      if (opIdx<0 || opIdx>=n) break;
      const DivInstrumentSCSP::Op& op=ins->scsp.ops[opIdx];
      unsigned char newMdl=c.value2&0xF;
      // Recompute TL like programSlotFM (needed for feedback ringPeak math).
      int tlInt=(int)floor((1.0-(double)op.level/127.0)*128.0+0.5);
      if (tlInt<0) tlInt=0;
      if (tlInt>255) tlInt=255;
      int slot=c.chan+opIdx;
      unsigned short d7=computeD7FromOp(newMdl, op.modSource, op.feedback,
                                         (unsigned char)tlInt, slot, c.chan);
      scsp_write_slot(slot,0x7,d7);
      break;
    }
    case DIV_CMD_SCSP_FEEDBACK: {
      int n=activeOpCount[c.chan];
      if (n<=0) break;
      DivInstrument* ins=parent->getIns(chan[c.chan].ins,DIV_INS_YMF292);
      if (ins->type!=DIV_INS_YMF292 || ins->scsp.mode!=DivInstrumentSCSP::SCSP_MODE_FM) break;
      unsigned char newFb=c.value&0x7F;
      for (int op=0; op<n; op++) {
        const DivInstrumentSCSP::Op& opdef=ins->scsp.ops[op];
        int tlInt=(int)floor((1.0-(double)opdef.level/127.0)*128.0+0.5);
        if (tlInt<0) tlInt=0;
        if (tlInt>255) tlInt=255;
        int slot=c.chan+op;
        unsigned short d7=computeD7FromOp(opdef.mdl, opdef.modSource, newFb,
                                           (unsigned char)tlInt, slot, c.chan);
        scsp_write_slot(slot,0x7,d7);
      }
      break;
    }
    default:
      break;
  }
  return 1;
}

void DivPlatformSCSP::notifyInsDeletion(void* ins) {
  for (int i=0; i<32; i++) {
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
  for (int i=0; i<32; i++) {
    chan[i]=DivPlatformSCSP::Channel(parent->song.compatFlags.linearPitch);
    chan[i].pitchTable=&pitchTable;
    chan[i].vol=0x7F;
    chan[i].outVol=0x7F;
    activeOpCount[i]=0;
    slotInUse[i]=false;
  }
  memset(regPool,0,sizeof(regPool));

  // Re-upload sample memory: scsp_init zeroed RAM, so we need to refill it.
  unsigned char* ram=scsp_get_ram_ptr();
  if (sampleMem!=NULL && ram!=NULL) {
    memcpy(ram,sampleMem,(sampleMemLen<RAM_SIZE)?sampleMemLen:RAM_SIZE);
  }

  // Push the song's on-chip DSP program (also reserves slots 0/1 as the
  // DSP output bus when a program is active).
  reloadDSP();
}

// FLEXIBILITY ROADMAP — DSP output bus.
//
// Hardware: per Sega doc 4.2/4.1, *any* slot can be the DSP effect-out tap
// by setting EFSDL>0 on it; the slot's PG/EG output is replaced by the
// DSP's EFREGxx contribution and routed to the direct mixer via DISDL/DIPAN.
// There is no architectural reason to limit the DSP to two slots, two
// EFREG taps, or fixed slot indices — multiple slots can each pick a
// different EFREGxx and be mixed/panned independently.
//
// Wrapper today: pins slots 0 and 1 as a stereo wet bus (EFSDL=7,
// EFPAN=0x10) when a DSP program is loaded, mirroring the JS reference and
// keeping the voice allocator simple. The cost is a hard "max 6 voices
// while DSP is active" limit and zero user control over which EFREGxx
// taps reach the mixer.
//
// Future: expose DSP routing as per-instrument or per-channel config —
// "tap N from EFREGxx, pan P, send level S" — and let the user choose
// which slots host the taps. The cleanest design is probably a
// song-level table of (slot, EFREGxx, pan, send) entries that the wrapper
// applies during reset/reload, with the rest of the slot pool unaffected.
// Multiple-tap mono mixes (e.g. 4 taps panned across the field) and
// dry-only DSP (taps with EFSDL=0 used purely for sample-modify chains)
// both fall out of the same table.
bool DivPlatformSCSP::reloadDSP() {
  dspLastErrors.clear();
  dspLastWarnings.clear();
  dspStepsLoaded=0;
  if (parent->song.scspDspSource.empty()) {
    scsp_dsp_clear();
    // Release the DSP output reservation: clear EFSDL on slots 0/1 and
    // free them in the allocator. While DSP was active no voice could
    // land on them (we kept slotInUse pinned), so we can clear unconditionally.
    scsp_slot_set_effect_output(0,0,0);
    scsp_slot_set_effect_output(1,0,0);
    slotInUse[0]=false;
    slotInUse[1]=false;
    return true;
  }
  SCSPDSPAssembly asm_out;
  bool ok=scspdspAssemble(parent->song.scspDspSource,parent->song.scspDspRBL,asm_out);
  if (ok) {
    scsp_dsp_load_arrays(asm_out.mpro,512,asm_out.coef,64,asm_out.madrs,32,asm_out.rbl);
    scsp_dsp_start();
    dspStepsLoaded=asm_out.steps;
    // Reserve slots 0 and 1 as the DSP output bus and mark them slotInUse
    // so the allocator never lands a voice there (a voice would call
    // programSlotFM, overwriting EFSDL and silently killing the effect).
    // EFPAN=0x10 (bit4=1, atten=0) sends to both L+R equally, so any DSP
    // that writes to EFREG00/EFREG01 is audible without depending on the
    // assumed pan-bit semantics in scsp_bridge's comment.
    scsp_slot_set_effect_output(0,7,0x10);
    scsp_slot_set_effect_output(1,7,0x10);
    slotInUse[0]=true;
    slotInUse[1]=true;
  }
  dspLastErrors=asm_out.errors;
  dspLastWarnings=asm_out.warnings;
  return ok;
}

void DivPlatformSCSP::forceIns() {
  for (int i=0; i<32; i++) {
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
  for (int i=0; i<32; i++) {
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
  return (index==0)?RAM_SIZE:0;
}

size_t DivPlatformSCSP::getSampleMemUsage(int index) {
  return (index==0)?sampleMemLen:0;
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
  memset(sampleStored,0,65536*sizeof(unsigned int));
  memset(sampleLoaded,0,65536*sizeof(bool));
  sampleMemLen=0;

  memCompo=DivMemoryComposition();
  memCompo.name="Sound RAM";

  size_t memPos=0;
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
    short* src=s->data16;
    if (src==NULL) continue;
    int avail=(int)RAM_SIZE-(int)memPos;
    if (avail<2) {
      logW("SCSP RAM full, sample %d (%s) skipped",
           i,s->name.c_str());
      continue;
    }
    if (byteLength>avail) {
      // Truncate to fit. The Saturn has the same 512KB limit on real
      // hardware, so a sample too large for SCSP RAM was never going
      // to play back in full anyway. Loud warning so the user knows.
      logW("SCSP RAM nearly full: truncating sample %d (%s) from %d to %d frames",
           i,s->name.c_str(),sampleLength,(avail&~1)/2);
      byteLength=avail&~1;  // even byte count
      sampleLength=byteLength/2;
    }
    memcpy(sampleMem+memPos,src,byteLength);
    sampleOff[i]=(unsigned int)memPos;
    sampleStored[i]=(unsigned int)sampleLength;
    sampleLoaded[i]=true;
    memCompo.entries.push_back(DivMemoryEntry(DIV_MEMORY_SAMPLE,"Sample",i,memPos,memPos+byteLength));
    memPos+=byteLength;
  }
  sampleMemLen=memPos;
  memCompo.used=memPos;
  memCompo.capacity=RAM_SIZE;

  // Push to live SCSP RAM
  unsigned char* ram=scsp_get_ram_ptr();
  if (ram!=NULL) {
    memcpy(ram,sampleMem,(sampleMemLen<RAM_SIZE)?sampleMemLen:RAM_SIZE);
  }
}

int DivPlatformSCSP::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;

  for (int i=0; i<32; i++) {
    isMuted[i]=false;
    activeOpCount[i]=0;
    oscBuf[i]=new DivDispatchOscBuffer;
  }

  setFlags(flags);

  sampleMem=new unsigned char[RAM_SIZE];
  sampleMemLen=0;
  memset(sampleMem,0,RAM_SIZE);
  sampleOff=new unsigned int[65536];
  memset(sampleOff,0,65536*sizeof(unsigned int));
  sampleStored=new unsigned int[65536];
  memset(sampleStored,0,65536*sizeof(unsigned int));
  sampleLoaded=new bool[65536];
  memset(sampleLoaded,0,65536*sizeof(bool));

  notifyPitchTable();
  reset();
  return 32;
}

void DivPlatformSCSP::quit() {
  for (int i=0; i<32; i++) {
    delete oscBuf[i];
  }
  delete[] sampleMem;
  delete[] sampleOff;
  delete[] sampleStored;
  delete[] sampleLoaded;
  sampleMem=NULL;
  sampleOff=NULL;
  sampleStored=NULL;
  sampleLoaded=NULL;
}

DivPlatformSCSP::~DivPlatformSCSP() {
}
