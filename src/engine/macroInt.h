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

// macroInt.h: the macro interpreter.

#ifndef _MACROINT_H
#define _MACROINT_H

#include "instrument.h"

class DivEngine;

/**
 * DivMacroStruct holds the state for a macro in a DivMacroInt.
 */ 
struct DivMacroStruct {
  // pos:
  // - current macro position (for sequences)
  // - current envelope/LFO state (for ADSR/LFO macros)
  // lastPos: the previous value of pos.
  // delay:
  // - the number of ticks to wait before the next step.
  // - set to Delay when initialized and then set to Step Length on each step.
  int pos, lastPos, delay;
  // current output value.
  int val;
  // has: whether there is a value on this tick.
  // had: whether there was a value on the previous tick, I think
  // actualHad: internally used to determine whether the macro is over.
  // finished: set to true when the macro has ended.
  // will: whether this macro is going to run. set on init.
  // linger: whether the macro will finish or linger on the last value. set to true for the volume macro.
  // began: whether this is the macro's first tick.
  // masked: disables this macro entirely. set by the disable/enable macro effects.
  // activeRelease: whether release should skip to the release point.
  // lfoDir: sets the direction of the LFO. used in LFO with triangle wave.
  bool has, had, actualHad, finished, will, linger, began, masked, activeRelease, lfoDir;
  // mode: a copy of the macro's mode. mostly unused.
  // type: the macro type. valid values are:
  // - 0: sequence
  // - 1: ADSR
  // - 2: LFO
  unsigned int mode, type;
  // the parameter that this macro will control.
  unsigned char macroType;

  /**
   * update this macro. executed on each tick.
   * @param source the source macro.
   * @param released whether the note has been released.
   * @param tick this is a bit complicated but I'll explain.
   * this tells the macro whether a song tick has occurred.
   * in low-latency mode, the engine ticks at the closest multiple of the tick rate to match 1000Hz.
   * several "sub-ticks" are created, allowing you to play notes in the middle of a song tick.
   * this determines whether enough ticks have passed to run macros at the correct tick rate.
   */
  void doMacro(DivInstrumentMacro& source, bool released, bool tick);
  /**
   * reset state.
   * called once we don't need this state anymore.
   */
  void init() {
    pos=lastPos=mode=type=delay=0;
    has=had=actualHad=will=false;
    linger=false;
    began=true;
    lfoDir=false;
    // TODO: test whether this breaks anything?
    val=0;
  }
  /**
   * initialize state.
   * called on macro restart.
   */
  void prepare(DivInstrumentMacro& source, DivEngine* e);
  DivMacroStruct(unsigned char mType):
    pos(0),
    lastPos(0),
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
    lfoDir(false),
    mode(0),
    type(0),
    macroType(mType) {}
};

/**
 * this is the macro interpreter. it runs macros.
 * normally there's one per dispatch channel.
 */
class DivMacroInt {
  // the DivEngine associated with this macro interpreter.
  DivEngine* e;
  // the related instrument.
  DivInstrument* ins;
  // list of macros to run. populated during note on.
  DivMacroStruct* macroList[128];
  // sources of macros to run.
  DivInstrumentMacro* macroSource[128];
  // number of macros to process.
  size_t macroListLen;
  // the current "sub-tick". in low-latency mode, this counts how many engine ticks remain until the next song tick.
  int subTick;
  // whether note/macro release occurred.
  bool released;
  public:
    // each DivMacroInt defines macro states for all macros.
    // this is done for convenience. not all macros may be running.
    // common macros
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
     * set mask on macro. used by the macro enable/disable effect.
     * @param id the macro to alter.
     * @param enable whether the mask is enabled (macro is disabled).
     */
    void mask(unsigned char id, bool enabled);

    /**
     * trigger macro release.
     */
    void release();

    /**
     * restart macro.
     * @param id the macro to restart.
     */
    void restart(unsigned char id);

    /**
     * trigger next macro tick. called on each engine tick.
     */
    void next();

    /**
     * set the engine.
     * @param eng the engine.
     */
    void setEngine(DivEngine* eng);

    /**
     * initialize the macro interpreter.
     * @param which an instrument, or NULL (no instrument).
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
