#ifndef _INSTRUMENT_H
#define _INSTRUMENT_H
#include "../ta-utils.h"

enum DivInstrumentType {
  DIV_INS_FM,
  DIV_INS_STD,
  DIV_INS_GB,
  DIV_INS_C64
};

struct DivInstrumentFM {
  unsigned char alg, fb, fms, ams, ops;
  struct Operator {
    unsigned char am, ar, dr, mult, rr, sl, tl, dt2, rs, dt, d2r, ssgEnv;
    unsigned char dam, dvb, egt, ksl, sus, vib, ws, ksr; // YMU759
  } op[4];
};

struct DivInstrumentSTD {
  int volMacro[256];
  int arpMacro[256];
  int dutyMacro[256];
  int waveMacro[256];
  bool arpMacroMode;
  unsigned char volMacroLen, arpMacroLen, dutyMacroLen, waveMacroLen;
  signed char volMacroLoop, arpMacroLoop, dutyMacroLoop, waveMacroLoop;
};

struct DivInstrumentGB {
  unsigned char envVol, envDir, envLen, soundLen;
};

struct DivInstrumentC64 {
  bool triOn, sawOn, pulseOn, noiseOn;
  unsigned char a, d, s, r;
  unsigned char duty;
  unsigned char ringMod, oscSync;
  bool toFilter, volIsCutoff, initFilter;
  unsigned char res, cut;
  bool hp, lp, bp, ch3off;
};

struct DivInstrument {
  String name;
  bool mode;
  DivInstrumentType type;
  DivInstrumentFM fm;
  DivInstrumentSTD std;
  DivInstrumentGB gb;
  DivInstrumentC64 c64;
};
#endif
