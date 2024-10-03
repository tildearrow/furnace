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

#include "dataErrors.h"
#include "engine.h"
#include "instrument.h"
#include "../ta-log.h"
#include "../fileutils.h"

const DivInstrument defaultIns;

#define _C(x) x==other.x

bool DivInstrumentFM::operator==(const DivInstrumentFM& other) {
  return (
    _C(alg) &&
    _C(fb) &&
    _C(fms) &&
    _C(ams) &&
    _C(fms2) &&
    _C(ams2) &&
    _C(ops) &&
    _C(opllPreset) &&
    _C(fixedDrums) &&
    _C(kickFreq) &&
    _C(snareHatFreq) &&
    _C(tomTopFreq) &&
    _C(op[0]) &&
    _C(op[1]) &&
    _C(op[2]) &&
    _C(op[3])
  );
}

bool DivInstrumentFM::Operator::operator==(const DivInstrumentFM::Operator& other) {
  return (
    _C(enable) &&
    _C(am) &&
    _C(ar) &&
    _C(dr) &&
    _C(mult) &&
    _C(rr) &&
    _C(sl) &&
    _C(tl) &&
    _C(dt2) &&
    _C(rs) &&
    _C(dt) &&
    _C(d2r) &&
    _C(ssgEnv) &&
    _C(dam) &&
    _C(dvb) &&
    _C(egt) &&
    _C(ksl) &&
    _C(sus) &&
    _C(vib) &&
    _C(ws) &&
    _C(ksr) &&
    _C(kvs)
  );
}

bool DivInstrumentGB::operator==(const DivInstrumentGB& other) {
  return (
    _C(envVol) &&
    _C(envDir) &&
    _C(envLen) &&
    _C(soundLen) &&
    _C(hwSeqLen) &&
    _C(softEnv) &&
    _C(alwaysInit) &&
    _C(doubleWave)
  );
}

bool DivInstrumentC64::operator==(const DivInstrumentC64& other) {
  return (
    _C(triOn) &&
    _C(sawOn) &&
    _C(pulseOn) &&
    _C(noiseOn) &&
    _C(a) &&
    _C(d) &&
    _C(s) &&
    _C(r) &&
    _C(duty) &&
    _C(ringMod) &&
    _C(oscSync) &&
    _C(toFilter) &&
    _C(initFilter) &&
    _C(dutyIsAbs) &&
    _C(filterIsAbs) &&
    _C(noTest) &&
    _C(resetDuty) &&
    _C(res) &&
    _C(cut) &&
    _C(hp) &&
    _C(lp) &&
    _C(bp) &&
    _C(ch3off)
  );
}

bool DivInstrumentAmiga::operator==(const DivInstrumentAmiga& other) {
  return (
    _C(initSample) &&
    _C(useNoteMap) &&
    _C(useSample) &&
    _C(useWave) &&
    _C(waveLen)
  );
}

bool DivInstrumentX1_010::operator==(const DivInstrumentX1_010& other) {
  return _C(bankSlot);
}

bool DivInstrumentN163::operator==(const DivInstrumentN163& other) {
  return (
    _C(wave) &&
    _C(wavePos) &&
    _C(waveLen) &&
    _C(waveMode) &&
    _C(perChanPos) &&
    _C(wavePosCh[0]) &&
    _C(wavePosCh[1]) &&
    _C(wavePosCh[2]) &&
    _C(wavePosCh[3]) &&
    _C(wavePosCh[4]) &&
    _C(wavePosCh[5]) &&
    _C(wavePosCh[6]) &&
    _C(wavePosCh[7]) &&
    _C(waveLenCh[0]) &&
    _C(waveLenCh[1]) &&
    _C(waveLenCh[2]) &&
    _C(waveLenCh[3]) &&
    _C(waveLenCh[4]) &&
    _C(waveLenCh[5]) &&
    _C(waveLenCh[6]) &&
    _C(waveLenCh[7])
  );
}

bool DivInstrumentFDS::operator==(const DivInstrumentFDS& other) {
  return (
    (memcmp(modTable,other.modTable,32)==0) &&
    _C(modSpeed) &&
    _C(modDepth) &&
    _C(initModTableWithFirstWave)
  );
}

bool DivInstrumentMultiPCM::operator==(const DivInstrumentMultiPCM& other) {
  return (
    _C(ar) &&
    _C(d1r) &&
    _C(dl) &&
    _C(d2r) &&
    _C(rr) &&
    _C(rc) &&
    _C(lfo) &&
    _C(vib) &&
    _C(am) &&
    _C(damp) &&
    _C(pseudoReverb) &&
    _C(lfoReset) &&
    _C(levelDirect)
  );
}

bool DivInstrumentWaveSynth::operator==(const DivInstrumentWaveSynth& other) {
  return (
    _C(wave1) &&
    _C(wave2) &&
    _C(rateDivider) &&
    _C(effect) &&
    _C(oneShot) &&
    _C(enabled) &&
    _C(global) &&
    _C(speed) &&
    _C(param1) &&
    _C(param2) &&
    _C(param3) &&
    _C(param4)
  );
}

bool DivInstrumentSoundUnit::operator==(const DivInstrumentSoundUnit& other) {
  return (
    _C(switchRoles) &&
    _C(hwSeqLen)
  );
}

bool DivInstrumentES5506::operator==(const DivInstrumentES5506& other) {
  return (
    _C(filter.mode) &&
    _C(filter.k1) &&
    _C(filter.k2) &&
    _C(envelope.ecount) &&
    _C(envelope.lVRamp) &&
    _C(envelope.rVRamp) &&
    _C(envelope.k1Ramp) &&
    _C(envelope.k2Ramp) &&
    _C(envelope.k1Slow) &&
    _C(envelope.k2Slow)
  );
}

bool DivInstrumentSNES::operator==(const DivInstrumentSNES& other) {
  return (
    _C(useEnv) &&
    _C(sus) &&
    _C(gainMode) &&
    _C(gain) &&
    _C(a) &&
    _C(d) &&
    _C(s) &&
    _C(r) &&
    _C(d2)
  );
}

bool DivInstrumentESFM::operator==(const DivInstrumentESFM& other) {
  return (
    _C(noise) &&
    _C(op[0]) &&
    _C(op[1]) &&
    _C(op[2]) &&
    _C(op[3])
  );
}

bool DivInstrumentESFM::Operator::operator==(const DivInstrumentESFM::Operator& other) {
  return (
    _C(delay) &&
    _C(outLvl) &&
    _C(modIn) &&
    _C(left) &&
    _C(right) &&
    _C(fixed) &&
    _C(ct) &&
    _C(dt)
  );
}

bool DivInstrumentSID3::operator==(const DivInstrumentSID3& other) {
  return (
    _C(triOn) &&
    _C(sawOn) &&
    _C(pulseOn) &&
    _C(noiseOn) &&
    _C(a) &&
    _C(d) &&
    _C(s) &&
    _C(r) &&
    _C(sr) &&
    _C(duty) &&
    _C(ringMod) &&
    _C(oscSync) &&
    _C(phase_mod) &&
    _C(phase_mod_source) &&
    _C(ring_mod_source) &&
    _C(sync_source) &&
    _C(specialWaveOn) &&
    _C(oneBitNoise) &&
    _C(separateNoisePitch) &&
    _C(special_wave) &&
    _C(doWavetable) &&
    _C(dutyIsAbs) &&
    _C(resetDuty) &&
    _C(phaseInv) &&
    _C(feedback) &&
    _C(mixMode) &&
    _C(filt[0]) &&
    _C(filt[1]) &&
    _C(filt[2]) &&
    _C(filt[3])
  );
}

bool DivInstrumentSID3::Filter::operator==(const DivInstrumentSID3::Filter& other) {
  return (
    _C(cutoff) &&
    _C(resonance) &&
    _C(output_volume) &&
    _C(distortion_level) &&
    _C(mode) &&
    _C(enabled) &&
    _C(init) &&
    _C(filter_matrix) &&
    _C(absoluteCutoff) &&
    _C(bindCutoffToNote) &&
    _C(bindCutoffToNoteStrength) &&
    _C(bindCutoffToNoteCenter) &&
    _C(bindCutoffToNoteDir) &&
    _C(bindCutoffOnNote) &&
    _C(bindResonanceToNote) &&
    _C(bindResonanceToNoteStrength) &&
    _C(bindResonanceToNoteCenter) &&
    _C(bindResonanceToNoteDir) &&
    _C(bindResonanceOnNote)
  );
}

bool DivInstrumentPowerNoise::operator==(const DivInstrumentPowerNoise& other) {
  return _C(octave);
}

bool DivInstrumentSID2::operator==(const DivInstrumentSID2& other) {
  return (
    _C(volume) &&
    _C(mixMode) &&
    _C(noiseMode)
  );
}

#undef _C

#define CONSIDER(x,t) \
  case t: \
    return &x; \
    break;

DivInstrumentMacro* DivInstrumentSTD::macroByType(DivMacroType type) {
  switch (type) {
    CONSIDER(volMacro,DIV_MACRO_VOL)
    CONSIDER(arpMacro,DIV_MACRO_ARP)
    CONSIDER(dutyMacro,DIV_MACRO_DUTY)
    CONSIDER(waveMacro,DIV_MACRO_WAVE)
    CONSIDER(pitchMacro,DIV_MACRO_PITCH)
    CONSIDER(ex1Macro,DIV_MACRO_EX1)
    CONSIDER(ex2Macro,DIV_MACRO_EX2)
    CONSIDER(ex3Macro,DIV_MACRO_EX3)
    CONSIDER(algMacro,DIV_MACRO_ALG)
    CONSIDER(fbMacro,DIV_MACRO_FB)
    CONSIDER(fmsMacro,DIV_MACRO_FMS)
    CONSIDER(amsMacro,DIV_MACRO_AMS)
    CONSIDER(panLMacro,DIV_MACRO_PAN_LEFT)
    CONSIDER(panRMacro,DIV_MACRO_PAN_RIGHT)
    CONSIDER(phaseResetMacro,DIV_MACRO_PHASE_RESET)
    CONSIDER(ex4Macro,DIV_MACRO_EX4)
    CONSIDER(ex5Macro,DIV_MACRO_EX5)
    CONSIDER(ex6Macro,DIV_MACRO_EX6)
    CONSIDER(ex7Macro,DIV_MACRO_EX7)
    CONSIDER(ex8Macro,DIV_MACRO_EX8)
  }

  return NULL;
}

#undef CONSIDER

#define FEATURE_BEGIN(x) \
  w->write(x,2); \
  size_t featStartSeek=w->tell(); \
  w->writeS(0);

#define FEATURE_END \
  size_t featEndSeek=w->tell(); \
  w->seek(featStartSeek,SEEK_SET); \
  w->writeS(featEndSeek-featStartSeek-2); \
  w->seek(featEndSeek,SEEK_SET);

void DivInstrument::writeFeatureNA(SafeWriter* w) {
  FEATURE_BEGIN("NA");

  w->writeString(name,false);

  FEATURE_END;
}

void DivInstrument::writeFeatureFM(SafeWriter* w, bool fui) {
  FEATURE_BEGIN("FM");

  int opCount=4;
  if (fui) {
    if (type==DIV_INS_OPLL) {
      opCount=2;
    } else if (type==DIV_INS_OPL) {
      opCount=(fm.ops==4)?4:2;
    }
  }
  
  w->writeC(
    (fm.op[3].enable?128:0)|
    (fm.op[2].enable?64:0)|
    (fm.op[1].enable?32:0)|
    (fm.op[0].enable?16:0)|
    opCount
  );

  // base data
  w->writeC(((fm.alg&7)<<4)|(fm.fb&7));
  w->writeC(((fm.fms2&7)<<5)|((fm.ams&3)<<3)|(fm.fms&7));
  w->writeC(((fm.ams2&3)<<6)|((fm.ops==4)?32:0)|(fm.opllPreset&31));

  // operator data
  for (int i=0; i<opCount; i++) {
    DivInstrumentFM::Operator& op=fm.op[i];

    w->writeC((op.ksr?128:0)|((op.dt&7)<<4)|(op.mult&15));
    w->writeC((op.sus?128:0)|(op.tl&127));
    w->writeC(((op.rs&3)<<6)|(op.vib?32:0)|(op.ar&31));
    w->writeC((op.am?128:0)|((op.ksl&3)<<5)|(op.dr&31));
    w->writeC((op.egt?128:0)|((op.kvs&3)<<5)|(op.d2r&31));
    w->writeC(((op.sl&15)<<4)|(op.rr&15));
    w->writeC(((op.dvb&15)<<4)|(op.ssgEnv&15));
    w->writeC(((op.dam&7)<<5)|((op.dt2&3)<<3)|(op.ws&7));
  }

  FEATURE_END;
}

bool MemPatch::calcDiff(const void* pre, const void* post, size_t inputSize) {
  bool diffValid=false;
  size_t firstDiff=0;
  size_t lastDiff=0;
  const unsigned char* preBytes=(const unsigned char*)pre;
  const unsigned char* postBytes=(const unsigned char*)post;

  // @NOTE: consider/profile using a memcmp==0 check to early-out, if it's potentially faster
  // for the common case, which is no change
  for (size_t ii=0; ii<inputSize; ++ii) {
    if (preBytes[ii] != postBytes[ii]) {
      lastDiff=ii;
      firstDiff=diffValid ? firstDiff : ii;
      diffValid=true;
    }
  }

  if (diffValid) {
    offset=firstDiff;
    size=lastDiff - firstDiff + 1;
    data=new unsigned char[size];

    // the diff is to make pre into post (MemPatch is general, not specific to
    // undo), so copy from postBytes
    memcpy(data, postBytes+offset, size);
  }

  return diffValid;
}

void MemPatch::applyAndReverse(void* target, size_t targetSize) {
  if (size==0) return;
  if (offset+size>targetSize) {
    logW("MemPatch (offset %d, size %d) exceeds target size (%d), can't apply!",offset,size,targetSize);
    return;
  }

  unsigned char* targetBytes=(unsigned char*)target;

  // swap this->data and its segment on target
  for (size_t ii=0; ii<size; ++ii) {
    unsigned char tmp=targetBytes[offset+ii];
    targetBytes[offset+ii] = data[ii];
    data[ii] = tmp;
  }
}

void DivInstrumentUndoStep::applyAndReverse(DivInstrument* target) {
  if (nameValid) {
    name.swap(target->name);
  }
  podPatch.applyAndReverse((DivInstrumentPOD*)target, sizeof(DivInstrumentPOD));
}

bool DivInstrumentUndoStep::makeUndoPatch(size_t processTime_, const DivInstrument* pre, const DivInstrument* post) {
  processTime=processTime_;

  // create the patch that will make post into pre
  podPatch.calcDiff((const DivInstrumentPOD*)post, (const DivInstrumentPOD*)pre, sizeof(DivInstrumentPOD));
  if (pre->name!=post->name) {
    nameValid=true;
    name=pre->name;
  }

  return nameValid || podPatch.isValid();
}

bool DivInstrument::recordUndoStepIfChanged(size_t processTime, const DivInstrument* old) {
  DivInstrumentUndoStep step;

  // generate a patch to go back to old
  if (step.makeUndoPatch(processTime, old, this)) {
    
      // make room
    if (undoHist.size()>=undoHist.capacity()) {
      delete undoHist.front();
      undoHist.pop_front();
    }

    // clear redo
    while (!redoHist.empty()) {
      delete redoHist.back();
      redoHist.pop_back();
    }

    DivInstrumentUndoStep* stepPtr=new DivInstrumentUndoStep;
    *stepPtr=step;
    step.podPatch.data=NULL; // don't let it delete the data ptr that's been copied!
    undoHist.push_back(stepPtr);

    // logI("DivInstrument::undoHist push (%u off, %u size)", stepPtr->podPatch.offset, stepPtr->podPatch.size);
    return true;
  }

  return false;
}

int DivInstrument::undo() {
  if (undoHist.empty()) return 0;

  DivInstrumentUndoStep* step=undoHist.back();
  undoHist.pop_back();
  // logI("DivInstrument::undo (%u off, %u size)", step->podPatch.offset, step->podPatch.size);
  step->applyAndReverse(this);

  // make room
  if (redoHist.size()>=redoHist.capacity()) {
      DivInstrumentUndoStep* step=redoHist.front();
      delete step;
      redoHist.pop_front();
  }
  redoHist.push_back(step);

  return 1;
}

int DivInstrument::redo() {
  if (redoHist.empty()) return 0;

  DivInstrumentUndoStep* step = redoHist.back();
  redoHist.pop_back();
  // logI("DivInstrument::redo (%u off, %u size)", step->podPatch.offset, step->podPatch.size);
  step->applyAndReverse(this);

  // make room
  if (undoHist.size()>=undoHist.capacity()) {
      DivInstrumentUndoStep* step=undoHist.front();
      delete step;
      undoHist.pop_front();
  }
  undoHist.push_back(step);

  return 1;
}

void DivInstrument::writeMacro(SafeWriter* w, const DivInstrumentMacro& m) {
  if (!m.len) return;

  // determine word size
  int macroMin=0x7fffffff;
  int macroMax=0x80000000;
  for (int i=0; i<m.len; i++) {
    if (m.val[i]<macroMin) macroMin=m.val[i];
    if (m.val[i]>macroMax) macroMax=m.val[i];
  }

  unsigned char wordSize=192; // 32-bit

  if (macroMin>=0 && macroMax<=255) {
    wordSize=0; // 8-bit unsigned
  } else if (macroMin>=-128 && macroMax<=127) {
    wordSize=64; // 8-bit signed
  } else if (macroMin>=-32768 && macroMax<=32767) {
    wordSize=128; // 16-bit signed
  } else {
    wordSize=192; // 32-bit signed
  }

  w->writeC(m.macroType&31);
  w->writeC(m.len);
  w->writeC(m.loop);
  w->writeC(m.rel);
  w->writeC(m.mode);
  w->writeC((m.open&0x3f)|wordSize);
  w->writeC(m.delay);
  w->writeC(m.speed);

  switch (wordSize) {
    case 0:
      for (int i=0; i<m.len; i++) {
        w->writeC((unsigned char)m.val[i]);
      }
      break;
    case 64:
      for (int i=0; i<m.len; i++) {
        w->writeC((signed char)m.val[i]);
      }
      break;
    case 128:
      for (int i=0; i<m.len; i++) {
        w->writeS((short)m.val[i]);
      }
      break;
    default: // 192
      for (int i=0; i<m.len; i++) {
        w->writeI(m.val[i]);
      }
      break;
  }
}

void DivInstrument::writeFeatureMA(SafeWriter* w) {
  FEATURE_BEGIN("MA");

  // if you update the macro header, please update this value as well.
  // it's the length.
  w->writeS(8);
  
  // write macros
  writeMacro(w,std.volMacro);
  writeMacro(w,std.arpMacro);
  writeMacro(w,std.dutyMacro);
  writeMacro(w,std.waveMacro);
  writeMacro(w,std.pitchMacro);
  writeMacro(w,std.ex1Macro);
  writeMacro(w,std.ex2Macro);
  writeMacro(w,std.ex3Macro);
  writeMacro(w,std.algMacro);
  writeMacro(w,std.fbMacro);
  writeMacro(w,std.fmsMacro);
  writeMacro(w,std.amsMacro);
  writeMacro(w,std.panLMacro);
  writeMacro(w,std.panRMacro);
  writeMacro(w,std.phaseResetMacro);
  writeMacro(w,std.ex4Macro);
  writeMacro(w,std.ex5Macro);
  writeMacro(w,std.ex6Macro);
  writeMacro(w,std.ex7Macro);
  writeMacro(w,std.ex8Macro);

  // "stop reading" code
  w->writeC(-1);

  FEATURE_END;
}

void DivInstrument::writeFeature64(SafeWriter* w) {
  FEATURE_BEGIN("64");

  w->writeC(
    (c64.dutyIsAbs?0x80:0)|
    (c64.initFilter?0x40:0)|
    (c64.toFilter?0x10:0)|
    (c64.noiseOn?8:0)|
    (c64.pulseOn?4:0)|
    (c64.sawOn?2:0)|
    (c64.triOn?1:0)
  );

  w->writeC(
    (c64.oscSync?0x80:0)|
    (c64.ringMod?0x40:0)|
    (c64.noTest?0x20:0)|
    (c64.filterIsAbs?0x10:0)|
    (c64.ch3off?8:0)|
    (c64.bp?4:0)|
    (c64.hp?2:0)|
    (c64.lp?1:0)
  );

  w->writeC(((c64.a&15)<<4)|(c64.d&15));
  w->writeC(((c64.s&15)<<4)|(c64.r&15));
  w->writeS(c64.duty);
  w->writeS((unsigned short)((c64.cut&4095)|((c64.res&15)<<12)));

  w->writeC(((c64.res>>4)&15)|(c64.resetDuty?0x10:0));

  FEATURE_END;
}

void DivInstrument::writeFeatureGB(SafeWriter* w) {
  FEATURE_BEGIN("GB");

  w->writeC(((gb.envLen&7)<<5)|(gb.envDir?16:0)|(gb.envVol&15));
  w->writeC(gb.soundLen);

  w->writeC(
    (gb.doubleWave?4:0)|
    (gb.alwaysInit?2:0)|
    (gb.softEnv?1:0)
  );

  w->writeC(gb.hwSeqLen);
  for (int i=0; i<gb.hwSeqLen; i++) {
    w->writeC(gb.hwSeq[i].cmd);
    w->writeS(gb.hwSeq[i].data);
  }

  FEATURE_END;
}

void DivInstrument::writeFeatureSM(SafeWriter* w) {
  FEATURE_BEGIN("SM");

  w->writeS(amiga.initSample);
  w->writeC(
    (amiga.useWave?4:0)|
    (amiga.useSample?2:0)|
    (amiga.useNoteMap?1:0)
  );
  w->writeC(amiga.waveLen);

  if (amiga.useNoteMap) {
    for (int note=0; note<120; note++) {
      w->writeS(amiga.noteMap[note].freq);
      w->writeS(amiga.noteMap[note].map);
    }
  }

  FEATURE_END;
}

void DivInstrument::writeFeatureOx(SafeWriter* w, int ope) {
  char opCode[3];
  opCode[0]='O';
  opCode[1]='1'+ope;
  opCode[2]=0;

  FEATURE_BEGIN(opCode);

  // if you update the macro header, please update this value as well.
  // it's the length.
  w->writeS(8);
  
  // write macros
  const DivInstrumentSTD::OpMacro& o=std.opMacros[ope];

  writeMacro(w,o.amMacro);
  writeMacro(w,o.arMacro);
  writeMacro(w,o.drMacro);
  writeMacro(w,o.multMacro);
  writeMacro(w,o.rrMacro);
  writeMacro(w,o.slMacro);
  writeMacro(w,o.tlMacro);
  writeMacro(w,o.dt2Macro);
  writeMacro(w,o.rsMacro);
  writeMacro(w,o.dtMacro);
  writeMacro(w,o.d2rMacro);
  writeMacro(w,o.ssgMacro);
  writeMacro(w,o.damMacro);
  writeMacro(w,o.dvbMacro);
  writeMacro(w,o.egtMacro);
  writeMacro(w,o.kslMacro);
  writeMacro(w,o.susMacro);
  writeMacro(w,o.vibMacro);
  writeMacro(w,o.wsMacro);
  writeMacro(w,o.ksrMacro);

  // "stop reading" code
  w->writeC(-1);

  FEATURE_END;
}

void DivInstrument::writeFeatureLD(SafeWriter* w) {
  FEATURE_BEGIN("LD");

  w->writeC(fm.fixedDrums);
  w->writeS(fm.kickFreq);
  w->writeS(fm.snareHatFreq);
  w->writeS(fm.tomTopFreq);

  FEATURE_END;
}

void DivInstrument::writeFeatureSN(SafeWriter* w) {
  FEATURE_BEGIN("SN");

  w->writeC(((snes.d&7)<<4)|(snes.a&15));
  w->writeC(((snes.s&7)<<5)|(snes.r&31));

  w->writeC(
    (snes.useEnv?16:0)|
    (snes.sus?8:0)|
    (snes.gainMode)
  );
  
  w->writeC(snes.gain);

  w->writeC(((snes.sus&3)<<5)|(snes.d2&31));

  FEATURE_END;
}

void DivInstrument::writeFeatureN1(SafeWriter* w) {
  FEATURE_BEGIN("N1");

  w->writeI(n163.wave);
  w->writeC(n163.wavePos);
  w->writeC(n163.waveLen);
  w->writeC(n163.waveMode);

  w->writeC(n163.perChanPos);

  if (n163.perChanPos) {
    for (int i=0; i<8; i++) {
      w->writeC(n163.wavePosCh[i]);
    }
    for (int i=0; i<8; i++) {
      w->writeC(n163.waveLenCh[i]);
    }
  }

  FEATURE_END;
}

void DivInstrument::writeFeatureFD(SafeWriter* w) {
  FEATURE_BEGIN("FD");

  w->writeI(fds.modSpeed);
  w->writeI(fds.modDepth);
  w->writeC(fds.initModTableWithFirstWave);
  w->write(fds.modTable,32);

  FEATURE_END;
}

void DivInstrument::writeFeatureWS(SafeWriter* w) {
  FEATURE_BEGIN("WS");

  w->writeI(ws.wave1);
  w->writeI(ws.wave2);
  w->writeC(ws.rateDivider);
  w->writeC(ws.effect);
  w->writeC(ws.enabled);
  w->writeC(ws.global);
  w->writeC(ws.speed);
  w->writeC(ws.param1);
  w->writeC(ws.param2);
  w->writeC(ws.param3);
  w->writeC(ws.param4);

  FEATURE_END;
}

size_t DivInstrument::writeFeatureSL(SafeWriter* w, std::vector<int>& list, const DivSong* song) {
  bool sampleUsed[256];
  memset(sampleUsed,0,256*sizeof(bool));

  if (amiga.initSample>=0 && amiga.initSample<(int)song->sample.size()) {
    sampleUsed[amiga.initSample]=true;
  }

  if (amiga.useNoteMap) {
    for (int i=0; i<120; i++) {
      if (amiga.noteMap[i].map>=0 && amiga.noteMap[i].map<(int)song->sample.size()) {
        sampleUsed[amiga.noteMap[i].map]=true;
      }
    }
  }

  for (size_t i=0; i<song->sample.size(); i++) {
    if (sampleUsed[i]) {
      list.push_back(i);
    }
  }

  if (list.empty()) return 0;

  FEATURE_BEGIN("SL");

  w->writeC(list.size());

  for (int i: list) {
    w->writeC(i);
  }

  size_t ret=w->tell();

  // pointers (these will be filled later)
  for (size_t i=0; i<list.size(); i++) {
    w->writeI(0);
  }

  FEATURE_END;

  return ret;
}

size_t DivInstrument::writeFeatureWL(SafeWriter* w, std::vector<int>& list, const DivSong* song) {
  bool waveUsed[256];
  memset(waveUsed,0,256*sizeof(bool));

  for (int i=0; i<std.waveMacro.len; i++) {
    if (std.waveMacro.val[i]>=0 && std.waveMacro.val[i]<(int)song->wave.size()) {
      waveUsed[std.waveMacro.val[i]]=true;
    }
  }

  if (ws.enabled) {
    if (ws.wave1>=0 && ws.wave1<(int)song->wave.size()) {
      waveUsed[ws.wave1]=true;
    }
    if ((ws.effect&0x80) && ws.wave2>=0 && ws.wave2<(int)song->wave.size()) {
      waveUsed[ws.wave2]=true;
    }
  }

  for (size_t i=0; i<song->wave.size(); i++) {
    if (waveUsed[i]) {
      list.push_back(i);
    }
  }

  if (list.empty()) return 0;

  FEATURE_BEGIN("WL");

  w->writeC(list.size());

  for (int i: list) {
    w->writeC(i);
  }

  size_t ret=w->tell();

  // pointers (these will be filled later)
  for (size_t i=0; i<list.size(); i++) {
    w->writeI(0);
  }

  FEATURE_END;

  return ret;
}

void DivInstrument::writeFeatureMP(SafeWriter* w) {
  FEATURE_BEGIN("MP");

  w->writeC(multipcm.ar);
  w->writeC(multipcm.d1r);
  w->writeC(multipcm.dl);
  w->writeC(multipcm.d2r);
  w->writeC(multipcm.rr);
  w->writeC(multipcm.rc);
  w->writeC(multipcm.lfo);
  w->writeC(multipcm.vib);
  w->writeC(multipcm.am);

  unsigned char next=(
    (multipcm.damp?1:0)&
    (multipcm.pseudoReverb?2:0)&
    (multipcm.lfoReset?4:0)&
    (multipcm.levelDirect?8:0)
  );
  w->writeC(next);

  FEATURE_END;
}

void DivInstrument::writeFeatureSU(SafeWriter* w) {
  FEATURE_BEGIN("SU");

  w->writeC(su.switchRoles);

  w->writeC(su.hwSeqLen);
  for (int i=0; i<su.hwSeqLen; i++) {
    w->writeC(su.hwSeq[i].cmd);
    w->writeC(su.hwSeq[i].bound);
    w->writeC(su.hwSeq[i].val);
    w->writeS(su.hwSeq[i].speed);
  }

  FEATURE_END;
}

void DivInstrument::writeFeatureES(SafeWriter* w) {
  FEATURE_BEGIN("ES");

  w->writeC(es5506.filter.mode);
  w->writeS(es5506.filter.k1);
  w->writeS(es5506.filter.k2);
  w->writeS(es5506.envelope.ecount);
  w->writeC(es5506.envelope.lVRamp);
  w->writeC(es5506.envelope.rVRamp);
  w->writeC(es5506.envelope.k1Ramp);
  w->writeC(es5506.envelope.k2Ramp);
  w->writeC(es5506.envelope.k1Slow);
  w->writeC(es5506.envelope.k2Slow);

  FEATURE_END;
}

void DivInstrument::writeFeatureX1(SafeWriter* w) {
  FEATURE_BEGIN("X1");

  w->writeI(x1_010.bankSlot);

  FEATURE_END;
}

void DivInstrument::writeFeatureNE(SafeWriter* w) {
  FEATURE_BEGIN("NE");

  w->writeC(amiga.useNoteMap?1:0);

  if (amiga.useNoteMap) {
    for (int note=0; note<120; note++) {
      w->writeC(amiga.noteMap[note].dpcmFreq);
      w->writeC(amiga.noteMap[note].dpcmDelta);
    }
  }

  FEATURE_END;
}

void DivInstrument::writeFeatureEF(SafeWriter* w) {
  FEATURE_BEGIN("EF");

  w->writeC(esfm.noise&3);
  for (int i=0; i<4; i++) {
    DivInstrumentESFM::Operator& op=esfm.op[i];

    w->writeC(((op.delay&7)<<5)|((op.outLvl&7)<<2)|((op.right&1)<<1)|(op.left&1));
    w->writeC((op.fixed&1)<<3|(op.modIn&7));
    w->writeC(op.ct);
    w->writeC(op.dt);
  }

  FEATURE_END;
}

void DivInstrument::writeFeaturePN(SafeWriter* w) {
  FEATURE_BEGIN("PN");

  w->writeC(powernoise.octave);

  FEATURE_END;
}

void DivInstrument::writeFeatureS2(SafeWriter* w) {
  FEATURE_BEGIN("S2");

  w->writeC((sid2.volume&15)|((sid2.mixMode&3)<<4)|((sid2.noiseMode&3)<<6));

  FEATURE_END;
}

void DivInstrument::writeFeatureS3(SafeWriter* w) {
  FEATURE_BEGIN("S3");

  w->writeC(
    (sid3.dutyIsAbs?0x80:0)|
    (sid3.noiseOn?8:0)|
    (sid3.pulseOn?4:0)|
    (sid3.sawOn?2:0)|
    (sid3.triOn?1:0)
  );

  w->writeC(sid3.a);
  w->writeC(sid3.d);
  w->writeC(sid3.s);
  w->writeC(sid3.sr);
  w->writeC(sid3.r);

  w->writeC(sid3.mixMode);

  w->writeS(sid3.duty);

  w->writeC(
    (sid3.phase_mod?0x80:0)|
    (sid3.specialWaveOn?0x40:0)|
    (sid3.oneBitNoise?0x20:0)|
    (sid3.separateNoisePitch?0x10:0)|
    (sid3.doWavetable?8:0)|
    (sid3.resetDuty?4:0)|
    (sid3.oscSync?2:0)|
    (sid3.ringMod?1:0)
  );

  w->writeC(sid3.phase_mod_source);
  w->writeC(sid3.ring_mod_source);
  w->writeC(sid3.sync_source);
  w->writeC(sid3.special_wave);
  w->writeC(sid3.phaseInv);
  w->writeC(sid3.feedback);

  w->writeC(4); // number of filters

  for (int i=0; i<4; i++) {
    w->writeC(
      (sid3.filt[i].enabled?0x80:0)|
      (sid3.filt[i].init?0x40:0)|
      (sid3.filt[i].absoluteCutoff?0x20:0)|
      (sid3.filt[i].bindCutoffToNote?0x10:0)|
      (sid3.filt[i].bindCutoffToNoteDir?8:0)|
      (sid3.filt[i].bindCutoffOnNote?4:0)|
      (sid3.filt[i].bindResonanceToNote?2:0)|
      (sid3.filt[i].bindResonanceToNoteDir?1:0)
    );

    w->writeC(
      (sid3.filt[i].bindResonanceOnNote?0x80:0)
    );

    w->writeS(sid3.filt[i].cutoff);

    w->writeC(sid3.filt[i].resonance);
    w->writeC(sid3.filt[i].output_volume);
    w->writeC(sid3.filt[i].distortion_level);
    w->writeC(sid3.filt[i].mode);
    w->writeC(sid3.filt[i].filter_matrix);

    w->writeC(sid3.filt[i].bindCutoffToNoteStrength);
    w->writeC(sid3.filt[i].bindCutoffToNoteCenter);
    w->writeC(sid3.filt[i].bindResonanceToNoteStrength);
    w->writeC(sid3.filt[i].bindResonanceToNoteCenter);
  }

  FEATURE_END;
}

void DivInstrument::putInsData2(SafeWriter* w, bool fui, const DivSong* song, bool insName) {
  size_t blockStartSeek=0;
  size_t blockEndSeek=0;
  size_t slSeek=0;
  size_t wlSeek=0;
  std::vector<int> waveList;
  std::vector<int> sampleList;

  std::vector<unsigned int> wavePtr;
  std::vector<unsigned int> samplePtr;

  if (fui) {
    w->write("FINS",4);
  } else {
    w->write("INS2",4);
    blockStartSeek=w->tell();
    w->writeI(0);
  }

  w->writeS(DIV_ENGINE_VERSION);
  w->writeC(type);
  w->writeC(0);

  // write features
  bool featureNA=false;
  bool featureFM=false;
  bool featureMA=false;
  bool feature64=false;
  bool featureGB=false;
  bool featureSM=false;
  bool featureOx[4];
  bool featureLD=false;
  bool featureSN=false;
  bool featureN1=false;
  bool featureFD=false;
  bool featureWS=false;
  bool featureSL=false;
  bool featureWL=false;
  bool featureMP=false;
  bool featureSU=false;
  bool featureES=false;
  bool featureX1=false;
  bool featureNE=false;
  bool featureEF=false;
  bool featurePN=false;
  bool featureS2=false;
  bool featureS3=false;

  bool checkForWL=false;

  featureOx[0]=false;
  featureOx[1]=false;
  featureOx[2]=false;
  featureOx[3]=false;

  // turn on base features if .fui
  if (fui) {
    switch (type) {
      case DIV_INS_STD:
        break;
      case DIV_INS_FM:
        featureFM=true;
        break;
      case DIV_INS_GB:
        featureGB=true;
        checkForWL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_C64:
        feature64=true;
        break;
      case DIV_INS_AMIGA:
        featureSM=true;
        if (!amiga.useWave) featureSL=true;
        break;
      case DIV_INS_PCE:
        checkForWL=true;
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_AY:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        break;
      case DIV_INS_AY8930:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        break;
      case DIV_INS_TIA:
        break;
      case DIV_INS_SAA1099:
        break;
      case DIV_INS_VIC:
        break;
      case DIV_INS_PET:
        break;
      case DIV_INS_VRC6:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        break;
      case DIV_INS_OPLL:
        featureFM=true;
        if (fm.fixedDrums) featureLD=true;
        break;
      case DIV_INS_OPL:
        featureFM=true;
        if (fm.fixedDrums) featureLD=true;
        break;
      case DIV_INS_FDS:
        checkForWL=true;
        featureFD=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_VBOY:
        checkForWL=true;
        featureFD=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_N163:
        checkForWL=true;
        featureN1=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_SCC:
        checkForWL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_OPZ:
        featureFM=true;
        break;
      case DIV_INS_POKEY:
        break;
      case DIV_INS_BEEPER:
        break;
      case DIV_INS_SWAN:
        checkForWL=true;
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_MIKEY:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        break;
      case DIV_INS_VERA:
        break;
      case DIV_INS_X1_010:
        checkForWL=true;
        featureX1=true;
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_VRC6_SAW:
        break;
      case DIV_INS_ES5506:
        featureSM=true;
        featureSL=true;
        featureES=true;
        break;
      case DIV_INS_MULTIPCM:
        featureSM=true;
        featureSL=true;
        featureMP=true;
        break;
      case DIV_INS_SNES:
        featureSM=true;
        if (!amiga.useWave) featureSL=true;
        featureSN=true;
        checkForWL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_SU:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        featureSU=true;
        break;
      case DIV_INS_NAMCO:
        checkForWL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_OPL_DRUMS:
        featureFM=true;
        if (fm.fixedDrums) featureLD=true;
        break;
      case DIV_INS_OPM:
        featureFM=true;
        break;
      case DIV_INS_NES:
        featureSM=true;
        featureNE=true;
        featureSL=true;
        break;
      case DIV_INS_MSM6258:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_MSM6295:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_ADPCMA:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_ADPCMB:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_SEGAPCM:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_QSOUND:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_YMZ280B:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_RF5C68:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_MSM5232:
        break;
      case DIV_INS_T6W28:
        break;
      case DIV_INS_K007232:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_GA20:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_POKEMINI:
        break;
      case DIV_INS_SM8521:
        checkForWL=true;
        if (ws.enabled) featureWS=true;
        break;
      case DIV_INS_PV1000:
        break;
      case DIV_INS_K053260:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_TED:
        break;
      case DIV_INS_C140:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_C219:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_ESFM:
        featureFM=true;
        featureEF=true;
        break;
      case DIV_INS_POWERNOISE:
        featurePN=true;
        break;
      case DIV_INS_POWERNOISE_SLOPE:
        featurePN=true;
        break;
      case DIV_INS_DAVE:
        break;
      case DIV_INS_NDS:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        break;
      case DIV_INS_GBA_DMA:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_GBA_MINMOD:
        featureSM=true;
        featureSL=true;
        break;
      case DIV_INS_BIFURCATOR:
        break;
      case DIV_INS_SID2:
        feature64=true;
        featureS2=true;
        break;
      case DIV_INS_SID3:
        featureS3=true;
        break;
      case DIV_INS_SUPERVISION:
        featureSM=true;
        if (amiga.useSample) featureSL=true;
        break;
      case DIV_INS_UPD1771C:
        break;
      case DIV_INS_MAX:
        break;
      case DIV_INS_NULL:
        break;
    }
  } else {
    // turn on features depending on what is set
    // almost 40 years of C++, and there still isn't an official way to easily compare two structs.
    // even Java, which many regard as having a slow runtime, has .equals().
    if (fm!=defaultIns.fm) {
      featureFM=true;
      featureLD=true;
    }
    if (c64!=defaultIns.c64) {
      feature64=true;
    }
    if (gb!=defaultIns.gb) {
      featureGB=true;
    }
    if (amiga!=defaultIns.amiga) {
      featureSM=true;
      featureNE=true;
    }
    if (snes!=defaultIns.snes) {
      featureSN=true;
    }
    if (n163!=defaultIns.n163) {
      featureN1=true;
    }
    if (fds!=defaultIns.fds) {
      featureFD=true;
    }
    if (ws!=defaultIns.ws) {
      featureWS=true;
    }
    if (multipcm!=defaultIns.multipcm) {
      featureMP=true;
    }
    if (su!=defaultIns.su) {
      featureSU=true;
    }
    if (es5506!=defaultIns.es5506) {
      featureES=true;
    }
    if (x1_010!=defaultIns.x1_010) {
      featureX1=true;
    }
    if (esfm!=defaultIns.esfm) {
      featureEF=true;
    }
    if (powernoise!=defaultIns.powernoise) {
      featurePN=true;
    }
    if (sid2!=defaultIns.sid2) {
      featureS2=true;
    }
    if (sid3!=defaultIns.sid3) {
      featureS3=true;
    }
  }

  // check ins name
  if (!name.empty() && insName) {
    featureNA=true;
  }

  // check macros
  if (std.volMacro.len ||
      std.arpMacro.len ||
      std.dutyMacro.len ||
      std.waveMacro.len ||
      std.pitchMacro.len ||
      std.ex1Macro.len ||
      std.ex2Macro.len ||
      std.ex3Macro.len ||
      std.algMacro.len ||
      std.fbMacro.len ||
      std.fmsMacro.len ||
      std.amsMacro.len ||
      std.panLMacro.len ||
      std.panRMacro.len ||
      std.phaseResetMacro.len ||
      std.ex4Macro.len ||
      std.ex5Macro.len ||
      std.ex6Macro.len ||
      std.ex7Macro.len ||
      std.ex8Macro.len) {
    featureMA=true;
  }

  // check whether to write wavetable list
  if (checkForWL && fui) {
    if (std.waveMacro.len || ws.enabled) {
      featureWL=true;
    }
  }

  if (featureFM || featureS3 || !fui) {
    // check FM macros
    int opCount=4;
    bool storeExtendedAsWell=true;
    if (fui) {
      if (type==DIV_INS_OPLL) {
        opCount=2;
      } else if (type==DIV_INS_OPL) {
        opCount=(fm.ops==4)?4:2;
      } else if (type==DIV_INS_FM || type==DIV_INS_OPM) {
        storeExtendedAsWell=false;
      }
    }
    for (int i=0; i<opCount; i++) {
      const DivInstrumentSTD::OpMacro& m=std.opMacros[i];
      if (m.amMacro.len ||
          m.arMacro.len ||
          m.drMacro.len ||
          m.multMacro.len ||
          m.rrMacro.len ||
          m.slMacro.len ||
          m.tlMacro.len ||
          m.dt2Macro.len ||
          m.rsMacro.len ||
          m.dtMacro.len ||
          m.d2rMacro.len ||
          m.ssgMacro.len) {
        featureOx[i]=true;
      }
      if (storeExtendedAsWell) {
        if (m.damMacro.len ||
            m.dvbMacro.len ||
            m.egtMacro.len ||
            m.kslMacro.len ||
            m.susMacro.len ||
            m.vibMacro.len ||
            m.wsMacro.len ||
            m.ksrMacro.len) {
          featureOx[i]=true;
        }
      }
    }
  }

  // write features
  if (featureNA) {
    writeFeatureNA(w);
  }
  if (featureFM) {
    writeFeatureFM(w,fui);
  }
  if (featureMA) {
    writeFeatureMA(w);
  }
  if (feature64) {
    writeFeature64(w);
  }
  if (featureGB) {
    writeFeatureGB(w);
  }
  if (featureSM) {
    writeFeatureSM(w);
  }
  for (int i=0; i<4; i++) {
    if (featureOx[i]) {
      writeFeatureOx(w,i);
    }
  }
  if (featureLD) {
    writeFeatureLD(w);
  }
  if (featureSN) {
    writeFeatureSN(w);
  }
  if (featureN1) {
    writeFeatureN1(w);
  }
  if (featureFD) {
    writeFeatureFD(w);
  }
  if (featureWS) {
    writeFeatureWS(w);
  }
  if (featureSL) {
    slSeek=writeFeatureSL(w,sampleList,song);
  }
  if (featureWL) {
    wlSeek=writeFeatureWL(w,waveList,song);
  }
  if (featureMP) {
    writeFeatureMP(w);
  }
  if (featureSU) {
    writeFeatureSU(w);
  }
  if (featureES) {
    writeFeatureES(w);
  }
  if (featureX1) {
    writeFeatureX1(w);
  }
  if (featureNE) {
    writeFeatureNE(w);
  }
  if (featureEF) {
    writeFeatureEF(w);
  }
  if (featurePN) {
    writeFeaturePN(w);
  }
  if (featureS2) {
    writeFeatureS2(w);
  }
  if (featureS3) {
    writeFeatureS3(w);
  }

  if (fui && (featureSL || featureWL)) {
    w->write("EN",2);

    if (wlSeek!=0 && !waveList.empty()) {
      for (int i: waveList) {
        if (i<0 || i>=(int)song->wave.size()) {
          wavePtr.push_back(0);
          continue;
        }
        DivWavetable* wave=song->wave[i];

        wavePtr.push_back(w->tell());
        wave->putWaveData(w);
      }

      w->seek(wlSeek,SEEK_SET);
      for (unsigned int i: wavePtr) {
        w->writeI(i);
      }
      w->seek(0,SEEK_END);
    }

    if (slSeek!=0 && !sampleList.empty()) {
      for (int i: sampleList) {
        if (i<0 || i>=(int)song->sample.size()) {
          samplePtr.push_back(0);
          continue;
        }
        DivSample* sample=song->sample[i];

        samplePtr.push_back(w->tell());
        sample->putSampleData(w);
      }

      w->seek(slSeek,SEEK_SET);
      for (unsigned int i: samplePtr) {
        w->writeI(i);
      }
      w->seek(0,SEEK_END);
    }
  }

  if (!fui) {
    w->write("EN",2);
  }

  blockEndSeek=w->tell();
  if (!fui) {
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
  }
  w->seek(0,SEEK_END);
}

#define READ_FEAT_BEGIN \
  unsigned short featLen=reader.readS(); \
  size_t endOfFeat=reader.tell()+featLen;

#define READ_FEAT_END \
  if (reader.tell()<endOfFeat) reader.seek(endOfFeat,SEEK_SET);

void DivInstrument::readFeatureNA(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  name=reader.readString();

  READ_FEAT_END;
}

void DivInstrument::readFeatureFM(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char opCount=reader.readC();

  fm.op[0].enable=(opCount&16);
  fm.op[1].enable=(opCount&32);
  fm.op[2].enable=(opCount&64);
  fm.op[3].enable=(opCount&128);

  opCount&=15;

  unsigned char next=reader.readC();
  fm.alg=(next>>4)&7;
  fm.fb=next&7;

  next=reader.readC();
  fm.fms2=(next>>5)&7;
  fm.ams=(next>>3)&3;
  fm.fms=next&7;

  next=reader.readC();
  fm.ams2=(next>>6)&3;
  fm.ops=(next&32)?4:2;
  fm.opllPreset=next&31;

  // read operators
  for (int i=0; i<opCount; i++) {
    DivInstrumentFM::Operator& op=fm.op[i];

    next=reader.readC();
    op.ksr=(next&128)?1:0;
    op.dt=(next>>4)&7;
    op.mult=next&15;

    next=reader.readC();
    op.sus=(next&128)?1:0;
    op.tl=next&127;

    next=reader.readC();
    op.rs=(next>>6)&3;
    op.vib=(next&32)?1:0;
    op.ar=next&31;

    next=reader.readC();
    op.am=(next&128)?1:0;
    op.ksl=(next>>5)&3;
    op.dr=next&31;

    next=reader.readC();
    op.egt=(next&128)?1:0;
    op.kvs=(next>>5)&3;
    op.d2r=next&31;

    next=reader.readC();
    op.sl=(next>>4)&15;
    op.rr=next&15;

    next=reader.readC();
    op.dvb=(next>>4)&15;
    op.ssgEnv=next&15;

    next=reader.readC();
    op.dam=(next>>5)&7;
    op.dt2=(next>>3)&3;
    op.ws=next&7;
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureMA(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned short macroHeaderLen=reader.readS();

  if (macroHeaderLen==0) {
    logW("invalid macro header length!");
    READ_FEAT_END;
    return;
  }

  DivInstrumentMacro* target=&std.volMacro;

  while (reader.tell()<endOfFeat) {
    size_t endOfMacroHeader=reader.tell()+macroHeaderLen;
    unsigned char macroCode=reader.readC();

    // end of macro list
    if (macroCode==255) break;

    switch (macroCode) {
      case 0:
        target=&std.volMacro;
        break;
      case 1:
        target=&std.arpMacro;
        break;
      case 2:
        target=&std.dutyMacro;
        break;
      case 3:
        target=&std.waveMacro;
        break;
      case 4:
        target=&std.pitchMacro;
        break;
      case 5:
        target=&std.ex1Macro;
        break;
      case 6:
        target=&std.ex2Macro;
        break;
      case 7:
        target=&std.ex3Macro;
        break;
      case 8:
        target=&std.algMacro;
        break;
      case 9:
        target=&std.fbMacro;
        break;
      case 10:
        target=&std.fmsMacro;
        break;
      case 11:
        target=&std.amsMacro;
        break;
      case 12:
        target=&std.panLMacro;
        break;
      case 13:
        target=&std.panRMacro;
        break;
      case 14:
        target=&std.phaseResetMacro;
        break;
      case 15:
        target=&std.ex4Macro;
        break;
      case 16:
        target=&std.ex5Macro;
        break;
      case 17:
        target=&std.ex6Macro;
        break;
      case 18:
        target=&std.ex7Macro;
        break;
      case 19:
        target=&std.ex8Macro;
        break;
      default:
        logW("invalid macro code %d!");
        break;
    }

    target->len=reader.readC();
    target->loop=reader.readC();
    target->rel=reader.readC();
    target->mode=reader.readC();

    unsigned char wordSize=reader.readC();
    target->open=wordSize&15;
    wordSize>>=6;

    target->delay=reader.readC();
    target->speed=reader.readC();

    reader.seek(endOfMacroHeader,SEEK_SET);

    // read macro
    switch (wordSize) {
      case 0:
        for (int i=0; i<target->len; i++) {
          target->val[i]=(unsigned char)reader.readC();
        }
        break;
      case 1:
        for (int i=0; i<target->len; i++) {
          target->val[i]=(signed char)reader.readC();
        }
        break;
      case 2:
        for (int i=0; i<target->len; i++) {
          target->val[i]=reader.readS();
        }
        break;
      default:
        for (int i=0; i<target->len; i++) {
          target->val[i]=reader.readI();
        }
        break;
    }
  }

  if (version<193) {
    if (type==DIV_INS_AY || type==DIV_INS_AY8930) {
      for (int j=0; j<std.waveMacro.len; j++) {
        std.waveMacro.val[j]++;
      }
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeature64(SafeReader& reader, bool& volIsCutoff, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();
  c64.dutyIsAbs=next&128;
  c64.initFilter=next&64;
  volIsCutoff=next&32;
  c64.toFilter=next&16;
  c64.noiseOn=next&8;
  c64.pulseOn=next&4;
  c64.sawOn=next&2;
  c64.triOn=next&1;

  next=reader.readC();
  c64.oscSync=(next&128)?1:0;
  c64.ringMod=(next&64)?1:0;
  c64.noTest=next&32;
  c64.filterIsAbs=next&16;
  c64.ch3off=next&8;
  c64.bp=next&4;
  c64.hp=next&2;
  c64.lp=next&1;

  next=reader.readC();
  c64.a=(next>>4)&15;
  c64.d=next&15;

  next=reader.readC();
  c64.s=(next>>4)&15;
  c64.r=next&15;

  c64.duty=reader.readS()&4095;

  unsigned short cr=reader.readS();
  c64.cut=cr&4095;
  c64.res=cr>>12;

  if (version>=199) {
    next=(unsigned char)reader.readC();
    c64.res|=(next&15)<<4;

    if (version>=222) {
      c64.resetDuty=next&0x10;
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureGB(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();
  gb.envLen=(next>>5)&7;
  gb.envDir=(next&16)?1:0;
  gb.envVol=next&15;

  gb.soundLen=reader.readC();

  next=reader.readC();
  if (version>=196) gb.doubleWave=next&4;
  gb.alwaysInit=next&2;
  gb.softEnv=next&1;

  gb.hwSeqLen=reader.readC();
  for (int i=0; i<gb.hwSeqLen; i++) {
    gb.hwSeq[i].cmd=reader.readC();
    gb.hwSeq[i].data=reader.readS();
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureSM(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  amiga.initSample=reader.readS();

  unsigned char next=reader.readC();
  amiga.useWave=next&4;
  amiga.useSample=next&2;
  amiga.useNoteMap=next&1;

  amiga.waveLen=(unsigned char)reader.readC();

  if (amiga.useNoteMap) {
    for (int note=0; note<120; note++) {
      amiga.noteMap[note].freq=reader.readS();
      amiga.noteMap[note].map=reader.readS();
    }

    if (version<152) {
      for (int note=0; note<120; note++) {
        amiga.noteMap[note].freq=note;
      }
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureOx(SafeReader& reader, int op, short version) {
  READ_FEAT_BEGIN;

  unsigned short macroHeaderLen=reader.readS();

  if (macroHeaderLen==0) {
    logW("invalid macro header length!");
    READ_FEAT_END;
    return;
  }

  DivInstrumentMacro* target=&std.opMacros[op].amMacro;

  while (reader.tell()<endOfFeat) {
    size_t endOfMacroHeader=reader.tell()+macroHeaderLen;
    unsigned char macroCode=reader.readC();

    // end of macro list
    if (macroCode==255) break;

    switch (macroCode) {
      case 0:
        target=&std.opMacros[op].amMacro;
        break;
      case 1:
        target=&std.opMacros[op].arMacro;
        break;
      case 2:
        target=&std.opMacros[op].drMacro;
        break;
      case 3:
        target=&std.opMacros[op].multMacro;
        break;
      case 4:
        target=&std.opMacros[op].rrMacro;
        break;
      case 5:
        target=&std.opMacros[op].slMacro;
        break;
      case 6:
        target=&std.opMacros[op].tlMacro;
        break;
      case 7:
        target=&std.opMacros[op].dt2Macro;
        break;
      case 8:
        target=&std.opMacros[op].rsMacro;
        break;
      case 9:
        target=&std.opMacros[op].dtMacro;
        break;
      case 10:
        target=&std.opMacros[op].d2rMacro;
        break;
      case 11:
        target=&std.opMacros[op].ssgMacro;
        break;
      case 12:
        target=&std.opMacros[op].damMacro;
        break;
      case 13:
        target=&std.opMacros[op].dvbMacro;
        break;
      case 14:
        target=&std.opMacros[op].egtMacro;
        break;
      case 15:
        target=&std.opMacros[op].kslMacro;
        break;
      case 16:
        target=&std.opMacros[op].susMacro;
        break;
      case 17:
        target=&std.opMacros[op].vibMacro;
        break;
      case 18:
        target=&std.opMacros[op].wsMacro;
        break;
      case 19:
        target=&std.opMacros[op].ksrMacro;
        break;
    }

    target->len=reader.readC();
    target->loop=reader.readC();
    target->rel=reader.readC();
    target->mode=reader.readC();

    unsigned char wordSize=reader.readC();
    target->open=wordSize&7;
    wordSize>>=6;

    target->delay=reader.readC();
    target->speed=reader.readC();

    reader.seek(endOfMacroHeader,SEEK_SET);

    // read macro
    switch (wordSize) {
      case 0:
        for (int i=0; i<target->len; i++) {
          target->val[i]=(unsigned char)reader.readC();
        }
        break;
      case 1:
        for (int i=0; i<target->len; i++) {
          target->val[i]=(signed char)reader.readC();
        }
        break;
      case 2:
        for (int i=0; i<target->len; i++) {
          target->val[i]=reader.readS();
        }
        break;
      default:
        for (int i=0; i<target->len; i++) {
          target->val[i]=reader.readI();
        }
        break;
    }

    // <167 TL macro compat
    if (macroCode==6 && version<167) {
      if (target->open&6) {
        for (int j=0; j<2; j++) {
          target->val[j]^=0x7f;
        }
      } else {
        for (int j=0; j<target->len; j++) {
          target->val[j]^=0x7f;
        }
      }
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureLD(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  fm.fixedDrums=reader.readC();
  fm.kickFreq=reader.readS();
  fm.snareHatFreq=reader.readS();
  fm.tomTopFreq=reader.readS();

  READ_FEAT_END;
}

void DivInstrument::readFeatureSN(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();
  snes.d=(next>>4)&7;
  snes.a=next&15;

  next=reader.readC();
  snes.s=(next>>5)&7;
  snes.r=next&31;

  next=reader.readC();
  snes.useEnv=next&16;
  snes.sus=(next&8)?1:0;
  snes.gainMode=(DivInstrumentSNES::GainMode)(next&7);

  if (snes.gainMode==1 || snes.gainMode==2 || snes.gainMode==3) snes.gainMode=DivInstrumentSNES::GAIN_MODE_DIRECT;

  snes.gain=reader.readC();
  
  if (version>=131) {
    next=reader.readC();
    snes.sus=(next>>5)&3;
    snes.d2=next&31;
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureN1(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  n163.wave=reader.readI();
  n163.wavePos=(unsigned char)reader.readC();
  n163.waveLen=(unsigned char)reader.readC();
  n163.waveMode=(unsigned char)reader.readC();

  if (version>=164) {
    n163.perChanPos=reader.readC();
    if (n163.perChanPos) {
      for (int i=0; i<8; i++) {
        n163.wavePosCh[i]=(unsigned char)reader.readC();
      }
      for (int i=0; i<8; i++) {
        n163.waveLenCh[i]=(unsigned char)reader.readC();
      }
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureFD(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  fds.modSpeed=reader.readI();
  fds.modDepth=reader.readI();
  fds.initModTableWithFirstWave=reader.readC();
  reader.read(fds.modTable,32);

  READ_FEAT_END;
}

void DivInstrument::readFeatureWS(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  ws.wave1=reader.readI();
  ws.wave2=reader.readI();
  ws.rateDivider=reader.readC();
  ws.effect=reader.readC();
  ws.enabled=reader.readC();
  ws.global=reader.readC();
  ws.speed=reader.readC();
  ws.param1=reader.readC();
  ws.param2=reader.readC();
  ws.param3=reader.readC();
  ws.param4=reader.readC();

  READ_FEAT_END;
}

void DivInstrument::readFeatureSL(SafeReader& reader, DivSong* song, short version) {
  READ_FEAT_BEGIN;

  unsigned int samplePtr[256];
  unsigned char sampleIndex[256];
  unsigned char sampleRemap[256];
  memset(samplePtr,0,256*sizeof(unsigned int));
  memset(sampleIndex,0,256);
  memset(sampleRemap,0,256);

  unsigned char sampleCount=reader.readC();

  for (int i=0; i<sampleCount; i++) {
    sampleIndex[i]=reader.readC();
  }
  for (int i=0; i<sampleCount; i++) {
    samplePtr[i]=reader.readI();
  }

  size_t lastSeek=reader.tell();

  // load samples
  for (int i=0; i<sampleCount; i++) {
    reader.seek(samplePtr[i],SEEK_SET);
    if (song->sample.size()>=256) {
      break;
    }
    DivSample* sample=new DivSample;
    int sampleCount=(int)song->sample.size();

    DivDataErrors result=sample->readSampleData(reader,version);
    if (result==DIV_DATA_SUCCESS) {
      song->sample.push_back(sample);
      song->sampleLen=sampleCount+1;
      sampleRemap[sampleIndex[i]]=sampleCount;
    } else {
      delete sample;
      sampleRemap[sampleIndex[i]]=0;
    }
  }

  reader.seek(lastSeek,SEEK_SET);

  // re-map samples
  if (amiga.initSample>=0 && amiga.initSample<256) {
    amiga.initSample=sampleRemap[amiga.initSample];
  }

  if (amiga.useNoteMap) {
    for (int i=0; i<120; i++) {
      if (amiga.noteMap[i].map>=0 && amiga.noteMap[i].map<256) {
        amiga.noteMap[i].map=sampleRemap[amiga.noteMap[i].map];
      }
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureWL(SafeReader& reader, DivSong* song, short version) {
  READ_FEAT_BEGIN;

  unsigned int wavePtr[256];
  unsigned char waveIndex[256];
  unsigned char waveRemap[256];
  memset(wavePtr,0,256*sizeof(unsigned int));
  memset(waveIndex,0,256);
  memset(waveRemap,0,256);

  unsigned char waveCount=reader.readC();

  for (int i=0; i<waveCount; i++) {
    waveIndex[i]=reader.readC();
  }
  for (int i=0; i<waveCount; i++) {
    wavePtr[i]=reader.readI();
  }

  size_t lastSeek=reader.tell();

  // load wavetables
  for (int i=0; i<waveCount; i++) {
    reader.seek(wavePtr[i],SEEK_SET);
    if (song->wave.size()>=256) {
      break;
    }
    DivWavetable* wave=new DivWavetable;
    int waveCount=(int)song->wave.size();

    DivDataErrors result=wave->readWaveData(reader,version);
    if (result==DIV_DATA_SUCCESS) {
      song->wave.push_back(wave);
      song->waveLen=waveCount+1;
      waveRemap[waveIndex[i]]=waveCount;
    } else {
      delete wave;
      waveRemap[waveIndex[i]]=0;
    }
  }

  reader.seek(lastSeek,SEEK_SET);

  // re-map wavetables
  if (ws.enabled) {
    if (ws.wave1>=0 && ws.wave1<256) ws.wave1=waveRemap[ws.wave1];
    if (ws.effect&0x80) {
      if (ws.wave2>=0 && ws.wave2<256) ws.wave2=waveRemap[ws.wave2];
    }
  }
  if (n163.wave>=0 && n163.wave<256) n163.wave=waveRemap[n163.wave];
  for (int i=0; i<std.waveMacro.len; i++) {
    if (std.waveMacro.val[i]>=0 && std.waveMacro.val[i]<256) std.waveMacro.val[i]=waveRemap[std.waveMacro.val[i]];
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureMP(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  multipcm.ar=reader.readC();
  multipcm.d1r=reader.readC();
  multipcm.dl=reader.readC();
  multipcm.d2r=reader.readC();
  multipcm.rr=reader.readC();
  multipcm.rc=reader.readC();
  multipcm.lfo=reader.readC();
  multipcm.vib=reader.readC();
  multipcm.am=reader.readC();

  if (version>=221) {
    unsigned char next=reader.readC();
    multipcm.damp=next&1;
    multipcm.pseudoReverb=next&2;
    multipcm.lfoReset=next&4;
    multipcm.levelDirect=next&8;
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureSU(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  su.switchRoles=reader.readC();

  if (version>=185) {
    su.hwSeqLen=reader.readC();
    for (int i=0; i<su.hwSeqLen; i++) {
      su.hwSeq[i].cmd=reader.readC();
      su.hwSeq[i].bound=reader.readC();
      su.hwSeq[i].val=reader.readC();
      su.hwSeq[i].speed=reader.readS();
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureES(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  es5506.filter.mode=(DivInstrumentES5506::Filter::FilterMode)reader.readC();
  es5506.filter.k1=reader.readS();
  es5506.filter.k2=reader.readS();
  es5506.envelope.ecount=reader.readS();
  es5506.envelope.lVRamp=reader.readC();
  es5506.envelope.rVRamp=reader.readC();
  es5506.envelope.k1Ramp=reader.readC();
  es5506.envelope.k2Ramp=reader.readC();
  es5506.envelope.k1Slow=reader.readC();
  es5506.envelope.k2Slow=reader.readC();

  READ_FEAT_END;
}

void DivInstrument::readFeatureX1(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  x1_010.bankSlot=reader.readI();

  READ_FEAT_END;
}

void DivInstrument::readFeatureNE(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  amiga.useNoteMap=reader.readC();

  if (amiga.useNoteMap) {
    for (int note=0; note<120; note++) {
      amiga.noteMap[note].dpcmFreq=reader.readC();
      amiga.noteMap[note].dpcmDelta=reader.readC();
    }
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureEF(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();
  esfm.noise=next&3;

  for (int i=0; i<4; i++) {
    DivInstrumentESFM::Operator& op=esfm.op[i];

    next=reader.readC();
    op.delay=(next>>5)&7;
    op.outLvl=(next>>2)&7;
    op.right=(next>>1)&1;
    op.left=next&1;

    next=reader.readC();
    op.modIn=next&7;
    op.fixed=(next>>3)&1;

    op.ct=reader.readC();
    op.dt=reader.readC();
  }

  READ_FEAT_END;
}

void DivInstrument::readFeaturePN(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  powernoise.octave=reader.readC();

  READ_FEAT_END;
}

void DivInstrument::readFeatureS2(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();

  sid2.volume=next&0xf;
  sid2.mixMode=(next>>4)&3;
  sid2.noiseMode=next>>6;

  READ_FEAT_END;
}

void DivInstrument::readFeatureS3(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();

  sid3.dutyIsAbs=next&0x80;
  sid3.noiseOn=next&8;
  sid3.pulseOn=next&4;
  sid3.sawOn=next&2;
  sid3.triOn=next&1;

  sid3.a=reader.readC();
  sid3.d=reader.readC();
  sid3.s=reader.readC();
  sid3.sr=reader.readC();
  sid3.r=reader.readC();

  sid3.mixMode=reader.readC();

  sid3.duty=reader.readS();

  next=reader.readC();

  sid3.phase_mod=next&0x80;
  sid3.specialWaveOn=next&0x40;
  sid3.oneBitNoise=next&0x20;
  sid3.separateNoisePitch=next&0x10;
  sid3.doWavetable=next&8;
  sid3.resetDuty=next&4;
  sid3.oscSync=next&2;
  sid3.ringMod=next&1;

  sid3.phase_mod_source=reader.readC();
  sid3.ring_mod_source=reader.readC();
  sid3.sync_source=reader.readC();
  sid3.special_wave=reader.readC();
  sid3.phaseInv=reader.readC();
  sid3.feedback=reader.readC();

  unsigned char numFilters=reader.readC();

  for (int i=0; i<numFilters; i++) {
    if (i>=4) break;

    next=reader.readC();

    sid3.filt[i].enabled=next&0x80;
    sid3.filt[i].init=next&0x40;
    sid3.filt[i].absoluteCutoff=next&0x20;
    sid3.filt[i].bindCutoffToNote=next&0x10;
    sid3.filt[i].bindCutoffToNoteDir=next&8;
    sid3.filt[i].bindCutoffOnNote=next&4;
    sid3.filt[i].bindResonanceToNote=next&2;
    sid3.filt[i].bindResonanceToNoteDir=next&1;

    next=reader.readC();

    sid3.filt[i].bindResonanceOnNote=next&0x80;

    sid3.filt[i].cutoff=reader.readS();

    sid3.filt[i].resonance=reader.readC();
    sid3.filt[i].output_volume=reader.readC();
    sid3.filt[i].distortion_level=reader.readC();
    sid3.filt[i].mode=reader.readC();
    sid3.filt[i].filter_matrix=reader.readC();

    sid3.filt[i].bindCutoffToNoteStrength=reader.readC();
    sid3.filt[i].bindCutoffToNoteCenter=reader.readC();
    sid3.filt[i].bindResonanceToNoteStrength=reader.readC();
    sid3.filt[i].bindResonanceToNoteCenter=reader.readC();
  }

  READ_FEAT_END;
}

DivDataErrors DivInstrument::readInsDataNew(SafeReader& reader, short version, bool fui, DivSong* song) {
  unsigned char featCode[2];
  bool volIsCutoff=false;

  int dataLen=reader.size()-4;
  if (!fui) {
    dataLen=reader.readI();
  }
  dataLen+=reader.tell();

  logV("data length: %d",dataLen);

  reader.readS(); // format version. ignored.

  type=(DivInstrumentType)reader.readS();

  // feature reading loop
  while ((int)reader.tell()<dataLen) {
    // read feature code
    reader.read(featCode,2);
    logV("- %c%c",featCode[0],featCode[1]);

    if (memcmp(featCode,"EN",2)==0) { // end of instrument
      break;
    } else if (memcmp(featCode,"NA",2)==0) { // name
      readFeatureNA(reader,version);
    } else if (memcmp(featCode,"FM",2)==0) { // FM
      readFeatureFM(reader,version);
    } else if (memcmp(featCode,"MA",2)==0) { // macros
      readFeatureMA(reader,version);
    } else if (memcmp(featCode,"64",2)==0) { // C64
      readFeature64(reader,volIsCutoff,version);
    } else if (memcmp(featCode,"GB",2)==0) { // Game Boy
      readFeatureGB(reader,version);
    } else if (memcmp(featCode,"SM",2)==0) { // sample
      readFeatureSM(reader,version);
    } else if (memcmp(featCode,"O1",2)==0) { // op1 macros
      readFeatureOx(reader,0,version);
    } else if (memcmp(featCode,"O2",2)==0) { // op2 macros
      readFeatureOx(reader,1,version);
    } else if (memcmp(featCode,"O3",2)==0) { // op3 macros
      readFeatureOx(reader,2,version);
    } else if (memcmp(featCode,"O4",2)==0) { // op4 macros
      readFeatureOx(reader,3,version);
    } else if (memcmp(featCode,"LD",2)==0) { // OPL drums
      readFeatureLD(reader,version);
    } else if (memcmp(featCode,"SN",2)==0) { // SNES
      readFeatureSN(reader,version);
    } else if (memcmp(featCode,"N1",2)==0) { // Namco 163
      readFeatureN1(reader,version);
    } else if (memcmp(featCode,"FD",2)==0) { // FDS/VB
      readFeatureFD(reader,version);
    } else if (memcmp(featCode,"WS",2)==0) { // WaveSynth
      readFeatureWS(reader,version);
    } else if (memcmp(featCode,"SL",2)==0 && fui && song!=NULL) { // sample list
      readFeatureSL(reader,song,version);
    } else if (memcmp(featCode,"WL",2)==0 && fui && song!=NULL) { // wave list
      readFeatureWL(reader,song,version);
    } else if (memcmp(featCode,"MP",2)==0) { // MultiPCM
      readFeatureMP(reader,version);
    } else if (memcmp(featCode,"SU",2)==0) { // Sound Unit
      readFeatureSU(reader,version);
    } else if (memcmp(featCode,"ES",2)==0) { // ES5506
      readFeatureES(reader,version);
    } else if (memcmp(featCode,"X1",2)==0) { // X1-010
      readFeatureX1(reader,version);
    } else if (memcmp(featCode,"NE",2)==0) { // NES (DPCM)
      readFeatureNE(reader,version);
    } else if (memcmp(featCode,"EF",2)==0) { // ESFM
      readFeatureEF(reader,version);
    } else if (memcmp(featCode,"PN",2)==0) { // PowerNoise
      readFeaturePN(reader,version);
    } else if (memcmp(featCode,"S2",2)==0) { // SID2
      readFeatureS2(reader,version);
    } else if (memcmp(featCode,"S3",2)==0) { // SID3
      readFeatureS3(reader,version);
    } else {
      if (song==NULL && (memcmp(featCode,"SL",2)==0 || (memcmp(featCode,"WL",2)==0))) {
        // nothing
      } else {
        logW("unknown feature code %c%c!",featCode[0],featCode[1]);
      }
      // skip feature
      unsigned short skip=reader.readS();
      reader.seek(skip,SEEK_CUR);
    }
  }

  // <187 C64 cutoff macro compatibility
  if (type==DIV_INS_C64 && volIsCutoff && version<187) {
    memcpy(&std.algMacro,&std.volMacro,sizeof(DivInstrumentMacro));
    std.algMacro.macroType=DIV_MACRO_ALG;
    std.volMacro=DivInstrumentMacro(DIV_MACRO_VOL,true);

    if (!c64.filterIsAbs) {
      for (int i=0; i<std.algMacro.len; i++) {
        std.algMacro.val[i]=-std.algMacro.val[i];
      }
    }
  }

  // <187 special/test/gate merge
  if (type==DIV_INS_C64 && version<187) {
    convertC64SpecialMacro();
  }

  return DIV_DATA_SUCCESS;
}

#define READ_MACRO_VALS(x,y) \
  for (int macroValPos=0; macroValPos<y; macroValPos++) x[macroValPos]=reader.readI();

DivDataErrors DivInstrument::readInsDataOld(SafeReader &reader, short version) {
  bool volIsCutoff=false;
  reader.readI(); // length. ignored.

  reader.readS(); // format version. ignored.
  type=(DivInstrumentType)reader.readC();
  reader.readC();
  name=reader.readString();

  // FM
  fm.alg=reader.readC();
  fm.fb=reader.readC();
  fm.fms=reader.readC();
  fm.ams=reader.readC();
  fm.ops=reader.readC();
  if (version>=60) {
    fm.opllPreset=reader.readC();
  } else {
    reader.readC();
  }
  reader.readC();
  reader.readC();

  for (int j=0; j<4; j++) {
    DivInstrumentFM::Operator& op=fm.op[j];
    op.am=reader.readC();
    op.ar=reader.readC();
    op.dr=reader.readC();
    op.mult=reader.readC();
    op.rr=reader.readC();
    op.sl=reader.readC();
    op.tl=reader.readC();
    op.dt2=reader.readC();
    op.rs=reader.readC();
    op.dt=reader.readC();
    op.d2r=reader.readC();
    op.ssgEnv=reader.readC();

    op.dam=reader.readC();
    op.dvb=reader.readC();
    op.egt=reader.readC();
    op.ksl=reader.readC();
    op.sus=reader.readC();
    op.vib=reader.readC();
    op.ws=reader.readC();
    op.ksr=reader.readC();

    if (version>=114) {
      op.enable=reader.readC();
    } else {
      reader.readC();
    }

    if (version>=115) {
      op.kvs=reader.readC();
    } else {
      op.kvs=2;
      reader.readC();
    }

    // reserved
    for (int k=0; k<10; k++) reader.readC();
  }

  // GB
  gb.envVol=reader.readC();
  gb.envDir=reader.readC();
  gb.envLen=reader.readC();
  gb.soundLen=reader.readC();

  // C64
  c64.triOn=reader.readC();
  c64.sawOn=reader.readC();
  c64.pulseOn=reader.readC();
  c64.noiseOn=reader.readC();
  c64.a=reader.readC();
  c64.d=reader.readC();
  c64.s=reader.readC();
  c64.r=reader.readC();
  c64.duty=reader.readS();
  c64.ringMod=reader.readC();
  c64.oscSync=reader.readC();
  c64.toFilter=reader.readC();
  c64.initFilter=reader.readC();
  volIsCutoff=reader.readC();
  c64.res=reader.readC();
  c64.lp=reader.readC();
  c64.bp=reader.readC();
  c64.hp=reader.readC();
  c64.ch3off=reader.readC();
  c64.cut=reader.readS();
  c64.dutyIsAbs=reader.readC();
  c64.filterIsAbs=reader.readC();

  // Amiga
  amiga.initSample=reader.readS();
  if (version>=82) {
    amiga.useWave=reader.readC();
    amiga.waveLen=(unsigned char)reader.readC();
  } else {
    reader.readC();
    reader.readC();
  }
  // reserved
  for (int k=0; k<12; k++) reader.readC();

  // standard
  std.volMacro.len=reader.readI();
  std.arpMacro.len=reader.readI();
  std.dutyMacro.len=reader.readI();
  std.waveMacro.len=reader.readI();
  if (version>=17) {
    std.pitchMacro.len=reader.readI();
    std.ex1Macro.len=reader.readI();
    std.ex2Macro.len=reader.readI();
    std.ex3Macro.len=reader.readI();
  }
  std.volMacro.loop=reader.readI();
  std.arpMacro.loop=reader.readI();
  std.dutyMacro.loop=reader.readI();
  std.waveMacro.loop=reader.readI();
  if (version>=17) {
    std.pitchMacro.loop=reader.readI();
    std.ex1Macro.loop=reader.readI();
    std.ex2Macro.loop=reader.readI();
    std.ex3Macro.loop=reader.readI();
  }
  std.arpMacro.mode=reader.readC();
  // these 3 were macro heights before but they are not used anymore
  int oldVolHeight=reader.readC();
  int oldDutyHeight=reader.readC();
  reader.readC(); // oldWaveHeight
  READ_MACRO_VALS(std.volMacro.val,std.volMacro.len);
  READ_MACRO_VALS(std.arpMacro.val,std.arpMacro.len);
  READ_MACRO_VALS(std.dutyMacro.val,std.dutyMacro.len);
  READ_MACRO_VALS(std.waveMacro.val,std.waveMacro.len);
  if (version<31) {
    if (!std.arpMacro.mode) for (int j=0; j<std.arpMacro.len; j++) {
      std.arpMacro.val[j]-=12;
    }
  }
  if (type==DIV_INS_C64 && version<87) {
    if (volIsCutoff && !c64.filterIsAbs) for (int j=0; j<std.volMacro.len; j++) {
      std.volMacro.val[j]-=18;
    }
    if (!c64.dutyIsAbs) for (int j=0; j<std.dutyMacro.len; j++) {
      std.dutyMacro.val[j]-=12;
    }
  }
  if (version<193) {
    if (type==DIV_INS_AY || type==DIV_INS_AY8930) {
      for (int j=0; j<std.waveMacro.len; j++) {
        std.waveMacro.val[j]++;
      }
    }
  }
  if (version>=17) {
    READ_MACRO_VALS(std.pitchMacro.val,std.pitchMacro.len);
    READ_MACRO_VALS(std.ex1Macro.val,std.ex1Macro.len);
    READ_MACRO_VALS(std.ex2Macro.val,std.ex2Macro.len);
    READ_MACRO_VALS(std.ex3Macro.val,std.ex3Macro.len);
  } else {
    if (type==DIV_INS_STD) {
      if (oldVolHeight==31) {
        type=DIV_INS_PCE;
      }
      if (oldDutyHeight==31) {
        type=DIV_INS_AY;
      }
    }
  }

  // FM macros
  if (version>=29) {
    std.algMacro.len=reader.readI();
    std.fbMacro.len=reader.readI();
    std.fmsMacro.len=reader.readI();
    std.amsMacro.len=reader.readI();
    std.algMacro.loop=reader.readI();
    std.fbMacro.loop=reader.readI();
    std.fmsMacro.loop=reader.readI();
    std.amsMacro.loop=reader.readI();
    std.volMacro.open=reader.readC();
    std.arpMacro.open=reader.readC();
    std.dutyMacro.open=reader.readC();
    std.waveMacro.open=reader.readC();
    std.pitchMacro.open=reader.readC();
    std.ex1Macro.open=reader.readC();
    std.ex2Macro.open=reader.readC();
    std.ex3Macro.open=reader.readC();
    std.algMacro.open=reader.readC();
    std.fbMacro.open=reader.readC();
    std.fmsMacro.open=reader.readC();
    std.amsMacro.open=reader.readC();

    READ_MACRO_VALS(std.algMacro.val,std.algMacro.len);
    READ_MACRO_VALS(std.fbMacro.val,std.fbMacro.len);
    READ_MACRO_VALS(std.fmsMacro.val,std.fmsMacro.len);
    READ_MACRO_VALS(std.amsMacro.val,std.amsMacro.len);

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacro.len=reader.readI();
      op.arMacro.len=reader.readI();
      op.drMacro.len=reader.readI();
      op.multMacro.len=reader.readI();
      op.rrMacro.len=reader.readI();
      op.slMacro.len=reader.readI();
      op.tlMacro.len=reader.readI();
      op.dt2Macro.len=reader.readI();
      op.rsMacro.len=reader.readI();
      op.dtMacro.len=reader.readI();
      op.d2rMacro.len=reader.readI();
      op.ssgMacro.len=reader.readI();

      op.amMacro.loop=reader.readI();
      op.arMacro.loop=reader.readI();
      op.drMacro.loop=reader.readI();
      op.multMacro.loop=reader.readI();
      op.rrMacro.loop=reader.readI();
      op.slMacro.loop=reader.readI();
      op.tlMacro.loop=reader.readI();
      op.dt2Macro.loop=reader.readI();
      op.rsMacro.loop=reader.readI();
      op.dtMacro.loop=reader.readI();
      op.d2rMacro.loop=reader.readI();
      op.ssgMacro.loop=reader.readI();

      op.amMacro.open=reader.readC();
      op.arMacro.open=reader.readC();
      op.drMacro.open=reader.readC();
      op.multMacro.open=reader.readC();
      op.rrMacro.open=reader.readC();
      op.slMacro.open=reader.readC();
      op.tlMacro.open=reader.readC();
      op.dt2Macro.open=reader.readC();
      op.rsMacro.open=reader.readC();
      op.dtMacro.open=reader.readC();
      op.d2rMacro.open=reader.readC();
      op.ssgMacro.open=reader.readC();
    }

    // FM macro low 8 bits
    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];
      for (int j=0; j<op.amMacro.len; j++) {
        op.amMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.arMacro.len; j++) {
        op.arMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.drMacro.len; j++) {
        op.drMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.multMacro.len; j++) {
        op.multMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.rrMacro.len; j++) {
        op.rrMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.slMacro.len; j++) {
        op.slMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.tlMacro.len; j++) {
        op.tlMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.dt2Macro.len; j++) {
        op.dt2Macro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.rsMacro.len; j++) {
        op.rsMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.dtMacro.len; j++) {
        op.dtMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.d2rMacro.len; j++) {
        op.d2rMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.ssgMacro.len; j++) {
        op.ssgMacro.val[j]=(unsigned char)reader.readC();
      }
    }
  }

  // release points
  if (version>=44) {
    std.volMacro.rel=reader.readI();
    std.arpMacro.rel=reader.readI();
    std.dutyMacro.rel=reader.readI();
    std.waveMacro.rel=reader.readI();
    std.pitchMacro.rel=reader.readI();
    std.ex1Macro.rel=reader.readI();
    std.ex2Macro.rel=reader.readI();
    std.ex3Macro.rel=reader.readI();
    std.algMacro.rel=reader.readI();
    std.fbMacro.rel=reader.readI();
    std.fmsMacro.rel=reader.readI();
    std.amsMacro.rel=reader.readI();

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacro.rel=reader.readI();
      op.arMacro.rel=reader.readI();
      op.drMacro.rel=reader.readI();
      op.multMacro.rel=reader.readI();
      op.rrMacro.rel=reader.readI();
      op.slMacro.rel=reader.readI();
      op.tlMacro.rel=reader.readI();
      op.dt2Macro.rel=reader.readI();
      op.rsMacro.rel=reader.readI();
      op.dtMacro.rel=reader.readI();
      op.d2rMacro.rel=reader.readI();
      op.ssgMacro.rel=reader.readI();
    }
  }

  // extended op macros
  if (version>=61) {
    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.damMacro.len=reader.readI();
      op.dvbMacro.len=reader.readI();
      op.egtMacro.len=reader.readI();
      op.kslMacro.len=reader.readI();
      op.susMacro.len=reader.readI();
      op.vibMacro.len=reader.readI();
      op.wsMacro.len=reader.readI();
      op.ksrMacro.len=reader.readI();

      op.damMacro.loop=reader.readI();
      op.dvbMacro.loop=reader.readI();
      op.egtMacro.loop=reader.readI();
      op.kslMacro.loop=reader.readI();
      op.susMacro.loop=reader.readI();
      op.vibMacro.loop=reader.readI();
      op.wsMacro.loop=reader.readI();
      op.ksrMacro.loop=reader.readI();

      op.damMacro.rel=reader.readI();
      op.dvbMacro.rel=reader.readI();
      op.egtMacro.rel=reader.readI();
      op.kslMacro.rel=reader.readI();
      op.susMacro.rel=reader.readI();
      op.vibMacro.rel=reader.readI();
      op.wsMacro.rel=reader.readI();
      op.ksrMacro.rel=reader.readI();

      op.damMacro.open=reader.readC();
      op.dvbMacro.open=reader.readC();
      op.egtMacro.open=reader.readC();
      op.kslMacro.open=reader.readC();
      op.susMacro.open=reader.readC();
      op.vibMacro.open=reader.readC();
      op.wsMacro.open=reader.readC();
      op.ksrMacro.open=reader.readC();
    }

    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];
      for (int j=0; j<op.damMacro.len; j++) {
        op.damMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.dvbMacro.len; j++) {
        op.dvbMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.egtMacro.len; j++) {
        op.egtMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.kslMacro.len; j++) {
        op.kslMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.susMacro.len; j++) {
        op.susMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.vibMacro.len; j++) {
        op.vibMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.wsMacro.len; j++) {
        op.wsMacro.val[j]=(unsigned char)reader.readC();
      }
      for (int j=0; j<op.ksrMacro.len; j++) {
        op.ksrMacro.val[j]=(unsigned char)reader.readC();
      }
    }
  }

  // OPL drum data
  if (version>=63) {
    fm.fixedDrums=reader.readC();
    reader.readC(); // reserved
    fm.kickFreq=reader.readS();
    fm.snareHatFreq=reader.readS();
    fm.tomTopFreq=reader.readS();
  }

  // clear noise macro if PCE instrument and version<63
  if (version<63 && type==DIV_INS_PCE) {
    std.dutyMacro.len=0;
    std.dutyMacro.loop=255;
    std.dutyMacro.rel=255;
  }

  // clear wave macro if OPLL instrument and version<70
  if (version<70 && type==DIV_INS_OPLL) {
    std.waveMacro.len=0;
    std.waveMacro.loop=255;
    std.waveMacro.rel=255;
  }

  // sample map
  if (version>=67) {
    amiga.useNoteMap=reader.readC();
    if (amiga.useNoteMap) {
      for (int note=0; note<120; note++) {
        amiga.noteMap[note].freq=reader.readI();
      }
      for (int note=0; note<120; note++) {
        amiga.noteMap[note].map=reader.readS();
      }

      if (version<152) {
        for (int note=0; note<120; note++) {
          amiga.noteMap[note].freq=note;
        }
      }
    }
  }

  // N163
  if (version>=73) {
    n163.wave=reader.readI();
    n163.wavePos=(unsigned char)reader.readC();
    n163.waveLen=(unsigned char)reader.readC();
    n163.waveMode=(unsigned char)reader.readC();
    reader.readC(); // reserved
  }

  if (version>=76) {
    // more macros
    std.panLMacro.len=reader.readI();
    std.panRMacro.len=reader.readI();
    std.phaseResetMacro.len=reader.readI();
    std.ex4Macro.len=reader.readI();
    std.ex5Macro.len=reader.readI();
    std.ex6Macro.len=reader.readI();
    std.ex7Macro.len=reader.readI();
    std.ex8Macro.len=reader.readI();

    std.panLMacro.loop=reader.readI();
    std.panRMacro.loop=reader.readI();
    std.phaseResetMacro.loop=reader.readI();
    std.ex4Macro.loop=reader.readI();
    std.ex5Macro.loop=reader.readI();
    std.ex6Macro.loop=reader.readI();
    std.ex7Macro.loop=reader.readI();
    std.ex8Macro.loop=reader.readI();

    std.panLMacro.rel=reader.readI();
    std.panRMacro.rel=reader.readI();
    std.phaseResetMacro.rel=reader.readI();
    std.ex4Macro.rel=reader.readI();
    std.ex5Macro.rel=reader.readI();
    std.ex6Macro.rel=reader.readI();
    std.ex7Macro.rel=reader.readI();
    std.ex8Macro.rel=reader.readI();

    std.panLMacro.open=reader.readC();
    std.panRMacro.open=reader.readC();
    std.phaseResetMacro.open=reader.readC();
    std.ex4Macro.open=reader.readC();
    std.ex5Macro.open=reader.readC();
    std.ex6Macro.open=reader.readC();
    std.ex7Macro.open=reader.readC();
    std.ex8Macro.open=reader.readC();

    READ_MACRO_VALS(std.panLMacro.val,std.panLMacro.len);
    READ_MACRO_VALS(std.panRMacro.val,std.panRMacro.len);
    READ_MACRO_VALS(std.phaseResetMacro.val,std.phaseResetMacro.len);
    READ_MACRO_VALS(std.ex4Macro.val,std.ex4Macro.len);
    READ_MACRO_VALS(std.ex5Macro.val,std.ex5Macro.len);
    READ_MACRO_VALS(std.ex6Macro.val,std.ex6Macro.len);
    READ_MACRO_VALS(std.ex7Macro.val,std.ex7Macro.len);
    READ_MACRO_VALS(std.ex8Macro.val,std.ex8Macro.len);

    // FDS
    fds.modSpeed=reader.readI();
    fds.modDepth=reader.readI();
    fds.initModTableWithFirstWave=reader.readC();
    reader.readC(); // reserved
    reader.readC();
    reader.readC();
    reader.read(fds.modTable,32);
  }

  // OPZ
  if (version>=77) {
    fm.fms2=reader.readC();
    fm.ams2=reader.readC();
  }

  // wave synth
  if (version>=79) {
    ws.wave1=reader.readI();
    ws.wave2=reader.readI();
    ws.rateDivider=reader.readC();
    ws.effect=reader.readC();
    ws.enabled=reader.readC();
    ws.global=reader.readC();
    ws.speed=reader.readC();
    ws.param1=reader.readC();
    ws.param2=reader.readC();
    ws.param3=reader.readC();
    ws.param4=reader.readC();
  }

  // other macro modes
  if (version>=84) {
    std.volMacro.mode=reader.readC();
    std.dutyMacro.mode=reader.readC();
    std.waveMacro.mode=reader.readC();
    std.pitchMacro.mode=reader.readC();
    std.ex1Macro.mode=reader.readC();
    std.ex2Macro.mode=reader.readC();
    std.ex3Macro.mode=reader.readC();
    std.algMacro.mode=reader.readC();
    std.fbMacro.mode=reader.readC();
    std.fmsMacro.mode=reader.readC();
    std.amsMacro.mode=reader.readC();
    std.panLMacro.mode=reader.readC();
    std.panRMacro.mode=reader.readC();
    std.phaseResetMacro.mode=reader.readC();
    std.ex4Macro.mode=reader.readC();
    std.ex5Macro.mode=reader.readC();
    std.ex6Macro.mode=reader.readC();
    std.ex7Macro.mode=reader.readC();
    std.ex8Macro.mode=reader.readC();
  }

  // C64 no test
  if (version>=89) {
    c64.noTest=reader.readC();
  }

  // MultiPCM
  if (version>=93) {
    multipcm.ar=reader.readC();
    multipcm.d1r=reader.readC();
    multipcm.dl=reader.readC();
    multipcm.d2r=reader.readC();
    multipcm.rr=reader.readC();
    multipcm.rc=reader.readC();
    multipcm.lfo=reader.readC();
    multipcm.vib=reader.readC();
    multipcm.am=reader.readC();
    // reserved
    for (int k=0; k<23; k++) reader.readC();
  }

  // Sound Unit
  if (version>=104) {
    amiga.useSample=reader.readC();
    su.switchRoles=reader.readC();
  }

  // GB hardware sequence
  if (version>=105) {
    gb.hwSeqLen=reader.readC();
    for (int i=0; i<gb.hwSeqLen; i++) {
      gb.hwSeq[i].cmd=reader.readC();
      gb.hwSeq[i].data=reader.readS();
    }
  }

  // GB additional flags
  if (version>=106) {
    gb.softEnv=reader.readC();
    gb.alwaysInit=reader.readC();
  }

  // ES5506
  if (version>=107) {
    es5506.filter.mode=(DivInstrumentES5506::Filter::FilterMode)reader.readC();
    es5506.filter.k1=reader.readS();
    es5506.filter.k2=reader.readS();
    es5506.envelope.ecount=reader.readS();
    es5506.envelope.lVRamp=reader.readC();
    es5506.envelope.rVRamp=reader.readC();
    es5506.envelope.k1Ramp=reader.readC();
    es5506.envelope.k2Ramp=reader.readC();
    es5506.envelope.k1Slow=reader.readC();
    es5506.envelope.k2Slow=reader.readC();
  }

  // SNES
  if (version>=109) {
    snes.useEnv=reader.readC();
    if (version<118) {
      // why why why
      reader.readC();
      reader.readC();
    } else {
      snes.gainMode=(DivInstrumentSNES::GainMode)reader.readC();
      snes.gain=reader.readC();
    }
    snes.a=reader.readC();
    snes.d=reader.readC();
    snes.s=reader.readC();
    snes.sus=(snes.s&8)?1:0;
    snes.s&=7;
    snes.r=reader.readC();
  }

  // macro speed/delay
  if (version>=111) {
    std.volMacro.speed=reader.readC();
    std.arpMacro.speed=reader.readC();
    std.dutyMacro.speed=reader.readC();
    std.waveMacro.speed=reader.readC();
    std.pitchMacro.speed=reader.readC();
    std.ex1Macro.speed=reader.readC();
    std.ex2Macro.speed=reader.readC();
    std.ex3Macro.speed=reader.readC();
    std.algMacro.speed=reader.readC();
    std.fbMacro.speed=reader.readC();
    std.fmsMacro.speed=reader.readC();
    std.amsMacro.speed=reader.readC();
    std.panLMacro.speed=reader.readC();
    std.panRMacro.speed=reader.readC();
    std.phaseResetMacro.speed=reader.readC();
    std.ex4Macro.speed=reader.readC();
    std.ex5Macro.speed=reader.readC();
    std.ex6Macro.speed=reader.readC();
    std.ex7Macro.speed=reader.readC();
    std.ex8Macro.speed=reader.readC();

    std.volMacro.delay=reader.readC();
    std.arpMacro.delay=reader.readC();
    std.dutyMacro.delay=reader.readC();
    std.waveMacro.delay=reader.readC();
    std.pitchMacro.delay=reader.readC();
    std.ex1Macro.delay=reader.readC();
    std.ex2Macro.delay=reader.readC();
    std.ex3Macro.delay=reader.readC();
    std.algMacro.delay=reader.readC();
    std.fbMacro.delay=reader.readC();
    std.fmsMacro.delay=reader.readC();
    std.amsMacro.delay=reader.readC();
    std.panLMacro.delay=reader.readC();
    std.panRMacro.delay=reader.readC();
    std.phaseResetMacro.delay=reader.readC();
    std.ex4Macro.delay=reader.readC();
    std.ex5Macro.delay=reader.readC();
    std.ex6Macro.delay=reader.readC();
    std.ex7Macro.delay=reader.readC();
    std.ex8Macro.delay=reader.readC();

    // op macro speed/delay
    for (int i=0; i<4; i++) {
      DivInstrumentSTD::OpMacro& op=std.opMacros[i];

      op.amMacro.speed=reader.readC();
      op.arMacro.speed=reader.readC();
      op.drMacro.speed=reader.readC();
      op.multMacro.speed=reader.readC();
      op.rrMacro.speed=reader.readC();
      op.slMacro.speed=reader.readC();
      op.tlMacro.speed=reader.readC();
      op.dt2Macro.speed=reader.readC();
      op.rsMacro.speed=reader.readC();
      op.dtMacro.speed=reader.readC();
      op.d2rMacro.speed=reader.readC();
      op.ssgMacro.speed=reader.readC();
      op.damMacro.speed=reader.readC();
      op.dvbMacro.speed=reader.readC();
      op.egtMacro.speed=reader.readC();
      op.kslMacro.speed=reader.readC();
      op.susMacro.speed=reader.readC();
      op.vibMacro.speed=reader.readC();
      op.wsMacro.speed=reader.readC();
      op.ksrMacro.speed=reader.readC();

      op.amMacro.delay=reader.readC();
      op.arMacro.delay=reader.readC();
      op.drMacro.delay=reader.readC();
      op.multMacro.delay=reader.readC();
      op.rrMacro.delay=reader.readC();
      op.slMacro.delay=reader.readC();
      op.tlMacro.delay=reader.readC();
      op.dt2Macro.delay=reader.readC();
      op.rsMacro.delay=reader.readC();
      op.dtMacro.delay=reader.readC();
      op.d2rMacro.delay=reader.readC();
      op.ssgMacro.delay=reader.readC();
      op.damMacro.delay=reader.readC();
      op.dvbMacro.delay=reader.readC();
      op.egtMacro.delay=reader.readC();
      op.kslMacro.delay=reader.readC();
      op.susMacro.delay=reader.readC();
      op.vibMacro.delay=reader.readC();
      op.wsMacro.delay=reader.readC();
      op.ksrMacro.delay=reader.readC();
    }
  }

  // old arp macro format
  if (version<112) {
    if (std.arpMacro.mode) {
      std.arpMacro.mode=0;
      for (int i=0; i<std.arpMacro.len; i++) {
        std.arpMacro.val[i]^=0x40000000;
      }
      if ((std.arpMacro.loop>=std.arpMacro.len || (std.arpMacro.rel>std.arpMacro.loop && std.arpMacro.rel<std.arpMacro.len)) && std.arpMacro.len<255) {
        std.arpMacro.val[std.arpMacro.len++]=0;
      }
    }
  }

  // <167 TL macro compat
  if (version<167) {
    for (int i=0; i<4; i++) {
      if (std.opMacros[i].tlMacro.open&6) {
          for (int j=0; j<2; j++) {
          std.opMacros[i].tlMacro.val[j]^=0x7f;
        }
      } else {
        for (int j=0; j<std.opMacros[i].tlMacro.len; j++) {
          std.opMacros[i].tlMacro.val[j]^=0x7f;
        }
      }
    }
  }

  // <187 C64 cutoff macro compatibility
  if (type==DIV_INS_C64 && volIsCutoff && version<187) {
    memcpy(&std.algMacro,&std.volMacro,sizeof(DivInstrumentMacro));
    std.algMacro.macroType=DIV_MACRO_ALG;
    std.volMacro=DivInstrumentMacro(DIV_MACRO_VOL,true);

    if (!c64.filterIsAbs) {
      for (int i=0; i<std.algMacro.len; i++) {
        std.algMacro.val[i]=-std.algMacro.val[i];
      }
    }
  }

  // <187 special/test/gate merge
  if (type==DIV_INS_C64 && version<187) {
    convertC64SpecialMacro();
  }

  return DIV_DATA_SUCCESS;
}

DivDataErrors DivInstrument::readInsData(SafeReader& reader, short version, DivSong* song) {
  // 0: old (INST)
  // 1: new (INS2, length)
  // 2: new (FINS, no length)
  int type=-1;

  char magic[4];
  reader.read(magic,4);
  if (memcmp(magic,"INST",4)==0) {
    type=0;
  } else if (memcmp(magic,"INS2",4)==0) {
    type=1;
  } else if (memcmp(magic,"IN2B",4)==0) { // DIV_FUR_VARIANT_B
    type=1;
  } else if (memcmp(magic,"FINS",4)==0) {
    type=2;
  } else if (memcmp(magic,"FINB",4)==0) { // DIV_FUR_VARIANT_B
    type=2;
  } else {
    logE("invalid instrument header!");
    return DIV_DATA_INVALID_HEADER;
  }

  if (type==1 || type==2) {
    logV("reading new instrument data...");
    return readInsDataNew(reader,version,type==2,song);
  }
  return readInsDataOld(reader,version);
}

void DivInstrument::convertC64SpecialMacro() {
  // merge special and test/gate macros into new special macro
  int maxLen=MAX(std.ex3Macro.len,std.ex4Macro.len);

  // skip if ex4 is not a sequence macro
  if (std.ex4Macro.open&6) return;

  // move ex4 macro up and fill in gate
  for (int i=0; i<std.ex4Macro.len; i++) {
    std.ex4Macro.val[i]=(std.ex4Macro.val[i]&1)?9:1;
  }
  
  // merge ex3 into ex4 if viable to
  if (std.ex3Macro.len>0 && !(std.ex3Macro.open&6)) {
    if (std.ex4Macro.len>0 && std.ex4Macro.len<maxLen) {
      for (int i=std.ex4Macro.len; i<maxLen; i++) {
        std.ex4Macro.val[i]=std.ex3Macro.val[std.ex4Macro.len-1];
      }
    } else {
      for (int i=0; i<maxLen; i++) {
        std.ex4Macro.val[i]=1;
      }
    }
    for (int i=0; i<maxLen; i++) {
      if (i>=std.ex3Macro.len) {
        std.ex4Macro.val[i]|=(std.ex3Macro.val[std.ex3Macro.len-1]&3)<<1;
      } else {
        std.ex4Macro.val[i]|=(std.ex3Macro.val[i]&3)<<1;
      }
    }
  }
  std.ex4Macro.len=maxLen;

  std.ex3Macro=DivInstrumentMacro(DIV_MACRO_EX3);
}

bool DivInstrument::save(const char* path, DivSong* song, bool writeInsName) {
  SafeWriter* w=new SafeWriter();
  w->init();

  putInsData2(w,true,song,writeInsName);

  FILE* outFile=ps_fopen(path,"wb");
  if (outFile==NULL) {
    logE("could not save instrument: %s!",strerror(errno));
    w->finish();
    return false;
  }
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logW("did not write entire instrument!");
  }
  fclose(outFile);
  w->finish();
  return true;
}

bool DivInstrument::saveDMP(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // write version
  w->writeC(11);

  // guess the system
  switch (type) {
    case DIV_INS_FM:
      // we can't tell Genesis and Neo Geo apart
      w->writeC(0x02);
      w->writeC(1);
      break;
    case DIV_INS_STD:
      w->writeC(0x03);
      w->writeC(0);
      break;
    case DIV_INS_NES:
      w->writeC(0x06);
      w->writeC(0);
      break;
    case DIV_INS_GB:
      w->writeC(0x04);
      w->writeC(0);
      break;
    case DIV_INS_C64:
      w->writeC(0x07);
      w->writeC(0);
      break;
    case DIV_INS_PCE:
      w->writeC(0x05);
      w->writeC(0);
      break;
    case DIV_INS_OPLL:
      // ???
      w->writeC(0x13);
      w->writeC(1);
      break;
    case DIV_INS_OPM:
      w->writeC(0x08);
      w->writeC(1);
      break;
    case DIV_INS_OPZ:
      // data will be lost
      w->writeC(0x08);
      w->writeC(1);
      break;
    case DIV_INS_FDS:
      // ???
      w->writeC(0x06);
      w->writeC(0);
      break;
    default:
      // not supported by .dmp
      w->finish();
      return false;
  }

  if (type==DIV_INS_FM || type==DIV_INS_OPM || type==DIV_INS_OPLL || type==DIV_INS_OPZ) {
    w->writeC(fm.fms);
    w->writeC(fm.fb);
    w->writeC(fm.alg);
    w->writeC(fm.ams);

    // TODO: OPLL params
    for (int i=0; i<4; i++) {
      DivInstrumentFM::Operator& op=fm.op[i];
      w->writeC(op.mult);
      w->writeC(op.tl);
      w->writeC(op.ar);
      w->writeC(op.dr);
      w->writeC(op.sl);
      w->writeC(op.rr);
      w->writeC(op.am);
      w->writeC(op.rs);
      w->writeC(op.dt|(op.dt2<<4));
      w->writeC(op.d2r);
      w->writeC(op.ssgEnv);
    }
  } else {
    if (type!=DIV_INS_GB) {
      w->writeC(std.volMacro.len);
      for (int i=0; i<std.volMacro.len; i++) {
        w->writeI(std.volMacro.val[i]);
      }
      if (std.volMacro.len>0) w->writeC(std.volMacro.loop);
    }

    bool arpMacroMode=false;
    int arpMacroHowManyFixed=0;
    int realArpMacroLen=std.arpMacro.len;
    for (int j=0; j<std.arpMacro.len; j++) {
      if ((std.arpMacro.val[j]&0xc0000000)==0x40000000 || (std.arpMacro.val[j]&0xc0000000)==0x80000000) {
        arpMacroHowManyFixed++;
      }
    }
    if (arpMacroHowManyFixed>=std.arpMacro.len-1) {
      arpMacroMode=true;
    }
    if (std.arpMacro.len>0) {
      if (arpMacroMode && std.arpMacro.val[std.arpMacro.len-1]==0 && std.arpMacro.loop>=std.arpMacro.len) {
        realArpMacroLen--;
      }
    }

    if (realArpMacroLen>127) realArpMacroLen=127;

    w->writeC(realArpMacroLen);

    if (arpMacroMode) {
      for (int j=0; j<realArpMacroLen; j++) {
        if ((std.arpMacro.val[j]&0xc0000000)==0x40000000 || (std.arpMacro.val[j]&0xc0000000)==0x80000000) {
          w->writeI(std.arpMacro.val[j]^0x40000000);
        } else {
          w->writeI(std.arpMacro.val[j]);
        }
      }
    } else {
      for (int j=0; j<realArpMacroLen; j++) {
        if ((std.arpMacro.val[j]&0xc0000000)==0x40000000 || (std.arpMacro.val[j]&0xc0000000)==0x80000000) {
          w->writeI((std.arpMacro.val[j]^0x40000000)+12);
        } else {
          w->writeI(std.arpMacro.val[j]+12);
        }
      }
    }
    if (realArpMacroLen>0) {
      w->writeC(std.arpMacro.loop);
    }
    w->writeC(arpMacroMode);

    w->writeC(std.dutyMacro.len);
    for (int i=0; i<std.dutyMacro.len; i++) {
      w->writeI(std.dutyMacro.val[i]);
    }
    if (std.dutyMacro.len>0) w->writeC(std.dutyMacro.loop);

    w->writeC(std.waveMacro.len);
    for (int i=0; i<std.waveMacro.len; i++) {
      if (type==DIV_INS_AY) {
        w->writeI(std.waveMacro.val[i]-1);
      } else {
        w->writeI(std.waveMacro.val[i]);
      }
    }
    if (std.waveMacro.len>0) w->writeC(std.waveMacro.loop);

    if (type==DIV_INS_C64) {
      w->writeC(c64.triOn);
      w->writeC(c64.sawOn);
      w->writeC(c64.pulseOn);
      w->writeC(c64.noiseOn);
      w->writeC(c64.a);
      w->writeC(c64.d);
      w->writeC(c64.s);
      w->writeC(c64.r);
      w->writeC((c64.duty*100)/4095);
      w->writeC(c64.ringMod);
      w->writeC(c64.oscSync);
      w->writeC(c64.toFilter);
      w->writeC(0); // this was volIsCutoff...
      w->writeC(c64.initFilter);
      w->writeC(c64.res);
      w->writeC((c64.cut*100)/2047);
      w->writeC(c64.hp);
      w->writeC(c64.lp);
      w->writeC(c64.bp);
      w->writeC(c64.ch3off);
    }
    if (type==DIV_INS_GB) {
      w->writeC(gb.envVol);
      w->writeC(gb.envDir);
      w->writeC(gb.envLen);
      w->writeC(gb.soundLen);
    }
  }

  FILE* outFile=ps_fopen(path,"wb");
  if (outFile==NULL) {
    logE("could not save instrument: %s!",strerror(errno));
    w->finish();
    return false;
  }
  if (fwrite(w->getFinalBuf(),1,w->size(),outFile)!=w->size()) {
    logW("did not write entire instrument!");
  }
  fclose(outFile);
  w->finish();
  return true;
}

DivInstrument::~DivInstrument() {
  // free undoHist/redoHist
  while (!undoHist.empty()) {
    delete undoHist.back();
    undoHist.pop_back();
  }
  while (!redoHist.empty()) {
    delete redoHist.back();
    redoHist.pop_back();
  }
}

DivInstrument::DivInstrument( const DivInstrument& ins ) {
  // undo/redo history is specifically not copied
  *(DivInstrumentPOD*)this=ins;
  name=ins.name;
}

DivInstrument& DivInstrument::operator=( const DivInstrument& ins ) {
  // undo/redo history is specifically not copied
  *(DivInstrumentPOD*)this=ins;
  name=ins.name;
  return *this;
}
