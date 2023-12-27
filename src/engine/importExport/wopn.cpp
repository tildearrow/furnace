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

void DivEngine::loadWOPN(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList;
  bool is_failed = false;

  uint16_t version;
  uint16_t meloBankCount;
  uint16_t percBankCount;
  std::vector<midibank_t*> meloMetadata;
  std::vector<midibank_t*> percMetadata;

  auto readWopnOp = [](SafeReader& reader, DivInstrumentFM::Operator& op) {
    uint8_t dtMul = reader.readC();
    uint8_t totalLevel = reader.readC();
    uint8_t arRateScale = reader.readC();
    uint8_t drAmpEnable = reader.readC();
    uint8_t d2r = reader.readC();
    uint8_t susRelease = reader.readC();
    uint8_t ssgEg = reader.readC();
    int total = 0;

    total += (op.mult = dtMul & 0xF);
    total += (op.dt = ((dtMul >> 4) & 0x7));
    total += (op.tl = totalLevel & 0x7F);
    total += (op.rs = ((arRateScale >> 6) & 0x3));
    total += (op.ar = arRateScale & 0x1F);
    total += (op.dr = drAmpEnable & 0x1F);
    total += (op.am = ((drAmpEnable >> 7) & 0x1));
    total += (op.d2r = d2r & 0x1F);
    total += (op.rr = susRelease & 0xF);
    total += (op.sl = ((susRelease >> 4) & 0xF));
    total += (op.ssgEnv = ssgEg);
    return total;
  };
  auto doParseWopnInstrument = [&](bool isPerc, midibank_t*& metadata, int patchNum) {
    DivInstrument* ins = new DivInstrument;
    try {
      long patchSum = 0;
      ins->type = DIV_INS_FM;
      ins->fm.ops = 4;

      // Establish if it is a blank instrument.
      String insName = reader.readString(32);
      patchSum += insName.size();

      // TODO adapt MIDI key offset to transpose?
      if (!reader.seek(3, SEEK_CUR)) {  // skip MIDI params
        throw EndOfFileException(&reader, reader.tell() + 3);
      }
      uint8_t feedAlgo = reader.readC();
      patchSum += feedAlgo;
      ins->fm.alg = (feedAlgo & 0x7);
      ins->fm.fb = ((feedAlgo >> 3) & 0x7);
      patchSum += reader.readC();  // Skip global bank flags - see WOPN/OPNI spec

      for (int i = 0; i < 4; ++i) {
        patchSum += readWopnOp(reader, ins->fm.op[i]);
      }

      if (version >= 2) {
        reader.readS_BE(); // skip keyon delay
        reader.readS_BE(); // skip keyoff delay
      }

      if (patchSum > 0) {
        // Write instrument
        // TODO: OPN2BankEditor hardcodes GM1 Melodic patch names which are not included in the bank file......
        ins->name = stringNotBlank(insName) 
          ? insName 
          : fmt::sprintf("%s[%s] %s Patch %d", 
            stripPath, metadata->name, (isPerc) ? "Drum" : "Melodic", patchNum);
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
    if (header == "WOPN2-BANK" || header == "WOPN2-B2NK") {  // omfg >_<
      version = reader.readS();
      if (!(version >= 2) || version > 0xF) {
        // version 1 doesn't have a version field........
        reader.seek(-2, SEEK_CUR);
        version = 1;
      }

      meloBankCount = reader.readS_BE();
      percBankCount = reader.readS_BE();
      reader.readC(); // skip chip-global LFO

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
          doParseWopnInstrument(false, meloMetadata[i], j);
        }
      }
      for (int i = 0; i < percBankCount; ++i) {
        for (int j = 0; j < 128; ++j) {
          doParseWopnInstrument(true, percMetadata[i], j);
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