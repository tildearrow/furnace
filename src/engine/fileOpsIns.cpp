/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#include "engine.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include <fmt/printf.h>

enum DivInsFormats {
  DIV_INSFORMAT_DMP,
  DIV_INSFORMAT_TFI,
  DIV_INSFORMAT_VGI,
  DIV_INSFORMAT_FTI,
  DIV_INSFORMAT_BTI,
  DIV_INSFORMAT_S3I,
  DIV_INSFORMAT_SBI,
  DIV_INSFORMAT_Y12,
  DIV_INSFORMAT_OPLI,
  DIV_INSFORMAT_OPNI,
  DIV_INSFORMAT_BNK,
  DIV_INSFORMAT_GYB,
  DIV_INSFORMAT_OPM,
  DIV_INSFORMAT_FF,
};

// Patch data structures

// SBI and some other OPL containers
struct sbi_t {
  uint8_t Mcharacteristics,
          Ccharacteristics,
          Mscaling_output,
          Cscaling_output,
          Meg_AD,
          Ceg_AD,
          Meg_SR,
          Ceg_SR,
          Mwave,
          Cwave,
          FeedConnect;
};

// Adlib Visual Composer BNK
struct bnkop_t {
  uint8_t ksl,
          multiple,
          feedback, // op1 only
          attack,
          sustain,
          eg,
          decay,
          releaseRate,
          totalLevel,
          am,
          vib,
          ksr,
          con; // op1 only
};
struct bnktimbre_t {
  uint8_t mode,
          percVoice;
  bnkop_t op[2];
  uint8_t wave0,
          wave1;
};

auto readSbiOpData = [](sbi_t& sbi, SafeReader& reader) {
  sbi.Mcharacteristics = reader.readC();
  sbi.Ccharacteristics = reader.readC();
  sbi.Mscaling_output = reader.readC();
  sbi.Cscaling_output = reader.readC();
  sbi.Meg_AD = reader.readC();
  sbi.Ceg_AD = reader.readC();
  sbi.Meg_SR = reader.readC();
  sbi.Ceg_SR = reader.readC();
  sbi.Mwave = reader.readC();
  sbi.Cwave = reader.readC();
  sbi.FeedConnect = reader.readC();
};

void DivEngine::loadDMP(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins=new DivInstrument;
  // this is a ridiculous mess
  unsigned char version=0;
  unsigned char sys=0;
  try {
    reader.seek(0,SEEK_SET);
    version=reader.readC();
    logD(".dmp version %d",version);
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
    return;
  }

  if (version>11) {
    lastError="unknown instrument version!";
    delete ins;
    return;
  }

  ins->name=stripPath;

  if (version>=11) { // 1.0
    try {
      sys=reader.readC();

      switch (sys) {
        case 1: // YMU759
          ins->type=DIV_INS_FM;
          logD("instrument type is YMU759");
          break;
        case 2: // Genesis
          ins->type=DIV_INS_FM;
          logD("instrument type is Genesis");
          break;
        case 3: // SMS
          ins->type=DIV_INS_STD;
          logD("instrument type is SMS");
          break;
        case 4: // Game Boy
          ins->type=DIV_INS_GB;
          logD("instrument type is Game Boy");
          break;
        case 5: // PC Engine
          ins->type=DIV_INS_PCE;
          logD("instrument type is PC Engine");
          break;
        case 6: // NES
          ins->type=DIV_INS_STD;
          logD("instrument type is NES");
          break;
        case 7: case 0x17: // C64
          ins->type=DIV_INS_C64;
          logD("instrument type is C64");
          break;
        case 8: // Arcade
          ins->type=DIV_INS_FM;
          logD("instrument type is Arcade");
          break;
        default:
          logD("instrument type is unknown");
          lastError="unknown instrument type!";
          delete ins;
          return;
          break;
      }
    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file");
      delete ins;
      return;
    }
  }

  try {
    bool mode=true;
    if (version>1) {
      mode=reader.readC();
      logD("instrument mode is %d",mode);
      if (mode==0) {
        if (version<11) {
          ins->type=DIV_INS_STD;
        }
      } else {
        ins->type=DIV_INS_FM;
      }
    } else {
      ins->type=DIV_INS_FM;
    }

    if (mode) { // FM
      logD("reading FM data...");
      if (version<10) {
        if (version>1) {
          // bullcrap! no way to determine the instrument type other than a vague FM/STD!
          if (reader.size()==51) {
            reader.readC();
            ins->fm.ops=4;
          } else {
            ins->fm.ops=reader.readC()?4:2;
          }
        } else {
          ins->fm.ops=reader.readC()?2:4;
        }
      } else {
        ins->fm.ops=4;
      }
      if (version>1) { // HELP! in which version of the format did we start storing FMS!
        ins->fm.fms=reader.readC();
      }
      ins->fm.fb=reader.readC();
      ins->fm.alg=reader.readC();
      // DITTO
      if (sys!=1) ins->fm.ams=reader.readC();

      for (int j=0; j<ins->fm.ops; j++) {
        logD("OP%d is at %d",j,reader.tell());
        ins->fm.op[j].mult=reader.readC();
        ins->fm.op[j].tl=reader.readC();
        ins->fm.op[j].ar=reader.readC();
        ins->fm.op[j].dr=reader.readC();
        ins->fm.op[j].sl=reader.readC();
        ins->fm.op[j].rr=reader.readC();
        ins->fm.op[j].am=reader.readC();
        // what the hell how do I tell!
        if (sys==1) { // YMU759
          ins->fm.op[j].ws=reader.readC();
          ins->fm.op[j].ksl=reader.readC();
          ins->fm.op[j].vib=reader.readC();
          ins->fm.op[j].egt=reader.readC();
          ins->fm.op[j].sus=reader.readC();
          ins->fm.op[j].ksr=reader.readC();
          ins->fm.op[j].dvb=reader.readC();
          ins->fm.op[j].dam=reader.readC();
        } else {
          ins->fm.op[j].rs=reader.readC();
          ins->fm.op[j].dt=reader.readC();
          ins->fm.op[j].dt2=ins->fm.op[j].dt>>4;
          ins->fm.op[j].dt&=15;
          ins->fm.op[j].d2r=reader.readC();
          ins->fm.op[j].ssgEnv=reader.readC();
        }
      }
    } else { // STD
      logD("reading STD data...");
      if (ins->type!=DIV_INS_GB) {
        ins->std.volMacro.len=reader.readC();
        if (version>5) {
          for (int i=0; i<ins->std.volMacro.len; i++) {
            ins->std.volMacro.val[i]=reader.readI();
          }
        } else {
          for (int i=0; i<ins->std.volMacro.len; i++) {
            ins->std.volMacro.val[i]=reader.readC();
          }
        }
        if (version<11) for (int i=0; i<ins->std.volMacro.len; i++) {
          if (ins->std.volMacro.val[i]>15 && ins->type==DIV_INS_STD) ins->type=DIV_INS_PCE;
        }
        if (ins->std.volMacro.len>0) {
          ins->std.volMacro.open=true;
          ins->std.volMacro.loop=reader.readC();
        } else {
          ins->std.volMacro.open=false;
        }
      }

      ins->std.arpMacro.len=reader.readC();
      if (version>5) {
        for (int i=0; i<ins->std.arpMacro.len; i++) {
          ins->std.arpMacro.val[i]=reader.readI();
        }
      } else {
        for (int i=0; i<ins->std.arpMacro.len; i++) {
          ins->std.arpMacro.val[i]=reader.readC();
        }
      }
      if (ins->std.arpMacro.len>0) {
        ins->std.arpMacro.open=true;
        ins->std.arpMacro.loop=reader.readC();
      } else {
        ins->std.arpMacro.open=false;
      }
      if (version>8) { // TODO: when?
        ins->std.arpMacro.mode=reader.readC();
      }

      ins->std.dutyMacro.len=reader.readC();
      if (version>5) {
        for (int i=0; i<ins->std.dutyMacro.len; i++) {
          ins->std.dutyMacro.val[i]=reader.readI();
        }
      } else {
        for (int i=0; i<ins->std.dutyMacro.len; i++) {
          ins->std.dutyMacro.val[i]=reader.readC();
        }
      }
      if (ins->std.dutyMacro.len>0) {
        ins->std.dutyMacro.open=true;
        ins->std.dutyMacro.loop=reader.readC();
      } else {
        ins->std.dutyMacro.open=false;
      }

      ins->std.waveMacro.len=reader.readC();
      if (version>5) {
        for (int i=0; i<ins->std.waveMacro.len; i++) {
          ins->std.waveMacro.val[i]=reader.readI();
        }
      } else {
        for (int i=0; i<ins->std.waveMacro.len; i++) {
          ins->std.waveMacro.val[i]=reader.readC();
        }
      }
      if (ins->std.waveMacro.len>0) {
        ins->std.waveMacro.open=true;
        ins->std.waveMacro.loop=reader.readC();
      } else {
        ins->std.waveMacro.open=false;
      }

      if (ins->type==DIV_INS_C64) {
        ins->c64.triOn=reader.readC();
        ins->c64.sawOn=reader.readC();
        ins->c64.pulseOn=reader.readC();
        ins->c64.noiseOn=reader.readC();

        ins->c64.a=reader.readC();
        ins->c64.d=reader.readC();
        ins->c64.s=reader.readC();
        ins->c64.r=reader.readC();

        ins->c64.duty=(reader.readC()*4095)/100;

        ins->c64.ringMod=reader.readC();
        ins->c64.oscSync=reader.readC();
        ins->c64.toFilter=reader.readC();
        if (version<0x07) { // TODO: UNSURE
          ins->c64.volIsCutoff=reader.readI();
        } else {
          ins->c64.volIsCutoff=reader.readC();
        }
        ins->c64.initFilter=reader.readC();

        ins->c64.res=reader.readC();
        ins->c64.cut=(reader.readC()*2047)/100;
        ins->c64.hp=reader.readC();
        ins->c64.bp=reader.readC();
        ins->c64.lp=reader.readC();
        ins->c64.ch3off=reader.readC();

        // weird storage
        if (ins->c64.volIsCutoff) {
          for (int j=0; j<ins->std.volMacro.len; j++) {
            ins->std.volMacro.val[j]-=18;
          }
        }
        for (int j=0; j<ins->std.dutyMacro.len; j++) {
          ins->std.dutyMacro.val[j]-=12;
        }
      }
      if (ins->type==DIV_INS_GB) {
        ins->gb.envVol=reader.readC();
        ins->gb.envDir=reader.readC();
        ins->gb.envLen=reader.readC();
        ins->gb.soundLen=reader.readC();
      }
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
    return;
  }

  ret.push_back(ins);
}

void DivEngine::loadTFI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins=new DivInstrument;
  try {
    reader.seek(0,SEEK_SET);

    ins->type=DIV_INS_FM;
    ins->name=stripPath;
    
    ins->fm.alg=reader.readC();
    ins->fm.fb=reader.readC();

    for (int i=0; i<4; i++) {
      DivInstrumentFM::Operator& op=ins->fm.op[i];

      op.mult=reader.readC();
      op.dt=reader.readC();
      op.tl=reader.readC();
      op.rs=reader.readC();
      op.ar=reader.readC();
      op.dr=reader.readC();
      op.d2r=reader.readC();
      op.rr=reader.readC();
      op.sl=reader.readC();
      op.ssgEnv=reader.readC();
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
    return;
  }

  ret.push_back(ins);
}

void DivEngine::loadVGI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins=new DivInstrument;
  try {
    reader.seek(0,SEEK_SET);

    ins->type=DIV_INS_FM;
    ins->name=stripPath;
    
    ins->fm.alg=reader.readC();
    ins->fm.fb=reader.readC();
    unsigned char fmsams=reader.readC();
    ins->fm.fms=fmsams&7;
    ins->fm.ams=fmsams>>4;

    for (int i=0; i<4; i++) {
      DivInstrumentFM::Operator& op=ins->fm.op[i];

      op.mult=reader.readC();
      op.dt=reader.readC();
      op.tl=reader.readC();
      op.rs=reader.readC();
      op.ar=reader.readC();
      op.dr=reader.readC();
      if (op.dr&0x80) {
        op.am=1;
        op.dr&=0x7f;
      }
      op.d2r=reader.readC();
      op.rr=reader.readC();
      op.sl=reader.readC();
      op.ssgEnv=reader.readC();
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
    return;
  }

  ret.push_back(ins);
}

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
    ins->name = reader.readString(28);
    ins->name = (ins->name.size() == 0) ? stripPath : ins->name;

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

void DivEngine::loadSBI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
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
    String patchName = reader.readString(32);
    patchName = (patchName.size() == 0) ? stripPath : patchName;

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
      ins->name = patchName;
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
      ret.push_back(ins);

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
      ins->name = patchName;
      ins->fm.alg = (sbi_op12.FeedConnect & 0x1) | ((sbi_op34.FeedConnect & 0x1) << 1);
      ins->fm.fb = ((sbi_op34.FeedConnect >> 1) & 0x7);
      writeOp(sbi_op12, opM, opC);
      writeOp(sbi_op34, opM4, opC4);

      if (is_6op) {
        // Freq Monster 801 6op SBIs use a 4+2op layout
        // Save the 4op portion before reading the 2op part
        ins->name = fmt::sprintf("%s (4op)", ins->name);
        ret.push_back(ins);

        readSbiOpData(sbi_op12, reader);

        ins = new DivInstrument;
        DivInstrumentFM::Operator& opM6 = ins->fm.op[0];
        DivInstrumentFM::Operator& opC6 = ins->fm.op[1];
        ins->type = DIV_INS_OPL;
        ins->fm.ops = 2;
        ins->name = fmt::sprintf("%s (2op)", patchName);
        writeOp(sbi_op12, opM6, opC6);
        ins->fm.alg = (sbi_op12.FeedConnect & 0x1);
        ins->fm.fb = ((sbi_op12.FeedConnect >> 1) & 0x7);
      }

      // Ignore rest of file once we've read in all we need.
      // Note: Freq Monster 801 adds a ton of other additional fields irrelevant to chip registers.
      //       If instrument transpose is ever supported, we can read it in maybe?
      reader.seek(0, SEEK_END);
      ret.push_back(ins);
    }

  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
  }
}

void DivEngine::loadOPLI(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  DivInstrument* ins = new DivInstrument;

  try {
    reader.seek(0, SEEK_SET);
    String header = reader.readString(11);
    if (header == "WOPL3-INST") {
      uint16_t version = reader.readS();
      if (version > 3) {
        logW("Unknown OPLI version.");
      }

      reader.readC();  // skip isPerc field

      ins->type = DIV_INS_OPL;
      String insName = reader.readString(32);
      insName = (insName.size() > 0) ? insName : stripPath;
      ins->name = insName;
      reader.seek(7, SEEK_CUR);  // skip MIDI params
      uint8_t instTypeFlags = reader.readC();  // [0EEEDCBA] - see WOPL/OPLI spec

      bool is_4op = ((instTypeFlags & 0x1) == 1);
      bool is_2x2op = (((instTypeFlags>>1) & 0x1) == 1);
      bool is_rhythm = (((instTypeFlags>>4) & 0x7) > 0);

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
          ret.push_back(ins);

          ins = new DivInstrument;
          ins->type = DIV_INS_OPL;
          ins->name = fmt::sprintf("%s (2)", insName);
          for (int i : {1,0}) {
            readOpliOp(reader, ins->fm.op[i]);
          }
        }
      }

      // Skip rest of file
      reader.seek(0, SEEK_END);
      ret.push_back(ins);
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
  }
}

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
      }

      reader.readC(); // skip isPerc
      ins->type = DIV_INS_FM;
      ins->fm.ops = 4;

      String insName = reader.readString(32);
      ins->name = (insName.size() > 0) ? insName : stripPath;
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
        op.tl = totalLevel & 0x3F;
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

      // Skip rest of file
      reader.seek(0, SEEK_END);
      ret.push_back(ins);
    }
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
  }
}

void DivEngine::loadY12(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {  
  DivInstrument *ins = new DivInstrument;

  try {
    reader.seek(0, SEEK_SET);
    ins->type = DIV_INS_FM;
    ins->fm.ops = 4;
    ins->name = stripPath;

    for (int i = 0; i < 4; ++i) {
      DivInstrumentFM::Operator& insOp = ins->fm.op[i];
      uint8_t tmp = reader.readC();
      insOp.mult = tmp & 0xF;
      insOp.dt = ((tmp >> 4) & 0x7);
      insOp.tl = (reader.readC() & 0x3F);
      tmp = reader.readC();
      insOp.rs = ((tmp >> 6) & 0x3);
      insOp.ar = tmp & 0x1F;
      tmp = reader.readC();
      insOp.dr = tmp & 0x1F;
      insOp.am = ((tmp >> 7) & 0x1);
      insOp.d2r = (reader.readC() & 0x1F);
      tmp = reader.readC();
      insOp.rr = tmp & 0xF;
      insOp.sl = ((tmp >> 4) & 0xF);
      insOp.ssgEnv = reader.readC();
      if (!reader.seek(9, SEEK_CUR)) {
        throw EndOfFileException(&reader, reader.tell() + 9);
      }
    }
    ins->fm.alg = reader.readC();
    ins->fm.fb = reader.readC();
    if (!reader.seek(62, SEEK_CUR)) {
      throw EndOfFileException(&reader, reader.tell() + 62);
    }
    ret.push_back(ins);
  } catch (EndOfFileException& e) {
    lastError="premature end of file";
    logE("premature end of file");
    delete ins;
  }
}

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

      // Read until EOF
      for (int i = 0; i < insCount; ++i) {
        bnktimbre_t timbre;
        DivInstrument *ins = new DivInstrument;

        ins->type = DIV_INS_OPL;
        ins->fm.ops = 2;

        timbre.mode = reader.readC();
        timbre.percVoice = reader.readC();
        if (timbre.mode == 1) {
          ins->fm.opllPreset = (uint8_t)(1<<4);
        }
        ins->fm.op[0].ksl = reader.readC();
        ins->fm.op[0].mult = reader.readC();
        ins->fm.fb = reader.readC();
        ins->fm.op[0].ar = reader.readC();
        ins->fm.op[0].sl = reader.readC();
        ins->fm.op[0].sus = (reader.readC() != 0) ? 1 : 0;
        ins->fm.op[0].dr = reader.readC();
        ins->fm.op[0].rr = reader.readC();
        ins->fm.op[0].tl = reader.readC();
        ins->fm.op[0].am = reader.readC();
        ins->fm.op[0].vib = reader.readC();
        ins->fm.op[0].ksr = reader.readC();
        ins->fm.alg = (reader.readC() == 0) ? 1 : 0;

        ins->fm.op[1].ksl = reader.readC();
        ins->fm.op[1].mult = reader.readC();
        reader.readC(); // skip
        ins->fm.op[1].ar = reader.readC();
        ins->fm.op[1].sl = reader.readC();
        ins->fm.op[1].sus = (reader.readC() != 0) ? 1 : 0;
        ins->fm.op[1].dr = reader.readC();
        ins->fm.op[1].rr = reader.readC();
        ins->fm.op[1].tl = reader.readC();
        ins->fm.op[1].am = reader.readC();
        ins->fm.op[1].vib = reader.readC();
        ins->fm.op[1].ksr = reader.readC();
        reader.readC(); // skip

        ins->fm.op[0].ws = reader.readC();
        ins->fm.op[1].ws = reader.readC();
        ins->name = instNames[i]->length() > 0 ? (*instNames[i]) : fmt::sprintf("%s[%d]", stripPath, i);

        insList.push_back(ins);
        ++readCount;
      }
      reader.seek(0, SEEK_END);

    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file");
      for (int i = readCount - 1; i >= 0; --i) {
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

  for (auto& name : instNames) {
    delete name;
  }
}

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
        // detune needs extra translation from register to furnace format
        const int dtNative = (buf >> 4) & 0x7;
        ins->fm.op[j].dt = (dtNative >= 4) ? (7 - dtNative) : (dtNative + 3);
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
    for (int i = readCount - 1; i >= 0; --i) {
      delete insList[i];
    }
    return;
  }

  for (unsigned int i = 0; i < insCount; ++i) {
    ret.push_back(insList[i]);
  }
}

void DivEngine::loadGYB(SafeReader& reader, std::vector<DivInstrument*>& ret, String& stripPath) {
  std::vector<DivInstrument*> insList;

  int readCount = 0;
  bool is_failed = false;
  auto readInstrument = [](SafeReader& reader, bool readRegB4) -> DivInstrument* {
    const int opOrder[] = {0,2,1,3};
    DivInstrument* ins = new DivInstrument;
    ins->type = DIV_INS_FM;
    ins->fm.ops = 4;

    // TODO see https://plutiedev.com/ym2612-registers 
    // and https://github.com/Wohlstand/OPN2BankEditor/blob/master/Specifications/GYB-file-specification.txt

    uint8_t tmp;
    
    for (int i : opOrder) {
      tmp = reader.readC(); // MUL/DT
      ins->fm.op[i].mult = tmp & 0xF;
      ins->fm.op[i].dt = ((tmp>>4) & 0x7);
    }
    for (int i : opOrder) {
      tmp = reader.readC(); // TL
      ins->fm.op[i].tl = tmp & 0x7F;
    }
    for (int i : opOrder) {
      tmp = reader.readC(); // AR/RS
      ins->fm.op[i].ar = tmp & 0x1F;
      ins->fm.op[i].rs = ((tmp>>6) & 0x3);
    }
    for (int i : opOrder) {
      tmp = reader.readC(); // DR/AM-ENA
      ins->fm.op[i].dr = tmp & 0x1F;
      ins->fm.op[i].am = ((tmp>>7) & 0x1);
    }
    for (int i : opOrder) {
      tmp = reader.readC(); // SR (D2R)
      ins->fm.op[i].d2r = tmp & 0x1F;
    }
    for (int i : opOrder) {
      tmp = reader.readC(); // RR/SL
      ins->fm.op[i].rr = tmp & 0xF;
      ins->fm.op[i].sl = ((tmp>>4) & 0xF);
    }
    for (int i : opOrder) {
      tmp = reader.readC(); // SSG-EG
      ins->fm.op[i].ssgEnv = tmp & 0xF;
    }
    // ALG/FB
    tmp = reader.readC();
    ins->fm.alg = tmp & 0x3;
    ins->fm.fb = ((tmp>>3) & 0x3);

    if (!readRegB4) { // PAN
      reader.readC(); // skip as not kept in Furnace instrument
    }
    return ins;
  };

  try {
    reader.seek(0, SEEK_SET);
    uint16_t header = reader.readS();
    uint8_t insMelodyCount, insDrumCount;

    if (header == 0x0C1A) { // 26 12 in decimal bytes
      uint8_t version = reader.readC();
      if ((version^3)>0) { // GYBv1/2
        insMelodyCount = reader.readC();
        insDrumCount = reader.readC();

        if (!reader.seek(0x100, SEEK_CUR)) { // skip MIDI instrument mapping
          throw EndOfFileException(&reader, reader.tell() + 0x100);
        }

        if (version == 2) {
          reader.readC(); // skip LFO speed (chip-global)
        }

        // Instrument data
        for (int i = 0; i < (insMelodyCount+insDrumCount); ++i) {
          // Note: melody and drum patches are interleaved...
          DivInstrument* newIns = readInstrument(reader, (version==2));
          reader.readC();  // skip transpose
          if (version == 2) {
            reader.readC();  // skip padding
          }
          insList.push_back(newIns);
          ++readCount;
        }

        // Instrument name
        for (int i = 0; i < (insMelodyCount+insDrumCount); ++i) {
          uint8_t nameLen = reader.readC();
          String insName = (nameLen > 0) ? reader.readString(nameLen) : fmt::sprintf("%s [%d]", stripPath, readCount);

          // TODO determine if a patch is 'empty' -> ignore and move on
          if (insList[i] == NULL) {
            continue;
          }
          insList[i]->name = insName;
        }
      } else { // GYBv3+
        reader.readC();  // skip LFO speed (chip-global)
        uint32_t fileSize = reader.readI();
        uint32_t bankOffset = reader.readI();
        uint32_t mapOffset = reader.readI();

        if (reader.size() != fileSize || bankOffset > fileSize || mapOffset > fileSize) {
          lastError = "GYBv3 file appears to have invalid data offsets.";
          logE("GYBv3 file appears to have invalid data offsets.");
          return;
        }

        if (!reader.seek(bankOffset, SEEK_SET)) {
          throw EndOfFileException(&reader, bankOffset);
        }
        uint16_t insCount = reader.readS();

        for (int i = 0; i < insCount; ++i) {
          DivInstrument* newIns = readInstrument(reader, true);
          reader.readC(); // skip transpose
          uint8_t additionalDataFlags = reader.readC() & 0x1; // skip additional data bitfield
          // TODO if chord notes attached, skip this
          if ((additionalDataFlags&1) > 0) {
            uint8_t notes = reader.readC();
            for (int j = 0; j < notes; ++j) {
              reader.readC();
            }
          }
          uint8_t nameLen = reader.readC();
          String insName = (nameLen > 0) ? reader.readString(nameLen) : fmt::sprintf("%s [%d]", stripPath, readCount++);
          // TODO determine if a patch is 'empty' -> ignore and move on
          newIns->name = insName;
          insList.push_back(newIns);
        }

        reader.seek(0, SEEK_END);
      }
    }
    
  } catch (EndOfFileException& e) {
    lastError = "premature end of file";
    logE("premature end of file");
    for (int i = readCount - 1; i >= 0; --i) {
      delete insList[i];
    }
    is_failed = true;
  }

  if (!is_failed) {
    for (int i = 0; i < readCount; ++i) {
      if (insList[i] != NULL) {
        ret.push_back(insList[i]);
      }
    }
  }
}
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
  auto readIntStrWithinRange = [](String&& input, int limitLow, int limitHigh) -> int {
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
    op.dt = readIntStrWithinRange(reader.readStringToken(), 0, 7);
    op.dt = (op.dt >= 4) ? (7 - op.dt) : (op.dt + 3);
    op.dt2 = readIntStrWithinRange(reader.readStringToken(), 0, 3);
    op.am = readIntStrWithinRange(reader.readStringToken(), 0, 1);
  };

  try {
    reader.seek(0, SEEK_SET);
    while (!reader.isEOF()) {
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
        newPatch->type = DIV_INS_FM;
        newPatch->fm.ops = 4;
      }

      // Read each line for their respective params. They may not be written in the same LINE order but they 
      // must absolutely be properly grouped per patch! Line prefixes must be separated by a space! (see inline comments)
      
      if (token.size() >= 2) {
        if (token[0] == '@') {
          // @:123 Name of patch
          // Note: Fallback to bank filename and current patch number in _file_ order (not @n order)
          newPatch->name = reader.readStringLine();
          newPatch->name = newPatch->name.size() > 0 ? newPatch->name : fmt::sprintf("%s[%d]", stripPath, readCount);
          patchNameRead = true;

        } else if (token.compare(0,3,"CH:") == 0) {
          // CH: PAN FL CON AMS PMS SLOT NE
          reader.readStringToken(); // skip PAN
          newPatch->fm.fb = readIntStrWithinRange(reader.readStringToken(), 0, 7);
          newPatch->fm.alg = readIntStrWithinRange(reader.readStringToken(), 0, 7);
          newPatch->fm.ams = readIntStrWithinRange(reader.readStringToken(), 0, 4);
          newPatch->fm.fms = readIntStrWithinRange(reader.readStringToken(), 0, 7);
          reader.readStringToken(); // skip SLOT
          reader.readStringToken(); // skip NE
          characteristicRead = true;

        } else if (token.compare(0,3,"C1:") == 0) {
          // C1: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          readOpmOperator(reader, newPatch->fm.op[2]);
          c1Read = true;

        } else if (token.compare(0,3,"C2:") == 0) {
          // C2: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          readOpmOperator(reader, newPatch->fm.op[3]);
          c2Read = true;

        } else if (token.compare(0,3,"M1:") == 0) {
          // M1: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          readOpmOperator(reader, newPatch->fm.op[0]);
          m1Read = true;

        } else if (token.compare(0,3,"M2:") == 0) {
          // M2: AR D1R D2R RR D1L TL KS MUL DT1 DT2 AMS-EN
          readOpmOperator(reader, newPatch->fm.op[1]);
          m2Read = true;

        } else if (token.compare(0,4,"LFO:") == 0) {
          // LFO: LFRQ AMD PMD WF NFRQ
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

std::vector<DivInstrument*> DivEngine::instrumentFromFile(const char* path) {
  std::vector<DivInstrument*> ret;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    lastError=strerror(errno);
    return ret;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  len=ftell(f);
  if (len<0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  if (len==(SIZE_MAX>>1)) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  if (len==0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return ret;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire instrument file buffer!");
    lastError="did not read entire instrument file!";
    delete[] buf;
    return ret;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceInstr=false;
  try {
    reader.read(magic,16);
    if (memcmp("-Furnace instr.-",magic,16)==0) {
      isFurnaceInstr=true;
    }
  } catch (EndOfFileException& e) {
    reader.seek(0,SEEK_SET);
  }

  if (isFurnaceInstr) {
    DivInstrument* ins=new DivInstrument;
    try {
      short version=reader.readS();
      reader.readS(); // reserved

      if (version>DIV_ENGINE_VERSION) {
        warnings="this instrument is made with a more recent version of Furnace!";
      }

      unsigned int dataPtr=reader.readI();
      reader.seek(dataPtr,SEEK_SET);

      if (ins->readInsData(reader,version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        delete ins;
        delete[] buf;
        return ret;
      } else {
        ret.push_back(ins);
      }
    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file");
      delete ins;
      delete[] buf;
      return ret;
    }
  } else { // read as a different format
    const char* ext=strrchr(path,'.');
    DivInsFormats format=DIV_INSFORMAT_DMP;
    if (ext!=NULL) {
      String extS;
      for (; *ext; ext++) {
        char i=*ext;
        if (i>='A' && i<='Z') {
          i+='a'-'A';
        }
        extS+=i;
      }
      if (extS==".dmp") {
        format=DIV_INSFORMAT_DMP;
      } else if (extS==".tfi") {
        format=DIV_INSFORMAT_TFI;
      } else if (extS==".vgi") {
        format=DIV_INSFORMAT_VGI;
      } else if (extS==".fti") {
        format=DIV_INSFORMAT_FTI;
      } else if (extS==".bti") {
        format=DIV_INSFORMAT_BTI;
      } else if (extS==".s3i") {
        format=DIV_INSFORMAT_S3I;
      } else if (extS==".sbi") {
        format=DIV_INSFORMAT_SBI;
      } else if (extS==".opli") {
        format=DIV_INSFORMAT_OPLI;
      } else if (extS==".opni") {
        format=DIV_INSFORMAT_OPNI;
      } else if (extS==".y12") {
        format=DIV_INSFORMAT_Y12;
      } else if (extS==".bnk") {
        format=DIV_INSFORMAT_BNK;
      } else if (extS==".gyb") {
        format=DIV_INSFORMAT_GYB;
      } else if (extS==".opm") {
        format=DIV_INSFORMAT_OPM;
      } else if (extS==".ff") {
        format=DIV_INSFORMAT_FF;
      }
    }

    switch (format) {
      case DIV_INSFORMAT_DMP: loadDMP(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_TFI: loadTFI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_VGI: loadVGI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_FTI: // TODO
        break;
      case DIV_INSFORMAT_BTI: // TODO
        break;
      case DIV_INSFORMAT_S3I: loadS3I(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_SBI: loadSBI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_OPLI: loadOPLI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_OPNI: loadOPNI(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_Y12: loadY12(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_BNK: loadBNK(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_FF: loadFF(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_GYB: loadGYB(reader,ret,stripPath);
        break;
      case DIV_INSFORMAT_OPM: loadOPM(reader,ret,stripPath);
        break;
    }

    if (reader.tell()<reader.size()) {
      addWarning("https://github.com/tildearrow/furnace/issues/84");
      addWarning("there is more data at the end of the file! what happened here!");
      addWarning(fmt::sprintf("exactly %d bytes, if you are curious",reader.size()-reader.tell()));
    }
  }

  delete[] buf;
  return ret;
}
