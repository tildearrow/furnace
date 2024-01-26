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

#include "importExport.h"

class DivEngine;

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
          ins->type=DIV_INS_OPM;
          logD("instrument type is Arcade");
          break;
        case 9: // Neo Geo
          ins->type=DIV_INS_FM;
          logD("instrument type is Neo Geo");
          break;
        default:
          logD("instrument type is unknown");
          lastError=fmt::sprintf("unknown instrument type %d!",sys);
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
        if (ins->type==DIV_INS_FM) {
          if (sys==9) {
            ins->type=DIV_INS_AY;
          } else {
            ins->type=DIV_INS_STD;
          }
        }
      } else {
        if (sys==3 || sys==6) {
          ins->type=DIV_INS_OPLL;
        } else if (sys==1) {
          ins->type=DIV_INS_OPL;
        } else if (sys==8) {
          ins->type=DIV_INS_OPM;
        } else {
          ins->type=DIV_INS_FM;
        }
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
          // HELP
          if (reader.size()==49) {
            ins->fm.ops=4;
            reader.readC();
          } else {
            ins->fm.ops=reader.readC()?2:4;
          }
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
          if (sys==3 || sys==6) { // OPLL/VRC7
            ins->fm.op[j].ksr=reader.readC()?1:0;
            ins->fm.op[j].vib=reader.readC();
            if (j==0) {
              ins->fm.opllPreset=ins->fm.op[j].vib>>4;
            }
            ins->fm.op[j].vib=ins->fm.op[j].vib?1:0;
            ins->fm.op[j].ksl=reader.readC()?1:0;
            ins->fm.op[j].ssgEnv=reader.readC();
          } else {
            ins->fm.op[j].rs=reader.readC();
            ins->fm.op[j].dt=reader.readC();
            ins->fm.op[j].dt2=ins->fm.op[j].dt>>4;
            ins->fm.op[j].dt&=15;
            ins->fm.op[j].d2r=reader.readC();
            ins->fm.op[j].ssgEnv=reader.readC();
          }
        }
      }
    } else { // STD
      logD("reading STD data...");
      if (ins->type!=DIV_INS_GB) {
        ins->std.get_macro(DIV_MACRO_VOL, true)->len=reader.readC();
        if (version>5) {
          for (int i=0; i<ins->std.get_macro(DIV_MACRO_VOL, true)->len; i++) {
            ins->std.get_macro(DIV_MACRO_VOL, true)->val[i]=reader.readI();
            if (ins->std.get_macro(DIV_MACRO_VOL, true)->val[i]>15 && sys==6) { // FDS
              ins->type=DIV_INS_FDS;
            }
          }
        } else {
          for (int i=0; i<ins->std.get_macro(DIV_MACRO_VOL, true)->len; i++) {
            ins->std.get_macro(DIV_MACRO_VOL, true)->val[i]=reader.readC();
          }
        }
        if (version<11) for (int i=0; i<ins->std.get_macro(DIV_MACRO_VOL, true)->len; i++) {
          if (ins->std.get_macro(DIV_MACRO_VOL, true)->val[i]>15 && ins->type==DIV_INS_STD) ins->type=DIV_INS_PCE;
        }
        if (ins->std.get_macro(DIV_MACRO_VOL, true)->len>0) {
          ins->std.get_macro(DIV_MACRO_VOL, true)->open=true;
          ins->std.get_macro(DIV_MACRO_VOL, true)->loop=reader.readC();
        } else {
          ins->std.get_macro(DIV_MACRO_VOL, true)->open=false;
        }
      }

      ins->std.get_macro(DIV_MACRO_ARP, true)->len=reader.readC();
      if (version>5) {
        for (int i=0; i<ins->std.get_macro(DIV_MACRO_ARP, true)->len; i++) {
          ins->std.get_macro(DIV_MACRO_ARP, true)->val[i]=reader.readI();
        }
      } else {
        for (int i=0; i<ins->std.get_macro(DIV_MACRO_ARP, true)->len; i++) {
          ins->std.get_macro(DIV_MACRO_ARP, true)->val[i]=reader.readC();
        }
      }
      if (ins->std.get_macro(DIV_MACRO_ARP, true)->len>0) {
        ins->std.get_macro(DIV_MACRO_ARP, true)->open=true;
        ins->std.get_macro(DIV_MACRO_ARP, true)->loop=reader.readC();
      } else {
        ins->std.get_macro(DIV_MACRO_ARP, true)->open=false;
      }
      if (version>8) { // TODO: when?
        ins->std.get_macro(DIV_MACRO_ARP, true)->mode=reader.readC();
      }

      ins->std.get_macro(DIV_MACRO_DUTY, true)->len=reader.readC();
      if (version>5) {
        for (int i=0; i<ins->std.get_macro(DIV_MACRO_DUTY, true)->len; i++) {
          ins->std.get_macro(DIV_MACRO_DUTY, true)->val[i]=reader.readI();
        }
      } else {
        for (int i=0; i<ins->std.get_macro(DIV_MACRO_DUTY, true)->len; i++) {
          ins->std.get_macro(DIV_MACRO_DUTY, true)->val[i]=reader.readC();
        }
      }
      if (ins->std.get_macro(DIV_MACRO_DUTY, true)->len>0) {
        ins->std.get_macro(DIV_MACRO_DUTY, true)->open=true;
        ins->std.get_macro(DIV_MACRO_DUTY, true)->loop=reader.readC();
      } else {
        ins->std.get_macro(DIV_MACRO_DUTY, true)->open=false;
      }

      ins->std.get_macro(DIV_MACRO_WAVE, true)->len=reader.readC();
      if (version>5) {
        for (int i=0; i<ins->std.get_macro(DIV_MACRO_WAVE, true)->len; i++) {
          ins->std.get_macro(DIV_MACRO_WAVE, true)->val[i]=reader.readI();
        }
      } else {
        for (int i=0; i<ins->std.get_macro(DIV_MACRO_WAVE, true)->len; i++) {
          ins->std.get_macro(DIV_MACRO_WAVE, true)->val[i]=reader.readC();
        }
      }
      if (ins->std.get_macro(DIV_MACRO_WAVE, true)->len>0) {
        ins->std.get_macro(DIV_MACRO_WAVE, true)->open=true;
        ins->std.get_macro(DIV_MACRO_WAVE, true)->loop=reader.readC();
      } else {
        ins->std.get_macro(DIV_MACRO_WAVE, true)->open=false;
      }

      if (ins->type==DIV_INS_C64) {
        bool volIsCutoff=false;

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
          volIsCutoff=reader.readI();
        } else {
          volIsCutoff=reader.readC();
        }
        ins->c64.initFilter=reader.readC();

        ins->c64.res=reader.readC();
        ins->c64.cut=(reader.readC()*2047)/100;
        ins->c64.hp=reader.readC();
        ins->c64.bp=reader.readC();
        ins->c64.lp=reader.readC();
        ins->c64.ch3off=reader.readC();

        // weird storage
        if (volIsCutoff) {
          // move to alg (new cutoff)
          ins->std.get_macro(DIV_MACRO_ALG, true)->len=ins->std.get_macro(DIV_MACRO_VOL, true)->len;
          ins->std.get_macro(DIV_MACRO_ALG, true)->loop=ins->std.get_macro(DIV_MACRO_VOL, true)->loop;
          ins->std.get_macro(DIV_MACRO_ALG, true)->rel=ins->std.get_macro(DIV_MACRO_VOL, true)->rel;
          for (int j=0; j<ins->std.get_macro(DIV_MACRO_ALG, true)->len; j++) {
            ins->std.get_macro(DIV_MACRO_ALG, true)->val[j]=-(ins->std.get_macro(DIV_MACRO_VOL, true)->val[j]-18);
          }
          ins->std.get_macro(DIV_MACRO_VOL, true)->len=0;
          memset(ins->std.get_macro(DIV_MACRO_VOL, true)->val,0,256*sizeof(int));
        }
        for (int j=0; j<ins->std.get_macro(DIV_MACRO_DUTY, true)->len; j++) {
          ins->std.get_macro(DIV_MACRO_DUTY, true)->val[j]-=12;
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