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

#include "shared.h"

class DivEngine;

void DivEngine::loadOPNI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins = new DivInstrument;

  try {
    reader.seek(0, SEEK_SET);

    String header = reader.readString(11);
    if (header == "WOPN2-INST" || header == "WOPN2-IN2T") {  // omfg >_<
      uint16_t version = reader.readS();
      if (!(version >= 2) || version > 0xF) {
        // version 1 doesn't have a version field........
        reader.seek(-2, SEEK_CUR);
        version = 1;
      }

      reader.readC(); // skip isPerc
      ins->type = DIV_INS_FM;
      ins->fm.ops = 4;

      String insName = reader.readString(32);
      ins->name = stringNotBlank(insName) ? insName : stripPath;
      // TODO adapt MIDI key offset to transpose?
      if (!reader.seek(3, SEEK_CUR)) {  // skip MIDI params
        throw EndOfFileException(&reader, reader.tell() + 3);
      }
      uint8_t feedAlgo = reader.readC();
      ins->fm.alg = (feedAlgo & 0x7);
      ins->fm.fb = ((feedAlgo>>3) & 0x7);
      reader.readC();  // Skip global bank flags - see WOPN/OPNI spec

      auto readOpniOp = [](SafeReader& reader, DivInstrumentFM::Operator& op) {
        uint8_t dtMul = reader.readC();
        uint8_t totalLevel = reader.readC();
        uint8_t arRateScale = reader.readC();
        uint8_t drAmpEnable = reader.readC();
        uint8_t d2r = reader.readC();
        uint8_t susRelease = reader.readC();
        uint8_t ssgEg = reader.readC();

        op.mult = dtMul & 0xF;
        op.dt = ((dtMul >> 4) & 0x7);
        op.tl = totalLevel & 0x7F;
        op.rs = ((arRateScale >> 6) & 0x3);
        op.ar = arRateScale & 0x1F;
        op.dr = drAmpEnable & 0x1F;
        op.am = ((drAmpEnable >> 7) & 0x1);
        op.d2r = d2r & 0x1F;
        op.rr = susRelease & 0xF;
        op.sl = ((susRelease >> 4) & 0xF);
        op.ssgEnv = ssgEg;
      };

      for (int i = 0; i < 4; ++i) {
        readOpniOp(reader, ins->fm.op[i]);
      }

      ret.push_back(ins);
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    if (ins != NULL) {
      delete ins;
    }
  }
}