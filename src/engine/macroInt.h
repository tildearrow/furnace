/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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
#include <vector>

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
  //DivMacroStruct* macroList[128];
  //DivInstrumentMacro* macroSource[128];
  std::vector<DivMacroStruct*> macroList;
  std::vector<DivInstrumentMacro*> macroSource;
  //size_t macroListLen;
  int subTick;
  bool released;
  public:
    // common macro

    std::vector<DivMacroStruct> macros;
  
    // FM operator macro
    struct IntOp {
      std::vector<DivMacroStruct> macros;

      IntOp()
      {
        macros.clear();
      }
    };

    std::vector<IntOp> op;

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

    DivMacroInt()
    {
      //memset(macroList,0,128*sizeof(void*));
      //memset(macroSource,0,128*sizeof(void*));
      macroList.clear();
      macroSource.clear();

      op.clear();
    }
};

#endif
