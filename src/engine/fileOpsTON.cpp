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

// Saturn .TON (tone bank) export for SCSP FM instruments. Output is byte-
// compatible with mid2seq's saturn_kit.py, which produces TON files that the
// SGL sound driver loads via slInitSound.
//
// File layout (all multi-byte fields big-endian):
//   uint16   mixer_offset
//   uint16   vl_offset
//   uint16   peg_offset
//   uint16   plfo_offset
//   uint16[] voice_offsets         (one per voice)
//   bytes    mixer (0x12)
//   bytes    vl    (0x0A — velocity → TL piecewise curve)
//   bytes    peg   (0x0A)
//   bytes    plfo  (0x04)
//   per voice: 4-byte header + N × 32-byte layer entries
//   bytes    PCM data (16-bit BE samples for the embedded waveforms)
//
// Each layer maps a Furnace SCSP op onto the SCSP slot register layout.
// MDL goes in byte 0x10; MDXSL/MDYSL are zeroed and the FM layer link in
// byte 0x1B (bit 7 + layer index) tells the driver to compute the ring-
// buffer offsets at slot allocation time.

#include "engine.h"
#include "instrument.h"
#include "safeReader.h"
#include "safeWriter.h"
#include "sample.h"
#include <fmt/printf.h>

#include <math.h>
#include <vector>
#include <map>
#include <string.h>

#define TON_WAVE_LEN 1024

// Convert a packed TL byte to its linear gain (matches scsp_voice.c).
static double tonTlToLinear(unsigned char tl) {
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

// Mirrors scsp.cpp:computeFeedbackMdl / scsp_engine.js:computeFeedbackMdl.
static unsigned char tonComputeFeedbackMdl(unsigned char tl, unsigned char feedback) {
  if (feedback==0) return 0;
  double tlLin=tonTlToLinear(tl);
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

// Pack a Furnace SCSP op + waveform into a 32-byte TON layer at `out`.
// `saOffset` is the byte offset of this op's waveform within the TON file's
// PCM section (caller adds the file-level pcm_base before writing).
// `leaSamples` is the loop-end sample count to use; for built-ins the
// caller passes 1024, for user samples the sample's length.
static void buildLayer(unsigned char* out,
                       const DivInstrumentSCSP::Op& op,
                       unsigned int saOffset,
                       unsigned int leaSamples) {
  memset(out,0,0x20);
  out[0x00]=0;    // start_note
  out[0x01]=127;  // end_note

  // byte 0x02: FMCB at bit 5 (carrier marker — informational; the Saturn
  // driver ignores it and uses DISDL/fm_layer for routing decisions).
  if (op.isCarrier) out[0x02]|=(1<<5);

  // byte 0x03: LPCTL[6:5] | PCM8B[4]=0 | SA[19:16]
  // Loop is on by default; the Saturn driver may key-off naturally on
  // a long sample's end, but FM modulators need a continuous loop.
  unsigned char lpctl=1;
  out[0x03]=((lpctl&3)<<5)|((saOffset>>16)&0xF);

  // bytes 0x04-0x05: SA low (BE)
  out[0x04]=(saOffset>>8)&0xFF;
  out[0x05]=saOffset&0xFF;

  // bytes 0x06-0x07: LSA (BE), 0x08-0x09: LEA (BE).
  // For built-ins LEA=1024 (the whole 1024-sample wave); for user samples
  // LEA is the sample's actual length so the loop covers the recording.
  unsigned int lsa=0;
  unsigned int lea=leaSamples;
  if (lea>0xFFFF) lea=0xFFFF;
  out[0x06]=(lsa>>8)&0xFF; out[0x07]=lsa&0xFF;
  out[0x08]=(lea>>8)&0xFF; out[0x09]=lea&0xFF;

  // bytes 0x0A-0x0B: D2R[15:11] | D1R[10:6] | EGHOLD[5] | AR[4:0]
  // D1R is split: 3 high bits in 0x0A's low nibble, 2 low bits in 0x0B's high nibble.
  out[0x0A]=((op.d2r&0x1F)<<3)|((op.d1r>>2)&0x7);
  out[0x0B]=((op.d1r&0x3)<<6)|(op.ar&0x1F);

  // bytes 0x0C-0x0D: LPSLNK | KRS[13:10] | DL[9:5] | RR[4:0]
  // KRS=0xF disables key-rate scaling so envelope rates depend only on
  // R, matching writeSlotEnvelope. saturn_kit.py writes KRS=0 by default,
  // but that's a known bug: re-imported envelopes time differently than the
  // same patch played live, which makes round-tripped TONs sound wrong.
  // We deliberately diverge so our TONs play back correctly.
  out[0x0C]=(0xF<<2)|((op.dl>>3)&0x3);
  out[0x0D]=((op.dl&0x7)<<5)|(op.rr&0x1F);

  // byte 0x0F: TL — derived from level the same way programSlotFM does
  // (linear-in-level mapping).
  int tlInt=(int)floor((1.0-(double)op.level/127.0)*128.0+0.5);
  if (tlInt<0) tlInt=0;
  if (tlInt>255) tlInt=255;
  out[0x0F]=(unsigned char)tlInt;

  // bytes 0x10-0x11: MDL[15:12] | MDXSL[11:6] | MDYSL[5:0].
  // The driver-side remapD7Raw recomputes MDXSL/MDYSL based on the FM-layer
  // link in byte 0x1B (so for FM cascades the values we put here are
  // overwritten). For self-feedback ops (no FM link) the driver keeps MDL
  // as-is and applies its own −32 distance, so we must encode an effective
  // MDL here — `op.mdl` is 0 in feedback-only presets, so we derive it from
  // `op.feedback` the same way scsp.cpp:computeD7FromOp does at runtime.
  // We also fill MDXSL/MDYSL with sensible defaults (mod-source distance
  // for FM, 32 for feedback) so importers that don't recompute see the
  // right shape.
  unsigned int regMdl=0, mdxsl=0, mdysl=0;
  if (op.modSource>=0 && op.mdl>0) {
    regMdl=(unsigned int)op.mdl & 0xF;
    int dist=(int)op.modSource - 0;
    unsigned int distMasked=(unsigned int)dist & 63u;
    mdxsl=distMasked;
    mdysl=distMasked;
  }
  if (op.feedback>0) {
    unsigned int fbDist=(unsigned int)(-32) & 63u;
    unsigned char fbMdl=tonComputeFeedbackMdl((unsigned char)tlInt,op.feedback);
    if (regMdl>0) {
      mdysl=fbDist;
      if ((unsigned int)fbMdl>regMdl) regMdl=(unsigned int)fbMdl;
    } else {
      regMdl=(unsigned int)fbMdl;
      mdxsl=fbDist;
      mdysl=fbDist;
    }
  }
  out[0x10]=(unsigned char)(((regMdl&0xF)<<4) | ((mdxsl>>2)&0xF));
  out[0x11]=(unsigned char)(((mdxsl&0x3)<<6) | (mdysl&0x3F));

  // byte 0x17: ISEL=0, IMXL=7 (DSP effect input enabled at full mix).
  out[0x17]=7;

  // byte 0x18: DISDL[7:5] | DIPAN[4:0]. Carriers route to direct mix.
  out[0x18]=(op.isCarrier?7:0)<<5;

  // byte 0x19: base_note. base_note = 69 - round(12*log2(ratio)) so the
  // Saturn driver's note-dependent OCT/FNS computation lands at the right
  // pitch when MIDI A4 (note=69) is played.
  int baseNote=69;
  if (op.freqRatio>0) {
    double ratio=(double)op.freqRatio/256.0;
    if (ratio>0.0) {
      int ratioSemitones=(int)floor(12.0*log2(ratio)+0.5);
      baseNote=69-ratioSemitones;
      if (baseNote<0) baseNote=0;
      if (baseNote>127) baseNote=127;
    }
  }
  out[0x19]=baseNote&0x7F;

  // byte 0x1B: FM generator/layer link. Bit 7 = "this op is modulated";
  // bits 6:0 = the modulating layer's index within this voice.
  if (op.modSource>=0) {
    out[0x1B]=(1<<7)|((unsigned char)op.modSource&0x7F);
  }
}

SafeWriter* DivEngine::saveSCSPTON() {
  // Collect the FM-mode SCSP instruments to export.
  std::vector<int> insIndices;
  for (size_t i=0; i<song.ins.size(); i++) {
    DivInstrument* ins=song.ins[i];
    if (ins->type!=DIV_INS_YMF292) continue;
    if (ins->scsp.mode!=DivInstrumentSCSP::SCSP_MODE_FM) continue;
    if (ins->scsp.opCount<1) continue;
    insIndices.push_back((int)i);
  }
  if (insIndices.empty()) {
    lastError=_("no FM-mode SCSP instruments to export");
    return NULL;
  }

  // Walk every op once to figure out which waveforms (built-ins) and which
  // user samples are referenced. Each unique resource gets emitted once and
  // its byte offset within the PCM section is recorded for SA patch-up.
  // sampleIdToOffset: map<original sample index, byte offset in pcmData>.
  // For each op with sampleId>=0 we'll embed the sample's 16-bit BE PCM.
  std::map<int,unsigned int> sampleOffset;  // index -> pcm offset
  std::map<int,unsigned int> sampleLength;  // index -> length in samples

  // First pass: collect referenced samples.
  for (int idx: insIndices) {
    DivInstrument* ins=song.ins[idx];
    int n=ins->scsp.opCount;
    if (n>6) n=6;
    for (int op=0; op<n; op++) {
      const DivInstrumentSCSP::Op& opdef=ins->scsp.ops[op];
      if (opdef.sampleId>=0 && opdef.sampleId<song.sampleLen) {
        sampleOffset[opdef.sampleId]=0;  // placeholder, filled below
      }
    }
  }

  std::vector<unsigned char> pcmData;

  // Emit user samples: 16-bit BE PCM. data16 is signed 16-bit native LE
  // in Furnace; we byte-swap into the TON's BE layout as we go.
  for (std::pair<const int,unsigned int>& kv: sampleOffset) {
    int idx=kv.first;
    DivSample* s=song.sample[idx];
    int n=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_16BIT);
    if (n<1) n=1;
    sampleOffset[idx]=(unsigned int)pcmData.size();
    sampleLength[idx]=(unsigned int)n;
    short* src=s->data16;
    for (int si=0; si<n; si++) {
      short iv=src?src[si]:0;
      pcmData.push_back((unsigned char)((iv>>8)&0xFF));
      pcmData.push_back((unsigned char)(iv&0xFF));
    }
  }

  // Build each voice's bytes: 4-byte header + 32 bytes per layer.
  // SA in each layer holds the relative offset within pcmData; we'll add
  // pcm_base after computing the file layout.
  std::vector<std::vector<unsigned char> > voices;
  for (int idx: insIndices) {
    DivInstrument* ins=song.ins[idx];
    int n=ins->scsp.opCount;
    if (n<1) n=1;
    if (n>6) n=6;
    std::vector<unsigned char> v(4+n*0x20,0);
    v[0]=2;        // bend_range
    v[2]=(unsigned char)(n-1);  // num_layers - 1
    for (int op=0; op<n; op++) {
      const DivInstrumentSCSP::Op& opdef=ins->scsp.ops[op];
      unsigned int sa=0, lea=0;
      if (opdef.sampleId>=0 && sampleOffset.count(opdef.sampleId)) {
        sa=sampleOffset[opdef.sampleId];
        lea=sampleLength[opdef.sampleId];
        if (lea>0xFFFF) lea=0xFFFF;
      }
      // sa=0,lea=0 if op has no sample assigned; the layer's SA/LEA point
      // at the start of pcmData with zero length, which the Saturn driver
      // treats as silence.
      buildLayer(&v[4+op*0x20], opdef, sa, lea);
    }
    voices.push_back(v);
  }

  // Default fixed tables — values lifted from mechs.ton (verified working
  // on real Saturn hardware via mid2seq).
  unsigned char vl[10]={25,16,54,9,49,102,19,93,122,43};
  unsigned char mixer[0x12];
  memset(mixer,0,0x12);
  mixer[0]=(7<<5)|0x1F;  // EFREG0 → left, full level
  mixer[1]=(7<<5)|0x0F;  // EFREG1 → right, full level
  unsigned char peg[0x0A]; memset(peg,0,0x0A);
  unsigned char plfo[0x04]; memset(plfo,0,0x04);

  // Compute file offsets.
  unsigned int headerSize=8+(unsigned int)voices.size()*2;
  unsigned int mixerOff=headerSize;
  unsigned int vlOff=mixerOff+0x12;
  unsigned int pegOff=vlOff+0x0A;
  unsigned int plfoOff=pegOff+0x0A;
  std::vector<unsigned int> voiceOff;
  unsigned int cur=plfoOff+0x04;
  for (size_t i=0; i<voices.size(); i++) {
    voiceOff.push_back(cur);
    cur+=(unsigned int)voices[i].size();
  }
  unsigned int pcmBase=cur;

  // Patch each voice's layer SA fields to include pcm_base.
  for (size_t i=0; i<voices.size(); i++) {
    std::vector<unsigned char>& v=voices[i];
    int nlayers=(int)v[2]+1;
    for (int li=0; li<nlayers; li++) {
      int loff=4+li*0x20;
      unsigned int oldSa=((v[loff+0x03]&0xF)<<16)|(v[loff+0x04]<<8)|v[loff+0x05];
      unsigned int newSa=pcmBase+oldSa;
      v[loff+0x03]=(v[loff+0x03]&0xF0)|((newSa>>16)&0xF);
      v[loff+0x04]=(newSa>>8)&0xFF;
      v[loff+0x05]=newSa&0xFF;
    }
  }

  // Emit. SafeWriter::writeS is little-endian on Furnace's primary targets;
  // TON is big-endian, so write each 16-bit field as two bytes manually.
  SafeWriter* w=new SafeWriter;
  w->init();
  auto writeBE16=[&](unsigned int v){
    w->writeC((unsigned char)((v>>8)&0xFF));
    w->writeC((unsigned char)(v&0xFF));
  };
  writeBE16(mixerOff);
  writeBE16(vlOff);
  writeBE16(pegOff);
  writeBE16(plfoOff);
  for (size_t i=0; i<voiceOff.size(); i++) writeBE16(voiceOff[i]);
  w->write(mixer,0x12);
  w->write(vl,0x0A);
  w->write(peg,0x0A);
  w->write(plfo,0x04);
  for (size_t i=0; i<voices.size(); i++) {
    w->write(voices[i].data(),voices[i].size());
  }
  if (!pcmData.empty()) w->write(pcmData.data(),pcmData.size());
  return w;
}

// ─── TON import ───────────────────────────────────────────────────────────────
//
// Inverse of saveSCSPTON. Each voice → one DivInstrumentSCSP (FM mode); each
// 32-byte layer → one Op. Built-in waveform PCM is detected by content match
// against `scsp_gen_waveform`, and any non-matching PCM is added as a new
// DivSample so the op can reference it via `sampleId`. Per the plan we go
// straight to high-level fields — no rawRegs passthrough.

#define TON_BUILTIN_MATCH_TOL 64  // max abs sample diff for builtin recognition

// Inverse of tonComputeFeedbackMdl: given the 4-bit MDL nibble we read from
// the file plus the carrier TL, recover the original feedback level. Lossy
// because MDL is quantized to 4 bits and can be capped by the maxMdl clamp,
// but recovers the rough magnitude (e.g. distinguishes a low kick feedback
// from an industrial-hit max feedback) much better than a fixed 0.3 default.
static unsigned char tonRecoverFeedback(unsigned char tl, unsigned char mdl) {
  if (mdl==0) return 0;
  double tlLin=tonTlToLinear(tl);
  double ringPeak=32767.0*4.0*tlLin*0.5;
  if (ringPeak<1.0) return 0;
  double needed=pow(2.0,(double)mdl-16.0);
  double targetBeta=needed*ringPeak*2.0*M_PI/1024.0;
  double feedback=(targetBeta/M_PI)*127.0;
  if (feedback<0) feedback=0;
  if (feedback>127) feedback=127;
  return (unsigned char)floor(feedback+0.5);
}

void DivEngine::loadTON(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  size_t fileLen=reader.size();
  if (fileLen<10) {
    lastError=_("TON file too small");
    return;
  }
  // SafeReader doesn't expose its buffer; copy the whole file into a local
  // vector so we can do byte-offset random access (TON's natural layout).
  reader.seek(0,SEEK_SET);
  std::vector<unsigned char> buf(fileLen);
  reader.read(buf.data(),fileLen);
  const unsigned char* data=buf.data();

  unsigned int mixerOff=((unsigned int)data[0]<<8)|data[1];
  if (mixerOff<10 || mixerOff>fileLen) {
    lastError=_("TON header invalid (mixer offset out of range)");
    return;
  }
  unsigned int nVoices=(mixerOff-8)/2;
  if (nVoices==0 || nVoices>256) {
    lastError=_("TON file has no voices or too many");
    return;
  }

  std::vector<unsigned int> voiceOff(nVoices);
  for (unsigned int i=0; i<nVoices; i++) {
    voiceOff[i]=((unsigned int)data[8+i*2]<<8)|data[9+i*2];
    if (voiceOff[i]+4>fileLen) {
      lastError=_("TON voice offset out of range");
      return;
    }
  }

  // Cache imported samples to avoid duplicates:
  //   - saToSampleId folds ops that reference the same TON memory offset.
  //   - contentToSampleId folds ops whose PCM bytes happen to match an
  //     already-imported sample (Bebhionn frequently emits the same
  //     waveform under two different SAs across instruments).
  std::map<unsigned int,int> saToSampleId;
  std::map<std::vector<short>,int> contentToSampleId;
  int tonSampleSeq=0;

  for (unsigned int vi=0; vi<nVoices; vi++) {
    unsigned int vo=voiceOff[vi];
    int nLayers=(int)data[vo+2]+1;
    if (nLayers<1) nLayers=1;
    if (nLayers>6) nLayers=6;  // DivInstrumentSCSP::Op[6]
    if (vo+4+nLayers*0x20>fileLen) {
      lastError=_("TON layer data out of range");
      return;
    }

    DivInstrument* ins=new DivInstrument;
    ins->type=DIV_INS_YMF292;
    ins->name=fmt::sprintf("%s:%d",stripPath,vi+1);
    ins->scsp.mode=DivInstrumentSCSP::SCSP_MODE_FM;
    ins->scsp.opCount=nLayers;

    for (int li=0; li<nLayers; li++) {
      const unsigned char* l=data+vo+4+li*0x20;
      DivInstrumentSCSP::Op& op=ins->scsp.ops[li];

      unsigned int sa=((unsigned int)(l[0x03]&0xF)<<16)|((unsigned int)l[0x04]<<8)|l[0x05];
      unsigned int lea=((unsigned int)l[0x08]<<8)|l[0x09];
      unsigned int lsa=((unsigned int)l[0x06]<<8)|l[0x07];

      unsigned char d2r=(l[0x0A]>>3)&0x1F;
      unsigned char d1r=((l[0x0A]&0x7)<<2)|((l[0x0B]>>6)&0x3);
      unsigned char ar=l[0x0B]&0x1F;
      unsigned char dl=((l[0x0C]&0x3)<<3)|((l[0x0D]>>5)&0x7);
      unsigned char rr=l[0x0D]&0x1F;
      unsigned char tl=l[0x0F];
      unsigned char mdl=(l[0x10]>>4)&0xF;
      unsigned int mdxsl=((unsigned int)(l[0x10]&0xF)<<2)|((l[0x11]>>6)&0x3);
      unsigned int mdysl=l[0x11]&0x3F;
      unsigned char disdl=(l[0x18]>>5)&0x7;
      unsigned char baseNote=l[0x19]&0x7F;
      unsigned char fmByte=l[0x1B];

      op.ar=ar; op.d1r=d1r; op.d2r=d2r; op.dl=dl; op.rr=rr;
      op.mdl=mdl;
      op.isCarrier=(disdl>0);

      // level = round((1 - tl/128) * 127), clamped
      int levelInt=(int)floor((1.0-(double)tl/128.0)*127.0+0.5);
      if (levelInt<0) levelInt=0;
      if (levelInt>127) levelInt=127;
      op.level=(unsigned char)levelInt;

      // freqRatio (Q8.8): ratio = 2^((69-baseNote)/12)
      double ratio=pow(2.0,((double)69-(double)baseNote)/12.0);
      int rq88=(int)floor(ratio*256.0+0.5);
      if (rq88<1) rq88=1;
      if (rq88>0xFFFF) rq88=0xFFFF;
      op.freqRatio=(unsigned short)rq88;

      // mod_source: high bit of byte 0x1B = "is modulated"; low 7 bits = layer.
      // The layer index is within this voice and matches the Op array index.
      if (fmByte&0x80) {
        op.modSource=(signed char)(fmByte&0x7F);
      } else {
        op.modSource=-1;
      }

      // Feedback detection. Three cases (matches scsp.cpp:computeD7FromOp's
      // packing rules):
      //   - mdxsl==32 && mdysl==32: pure self-feedback (no FM mod). The MDL
      //     in the file is exactly what computeFeedbackMdl produced, so we
      //     can invert it to get the original feedback magnitude.
      //   - mdxsl!=32 && mdysl==32: FM cascade combined with feedback. The
      //     stored MDL is `max(modulator_mdl, fbMdl)` so we can't separate
      //     them — fall back to a moderate default (~0.3).
      //   - otherwise: no feedback.
      if (mdxsl==32 && mdysl==32) {
        op.feedback=tonRecoverFeedback(tl,mdl);
      } else if (mdysl==32) {
        op.feedback=38;
      } else {
        op.feedback=0;
      }

      // Resolve the waveform: import the PCM block as a DivSample, with
      // dedup first by SA (cheap), then by exact PCM content (catches the
      // common case of Bebhionn emitting the same waveform under multiple
      // SAs across different instruments).
      int sampleId=-1;
      std::map<unsigned int,int>::iterator itS=saToSampleId.find(sa);
      if (itS!=saToSampleId.end()) {
        sampleId=itS->second;
      } else {
        unsigned int byteEnd=sa+lea*2;
        if (lea>0 && byteEnd<=fileLen) {
          // Decode 16-bit BE PCM into a content key for dedup lookup.
          std::vector<short> pcm(lea);
          for (unsigned int s=0; s<lea; s++) {
            int hi=data[sa+s*2];
            int lo=data[sa+s*2+1];
            int v=(hi<<8)|lo;
            if (v>=0x8000) v-=0x10000;
            pcm[s]=(short)v;
          }
          std::map<std::vector<short>,int>::iterator itC=contentToSampleId.find(pcm);
          if (itC!=contentToSampleId.end()) {
            sampleId=itC->second;
          } else {
            DivSample* samp=new DivSample;
            samp->name=fmt::sprintf("%s_%02d",stripPath,++tonSampleSeq);
            samp->depth=DIV_SAMPLE_DEPTH_16BIT;
            samp->init(lea);
            samp->centerRate=44100;
            samp->loopStart=(int)lsa;
            samp->loopEnd=(int)lea;
            // TON layers are typically full-period FM waveforms that need
            // to loop continuously when used as an FM carrier or modulator.
            if (lsa<lea) {
              samp->loop=true;
              samp->loopMode=DIV_SAMPLE_LOOP_FORWARD;
            }
            for (unsigned int s=0; s<lea; s++) samp->data16[s]=pcm[s];
            sampleId=(int)song.sample.size();
            song.sample.push_back(samp);
            song.sampleLen=(int)song.sample.size();
            contentToSampleId[pcm]=sampleId;
          }
          saToSampleId[sa]=sampleId;
        }
      }
      op.sampleId=(signed short)sampleId;
      op.lpctlOp=1;
      op.loopStart=(unsigned short)((lsa<=0xFFFF)?lsa:0);
      op.loopEnd=(unsigned short)((lea<=0xFFFF)?lea:0xFFFF);
    }

    ret.push_back(ins);
  }

  // Make sure newly-added samples get rendered into chip RAM the next time
  // the dispatcher runs.
  if (!saToSampleId.empty()) {
    renderSamples();
  }
}
