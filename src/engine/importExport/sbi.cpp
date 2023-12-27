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

void DivEngine::loadSBI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList; // in case 2x2op
  DivInstrument* ins=new DivInstrument;
  try {
    reader.seek(0, SEEK_SET);
    ins->type = DIV_INS_OPL;

    int sbi_header = reader.readI();
    // SBI header determines format
    bool is_2op = (sbi_header == 0x1A494253 || sbi_header == 0x1A504F32); // SBI\x1A or 2OP\x1A
    bool is_4op = (sbi_header == 0x1A504F34); // 4OP\x1A
    bool is_6op = (sbi_header == 0x1A504F36); // 6OP\x1A - Freq Monster 801-specific

    // 32-byte null terminated instrument name
    String insName = reader.readString(32);
    insName = stringNotBlank(insName) ? insName : stripPath;

    auto writeOp = [](sbi_t& sbi, DivInstrumentFM::Operator& opM, DivInstrumentFM::Operator& opC) {
      opM.mult = sbi.Mcharacteristics & 0xF;
      opM.ksr = ((sbi.Mcharacteristics >> 4) & 0x1);
      opM.sus = ((sbi.Mcharacteristics >> 5) & 0x1);
      opM.vib = ((sbi.Mcharacteristics >> 6) & 0x1);
      opM.am = ((sbi.Mcharacteristics >> 7) & 0x1);
      opM.tl = sbi.Mscaling_output & 0x3F;
      opM.ksl = ((sbi.Mscaling_output >> 6) & 0x3);
      opM.ar = ((sbi.Meg_AD >> 4) & 0xF);
      opM.dr = (sbi.Meg_AD & 0xF);
      opM.rr = (sbi.Meg_SR & 0xF);
      opM.sl = ((sbi.Meg_SR >> 4) & 0xF);
      opM.ws = sbi.Mwave;

      opC.mult = sbi.Ccharacteristics & 0xF;
      opC.ksr = ((sbi.Ccharacteristics >> 4) & 0x1);
      opC.sus = ((sbi.Ccharacteristics >> 5) & 0x1);
      opC.vib = ((sbi.Ccharacteristics >> 6) & 0x1);
      opC.am = ((sbi.Ccharacteristics >> 7) & 0x1);
      opC.tl = sbi.Cscaling_output & 0x3F;
      opC.ksl = ((sbi.Cscaling_output >> 6) & 0x3);
      opC.ar = ((sbi.Ceg_AD >> 4) & 0xF);
      opC.dr = (sbi.Ceg_AD & 0xF);
      opC.rr = (sbi.Ceg_SR & 0xF);
      opC.sl = ((sbi.Ceg_SR >> 4) & 0xF);
      opC.ws = sbi.Cwave;
    };

    sbi_t sbi_op12;  // 2op (+6op portion)
    sbi_t sbi_op34;  // 4op
    
    readSbiOpData(sbi_op12, reader);

    if (is_2op) {
      DivInstrumentFM::Operator& opM = ins->fm.op[0];
      DivInstrumentFM::Operator& opC = ins->fm.op[1];
      ins->fm.ops = 2;
      ins->name = insName;
      writeOp(sbi_op12, opM, opC);
      ins->fm.alg = (sbi_op12.FeedConnect & 0x1);
      ins->fm.fb = ((sbi_op12.FeedConnect >> 1) & 0x7);

      // SBTimbre extensions
      uint8_t perc_voc = reader.readC();
      if (perc_voc >= 6) {
        ins->fm.opllPreset = (uint8_t)(1 << 4);
      }

      // Ignore rest of file - rest is 'reserved padding'.
      reader.seek(4, SEEK_CUR);
      insList.push_back(ins);

    } else if (is_4op || is_6op) {
      readSbiOpData(sbi_op34, reader);
      
      // Operator placement is different so need to place in correct registers.
      // Note: 6op is an unofficial extension of 4op SBIs by Darron Broad (Freq Monster 801).
      // We'll only use the 4op portion here for pure OPL3.
      DivInstrumentFM::Operator& opM = ins->fm.op[0];
      DivInstrumentFM::Operator& opC = ins->fm.op[2];
      DivInstrumentFM::Operator& opM4 = ins->fm.op[1];
      DivInstrumentFM::Operator& opC4 = ins->fm.op[3];
      ins->fm.ops = 4;
      ins->name = insName;
      ins->fm.alg = (sbi_op12.FeedConnect & 0x1) | ((sbi_op34.FeedConnect & 0x1) << 1);
      ins->fm.fb = ((sbi_op34.FeedConnect >> 1) & 0x7);
      writeOp(sbi_op12, opM, opC);
      writeOp(sbi_op34, opM4, opC4);

      if (is_6op) {
        // Freq Monster 801 6op SBIs use a 4+2op layout
        // Save the 4op portion before reading the 2op part
        ins->name = fmt::sprintf("%s (4op)", ins->name);
        insList.push_back(ins);

        readSbiOpData(sbi_op12, reader);

        ins = new DivInstrument;
        DivInstrumentFM::Operator& opM6 = ins->fm.op[0];
        DivInstrumentFM::Operator& opC6 = ins->fm.op[1];
        ins->type = DIV_INS_OPL;
        ins->fm.ops = 2;
        ins->name = fmt::sprintf("%s (2op)", insName);
        writeOp(sbi_op12, opM6, opC6);
        ins->fm.alg = (sbi_op12.FeedConnect & 0x1);
        ins->fm.fb = ((sbi_op12.FeedConnect >> 1) & 0x7);
      }

      // Ignore rest of file once we've read in all we need.
      // Note: Freq Monster 801 adds a ton of other additional fields irrelevant to chip registers.
      //       If instrument transpose is ever supported, we can read it in maybe?
      reader.seek(0, SEEK_END);
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