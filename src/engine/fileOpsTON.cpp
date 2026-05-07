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
// compatible with mid2seq's saturn_kit.py and bebhionn's ton_io.js, both of
// which produce TON files that the SGL sound driver loads via slInitSound.
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
#include "safeWriter.h"

extern "C" {
#include "../../extern/scsp/scsp_waveforms.h"
}

#include <math.h>
#include <vector>
#include <map>
#include <stdint.h>

#define TON_WAVE_LEN 1024

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
  // KRS=0 (Saturn driver default), matches saturn_kit/bebhionn.
  out[0x0C]=((op.dl>>3)&0x3);
  out[0x0D]=((op.dl&0x7)<<5)|(op.rr&0x1F);

  // byte 0x0F: TL — derived from level the same way programSlotFM does
  // (linear-in-level mapping that matches bebhionn TON persistence).
  int tlInt=(int)floor((1.0-(double)op.level/127.0)*128.0+0.5);
  if (tlInt<0) tlInt=0;
  if (tlInt>255) tlInt=255;
  out[0x0F]=(unsigned char)tlInt;

  // byte 0x10: MDL high nibble; MDXSL/MDYSL stay 0 — the Saturn driver
  // computes (modSlot - slot) at runtime from the FM layer link below.
  out[0x10]=(op.mdl&0xF)<<4;

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
  unsigned int waveOffset[SCSP_NUM_BUILTINS];
  bool waveUsed[SCSP_NUM_BUILTINS];
  for (int i=0; i<SCSP_NUM_BUILTINS; i++) { waveOffset[i]=0; waveUsed[i]=false; }

  // sampleIdToOffset: map<original sample index, byte offset in pcmData>.
  // For each op with sampleId>=0 we'll embed the sample's 16-bit BE PCM.
  std::map<int,unsigned int> sampleOffset;  // index -> pcm offset
  std::map<int,unsigned int> sampleLength;  // index -> length in samples

  // First pass: collect references.
  for (int idx: insIndices) {
    DivInstrument* ins=song.ins[idx];
    int n=ins->scsp.opCount;
    if (n>6) n=6;
    for (int op=0; op<n; op++) {
      const DivInstrumentSCSP::Op& opdef=ins->scsp.ops[op];
      if (opdef.sampleId>=0 && opdef.sampleId<song.sampleLen) {
        sampleOffset[opdef.sampleId]=0;  // placeholder, filled below
      } else {
        int w=opdef.waveform;
        if (w>=0 && w<SCSP_NUM_BUILTINS) waveUsed[w]=true;
      }
    }
  }

  std::vector<unsigned char> pcmData;

  // Emit built-ins first.
  for (int w=0; w<SCSP_NUM_BUILTINS; w++) {
    if (!waveUsed[w]) continue;
    waveOffset[w]=(unsigned int)pcmData.size();
    float fbuf[TON_WAVE_LEN];
    scsp_gen_waveform(w,fbuf,TON_WAVE_LEN);
    for (int s=0; s<TON_WAVE_LEN; s++) {
      double v=fbuf[s]*0.9*32767.0;
      if (v<-32768.0) v=-32768.0;
      if (v>32767.0) v=32767.0;
      int16_t iv=(int16_t)floor(v+0.5);
      pcmData.push_back((unsigned char)((iv>>8)&0xFF));
      pcmData.push_back((unsigned char)(iv&0xFF));
    }
  }
  // Then user samples: 16-bit BE PCM. data16 is signed 16-bit native LE
  // in Furnace; we byte-swap into the TON's BE layout as we go.
  for (auto& kv: sampleOffset) {
    int idx=kv.first;
    DivSample* s=song.sample[idx];
    int n=s->getLoopEndPosition(DIV_SAMPLE_DEPTH_16BIT);
    if (n<1) n=1;
    sampleOffset[idx]=(unsigned int)pcmData.size();
    sampleLength[idx]=(unsigned int)n;
    short* src=s->data16;
    for (int si=0; si<n; si++) {
      int16_t iv=src?src[si]:0;
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
      unsigned int sa;
      unsigned int lea;
      if (opdef.sampleId>=0 && sampleOffset.count(opdef.sampleId)) {
        sa=sampleOffset[opdef.sampleId];
        lea=sampleLength[opdef.sampleId];
        if (lea>0xFFFF) lea=0xFFFF;
      } else {
        int w=opdef.waveform;
        if (w<0 || w>=SCSP_NUM_BUILTINS) w=0;
        sa=waveOffset[w];
        lea=TON_WAVE_LEN;
      }
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
