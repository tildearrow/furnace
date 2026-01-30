/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

class DivEngine;

struct DivMacroStruct {
  int pos, lastPos, lfoPos, delay;
  int val;
  bool has, had, actualHad, finished, will, linger, began, masked, activeRelease;
  unsigned int mode, type;
  unsigned char macroType;
  void doMacro(DivInstrumentMacro& source, bool released, bool tick);
  void init() {
    pos=lastPos=lfoPos=mode=type=delay=0;
    has=had=actualHad=will=false;
    linger=false;
    began=true;
    // TODO: test whether this breaks anything?
    val=0;
  }
  void prepare(DivInstrumentMacro& source, DivEngine* e);
  DivMacroStruct(unsigned char mType):
    pos(0),
    lastPos(0),
    lfoPos(0),
    delay(0),
    val(0),
    has(false),
    had(false),
    actualHad(false),
    finished(false),
    will(false),
    linger(false),
    began(true),
    masked(false),
    activeRelease(false),
    mode(0),
    type(0),
    macroType(mType) {}
};

class DivMacroInt {
  DivEngine* e;
  DivInstrument* ins;
  DivMacroStruct* macroList[128];
  DivInstrumentMacro* macroSource[128];
  size_t macroListLen;
  int subTick;
  bool released;
  public:
    // common macro
    DivMacroStruct vol;
    DivMacroStruct arp;
    DivMacroStruct duty, wave, pitch, ex1, ex2, ex3;
    DivMacroStruct alg, fb, fms, ams;
    DivMacroStruct panL, panR, phaseReset, ex4, ex5, ex6, ex7, ex8;
    DivMacroStruct ex9, ex10;
  
    // FM operator macro
    struct IntOp {
      DivMacroStruct am, ar, dr, mult;
      DivMacroStruct rr, sl, tl, dt2;
      DivMacroStruct rs, dt, d2r, ssg;
      DivMacroStruct dam, dvb, egt, ksl;
      DivMacroStruct sus, vib, ws, ksr;
      IntOp():
        am(DIV_MACRO_OP_AM),
        ar(DIV_MACRO_OP_AR),
        dr(DIV_MACRO_OP_DR),
        mult(DIV_MACRO_OP_MULT),
        rr(DIV_MACRO_OP_RR),
        sl(DIV_MACRO_OP_SL),
        tl(DIV_MACRO_OP_TL),
        dt2(DIV_MACRO_OP_DT2),
        rs(DIV_MACRO_OP_RS),
        dt(DIV_MACRO_OP_DT),
        d2r(DIV_MACRO_OP_D2R),
        ssg(DIV_MACRO_OP_SSG),
        dam(DIV_MACRO_OP_DAM),
        dvb(DIV_MACRO_OP_DVB),
        egt(DIV_MACRO_OP_EGT),
        ksl(DIV_MACRO_OP_KSL),
        sus(DIV_MACRO_OP_SUS),
        vib(DIV_MACRO_OP_VIB),
        ws(DIV_MACRO_OP_WS),
        ksr(DIV_MACRO_OP_KSR) {}
    } op[4];

    // state
    bool hasRelease;

    /**
     * set mask on macro.
     */
    void mask(unsigned char id, bool enabled);

    /**
     * trigger macro release.
     */
    void release();

    /**
     * restart macro.
     */
    void restart(unsigned char id);

    /**
     * trigger next macro tick.
     */
    void next();

    /**
     * set the engine.
     * @param the engine
     */
    void setEngine(DivEngine* eng);

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

    /**
     * get DivMacroStruct by macro type.
     * @param which the macro type.
     * @return a DivMacroStruct, or NULL if none found.
     */
    DivMacroStruct* structByType(unsigned char which);

    DivMacroInt():
      e(NULL),
      ins(NULL),
      macroListLen(0),
      subTick(1),
      released(false),
      vol(DIV_MACRO_VOL),
      arp(DIV_MACRO_ARP),
      duty(DIV_MACRO_DUTY),
      wave(DIV_MACRO_WAVE),
      pitch(DIV_MACRO_PITCH),
      ex1(DIV_MACRO_EX1),
      ex2(DIV_MACRO_EX2),
      ex3(DIV_MACRO_EX3),
      alg(DIV_MACRO_ALG),
      fb(DIV_MACRO_FB),
      fms(DIV_MACRO_FMS),
      ams(DIV_MACRO_AMS),
      panL(DIV_MACRO_PAN_LEFT),
      panR(DIV_MACRO_PAN_RIGHT),
      phaseReset(DIV_MACRO_PHASE_RESET),
      ex4(DIV_MACRO_EX4),
      ex5(DIV_MACRO_EX5),
      ex6(DIV_MACRO_EX6),
      ex7(DIV_MACRO_EX7),
      ex8(DIV_MACRO_EX8),
      ex9(DIV_MACRO_EX9),
      ex10(DIV_MACRO_EX10),
      hasRelease(false) {
      memset(macroList,0,128*sizeof(void*));
      memset(macroSource,0,128*sizeof(void*));
    }
};

#endif
