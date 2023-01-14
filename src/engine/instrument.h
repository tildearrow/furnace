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

#ifndef _INSTRUMENT_H
#define _INSTRUMENT_H
#include "safeWriter.h"
#include "dataErrors.h"
#include "../ta-utils.h"
#include <vector>

struct DivSong;

// NOTICE!
// before adding new instrument types to this struct, please ask me first.
// absolutely zero support granted to conflicting formats.
enum DivInstrumentType: unsigned short {
  DIV_INS_STD=0,
  DIV_INS_FM=1,
  DIV_INS_GB=2,
  DIV_INS_C64=3,
  DIV_INS_AMIGA=4,
  DIV_INS_PCE=5,
  DIV_INS_AY=6,
  DIV_INS_AY8930=7,
  DIV_INS_TIA=8,
  DIV_INS_SAA1099=9,
  DIV_INS_VIC=10,
  DIV_INS_PET=11,
  DIV_INS_VRC6=12,
  DIV_INS_OPLL=13,
  DIV_INS_OPL=14,
  DIV_INS_FDS=15,
  DIV_INS_VBOY=16,
  DIV_INS_N163=17,
  DIV_INS_SCC=18,
  DIV_INS_OPZ=19,
  DIV_INS_POKEY=20,
  DIV_INS_BEEPER=21,
  DIV_INS_SWAN=22,
  DIV_INS_MIKEY=23,
  DIV_INS_VERA=24,
  DIV_INS_X1_010=25,
  DIV_INS_VRC6_SAW=26,
  DIV_INS_ES5506=27,
  DIV_INS_MULTIPCM=28,
  DIV_INS_SNES=29,
  DIV_INS_SU=30,
  DIV_INS_NAMCO=31,
  DIV_INS_OPL_DRUMS=32,
  DIV_INS_OPM=33,
  DIV_INS_NES=34,
  DIV_INS_MSM6258=35,
  DIV_INS_MSM6295=36,
  DIV_INS_ADPCMA=37,
  DIV_INS_ADPCMB=38,
  DIV_INS_SEGAPCM=39,
  DIV_INS_QSOUND=40,
  DIV_INS_YMZ280B=41,
  DIV_INS_RF5C68=42,
  DIV_INS_MSM5232=43,
  DIV_INS_T6W28=44,
  DIV_INS_K007232=45,
  DIV_INS_GA20=46,
  DIV_INS_POKEMINI=47,
  DIV_INS_MAX,
  DIV_INS_NULL
};

// FM operator structure:
// - OPN:
//   - AM, AR, DR, MULT, RR, SL, TL, RS, DT, D2R, SSG-EG
// - OPM:
//   - AM, AR, DR, MULT, RR, SL, TL, DT2, RS, DT, D2R
// - OPLL:
//   - AM, AR, DR, MULT, RR, SL, TL, SSG-EG&8 = EG-S
//   - KSL, VIB, KSR
// - OPL:
//   - AM, AR, DR, MULT, RR, SL, TL, SSG-EG&8 = EG-S
//   - KSL, VIB, WS (OPL2/3), KSR
// - OPZ:
//   - AM, AR, DR, MULT (CRS), RR, SL, TL, DT2, RS, DT, D2R
//   - WS, DVB = MULT (FINE), DAM = REV, KSL = EGShift, EGT = Fixed

struct DivInstrumentFM {
  unsigned char alg, fb, fms, ams, fms2, ams2, ops, opllPreset;
  bool fixedDrums;
  unsigned short kickFreq, snareHatFreq, tomTopFreq;

  bool operator==(const DivInstrumentFM& other);
  bool operator!=(const DivInstrumentFM& other) {
    return !(*this==other);
  }
  struct Operator {
    bool enable;
    unsigned char am, ar, dr, mult, rr, sl, tl, dt2, rs, dt, d2r, ssgEnv;
    unsigned char dam, dvb, egt, ksl, sus, vib, ws, ksr; // YMU759/OPL/OPZ
    unsigned char kvs;

    bool operator==(const Operator& other);
    bool operator!=(const Operator& other) {
      return !(*this==other);
    }
    Operator():
      enable(true),
      am(0),
      ar(0),
      dr(0),
      mult(0),
      rr(0),
      sl(0),
      tl(0),
      dt2(0),
      rs(0),
      dt(0),
      d2r(0),
      ssgEnv(0),
      dam(0),
      dvb(0),
      egt(0),
      ksl(0),
      sus(0),
      vib(0),
      ws(0),
      ksr(0),
      kvs(2) {}
  } op[4];
  DivInstrumentFM():
    alg(0),
    fb(0),
    fms(0),
    ams(0),
    fms2(0),
    ams2(0),
    ops(2),
    opllPreset(0),
    fixedDrums(false),
    kickFreq(0x520),
    snareHatFreq(0x550),
    tomTopFreq(0x1c0) {
    // default instrument
    fb=4;
    op[0].tl=42;
    op[0].ar=31;
    op[0].dr=8;
    op[0].sl=15;
    op[0].rr=3;
    op[0].mult=5;
    op[0].dt=5;

    op[2].tl=18;
    op[2].ar=31;
    op[2].dr=10;
    op[2].sl=15;
    op[2].rr=4;
    op[2].mult=1;
    op[2].dt=0;

    op[1].tl=48;
    op[1].ar=31;
    op[1].dr=4;
    op[1].sl=11;
    op[1].rr=1;
    op[1].mult=1;
    op[1].dt=5;

    op[3].tl=2;
    op[3].ar=31;
    op[3].dr=9;
    op[3].sl=15;
    op[3].rr=9;
    op[3].mult=1;
    op[3].dt=0;
  }
};

// this is getting out of hand
struct DivInstrumentMacro {
  String name;
  int val[256];
  unsigned int mode;
  unsigned char open;
  unsigned char len, delay, speed, loop, rel;
  
  // the following variables are used by the GUI and not saved in the file
  int vScroll, vZoom;
  int typeMemory[16];
  unsigned char lenMemory;

  explicit DivInstrumentMacro(const String& n, bool initOpen=false):
    name(n),
    mode(0),
    open(initOpen),
    len(0),
    delay(0),
    speed(1),
    loop(255),
    rel(255),
    vScroll(0),
    vZoom(-1),
    lenMemory(0) {
    memset(val,0,256*sizeof(int));
    memset(typeMemory,0,16*sizeof(int));
  }
};

struct DivInstrumentSTD {
  DivInstrumentMacro volMacro;
  DivInstrumentMacro arpMacro;
  DivInstrumentMacro dutyMacro;
  DivInstrumentMacro waveMacro;
  DivInstrumentMacro pitchMacro;
  DivInstrumentMacro ex1Macro;
  DivInstrumentMacro ex2Macro;
  DivInstrumentMacro ex3Macro;
  DivInstrumentMacro algMacro;
  DivInstrumentMacro fbMacro;
  DivInstrumentMacro fmsMacro;
  DivInstrumentMacro amsMacro;
  DivInstrumentMacro panLMacro;
  DivInstrumentMacro panRMacro;
  DivInstrumentMacro phaseResetMacro;
  DivInstrumentMacro ex4Macro;
  DivInstrumentMacro ex5Macro;
  DivInstrumentMacro ex6Macro;
  DivInstrumentMacro ex7Macro;
  DivInstrumentMacro ex8Macro;

  struct OpMacro {
    // ar, dr, mult, rr, sl, tl, dt2, rs, dt, d2r, ssgEnv;
    DivInstrumentMacro amMacro;
    DivInstrumentMacro arMacro;
    DivInstrumentMacro drMacro;
    DivInstrumentMacro multMacro;
    DivInstrumentMacro rrMacro;
    DivInstrumentMacro slMacro;
    DivInstrumentMacro tlMacro;
    DivInstrumentMacro dt2Macro;
    DivInstrumentMacro rsMacro;
    DivInstrumentMacro dtMacro;
    DivInstrumentMacro d2rMacro;
    DivInstrumentMacro ssgMacro;
    DivInstrumentMacro damMacro;
    DivInstrumentMacro dvbMacro;
    DivInstrumentMacro egtMacro;
    DivInstrumentMacro kslMacro;
    DivInstrumentMacro susMacro;
    DivInstrumentMacro vibMacro;
    DivInstrumentMacro wsMacro;
    DivInstrumentMacro ksrMacro;
    OpMacro():
      amMacro("am"), arMacro("ar"), drMacro("dr"), multMacro("mult"),
      rrMacro("rr"), slMacro("sl"), tlMacro("tl",true), dt2Macro("dt2"),
      rsMacro("rs"), dtMacro("dt"), d2rMacro("d2r"), ssgMacro("ssg"),
      damMacro("dam"), dvbMacro("dvb"), egtMacro("egt"), kslMacro("ksl"),
      susMacro("sus"), vibMacro("vib"), wsMacro("ws"), ksrMacro("ksr") {}
  } opMacros[4];
  DivInstrumentSTD():
    volMacro("vol",true),
    arpMacro("arp"),
    dutyMacro("duty"),
    waveMacro("wave"),
    pitchMacro("pitch"),
    ex1Macro("ex1"),
    ex2Macro("ex2"),
    ex3Macro("ex3"),
    algMacro("alg"),
    fbMacro("fb"),
    fmsMacro("fms"),
    amsMacro("ams"),
    panLMacro("panL"),
    panRMacro("panR"),
    phaseResetMacro("phaseReset"),
    ex4Macro("ex4"),
    ex5Macro("ex5"),
    ex6Macro("ex6"), 
    ex7Macro("ex7"),
    ex8Macro("ex8") {}
};

struct DivInstrumentGB {
  unsigned char envVol, envDir, envLen, soundLen, hwSeqLen;
  bool softEnv, alwaysInit;
  enum HWSeqCommands: unsigned char {
    DIV_GB_HWCMD_ENVELOPE=0,
    DIV_GB_HWCMD_SWEEP,
    DIV_GB_HWCMD_WAIT,
    DIV_GB_HWCMD_WAIT_REL,
    DIV_GB_HWCMD_LOOP,
    DIV_GB_HWCMD_LOOP_REL,

    DIV_GB_HWCMD_MAX
  };
  struct HWSeqCommand {
    unsigned char cmd;
    unsigned short data;
  } hwSeq[256];

  bool operator==(const DivInstrumentGB& other);
  bool operator!=(const DivInstrumentGB& other) {
    return !(*this==other);
  }

  DivInstrumentGB():
    envVol(15),
    envDir(0),
    envLen(2),
    soundLen(64),
    hwSeqLen(0),
    softEnv(false),
    alwaysInit(false) {
    memset(hwSeq,0,256*sizeof(int));
  }
};

struct DivInstrumentC64 {
  bool triOn, sawOn, pulseOn, noiseOn;
  unsigned char a, d, s, r;
  unsigned short duty;
  unsigned char ringMod, oscSync;
  bool toFilter, volIsCutoff, initFilter, dutyIsAbs, filterIsAbs, noTest;
  unsigned char res;
  unsigned short cut;
  bool hp, lp, bp, ch3off;

  bool operator==(const DivInstrumentC64& other);
  bool operator!=(const DivInstrumentC64& other) {
    return !(*this==other);
  }

  DivInstrumentC64():
    triOn(false),
    sawOn(true),
    pulseOn(false),
    noiseOn(false),
    a(0),
    d(8),
    s(0),
    r(0),
    duty(2048),
    ringMod(0),
    oscSync(0),
    toFilter(false),
    volIsCutoff(false),
    initFilter(false),
    dutyIsAbs(false),
    filterIsAbs(false),
    noTest(false),
    res(0),
    cut(0),
    hp(false),
    lp(false),
    bp(false),
    ch3off(false) {}
};

struct DivInstrumentAmiga {
  struct SampleMap {
    int freq;
    short map;
    SampleMap(int f=0, short m=-1):
      freq(f),
      map(m) {}
  };
  short initSample;
  bool useNoteMap;
  bool useSample;
  bool useWave;
  unsigned char waveLen;
  SampleMap noteMap[120];

  bool operator==(const DivInstrumentAmiga& other);
  bool operator!=(const DivInstrumentAmiga& other) {
    return !(*this==other);
  }

  /**
   * get the sample at specified note.
   * @return the sample.
   */
  inline short getSample(int note) {
    if (useNoteMap) {
      if (note<0) note=0;
      if (note>119) note=119;
      return noteMap[note].map;
    }
    return initSample;
  }

  /**
   * get the sample frequency at specified note.
   * @return the frequency, or -1 if not using note map.
   */
  inline int getFreq(int note) {
    if (useNoteMap) {
      if (note<0) note=0;
      if (note>119) note=119;
      return noteMap[note].freq;
    }
    return -1;
  }

  DivInstrumentAmiga():
    initSample(0),
    useNoteMap(false),
    useSample(false),
    useWave(false),
    waveLen(31) {
    for (SampleMap& elem: noteMap) {
      elem=SampleMap();
    }
  }
};

struct DivInstrumentX1_010 {
  int bankSlot;

  bool operator==(const DivInstrumentX1_010& other);
  bool operator!=(const DivInstrumentX1_010& other) {
    return !(*this==other);
  }

  DivInstrumentX1_010():
    bankSlot(0) {}
};

struct DivInstrumentN163 {
  int wave, wavePos, waveLen;
  unsigned char waveMode;

  bool operator==(const DivInstrumentN163& other);
  bool operator!=(const DivInstrumentN163& other) {
    return !(*this==other);
  }

  DivInstrumentN163():
    wave(-1),
    wavePos(0),
    waveLen(32),
    waveMode(3) {}
};

struct DivInstrumentFDS {
  signed char modTable[32];
  int modSpeed, modDepth;
  // this is here for compatibility.
  bool initModTableWithFirstWave;

  bool operator==(const DivInstrumentFDS& other);
  bool operator!=(const DivInstrumentFDS& other) {
    return !(*this==other);
  }

  DivInstrumentFDS():
    modSpeed(0),
    modDepth(0),
    initModTableWithFirstWave(false) {
    memset(modTable,0,32);
  }
};

struct DivInstrumentMultiPCM {
  unsigned char ar, d1r, dl, d2r, rr, rc;
  unsigned char lfo, vib, am;

  bool operator==(const DivInstrumentMultiPCM& other);
  bool operator!=(const DivInstrumentMultiPCM& other) {
    return !(*this==other);
  }

  DivInstrumentMultiPCM():
    ar(15), d1r(15), dl(0), d2r(0), rr(15), rc(15),
    lfo(0), vib(0), am(0) {
  }
};

enum DivWaveSynthEffects {
  DIV_WS_NONE=0,
  // one waveform effects
  DIV_WS_INVERT,
  DIV_WS_ADD,
  DIV_WS_SUBTRACT,
  DIV_WS_AVERAGE,
  DIV_WS_PHASE,
  DIV_WS_CHORUS,

  DIV_WS_SINGLE_MAX,
  
  // two waveform effects
  DIV_WS_NONE_DUAL=128,
  DIV_WS_WIPE,
  DIV_WS_FADE,
  DIV_WS_PING_PONG,
  DIV_WS_OVERLAY,
  DIV_WS_NEGATIVE_OVERLAY,
  DIV_WS_SLIDE,
  DIV_WS_MIX,
  DIV_WS_PHASE_MOD,

  DIV_WS_DUAL_MAX
};

struct DivInstrumentWaveSynth {
  int wave1, wave2;
  unsigned char rateDivider;
  unsigned char effect;
  bool oneShot, enabled, global;
  unsigned char speed, param1, param2, param3, param4;

  bool operator==(const DivInstrumentWaveSynth& other);
  bool operator!=(const DivInstrumentWaveSynth& other) {
    return !(*this==other);
  }

  DivInstrumentWaveSynth():
    wave1(0),
    wave2(0),
    rateDivider(1),
    effect(DIV_WS_NONE),
    oneShot(false),
    enabled(false),
    global(false),
    speed(0),
    param1(0),
    param2(0),
    param3(0),
    param4(0) {}
};

struct DivInstrumentSoundUnit {
  bool switchRoles;

  bool operator==(const DivInstrumentSoundUnit& other);
  bool operator!=(const DivInstrumentSoundUnit& other) {
    return !(*this==other);
  }

  DivInstrumentSoundUnit():
    switchRoles(false) {}
};

struct DivInstrumentES5506 {
  struct Filter {
    enum FilterMode: unsigned char { // filter mode for pole 4,3
      FILTER_MODE_HPK2_HPK2=0,
      FILTER_MODE_HPK2_LPK1,
      FILTER_MODE_LPK2_LPK2,
      FILTER_MODE_LPK2_LPK1,
    };
    FilterMode mode;
    unsigned short k1, k2;
    Filter():
      mode(FILTER_MODE_LPK2_LPK1),
      k1(0xffff),
      k2(0xffff) {}
  };
  struct Envelope {
    unsigned short ecount;
    signed char lVRamp, rVRamp;
    signed char k1Ramp, k2Ramp;
    bool k1Slow, k2Slow;
    Envelope():
      ecount(0),
      lVRamp(0),
      rVRamp(0),
      k1Ramp(0),
      k2Ramp(0),
      k1Slow(false),
      k2Slow(false) {}
  };
  Filter filter;
  Envelope envelope;

  bool operator==(const DivInstrumentES5506& other);
  bool operator!=(const DivInstrumentES5506& other) {
    return !(*this==other);
  }

  DivInstrumentES5506():
    filter(Filter()),
    envelope(Envelope()) {}
};

struct DivInstrumentSNES {
  enum GainMode: unsigned char {
    GAIN_MODE_DIRECT=0,
    GAIN_MODE_DEC_LINEAR=4,
    GAIN_MODE_DEC_LOG=5,
    GAIN_MODE_INC_LINEAR=6,
    GAIN_MODE_INC_INVLOG=7
  };
  bool useEnv;
  // 0: no sustain (key off = cut)
  // 1: sustain (R = d2; key off = dec linear to r)
  // 2: sustain (R = d2; key off = dec exponential to r)
  // 3: sustain (R = d2; key off = R to r)
  unsigned char sus;
  GainMode gainMode;
  unsigned char gain;
  unsigned char a, d, s, r, d2;

  bool operator==(const DivInstrumentSNES& other);
  bool operator!=(const DivInstrumentSNES& other) {
    return !(*this==other);
  }

  DivInstrumentSNES():
    useEnv(true),
    sus(0),
    gainMode(GAIN_MODE_DIRECT),
    gain(127),
    a(15),
    d(7),
    s(7),
    r(0),
    d2(0) {}
};

struct DivInstrument {
  String name;
  DivInstrumentType type;
  DivInstrumentFM fm;
  DivInstrumentSTD std;
  DivInstrumentGB gb;
  DivInstrumentC64 c64;
  DivInstrumentAmiga amiga;
  DivInstrumentX1_010 x1_010;
  DivInstrumentN163 n163;
  DivInstrumentFDS fds;
  DivInstrumentMultiPCM multipcm;
  DivInstrumentWaveSynth ws;
  DivInstrumentSoundUnit su;
  DivInstrumentES5506 es5506;
  DivInstrumentSNES snes;

  /**
   * these are internal functions.
   */
  void writeMacro(SafeWriter* w, const DivInstrumentMacro& m, unsigned char macroCode);
  void writeFeatureNA(SafeWriter* w);
  void writeFeatureFM(SafeWriter* w, bool fui);
  void writeFeatureMA(SafeWriter* w);
  void writeFeature64(SafeWriter* w);
  void writeFeatureGB(SafeWriter* w);
  void writeFeatureSM(SafeWriter* w);
  void writeFeatureOx(SafeWriter* w, int op);
  void writeFeatureLD(SafeWriter* w);
  void writeFeatureSN(SafeWriter* w);
  void writeFeatureN1(SafeWriter* w);
  void writeFeatureFD(SafeWriter* w);
  void writeFeatureWS(SafeWriter* w);
  size_t writeFeatureSL(SafeWriter* w, std::vector<int>& list, const DivSong* song);
  size_t writeFeatureWL(SafeWriter* w, std::vector<int>& list, const DivSong* song);
  void writeFeatureMP(SafeWriter* w);
  void writeFeatureSU(SafeWriter* w);
  void writeFeatureES(SafeWriter* w);
  void writeFeatureX1(SafeWriter* w);

  void readFeatureNA(SafeReader& reader, short version);
  void readFeatureFM(SafeReader& reader, short version);
  void readFeatureMA(SafeReader& reader, short version);
  void readFeature64(SafeReader& reader, short version);
  void readFeatureGB(SafeReader& reader, short version);
  void readFeatureSM(SafeReader& reader, short version);
  void readFeatureOx(SafeReader& reader, int op, short version);
  void readFeatureLD(SafeReader& reader, short version);
  void readFeatureSN(SafeReader& reader, short version);
  void readFeatureN1(SafeReader& reader, short version);
  void readFeatureFD(SafeReader& reader, short version);
  void readFeatureWS(SafeReader& reader, short version);
  void readFeatureSL(SafeReader& reader, DivSong* song, short version);
  void readFeatureWL(SafeReader& reader, DivSong* song, short version);
  void readFeatureMP(SafeReader& reader, short version);
  void readFeatureSU(SafeReader& reader, short version);
  void readFeatureES(SafeReader& reader, short version);
  void readFeatureX1(SafeReader& reader, short version);

  DivDataErrors readInsDataOld(SafeReader& reader, short version);
  DivDataErrors readInsDataNew(SafeReader& reader, short version, bool fui, DivSong* song);
  
  /**
   * save the instrument to a SafeWriter.
   * @param w the SafeWriter in question.
   */
  void putInsData(SafeWriter* w);

  /**
   * save the instrument to a SafeWriter using new format.
   * @param w the SafeWriter in question.
   */
  void putInsData2(SafeWriter* w, bool fui=false, const DivSong* song=NULL);

  /**
   * read instrument data in .fui format.
   * @param reader the reader.
   * @param version the format version.
   * @return a DivDataErrors.
   */
  DivDataErrors readInsData(SafeReader& reader, short version, DivSong* song=NULL);

  /**
   * save this instrument to a file.
   * @param path file path.
   * @param oldFormat whether to save in legacy Furnace ins format.
   * @param song if new format, a DivSong to read wavetables and samples.
   * @return whether it was successful.
   */
  bool save(const char* path, bool oldFormat=false, DivSong* song=NULL);

  /**
   * save this instrument to a file in .dmp format.
   * @param path file path.
   * @return whether it was successful.
   */
  bool saveDMP(const char* path);
  DivInstrument():
    name(""),
    type(DIV_INS_FM) {
  }
};
#endif
