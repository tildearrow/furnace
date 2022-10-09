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

#ifndef _R9_H
#define _R9_H

#include "../engine.h"

struct TiaRegisters {

  unsigned char audc0;
  unsigned char audc1;
  unsigned char audf0;
  unsigned char audf1;
  unsigned char audv0;
  unsigned char audv1;

  bool write(const DivRegWrite& registerWrite);

};

class R9 {

  DivEngine* e;

  void dumpRegisters(SafeWriter* w);
  void writeTrackData(SafeWriter* w);
  void writeWaveformHeader(SafeWriter* w, const char * key);
  void writeRegisters(SafeWriter* w, const TiaRegisters& reg, int channel);

public:    

  R9(DivEngine* _e) : e(_e) {}
  ~R9() {}
  SafeWriter* buildROM(int sysIndex);

};

#endif // _R9_H