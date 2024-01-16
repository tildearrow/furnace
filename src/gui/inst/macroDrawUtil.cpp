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

#include "macroDraw.h"

String genericGuide(float value) {
  return fmt::sprintf("%d",(int)value);
}

int deBit30(const int val) {
  if ((val&0xc0000000)==0x40000000 || (val&0xc0000000)==0x80000000) return val^0x40000000;
  return val;
}

bool enBit30(const int val) {
  if ((val&0xc0000000)==0x40000000 || (val&0xc0000000)==0x80000000) return true;
  return false;
}

String macroHoverNote(int id, float val, void* u) {
  int* macroVal=(int*)u;
  if ((macroVal[id]&0xc0000000)==0x40000000 || (macroVal[id]&0xc0000000)==0x80000000) {
    if (val<-60 || val>=120) return "???";
    return fmt::sprintf("%d: %s",id,noteNames[(int)val+60]);
  }
  return fmt::sprintf("%d: %d",id,(int)val);
}

String macroHover(int id, float val, void* u) {
  return fmt::sprintf("%d: %d",id,(int)val);
}

String macroHoverLoop(int id, float val, void* u) {
  if (val>1) return "Release";
  if (val>0) return "Loop";
  return "";
}

String macroHoverBit30(int id, float val, void* u) {
  if (val>0) return "Fixed";
  return "Relative";
}

char* int_to_char_array(int num)
{
  static char numberstring[50];
  memset(numberstring, 0, 50);
  snprintf(numberstring, 50, "%d", num);
  return numberstring;
}

String macroHoverGain(int id, float val, void* u) {
  if (val>=224.0f) {
    return fmt::sprintf("%d: +%d (exponential)",id,(int)(val-224));
  }
  if (val>=192.0f) {
    return fmt::sprintf("%d: +%d (linear)",id,(int)(val-192));
  }
  if (val>=160.0f) {
    return fmt::sprintf("%d: -%d (exponential)",id,(int)(val-160));
  }
  if (val>=128.0f) {
    return fmt::sprintf("%d: -%d (linear)",id,(int)(val-128));
  }
  return fmt::sprintf("%d: %d (direct)",id,(int)val);
}

String macroHoverES5506FilterMode(int id, float val, void* u) {
  String mode="???";
  switch (((int)val)&3) {
    case 0:
      mode="HP/K2, HP/K2";
      break;
    case 1:
      mode="HP/K2, LP/K1";
      break;
    case 2:
      mode="LP/K2, LP/K2";
      break;
    case 3:
      mode="LP/K2, LP/K1";
      break;
    default:
      break;
  }
  return fmt::sprintf("%d: %s",id,mode);
}

String macroLFOWaves(int id, float val, void* u) {
  switch (((int)val)&3) {
    case 0:
      return "Saw";
    case 1:
      return "Square";
    case 2:
      return "Triangle";
    case 3:
      return "Random";
    default:
      return "???";
  }
  return "???";
}