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
  DIV_INSFORMAT_OPM
};

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
    logW("did not read entire instrument file buffer!\n");
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

  DivInstrument* ins=new DivInstrument;
  if (isFurnaceInstr) {
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
      }
    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file!\n");
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
      if (extS==String(".dmp")) {
        format=DIV_INSFORMAT_DMP;
      } else if (extS==String(".tfi")) {
        format=DIV_INSFORMAT_TFI;
      } else if (extS==String(".vgi")) {
        format=DIV_INSFORMAT_VGI;
      } else if (extS==String(".fti")) {
        format=DIV_INSFORMAT_FTI;
      } else if (extS==String(".bti")) {
        format=DIV_INSFORMAT_BTI;
      } else if (extS==String(".s3i")) {
        format=DIV_INSFORMAT_S3I;
      } else if (extS==String(".sbi")) {
        format=DIV_INSFORMAT_SBI;
      } else if (extS==String(".opm")) {
        format=DIV_INSFORMAT_OPM;
      }
    }

    // TDOO these really should be re-organized to separate functions per instrument file type.
    switch (format) {
      case DIV_INSFORMAT_DMP: {
        // this is a ridiculous mess
        unsigned char version=0;
        unsigned char sys=0;
        try {
          reader.seek(0,SEEK_SET);
          version=reader.readC();
          logD(".dmp version %d\n",version);
        } catch (EndOfFileException& e) {
          lastError="premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return ret;
        }

        if (version>11) {
          lastError="unknown instrument version!";
          delete ins;
          delete[] buf;
          return ret;
        }

        ins->name=stripPath;

        if (version>=11) { // 1.0
          try {
            sys=reader.readC();
      
            switch (sys) {
              case 1: // YMU759
                ins->type=DIV_INS_FM;
                logD("instrument type is YMU759\n");
                break;
              case 2: // Genesis
                ins->type=DIV_INS_FM;
                logD("instrument type is Genesis\n");
                break;
              case 3: // SMS
                ins->type=DIV_INS_STD;
                logD("instrument type is SMS\n");
                break;
              case 4: // Game Boy
                ins->type=DIV_INS_GB;
                logD("instrument type is Game Boy\n");
                break;
              case 5: // PC Engine
                ins->type=DIV_INS_PCE;
                logD("instrument type is PC Engine\n");
                break;
              case 6: // NES
                ins->type=DIV_INS_STD;
                logD("instrument type is NES\n");
                break;
              case 7: case 0x17: // C64
                ins->type=DIV_INS_C64;
                logD("instrument type is C64\n");
                break;
              case 8: // Arcade
                ins->type=DIV_INS_FM;
                logD("instrument type is Arcade\n");
                break;
              default:
                logD("instrument type is unknown\n");
                lastError="unknown instrument type!";
                delete ins;
                delete[] buf;
                return ret;
                break;
            }
          } catch (EndOfFileException& e) {
            lastError="premature end of file";
            logE("premature end of file!\n");
            delete ins;
            delete[] buf;
            return ret;
          }
        }

        try {
          bool mode=true;
          if (version>1) {
            mode=reader.readC();
            logD("instrument mode is %d\n",mode);
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
            logD("reading FM data...\n");
            if (version<10) {
              if (version>1) {
                ins->fm.ops=reader.readC()?4:2;
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
              logD("OP%d is at %d\n",j,reader.tell());
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
            logD("reading STD data...\n");
            if (ins->type!=DIV_INS_GB) {
              ins->std.volMacroLen=reader.readC();
              if (version>5) {
                for (int i=0; i<ins->std.volMacroLen; i++) {
                  ins->std.volMacro[i]=reader.readI();
                }
              } else {
                for (int i=0; i<ins->std.volMacroLen; i++) {
                  ins->std.volMacro[i]=reader.readC();
                }
              }
              if (version<11) for (int i=0; i<ins->std.volMacroLen; i++) {
                if (ins->std.volMacro[i]>15 && ins->type==DIV_INS_STD) ins->type=DIV_INS_PCE;
              }
              if (ins->std.volMacroLen>0) {
                ins->std.volMacroOpen=true;
                ins->std.volMacroLoop=reader.readC();
              } else {
                ins->std.volMacroOpen=false;
              }
            }

            ins->std.arpMacroLen=reader.readC();
            if (version>5) {
              for (int i=0; i<ins->std.arpMacroLen; i++) {
                ins->std.arpMacro[i]=reader.readI();
              }
            } else {
              for (int i=0; i<ins->std.arpMacroLen; i++) {
                ins->std.arpMacro[i]=reader.readC();
              }
            }
            if (ins->std.arpMacroLen>0) {
              ins->std.arpMacroOpen=true;
              ins->std.arpMacroLoop=reader.readC();
            } else {
              ins->std.arpMacroOpen=false;
            }
            if (version>8) { // TODO: when?
              ins->std.arpMacroMode=reader.readC();
            }

            ins->std.dutyMacroLen=reader.readC();
            if (version>5) {
              for (int i=0; i<ins->std.dutyMacroLen; i++) {
                ins->std.dutyMacro[i]=reader.readI();
              }
            } else {
              for (int i=0; i<ins->std.dutyMacroLen; i++) {
                ins->std.dutyMacro[i]=reader.readC();
              }
            }
            if (ins->std.dutyMacroLen>0) {
              ins->std.dutyMacroOpen=true;
              ins->std.dutyMacroLoop=reader.readC();
            } else {
              ins->std.dutyMacroOpen=false;
            }

            ins->std.waveMacroLen=reader.readC();
            if (version>5) {
              for (int i=0; i<ins->std.waveMacroLen; i++) {
                ins->std.waveMacro[i]=reader.readI();
              }
            } else {
              for (int i=0; i<ins->std.waveMacroLen; i++) {
                ins->std.waveMacro[i]=reader.readC();
              }
            }
            if (ins->std.waveMacroLen>0) {
              ins->std.waveMacroOpen=true;
              ins->std.waveMacroLoop=reader.readC();
            } else {
              ins->std.waveMacroOpen=false;
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
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return ret;
        }
        break;
      }
      case DIV_INSFORMAT_TFI:
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
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return ret;
        }
        break;
      case DIV_INSFORMAT_VGI:
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
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return ret;
        }
        break;
      case DIV_INSFORMAT_FTI: // TODO
        break;
      case DIV_INSFORMAT_BTI: // TODO
        break;
      case DIV_INSFORMAT_OPM: // TODO
        break;
      case DIV_INSFORMAT_S3I:
        try {
          reader.seek(0, SEEK_SET);

          uint8_t s3i_type = reader.readC();

          if (s3i_type >= 2) {
            ins->type = DIV_INS_OPL;
            // skip internal filename - we'll use the long name description
            reader.seek(12, SEEK_CUR);

            // skip reserved bytes
            reader.seek(3, SEEK_CUR);

            // 12-byte opl value
            uint8_t s3i_Mcharacteristics = reader.readC();
            uint8_t s3i_Ccharacteristics = reader.readC();
            uint8_t s3i_Mscaling_output = reader.readC();
            uint8_t s3i_Cscaling_output = reader.readC();
            uint8_t s3i_Meg_AD = reader.readC();
            uint8_t s3i_Ceg_AD = reader.readC();
            uint8_t s3i_Meg_SR = reader.readC();
            uint8_t s3i_Ceg_SR = reader.readC();
            uint8_t s3i_Mwave = reader.readC();
            uint8_t s3i_Cwave = reader.readC();
            uint8_t s3i_FeedConnect = reader.readC();
            
            DivInstrumentFM::Operator& opM = ins->fm.op[0];
            DivInstrumentFM::Operator& opC = ins->fm.op[1];
            ins->fm.ops = 2;
            opM.mult = s3i_Mcharacteristics & 0xF;
            opM.ksr = ((s3i_Mcharacteristics >> 4) & 0x1);
            opM.sus = ((s3i_Mcharacteristics >> 5) & 0x1);
            opM.vib = ((s3i_Mcharacteristics >> 6) & 0x1);
            opM.am = ((s3i_Mcharacteristics >> 7) & 0x1);
            opM.tl = s3i_Mscaling_output & 0x3F;
            opM.ksl = ((s3i_Mscaling_output >> 6) & 0x3);
            opM.ar = ((s3i_Meg_AD >> 4) & 0xF);
            opM.dr = (s3i_Meg_AD & 0xF);
            opM.rr = (s3i_Meg_SR & 0xF);
            opM.sl = ((s3i_Meg_SR >> 4) & 0xF);
            opM.ws = s3i_Mwave;

            ins->fm.alg = (s3i_FeedConnect & 0x1);
            ins->fm.fb = ((s3i_FeedConnect >> 1) & 0x7);

            opC.mult = s3i_Ccharacteristics & 0xF;
            opC.ksr = ((s3i_Ccharacteristics >> 4) & 0x1);
            opC.sus = ((s3i_Ccharacteristics >> 5) & 0x1);
            opC.vib = ((s3i_Ccharacteristics >> 6) & 0x1);
            opC.am = ((s3i_Ccharacteristics >> 7) & 0x1);
            opC.tl = s3i_Cscaling_output & 0x3F;
            opC.ksl = ((s3i_Cscaling_output >> 6) & 0x3);
            opC.ar = ((s3i_Ceg_AD >> 4) & 0xF);
            opC.dr = (s3i_Ceg_AD & 0xF);
            opC.rr = (s3i_Ceg_SR & 0xF);
            opC.sl = ((s3i_Ceg_SR >> 4) & 0xF);
            opC.ws = s3i_Cwave;

            // Skip more stuff we don't need
            reader.seek(21, SEEK_CUR);
          } else {
            logE("S3I PCM samples currently not supported.");
          }
          ins->name = reader.readString(28);
          int s3i_signature = reader.readI();

          if (s3i_signature != 0x49524353) {
            logW("S3I signature invalid.");
          };
        }
        catch (EndOfFileException& e) {
          lastError = "premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return ret;
        }
        break;
      case DIV_INSFORMAT_SBI:
        try {
          reader.seek(0, SEEK_SET);
          ins->type = DIV_INS_OPL;

          int sbi_header = reader.readI();
          // SBI header determines format
          bool is_2op = (sbi_header == 0x1A494253); // SBI\x1A
          bool is_4op = (sbi_header == 0x1A504F34); // 4OP\x1A
          bool is_6op = (sbi_header == 0x1A504F36); // 6OP\x1A - Freq Monster 801-specific

          // 32-byte null terminated instrument name
          ins->name = reader.readString(32);

          // 2op SBI
          uint8_t sbi_Mcharacteristics = reader.readC();
          uint8_t sbi_Ccharacteristics = reader.readC();
          uint8_t sbi_Mscaling_output = reader.readC();
          uint8_t sbi_Cscaling_output = reader.readC();
          uint8_t sbi_Meg_AD = reader.readC();
          uint8_t sbi_Ceg_AD = reader.readC();
          uint8_t sbi_Meg_SR = reader.readC();
          uint8_t sbi_Ceg_SR = reader.readC();
          uint8_t sbi_Mwave = reader.readC();
          uint8_t sbi_Cwave = reader.readC();
          uint8_t sbi_FeedConnect = reader.readC();

          // 4op SBI
          uint8_t sbi_M4characteristics;
          uint8_t sbi_C4characteristics;
          uint8_t sbi_M4scaling_output;
          uint8_t sbi_C4scaling_output;
          uint8_t sbi_M4eg_AD;
          uint8_t sbi_C4eg_AD;
          uint8_t sbi_M4eg_SR;
          uint8_t sbi_C4eg_SR;
          uint8_t sbi_M4wave;
          uint8_t sbi_C4wave;
          uint8_t sbi_4opConnect;
          
          if (is_2op) {
            DivInstrumentFM::Operator& opM = ins->fm.op[0];
            DivInstrumentFM::Operator& opC = ins->fm.op[1];
            ins->fm.ops = 2;
            opM.mult = sbi_Mcharacteristics & 0xF;
            opM.ksr = ((sbi_Mcharacteristics >> 4) & 0x1);
            opM.sus = ((sbi_Mcharacteristics >> 5) & 0x1);
            opM.vib = ((sbi_Mcharacteristics >> 6) & 0x1);
            opM.am = ((sbi_Mcharacteristics >> 7) & 0x1);
            opM.tl = sbi_Mscaling_output & 0x3F;
            opM.ksl = ((sbi_Mscaling_output >> 6) & 0x3);
            opM.ar = ((sbi_Meg_AD >> 4) & 0xF);
            opM.dr = (sbi_Meg_AD & 0xF);
            opM.rr = (sbi_Meg_SR & 0xF);
            opM.sl = ((sbi_Meg_SR >> 4) & 0xF);
            opM.ws = sbi_Mwave;

            ins->fm.alg = (sbi_FeedConnect & 0x1);
            ins->fm.fb = ((sbi_FeedConnect >> 1) & 0x7);

            opC.mult = sbi_Ccharacteristics & 0xF;
            opC.ksr = ((sbi_Ccharacteristics >> 4) & 0x1);
            opC.sus = ((sbi_Ccharacteristics >> 5) & 0x1);
            opC.vib = ((sbi_Ccharacteristics >> 6) & 0x1);
            opC.am = ((sbi_Ccharacteristics >> 7) & 0x1);
            opC.tl = sbi_Cscaling_output & 0x3F;
            opC.ksl = ((sbi_Cscaling_output >> 6) & 0x3);
            opC.ar = ((sbi_Ceg_AD >> 4) & 0xF);
            opC.dr = (sbi_Ceg_AD & 0xF);
            opC.rr = (sbi_Ceg_SR & 0xF);
            opC.sl = ((sbi_Ceg_SR >> 4) & 0xF);
            opC.ws = sbi_Cwave;

            // Ignore rest of file - rest is 'reserved padding'.
            reader.seek(0, SEEK_END);
          }

          if (is_4op || is_6op) {
            // Operator placement is different so need to place in correct registers.
            // Note: 6op is an unofficial extension of 4op SBIs by Darron Broad (Freq Monster 801).
            // We'll only use the 4op portion here for pure OPL3.
            DivInstrumentFM::Operator& opM = ins->fm.op[0];
            DivInstrumentFM::Operator& opC = ins->fm.op[2];
            DivInstrumentFM::Operator& opM4 = ins->fm.op[1];
            DivInstrumentFM::Operator& opC4 = ins->fm.op[3];
            ins->fm.ops = 4;

            sbi_M4characteristics = reader.readC();
            sbi_C4characteristics = reader.readC();
            sbi_M4scaling_output = reader.readC();
            sbi_C4scaling_output = reader.readC();
            sbi_M4eg_AD = reader.readC();
            sbi_C4eg_AD = reader.readC();
            sbi_M4eg_SR = reader.readC();
            sbi_C4eg_SR = reader.readC();
            sbi_M4wave = reader.readC();
            sbi_C4wave = reader.readC();
            sbi_4opConnect = reader.readC();
            
            ins->fm.alg = (sbi_FeedConnect & 0x1) | ((sbi_4opConnect & 0x1) << 1);
            ins->fm.fb = ((sbi_FeedConnect >> 1) & 0x7);

            opM.mult = sbi_Mcharacteristics & 0xF;
            opM.ksr = ((sbi_Mcharacteristics >> 4) & 0x1);
            opM.sus = ((sbi_Mcharacteristics >> 5) & 0x1);
            opM.vib = ((sbi_Mcharacteristics >> 6) & 0x1);
            opM.am = ((sbi_Mcharacteristics >> 7) & 0x1);
            opM.tl = sbi_Mscaling_output & 0x3F;
            opM.ksl = ((sbi_Mscaling_output >> 6) & 0x3);
            opM.ar = ((sbi_Meg_AD >> 4) & 0xF);
            opM.dr = (sbi_Meg_AD & 0xF);
            opM.rr = (sbi_Meg_SR & 0xF);
            opM.sl = ((sbi_Meg_SR >> 4) & 0xF);
            opM.ws = sbi_Mwave;

            opC.mult = sbi_Ccharacteristics & 0xF;
            opC.ksr = ((sbi_Ccharacteristics >> 4) & 0x1);
            opC.sus = ((sbi_Ccharacteristics >> 5) & 0x1);
            opC.vib = ((sbi_Ccharacteristics >> 6) & 0x1);
            opC.am = ((sbi_Ccharacteristics >> 7) & 0x1);
            opC.tl = sbi_Cscaling_output & 0x3F;
            opC.ksl = ((sbi_Cscaling_output >> 6) & 0x3);
            opC.ar = ((sbi_Ceg_AD >> 4) & 0xF);
            opC.dr = (sbi_Ceg_AD & 0xF);
            opC.rr = (sbi_Ceg_SR & 0xF);
            opC.sl = ((sbi_Ceg_SR >> 4) & 0xF);
            opC.ws = sbi_Cwave;
            
            opM4.mult = sbi_M4characteristics & 0xF;
            opM4.ksr = ((sbi_M4characteristics >> 4) & 0x1);
            opM4.sus = ((sbi_M4characteristics >> 5) & 0x1);
            opM4.vib = ((sbi_M4characteristics >> 6) & 0x1);
            opM4.am = ((sbi_M4characteristics >> 7) & 0x1);
            opM4.tl = sbi_M4scaling_output & 0x3F;
            opM4.ksl = ((sbi_M4scaling_output >> 6) & 0x3);
            opM4.ar = ((sbi_M4eg_AD >> 4) & 0xF);
            opM4.dr = (sbi_M4eg_AD & 0xF);
            opM4.rr = (sbi_M4eg_SR & 0xF);
            opM4.sl = ((sbi_M4eg_SR >> 4) & 0xF);
            opM4.ws = sbi_M4wave;

            opC4.mult = sbi_C4characteristics & 0xF;
            opC4.ksr = ((sbi_C4characteristics >> 4) & 0x1);
            opC4.sus = ((sbi_C4characteristics >> 5) & 0x1);
            opC4.vib = ((sbi_C4characteristics >> 6) & 0x1);
            opC4.am = ((sbi_C4characteristics >> 7) & 0x1);
            opC4.tl = sbi_C4scaling_output & 0x3F;
            opC4.ksl = ((sbi_C4scaling_output >> 6) & 0x3);
            opC4.ar = ((sbi_C4eg_AD >> 4) & 0xF);
            opC4.dr = (sbi_C4eg_AD & 0xF);
            opC4.rr = (sbi_C4eg_SR & 0xF);
            opC4.sl = ((sbi_C4eg_SR >> 4) & 0xF);
            opC4.ws = sbi_C4wave;

            // Ignore rest of file once we've read in all we need.
            // Note: Freq Monster 801 adds a ton of other additional fields irrelevant to chip registers.
            reader.seek(0, SEEK_END);
          }

        } catch (EndOfFileException& e) {
          lastError = "premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return ret;
        }
        break;
    }

    if (reader.tell()<reader.size()) {
      addWarning("https://github.com/tildearrow/furnace/issues/84");
      addWarning("there is more data at the end of the file! what happened here!");
      addWarning(fmt::sprintf("exactly %d bytes, if you are curious",reader.size()-reader.tell()));
    }
  }

  ret.push_back(ins);
  return ret;
}