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

#ifndef _MACROINT_H
#define _MACROINT_H

#include "instrument.h"

class DivMacroInt {
  DivInstrument* ins;
  int volPos, arpPos, dutyPos, wavePos, pitchPos, ex1Pos, ex2Pos, ex3Pos;
  int algPos, fbPos, fmsPos, amsPos;
  bool released;
  public:
    int vol;
    int arp;
    int duty, wave, pitch, ex1, ex2, ex3;
    int alg, fb, fms, ams;
    bool hasVol, hasArp, hasDuty, hasWave, hasPitch, hasEx1, hasEx2, hasEx3, hasAlg, hasFb, hasFms, hasAms;
    bool hadVol, hadArp, hadDuty, hadWave, hadPitch, hadEx1, hadEx2, hadEx3, hadAlg, hadFb, hadFms, hadAms;
    bool finishedVol, finishedArp, finishedDuty, finishedWave, finishedPitch, finishedEx1, finishedEx2, finishedEx3;
    bool finishedAlg, finishedFb, finishedFms, finishedAms;
    bool willVol, willArp, willDuty, willWave, willPitch, willEx1, willEx2, willEx3, willAlg, willFb, willFms, willAms;
    bool arpMode;
    struct IntOp {
      int amPos, arPos, drPos, multPos;
      int rrPos, slPos, tlPos, dt2Pos;
      int rsPos, dtPos, d2rPos, ssgPos;
      int damPos, dvbPos, egtPos, kslPos;
      int susPos, vibPos, wsPos, ksrPos;

      int am, ar, dr, mult;
      int rr, sl, tl, dt2;
      int rs, dt, d2r, ssg;
      int dam, dvb, egt, ksl;
      int sus, vib, ws, ksr;

      bool hasAm, hasAr, hasDr, hasMult;
      bool hasRr, hasSl, hasTl, hasDt2;
      bool hasRs, hasDt, hasD2r, hasSsg;
      bool hasDam, hasDvb, hasEgt, hasKsl;
      bool hasSus, hasVib, hasWs, hasKsr;

      bool hadAm, hadAr, hadDr, hadMult;
      bool hadRr, hadSl, hadTl, hadDt2;
      bool hadRs, hadDt, hadD2r, hadSsg;
      bool hadDam, hadDvb, hadEgt, hadKsl;
      bool hadSus, hadVib, hadWs, hadKsr;

      bool finishedAm, finishedAr, finishedDr, finishedMult;
      bool finishedRr, finishedSl, finishedTl, finishedDt2;
      bool finishedRs, finishedDt, finishedD2r, finishedSsg;
      bool finishedDam, finishedDvb, finishedEgt, finishedKsl;
      bool finishedSus, finishedVib, finishedWs, finishedKsr;

      bool willAm, willAr, willDr, willMult;
      bool willRr, willSl, willTl, willDt2;
      bool willRs, willDt, willD2r, willSsg;
      bool willDam, willDvb, willEgt, willKsl;
      bool willSus, willVib, willWs, willKsr;
      IntOp():
        amPos(0),
        arPos(0),
        drPos(0),
        multPos(0),
        rrPos(0),
        slPos(0),
        tlPos(0),
        dt2Pos(0),
        rsPos(0),
        dtPos(0),
        d2rPos(0),
        ssgPos(0),
        damPos(0),
        dvbPos(0),
        egtPos(0),
        kslPos(0),
        susPos(0),
        vibPos(0),
        wsPos(0),
        ksrPos(0),
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
        ssg(0),
        dam(0),
        dvb(0),
        egt(0),
        ksl(0),
        sus(0),
        vib(0),
        ws(0),
        ksr(0),
        hasAm(false), hasAr(false), hasDr(false), hasMult(false),
        hasRr(false), hasSl(false), hasTl(false), hasDt2(false),
        hasRs(false), hasDt(false), hasD2r(false), hasSsg(false),
        hasDam(false), hasDvb(false), hasEgt(false), hasKsl(false),
        hasSus(false), hasVib(false), hasWs(false), hasKsr(false),
        hadAm(false), hadAr(false), hadDr(false), hadMult(false),
        hadRr(false), hadSl(false), hadTl(false), hadDt2(false),
        hadRs(false), hadDt(false), hadD2r(false), hadSsg(false),
        hadDam(false), hadDvb(false), hadEgt(false), hadKsl(false),
        hadSus(false), hadVib(false), hadWs(false), hadKsr(false),
        finishedAm(false), finishedAr(false), finishedDr(false), finishedMult(false),
        finishedRr(false), finishedSl(false), finishedTl(false), finishedDt2(false),
        finishedRs(false), finishedDt(false), finishedD2r(false), finishedSsg(false),
        finishedDam(false), finishedDvb(false), finishedEgt(false), finishedKsl(false),
        finishedSus(false), finishedVib(false), finishedWs(false), finishedKsr(false),
        willAm(false), willAr(false), willDr(false), willMult(false),
        willRr(false), willSl(false), willTl(false), willDt2(false),
        willRs(false), willDt(false), willD2r(false), willSsg(false),
        willDam(false), willDvb(false), willEgt(false), willKsl(false),
        willSus(false), willVib(false), willWs(false), willKsr(false) {}
    } op[4];

    /**
     * trigger macro release.
     */
    void release();

    /**
     * trigger next macro tick.
     */
    void next();

    /**
     * initialize the macro interpreter.
     * @param which an instrument, or NULL.
     */
    void init(DivInstrument* which);

    /**
     * notify this macro interpreter that an instrument has been deleted.
     * @param which the instrument in question.
     */
    void notifyInsDeletion(DivInstrument* which);

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
      algPos(0),
      fbPos(0),
      fmsPos(0),
      amsPos(0),
      released(false),
      vol(0),
      arp(0),
      duty(0),
      wave(0),
      pitch(0),
      ex1(0),
      ex2(0),
      ex3(0),
      alg(0),
      fb(0),
      fms(0),
      ams(0),
      hasVol(false),
      hasArp(false),
      hasDuty(false),
      hasWave(false),
      hasPitch(false),
      hasEx1(false),
      hasEx2(false),
      hasEx3(false),
      hasAlg(false),
      hasFb(false),
      hasFms(false),
      hasAms(false),
      hadVol(false),
      hadArp(false),
      hadDuty(false),
      hadWave(false),
      hadPitch(false),
      hadEx1(false),
      hadEx2(false),
      hadEx3(false),
      hadAlg(false),
      hadFb(false),
      hadFms(false),
      hadAms(false),
      finishedVol(false),
      finishedArp(false),
      finishedDuty(false),
      finishedWave(false),
      finishedPitch(false),
      finishedEx1(false),
      finishedEx2(false),
      finishedEx3(false),
      finishedAlg(false),
      finishedFb(false),
      finishedFms(false),
      finishedAms(false),
      willVol(false),
      willArp(false),
      willDuty(false),
      willWave(false),
      willPitch(false),
      willEx1(false),
      willEx2(false),
      willEx3(false),
      willAlg(false),
      willFb(false),
      willFms(false),
      willAms(false),
      arpMode(false) {}
};

#endif
