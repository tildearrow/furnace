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

#ifndef _INSTRUMENT_H
#define _INSTRUMENT_H
#include "safeWriter.h"
#include "dataErrors.h"
#include "../ta-utils.h"
#include "../pch.h"
#include "../fixedQueue.h"

struct DivSong;
struct DivInstrument;

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
  DIV_INS_SM8521=48,
  DIV_INS_PV1000=49,
  DIV_INS_K053260=50,
  // DIV_INS_YMF292=51,
  DIV_INS_TED=52,
  DIV_INS_C140=53,
  DIV_INS_C219=54,
  DIV_INS_ESFM=55,
  DIV_INS_POWERNOISE=56,
  DIV_INS_POWERNOISE_SLOPE=57,
  DIV_INS_DAVE=58,
  DIV_INS_NDS=59,
  DIV_INS_GBA_DMA=60,
  DIV_INS_GBA_MINMOD=61,
  DIV_INS_BIFURCATOR=62,
  DIV_INS_SID2=63, // coincidence!
  DIV_INS_SUPERVISION=64,
  DIV_INS_UPD1771C=65,
  DIV_INS_SID3=66,
  DIV_INS_MAX,
  DIV_INS_NULL
};

enum DivMacroType: unsigned char {
  DIV_MACRO_VOL=0,
  DIV_MACRO_ARP,
  DIV_MACRO_DUTY,
  DIV_MACRO_WAVE,
  DIV_MACRO_PITCH,
  DIV_MACRO_EX1,
  DIV_MACRO_EX2,
  DIV_MACRO_EX3,
  DIV_MACRO_ALG,
  DIV_MACRO_FB,
  DIV_MACRO_FMS,
  DIV_MACRO_AMS,
  DIV_MACRO_PAN_LEFT,
  DIV_MACRO_PAN_RIGHT,
  DIV_MACRO_PHASE_RESET,
  DIV_MACRO_EX4,
  DIV_MACRO_EX5,
  DIV_MACRO_EX6,
  DIV_MACRO_EX7,
  DIV_MACRO_EX8
};

enum DivMacroTypeOp: unsigned char {
  DIV_MACRO_OP_AM=32,
  DIV_MACRO_OP_AR,
  DIV_MACRO_OP_DR,
  DIV_MACRO_OP_MULT,
  DIV_MACRO_OP_RR,
  DIV_MACRO_OP_SL,
  DIV_MACRO_OP_TL,
  DIV_MACRO_OP_DT2,
  DIV_MACRO_OP_RS,
  DIV_MACRO_OP_DT,
  DIV_MACRO_OP_D2R,
  DIV_MACRO_OP_SSG,
  DIV_MACRO_OP_DAM,
  DIV_MACRO_OP_DVB,
  DIV_MACRO_OP_EGT,
  DIV_MACRO_OP_KSL,
  DIV_MACRO_OP_SUS,
  DIV_MACRO_OP_VIB,
  DIV_MACRO_OP_WS,
  DIV_MACRO_OP_KSR,
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
  unsigned char alg, fb, fms, ams, fms2, ams2, ops, opllPreset, block;
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
    block(0),
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
  int val[256];
  unsigned int mode;
  unsigned char open;
  unsigned char len, delay, speed, loop, rel;
  // 0-31: normal
  // 32+: operator (top 3 bits select operator, starting from 1)
  unsigned char macroType;

  explicit DivInstrumentMacro(unsigned char initType, bool initOpen=false):
    mode(0),
    open(initOpen),
    len(0),
    delay(0),
    speed(1),
    loop(255),
    rel(255),
    macroType(initType) {
    memset(val,0,256*sizeof(int));
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
      amMacro(DIV_MACRO_OP_AM), arMacro(DIV_MACRO_OP_AR), drMacro(DIV_MACRO_OP_DR), multMacro(DIV_MACRO_OP_MULT),
      rrMacro(DIV_MACRO_OP_RR), slMacro(DIV_MACRO_OP_SL), tlMacro(DIV_MACRO_OP_TL,true), dt2Macro(DIV_MACRO_OP_DT2),
      rsMacro(DIV_MACRO_OP_RS), dtMacro(DIV_MACRO_OP_DT), d2rMacro(DIV_MACRO_OP_D2R), ssgMacro(DIV_MACRO_OP_SSG),
      damMacro(DIV_MACRO_OP_DAM), dvbMacro(DIV_MACRO_OP_DVB), egtMacro(DIV_MACRO_OP_EGT), kslMacro(DIV_MACRO_OP_KSL),
      susMacro(DIV_MACRO_OP_SUS), vibMacro(DIV_MACRO_OP_VIB), wsMacro(DIV_MACRO_OP_WS), ksrMacro(DIV_MACRO_OP_KSR) {}
  } opMacros[4];

  DivInstrumentMacro* macroByType(DivMacroType type);

  DivInstrumentSTD():
    volMacro(DIV_MACRO_VOL,true),
    arpMacro(DIV_MACRO_ARP),
    dutyMacro(DIV_MACRO_DUTY),
    waveMacro(DIV_MACRO_WAVE),
    pitchMacro(DIV_MACRO_PITCH),
    ex1Macro(DIV_MACRO_EX1),
    ex2Macro(DIV_MACRO_EX2),
    ex3Macro(DIV_MACRO_EX3),
    algMacro(DIV_MACRO_ALG),
    fbMacro(DIV_MACRO_FB),
    fmsMacro(DIV_MACRO_FMS),
    amsMacro(DIV_MACRO_AMS),
    panLMacro(DIV_MACRO_PAN_LEFT),
    panRMacro(DIV_MACRO_PAN_RIGHT),
    phaseResetMacro(DIV_MACRO_PHASE_RESET),
    ex4Macro(DIV_MACRO_EX4),
    ex5Macro(DIV_MACRO_EX5),
    ex6Macro(DIV_MACRO_EX6),
    ex7Macro(DIV_MACRO_EX7),
    ex8Macro(DIV_MACRO_EX8) {
    for (int i=0; i<4; i++) {
      opMacros[i].amMacro.macroType=DIV_MACRO_OP_AM+(i<<5);
      opMacros[i].arMacro.macroType=DIV_MACRO_OP_AR+(i<<5);
      opMacros[i].drMacro.macroType=DIV_MACRO_OP_DR+(i<<5);
      opMacros[i].multMacro.macroType=DIV_MACRO_OP_MULT+(i<<5);
      opMacros[i].rrMacro.macroType=DIV_MACRO_OP_RR+(i<<5);
      opMacros[i].slMacro.macroType=DIV_MACRO_OP_SL+(i<<5);
      opMacros[i].tlMacro.macroType=DIV_MACRO_OP_TL+(i<<5);
      opMacros[i].dt2Macro.macroType=DIV_MACRO_OP_DT2+(i<<5);
      opMacros[i].rsMacro.macroType=DIV_MACRO_OP_RS+(i<<5);
      opMacros[i].dtMacro.macroType=DIV_MACRO_OP_DT+(i<<5);
      opMacros[i].d2rMacro.macroType=DIV_MACRO_OP_D2R+(i<<5);
      opMacros[i].ssgMacro.macroType=DIV_MACRO_OP_SSG+(i<<5);

      opMacros[i].damMacro.macroType=DIV_MACRO_OP_DAM+(i<<5);
      opMacros[i].dvbMacro.macroType=DIV_MACRO_OP_DVB+(i<<5);
      opMacros[i].egtMacro.macroType=DIV_MACRO_OP_EGT+(i<<5);
      opMacros[i].kslMacro.macroType=DIV_MACRO_OP_KSL+(i<<5);
      opMacros[i].susMacro.macroType=DIV_MACRO_OP_SUS+(i<<5);
      opMacros[i].vibMacro.macroType=DIV_MACRO_OP_VIB+(i<<5);
      opMacros[i].wsMacro.macroType=DIV_MACRO_OP_WS+(i<<5);
      opMacros[i].ksrMacro.macroType=DIV_MACRO_OP_KSR+(i<<5);
    }
  }
};

struct DivInstrumentGB {
  unsigned char envVol, envDir, envLen, soundLen, hwSeqLen;
  bool softEnv, alwaysInit, doubleWave;
  enum HWSeqCommands: unsigned char {
    DIV_GB_HWCMD_ENVELOPE=0,
    DIV_GB_HWCMD_SWEEP,
    DIV_GB_HWCMD_WAIT,
    DIV_GB_HWCMD_WAIT_REL,
    DIV_GB_HWCMD_LOOP,
    DIV_GB_HWCMD_LOOP_REL,

    DIV_GB_HWCMD_MAX
  };
  struct HWSeqCommandGB {
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
    alwaysInit(false),
    doubleWave(false) {
    memset(hwSeq,0,256*sizeof(HWSeqCommandGB));
  }
};

struct DivInstrumentC64 {
  bool triOn, sawOn, pulseOn, noiseOn;
  unsigned char a, d, s, r;
  unsigned short duty;
  unsigned char ringMod, oscSync;
  bool toFilter, initFilter, dutyIsAbs, filterIsAbs, noTest, resetDuty;
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
    initFilter(false),
    dutyIsAbs(false),
    filterIsAbs(false),
    noTest(false),
    resetDuty(true),
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
    signed char dpcmFreq;
    signed char dpcmDelta;
    SampleMap(int f=0, short m=-1, signed char df=15, signed char dd=-1):
      freq(f),
      map(m),
      dpcmFreq(df),
      dpcmDelta(dd) {}
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
   * get the sample playback note at specified note.
   * @return the note, or -1 if not using note map.
   */
  inline int getFreq(int note) {
    if (useNoteMap) {
      if (note<0) note=0;
      if (note>119) note=119;
      return noteMap[note].freq;
    }
    return note;
  }

  /**
   * get the DPCM pitch at specified note.
   * @return the pitch, or -1 if not using note map.
   */
  inline signed char getDPCMFreq(int note) {
    if (useNoteMap) {
      if (note<0) note=0;
      if (note>119) note=119;
      return noteMap[note].dpcmFreq;
    }
    return -1;
  }

  /**
   * get the DPCM delta counter value at specified note.
   * @return the delta counter value, or -1 if not using note map.
   */
  inline signed char getDPCMDelta(int note) {
    if (useNoteMap) {
      if (note<0) note=0;
      if (note>119) note=119;
      return noteMap[note].dpcmDelta;
    }
    return -1;
  }

  DivInstrumentAmiga():
    initSample(0),
    useNoteMap(false),
    useSample(false),
    useWave(false),
    waveLen(31) {
    for (int i=0; i<120; i++) {
      noteMap[i].map=-1;
      noteMap[i].freq=i;
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
  bool perChanPos;
  int wavePosCh[8];
  int waveLenCh[8];

  bool operator==(const DivInstrumentN163& other);
  bool operator!=(const DivInstrumentN163& other) {
    return !(*this==other);
  }

  DivInstrumentN163():
    wave(-1),
    wavePos(0),
    waveLen(32),
    waveMode(3),
    perChanPos(false) {
    for (int i=0; i<8; i++) {
      wavePosCh[i]=(i&3)<<5;
      waveLenCh[i]=32;
    }
  }
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
  bool damp, pseudoReverb, lfoReset, levelDirect;

  bool operator==(const DivInstrumentMultiPCM& other);
  bool operator!=(const DivInstrumentMultiPCM& other) {
    return !(*this==other);
  }

  DivInstrumentMultiPCM():
    ar(15), d1r(15), dl(0), d2r(0), rr(15), rc(15),
    lfo(0), vib(0), am(0),
    damp(false),
    pseudoReverb(false),
    lfoReset(false),
    levelDirect(true) {
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
  unsigned char hwSeqLen;
  enum HWSeqCommands: unsigned char {
    DIV_SU_HWCMD_VOL=0,
    DIV_SU_HWCMD_PITCH,
    DIV_SU_HWCMD_CUT,
    DIV_SU_HWCMD_WAIT,
    DIV_SU_HWCMD_WAIT_REL,
    DIV_SU_HWCMD_LOOP,
    DIV_SU_HWCMD_LOOP_REL,

    DIV_SU_HWCMD_MAX
  };
  struct HWSeqCommandSU {
    unsigned char cmd;
    unsigned char bound;
    unsigned char val;
    unsigned short speed;
    unsigned short padding;
  } hwSeq[256];

  bool operator==(const DivInstrumentSoundUnit& other);
  bool operator!=(const DivInstrumentSoundUnit& other) {
    return !(*this==other);
  }

  DivInstrumentSoundUnit():
    switchRoles(false),
    hwSeqLen(0) {
    memset(hwSeq,0,256*sizeof(HWSeqCommandSU));
  }
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

// ESFM operator structure:
// - DELAY, OUT, MOD, L, R, NOISE
//   - Virtual: CT, DT, FIXED
//   - In FM struct: AM, DAM, AR, DR, MULT, RR, SL, TL
//   - In FM struct: KSL, VIB, DVB, WS, SUS, KSR
//   - Not in struct: FNUML, FNUMH, BLOCK

struct DivInstrumentESFM {
  bool operator==(const DivInstrumentESFM& other);
  bool operator!=(const DivInstrumentESFM& other) {
    return !(*this==other);
  }

  // Only works on OP4, so putting it outside the Operator struct instead
  unsigned char noise;
  struct Operator {
    unsigned char delay, outLvl, modIn, left, right, fixed;
    signed char ct, dt;

    bool operator==(const Operator& other);
    bool operator!=(const Operator& other) {
      return !(*this==other);
    }
    Operator():
      delay(0),
      outLvl(0),
      modIn(0),
      left(1),
      right(1),
      fixed(0),
      ct(0),
      dt(0) {}
  } op[4];
  DivInstrumentESFM():
    noise(0)
    {
      op[0].modIn=4;
      op[0].outLvl=0;

      op[1].modIn=7;
      op[1].outLvl=0;

      op[2].modIn=7;
      op[2].outLvl=0;

      op[3].modIn=7;
      op[3].outLvl=7;
    }
};

struct DivInstrumentPowerNoise {
  unsigned char octave;

  bool operator==(const DivInstrumentPowerNoise& other);
  bool operator!=(const DivInstrumentPowerNoise& other) {
    return !(*this==other);
  }
  DivInstrumentPowerNoise():
    octave(0) {}
};

struct DivInstrumentSID2 {
  unsigned char volume;
  unsigned char mixMode;
  unsigned char noiseMode;
  
  bool operator==(const DivInstrumentSID2& other);
  bool operator!=(const DivInstrumentSID2& other) {
    return !(*this==other);
  }
  DivInstrumentSID2():
    volume(15),
    mixMode(0),
    noiseMode(0) {}
};

struct DivInstrumentSID3 {
  bool triOn, sawOn, pulseOn, noiseOn;
  unsigned char a, d, s, r;
  unsigned char sr;
  unsigned short duty;
  unsigned char ringMod, oscSync;
  bool phase_mod;
  unsigned char phase_mod_source, ring_mod_source, sync_source;
  bool specialWaveOn;
  bool oneBitNoise;
  bool separateNoisePitch;
  unsigned char special_wave;
  bool doWavetable;
  bool dutyIsAbs;
  bool resetDuty;
  unsigned char phaseInv;
  unsigned char feedback;
  unsigned char mixMode;

  struct Filter {
    unsigned short cutoff;
    unsigned char resonance;
    unsigned char output_volume;
    unsigned char distortion_level;
    unsigned char mode;
    bool enabled;
    bool init;
    unsigned char filter_matrix;

    // this is done purely in software
    bool absoluteCutoff;
    bool bindCutoffToNote;
    unsigned char bindCutoffToNoteStrength; // how much cutoff changes over e.g. 1 semitone
    unsigned char bindCutoffToNoteCenter; // central note of the cutoff change
    bool bindCutoffToNoteDir; // if we decrease or increase cutoff if e.g. we go upper in note space
    bool bindCutoffOnNote; // only do cutoff scaling once, on new note

    bool bindResonanceToNote;
    unsigned char bindResonanceToNoteStrength; // how much resonance changes over e.g. 1 semitone
    unsigned char bindResonanceToNoteCenter; // central note of the resonance change
    bool bindResonanceToNoteDir; // if we decrease or increase resonance if e.g. we go upper in note space
    bool bindResonanceOnNote; // only do resonance scaling once, on new note

    bool operator==(const Filter& other);
    bool operator!=(const Filter& other) {
      return !(*this==other);
    }
    Filter():
      cutoff(0),
      resonance(0),
      output_volume(0),
      distortion_level(0),
      mode(0),
      enabled(false),
      init(false),
      filter_matrix(0),
      absoluteCutoff(false),
      bindCutoffToNote(false),
      bindCutoffToNoteStrength(0),
      bindCutoffToNoteCenter(0),
      bindCutoffToNoteDir(false),
      bindCutoffOnNote(false),
      bindResonanceToNote(false),
      bindResonanceToNoteStrength(0),
      bindResonanceToNoteCenter(0),
      bindResonanceToNoteDir(false),
      bindResonanceOnNote(false) {}
  } filt[4];
  
  bool operator==(const DivInstrumentSID3& other);
  bool operator!=(const DivInstrumentSID3& other) {
    return !(*this==other);
  }
  DivInstrumentSID3():
    triOn(false),
    sawOn(true),
    pulseOn(false),
    noiseOn(false),
    a(0),
    d(64),
    s(0),
    r(0),
    sr(0),
    duty(32768),
    ringMod(0),
    oscSync(0),
    phase_mod(false),
    phase_mod_source(0),
    ring_mod_source(0),
    sync_source(0),
    specialWaveOn(false),
    oneBitNoise(false),
    separateNoisePitch(false),
    special_wave(0),
    doWavetable(false),
    dutyIsAbs(true),
    resetDuty(false),
    phaseInv(0),
    feedback(0),
    mixMode(0) {
      filt[0].mode=16|32; // default settings so filter just works, connect to input and channel output
      filt[0].output_volume=0xff;
    }
};

struct DivInstrumentPOD {
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
  DivInstrumentESFM esfm;
  DivInstrumentPowerNoise powernoise;
  DivInstrumentSID2 sid2;
  DivInstrumentSID3 sid3;

  DivInstrumentPOD() :
    type(DIV_INS_FM) {
  }
};

struct DivInstrumentTemp {
  // the following variables are used by the GUI and not saved in the file
  int vScroll[160];
  int vZoom[160];
  int typeMemory[160][16];
  unsigned char lenMemory[160];
  DivInstrumentTemp() {
    memset(vScroll,0,160*sizeof(int));
    memset(vZoom,-1,160*sizeof(int));
    memset(typeMemory,0,160*16*sizeof(int));
    memset(lenMemory,0,160*sizeof(int));
  }
};

struct MemPatch {
  MemPatch() :
    data(NULL)
    , offset(0)
    , size(0) {
  }

  ~MemPatch() {
    if (data) {
      delete[] data;
      data=NULL;
    }
  }

  bool calcDiff(const void* pre, const void* post, size_t size);
  void applyAndReverse(void* target, size_t inputSize);
  bool isValid() const { return size>0; }

  unsigned char* data;
  size_t offset;
  size_t size;
};

struct DivInstrumentUndoStep {
  DivInstrumentUndoStep() :
    name(""),
    nameValid(false),
    processTime(0) {
  }

  MemPatch podPatch;
  String name;
  bool nameValid;
  size_t processTime;

  void applyAndReverse(DivInstrument* target);
  bool makeUndoPatch(size_t processTime_, const DivInstrument* pre, const DivInstrument* post);
};

struct DivInstrument : DivInstrumentPOD {
  String name;

  DivInstrumentTemp temp;

  DivInstrument() :
    name("") {
      // clear and construct DivInstrumentPOD so it doesn't have any garbage in the padding
      memset((unsigned char*)(DivInstrumentPOD*)this, 0, sizeof(DivInstrumentPOD));
      new ((DivInstrumentPOD*)this) DivInstrumentPOD;
  }

  ~DivInstrument();

  /**
   * copy/assignment to specifically avoid leaking or dangling pointers to undo step
   */
  DivInstrument( const DivInstrument& ins );
  DivInstrument& operator=( const DivInstrument& ins );

  /**
   * undo stuff
   */
  FixedQueue<DivInstrumentUndoStep*, 128> undoHist;
  FixedQueue<DivInstrumentUndoStep*, 128> redoHist;
  bool recordUndoStepIfChanged(size_t processTime, const DivInstrument* old);
  int undo();
  int redo();

  /**
   * these are internal functions.
   */
  void writeMacro(SafeWriter* w, const DivInstrumentMacro& m);
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
  void writeFeatureNE(SafeWriter* w);
  void writeFeatureEF(SafeWriter* w);
  void writeFeaturePN(SafeWriter* w);
  void writeFeatureS2(SafeWriter* w);
  void writeFeatureS3(SafeWriter* w);

  void readFeatureNA(SafeReader& reader, short version);
  void readFeatureFM(SafeReader& reader, short version);
  void readFeatureMA(SafeReader& reader, short version);
  void readFeature64(SafeReader& reader, bool& volIsCutoff, short version);
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
  void readFeatureNE(SafeReader& reader, short version);
  void readFeatureEF(SafeReader& reader, short version);
  void readFeaturePN(SafeReader& reader, short version);
  void readFeatureS2(SafeReader& reader, short version);
  void readFeatureS3(SafeReader& reader, short version);

  DivDataErrors readInsDataOld(SafeReader& reader, short version);
  DivDataErrors readInsDataNew(SafeReader& reader, short version, bool fui, DivSong* song);

  void convertC64SpecialMacro();

  /**
   * save the instrument to a SafeWriter.
   * @param w the SafeWriter in question.
   */
  void putInsData2(SafeWriter* w, bool fui=false, const DivSong* song=NULL, bool insName=true);

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
   * @param song if new format, a DivSong to read wavetables and samples.
   * @param writeInsName whether to write the instrument name or not. ignored if old format.
   * @return whether it was successful.
   */
  bool save(const char* path, DivSong* song=NULL, bool writeInsName=true);

  /**
   * save this instrument to a file in .dmp format.
   * @param path file path.
   * @return whether it was successful.
   */
  bool saveDMP(const char* path);
};
#endif
