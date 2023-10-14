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

#ifndef _ATARI2600_EXPORT_H
#define _ATARI2600_EXPORT_H

#include "../engine.h"

struct TiaRegisters {

  unsigned char audc0;
  unsigned char audc1;
  unsigned char audf0;
  unsigned char audf1;
  unsigned char audv0;
  unsigned char audv1;

  bool write(const DivRegWrite& registerWrite);

  size_t hash() {
    return (size_t)audc0 | ((size_t)audc1 << 8) |
      ((size_t)audf0 << 16) | ((size_t)audf1 << 24) | 
      ((size_t)audv0 << 32) + ((size_t)audv1 << 40); 
  }

};

struct TiaChannelState {

  TiaChannelState() {}
  TiaChannelState(unsigned char audcx, unsigned char audfx, unsigned char audvx) :
    audcx(audcx), audfx(audfx), audvx(audvx) {}

  unsigned char audcx;
  unsigned char audfx;
  unsigned char audvx;

};

struct TiaNote {

  TiaRegisters registers;
  char duration;

  TiaNote() {}

  TiaNote(const TiaNote &n) : registers(n.registers), duration(n.duration) {}

  TiaNote(const TiaRegisters &registers) : registers(registers), duration(-1) {}

  size_t hash() {
    return registers.hash() | ((size_t)duration << 48); // note: expect registers hash to use bits 0..47
  }

};

class DivExportAtari2600 : public DivROMExport {

  void writeTrackData_CRD(DivEngine* e, SafeWriter* w);

  void writeWaveformHeader(SafeWriter* w, const char* key);
  size_t writeTextGraphics(SafeWriter* w, const char* value);
  size_t writeNote(SafeWriter* w, const TiaNote& note, TiaChannelState& state);

public:

  ~DivExportAtari2600() {}

  std::vector<DivROMExportOutput> go(DivEngine* e) override;

};

#endif // _ATARI2600_EXPORT_H