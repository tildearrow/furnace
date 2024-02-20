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

#include "fileOpsCommon.h"

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
      ds.oldAlwaysSetVolume=true;

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
          ins->std.volMacro.len=reader.readC();
          for (int j=0; j<ins->std.volMacro.len; j++) {
            if (ds.version<0x0e) {
              ins->std.volMacro.val[j]=reader.readC();
            } else {
              ins->std.volMacro.val[j]=reader.readI();
            }
          }
          if (ins->std.volMacro.len>0) {
            ins->std.volMacro.open=true;
            ins->std.volMacro.loop=reader.readC();
          } else {
            ins->std.volMacro.open=false;
          }
        }

        ins->std.arpMacro.len=reader.readC();
        for (int j=0; j<ins->std.arpMacro.len; j++) {
          if (ds.version<0x0e) {
            ins->std.arpMacro.val[j]=reader.readC();
          } else {
            ins->std.arpMacro.val[j]=reader.readI();
          }
        }
        if (ins->std.arpMacro.len>0) {
          ins->std.arpMacro.loop=reader.readC();
          ins->std.arpMacro.open=true;
        } else {
          ins->std.arpMacro.open=false;
        }
        if (ds.version>0x0f) {
          ins->std.arpMacro.mode=reader.readC();
        }
        if (!ins->std.arpMacro.mode) {
          for (int j=0; j<ins->std.arpMacro.len; j++) {
            ins->std.arpMacro.val[j]-=12;
          }
        } else {
          ins->std.arpMacro.mode=0;
          for (int j=0; j<ins->std.arpMacro.len; j++) {
            ins->std.arpMacro.val[j]^=0x40000000;
          }
          if (ins->std.arpMacro.loop==255 && ins->std.arpMacro.len<255) {
            ins->std.arpMacro.val[ins->std.arpMacro.len++]=0;
          }
        }

        ins->std.dutyMacro.len=reader.readC();
        for (int j=0; j<ins->std.dutyMacro.len; j++) {
          if (ds.version<0x0e) {
            ins->std.dutyMacro.val[j]=reader.readC();
          } else {
            ins->std.dutyMacro.val[j]=reader.readI();
          }
          /*if ((ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) && ins->std.dutyMacro.val[j]>24) {
            ins->std.dutyMacro.val[j]=24;
          }*/
        }
        if (ins->std.dutyMacro.len>0) {
          ins->std.dutyMacro.open=true;
          ins->std.dutyMacro.loop=reader.readC();
        } else {
          ins->std.dutyMacro.open=false;
        }

        ins->std.waveMacro.len=reader.readC();
        for (int j=0; j<ins->std.waveMacro.len; j++) {
          if (ds.version<0x0e) {
            ins->std.waveMacro.val[j]=reader.readC();
          } else {
            ins->std.waveMacro.val[j]=reader.readI();
          }

          // piece of crap offset by 1
          if (ds.system[0]==DIV_SYSTEM_YM2610 || ds.system[0]==DIV_SYSTEM_YM2610_EXT) {
            ins->std.waveMacro.val[j]++;
          }
        }
        if (ins->std.waveMacro.len>0) {
          ins->std.waveMacro.open=true;
          ins->std.waveMacro.loop=reader.readC();
        } else {
          ins->std.waveMacro.open=false;
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
            ins->std.algMacro.len=ins->std.volMacro.len;
            ins->std.algMacro.loop=ins->std.volMacro.loop;
            ins->std.algMacro.rel=ins->std.volMacro.rel;
            for (int j=0; j<ins->std.algMacro.len; j++) {
              ins->std.algMacro.val[j]=-(ins->std.volMacro.val[j]-18);
            }
            ins->std.volMacro.len=0;
            memset(ins->std.volMacro.val,0,256*sizeof(int));
          }
          for (int j=0; j<ins->std.dutyMacro.len; j++) {
            ins->std.dutyMacro.val[j]-=12;
          }
        }

        if (ds.system[0]==DIV_SYSTEM_GB && ds.version>0x11) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();
          ins->std.volMacro.open=false;

          logD("GB data: vol %d dir %d len %d sl %d",ins->gb.envVol,ins->gb.envDir,ins->gb.envLen,ins->gb.soundLen);
        } else if (ds.system[0]==DIV_SYSTEM_GB) {
          // set software envelope flag
          ins->gb.softEnv=true;
          // try to convert macro to envelope in case the user decides to switch to them
          if (ins->std.volMacro.len>0) {
            ins->gb.envVol=ins->std.volMacro.val[0];
            if (ins->std.volMacro.val[0]<ins->std.volMacro.val[1]) {
              ins->gb.envDir=true;
            }
            if (ins->std.volMacro.val[ins->std.volMacro.len-1]==0) {
              ins->gb.soundLen=ins->std.volMacro.len*2;
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

SafeWriter* DivEngine::saveDMF(unsigned char version) {
  // fail if version is not supported
  if (version>26) version=26;
  if (version<24) {
    logE("cannot save in this version!");
    lastError="invalid version to save in! this is a bug!";
    return NULL;
  }
  // check whether system is compound
  bool isFlat=false;
  if (song.systemLen==2) {
    if (song.system[0]==DIV_SYSTEM_YM2612 && song.system[1]==DIV_SYSTEM_SMS) {
      isFlat=true;
    }
    if (song.system[0]==DIV_SYSTEM_YM2612_EXT && song.system[1]==DIV_SYSTEM_SMS) {
      isFlat=true;
    }
    if (song.system[0]==DIV_SYSTEM_YM2151 && song.system[1]==DIV_SYSTEM_SEGAPCM_COMPAT) {
      isFlat=true;
    }
    if (song.system[0]==DIV_SYSTEM_SMS && song.system[1]==DIV_SYSTEM_OPLL) {
      isFlat=true;
    }
    if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_VRC7) {
      isFlat=true;
    }
    if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_FDS) {
      isFlat=true;
    }
  }
  // fail if more than one system
  if (!isFlat && song.systemLen!=1) {
      logE("cannot save multiple systems in this format!");
      lastError="multiple systems not possible on .dmf";
      return NULL;
    }
  // fail if this is an YMU759 song
  if (song.system[0]==DIV_SYSTEM_YMU759) {
    logE("cannot save YMU759 song!");
    lastError="YMU759 song saving is not supported";
    return NULL;
  }
  // fail if the system is SMS+OPLL and version<25
  if (version<25 && song.system[0]==DIV_SYSTEM_SMS && song.system[1]==DIV_SYSTEM_OPLL) {
    logE("Master System FM expansion not supported in 1.0/legacy .dmf!");
    lastError="Master System FM expansion not supported in 1.0/legacy .dmf!";
    return NULL;
  }
  // fail if the system is NES+VRC7 and version<25
  if (version<25 && song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_VRC7) {
    logE("NES + VRC7 not supported in 1.0/legacy .dmf!");
    lastError="NES + VRC7 not supported in 1.0/legacy .dmf!";
    return NULL;
  }
  // fail if the system is FDS and version<25
  if (version<25 && song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_FDS) {
    logE("FDS not supported in 1.0/legacy .dmf!");
    lastError="FDS not supported in 1.0/legacy .dmf!";
    return NULL;
  }
  // fail if the system is Furnace-exclusive
  if (!isFlat && systemToFileDMF(song.system[0])==0) {
    logE("cannot save Furnace-exclusive system song!");
    lastError="this system is not possible on .dmf";
    return NULL;
  }
  // fail if values are out of range
  if (curSubSong->ordersLen>127) {
    logE("maximum .dmf song length is 127!");
    lastError="maximum .dmf song length is 127";
    return NULL;
  }
  if (song.ins.size()>128) {
    logE("maximum number of instruments in .dmf is 128!");
    lastError="maximum number of instruments in .dmf is 128";
    return NULL;
  }
  if (song.wave.size()>64) {
    logE("maximum number of wavetables in .dmf is 64!");
    lastError="maximum number of wavetables in .dmf is 64";
    return NULL;
  }
  for (int i=0; i<chans; i++) {
    for (int j=0; j<curSubSong->ordersLen; j++) {
      if (curOrders->ord[i][j]>0x7f) {
        logE("order %d, %d is out of range (0-127)!",curOrders->ord[i][j]);
        lastError=fmt::sprintf("order %d, %d is out of range (0-127)",curOrders->ord[i][j]);
        return NULL;
      }
    }
  }
  saveLock.lock();
  warnings="";
  song.version=version;
  song.isDMF=true;

  SafeWriter* w=new SafeWriter;
  w->init();
  // write magic
  w->write(DIV_DMF_MAGIC,16);
  // version
  w->writeC(version);
  DivSystem sys=DIV_SYSTEM_NULL;
  if (song.system[0]==DIV_SYSTEM_YM2612 && song.system[1]==DIV_SYSTEM_SMS) {
    w->writeC(systemToFileDMF(DIV_SYSTEM_GENESIS));
    sys=DIV_SYSTEM_GENESIS;
  } else if (song.system[0]==DIV_SYSTEM_YM2612_EXT && song.system[1]==DIV_SYSTEM_SMS) {
    w->writeC(systemToFileDMF(DIV_SYSTEM_GENESIS_EXT));
    sys=DIV_SYSTEM_GENESIS_EXT;
  } else if (song.system[0]==DIV_SYSTEM_YM2151 && song.system[1]==DIV_SYSTEM_SEGAPCM_COMPAT) {
    w->writeC(systemToFileDMF(DIV_SYSTEM_ARCADE));
    sys=DIV_SYSTEM_ARCADE;
  } else if (song.system[0]==DIV_SYSTEM_SMS && song.system[1]==DIV_SYSTEM_OPLL) {
    w->writeC(systemToFileDMF(DIV_SYSTEM_SMS_OPLL));
    sys=DIV_SYSTEM_SMS_OPLL;
  } else if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_VRC7) {
    w->writeC(systemToFileDMF(DIV_SYSTEM_NES_VRC7));
    sys=DIV_SYSTEM_NES_VRC7;
  } else if (song.system[0]==DIV_SYSTEM_NES && song.system[1]==DIV_SYSTEM_FDS) {
    w->writeC(systemToFileDMF(DIV_SYSTEM_NES_FDS));
    sys=DIV_SYSTEM_NES_FDS;
  } else {
    w->writeC(systemToFileDMF(song.system[0]));
    sys=song.system[0];
  }

  // song info
  w->writeString(song.name,true);
  w->writeString(song.author,true);
  w->writeC(curSubSong->hilightA);
  w->writeC(curSubSong->hilightB);

  int intHz=curSubSong->hz;
  
  w->writeC(curSubSong->timeBase);
  w->writeC(curSubSong->speeds.val[0]);
  w->writeC((curSubSong->speeds.len>=2)?curSubSong->speeds.val[1]:curSubSong->speeds.val[0]);
  w->writeC((intHz<=53)?0:1);
  w->writeC((intHz!=60 && intHz!=50));
  char customHz[4];
  memset(customHz,0,4);
  snprintf(customHz,4,"%d",(int)curSubSong->hz);
  w->write(customHz,3);
  w->writeI(curSubSong->patLen);
  w->writeC(curSubSong->ordersLen);

  for (int i=0; i<chans; i++) {
    for (int j=0; j<curSubSong->ordersLen; j++) {
      w->writeC(curOrders->ord[i][j]);
      if (version>=25) {
        DivPattern* pat=curPat[i].getPattern(j,false);
        w->writeString(pat->name,true);
      }
    }
  }

  if (song.subsong.size()>1) {
    addWarning("only the currently selected subsong will be saved");
  }

  if (!song.grooves.empty()) {
    addWarning("grooves will not be saved");
  }

  if (curSubSong->speeds.len>2) {
    addWarning("only the first two speeds will be effective");
  }

  if (curSubSong->virtualTempoD!=curSubSong->virtualTempoN) {
    addWarning(".dmf format does not support virtual tempo");
  }

  if (song.tuning<439.99 && song.tuning>440.01) {
    addWarning(".dmf format does not support tuning");
  }

  if (sys==DIV_SYSTEM_C64_6581 || sys==DIV_SYSTEM_C64_8580) {
    addWarning("absolute duty/cutoff macro not available in .dmf!");
    addWarning("duty precision will be lost");
  }

  for (DivInstrument* i: song.ins) {
    if (i->type==DIV_INS_AMIGA) {
      addWarning(".dmf format does not support arbitrary-pitch sample mode");
      break;
    }
  }

  for (DivInstrument* i: song.ins) {
    if (i->type==DIV_INS_FM || i->type==DIV_INS_OPM) {
      addWarning("no FM macros in .dmf format");
      break;
    }
  }

  w->writeC(song.ins.size());
  for (DivInstrument* i: song.ins) {
    w->writeString(i->name,true);

    // safety check
    if (!isFMSystem(sys) && i->type!=DIV_INS_STD && i->type!=DIV_INS_NES && i->type!=DIV_INS_FDS) {
      switch (song.system[0]) {
        case DIV_SYSTEM_GB:
          i->type=DIV_INS_GB;
          break;
        case DIV_SYSTEM_NES:
          i->type=DIV_INS_NES;
          break;
        case DIV_SYSTEM_C64_6581:
        case DIV_SYSTEM_C64_8580:
          i->type=DIV_INS_C64;
          break;
        case DIV_SYSTEM_PCE:
          i->type=DIV_INS_PCE;
          break;
        case DIV_SYSTEM_YM2610:
        case DIV_SYSTEM_YM2610_EXT:
          i->type=DIV_INS_AY;
          break;
        default:
          i->type=DIV_INS_STD;
          break;
      }
    }
    if (!isSTDSystem(sys) && i->type!=DIV_INS_FM && i->type!=DIV_INS_OPM) {
      if (sys==DIV_SYSTEM_ARCADE) {
        i->type=DIV_INS_OPM;
      } else {
        i->type=DIV_INS_FM;
      }
    }

    w->writeC((i->type==DIV_INS_FM || i->type==DIV_INS_OPM || i->type==DIV_INS_OPLL)?1:0);
    if (i->type==DIV_INS_FM || i->type==DIV_INS_OPM || i->type==DIV_INS_OPLL) { // FM
      w->writeC(i->fm.alg);
      w->writeC(i->fm.fb);
      w->writeC(i->fm.fms);
      w->writeC(i->fm.ams);

      for (int j=0; j<4; j++) {
        DivInstrumentFM::Operator& op=i->fm.op[j];
        w->writeC(op.am);
        w->writeC(op.ar);
        w->writeC(op.dr);
        w->writeC(op.mult);
        w->writeC(op.rr);
        w->writeC(op.sl);
        w->writeC(op.tl);
        if ((sys==DIV_SYSTEM_SMS_OPLL || sys==DIV_SYSTEM_NES_VRC7) && j==0) {
          w->writeC(i->fm.opllPreset);
        } else {
          w->writeC(op.dt2);
        }
        if (sys==DIV_SYSTEM_SMS_OPLL || sys==DIV_SYSTEM_NES_VRC7) {
          w->writeC(op.ksr);
          w->writeC(op.vib);
          w->writeC(op.ksl);
          w->writeC(op.ssgEnv);
        } else {
          w->writeC(op.rs);
          w->writeC(op.dt);
          w->writeC(op.d2r);
          w->writeC(op.ssgEnv);
        }
      }
    } else { // STD
      bool volIsCutoff=false;

      if (sys!=DIV_SYSTEM_GB) {
        int realVolMacroLen=i->std.volMacro.len;
        if (realVolMacroLen>127) realVolMacroLen=127;
        if (sys==DIV_SYSTEM_C64_6581 || sys==DIV_SYSTEM_C64_8580) {
          if (i->std.algMacro.len>0) volIsCutoff=true;
          if (volIsCutoff) {
            if (i->std.volMacro.len>0) {
              addWarning(".dmf only supports volume or cutoff macro in C64, but not both. volume macro will be lost.");
            }
            realVolMacroLen=i->std.algMacro.len;
            if (realVolMacroLen>127) realVolMacroLen=127;
            w->writeC(realVolMacroLen);
            for (int j=0; j<realVolMacroLen; j++) {
              w->writeI((-i->std.algMacro.val[j])+18);
            }
          } else {
            w->writeC(realVolMacroLen);
            for (int j=0; j<realVolMacroLen; j++) {
              w->writeI(i->std.volMacro.val[j]);
            }
          }
        } else {
          w->writeC(realVolMacroLen);
          for (int j=0; j<realVolMacroLen; j++) {
            w->writeI(i->std.volMacro.val[j]);
          }
        }
        if (realVolMacroLen>0) {
          if (volIsCutoff) {
            w->writeC(i->std.algMacro.loop);
          } else {
            w->writeC(i->std.volMacro.loop);
          }
        }
      }

      bool arpMacroMode=false;
      int arpMacroHowManyFixed=0;
      int realArpMacroLen=i->std.arpMacro.len;
      for (int j=0; j<i->std.arpMacro.len; j++) {
        if ((i->std.arpMacro.val[j]&0xc0000000)==0x40000000 || (i->std.arpMacro.val[j]&0xc0000000)==0x80000000) {
          arpMacroHowManyFixed++;
        }
      }
      if (arpMacroHowManyFixed>=i->std.arpMacro.len-1) {
        arpMacroMode=true;
      }
      if (i->std.arpMacro.len>0) {
        if (arpMacroMode && i->std.arpMacro.val[i->std.arpMacro.len-1]==0 && i->std.arpMacro.loop>=i->std.arpMacro.len) {
          realArpMacroLen--;
        }
      }

      if (realArpMacroLen>127) realArpMacroLen=127;

      w->writeC(realArpMacroLen);

      if (arpMacroMode) {
        for (int j=0; j<realArpMacroLen; j++) {
          if ((i->std.arpMacro.val[j]&0xc0000000)==0x40000000 || (i->std.arpMacro.val[j]&0xc0000000)==0x80000000) {
            w->writeI(i->std.arpMacro.val[j]^0x40000000);
          } else {
            w->writeI(i->std.arpMacro.val[j]);
          }
        }
      } else {
        for (int j=0; j<realArpMacroLen; j++) {
          if ((i->std.arpMacro.val[j]&0xc0000000)==0x40000000 || (i->std.arpMacro.val[j]&0xc0000000)==0x80000000) {
            w->writeI((i->std.arpMacro.val[j]^0x40000000)+12);
          } else {
            w->writeI(i->std.arpMacro.val[j]+12);
          }
        }
      }
      if (realArpMacroLen>0) {
        w->writeC(i->std.arpMacro.loop);
      }
      w->writeC(arpMacroMode);

      int realDutyMacroLen=i->std.dutyMacro.len;
      if (realDutyMacroLen>127) realDutyMacroLen=127;
      w->writeC(realDutyMacroLen);
      if (sys==DIV_SYSTEM_C64_6581 || sys==DIV_SYSTEM_C64_8580) {
        for (int j=0; j<realDutyMacroLen; j++) {
          w->writeI(i->std.dutyMacro.val[j]+12);
        }
      } else {
        for (int j=0; j<realDutyMacroLen; j++) {
          w->writeI(i->std.dutyMacro.val[j]);
        }
      }
      if (realDutyMacroLen>0) {
        w->writeC(i->std.dutyMacro.loop);
      }

      int realWaveMacroLen=i->std.waveMacro.len;
      if (realWaveMacroLen>127) realWaveMacroLen=127;
      w->writeC(realWaveMacroLen);
      for (int j=0; j<realWaveMacroLen; j++) {
        // piece of crap offset by 1
        if (song.system[0]==DIV_SYSTEM_YM2610 || song.system[0]==DIV_SYSTEM_YM2610_EXT) {
          w->writeI(i->std.waveMacro.val[j]-1);
        } else {
          w->writeI(i->std.waveMacro.val[j]);
        }
      }
      if (realWaveMacroLen>0) {
        w->writeC(i->std.waveMacro.loop);
      }

      if (sys==DIV_SYSTEM_C64_6581 || sys==DIV_SYSTEM_C64_8580) {
        w->writeC(i->c64.triOn);
        w->writeC(i->c64.sawOn);
        w->writeC(i->c64.pulseOn);
        w->writeC(i->c64.noiseOn);

        w->writeC(i->c64.a);
        w->writeC(i->c64.d);
        w->writeC(i->c64.s);
        w->writeC(i->c64.r);

        logW("duty and cutoff precision will be lost!");
        w->writeC((i->c64.duty*100)/4095);

        w->writeC(i->c64.ringMod);
        w->writeC(i->c64.oscSync);

        w->writeC(i->c64.toFilter);
        w->writeC(volIsCutoff);
        w->writeC(i->c64.initFilter);

        w->writeC(i->c64.res);
        w->writeC((i->c64.cut*100)/2047);
        w->writeC(i->c64.hp);
        w->writeC(i->c64.bp);
        w->writeC(i->c64.lp);
        w->writeC(i->c64.ch3off);
      }

      if (sys==DIV_SYSTEM_GB) {
        w->writeC(i->gb.envVol);
        w->writeC(i->gb.envDir);
        w->writeC(i->gb.envLen);
        w->writeC(i->gb.soundLen);
      }
    }
  }

  w->writeC(song.wave.size());
  for (DivWavetable* i: song.wave) {
    w->writeI(i->len);
    if (sys==DIV_SYSTEM_NES_FDS && version<26) {
      for (int j=0; j<i->len; j++) {
        w->writeI(i->data[j]>>2);
      }
    } else {
      for (int j=0; j<i->len; j++) {
        w->writeI(i->data[j]);
      }
    }
  }

  bool relWarning=false;

  for (int i=0; i<getChannelCount(sys); i++) {
    w->writeC(curPat[i].effectCols);

    for (int j=0; j<curSubSong->ordersLen; j++) {
      DivPattern* pat=curPat[i].getPattern(curOrders->ord[i][j],false);
      for (int k=0; k<curSubSong->patLen; k++) {
        if ((pat->data[k][0]==101 || pat->data[k][0]==102) && pat->data[k][1]==0) {
          w->writeS(100);
          w->writeS(0);
          if (!relWarning) {
            relWarning=true;
            addWarning("note/macro release will be converted to note off!");
          }
        } else {
          w->writeS(pat->data[k][0]); // note
          w->writeS(pat->data[k][1]); // octave
        }
        w->writeS(pat->data[k][3]); // volume
#ifdef TA_BIG_ENDIAN
        for (int l=0; l<curPat[i].effectCols*2; l++) {
          w->writeS(pat->data[k][4+l]);
        }
#else
        w->write(&pat->data[k][4],2*curPat[i].effectCols*2); // effects
#endif
        w->writeS(pat->data[k][2]); // instrument
      }
    }
  }

  if (song.sample.size()>0) {
    addWarning("samples' rates will be rounded to nearest compatible value");
  }

  w->writeC(song.sample.size());
  for (DivSample* i: song.sample) {
    w->writeI(i->samples);
    w->writeString(i->name,true);
    w->writeC(divToFileRate(i->rate));
    w->writeC(5);
    w->writeC(50);
    // i'm too lazy to deal with .dmf's weird way of storing 8-bit samples
    w->writeC(16);
    // well I can't be lazy if it's on a big-endian system
#ifdef TA_BIG_ENDIAN
    for (unsigned int j=0; j<i->length16; j++) {
      w->writeC(((unsigned short)i->data16[j])&0xff);
      w->writeC(((unsigned short)i->data16[j])>>8);
    }
#else
    w->write(i->data16,i->length16);
#endif
  }
  
  saveLock.unlock();
  return w;
}

static const char* trueFalse[2]={
  "no", "yes"
};

static const char* gbEnvDir[2]={
  "down", "up"
};

static const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

static const char* notesNegative[12]={
  "c_", "c+", "d_", "d+", "e_", "f_", "f+", "g_", "g+", "a_", "a+", "b_"
};

static const char* sampleLoopModes[4]={
  "forward", "backward", "ping-pong", "invalid"
};

void writeTextMacro(SafeWriter* w, DivInstrumentMacro& m, const char* name, bool& wroteMacroHeader) {
  if ((m.open&6)==0 && m.len<1) return;
  if (!wroteMacroHeader) {
    w->writeText("- macros:\n");
    wroteMacroHeader=true;
  }
  w->writeText(fmt::sprintf("  - %s:",name));
  int len=m.len;
  switch (m.open&6) {
    case 2:
      len=16;
      w->writeText(" [ADSR]");
      break;
    case 4:
      len=16;
      w->writeText(" [LFO]");
      break;
  }
  if (m.mode) {
    w->writeText(fmt::sprintf(" [MODE %d]",m.mode));
  }
  if (m.delay>0) {
    w->writeText(fmt::sprintf(" [DELAY %d]",m.delay));
  }
  if (m.speed>1) {
    w->writeText(fmt::sprintf(" [SPEED %d]",m.speed));
  }
  for (int i=0; i<len; i++) {
    if (i==m.loop) {
      w->writeText(" |");
    }
    if (i==m.rel) {
      w->writeText(" /");
    }
    w->writeText(fmt::sprintf(" %d",m.val[i]));
  }
  w->writeText("\n");
}

SafeWriter* DivEngine::saveText(bool separatePatterns) {
  saveLock.lock();

  SafeWriter* w=new SafeWriter;
  w->init();

  w->writeText(fmt::sprintf("# Furnace Text Export\n\ngenerated by Furnace %s (%d)\n\n# Song Information\n\n",DIV_VERSION,DIV_ENGINE_VERSION));
  w->writeText(fmt::sprintf("- name: %s\n",song.name));
  w->writeText(fmt::sprintf("- author: %s\n",song.author));
  w->writeText(fmt::sprintf("- album: %s\n",song.category));
  w->writeText(fmt::sprintf("- system: %s\n",song.systemName));
  w->writeText(fmt::sprintf("- tuning: %g\n\n",song.tuning));
  
  w->writeText(fmt::sprintf("- instruments: %d\n",song.insLen));
  w->writeText(fmt::sprintf("- wavetables: %d\n",song.waveLen));
  w->writeText(fmt::sprintf("- samples: %d\n\n",song.sampleLen));
  
  w->writeText("# Sound Chips\n\n");

  for (int i=0; i<song.systemLen; i++) {
    w->writeText(fmt::sprintf("- %s\n",getSystemName(song.system[i])));
    w->writeText(fmt::sprintf("  - id: %.2X\n",(int)song.system[i]));
    w->writeText(fmt::sprintf("  - volume: %g\n",song.systemVol[i]));
    w->writeText(fmt::sprintf("  - panning: %g\n",song.systemPan[i]));
    w->writeText(fmt::sprintf("  - front/rear: %g\n",song.systemPanFR[i]));
    
    String sysFlags=song.systemFlags[i].toString();

    if (!sysFlags.empty()) {
      w->writeText(fmt::sprintf("  - flags:\n```\n%s\n```\n",sysFlags));
    }
  }

  if (!song.notes.empty()) {
    w->writeText("\n# Song Comments\n\n");
    w->writeText(song.notes);
  }

  w->writeText("\n# Instruments\n\n");

  for (int i=0; i<song.insLen; i++) {
    DivInstrument* ins=song.ins[i];

    w->writeText(fmt::sprintf("## %.2X: %s\n\n",i,ins->name));

    w->writeText(fmt::sprintf("- type: %d\n",(int)ins->type));

    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPM || ins->type==DIV_INS_ESFM) {
      w->writeText("- FM parameters:\n");
      w->writeText(fmt::sprintf("  - ALG: %d\n",ins->fm.alg));
      w->writeText(fmt::sprintf("  - FB: %d\n",ins->fm.fb));
      w->writeText(fmt::sprintf("  - FMS: %d\n",ins->fm.fms));
      w->writeText(fmt::sprintf("  - AMS: %d\n",ins->fm.ams));
      w->writeText(fmt::sprintf("  - FMS2: %d\n",ins->fm.fms2));
      w->writeText(fmt::sprintf("  - AMS2: %d\n",ins->fm.ams2));
      w->writeText(fmt::sprintf("  - operators: %d\n",ins->fm.ops));
      w->writeText(fmt::sprintf("  - OPLL patch: %d\n",ins->fm.opllPreset));
      w->writeText(fmt::sprintf("  - fixed drum freq: %s\n",trueFalse[ins->fm.fixedDrums?1:0]));
      w->writeText(fmt::sprintf("  - kick freq: %.4X\n",ins->fm.kickFreq));
      w->writeText(fmt::sprintf("  - snare/hat freq: %.4X\n",ins->fm.snareHatFreq));
      w->writeText(fmt::sprintf("  - tom/top freq: %.4X\n",ins->fm.tomTopFreq));
      
      for (int j=0; j<ins->fm.ops; j++) {
        DivInstrumentFM::Operator& op=ins->fm.op[j];

        w->writeText(fmt::sprintf("  - operator %d:\n",j));
        w->writeText(fmt::sprintf("    - enabled: %s\n",trueFalse[op.enable?1:0]));
        w->writeText(fmt::sprintf("    - AM: %d\n",op.am));
        w->writeText(fmt::sprintf("    - AR: %d\n",op.ar));
        w->writeText(fmt::sprintf("    - DR: %d\n",op.dr));
        w->writeText(fmt::sprintf("    - MULT: %d\n",op.mult));
        w->writeText(fmt::sprintf("    - RR: %d\n",op.rr));
        w->writeText(fmt::sprintf("    - SL: %d\n",op.sl));
        w->writeText(fmt::sprintf("    - TL: %d\n",op.tl));
        w->writeText(fmt::sprintf("    - DT2: %d\n",op.dt2));
        w->writeText(fmt::sprintf("    - RS: %d\n",op.rs));
        w->writeText(fmt::sprintf("    - DT: %d\n",op.dt));
        w->writeText(fmt::sprintf("    - D2R: %d\n",op.d2r));
        w->writeText(fmt::sprintf("    - SSG-EG: %d\n",op.ssgEnv));
        w->writeText(fmt::sprintf("    - DAM: %d\n",op.dam));
        w->writeText(fmt::sprintf("    - DVB: %d\n",op.dvb));
        w->writeText(fmt::sprintf("    - EGT: %d\n",op.egt));
        w->writeText(fmt::sprintf("    - KSL: %d\n",op.ksl));
        w->writeText(fmt::sprintf("    - SUS: %d\n",op.sus));
        w->writeText(fmt::sprintf("    - VIB: %d\n",op.vib));
        w->writeText(fmt::sprintf("    - WS: %d\n",op.ws));
        w->writeText(fmt::sprintf("    - KSR: %d\n",op.ksr));
        w->writeText(fmt::sprintf("    - TL volume scale: %d\n",op.kvs));
      }
    }

    if (ins->type==DIV_INS_ESFM) {
      w->writeText("- ESFM parameters:\n");
      w->writeText(fmt::sprintf("  - noise mode: %d\n",ins->esfm.noise));

      for (int j=0; j<ins->fm.ops; j++) {
        DivInstrumentESFM::Operator& opE=ins->esfm.op[j];

        w->writeText(fmt::sprintf("  - operator %d:\n",j));
        w->writeText(fmt::sprintf("    - DL: %d\n",opE.delay));
        w->writeText(fmt::sprintf("    - OL: %d\n",opE.outLvl));
        w->writeText(fmt::sprintf("    - MI: %d\n",opE.modIn));
        w->writeText(fmt::sprintf("    - output left: %s\n",trueFalse[opE.left?1:0]));
        w->writeText(fmt::sprintf("    - output right: %s\n",trueFalse[opE.right?1:0]));
        w->writeText(fmt::sprintf("    - CT: %d\n",opE.ct));
        w->writeText(fmt::sprintf("    - DT: %d\n",opE.dt));
        w->writeText(fmt::sprintf("    - fixed frequency: %s\n",trueFalse[opE.fixed?1:0]));
      }
    }

    if (ins->type==DIV_INS_GB) {
      w->writeText("- Game Boy parameters:\n");
      w->writeText(fmt::sprintf("  - volume: %d\n",ins->gb.envVol));
      w->writeText(fmt::sprintf("  - direction: %s\n",gbEnvDir[ins->gb.envDir?1:0]));
      w->writeText(fmt::sprintf("  - length: %d\n",ins->gb.envLen));
      w->writeText(fmt::sprintf("  - sound length: %d\n",ins->gb.soundLen));
      w->writeText(fmt::sprintf("  - use software envelope: %s\n",trueFalse[ins->gb.softEnv?1:0]));
      w->writeText(fmt::sprintf("  - always initialize: %s\n",trueFalse[ins->gb.softEnv?1:0]));
      if (ins->gb.hwSeqLen>0) {
        w->writeText("  - hardware sequence:\n");
        for (int j=0; j<ins->gb.hwSeqLen; j++) {
          w->writeText(fmt::sprintf("    - %d: %.2X %.4X\n",j,ins->gb.hwSeq[j].cmd,ins->gb.hwSeq[j].data));
        }
      }
    }

    bool header=false;
    writeTextMacro(w,ins->std.volMacro,"vol",header);
    writeTextMacro(w,ins->std.arpMacro,"arp",header);
    writeTextMacro(w,ins->std.dutyMacro,"duty",header);
    writeTextMacro(w,ins->std.waveMacro,"wave",header);
    writeTextMacro(w,ins->std.pitchMacro,"pitch",header);
    writeTextMacro(w,ins->std.panLMacro,"panL",header);
    writeTextMacro(w,ins->std.panRMacro,"panR",header);
    writeTextMacro(w,ins->std.phaseResetMacro,"phaseReset",header);
    writeTextMacro(w,ins->std.ex1Macro,"ex1",header);
    writeTextMacro(w,ins->std.ex2Macro,"ex2",header);
    writeTextMacro(w,ins->std.ex3Macro,"ex3",header);
    writeTextMacro(w,ins->std.ex4Macro,"ex4",header);
    writeTextMacro(w,ins->std.ex5Macro,"ex5",header);
    writeTextMacro(w,ins->std.ex6Macro,"ex6",header);
    writeTextMacro(w,ins->std.ex7Macro,"ex7",header);
    writeTextMacro(w,ins->std.ex8Macro,"ex8",header);
    writeTextMacro(w,ins->std.algMacro,"alg",header);
    writeTextMacro(w,ins->std.fbMacro,"fb",header);
    writeTextMacro(w,ins->std.fmsMacro,"fms",header);
    writeTextMacro(w,ins->std.amsMacro,"ams",header);

    // TODO: the rest
    w->writeText("\n");
  }

  w->writeText("\n# Wavetables\n\n");

  for (int i=0; i<song.waveLen; i++) {
    DivWavetable* wave=song.wave[i];

    w->writeText(fmt::sprintf("- %d (%dx%d):",i,wave->len+1,wave->max+1));
    for (int j=0; j<=wave->len; j++) {
      w->writeText(fmt::sprintf(" %d",wave->data[j]));
    }
    w->writeText("\n");
  }

  w->writeText("\n# Samples\n\n");

  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];

    w->writeText(fmt::sprintf("## %.2X: %s\n\n",i,sample->name));

    w->writeText(fmt::sprintf("- format: %d\n",(int)sample->depth));
    w->writeText(fmt::sprintf("- data length: %d\n",sample->getCurBufLen()));
    w->writeText(fmt::sprintf("- samples: %d\n",sample->samples));
    w->writeText(fmt::sprintf("- rate: %d\n",sample->centerRate));
    w->writeText(fmt::sprintf("- compat rate: %d\n",sample->rate));
    w->writeText(fmt::sprintf("- loop: %s\n",trueFalse[sample->loop?1:0]));
    if (sample->loop) {
      w->writeText(fmt::sprintf("  - start: %d\n",sample->loopStart));
      w->writeText(fmt::sprintf("  - end: %d\n",sample->loopEnd));
      w->writeText(fmt::sprintf("  - mode: %s\n",sampleLoopModes[sample->loopMode&3]));
    }
    w->writeText(fmt::sprintf("- BRR emphasis: %s\n",trueFalse[sample->brrEmphasis?1:0]));
    w->writeText(fmt::sprintf("- dither: %s\n",trueFalse[sample->dither?1:0]));

    // TODO' render matrix
    unsigned char* buf=(unsigned char*)sample->getCurBuf();
    unsigned int bufLen=sample->getCurBufLen();
    w->writeText("\n```");
    for (unsigned int i=0; i<bufLen; i++) {
      if ((i&15)==0) w->writeText(fmt::sprintf("\n%.8X:",i));
      w->writeText(fmt::sprintf(" %.2X",buf[i]));
    }
    w->writeText("\n```\n\n");
  }

  w->writeText("\n# Subsongs\n\n");

  for (size_t i=0; i<song.subsong.size(); i++) {
    DivSubSong* s=song.subsong[i];
    w->writeText(fmt::sprintf("## %d: %s\n\n",(int)i,s->name));

    w->writeText(fmt::sprintf("- tick rate: %g\n",s->hz));
    w->writeText(fmt::sprintf("- speeds:"));
    for (int j=0; j<s->speeds.len; j++) {
      w->writeText(fmt::sprintf(" %d",s->speeds.val[j]));
    }
    w->writeText("\n");
    w->writeText(fmt::sprintf("- virtual tempo: %d/%d\n",s->virtualTempoN,s->virtualTempoD));
    w->writeText(fmt::sprintf("- time base: %d\n",s->timeBase));
    w->writeText(fmt::sprintf("- pattern length: %d\n",s->patLen));
    w->writeText(fmt::sprintf("\norders:\n```\n"));

    for (int j=0; j<s->ordersLen; j++) {
      w->writeText(fmt::sprintf("%.2X |",j));
      for (int k=0; k<chans; k++) {
        w->writeText(fmt::sprintf(" %.2X",s->orders.ord[k][j]));
      }
      w->writeText("\n");
    }
    w->writeText("```\n\n## Patterns\n\n");

    if (separatePatterns) {
      w->writeText("TODO: separate patterns\n\n");
    } else {
      for (int j=0; j<s->ordersLen; j++) {
        w->writeText(fmt::sprintf("----- ORDER %.2X\n",j));

        for (int k=0; k<s->patLen; k++) {
          w->writeText(fmt::sprintf("%.2X ",k));

          for (int l=0; l<chans; l++) {
            DivPattern* p=s->pat[l].getPattern(s->orders.ord[l][j],false);

            int note=p->data[k][0];
            int octave=p->data[k][1];

            if (note==0 && octave==0) {
              w->writeText("|... ");
            } else if (note==100) {
              w->writeText("|OFF ");
            } else if (note==101) {
              w->writeText("|=== ");
            } else if (note==102) {
              w->writeText("|REL ");
            } else if ((octave>9 && octave<250) || note>12) {
              w->writeText("|??? ");
            } else {
              if (octave>=128) octave-=256;
              if (note>11) {
                note-=12;
                octave++;
              }
              w->writeText(fmt::sprintf("|%s%d ",(octave<0)?notesNegative[note]:notes[note],(octave<0)?(-octave):octave));
            }

            if (p->data[k][2]==-1) {
              w->writeText(".. ");
            } else {
              w->writeText(fmt::sprintf("%.2X ",p->data[k][2]&0xff));
            }

            if (p->data[k][3]==-1) {
              w->writeText("..");
            } else {
              w->writeText(fmt::sprintf("%.2X",p->data[k][3]&0xff));
            }

            for (int m=0; m<s->pat[l].effectCols; m++) {
              if (p->data[k][4+(m<<1)]==-1) {
                w->writeText(" ..");
              } else {
                w->writeText(fmt::sprintf(" %.2X",p->data[k][4+(m<<1)]&0xff));
              }
              if (p->data[k][5+(m<<1)]==-1) {
                w->writeText("..");
              } else {
                w->writeText(fmt::sprintf("%.2X",p->data[k][5+(m<<1)]&0xff));
              }
            }
          }

          w->writeText("\n");
        }
      }
    }

  }

  saveLock.unlock();
  return w;
}
