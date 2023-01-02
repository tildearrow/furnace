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
    _C(alwaysInit)
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
    _C(volIsCutoff) &&
    _C(initFilter) &&
    _C(dutyIsAbs) &&
    _C(filterIsAbs) &&
    _C(noTest) &&
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
    _C(waveMode)
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
    _C(am)
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
  return _C(switchRoles);
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

#undef _C

#define FEATURE_BEGIN(x) \
  logV("- %s",x); \
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

void DivInstrument::writeMacro(SafeWriter* w, const DivInstrumentMacro& m, unsigned char macroCode) {
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

  w->writeC(macroCode);
  w->writeC(m.len);
  w->writeC(m.loop);
  w->writeC(m.rel);
  w->writeC(m.mode);
  w->writeC(m.open|wordSize);
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
  writeMacro(w,std.volMacro,0);
  writeMacro(w,std.arpMacro,1);
  writeMacro(w,std.dutyMacro,2);
  writeMacro(w,std.waveMacro,3);
  writeMacro(w,std.pitchMacro,4);
  writeMacro(w,std.ex1Macro,5);
  writeMacro(w,std.ex2Macro,6);
  writeMacro(w,std.ex3Macro,7);
  writeMacro(w,std.algMacro,8);
  writeMacro(w,std.fbMacro,9);
  writeMacro(w,std.fmsMacro,10);
  writeMacro(w,std.amsMacro,11);
  writeMacro(w,std.panLMacro,12);
  writeMacro(w,std.panRMacro,13);
  writeMacro(w,std.phaseResetMacro,14);
  writeMacro(w,std.ex4Macro,15);
  writeMacro(w,std.ex5Macro,16);
  writeMacro(w,std.ex6Macro,17);
  writeMacro(w,std.ex7Macro,18);
  writeMacro(w,std.ex8Macro,19);

  // "stop reading" code
  w->writeC(-1);

  FEATURE_END;
}

void DivInstrument::writeFeature64(SafeWriter* w) {
  FEATURE_BEGIN("64");

  w->writeC(
    (c64.dutyIsAbs?0x80:0)|
    (c64.initFilter?0x40:0)|
    (c64.volIsCutoff?0x20:0)|
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
  w->writeS((unsigned short)((c64.cut&2047)|(c64.res<<12)));

  FEATURE_END;
}

void DivInstrument::writeFeatureGB(SafeWriter* w) {
  FEATURE_BEGIN("GB");

  w->writeC(((gb.envLen&7)<<5)|(gb.envDir?16:0)|(gb.envVol&15));
  w->writeC(gb.soundLen);

  w->writeC(
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

  writeMacro(w,o.amMacro,0);
  writeMacro(w,o.arMacro,1);
  writeMacro(w,o.drMacro,2);
  writeMacro(w,o.multMacro,3);
  writeMacro(w,o.rrMacro,4);
  writeMacro(w,o.slMacro,5);
  writeMacro(w,o.tlMacro,6);
  writeMacro(w,o.dt2Macro,7);
  writeMacro(w,o.rsMacro,8);
  writeMacro(w,o.dtMacro,9);
  writeMacro(w,o.d2rMacro,10);
  writeMacro(w,o.ssgMacro,11);
  writeMacro(w,o.damMacro,12);
  writeMacro(w,o.dvbMacro,13);
  writeMacro(w,o.egtMacro,14);
  writeMacro(w,o.kslMacro,15);
  writeMacro(w,o.susMacro,16);
  writeMacro(w,o.vibMacro,17);
  writeMacro(w,o.wsMacro,18);
  writeMacro(w,o.ksrMacro,19);

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

  FEATURE_END;
}

void DivInstrument::writeFeatureSU(SafeWriter* w) {
  FEATURE_BEGIN("SU");

  w->writeC(su.switchRoles);

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

void DivInstrument::putInsData2(SafeWriter* w, bool fui, const DivSong* song) {
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
  }

  // check ins name
  if (!name.empty()) {
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

  if (featureFM || !fui) {
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

void DivInstrument::putInsData(SafeWriter* w) {
  size_t blockStartSeek, blockEndSeek;

  w->write("INST",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeS(DIV_ENGINE_VERSION);

  w->writeC(type);
  w->writeC(0);

  w->writeString(name,false);

  // FM
  w->writeC(fm.alg);
  w->writeC(fm.fb);
  w->writeC(fm.fms);
  w->writeC(fm.ams);
  w->writeC(fm.ops);
  w->writeC(fm.opllPreset);
  w->writeC(0); // reserved
  w->writeC(0);

  for (int j=0; j<4; j++) {
    DivInstrumentFM::Operator& op=fm.op[j];
    w->writeC(op.am);
    w->writeC(op.ar);
    w->writeC(op.dr);
    w->writeC(op.mult);
    w->writeC(op.rr);
    w->writeC(op.sl);
    w->writeC(op.tl);
    w->writeC(op.dt2);
    w->writeC(op.rs);
    w->writeC(op.dt);
    w->writeC(op.d2r);
    w->writeC(op.ssgEnv);

    w->writeC(op.dam);
    w->writeC(op.dvb);
    w->writeC(op.egt);
    w->writeC(op.ksl);
    w->writeC(op.sus);
    w->writeC(op.vib);
    w->writeC(op.ws);
    w->writeC(op.ksr);

    w->writeC(op.enable);
    w->writeC(op.kvs);

    // reserved
    for (int k=0; k<10; k++) {
      w->writeC(0);
    }
  }

  // GB
  w->writeC(gb.envVol);
  w->writeC(gb.envDir);
  w->writeC(gb.envLen);
  w->writeC(gb.soundLen);

  // C64
  w->writeC(c64.triOn);
  w->writeC(c64.sawOn);
  w->writeC(c64.pulseOn);
  w->writeC(c64.noiseOn);
  w->writeC(c64.a);
  w->writeC(c64.d);
  w->writeC(c64.s);
  w->writeC(c64.r);
  w->writeS(c64.duty);
  w->writeC(c64.ringMod);
  w->writeC(c64.oscSync);
  w->writeC(c64.toFilter);
  w->writeC(c64.initFilter);
  w->writeC(c64.volIsCutoff);
  w->writeC(c64.res);
  w->writeC(c64.lp);
  w->writeC(c64.bp);
  w->writeC(c64.hp);
  w->writeC(c64.ch3off);
  w->writeS(c64.cut);
  w->writeC(c64.dutyIsAbs);
  w->writeC(c64.filterIsAbs);
  
  // Amiga
  w->writeS(amiga.initSample);
  w->writeC(amiga.useWave);
  w->writeC(amiga.waveLen);
  for (int j=0; j<12; j++) { // reserved
    w->writeC(0);
  }

  // standard
  w->writeI(std.volMacro.len);
  w->writeI(std.arpMacro.len);
  w->writeI(std.dutyMacro.len);
  w->writeI(std.waveMacro.len);
  w->writeI(std.pitchMacro.len);
  w->writeI(std.ex1Macro.len);
  w->writeI(std.ex2Macro.len);
  w->writeI(std.ex3Macro.len);
  w->writeI(std.volMacro.loop);
  w->writeI(std.arpMacro.loop);
  w->writeI(std.dutyMacro.loop);
  w->writeI(std.waveMacro.loop);
  w->writeI(std.pitchMacro.loop);
  w->writeI(std.ex1Macro.loop);
  w->writeI(std.ex2Macro.loop);
  w->writeI(std.ex3Macro.loop);
  w->writeC(0); // this was arp macro mode
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  for (int j=0; j<std.volMacro.len; j++) {
    w->writeI(std.volMacro.val[j]);
  }
  for (int j=0; j<std.arpMacro.len; j++) {
    w->writeI(std.arpMacro.val[j]);
  }
  for (int j=0; j<std.dutyMacro.len; j++) {
    w->writeI(std.dutyMacro.val[j]);
  }
  for (int j=0; j<std.waveMacro.len; j++) {
    w->writeI(std.waveMacro.val[j]);
  }
  for (int j=0; j<std.pitchMacro.len; j++) {
    w->writeI(std.pitchMacro.val[j]);
  }
  for (int j=0; j<std.ex1Macro.len; j++) {
    w->writeI(std.ex1Macro.val[j]);
  }
  for (int j=0; j<std.ex2Macro.len; j++) {
    w->writeI(std.ex2Macro.val[j]);
  }
  for (int j=0; j<std.ex3Macro.len; j++) {
    w->writeI(std.ex3Macro.val[j]);
  }

  // FM macros and open status
  w->writeI(std.algMacro.len);
  w->writeI(std.fbMacro.len);
  w->writeI(std.fmsMacro.len);
  w->writeI(std.amsMacro.len);
  w->writeI(std.algMacro.loop);
  w->writeI(std.fbMacro.loop);
  w->writeI(std.fmsMacro.loop);
  w->writeI(std.amsMacro.loop);

  w->writeC(std.volMacro.open);
  w->writeC(std.arpMacro.open);
  w->writeC(std.dutyMacro.open);
  w->writeC(std.waveMacro.open);
  w->writeC(std.pitchMacro.open);
  w->writeC(std.ex1Macro.open);
  w->writeC(std.ex2Macro.open);
  w->writeC(std.ex3Macro.open);
  w->writeC(std.algMacro.open);
  w->writeC(std.fbMacro.open);
  w->writeC(std.fmsMacro.open);
  w->writeC(std.amsMacro.open);

  for (int j=0; j<std.algMacro.len; j++) {
    w->writeI(std.algMacro.val[j]);
  }
  for (int j=0; j<std.fbMacro.len; j++) {
    w->writeI(std.fbMacro.val[j]);
  }
  for (int j=0; j<std.fmsMacro.len; j++) {
    w->writeI(std.fmsMacro.val[j]);
  }
  for (int j=0; j<std.amsMacro.len; j++) {
    w->writeI(std.amsMacro.val[j]);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.amMacro.len);
    w->writeI(op.arMacro.len);
    w->writeI(op.drMacro.len);
    w->writeI(op.multMacro.len);
    w->writeI(op.rrMacro.len);
    w->writeI(op.slMacro.len);
    w->writeI(op.tlMacro.len);
    w->writeI(op.dt2Macro.len);
    w->writeI(op.rsMacro.len);
    w->writeI(op.dtMacro.len);
    w->writeI(op.d2rMacro.len);
    w->writeI(op.ssgMacro.len);
    w->writeI(op.amMacro.loop);
    w->writeI(op.arMacro.loop);
    w->writeI(op.drMacro.loop);
    w->writeI(op.multMacro.loop);
    w->writeI(op.rrMacro.loop);
    w->writeI(op.slMacro.loop);
    w->writeI(op.tlMacro.loop);
    w->writeI(op.dt2Macro.loop);
    w->writeI(op.rsMacro.loop);
    w->writeI(op.dtMacro.loop);
    w->writeI(op.d2rMacro.loop);
    w->writeI(op.ssgMacro.loop);
    w->writeC(op.amMacro.open);
    w->writeC(op.arMacro.open);
    w->writeC(op.drMacro.open);
    w->writeC(op.multMacro.open);
    w->writeC(op.rrMacro.open);
    w->writeC(op.slMacro.open);
    w->writeC(op.tlMacro.open);
    w->writeC(op.dt2Macro.open);
    w->writeC(op.rsMacro.open);
    w->writeC(op.dtMacro.open);
    w->writeC(op.d2rMacro.open);
    w->writeC(op.ssgMacro.open);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];
    for (int j=0; j<op.amMacro.len; j++) {
      w->writeC(op.amMacro.val[j]&0xff);
    }
    for (int j=0; j<op.arMacro.len; j++) {
      w->writeC(op.arMacro.val[j]&0xff);
    }
    for (int j=0; j<op.drMacro.len; j++) {
      w->writeC(op.drMacro.val[j]&0xff);
    }
    for (int j=0; j<op.multMacro.len; j++) {
      w->writeC(op.multMacro.val[j]&0xff);
    }
    for (int j=0; j<op.rrMacro.len; j++) {
      w->writeC(op.rrMacro.val[j]&0xff);
    }
    for (int j=0; j<op.slMacro.len; j++) {
      w->writeC(op.slMacro.val[j]&0xff);
    }
    for (int j=0; j<op.tlMacro.len; j++) {
      w->writeC(op.tlMacro.val[j]&0xff);
    }
    for (int j=0; j<op.dt2Macro.len; j++) {
      w->writeC(op.dt2Macro.val[j]&0xff);
    }
    for (int j=0; j<op.rsMacro.len; j++) {
      w->writeC(op.rsMacro.val[j]&0xff);
    }
    for (int j=0; j<op.dtMacro.len; j++) {
      w->writeC(op.dtMacro.val[j]&0xff);
    }
    for (int j=0; j<op.d2rMacro.len; j++) {
      w->writeC(op.d2rMacro.val[j]&0xff);
    }
    for (int j=0; j<op.ssgMacro.len; j++) {
      w->writeC(op.ssgMacro.val[j]&0xff);
    }
  }

  // release points
  w->writeI(std.volMacro.rel);
  w->writeI(std.arpMacro.rel);
  w->writeI(std.dutyMacro.rel);
  w->writeI(std.waveMacro.rel);
  w->writeI(std.pitchMacro.rel);
  w->writeI(std.ex1Macro.rel);
  w->writeI(std.ex2Macro.rel);
  w->writeI(std.ex3Macro.rel);
  w->writeI(std.algMacro.rel);
  w->writeI(std.fbMacro.rel);
  w->writeI(std.fmsMacro.rel);
  w->writeI(std.amsMacro.rel);
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.amMacro.rel);
    w->writeI(op.arMacro.rel);
    w->writeI(op.drMacro.rel);
    w->writeI(op.multMacro.rel);
    w->writeI(op.rrMacro.rel);
    w->writeI(op.slMacro.rel);
    w->writeI(op.tlMacro.rel);
    w->writeI(op.dt2Macro.rel);
    w->writeI(op.rsMacro.rel);
    w->writeI(op.dtMacro.rel);
    w->writeI(op.d2rMacro.rel);
    w->writeI(op.ssgMacro.rel);
  }

  // extended op macros
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeI(op.damMacro.len);
    w->writeI(op.dvbMacro.len);
    w->writeI(op.egtMacro.len);
    w->writeI(op.kslMacro.len);
    w->writeI(op.susMacro.len);
    w->writeI(op.vibMacro.len);
    w->writeI(op.wsMacro.len);
    w->writeI(op.ksrMacro.len);
    
    w->writeI(op.damMacro.loop);
    w->writeI(op.dvbMacro.loop);
    w->writeI(op.egtMacro.loop);
    w->writeI(op.kslMacro.loop);
    w->writeI(op.susMacro.loop);
    w->writeI(op.vibMacro.loop);
    w->writeI(op.wsMacro.loop);
    w->writeI(op.ksrMacro.loop);

    w->writeI(op.damMacro.rel);
    w->writeI(op.dvbMacro.rel);
    w->writeI(op.egtMacro.rel);
    w->writeI(op.kslMacro.rel);
    w->writeI(op.susMacro.rel);
    w->writeI(op.vibMacro.rel);
    w->writeI(op.wsMacro.rel);
    w->writeI(op.ksrMacro.rel);

    w->writeC(op.damMacro.open);
    w->writeC(op.dvbMacro.open);
    w->writeC(op.egtMacro.open);
    w->writeC(op.kslMacro.open);
    w->writeC(op.susMacro.open);
    w->writeC(op.vibMacro.open);
    w->writeC(op.wsMacro.open);
    w->writeC(op.ksrMacro.open);
  }

  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];
    for (int j=0; j<op.damMacro.len; j++) {
      w->writeC(op.damMacro.val[j]);
    }
    for (int j=0; j<op.dvbMacro.len; j++) {
      w->writeC(op.dvbMacro.val[j]);
    }
    for (int j=0; j<op.egtMacro.len; j++) {
      w->writeC(op.egtMacro.val[j]);
    }
    for (int j=0; j<op.kslMacro.len; j++) {
      w->writeC(op.kslMacro.val[j]);
    }
    for (int j=0; j<op.susMacro.len; j++) {
      w->writeC(op.susMacro.val[j]);
    }
    for (int j=0; j<op.vibMacro.len; j++) {
      w->writeC(op.vibMacro.val[j]);
    }
    for (int j=0; j<op.wsMacro.len; j++) {
      w->writeC(op.wsMacro.val[j]);
    }
    for (int j=0; j<op.ksrMacro.len; j++) {
      w->writeC(op.ksrMacro.val[j]);
    }
  }

  // OPL drum data
  w->writeC(fm.fixedDrums);
  w->writeC(0); // reserved
  w->writeS(fm.kickFreq);
  w->writeS(fm.snareHatFreq);
  w->writeS(fm.tomTopFreq);

  // sample map
  w->writeC(amiga.useNoteMap);
  if (amiga.useNoteMap) {
    for (int note=0; note<120; note++) {
      w->writeI(amiga.noteMap[note].freq);
    }
    for (int note=0; note<120; note++) {
      w->writeS(amiga.noteMap[note].map);
    }
  }

  // N163
  w->writeI(n163.wave);
  w->writeC(n163.wavePos);
  w->writeC(n163.waveLen);
  w->writeC(n163.waveMode);
  w->writeC(0); // reserved

  // more macros
  w->writeI(std.panLMacro.len);
  w->writeI(std.panRMacro.len);
  w->writeI(std.phaseResetMacro.len);
  w->writeI(std.ex4Macro.len);
  w->writeI(std.ex5Macro.len);
  w->writeI(std.ex6Macro.len);
  w->writeI(std.ex7Macro.len);
  w->writeI(std.ex8Macro.len);
  
  w->writeI(std.panLMacro.loop);
  w->writeI(std.panRMacro.loop);
  w->writeI(std.phaseResetMacro.loop);
  w->writeI(std.ex4Macro.loop);
  w->writeI(std.ex5Macro.loop);
  w->writeI(std.ex6Macro.loop);
  w->writeI(std.ex7Macro.loop);
  w->writeI(std.ex8Macro.loop);

  w->writeI(std.panLMacro.rel);
  w->writeI(std.panRMacro.rel);
  w->writeI(std.phaseResetMacro.rel);
  w->writeI(std.ex4Macro.rel);
  w->writeI(std.ex5Macro.rel);
  w->writeI(std.ex6Macro.rel);
  w->writeI(std.ex7Macro.rel);
  w->writeI(std.ex8Macro.rel);

  w->writeC(std.panLMacro.open);
  w->writeC(std.panRMacro.open);
  w->writeC(std.phaseResetMacro.open);
  w->writeC(std.ex4Macro.open);
  w->writeC(std.ex5Macro.open);
  w->writeC(std.ex6Macro.open);
  w->writeC(std.ex7Macro.open);
  w->writeC(std.ex8Macro.open);

  for (int j=0; j<std.panLMacro.len; j++) {
    w->writeI(std.panLMacro.val[j]);
  }
  for (int j=0; j<std.panRMacro.len; j++) {
    w->writeI(std.panRMacro.val[j]);
  }
  for (int j=0; j<std.phaseResetMacro.len; j++) {
    w->writeI(std.phaseResetMacro.val[j]);
  }
  for (int j=0; j<std.ex4Macro.len; j++) {
    w->writeI(std.ex4Macro.val[j]);
  }
  for (int j=0; j<std.ex5Macro.len; j++) {
    w->writeI(std.ex5Macro.val[j]);
  }
  for (int j=0; j<std.ex6Macro.len; j++) {
    w->writeI(std.ex6Macro.val[j]);
  }
  for (int j=0; j<std.ex7Macro.len; j++) {
    w->writeI(std.ex7Macro.val[j]);
  }
  for (int j=0; j<std.ex8Macro.len; j++) {
    w->writeI(std.ex8Macro.val[j]);
  }

  // FDS
  w->writeI(fds.modSpeed);
  w->writeI(fds.modDepth);
  w->writeC(fds.initModTableWithFirstWave);
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  w->write(fds.modTable,32);

  // OPZ
  w->writeC(fm.fms2);
  w->writeC(fm.ams2);
  
  // wave synth
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

  // other macro modes
  w->writeC(std.volMacro.mode);
  w->writeC(std.dutyMacro.mode);
  w->writeC(std.waveMacro.mode);
  w->writeC(std.pitchMacro.mode);
  w->writeC(std.ex1Macro.mode);
  w->writeC(std.ex2Macro.mode);
  w->writeC(std.ex3Macro.mode);
  w->writeC(std.algMacro.mode);
  w->writeC(std.fbMacro.mode);
  w->writeC(std.fmsMacro.mode);
  w->writeC(std.amsMacro.mode);
  w->writeC(std.panLMacro.mode);
  w->writeC(std.panRMacro.mode);
  w->writeC(std.phaseResetMacro.mode);
  w->writeC(std.ex4Macro.mode);
  w->writeC(std.ex5Macro.mode);
  w->writeC(std.ex6Macro.mode);
  w->writeC(std.ex7Macro.mode);
  w->writeC(std.ex8Macro.mode);

  // C64 no test
  w->writeC(c64.noTest);

  // MultiPCM
  w->writeC(multipcm.ar);
  w->writeC(multipcm.d1r);
  w->writeC(multipcm.dl);
  w->writeC(multipcm.d2r);
  w->writeC(multipcm.rr);
  w->writeC(multipcm.rc);
  w->writeC(multipcm.lfo);
  w->writeC(multipcm.vib);
  w->writeC(multipcm.am);
  for (int j=0; j<23; j++) { // reserved
    w->writeC(0);
  }

  // Sound Unit
  w->writeC(amiga.useSample);
  w->writeC(su.switchRoles);

  // GB hardware sequence
  w->writeC(gb.hwSeqLen);
  for (int i=0; i<gb.hwSeqLen; i++) {
    w->writeC(gb.hwSeq[i].cmd);
    w->writeS(gb.hwSeq[i].data);
  }

  // GB additional flags
  w->writeC(gb.softEnv);
  w->writeC(gb.alwaysInit);

  // ES5506
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
  
  // SNES
  w->writeC(snes.useEnv);
  w->writeC(snes.gainMode);
  w->writeC(snes.gain);
  w->writeC(snes.a);
  w->writeC(snes.d);
  w->writeC((snes.s&7)|(snes.sus?8:0));
  w->writeC(snes.r);

  // macro speed/delay
  w->writeC(std.volMacro.speed);
  w->writeC(std.arpMacro.speed);
  w->writeC(std.dutyMacro.speed);
  w->writeC(std.waveMacro.speed);
  w->writeC(std.pitchMacro.speed);
  w->writeC(std.ex1Macro.speed);
  w->writeC(std.ex2Macro.speed);
  w->writeC(std.ex3Macro.speed);
  w->writeC(std.algMacro.speed);
  w->writeC(std.fbMacro.speed);
  w->writeC(std.fmsMacro.speed);
  w->writeC(std.amsMacro.speed);
  w->writeC(std.panLMacro.speed);
  w->writeC(std.panRMacro.speed);
  w->writeC(std.phaseResetMacro.speed);
  w->writeC(std.ex4Macro.speed);
  w->writeC(std.ex5Macro.speed);
  w->writeC(std.ex6Macro.speed);
  w->writeC(std.ex7Macro.speed);
  w->writeC(std.ex8Macro.speed);

  w->writeC(std.volMacro.delay);
  w->writeC(std.arpMacro.delay);
  w->writeC(std.dutyMacro.delay);
  w->writeC(std.waveMacro.delay);
  w->writeC(std.pitchMacro.delay);
  w->writeC(std.ex1Macro.delay);
  w->writeC(std.ex2Macro.delay);
  w->writeC(std.ex3Macro.delay);
  w->writeC(std.algMacro.delay);
  w->writeC(std.fbMacro.delay);
  w->writeC(std.fmsMacro.delay);
  w->writeC(std.amsMacro.delay);
  w->writeC(std.panLMacro.delay);
  w->writeC(std.panRMacro.delay);
  w->writeC(std.phaseResetMacro.delay);
  w->writeC(std.ex4Macro.delay);
  w->writeC(std.ex5Macro.delay);
  w->writeC(std.ex6Macro.delay);
  w->writeC(std.ex7Macro.delay);
  w->writeC(std.ex8Macro.delay);

  // op macro speed/delay
  for (int i=0; i<4; i++) {
    DivInstrumentSTD::OpMacro& op=std.opMacros[i];

    w->writeC(op.amMacro.speed);
    w->writeC(op.arMacro.speed);
    w->writeC(op.drMacro.speed);
    w->writeC(op.multMacro.speed);
    w->writeC(op.rrMacro.speed);
    w->writeC(op.slMacro.speed);
    w->writeC(op.tlMacro.speed);
    w->writeC(op.dt2Macro.speed);
    w->writeC(op.rsMacro.speed);
    w->writeC(op.dtMacro.speed);
    w->writeC(op.d2rMacro.speed);
    w->writeC(op.ssgMacro.speed);
    w->writeC(op.damMacro.speed);
    w->writeC(op.dvbMacro.speed);
    w->writeC(op.egtMacro.speed);
    w->writeC(op.kslMacro.speed);
    w->writeC(op.susMacro.speed);
    w->writeC(op.vibMacro.speed);
    w->writeC(op.wsMacro.speed);
    w->writeC(op.ksrMacro.speed);

    w->writeC(op.amMacro.delay);
    w->writeC(op.arMacro.delay);
    w->writeC(op.drMacro.delay);
    w->writeC(op.multMacro.delay);
    w->writeC(op.rrMacro.delay);
    w->writeC(op.slMacro.delay);
    w->writeC(op.tlMacro.delay);
    w->writeC(op.dt2Macro.delay);
    w->writeC(op.rsMacro.delay);
    w->writeC(op.dtMacro.delay);
    w->writeC(op.d2rMacro.delay);
    w->writeC(op.ssgMacro.delay);
    w->writeC(op.damMacro.delay);
    w->writeC(op.dvbMacro.delay);
    w->writeC(op.egtMacro.delay);
    w->writeC(op.kslMacro.delay);
    w->writeC(op.susMacro.delay);
    w->writeC(op.vibMacro.delay);
    w->writeC(op.wsMacro.delay);
    w->writeC(op.ksrMacro.delay);
  }

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
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
  }

  READ_FEAT_END;
}

void DivInstrument::readFeature64(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  unsigned char next=reader.readC();
  c64.dutyIsAbs=next&128;
  c64.initFilter=next&64;
  c64.volIsCutoff=next&32;
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
  c64.cut=cr&2047;
  c64.res=cr>>12;

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
  }

  READ_FEAT_END;
}

void DivInstrument::readFeatureOx(SafeReader& reader, int op, short version) {
  READ_FEAT_BEGIN;

  unsigned short macroHeaderLen=reader.readS();

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

  reader.seek(lastSeek,SEEK_CUR);

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

  reader.seek(lastSeek,SEEK_CUR);

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

  READ_FEAT_END;
}

void DivInstrument::readFeatureSU(SafeReader& reader, short version) {
  READ_FEAT_BEGIN;

  su.switchRoles=reader.readC();

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

DivDataErrors DivInstrument::readInsDataNew(SafeReader& reader, short version, bool fui, DivSong* song) {
  unsigned char featCode[2];

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
      readFeature64(reader,version);
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

  return DIV_DATA_SUCCESS;
}

#define READ_MACRO_VALS(x,y) \
  for (int macroValPos=0; macroValPos<y; macroValPos++) x[macroValPos]=reader.readI();

DivDataErrors DivInstrument::readInsDataOld(SafeReader &reader, short version) {
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
  c64.volIsCutoff=reader.readC();
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
    if (c64.volIsCutoff && !c64.filterIsAbs) for (int j=0; j<std.volMacro.len; j++) {
      std.volMacro.val[j]-=18;
    }
    if (!c64.dutyIsAbs) for (int j=0; j<std.dutyMacro.len; j++) {
      std.dutyMacro.val[j]-=12;
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
  } else if (memcmp(magic,"FINS",4)==0) {
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

bool DivInstrument::save(const char* path, bool oldFormat, DivSong* song) {
  SafeWriter* w=new SafeWriter();
  w->init();

  if (oldFormat) {
    // write magic
    w->write("-Furnace instr.-",16);

    // write version
    w->writeS(DIV_ENGINE_VERSION);

    // reserved
    w->writeS(0);

    // pointer to data
    w->writeI(32);

    // currently reserved (TODO; wavetable and sample here)
    w->writeS(0);
    w->writeS(0);
    w->writeI(0);

    putInsData(w);
  } else {
    putInsData2(w,true,song);
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

bool DivInstrument::saveDMP(const char* path) {
  SafeWriter* w=new SafeWriter();
  w->init();

  // write version
  w->writeC(11);

  // guess the system
  switch (type) {
    case DIV_INS_FM:
      // we can't tell between Genesis, Neo Geo and Arcade ins type yet
      w->writeC(0x02);
      w->writeC(1);
      break;
    case DIV_INS_STD:
      // we can't tell between SMS and NES ins type yet
      w->writeC(0x03);
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
      w->writeC(0x06);
      w->writeC(0);
      break;
    case DIV_INS_OPLL:
      // ???
      w->writeC(0x13);
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

  if (type==DIV_INS_FM || type==DIV_INS_OPLL || type==DIV_INS_OPZ) {
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
      w->writeI(std.dutyMacro.val[i]+12);
    }
    if (std.dutyMacro.len>0) w->writeC(std.dutyMacro.loop);

    w->writeC(std.waveMacro.len);
    for (int i=0; i<std.waveMacro.len; i++) {
      w->writeI(std.waveMacro.val[i]+12);
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
      w->writeC(c64.volIsCutoff);
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
