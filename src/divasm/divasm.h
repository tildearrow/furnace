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

// this is a mini-assembler written for Furnace.
// it will be used in future ROM export (yes, that's right, ROM is baked at export time).

#include "../engine/safeWriter.h"

struct DivASMResult {
  int line, err;
  DivASMResult():
    line(-1),
    err(0) {}
};

enum DivASMTarget {
  DIV_ASM_TARGET_DUMMY=0,
  DIV_ASM_TARGET_6502,
  DIV_ASM_TARGET_SPC700,
  DIV_ASM_TARGET_Z80
};

struct DivLabel {
  String name;
  unsigned int location;
  bool direct;
};

struct DivOper {
  String operation;
  int line;
};

class DivASM {
  SafeWriter* result;

  std::vector<DivLabel> labels;
  std::vector<DivOper> ops;

  public:
    DivASMResult getError();
    SafeWriter* assemble(SafeReader* data);

    DivASM();
    ~DivASM();
};
