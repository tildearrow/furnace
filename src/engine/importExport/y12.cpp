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

#include "importExport.h"

class DivEngine;

void DivEngine::loadY12(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {  
  DivInstrument *ins = new DivInstrument;

  try {
    reader.seek(0, SEEK_SET);
    ins->type = DIV_INS_FM;
    ins->fm.ops = 4;
    ins->name = stripPath;

    for (int i = 0; i < 4; ++i) {
      DivInstrumentFM::Operator& insOp = ins->fm.op[i];
      uint8_t tmp = reader.readC();
      insOp.mult = tmp & 0xF;
      // ???
      insOp.dt = ((3 + (tmp >> 4)) & 0x7);
      insOp.tl = (reader.readC() & 0x7F);
      tmp = reader.readC();
      insOp.rs = ((tmp >> 6) & 0x3);
      insOp.ar = tmp & 0x1F;
      tmp = reader.readC();
      insOp.dr = tmp & 0x1F;
      insOp.am = ((tmp >> 7) & 0x1);
      insOp.d2r = (reader.readC() & 0x1F);
      tmp = reader.readC();
      insOp.rr = tmp & 0xF;
      insOp.sl = ((tmp >> 4) & 0xF);
      insOp.ssgEnv = reader.readC();
      if (!reader.seek(9, SEEK_CUR)) {
        throw EndOfFileException(&reader, reader.tell() + 9);
      }
    }
    ins->fm.alg = reader.readC();
    ins->fm.fb = reader.readC();
    if (!reader.seek(62, SEEK_CUR)) {
      throw EndOfFileException(&reader, reader.tell() + 62);
    }
    ret.push_back(ins);
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    if (ins != NULL) {
      delete ins;
    }
  }
}