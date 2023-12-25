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

void DivEngine::loadS3I(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins=new DivInstrument;
  try {
    reader.seek(0, SEEK_SET);

    uint8_t s3i_type = reader.readC();

    if (s3i_type >= 2) {
      ins->type = DIV_INS_OPL;
      if (s3i_type > 2 && s3i_type <= 7) {
        ins->fm.opllPreset = (uint8_t)(1<<4);  // Flag as Drum preset.
      }
      // skip internal filename - we'll use the long name description
      reader.seek(12, SEEK_CUR);

      // skip reserved bytes
      reader.seek(3, SEEK_CUR);

      // 12-byte opl value - identical to SBI format
      sbi_t s3i;
      readSbiOpData(s3i, reader);
      
      DivInstrumentFM::Operator& opM = ins->fm.op[0];
      DivInstrumentFM::Operator& opC = ins->fm.op[1];
      ins->fm.ops = 2;
      opM.mult = s3i.Mcharacteristics & 0xF;
      opM.ksr = ((s3i.Mcharacteristics >> 4) & 0x1);
      opM.sus = ((s3i.Mcharacteristics >> 5) & 0x1);
      opM.vib = ((s3i.Mcharacteristics >> 6) & 0x1);
      opM.am = ((s3i.Mcharacteristics >> 7) & 0x1);
      opM.tl = s3i.Mscaling_output & 0x3F;
      opM.ksl = ((s3i.Mscaling_output >> 6) & 0x3);
      opM.ar = ((s3i.Meg_AD >> 4) & 0xF);
      opM.dr = (s3i.Meg_AD & 0xF);
      opM.rr = (s3i.Meg_SR & 0xF);
      opM.sl = ((s3i.Meg_SR >> 4) & 0xF);
      opM.ws = s3i.Mwave;

      ins->fm.alg = (s3i.FeedConnect & 0x1);
      ins->fm.fb = ((s3i.FeedConnect >> 1) & 0x7);

      opC.mult = s3i.Ccharacteristics & 0xF;
      opC.ksr = ((s3i.Ccharacteristics >> 4) & 0x1);
      opC.sus = ((s3i.Ccharacteristics >> 5) & 0x1);
      opC.vib = ((s3i.Ccharacteristics >> 6) & 0x1);
      opC.am = ((s3i.Ccharacteristics >> 7) & 0x1);
      opC.tl = s3i.Cscaling_output & 0x3F;
      opC.ksl = ((s3i.Cscaling_output >> 6) & 0x3);
      opC.ar = ((s3i.Ceg_AD >> 4) & 0xF);
      opC.dr = (s3i.Ceg_AD & 0xF);
      opC.rr = (s3i.Ceg_SR & 0xF);
      opC.sl = ((s3i.Ceg_SR >> 4) & 0xF);
      opC.ws = s3i.Cwave;

      // Skip more stuff we don't need
      reader.seek(21, SEEK_CUR);
    } else {
      lastError="S3I PCM samples currently not supported.";
      logE("S3I PCM samples currently not supported.");
    }
    String insName = reader.readString(28);
    ins->name = stringNotBlank(insName) ? insName : stripPath;

    int s3i_signature = reader.readI();

    if (s3i_signature != 0x49524353) {
      addWarning("S3I signature invalid.");
      logW("S3I signature invalid.");
    };
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
    return;
  }

  ret.push_back(ins);
}