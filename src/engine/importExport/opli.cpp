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

void DivEngine::loadOPLI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList; // in case 2x2op
  DivInstrument* ins = new DivInstrument;

  auto readOpliOp = [](SafeReader& reader, DivInstrumentFM::Operator& op) {
    uint8_t characteristics = reader.readC();
    uint8_t keyScaleLevel = reader.readC();
    uint8_t attackDecay = reader.readC();
    uint8_t sustainRelease = reader.readC();
    uint8_t waveSelect = reader.readC();

    op.mult = characteristics & 0xF;
    op.ksr = ((characteristics >> 4) & 0x1);
    op.sus = ((characteristics >> 5) & 0x1);
    op.vib = ((characteristics >> 6) & 0x1);
    op.am = ((characteristics >> 7) & 0x1);
    op.tl = keyScaleLevel & 0x3F;
    op.ksl = ((keyScaleLevel >> 6) & 0x3);
    op.ar = ((attackDecay >> 4) & 0xF);
    op.dr = attackDecay & 0xF;
    op.rr = sustainRelease & 0xF;
    op.sl = ((sustainRelease >> 4) & 0xF);
    op.ws = waveSelect;
  };

  try {
    reader.seek(0, SEEK_SET);
    String header = reader.readString(11);

    if (header == "WOPL3-INST") {
      reader.readS();  // skip version (presently no difference here)
      reader.readC();  // skip isPerc field

      ins->type = DIV_INS_OPL;
      String insName = reader.readString(32);
      insName = stringNotBlank(insName) ? insName : stripPath;
      ins->name = insName;
      // TODO adapt MIDI key offset to transpose?
      reader.seek(7, SEEK_CUR);  // skip MIDI params
      uint8_t instTypeFlags = reader.readC();  // [0EEEDCBA] - see WOPL/OPLI spec

      bool is_4op = ((instTypeFlags & 0x1) == 1);
      bool is_2x2op = (((instTypeFlags>>1) & 0x1) == 1);
      bool is_rhythm = (((instTypeFlags>>4) & 0x7) > 0);

      uint8_t feedConnect = reader.readC();
      uint8_t feedConnect2nd = reader.readC();

      ins->fm.alg = (feedConnect & 0x1);
      ins->fm.fb = ((feedConnect >> 1) & 0xF);

      if (is_4op && !is_2x2op) {
        ins->fm.ops = 4;
        ins->fm.alg = (feedConnect & 0x1) | ((feedConnect2nd & 0x1) << 1);
        for (int i : {2,0,3,1}) { // omfg >_<
          readOpliOp(reader, ins->fm.op[i]);
        }
      } else {
        ins->fm.ops = 2;
        for (int i : {1,0}) {
          readOpliOp(reader, ins->fm.op[i]);
        }
        if (is_rhythm) {
          ins->fm.opllPreset = (uint8_t)(1<<4);

        } else if (is_2x2op) {
          // Note: Pair detuning offset not mappable. Use E5xx effect :P
          ins->name = fmt::sprintf("%s (1)", insName);
          insList.push_back(ins);

          ins = new DivInstrument;
          ins->type = DIV_INS_OPL;
          ins->name = fmt::sprintf("%s (2)", insName);
          ins->fm.alg = (feedConnect2nd & 0x1);
          ins->fm.fb = ((feedConnect2nd >> 1) & 0xF);
          for (int i : {1,0}) {
            readOpliOp(reader, ins->fm.op[i]);
          }
        }

        if (!is_2x2op) {
          reader.seek(10, SEEK_CUR); // skip unused operator pair
        }
      }

      insList.push_back(ins);
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    if (ins != NULL) {
      delete ins;
    }
    for (DivInstrument* p : insList) {
      delete p;
    }
    return;
  }

  for (DivInstrument* p : insList) {
    ret.push_back(p);
  }
}