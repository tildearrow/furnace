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

#include "importExport.h"

class DivEngine;

static double samplePitches[11]={
  0.1666666666, 0.2, 0.25, 0.333333333, 0.5,
  1,
  2, 3, 4, 5, 6
};

bool DivEngine::loadDMF(unsigned char* file, size_t len) {
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;
    unsigned char historicColIns[DIV_MAX_CHANS];
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      historicColIns[i]=i;
    }

    ds.nullWave.len=32;
    for (int i=0; i<32; i++) {
      ds.nullWave.data[i]=15;
    }

    ds.isDMF=true;

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=(unsigned char)reader.readC();
    logI("module version %d (0x%.2x)",ds.version,ds.version);
    if (ds.version>0x1b) {
      logE("this version is not supported by Furnace yet!");
      lastError="this version is not supported by Furnace yet";
      delete[] file;
      return false;
    }
    unsigned char sys=0;
    ds.systemLen=1;
    if (ds.version<0x09) {
      // V E R S I O N  -> 3 <-
      // AWESOME
      ds.system[0]=DIV_SYSTEM_YMU759;
    } else {
      sys=reader.readC();
      ds.system[0]=systemFromFileDMF(sys);
    }
    if (ds.system[0]==DIV_SYSTEM_NULL) {
      logE("invalid system 0x%.2x!",sys);
      lastError="system not supported. running old version?";
      delete[] file;
      return false;
    }
    
    if (ds.system[0]==DIV_SYSTEM_YMU759 && ds.version<0x10) {
      ds.vendor=reader.readString((unsigned char)reader.readC());
      ds.carrier=reader.readString((unsigned char)reader.readC());
      ds.category=reader.readString((unsigned char)reader.readC());
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      ds.writer=reader.readString((unsigned char)reader.readC());
      ds.composer=reader.readString((unsigned char)reader.readC());
      ds.arranger=reader.readString((unsigned char)reader.readC());
      ds.copyright=reader.readString((unsigned char)reader.readC());
      ds.manGroup=reader.readString((unsigned char)reader.readC());
      ds.manInfo=reader.readString((unsigned char)reader.readC());
      ds.createdDate=reader.readString((unsigned char)reader.readC());
      ds.revisionDate=reader.readString((unsigned char)reader.readC());
      logI("%s by %s",ds.name.c_str(),ds.author.c_str());
      logI("has YMU-specific data:");
      logI("- carrier: %s",ds.carrier.c_str());
      logI("- category: %s",ds.category.c_str());
      logI("- vendor: %s",ds.vendor.c_str());
      logI("- writer: %s",ds.writer.c_str());
      logI("- composer: %s",ds.composer.c_str());
      logI("- arranger: %s",ds.arranger.c_str());
      logI("- copyright: %s",ds.copyright.c_str());
      logI("- management group: %s",ds.manGroup.c_str());
      logI("- management info: %s",ds.manInfo.c_str());
      logI("- created on: %s",ds.createdDate.c_str());
      logI("- revision date: %s",ds.revisionDate.c_str());
    } else {
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      logI("%s by %s",ds.name.c_str(),ds.author.c_str());
    }

    // compatibility flags
    if (!getConfInt("noDMFCompat",0)) {
      ds.limitSlides=true;
      ds.linearPitch=1;
      ds.loopModality=0;
      ds.properNoiseLayout=false;
      ds.waveDutyIsVol=false;
      // TODO: WHAT?! geodude.dmf fails when this is true
      // but isn't that how Defle behaves???
      ds.resetMacroOnPorta=false;
      ds.legacyVolumeSlides=true;
      ds.compatibleArpeggio=true;
      ds.noteOffResetsSlides=true;
      ds.targetResetsSlides=true;
      ds.arpNonPorta=false;
      ds.algMacroBehavior=false;
      ds.brokenShortcutSlides=false;
      ds.ignoreDuplicateSlides=true;
      ds.brokenDACMode=true;
      ds.oneTickCut=false;
      ds.newInsTriggersInPorta=true;
      ds.arp0Reset=true;
      ds.brokenSpeedSel=true;
      ds.noSlidesOnFirstTick=false;
      ds.rowResetsArpPos=false;
      ds.ignoreJumpAtEnd=true;
      ds.buggyPortaAfterSlide=true;
      ds.gbInsAffectsEnvelope=true;
      ds.ignoreDACModeOutsideIntendedChannel=false;
      ds.e1e2AlsoTakePriority=true;
      ds.fbPortaPause=true;
      ds.snDutyReset=true;
      ds.oldOctaveBoundary=false;
      ds.noOPN2Vol=true;
      ds.newVolumeScaling=false;
      ds.volMacroLinger=false;
      ds.brokenOutVol=true;
      ds.brokenOutVol2=true;
      ds.e1e2StopOnSameNote=true;
      ds.brokenPortaArp=false;
      ds.snNoLowPeriods=true;
      ds.disableSampleMacro=true;
      ds.preNoteNoEffect=true;
      ds.oldDPCM=true;
      ds.delayBehavior=0;
      ds.jumpTreatment=2;

      // 1.1 compat flags
      if (ds.version>24) {
        ds.waveDutyIsVol=true;
        ds.legacyVolumeSlides=false;
      }

      // Neo Geo detune is caused by Defle running Neo Geo at the wrong clock.
      /*
      if (ds.system[0]==DIV_SYSTEM_YM2610 || ds.system[0]==DIV_SYSTEM_YM2610_EXT
      || ds.system[0]==DIV_SYSTEM_YM2610_FULL || ds.system[0]==DIV_SYSTEM_YM2610_FULL_EXT
      || ds.system[0]==DIV_SYSTEM_YM2610B || ds.system[0]==DIV_SYSTEM_YM2610B_EXT) {
        ds.tuning=443.23;
      }
      */

      // Genesis detuned on Defle v10 and earlier
      /*if (ds.version<19 && ds.system[0]==DIV_SYSTEM_GENESIS) {
        ds.tuning=443.23;
      }*/
      // C64 detuned on Defle v11 and earlier
      /*if (ds.version<21 && (ds.system[0]==DIV_SYSTEM_C64_6581 || ds.system[0]==DIV_SYSTEM_C64_8580)) {
        ds.tuning=433.2;
      }*/

      // Game Boy arp+soundLen screwery
      if (ds.system[0]==DIV_SYSTEM_GB) {
        ds.systemFlags[0].set("enoughAlready",true);
      }
    }

    logI("reading module data...");
    if (ds.version>0x0c) {
      ds.subsong[0]->hilightA=reader.readC();
      ds.subsong[0]->hilightB=reader.readC();
    }

    bool customTempo=false;

    ds.subsong[0]->timeBase=reader.readC();
    ds.subsong[0]->speeds.len=2;
    ds.subsong[0]->speeds.val[0]=reader.readC();
    if (ds.version>0x07) {
      ds.subsong[0]->speeds.val[1]=reader.readC();
      bool pal=reader.readC();
      ds.subsong[0]->hz=pal?60:50;
      customTempo=reader.readC();
    } else {
      ds.subsong[0]->speeds.len=1;
    }
    if (ds.version>0x0a) {
      String hz=reader.readString(3);
      if (customTempo) {
        try {
          ds.subsong[0]->hz=std::stoi(hz);
        } catch (std::exception& e) {
          logW("invalid custom Hz!");
          ds.subsong[0]->hz=60;
        }
      }
    }
    if (ds.version>0x17) {
      ds.subsong[0]->patLen=reader.readI();
    } else {
      ds.subsong[0]->patLen=(unsigned char)reader.readC();
    }
    ds.subsong[0]->ordersLen=(unsigned char)reader.readC();

    if (ds.subsong[0]->patLen<0) {
      logE("pattern length is negative!");
      lastError="pattern lengrh is negative!";
      delete[] file;
      return false;
    }
    if (ds.subsong[0]->patLen>256) {
      logE("pattern length is too large!");
      lastError="pattern length is too large!";
      delete[] file;
      return false;
    }
    if (ds.subsong[0]->ordersLen<0) {
      logE("song length is negative!");
      lastError="song length is negative!";
      delete[] file;
      return false;
    }
    if (ds.subsong[0]->ordersLen>127) {
      logE("song is too long!");
      lastError="song is too long!";
      delete[] file;
      return false;
    }

    if (ds.version<20 && ds.version>3) {
      ds.subsong[0]->arpLen=reader.readC();
    } else {
      ds.subsong[0]->arpLen=1;
    }

    if (ds.system[0]==DIV_SYSTEM_YMU759) {
      switch (ds.subsong[0]->timeBase) {
        case 0:
          ds.subsong[0]->hz=248;
          break;
        case 1:
          ds.subsong[0]->hz=200;
          break;
        case 2:
          ds.subsong[0]->hz=100;
          break;
        case 3:
          ds.subsong[0]->hz=50;
          break;
        case 4:
          ds.subsong[0]->hz=25;
          break;
        case 5:
          ds.subsong[0]->hz=20;
          break;
        default:
          ds.subsong[0]->hz=248;
          break;
      }
      ds.subsong[0]->timeBase=0;
      addWarning("Yamaha YMU759 emulation is incomplete! please migrate your song to the OPL3 system.");
    }

    logV("%x",reader.tell());

    logI("reading pattern matrix (%d * %d = %d)...",ds.subsong[0]->ordersLen,getChannelCount(ds.system[0]),ds.subsong[0]->ordersLen*getChannelCount(ds.system[0]));
    for (int i=0; i<getChannelCount(ds.system[0]); i++) {
      for (int j=0; j<ds.subsong[0]->ordersLen; j++) {
        ds.subsong[0]->orders.ord[i][j]=reader.readC();
        if (ds.subsong[0]->orders.ord[i][j]>0x7f) {
          logE("order at %d, %d out of range! (%d)",i,j,ds.subsong[0]->orders.ord[i][j]);
          lastError=fmt::sprintf("order at %d, %d out of range! (%d)",i,j,ds.subsong[0]->orders.ord[i][j]);
          delete[] file;
          return false;
        }
        if (ds.version>0x18) { // 1.1 pattern names
          ds.subsong[0]->pat[i].getPattern(j,true)->name=reader.readString((unsigned char)reader.readC());
        }
      }
      if (ds.version>0x03 && ds.version<0x06 && i<16) {
        historicColIns[i]=reader.readC();
      }
    }

    logV("%x",reader.tell());

    if (ds.version>0x05) {
      ds.insLen=(unsigned char)reader.readC();
    } else {
      ds.insLen=16;
    }
    logI("reading instruments (%d)...",ds.insLen);
    if (ds.insLen>0) ds.ins.reserve(ds.insLen);
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      unsigned char mode=0;
      if (ds.version>0x05) {
        ins->name=reader.readString((unsigned char)reader.readC());
      }
      logD("%d name: %s",i,ins->name.c_str());
      if (ds.version<0x0b) {
        // instruments in ancient versions were all FM.
        mode=1;
      } else {
        mode=reader.readC();
        if (mode>1) logW("%d: invalid instrument mode %d!",i,mode);
      }
      ins->type=mode?DIV_INS_FM:DIV_INS_STD;
      if (ds.system[0]==DIV_SYSTEM_GB) {
        ins->type=DIV_INS_GB;
      }
      if (ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) {
        ins->type=DIV_INS_C64;
      }
      if (ds.system[0]==DIV_SYSTEM_YM2610 || ds.system[0]==DIV_SYSTEM_YM2610_EXT
       || ds.system[0]==DIV_SYSTEM_YM2610_FULL || ds.system[0]==DIV_SYSTEM_YM2610_FULL_EXT
       || ds.system[0]==DIV_SYSTEM_YM2610B || ds.system[0]==DIV_SYSTEM_YM2610B_EXT) {
        if (!mode) {
          ins->type=DIV_INS_AY;
        }
      }
      if (ds.system[0]==DIV_SYSTEM_PCE) {
        ins->type=DIV_INS_PCE;
      }
      if ((ds.system[0]==DIV_SYSTEM_SMS_OPLL || ds.system[0]==DIV_SYSTEM_NES_VRC7) && ins->type==DIV_INS_FM) {
        ins->type=DIV_INS_OPLL;
      }
      if (ds.system[0]==DIV_SYSTEM_YMU759) {
        ins->type=DIV_INS_OPL;
      }
      if (ds.system[0]==DIV_SYSTEM_ARCADE) {
        ins->type=DIV_INS_OPM;
      }
      if ((ds.system[0]==DIV_SYSTEM_NES || ds.system[0]==DIV_SYSTEM_NES_VRC7 || ds.system[0]==DIV_SYSTEM_NES_FDS) && ins->type==DIV_INS_STD) {
        ins->type=DIV_INS_NES;
      }

      if (mode) { // FM
        if (ds.version>0x05) {
          ins->fm.alg=reader.readC();
          if (ds.version<0x13) {
            reader.readC();
          }
          ins->fm.fb=reader.readC();
          if (ds.version<0x13) {
            reader.readC();
          }
          ins->fm.fms=reader.readC();
          if (ds.version<0x13) {
            reader.readC();
            ins->fm.ops=2+reader.readC()*2;
            if (ds.system[0]!=DIV_SYSTEM_YMU759) ins->fm.ops=4;
          } else {
            ins->fm.ops=4;
          }
          ins->fm.ams=reader.readC();
        } else {
          ins->fm.alg=reader.readC();
          reader.readC();
          ins->fm.fb=reader.readC();
          reader.readC(); // apparently an index of sorts starting from 0x59?
          ins->fm.fms=reader.readC();
          reader.readC(); // 0x59+index?
          ins->fm.ops=2+reader.readC()*2;
        }

        logD("ALG %d FB %d FMS %d AMS %d OPS %d",ins->fm.alg,ins->fm.fb,ins->fm.fms,ins->fm.ams,ins->fm.ops);
        if (ins->fm.ops!=2 && ins->fm.ops!=4) {
          logE("invalid op count %d. did we read it wrong?",ins->fm.ops);
          lastError="file is corrupt or unreadable at operators";
          delete[] file;
          return false;
        }

        for (int j=0; j<ins->fm.ops; j++) {
          ins->fm.op[j].am=reader.readC();
          ins->fm.op[j].ar=reader.readC();
          if (ds.system[0]==DIV_SYSTEM_SMS_OPLL || ds.system[0]==DIV_SYSTEM_NES_VRC7) {
            ins->fm.op[j].ar&=15;
          }
          if (ds.version<0x13) {
            ins->fm.op[j].dam=reader.readC();
          }
          ins->fm.op[j].dr=reader.readC();
          if (ds.system[0]==DIV_SYSTEM_SMS_OPLL || ds.system[0]==DIV_SYSTEM_NES_VRC7) {
            ins->fm.op[j].dr&=15;
          }
          if (ds.version<0x13) {
            ins->fm.op[j].dvb=reader.readC();
            ins->fm.op[j].egt=reader.readC();
            ins->fm.op[j].ksl=reader.readC();
            if (ds.version<0x11) { // don't know when did this change
              ins->fm.op[j].ksr=reader.readC();
            }
          }
          ins->fm.op[j].mult=reader.readC();
          ins->fm.op[j].rr=reader.readC();
          ins->fm.op[j].sl=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].sus=reader.readC();
          }
          ins->fm.op[j].tl=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].vib=reader.readC();
            ins->fm.op[j].ws=reader.readC();
          } else {
            if (ds.system[0]==DIV_SYSTEM_SMS_OPLL || ds.system[0]==DIV_SYSTEM_NES_VRC7) {
              if (j==0) {
                ins->fm.opllPreset=reader.readC();
              } else {
                reader.readC();
              }
            } else {
              ins->fm.op[j].dt2=reader.readC();
            }
          }
          if (ds.version>0x05) {
            if (ds.system[0]==DIV_SYSTEM_SMS_OPLL || ds.system[0]==DIV_SYSTEM_NES_VRC7) {
              ins->fm.op[j].ksr=reader.readC();
              ins->fm.op[j].vib=reader.readC();
              ins->fm.op[j].ksl=reader.readC();
              ins->fm.op[j].ssgEnv=reader.readC();
            } else {
              ins->fm.op[j].rs=reader.readC();
              ins->fm.op[j].dt=reader.readC();
              ins->fm.op[j].d2r=reader.readC();
              ins->fm.op[j].ssgEnv=reader.readC();
            }
          }
          if (ds.version<0x12) { // before version 10 all ops were responsive to volume
            ins->fm.op[j].kvs=1;
          }

          logD("OP%d: AM %d AR %d DAM %d DR %d DVB %d EGT %d KSL %d MULT %d RR %d SL %d SUS %d TL %d VIB %d WS %d RS %d DT %d D2R %d SSG-EG %d",j,
               ins->fm.op[j].am,
               ins->fm.op[j].ar,
               ins->fm.op[j].dam,
               ins->fm.op[j].dr,
               ins->fm.op[j].dvb,
               ins->fm.op[j].egt,
               ins->fm.op[j].ksl,
               ins->fm.op[j].mult,
               ins->fm.op[j].rr,
               ins->fm.op[j].sl,
               ins->fm.op[j].sus,
               ins->fm.op[j].tl,
               ins->fm.op[j].vib,
               ins->fm.op[j].ws,
               ins->fm.op[j].rs,
               ins->fm.op[j].dt,
               ins->fm.op[j].d2r,
               ins->fm.op[j].ssgEnv
               );
        }

        // swap alg operator 2 and 3 if YMU759
        if (ds.system[0]==DIV_SYSTEM_YMU759 && ins->fm.ops==4) {
          DivInstrumentFM::Operator oldOp=ins->fm.op[2];
          ins->fm.op[2]=ins->fm.op[1];
          ins->fm.op[1]=oldOp;

          if (ins->fm.alg==1) {
            ins->fm.alg=2;
          } else if (ins->fm.alg==2) {
            ins->fm.alg=1;
          }
        }
      } else { // STD
        if (ds.system[0]!=DIV_SYSTEM_GB || ds.version<0x12) {
          ins->std.get_macro(DIV_MACRO_VOL, true)->len=reader.readC();
          for (int j=0; j<ins->std.get_macro(DIV_MACRO_VOL, true)->len; j++) {
            if (ds.version<0x0e) {
              ins->std.get_macro(DIV_MACRO_VOL, true)->val[j]=reader.readC();
            } else {
              ins->std.get_macro(DIV_MACRO_VOL, true)->val[j]=reader.readI();
            }
          }
          if (ins->std.get_macro(DIV_MACRO_VOL, true)->len>0) {
            ins->std.get_macro(DIV_MACRO_VOL, true)->open=true;
            ins->std.get_macro(DIV_MACRO_VOL, true)->loop=reader.readC();
          } else {
            ins->std.get_macro(DIV_MACRO_VOL, true)->open=false;
          }
        }

        ins->std.get_macro(DIV_MACRO_ARP, true)->len=reader.readC();
        for (int j=0; j<ins->std.get_macro(DIV_MACRO_ARP, true)->len; j++) {
          if (ds.version<0x0e) {
            ins->std.get_macro(DIV_MACRO_ARP, true)->val[j]=reader.readC();
          } else {
            ins->std.get_macro(DIV_MACRO_ARP, true)->val[j]=reader.readI();
          }
        }
        if (ins->std.get_macro(DIV_MACRO_ARP, true)->len>0) {
          ins->std.get_macro(DIV_MACRO_ARP, true)->loop=reader.readC();
          ins->std.get_macro(DIV_MACRO_ARP, true)->open=true;
        } else {
          ins->std.get_macro(DIV_MACRO_ARP, true)->open=false;
        }
        if (ds.version>0x0f) {
          ins->std.get_macro(DIV_MACRO_ARP, true)->mode=reader.readC();
        }
        if (!ins->std.get_macro(DIV_MACRO_ARP, true)->mode) {
          for (int j=0; j<ins->std.get_macro(DIV_MACRO_ARP, true)->len; j++) {
            ins->std.get_macro(DIV_MACRO_ARP, true)->val[j]-=12;
          }
        } else {
          ins->std.get_macro(DIV_MACRO_ARP, true)->mode=0;
          for (int j=0; j<ins->std.get_macro(DIV_MACRO_ARP, true)->len; j++) {
            ins->std.get_macro(DIV_MACRO_ARP, true)->val[j]^=0x40000000;
          }
          if (ins->std.get_macro(DIV_MACRO_ARP, true)->loop==255 && ins->std.get_macro(DIV_MACRO_ARP, true)->len<255) {
            ins->std.get_macro(DIV_MACRO_ARP, true)->val[ins->std.get_macro(DIV_MACRO_ARP, true)->len++]=0;
          }
        }

        ins->std.get_macro(DIV_MACRO_DUTY, true)->len=reader.readC();
        for (int j=0; j<ins->std.get_macro(DIV_MACRO_DUTY, true)->len; j++) {
          if (ds.version<0x0e) {
            ins->std.get_macro(DIV_MACRO_DUTY, true)->val[j]=reader.readC();
          } else {
            ins->std.get_macro(DIV_MACRO_DUTY, true)->val[j]=reader.readI();
          }
          /*if ((ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) && ins->std.get_macro(DIV_MACRO_DUTY, true)->val[j]>24) {
            ins->std.get_macro(DIV_MACRO_DUTY, true)->val[j]=24;
          }*/
        }
        if (ins->std.get_macro(DIV_MACRO_DUTY, true)->len>0) {
          ins->std.get_macro(DIV_MACRO_DUTY, true)->open=true;
          ins->std.get_macro(DIV_MACRO_DUTY, true)->loop=reader.readC();
        } else {
          ins->std.get_macro(DIV_MACRO_DUTY, true)->open=false;
        }

        ins->std.get_macro(DIV_MACRO_WAVE, true)->len=reader.readC();
        for (int j=0; j<ins->std.get_macro(DIV_MACRO_WAVE, true)->len; j++) {
          if (ds.version<0x0e) {
            ins->std.get_macro(DIV_MACRO_WAVE, true)->val[j]=reader.readC();
          } else {
            ins->std.get_macro(DIV_MACRO_WAVE, true)->val[j]=reader.readI();
          }
        }
        if (ins->std.get_macro(DIV_MACRO_WAVE, true)->len>0) {
          ins->std.get_macro(DIV_MACRO_WAVE, true)->open=true;
          ins->std.get_macro(DIV_MACRO_WAVE, true)->loop=reader.readC();
        } else {
          ins->std.get_macro(DIV_MACRO_WAVE, true)->open=false;
        }

        if (ds.system[0]==DIV_SYSTEM_C64_6581 || ds.system[0]==DIV_SYSTEM_C64_8580) {
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
          if (ds.version<0x11) {
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

        if (ds.system[0]==DIV_SYSTEM_GB && ds.version>0x11) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();
          ins->std.get_macro(DIV_MACRO_VOL, true)->open=false;

          logD("GB data: vol %d dir %d len %d sl %d",ins->gb.envVol,ins->gb.envDir,ins->gb.envLen,ins->gb.soundLen);
        } else if (ds.system[0]==DIV_SYSTEM_GB) {
          // set software envelope flag
          ins->gb.softEnv=true;
          // try to convert macro to envelope in case the user decides to switch to them
          if (ins->std.get_macro(DIV_MACRO_VOL, true)->len>0) {
            ins->gb.envVol=ins->std.get_macro(DIV_MACRO_VOL, true)->val[0];
            if (ins->std.get_macro(DIV_MACRO_VOL, true)->val[0]<ins->std.get_macro(DIV_MACRO_VOL, true)->val[1]) {
              ins->gb.envDir=true;
            }
            if (ins->std.get_macro(DIV_MACRO_VOL, true)->val[ins->std.get_macro(DIV_MACRO_VOL, true)->len-1]==0) {
              ins->gb.soundLen=ins->std.get_macro(DIV_MACRO_VOL, true)->len*2;
            }
          }
        }
      }

      ds.ins.push_back(ins);
    }

    if (ds.version>0x0b) {
      ds.waveLen=(unsigned char)reader.readC();
      logI("reading wavetables (%d)...",ds.waveLen);
      if (ds.waveLen>0) ds.wave.reserve(ds.waveLen);
      for (int i=0; i<ds.waveLen; i++) {
        DivWavetable* wave=new DivWavetable;
        wave->len=(unsigned char)reader.readI();
        if (ds.system[0]==DIV_SYSTEM_GB) {
          wave->max=15;
        }
        if (ds.system[0]==DIV_SYSTEM_NES_FDS) {
          wave->max=63;
        }
        if (wave->len>65) {
          logE("invalid wave length %d. are we doing something wrong?",wave->len);
          lastError="file is corrupt or unreadable at wavetables";
          delete[] file;
          return false;
        }
        logD("%d length %d",i,wave->len);
        for (int j=0; j<wave->len; j++) {
          if (ds.version<0x0e) {
            wave->data[j]=reader.readC();
          } else {
            wave->data[j]=reader.readI();
          }
          wave->data[j]&=wave->max;
        }
        // #FDS4Bit
        if (ds.system[0]==DIV_SYSTEM_NES_FDS && ds.version<0x1a) {
          for (int j=0; j<wave->len; j++) {
            wave->data[j]*=4;
          }
        }
        ds.wave.push_back(wave);
      }

      // sometimes there's a single length 0 wavetable in the file. I don't know why.
      if (ds.waveLen==1) {
        if (ds.wave[0]->len==0) {
          ds.clearWavetables();
        }
      }
    }

    logV("%x",reader.tell());

    logI("reading patterns (%d channels, %d orders)...",getChannelCount(ds.system[0]),ds.subsong[0]->ordersLen);
    for (int i=0; i<getChannelCount(ds.system[0]); i++) {
      DivChannelData& chan=ds.subsong[0]->pat[i];
      if (ds.version<0x0a) {
        chan.effectCols=1;
      } else {
        chan.effectCols=reader.readC();
      }
      logD("%d fx rows: %d",i,chan.effectCols);
      if (chan.effectCols>4 || chan.effectCols<1) {
        logE("invalid effect column count %d. are you sure everything is ok?",chan.effectCols);
        lastError="file is corrupt or unreadable at effect columns";
        delete[] file;
        return false;
      }
      for (int j=0; j<ds.subsong[0]->ordersLen; j++) {
        DivPattern* pat=chan.getPattern(ds.subsong[0]->orders.ord[i][j],true);
        if (ds.version>0x08) { // current pattern format
          for (int k=0; k<ds.subsong[0]->patLen; k++) {
            // note
            pat->data[k][0]=reader.readS();
            // octave
            pat->data[k][1]=reader.readS();
            if (ds.system[0]==DIV_SYSTEM_SMS && ds.version<0x0e && pat->data[k][1]>0) {
              // apparently it was up one octave before
              pat->data[k][1]--;
            } else if (ds.system[0]==DIV_SYSTEM_GENESIS && ds.version<0x0e && pat->data[k][1]>0 && i>5) {
              // ditto
              pat->data[k][1]--;
            }
            if (ds.version<0x12) {
              if (ds.system[0]==DIV_SYSTEM_GB && i==3 && pat->data[k][1]>0) {
                // back then noise was 2 octaves lower
                pat->data[k][1]-=2;
              }
            }
            if (ds.system[0]==DIV_SYSTEM_YMU759 && pat->data[k][0]!=0) {
              // apparently YMU759 is stored 2 octaves lower
              pat->data[k][1]+=2;
            }
            if (pat->data[k][0]==0 && pat->data[k][1]!=0) {
              logD("what? %d:%d:%d note %d octave %d",i,j,k,pat->data[k][0],pat->data[k][1]);
              pat->data[k][0]=12;
              pat->data[k][1]--;
            }
            // volume
            pat->data[k][3]=reader.readS();
            if (ds.version<0x0a) {
              // back then volume was stored as 00-ff instead of 00-7f/0-f
              if (i>5) {
                pat->data[k][3]>>=4;
              } else {
                pat->data[k][3]>>=1;
              }
            }
            if (ds.version<0x12) {
              if (ds.system[0]==DIV_SYSTEM_GB && i==2 && pat->data[k][3]>0) {
                // volume range of GB wave channel was 0-3 rather than 0-F
                pat->data[k][3]=(pat->data[k][3]&3)*5;
              }
            }
            for (int l=0; l<chan.effectCols; l++) {
              // effect
              pat->data[k][4+(l<<1)]=reader.readS();
              pat->data[k][5+(l<<1)]=reader.readS();

              if (ds.version<0x14) {
                if (pat->data[k][4+(l<<1)]==0xe5 && pat->data[k][5+(l<<1)]!=-1) {
                  pat->data[k][5+(l<<1)]=128+((pat->data[k][5+(l<<1)]-128)/4);
                }
              }
            }
            // instrument
            pat->data[k][2]=reader.readS();

            // this is sad
            if (ds.system[0]==DIV_SYSTEM_NES_FDS) {
              if (i==5 && pat->data[k][2]!=-1) {
                if (pat->data[k][2]>=0 && pat->data[k][2]<ds.insLen) {
                  ds.ins[pat->data[k][2]]->type=DIV_INS_FDS;
                }
              }
            }
          }
        } else { // historic pattern format
          if (i<16) pat->data[0][2]=historicColIns[i];
          for (int k=0; k<ds.subsong[0]->patLen; k++) {
            // note
            pat->data[k][0]=reader.readC();
            // octave
            pat->data[k][1]=reader.readC();
            if (pat->data[k][0]!=0) {
              // YMU759 is stored 2 octaves lower
              pat->data[k][1]+=2;
            }
            if (pat->data[k][0]==0 && pat->data[k][1]!=0) {
              logD("what? %d:%d:%d note %d octave %d",i,j,k,pat->data[k][0],pat->data[k][1]);
              pat->data[k][0]=12;
              pat->data[k][1]--;
            }
            // volume and effect
            unsigned char vol=reader.readC();
            unsigned char fx=reader.readC();
            unsigned char fxVal=reader.readC();
            pat->data[k][3]=(vol==0x80 || vol==0xff)?-1:vol;
            // effect
            pat->data[k][4]=(fx==0x80 || fx==0xff)?-1:fx;
            pat->data[k][5]=(fxVal==0x80 || fx==0xff)?-1:fxVal;
            // instrument
            if (ds.version>0x05) {
              pat->data[k][2]=reader.readC();
              if (pat->data[k][2]==0x80 || pat->data[k][2]==0xff) pat->data[k][2]=-1;
            }
          }
        }
      }
    }

    int ymuSampleRate=20;

    ds.sampleLen=(unsigned char)reader.readC();
    logI("reading samples (%d)...",ds.sampleLen);
    if (ds.version<0x0b && ds.sampleLen>0) {
      // it appears this byte stored the YMU759 sample rate
      ymuSampleRate=reader.readC();
    }
    if (ds.sampleLen>0) ds.sample.reserve(ds.sampleLen);
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;
      int length=reader.readI();
      int cutStart=0;
      int cutEnd=length;
      int pitch=5;
      int vol=50;
      short* data;
      unsigned char* adpcmData;
      if (length<0) {
        logE("invalid sample length %d. are we doing something wrong?",length);
        lastError="file is corrupt or unreadable at samples";
        delete[] file;
        return false;
      }
      if (ds.version>0x16) {
        sample->name=reader.readString((unsigned char)reader.readC());
      } else {
        sample->name="";
      }
      logD("%d name %s (%d)",i,sample->name.c_str(),length);
      sample->rate=22050;
      if (ds.version>=0x0b) {
        sample->rate=fileToDivRate(reader.readC());
        sample->centerRate=sample->rate;
        pitch=reader.readC();
        vol=reader.readC();
      }
      if (ds.version<=0x08) {
        sample->rate=ymuSampleRate*400;
      }
      if (ds.version>0x15) {
        sample->depth=(DivSampleDepth)reader.readC();
        if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) {
          logW("%d: sample depth is wrong! (%d)",i,(int)sample->depth);
          sample->depth=DIV_SAMPLE_DEPTH_16BIT;
        }
      } else {
        if (ds.version>0x08) {
          sample->depth=DIV_SAMPLE_DEPTH_16BIT;
        } else {
          // it appears samples were stored as ADPCM back then
          sample->depth=DIV_SAMPLE_DEPTH_YMZ_ADPCM;
        }
      }
      if (ds.version>=0x1b) {
        // what the hell man...
        cutStart=reader.readI();
        cutEnd=reader.readI();
        logV("cutStart: %d cutEnd: %d",cutStart,cutEnd);
      }
      if (length>0) {
        if (ds.version>0x08) {
          if (ds.version<0x0b) {
            data=new short[1+(length/2)];
            reader.read(data,length);
            length/=2;
          } else {
            data=new short[length];
            reader.read(data,length*2);
          }
          
#ifdef TA_BIG_ENDIAN
          // convert to big-endian
          for (int pos=0; pos<length; pos++) {
            data[pos]=(short)((((unsigned short)data[pos])<<8)|(((unsigned short)data[pos])>>8));
          }
#endif

          int scaledLen=ceil((double)length/samplePitches[pitch]);

          if (scaledLen>0) {
            // resample
            logD("%d: scaling from %d...",i,pitch);
            
            short* newData=new short[scaledLen];
            memset(newData,0,scaledLen*sizeof(short));
            int k=0;
            float mult=(float)(vol)/50.0f;
            for (double j=0; j<length; j+=samplePitches[pitch]) {
              if (k>=scaledLen) {
                break;
              }
              if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                float next=(float)(data[(unsigned int)j]-0x80)*mult;
                newData[k++]=fmin(fmax(next,-128),127);
              } else {
                float next=(float)data[(unsigned int)j]*mult;
                newData[k++]=fmin(fmax(next,-32768),32767);
              }
            }

            delete[] data;
            data=newData;
          }

          logV("length: %d. scaledLen: %d.",length,scaledLen);

          if (ds.version>=0x1b) {
            if (cutStart<0 || cutStart>scaledLen) {
              logE("cutStart is out of range! (%d, scaledLen: %d)",cutStart,scaledLen);
              lastError="file is corrupt or unreadable at samples";
              delete[] file;
              return false;
            }
            if (cutEnd<0 || cutEnd>scaledLen) {
              logE("cutEnd is out of range! (%d, scaledLen: %d)",cutEnd,scaledLen);
              lastError="file is corrupt or unreadable at samples";
              delete[] file;
              return false;
            }
            if (cutEnd<cutStart) {
              logE("cutEnd %d is before cutStart %d. what's going on?",cutEnd,cutStart);
              lastError="file is corrupt or unreadable at samples";
              delete[] file;
              return false;
            }
            if (cutStart!=0 || cutEnd!=scaledLen) {
              // cut data
              short* newData=new short[cutEnd-cutStart];
              memcpy(newData,&data[cutStart],(cutEnd-cutStart)*sizeof(short));
              delete[] data;
              data=newData;
              scaledLen=cutEnd-cutStart;
              cutStart=0;
              cutEnd=scaledLen;
            }
          }

          // copy data
          if (!sample->init(scaledLen)) {
            logE("%d: error while initializing sample!",i);
          } else {
            for (int i=0; i<scaledLen; i++) {
              if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
                sample->data8[i]=data[i];
              } else {
                sample->data16[i]=data[i];
              }
            }
          }

          delete[] data;
        } else {
          // YMZ ADPCM
          adpcmData=new unsigned char[length];
          logV("%x",reader.tell());
          reader.read(adpcmData,length);
          for (int i=0; i<length; i++) {
            adpcmData[i]=(adpcmData[i]<<4)|(adpcmData[i]>>4);
          }
          if (!sample->init(length*2)) {
            logE("%d: error while initializing sample!",i);
          }

          memcpy(sample->dataZ,adpcmData,length);
          delete[] adpcmData;
        }
      }
      ds.sample.push_back(sample);
    }

    if (reader.tell()<reader.size()) {
      if ((reader.tell()+1)!=reader.size()) {
        logW("premature end of song (we are at %x, but size is %x)",reader.tell(),reader.size());
      }
    }

    // handle compound systems
    if (ds.system[0]==DIV_SYSTEM_GENESIS) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_YM2612;
      ds.system[1]=DIV_SYSTEM_SMS;
      ds.systemVol[1]=0.5f;
    }
    if (ds.system[0]==DIV_SYSTEM_GENESIS_EXT) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_YM2612_EXT;
      ds.system[1]=DIV_SYSTEM_SMS;
      ds.systemVol[1]=0.5f;
    }
    if (ds.system[0]==DIV_SYSTEM_ARCADE) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_YM2151;
      ds.system[1]=DIV_SYSTEM_SEGAPCM_COMPAT;
    }
    if (ds.system[0]==DIV_SYSTEM_SMS_OPLL) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_SMS;
      ds.system[1]=DIV_SYSTEM_OPLL;
    }
    if (ds.system[0]==DIV_SYSTEM_NES_VRC7) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_NES;
      ds.system[1]=DIV_SYSTEM_VRC7;
    }
    if (ds.system[0]==DIV_SYSTEM_NES_FDS) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_NES;
      ds.system[1]=DIV_SYSTEM_FDS;
    }

    // SMS noise freq
    if (ds.system[0]==DIV_SYSTEM_SMS) {
      ds.systemFlags[0].set("noEasyNoise",true);
    }

    // NES PCM
    if (ds.system[0]==DIV_SYSTEM_NES) {
      ds.systemFlags[0].set("dpcmMode",false);
    }

    // C64 no key priority, reset time and multiply relative
    if (ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) {
      ds.systemFlags[0].set("keyPriority",false);
      ds.systemFlags[0].set("initResetTime",1);
      ds.systemFlags[0].set("multiplyRel",true);
    }

    // OPM broken pitch
    if (ds.system[0]==DIV_SYSTEM_YM2151) {
      ds.systemFlags[0].set("brokenPitch",true);
    }

    ds.systemName=getSongSystemLegacyName(ds,!getConfInt("noMultiSystem",0));

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    changeSong(0);
    recalcChans();
    saveLock.unlock();
    BUSY_END;
    if (active) {
      initDispatch();
      BUSY_BEGIN;
      renderSamples();
      reset();
      BUSY_END;
    }
  } catch (EndOfFileException& e) {
    logE("premature end of file!");
    lastError="incomplete file";
    delete[] file;
    return false;
  }
  delete[] file;
  return true;
}