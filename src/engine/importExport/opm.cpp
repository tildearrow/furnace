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

#include "shared.h"

class DivEngine;

void DivEngine::loadOPM(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList;

  int readCount = 0;
  bool is_failed = false;

  bool patchNameRead = false,
       lfoRead = false,
       characteristicRead = false,
       m1Read = false,
       c1Read = false,
       m2Read = false,
       c2Read = false;

  DivInstrument* newPatch = NULL;
  
  auto completePatchRead = [&]() -> bool {
    return patchNameRead && lfoRead && characteristicRead && m1Read && c1Read && m2Read && c2Read;
  };
  auto resetPatchRead = [&]() {
    patchNameRead = lfoRead = characteristicRead = m1Read = c1Read = m2Read = c2Read = false;
    newPatch = NULL;
  };
  auto readIntStrWithinRange = [](String&& input, int limitLow = INT_MIN, int limitHigh = INT_MAX) -> int {
    int x = std::stoi(input.c_str());
    if (x > limitHigh || x < limitLow) {
      throw std::invalid_argument(fmt::sprintf("%s is out of bounds of range [%d..%d]", input, limitLow, limitHigh));
    }
    return x;
  };
  auto readOpmOperator = [&](SafeReader& reader, DivInstrumentFM::Operator& op) {
    op.ar = readIntStrWithinRange(reader.readStringToken(), 0, 31);
    op.dr = readIntStrWithinRange(reader.readStringToken(), 0, 31);
    op.d2r = readIntStrWithinRange(reader.readStringToken(), 0, 31);
    op.rr = readIntStrWithinRange(reader.readStringToken(), 0, 31);
    op.sl = readIntStrWithinRange(reader.readStringToken(), 0, 15);
    op.tl = readIntStrWithinRange(reader.readStringToken(), 0, 127);
    op.rs = readIntStrWithinRange(reader.readStringToken(), 0, 3);;
    op.mult = readIntStrWithinRange(reader.readStringToken(), 0, 15);
    op.dt = fmDtRegisterToFurnace(readIntStrWithinRange(reader.readStringToken(), 0, 7));
    op.dt2 = readIntStrWithinRange(reader.readStringToken(), 0, 3);
    op.am = readIntStrWithinRange(reader.readStringToken(), 0) > 0 ? 1 : 0;
  };
  auto seekGroupValStart = [](SafeReader& reader, int pos) {
    // Seek to position then move to next ':' character
    if (!reader.seek(pos, SEEK_SET)) {
      throw EndOfFileException(&reader, pos);
    }
    reader.readStringToken(':', false);
  };

  try {
    reader.seek(0, SEEK_SET);
    while (!reader.isEOF()) {
      // Checking line prefixes since they sometimes may not have a space after the ':'
      size_t linePos = reader.tell();
      String token = reader.readStringToken();
      if (token.size() == 0) {
        continue;
      }

      if (token.compare(0,2,"//") == 0) {
        if (!reader.isEOF()) {
          reader.readStringLine();
        }
        continue;
      }

      // At this point we know any other line would be associated with patch params
      if (newPatch == NULL) {
        newPatch = new DivInstrument;
        newPatch->type = DIV_INS_OPM;
        newPatch->fm.ops = 4;
      }

      // Read each line for their respective params. They may not be written in the same LINE order but they 
      // must absolutely be properly grouped per patch! See inline comments indicating line structure examples.
      
      if (token.size() >= 2) {
        if (token[0] == '@') {
          // @:123 Name of patch
          seekGroupValStart(reader, linePos);
          // Note: Fallback to bank filename and current patch number specified by @n
          String opmPatchNum = reader.readStringToken();
          String insName = reader.readStringLine();
          newPatch->name = stringNotBlank(insName) 
            ? insName 
            : fmt::sprintf("%s @%s", stripPath, opmPatchNum);
          patchNameRead = true;

        } else if (token.compare(0,3,"CH:") == 0) {
          // CH: PAN FL CON AMS PMS SLOT NE
          seekGroupValStart(reader, linePos);
          reader.readStringToken(); // skip PAN
          newPatch->fm.fb = readIntStrWithinRange(reader.readStringToken(), 0, 7);
          newPatch->fm.alg = readIntStrWithinRange(reader.readStringToken(), 0, 7);
          newPatch->fm.ams = readIntStrWithinRange(reader.readStringToken(), 0, 4);
          newPatch->fm.fms = readIntStrWithinRange(reader.readStringToken(), 0, 7);
          reader.readStringToken(); // skip SLOT (no furnace equivalent...yet?)
          reader.readStringToken(); // skip NE   (^^^)
          characteristicRead = true;

        } else if (token.compare(0,3,"C1:") == 0) {
          // C1: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          seekGroupValStart(reader, linePos);
          readOpmOperator(reader, newPatch->fm.op[2]);
          c1Read = true;

        } else if (token.compare(0,3,"C2:") == 0) {
          // C2: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          seekGroupValStart(reader, linePos);
          readOpmOperator(reader, newPatch->fm.op[3]);
          c2Read = true;

        } else if (token.compare(0,3,"M1:") == 0) {
          // M1: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          seekGroupValStart(reader, linePos);
          readOpmOperator(reader, newPatch->fm.op[0]);
          m1Read = true;

        } else if (token.compare(0,3,"M2:") == 0) {
          // M2: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          seekGroupValStart(reader, linePos);
          readOpmOperator(reader, newPatch->fm.op[1]);
          m2Read = true;

        } else if (token.compare(0,4,"LFO:") == 0) {
          // LFO:LFRQ AMD PMD WF NFRQ
          seekGroupValStart(reader, linePos);
          // Furnace patches do not store these as they are chip-global.
          reader.readStringLine();
          lfoRead = true;
        } else {
          // other unsupported lines ignored.
          reader.readStringLine();
        }
      }

      if (completePatchRead()) {
        insList.push_back(newPatch);
        resetPatchRead();
        ++readCount;
      }
    }

    if (newPatch != NULL) {
      addWarning("Last OPM patch read was incomplete and therefore not imported.");
      logW("Last OPM patch read was incomplete and therefore not imported.");
      delete newPatch;
      newPatch = NULL;
    }

    for (int i = 0; i < readCount; ++i) {
      ret.push_back(insList[i]);
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    is_failed = true;
  } catch (std::invalid_argument& e) {
    lastError=fmt::sprintf("Invalid value found in patch file. %s", e.what());
    logE("Invalid value found in patch file.");
    logE(e.what());
    is_failed = true;
  }

  if (is_failed) {
    for (int i = readCount - 1; i >= 0; --i) {
      delete insList[i];
    }
    if (newPatch != NULL) {
      delete newPatch;
    }
  }
}