#ifndef _MACROINT_H
#define _MACROINT_H

#include "instrument.h"

class DivMacroInt {
  DivInstrument* ins;
  int volPos, arpPos, dutyPos, wavePos, pitchPos, ex1Pos, ex2Pos, ex3Pos;
  public:
    int vol;
    int arp;
    int duty, wave, pitch, ex1, ex2, ex3;
    bool hasVol, hasArp, hasDuty, hasWave, hasPitch, hasEx1, hasEx2, hasEx3;
    bool hadVol, hadArp, hadDuty, hadWave, hadPitch, hadEx1, hadEx2, hadEx3;
    bool finishedVol, finishedArp, finishedDuty, finishedWave, finishedPitch, finishedEx1, finishedEx2, finishedEx3;
    bool willVol, willArp, willDuty, willWave, willPitch, willEx1, willEx2, willEx3;
    bool arpMode;
    void next();
    void init(DivInstrument* which);
    DivMacroInt():
      ins(NULL),
      volPos(0),
      arpPos(0),
      dutyPos(0),
      wavePos(0),
      pitchPos(0),
      ex1Pos(0),
      ex2Pos(0),
      ex3Pos(0),
      vol(0),
      arp(0),
      duty(0),
      wave(0),
      pitch(0),
      ex1(0),
      ex2(0),
      ex3(0),
      hasVol(false),
      hasArp(false),
      hasDuty(false),
      hasWave(false),
      hasPitch(false),
      hasEx1(false),
      hasEx2(false),
      hasEx3(false),
      hadVol(false),
      hadArp(false),
      hadDuty(false),
      hadWave(false),
      hadPitch(false),
      hadEx1(false),
      hadEx2(false),
      hadEx3(false),
      finishedVol(false),
      finishedArp(false),
      finishedDuty(false),
      finishedWave(false),
      finishedPitch(false),
      finishedEx1(false),
      finishedEx2(false),
      finishedEx3(false),
      willVol(false),
      willArp(false),
      willDuty(false),
      willWave(false),
      willPitch(false),
      willEx1(false),
      willEx2(false),
      willEx3(false),
      arpMode(false) {}
};

#endif
