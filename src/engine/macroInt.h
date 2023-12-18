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
  std::vector<DivMacroStruct*> macroList;
  std::vector<DivInstrumentMacro*> macroSource;
  int subTick;
  bool released;
  public:
    // common macro

    std::vector<DivMacroStruct> macros;
    uint8_t macro_mask[4];
  
    // FM operator macro
    struct IntOp
    {
      std::vector<DivMacroStruct> macros;
      uint8_t macro_mask[4];

      IntOp()
      {
        macros.clear();

        for(int i = 0; i < 4; i++)
        {
          macro_mask[i] = 0;
        }
      }

      DivMacroStruct* op_get_div_macro_struct(uint8_t macro_id)
      {
        static DivMacroStruct dummy = DivMacroStruct(0xff);

        for(int i = 0; i < (int)macros.size(); i++)
        {
          if(macros[i].macroType == macro_id) return &macros[i];
        }

        return &dummy;
      }
    };

    IntOp* get_int_op(int index)
    {
      if(index >= (int)op.size())
      {
        int limit = index + 1 - op.size();

        for(int j = 0; j < limit; j++)
        {
          op.push_back(IntOp());
        }
      }

      return &op[index];
    }

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
     * @return a DivMacroStruct pointer, or NULL if none found.
     */
    DivMacroStruct* structByType(unsigned char which);

    /**
     * Add macro of specified type to corresponding macro lists.
     * @param macro_type Macro type.
     * @param m DivInstrumentMacro pointer.
     */
    void add_macro(uint8_t macro_type, DivInstrumentMacro* m);

    /**
     * Add macro of specified type to macro lists of OpInt struct corresponding to OPER operator.
     * @param oper Operator index.
     * @param macro_type Macro type.
     * @param m DivInstrumentMacro pointer.
     */
    void add_op_macro(uint8_t oper, uint8_t macro_type, DivInstrumentMacro* m);

    /**
     * Get DivMacroStruct pointer for a given macro_id.
     * @param macro_id Macro ID.
     * @return a DivMacroStruct pointer, or NULL if none found.
     */
    DivMacroStruct* get_div_macro_struct(uint8_t macro_id);

    /**
     * Mask (enable or disable) macro.
     * @param id Macro ID.
     * @param enabled Flag for enabling/disabling macro.
     */
    void consider_macro(unsigned char id, bool enabled);

    /**
     * Mask (enable or disable) FM operator macro.
     * @param oper FM operator index.
     * @param id Macro ID.
     * @param enabled Flag for enabling/disabling macro.
     */
    void consider_op_macro(unsigned char oper, unsigned char id, bool enabled);

    /**
     * Get DivMacroStruct pointer for macro with given type.
     * @param type Macro type.
     * @return a DivMacroStruct pointer, or NULL if none found.
     */
    DivMacroStruct* get_macro_by_type(unsigned char type);

    /**
     * Get DivMacroStruct pointer for FM operator macro with given type.
     * @param oper FM operator index.
     * @param type Macro type.
     * @return a DivMacroStruct pointer, or NULL if none found.
     */
    DivMacroStruct* get_op_macro_by_type(unsigned char oper, unsigned char type);

    /**
     * Get number of instrument macros that actually need to be executed.
     * @param ins DivInstrument pointer.
     * @return a number of macros that actually need to be processed.
     */
    int get_macro_count(DivInstrument* ins);

    /**
     * Get number of FM operator macros that actually need to be executed.
     * @param ins DivInstrument pointer.
     * @param oper FM operator index.
     * @return a number of specified FM operator macros that actually need to be processed.
     */
    int get_op_macro_count(DivInstrument* ins, uint8_t oper);

    DivMacroInt()
    {
      macros.clear();
      macroList.clear();
      macroSource.clear();

      e = NULL;
      ins = NULL;
      subTick = 0;
      released = false;
      hasRelease = false;

      for(int i = 0; i < 4; i++)
      {
        macro_mask[i] = 0;
      }

      op.clear();
    }
};

#endif
