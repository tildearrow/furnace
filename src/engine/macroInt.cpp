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

#include "macroInt.h"
#include "instrument.h"
#include "engine.h"
#include "../ta-log.h"

#define ADSR_LOW source.val[0]
#define ADSR_HIGH source.val[1]
#define ADSR_AR source.val[2]
#define ADSR_HT source.val[3]
#define ADSR_DR source.val[4]
#define ADSR_SL source.val[5]
#define ADSR_ST source.val[6]
#define ADSR_SR source.val[7]
#define ADSR_RR source.val[8]

#define LFO_SPEED source.val[11]
#define LFO_WAVE source.val[12]
#define LFO_PHASE source.val[13]
#define LFO_LOOP source.val[14]
#define LFO_GLOBAL source.val[15]

void DivMacroStruct::prepare(DivInstrumentMacro& source, DivEngine* e) {
  has=had=actualHad=will=true;
  mode=source.mode;
  type=(source.open>>1)&3;
  activeRelease=source.open&8;
  linger=(source.macroType==DIV_MACRO_VOL && e->song.volMacroLinger);
  lfoPos=LFO_PHASE;
}

void DivMacroStruct::doMacro(DivInstrumentMacro& source, bool released, bool tick) {
  if (!tick) {
    had=false;
    return;
  }
  if (masked) {
    had=false;
    has=false;
    return;
  }
  if (released && type==1 && lastPos<3) delay=0;
  if (released && type==0 && pos<source.rel && source.rel<source.len && activeRelease) {
    delay=0;
    pos=source.rel;
  }
  if (delay>0) {
    delay--;
    if (!linger) had=false;
    return;
  }
  if (began && source.delay>0) {
    delay=source.delay;
  } else {
    delay=source.speed-1;
  }
  if (began) {
    began=false;
  }
  if (finished) {
    finished=false;
  }
  if (actualHad!=has) {
    finished=true;
  }
  actualHad=has;
  had=actualHad;

  if (has) {
    if (type==0) { // sequence
      lastPos=pos;
      val=source.val[pos++];
      if (pos>source.rel && !released) {
        if (source.loop<source.len && source.loop<source.rel) {
          pos=source.loop;
        } else {
          pos--;
        }
      }
      if (pos>=source.len) {
        if (source.loop<source.len && (source.loop>=source.rel || source.rel>=source.len)) {
          pos=source.loop;
        } else if (linger) {
          pos--;
        } else {
          has=false;
        }
      }
    }
    if (type==1) { // ADSR
      if (released && lastPos<3) lastPos=3;
      switch (lastPos) {
        case 0: // attack
          pos+=ADSR_AR;
          if (pos>255) {
            pos=255;
            lastPos=1;
            delay=ADSR_HT;
          }
          break;
        case 1: // decay
          pos-=ADSR_DR;
          if (pos<=ADSR_SL) {
            pos=ADSR_SL;
            lastPos=2;
            delay=ADSR_ST;
          }
          break;
        case 2: // sustain
          pos-=ADSR_SR;
          if (pos<0) {
            pos=0;
            lastPos=4;
          }
          break;
        case 3: // release
          pos-=ADSR_RR;
          if (pos<0) {
            pos=0;
            lastPos=4;
          }
          break;
        case 4: // end
          pos=0;
          if (!linger) has=false;
          break;
      }
      val=ADSR_LOW+((pos+(ADSR_HIGH-ADSR_LOW)*pos)>>8);
    }
    if (type==2) { // LFO
      lfoPos+=LFO_SPEED;
      lfoPos&=1023;

      int lfoOut=0;
      switch (LFO_WAVE&3) {
        case 0: // triangle
          lfoOut=((lfoPos&512)?(1023-lfoPos):(lfoPos))>>1;
          break;
        case 1: // saw
          lfoOut=lfoPos>>2;
          break;
        case 2: // pulse
          lfoOut=(lfoPos&512)?255:0;
          break;
      }
      val=ADSR_LOW+((lfoOut+(ADSR_HIGH-ADSR_LOW)*lfoOut)>>8);
    }
  }
}

void DivMacroInt::next() {
  if (ins==NULL) return;
  // run macros
  // TODO: potentially get rid of list to avoid allocations
  subTick--;
  for (size_t i=0; i<macroList.size(); i++) {
    if (macroList[i]!=NULL && macroSource[i]!=NULL) {
      macroList[i]->doMacro(*macroSource[i],released,subTick==0);
    }
  }
  if (subTick<=0) {
    if (e==NULL) {
      subTick=1;
    } else {
      subTick=e->tickMult;
    }
  }
}

void DivMacroInt::consider_macro(unsigned char id, bool enabled)
{
  for(int i = 0; i < 0x20; i++)
  {
    if(macros[i].macroType == id)
    {
      macros[i].masked = enabled;
      return;
    }
  }

  return;
}

void DivMacroInt::consider_op_macro(unsigned char oper, unsigned char id, bool enabled)
{
  for(int i = 0; i < 0x20; i++)
  {
    if(op[oper].macros[i].macroType == id)
    {
      op[oper].macros[i].masked = enabled;
      return;
    }
  }

  return;
}

void DivMacroInt::mask(unsigned char id, bool enabled)
{
  if(id < 0x20)
  {
    consider_macro(id, enabled);
  }

  else
  {
    consider_op_macro((id >> 5) - 1, id, enabled);
  }
}

#undef CONSIDER_OP
#undef CONSIDER

void DivMacroInt::release() {
  released=true;
}

void DivMacroInt::setEngine(DivEngine* eng) {
  e=eng;
}

void DivMacroInt::add_macro(DivMacroStruct* ms, DivInstrumentMacro* m)
{
    /*for (int i = 0; i < macros.size(); i++)
    {
      if (macros[i].macroType == m->macroType)
      {
        macroList[i] = ms;
        macroSource[i] = m;
        return;
      }
    }*/

  if(!(ms->masked))
  {
    macros.push_back(*ms);
    macroList.push_back(ms);
    macroSource.push_back(m);
  }
}

void DivMacroInt::add_op_macro(uint8_t oper, DivMacroStruct* ms, DivInstrumentMacro* m)
{
  /*for (int i = 0; i < op[oper].macros.size(); i++)
  {
    if (op[oper].macros[i].macroType == m->macroType)
    {
      macroList[i] = ms;
      macroSource[i] = m;
      return;
    }
  }*/

  if(!(ms->masked))
  {
    op[oper].macros.push_back(*ms);
    macroList.push_back(ms);
    macroSource.push_back(m);

    macroList[macroList.size() - 1] = &op[oper].macros[op[oper].macros.size() - 1];
  }
}

DivMacroStruct* DivMacroInt::get_div_macro_struct(uint8_t macro_id)
{
  static DivMacroStruct dummy = DivMacroStruct(0xff);
  for(int i = 0; i < macroList.size(); i++)
  {
    if(macroList[i]->macroType == macro_id)
    {
      return macroList[i];
    }
  }

  return &dummy;
}

void DivMacroInt::init(DivInstrument* which)
{
  ins=which;
  // initialize
  macroList.clear();
  macroSource.clear();
  subTick=1;

  hasRelease=false;
  released=false;

  if (ins==NULL) return;

  // prepare common macro
  macros.clear();

  for(int i = 0; i < ins->std.macros.size(); i++)
  {
    if(ins->std.macros[i].len > 0)
    {
      add_macro(new DivMacroStruct(ins->std.macros[i].macroType), &ins->std.macros[i]);
    }
  }

  if(op.size() < ins->std.ops.size()) //if operators vector isn't initialized
  {
    int init = op.size();

    for(int i = 0; i < ins->std.ops.size() - init; i++)
    {
      op.push_back(IntOp());
    }
  }

  // prepare FM operator macros
  for (int oper=0; oper<ins->std.ops.size(); oper++)
  {
    op[oper].macros.clear();

    for (int i=0; i<ins->std.ops[oper].macros.size(); i++)
    {
      if(ins->std.ops[oper].macros[i].len > 0)
      {
        add_op_macro(oper, new DivMacroStruct(ins->std.ops[oper].macros[i].macroType), &ins->std.ops[oper].macros[i]);
      }
    }
  }

  for (size_t i=0; i<macroList.size(); i++) {
    if (macroSource[i]!=NULL) {
      macroList[i]->prepare(*macroSource[i],e);
      // check ADSR mode
      if ((macroSource[i]->open&6)==2) {
        if (macroSource[i]->val[8]>0) {
          hasRelease=true;
        }
      } else if (macroSource[i]->rel<macroSource[i]->len) {
        hasRelease=true;
      }
    }
  }
}

void DivMacroInt::notifyInsDeletion(DivInstrument* which) {
  if (ins==which) {
    init(NULL);
  }
}

//#define CONSIDER(x,y) case (y&0x1f): return &x; break;

DivMacroStruct* DivMacroInt::get_macro_by_type(unsigned char type)
{
  for(int i = 0; i < macroList.size(); i++)
  {
    if(macroList[i]->macroType == type)
    {
      return macroList[i];
    }
  }

  return NULL;
}

DivMacroStruct* DivMacroInt::get_op_macro_by_type(unsigned char oper, unsigned char type)
{
  /*if(oper >= op.size())
  {
    int limit = 1 + oper - op.size();

    for(int i = 0; i < limit; i++)
    {
      op.push_back(IntOp());
    }
  }

  for(int i = 0; i < op[oper].macros.size(); i++)
  {
    if(op[oper].macros[i].macroType == type)
    {
      return &op[oper].macros[i];
    }
  }*/

  for(int i = 0; i < macroList.size(); i++)
  {
    if(macroList[i]->macroType == type)
    {
      return macroList[i];
    }
  }

  return NULL;
}

DivMacroStruct* DivMacroInt::structByType(unsigned char type)
{
  if (type>=0x20)
  {
    unsigned char o = ((type >> 5) - 1) & 3;
    
    return get_op_macro_by_type(o, type & 31);
  }

  return get_macro_by_type(type);
}

#undef CONSIDER
