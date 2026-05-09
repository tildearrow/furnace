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

#ifndef _SCSPDSPASM_H
#define _SCSPDSPASM_H

// USC (micro-source) assembler for the SCSP/Saturn on-chip DSP. Output is
// the same shape the SCSP emulator's scsp_dsp_load_arrays() expects — 128-step
// MPRO, 64 COEF, 32 MADRS, plus an RBL ring-buffer length selector.

#include <string>
#include <vector>

struct SCSPDSPAssembly {
  unsigned short mpro[512];   // 128 steps × 4 16-bit words (zero-padded after `steps`)
  short          coef[64];    // SCSP reads bits [15:3] as signed-13; values are pre-shifted
  unsigned short madrs[32];
  int rbl;              // 0..3 ring-buffer length selector
  int steps;            // active MPRO steps after alignment
  std::vector<std::string> errors;    // empty on success
  std::vector<std::string> warnings;  // alignment NOPs, truncation, etc.
};

// Assemble `src` into `out`. Returns true if `out.errors` is empty after
// the call. `rbl` is clamped to 0..3.
bool scspdspAssemble(const std::string& src, int rbl, SCSPDSPAssembly& out);

#endif
