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

void DivEngine::loadBNK(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList;
  std::vector<String*> instNames;
  reader.seek(0, SEEK_SET);

  // First distinguish between GEMS BNK and Adlib BNK
  uint64_t header = reader.readL();
  bool is_adlib = ((header>>8) == 0x2d42494c444100L);
  bool is_failed = false;
  int readCount = 0;
  int insCount = 0;

  if (is_adlib) {
    try {
      reader.seek(0x0c, SEEK_SET);
      uint32_t name_offset = reader.readI();
      reader.seek(0x10, SEEK_SET);
      uint32_t data_offset = reader.readI();

      // Seek to BNK patch names
      reader.seek(name_offset, SEEK_SET);
      while (reader.tell() < data_offset) {
        reader.seek(3, SEEK_CUR);
        instNames.push_back(new String(reader.readString(9)));
        ++insCount;
      }

      // Seek to BNK data
      if (!reader.seek(data_offset, SEEK_SET)) {
        throw EndOfFileException(&reader, data_offset);
      };

      // Read until all patches have been accounted for.
      for (int i = 0; i < insCount; ++i) {
        DivInstrument *ins = new DivInstrument;

        ins->type = DIV_INS_OPL;
        ins->fm.ops = 2;

        uint8_t timbreMode = reader.readC();
        reader.readC();  // skip timbre perc voice
        if (timbreMode == 1) {
          ins->fm.opllPreset = (uint8_t)(1<<4);
        }

        for (int i = 0; i < 2; ++i) {
          ins->fm.op[i].ksl = reader.readC();
          ins->fm.op[i].mult = reader.readC();
          uint8_t fb = reader.readC();
          if (i==0) {
            ins->fm.fb = fb;
          }
          ins->fm.op[i].ar = reader.readC();
          ins->fm.op[i].sl = reader.readC();
          ins->fm.op[i].sus = (reader.readC() != 0) ? 1 : 0;
          ins->fm.op[i].dr = reader.readC();
          ins->fm.op[i].rr = reader.readC();
          ins->fm.op[i].tl = reader.readC();
          ins->fm.op[i].am = reader.readC();
          ins->fm.op[i].vib = reader.readC();
          ins->fm.op[i].ksr = reader.readC();
          uint8_t alg = (reader.readC() == 0) ? 1 : 0;
          if (i==0) {
            ins->fm.alg = alg;
          }
        }
        ins->fm.op[0].ws = reader.readC();
        ins->fm.op[1].ws = reader.readC();
        ins->name = stringNotBlank(*instNames[i]) ? (*instNames[i]) : fmt::sprintf("%s[%d]", stripPath, i);

        insList.push_back(ins);
        ++readCount;
      }
      // All data read, don't care about the rest.
      reader.seek(0, SEEK_END);

    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file");
      for (int i = 0; i < readCount; ++i) {
        delete insList[i];
      }
      is_failed = true;
    }

  } else {
    // assume GEMS BNK for now.
    lastError="GEMS BNK currently not supported.";
    logE("GEMS BNK currently not supported.");
  }

  if (!is_failed) {
    for (int i = 0; i < readCount; ++i) {
      ret.push_back(insList[i]);
    }
  }

  for (String* name : instNames) {
    delete name;
  }
}