#ifndef _MACROINT_H
#define _MACROINT_H

#include "instrument.h"

class DivMacroInt {
  DivInstrument* ins;
  int volPos, arpPos, dutyPos, wavePos;
  public:
    unsigned char vol;
    signed char arp;
    unsigned char duty, wave;
    bool hasVol, hasArp, hasDuty, hasWave;
    bool hadVol, hadArp, hadDuty, hadWave;
    bool finishedVol, finishedArp, finishedDuty, finishedWave;
    bool willVol, willArp, willDuty, willWave;
    bool arpMode;
    void next();
    void init(DivInstrument* which);
    DivMacroInt():
      ins(NULL),
      volPos(0),
      arpPos(0),
      dutyPos(0),
      wavePos(0),
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
      hadWave(false),
      finishedVol(false),
      finishedArp(false),
      finishedDuty(false),
      finishedWave(false),
      willVol(false),
      willArp(false),
      willDuty(false),
      willWave(false),
      arpMode(false) {}
};

#endif
