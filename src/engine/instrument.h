#ifndef _INSTRUMENT_H
#define _INSTRUMENT_H
#include "../ta-utils.h"

enum DivInstrumentType {
  DIV_INS_STD=0,
  DIV_INS_FM=1,
  DIV_INS_GB=2,
  DIV_INS_C64=3,
  DIV_INS_AMIGA=4
};

struct DivInstrumentFM {
  unsigned char alg, fb, fms, ams, ops;
  struct Operator {
    unsigned char am, ar, dr, mult, rr, sl, tl, dt2, rs, dt, d2r, ssgEnv;
    unsigned char dam, dvb, egt, ksl, sus, vib, ws, ksr; // YMU759
    Operator():
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
      ksr(0) {}
  } op[4];
  DivInstrumentFM():
    alg(0),
    fb(0),
    fms(0),
    ams(0),
    ops(4) {
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

struct DivInstrumentSTD {
  int volMacro[256];
  int arpMacro[256];
  int dutyMacro[256];
  int waveMacro[256];
  bool arpMacroMode;
  unsigned char volMacroLen, arpMacroLen, dutyMacroLen, waveMacroLen;
  signed char volMacroLoop, arpMacroLoop, dutyMacroLoop, waveMacroLoop;
  DivInstrumentSTD():
    arpMacroMode(false),
    volMacroLen(0),
    arpMacroLen(0),
    dutyMacroLen(0),
    waveMacroLen(0),
    volMacroLoop(-1),
    arpMacroLoop(-1),
    dutyMacroLoop(-1),
    waveMacroLoop(-1) {
      memset(volMacro,0,256*sizeof(int));
      memset(arpMacro,0,256*sizeof(int));
      memset(dutyMacro,0,256*sizeof(int));
      memset(waveMacro,0,256*sizeof(int));
    }
};

struct DivInstrumentGB {
  unsigned char envVol, envDir, envLen, soundLen;
  DivInstrumentGB():
    envVol(15),
    envDir(0),
    envLen(2),
    soundLen(64) {}
};

struct DivInstrumentC64 {
  bool triOn, sawOn, pulseOn, noiseOn;
  unsigned char a, d, s, r;
  unsigned char duty;
  unsigned char ringMod, oscSync;
  bool toFilter, volIsCutoff, initFilter, dutyIsAbs, filterIsAbs;
  unsigned char res, cut;
  bool hp, lp, bp, ch3off;

  DivInstrumentC64():
    triOn(false),
    sawOn(true),
    pulseOn(false),
    noiseOn(false),
    a(0),
    d(8),
    s(0),
    r(0),
    duty(50),
    ringMod(0),
    oscSync(0),
    toFilter(false),
    volIsCutoff(false),
    initFilter(false),
    dutyIsAbs(false),
    filterIsAbs(false),
    res(0),
    cut(0),
    hp(false),
    lp(false),
    bp(false),
    ch3off(false) {}
};

struct DivInstrumentAmiga {
  short initSample;

  DivInstrumentAmiga():
    initSample(0) {}
};

struct DivInstrument {
  String name;
  bool mode;
  DivInstrumentType type;
  DivInstrumentFM fm;
  DivInstrumentSTD std;
  DivInstrumentGB gb;
  DivInstrumentC64 c64;
  DivInstrumentAmiga amiga;
  DivInstrument():
    name(""),
    mode(false),
    type(DIV_INS_STD) {
  }
};
#endif
