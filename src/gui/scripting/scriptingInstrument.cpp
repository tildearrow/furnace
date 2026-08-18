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

#include "scripting.h"

// instruments are so complicated the API helper functions are kept separate

void writeMacro(DivInstrumentMacro* macro, const char* key, const char* subkey, int value) {
  #define CHECK_PARAM(_k,_p,_mn,_mx) if (strcmp(key,_k)==0) {macro->_p=CLAMP(value,_mn,_mx);}
  CHECK_PARAM("delay",delay,0,255)
  else CHECK_PARAM("speed",speed,0,255)
  else CHECK_PARAM("loop",loop,0,255)
  else CHECK_PARAM("release",rel,0,255)
  else CHECK_PARAM("mode",mode,0,255)
  else if (strcmp(key,"open")==0) {
    macro->open=(macro->open&(~1))|CLAMP(value,0,1);
  } else if (strcmp(key,"instantRelease")==0) {
    macro->open=(macro->open&(~(1<<3)))|(CLAMP(value,0,1)<<3);
  }
  #undef CHECK_PARAM
  #define CHECK_PARAM(_k,_v) if (strcmp(subkey,_k)==0) {macro->val[_v]=value;}
  if (subkey) {
    if (strcmp(key,"envelope")==0) {
      CHECK_PARAM("bottom",0)
      else CHECK_PARAM("top",1)
      else CHECK_PARAM("attack",2)
      else CHECK_PARAM("hold",3)
      else CHECK_PARAM("decay",4)
      else CHECK_PARAM("sustain",5)
      else CHECK_PARAM("susTime",6)
      else CHECK_PARAM("susDecay",7)
      else CHECK_PARAM("release",8)
      macro->open=(macro->open&(~6))|2;
    } else if (strcmp(key,"lfo")==0) {
      CHECK_PARAM("bottom",0)
      else CHECK_PARAM("top",1)
      else CHECK_PARAM("speed",11)
      else CHECK_PARAM("waveform",12)
      else CHECK_PARAM("phase",13)
      macro->open=(macro->open&(~6))|4;
    }
  } else {
    macro->open=(macro->open&(~6));
    macro->len=0;
  }
}
