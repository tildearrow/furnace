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

void DivEngine::loadGYB(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList;
  int readCount = 0;
  bool is_failed = false;

  auto readInstrument = [&](SafeReader& reader, bool readRegB4) -> DivInstrument* {
    const int opOrder[] = { 0,1,2,3 };
    DivInstrument* ins = new DivInstrument;
    ins->type = DIV_INS_FM;
    ins->fm.ops = 4;

    // see https://plutiedev.com/ym2612-registers 
    // and https://github.com/Wohlstand/OPN2BankEditor/blob/master/Specifications/GYB-file-specification.txt

    try {
      uint8_t reg;
      for (int i : opOrder) {
        reg = reader.readC(); // MUL/DT
        ins->fm.op[i].mult = reg & 0xF;
        ins->fm.op[i].dt = fmDtRegisterToFurnace((reg >> 4) & 0x7);
      }
      for (int i : opOrder) {
        reg = reader.readC(); // TL
        ins->fm.op[i].tl = reg & 0x7F;
      }
      for (int i : opOrder) {
        reg = reader.readC(); // AR/RS
        ins->fm.op[i].ar = reg & 0x1F;
        ins->fm.op[i].rs = ((reg >> 6) & 0x3);
      }
      for (int i : opOrder) {
        reg = reader.readC(); // DR/AM-ENA
        ins->fm.op[i].dr = reg & 0x1F;
        ins->fm.op[i].am = ((reg >> 7) & 0x1);
      }
      for (int i : opOrder) {
        reg = reader.readC(); // SR (D2R)
        ins->fm.op[i].d2r = reg & 0x1F;
      }
      for (int i : opOrder) {
        reg = reader.readC(); // RR/SL
        ins->fm.op[i].rr = reg & 0xF;
        ins->fm.op[i].sl = ((reg >> 4) & 0xF);
      }
      for (int i : opOrder) {
        reg = reader.readC(); // SSG-EG
        ins->fm.op[i].ssgEnv = reg & 0xF;
      }
      // ALG/FB
      reg = reader.readC();
      ins->fm.alg = reg & 0x7;
      ins->fm.fb = ((reg >> 3) & 0x7);

      if (readRegB4) { // PAN / PMS / AMS
        reg = reader.readC();
        ins->fm.fms = reg & 0x7;
        ins->fm.ams = ((reg >> 4) & 0x3);
      }
      insList.push_back(ins);
      ++readCount;
      return ins;

    } catch (...) {
      // Deallocate and rethrow to outer handler
      delete ins;
      throw;
    }
  };
  auto readInstrumentName = [&](SafeReader& reader, DivInstrument* ins) {
    uint8_t nameLen = reader.readC();
    String insName = (nameLen>0) ? reader.readString(nameLen) : "";
    ins->name = stringNotBlank(insName) 
      ? insName 
      : fmt::sprintf("%s [%d]", stripPath, readCount - 1);
  };

  try {
    reader.seek(0, SEEK_SET);
    uint16_t header = reader.readS();
    uint8_t insMelodyCount, insDrumCount;

    if (header == 0x0C1A) { // 26 12 in decimal bytes
      uint8_t version = reader.readC();

      if ((version ^ 3) > 0) {
        // GYBv1/2
        insMelodyCount = reader.readC();
        insDrumCount = reader.readC();

        if (insMelodyCount > 128 || insDrumCount > 128) {
          throw std::invalid_argument("GYBv1/2 patch count is out of bounds.");
        }

        if (!reader.seek(0x100, SEEK_CUR)) { // skip MIDI instrument mapping
          throw EndOfFileException(&reader, reader.tell() + 0x100);
        }

        if (version == 2) {
          reader.readC(); // skip LFO speed (chip-global)
        }

        // Instrument data
        for (int i = 0; i < (insMelodyCount+insDrumCount); ++i) {
          readInstrument(reader, (version == 2));

          // Additional data
          reader.readC();  // skip transpose
          if (version == 2) {
            reader.readC();  // skip padding
          }
        }

        // Instrument name
        for (int i = 0; i < (insMelodyCount+insDrumCount); ++i) {
          readInstrumentName(reader, insList[i]);
        }

        // Map to note assignment currently not supported.

      } else {
        // GYBv3+
        reader.readC();  // skip LFO speed (chip-global)
        uint32_t fileSize = reader.readI();
        uint32_t bankOffset = reader.readI();
        uint32_t mapOffset = reader.readI();

        if (bankOffset > fileSize || mapOffset > fileSize) {
          lastError = "GYBv3 file appears to have invalid data offsets.";
          logE("GYBv3 file appears to have invalid data offsets.");
        }

        if (!reader.seek(bankOffset, SEEK_SET)) {
          throw EndOfFileException(&reader, bankOffset);
        }
        uint16_t insCount = reader.readS();

        size_t patchPosOffset = reader.tell();
        for (int i = 0; i < insCount; ++i) {
          uint16_t patchSize = reader.readS();
          readInstrument(reader, true);

          // Additional data
          reader.readC(); // skip transpose
          uint8_t additionalDataFlags = reader.readC() & 0x1; // skip additional data bitfield
          
          // if chord notes attached, skip this
          if ((additionalDataFlags&1) > 0) {
            uint8_t notes = reader.readC();
            for (int j = 0; j < notes; ++j) {
              reader.readC();
            }
          }

          // Instrument Name
          readInstrumentName(reader, insList[i]);

          // Retrieve next patch
          if (!reader.seek(patchPosOffset + patchSize, SEEK_SET)) {
            throw EndOfFileException(&reader, patchPosOffset + patchSize);
          }
          patchPosOffset = reader.tell();
        }
      }
      reader.seek(0, SEEK_END);
    }
    
  } catch (EndOfFileException& e) {
    lastError = "premature end of file";
    logE("premature end of file");
    is_failed = true;

  } catch (std::invalid_argument& e) {
    lastError = fmt::sprintf("Invalid value found in patch file. %s", e.what());
    logE("Invalid value found in patch file.");
    logE(e.what());
    is_failed = true;
  }

  if (!is_failed) {
    for (int i = 0; i < readCount; ++i) {
      if (insList[i] != NULL) {
        ret.push_back(insList[i]);
      }
    }
  } else {
    for (int i = 0; i < readCount; ++i) {
      delete insList[i];
    }
  }
}