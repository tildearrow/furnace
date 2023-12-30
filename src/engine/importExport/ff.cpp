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

void DivEngine::loadFF(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* insList[256];
  memset(insList,0,256*sizeof(void*));
  int readCount = 0;
  size_t insCount = reader.size();
  insCount = (insCount >> 5) + (((insCount % 0x20) > 0) ? 1 : 0);
  if (insCount > 256) insCount = 256;
  uint8_t buf;
  try {
    reader.seek(0, SEEK_SET);
    for (unsigned int i = 0; i < insCount; ++i) {
      insList[i] = new DivInstrument;
      DivInstrument* ins = insList[i];

      ins->type = DIV_INS_FM;
      DivInstrumentFM::Operator op;

      for (unsigned int j = 0; j < 4; j++) {
        buf = reader.readC();
        ins->fm.op[j].mult = buf & 0xf;
        ins->fm.op[j].dt = fmDtRegisterToFurnace((buf >> 4) & 0x7);
        ins->fm.op[j].ssgEnv = (buf >> 4) & 0x8;
      }
      for (unsigned int j = 0; j < 4; j++) {
        buf = reader.readC();
        ins->fm.op[j].tl = buf & 0x7f;
        ins->fm.op[j].ssgEnv |= (buf >> 5) & 0x4;
      }
      for (unsigned int j = 0; j < 4; j++) {
        buf = reader.readC();
        ins->fm.op[j].ar = buf & 0x1f;
        ins->fm.op[j].rs = buf >> 6;
      }
      for (unsigned int j = 0; j < 4; j++) {
        buf = reader.readC();
        ins->fm.op[j].dr = buf & 0x1f;
        ins->fm.op[j].ssgEnv |= (buf >> 5) & 0x3;
        ins->fm.op[j].am = buf >> 7;
      }
      for (unsigned int j = 0; j < 4; j++) {
        buf = reader.readC();
        ins->fm.op[j].d2r = buf & 0x1f;
      }
      for (unsigned int j = 0; j < 4; j++) {
        buf = reader.readC();
        ins->fm.op[j].rr = buf & 0xf;
        ins->fm.op[j].sl = buf >> 4;
      }

      buf = reader.readC();
      ins->fm.alg = buf & 0x7;
      ins->fm.fb = (buf >> 3) & 0x7;

      // FIXME This is encoded in Shift-JIS
      ins->name = reader.readString(7);
      ++readCount;
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    // Include incomplete entry in deletion.
    for (int i = readCount; i >= 0; --i) {
      delete insList[i];
    }
    return;
  }

  for (unsigned int i = 0; i < insCount; ++i) {
    ret.push_back(insList[i]);
  }
}