/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
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

void DivEngine::loadTFI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins=new DivInstrument;
  try {
    reader.seek(0,SEEK_SET);

    ins->type=DIV_INS_FM;
    ins->name=stripPath;
    
    ins->fm.alg=reader.readC();
    ins->fm.fb=reader.readC();

    for (int i=0; i<4; i++) {
      DivInstrumentFM::Operator& op=ins->fm.op[i];

      op.mult=reader.readC();
      op.dt=reader.readC();
      op.tl=reader.readC();
      op.rs=reader.readC();
      op.ar=reader.readC();
      op.dr=reader.readC();
      op.d2r=reader.readC();
      op.rr=reader.readC();
      op.sl=reader.readC();
      op.ssgEnv=reader.readC();
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
    return;
  }

  ret.push_back(ins);
}