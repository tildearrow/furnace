#ifndef _MACROINT_H
#define _MACROINT_H

#include "instrument.h"

class DivMacroInt {
  DivInstrument* ins;
  int volPos, arpPos, dutyPos, wavePos;
  public:
    unsigned char vol, arp, duty, wave;
    bool hasVol, hasArp, hasDuty, hasWave;
    bool hadVol, hadArp, hadDuty, hadWave;
    void next();
    void init(DivInstrument* which);
    DivMacroInt():
      ins(NULL),
      vol(0),
      arp(0),
      duty(0),
      wave(0),
      hasVol(false),
      hasArp(false),
      hasDuty(false),
      hasWave(false),
      hadVol(false),
      hadArp(false),
      hadDuty(false),
      hadWave(false) {}
};

#endif
