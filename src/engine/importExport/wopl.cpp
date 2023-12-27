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

void DivEngine::loadWOPL(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList;
  bool is_failed = false;

  uint16_t version;
  uint16_t meloBankCount;
  uint16_t percBankCount;
  std::vector<midibank_t*> meloMetadata;
  std::vector<midibank_t*> percMetadata;

  auto readWoplOp = [](SafeReader& reader, DivInstrumentFM::Operator& op) {
    uint8_t characteristics = reader.readC();
    uint8_t keyScaleLevel = reader.readC();
    uint8_t attackDecay = reader.readC();
    uint8_t sustainRelease = reader.readC();
    uint8_t waveSelect = reader.readC();
    int total = 0;

    total += (op.mult = characteristics & 0xF);
    total += (op.ksr = ((characteristics >> 4) & 0x1));
    total += (op.sus = ((characteristics >> 5) & 0x1));
    total += (op.vib = ((characteristics >> 6) & 0x1));
    total += (op.am = ((characteristics >> 7) & 0x1));
    total += (op.tl = keyScaleLevel & 0x3F);
    total += (op.ksl = ((keyScaleLevel >> 6) & 0x3));
    total += (op.ar = ((attackDecay >> 4) & 0xF));
    total += (op.dr = attackDecay & 0xF);
    total += (op.rr = sustainRelease & 0xF);
    total += (op.sl = ((sustainRelease >> 4) & 0xF));
    total += (op.ws = waveSelect);
    return total;
  };

  auto doParseWoplInstrument = [&](bool isPerc, midibank_t*& metadata, int patchNum) {
    DivInstrument* ins = new DivInstrument;
    try {
      long patchSum = 0;
      ins->type = DIV_INS_OPL;

      // Establish if it is a blank instrument.
      String insName = reader.readString(32);
      patchSum += insName.size();
      
      // TODO adapt MIDI key offset to transpose?
      reader.seek(7, SEEK_CUR);  // skip MIDI params
      uint8_t instTypeFlags = reader.readC();  // [0EEEDCBA] - see WOPL/OPLI spec

      bool is_4op = ((instTypeFlags & 0x1) == 1);
      bool is_2x2op = (((instTypeFlags>>1) & 0x1) == 1);
      bool is_rhythm = (((instTypeFlags>>4) & 0x7) > 0);

      uint8_t feedConnect = reader.readC();
      uint8_t feedConnect2nd = reader.readC();

      ins->fm.alg = (feedConnect & 0x1);
      ins->fm.fb = ((feedConnect>>1) & 0xF);

      if (is_4op && !is_2x2op) {
        ins->fm.ops = 4;
        ins->fm.alg = (feedConnect & 0x1) | ((feedConnect2nd & 0x1) << 1);
        for (int i : {2,0,3,1}) { // omfg >_<
          patchSum += readWoplOp(reader, ins->fm.op[i]);
        }
      } else {
        ins->fm.ops = 2;
        for (int i : {1,0}) {
          patchSum += readWoplOp(reader, ins->fm.op[i]);
        }
        if (is_rhythm) {
          ins->fm.opllPreset = (uint8_t)(1<<4);
        } else if (is_2x2op) {
          // Note: Pair detuning offset not mappable. Use E5xx effect :P
          ins->name = stringNotBlank(insName)
            ? fmt::sprintf("%s (1)", insName)
            : fmt::sprintf("%s[%s] %s Patch %d (1)",
              stripPath, metadata->name, (isPerc) ? "Drum" : "Melodic", patchNum);
          insList.push_back(ins);
          patchSum = 0;
          ins = new DivInstrument;
          ins->type = DIV_INS_OPL;
          ins->name = fmt::sprintf("%s (2)", insName);
          ins->fm.alg = (feedConnect2nd & 0x1);
          ins->fm.fb = ((feedConnect2nd >> 1) & 0xF);
          for (int i : {1,0}) {
            patchSum += readWoplOp(reader, ins->fm.op[i]);
          }
        }

        if (!is_2x2op) {
          reader.seek(10, SEEK_CUR); // skip unused operator pair
        }
      }

      if (version >= 3) {
        reader.readS_BE(); // skip keyon delay
        reader.readS_BE(); // skip keyoff delay
      }

      if (patchSum > 0) {
        // Write instrument
        // TODO: OPL3BankEditor hardcodes GM1 Melodic patch names which are not included in the bank file......
        if (is_2x2op) {
          ins->name = stringNotBlank(insName)
            ? fmt::sprintf("%s (2)", insName)
            : fmt::sprintf("%s[%s] %s Patch %d (2)",
              stripPath, metadata->name, (isPerc) ? "Drum" : "Melodic", patchNum);
        } else {
          ins->name = stringNotBlank(insName)
            ? insName
            : fmt::sprintf("%s[%s] %s Patch %d",
              stripPath, metadata->name, (isPerc) ? "Drum" : "Melodic", patchNum);
        }
        insList.push_back(ins);
      } else {
        // Empty instrument
        delete ins;
      }
    } catch (...) {
      // Deallocate and allow outer handler to do the rest.
      delete ins;
      throw;
    }
  };

  try {
    reader.seek(0, SEEK_SET);

    String header = reader.readString(11);
    if (header == "WOPL3-BANK") {
      version = reader.readS();
      meloBankCount = reader.readS_BE();
      percBankCount = reader.readS_BE();
      reader.readC(); // skip chip-global LFO
      reader.readC(); // skip additional flags

      if (version >= 2) {
        for (int i = 0; i < meloBankCount; ++i) {
          meloMetadata.push_back(new midibank_t);
          String bankName = reader.readString(32);
          meloMetadata[i]->bankLsb = reader.readC();
          meloMetadata[i]->bankMsb = reader.readC();
          meloMetadata[i]->name = stringNotBlank(bankName)
            ? bankName
            : fmt::sprintf("%d/%d", meloMetadata[i]->bankMsb, meloMetadata[i]->bankLsb);
        }

        for (int i = 0; i < percBankCount; ++i) {
          percMetadata.push_back(new midibank_t);
          String bankName = reader.readString(32);
          percMetadata[i]->bankLsb = reader.readC();
          percMetadata[i]->bankMsb = reader.readC();
          percMetadata[i]->name = stringNotBlank(bankName)
            ? bankName
            : fmt::sprintf("%d/%d", percMetadata[i]->bankMsb, percMetadata[i]->bankLsb);
        }
      } else {
        // TODO do version 1 multibank sets even exist?
        meloMetadata.push_back(new midibank_t);
        meloMetadata[0]->bankLsb = 0;
        meloMetadata[0]->bankMsb = 0;
        meloMetadata[0]->name = "0/0";
        percMetadata.push_back(new midibank_t);
        percMetadata[0]->bankLsb = 0;
        percMetadata[0]->bankMsb = 0;
        percMetadata[0]->name = "0/0";
      }

      for (int i = 0; i < meloBankCount; ++i) {
        for (int j = 0; j < 128; ++j) {
          doParseWoplInstrument(false, meloMetadata[i], j);
        }
      }
      for (int i = 0; i < percBankCount; ++i) {
        for (int j = 0; j < 128; ++j) {
          doParseWoplInstrument(true, percMetadata[i], j);
        }
      }
    }
  } catch (EndOfFileException& e) {
    lastError = "premature end of file";
    logE("premature end of file");
    is_failed = true;
  }

  for (midibank_t* m : meloMetadata) {
    delete m;
  }
  for (midibank_t* m : percMetadata) {
    delete m;
  }

  if (is_failed) {
    for (DivInstrument* p : insList) {
      delete p;
    }
  } else {
    for (DivInstrument* p : insList) {
      ret.push_back(p);
    }
  }
}