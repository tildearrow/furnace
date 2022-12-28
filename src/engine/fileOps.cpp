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
#include "instrument.h"
#include "song.h"
#include <zlib.h>
#include <fmt/printf.h>

#define DIV_READ_SIZE 131072
#define DIV_DMF_MAGIC ".DelekDefleMask."
#define DIV_FUR_MAGIC "-Furnace module-"
#define DIV_FTM_MAGIC "FamiTracker Module"
#define DIV_FC13_MAGIC "SMOD"
#define DIV_FC14_MAGIC "FC14"

struct InflateBlock {
  unsigned char* buf;
  size_t len;
  size_t blockSize;
  InflateBlock(size_t s) {
    buf=new unsigned char[s];
    len=s;
    blockSize=0;
  }
  ~InflateBlock() {
    delete[] buf;
    len=0;
  }
};

struct NotZlibException {
  int what;
  NotZlibException(int w):
    what(w) {}
};

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
    if (ds.version>0x1a) {
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

    logI("reading module data...");
    if (ds.version>0x0c) {
      ds.subsong[0]->hilightA=reader.readC();
      ds.subsong[0]->hilightB=reader.readC();
    }

    ds.subsong[0]->timeBase=reader.readC();
    ds.subsong[0]->speed1=reader.readC();
    if (ds.version>0x07) {
      ds.subsong[0]->speed2=reader.readC();
      ds.subsong[0]->pal=reader.readC();
      ds.subsong[0]->hz=(ds.subsong[0]->pal)?60:50;
      ds.subsong[0]->customTempo=reader.readC();
    } else {
      ds.subsong[0]->speed2=ds.subsong[0]->speed1;
    }
    if (ds.version>0x0a) {
      String hz=reader.readString(3);
      if (ds.subsong[0]->customTempo) {
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
      ds.subsong[0]->customTempo=true;
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
        }
        if (ins->std.waveMacro.len>0) {
          ins->std.waveMacro.open=true;
          ins->std.waveMacro.loop=reader.readC();
        } else {
          ins->std.waveMacro.open=false;
        }

        if (ds.system[0]==DIV_SYSTEM_C64_6581 || ds.system[0]==DIV_SYSTEM_C64_8580) {
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
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;
      int length=reader.readI();
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
        pitch=reader.readC();
        vol=reader.readC();
      }
      if (ds.version<=0x08) {
        sample->rate=ymuSampleRate*400;
      }
      if (ds.version>0x15) {
        sample->depth=(DivSampleDepth)reader.readC();
        if (sample->depth!=DIV_SAMPLE_DEPTH_8BIT && sample->depth!=DIV_SAMPLE_DEPTH_16BIT) {
          logW("%d: sample depth is wrong! (%d)",i,sample->depth);
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

          if (pitch!=5) {
            logD("%d: scaling from %d...",i,pitch);
          }

          // render data
          if (!sample->init((double)length/samplePitches[pitch])) {
            logE("%d: error while initializing sample!",i);
          }

          unsigned int k=0;
          float mult=(float)(vol)/50.0f;
          for (double j=0; j<length; j+=samplePitches[pitch]) {
            if (k>=sample->samples) {
              break;
            }
            if (sample->depth==DIV_SAMPLE_DEPTH_8BIT) {
              float next=(float)(data[(unsigned int)j]-0x80)*mult;
              sample->data8[k++]=fmin(fmax(next,-128),127);
            } else {
              float next=(float)data[(unsigned int)j]*mult;
              sample->data16[k++]=fmin(fmax(next,-32768),32767);
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
      ds.systemVol[1]=32;
    }
    if (ds.system[0]==DIV_SYSTEM_GENESIS_EXT) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_YM2612_EXT;
      ds.system[1]=DIV_SYSTEM_SMS;
      ds.systemVol[1]=32;
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

void DivEngine::convertOldFlags(unsigned int oldFlags, DivConfig& newFlags, DivSystem sys) {
  newFlags.clear();

  switch (sys) {
    case DIV_SYSTEM_SMS:
      switch (oldFlags&0xff03) {
        case 0x0000:
          newFlags.set("clockSel",0);
          break;
        case 0x0001:
          newFlags.set("clockSel",1);
          break;
        case 0x0002:
          newFlags.set("clockSel",2);
          break;
        case 0x0003:
          newFlags.set("clockSel",3);
          break;
        case 0x0100:
          newFlags.set("clockSel",4);
          break;
        case 0x0101:
          newFlags.set("clockSel",5);
          break;
        case 0x0102:
          newFlags.set("clockSel",6);
          break;
      }
      switch (oldFlags&0xcc) {
        case 0x00:
          newFlags.set("chipType",0);
          break;
        case 0x04:
          newFlags.set("chipType",1);
          break;
        case 0x08:
          newFlags.set("chipType",2);
          break;
        case 0x0c:
          newFlags.set("chipType",3);
          break;
        case 0x40:
          newFlags.set("chipType",4);
          break;
        case 0x44:
          newFlags.set("chipType",5);
          break;
        case 0x48:
          newFlags.set("chipType",6);
          break;
        case 0x4c:
          newFlags.set("chipType",7);
          break;
        case 0x80:
          newFlags.set("chipType",8);
          break;
        case 0x84:
          newFlags.set("chipType",9);
          break;
      }
      if (oldFlags&16) newFlags.set("noPhaseReset",true);
      break;
    case DIV_SYSTEM_GB:
      newFlags.set("chipType",(int)(oldFlags&3));
      if (oldFlags&8) newFlags.set("noAntiClick",true);
      break;
    case DIV_SYSTEM_PCE:
      newFlags.set("clockSel",(int)(oldFlags&1));
      newFlags.set("chipType",(oldFlags&4)?1:0);
      if (oldFlags&8) newFlags.set("noAntiClick",true);
      break;
    case DIV_SYSTEM_NES:
    case DIV_SYSTEM_VRC6:
    case DIV_SYSTEM_FDS:
    case DIV_SYSTEM_MMC5:
    case DIV_SYSTEM_SAA1099:
    case DIV_SYSTEM_OPZ:
      switch (oldFlags) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_C64_6581:
    case DIV_SYSTEM_C64_8580:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_YM2610:
    case DIV_SYSTEM_YM2610_EXT:
    case DIV_SYSTEM_YM2610_FULL:
    case DIV_SYSTEM_YM2610_FULL_EXT:
    case DIV_SYSTEM_YM2610B:
    case DIV_SYSTEM_YM2610B_EXT:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
      }
      break;
    case DIV_SYSTEM_AY8910:
    case DIV_SYSTEM_AY8930:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
        case 6:
          newFlags.set("clockSel",6);
          break;
        case 7:
          newFlags.set("clockSel",7);
          break;
        case 8:
          newFlags.set("clockSel",8);
          break;
        case 9:
          newFlags.set("clockSel",9);
          break;
        case 10:
          newFlags.set("clockSel",10);
          break;
        case 11:
          newFlags.set("clockSel",11);
          break;
        case 12:
          newFlags.set("clockSel",12);
          break;
        case 13:
          if (sys==DIV_SYSTEM_AY8910) newFlags.set("clockSel",13);
          break;
        case 14:
          if (sys==DIV_SYSTEM_AY8910) newFlags.set("clockSel",14);
          break;
      }
      if (sys==DIV_SYSTEM_AY8910) switch ((oldFlags>>4)&3) {
        case 0:
          newFlags.set("chipType",0);
          break;
        case 1:
          newFlags.set("chipType",1);
          break;
        case 2:
          newFlags.set("chipType",2);
          break;
        case 3:
          newFlags.set("chipType",3);
          break;
      }
      if (oldFlags&64) newFlags.set("stereo",true);
      if (oldFlags&128) newFlags.set("halfClock",true);
      newFlags.set("stereoSep",(int)((oldFlags>>8)&255));
      break;
    case DIV_SYSTEM_AMIGA:
      if (oldFlags&1) newFlags.set("clockSel",1);
      if (oldFlags&2) newFlags.set("chipType",1);
      if (oldFlags&4) newFlags.set("bypassLimits",true);
      newFlags.set("stereoSep",(int)((oldFlags>>8)&127));
      break;
    case DIV_SYSTEM_YM2151:
      switch (oldFlags&255) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_YM2612:
    case DIV_SYSTEM_YM2612_EXT:
    case DIV_SYSTEM_YM2612_DUALPCM:
    case DIV_SYSTEM_YM2612_DUALPCM_EXT:
      switch (oldFlags&0x7fffffff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
      }
      if (oldFlags&0x80000000) newFlags.set("ladderEffect",true);
      break;
    case DIV_SYSTEM_TIA:
      newFlags.set("clockSel",(int)(oldFlags&1));
      switch ((oldFlags>>1)&3) {
        case 0:
          newFlags.set("mixingType",0);
          break;
        case 1:
          newFlags.set("mixingType",1);
          break;
        case 2:
          newFlags.set("mixingType",2);
          break;
      }
      break;
    case DIV_SYSTEM_VIC20:
      newFlags.set("clockSel",(int)(oldFlags&1));
      break;
    case DIV_SYSTEM_SNES:
      newFlags.set("volScaleL",(int)(oldFlags&127));
      newFlags.set("volScaleR",(int)((oldFlags>>8)&127));
      break;
    case DIV_SYSTEM_OPLL:
    case DIV_SYSTEM_OPLL_DRUMS:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      switch (oldFlags>>4) {
        case 0:
          newFlags.set("patchSet",0);
          break;
        case 1:
          newFlags.set("patchSet",1);
          break;
        case 2:
          newFlags.set("patchSet",2);
          break;
        case 3:
          newFlags.set("patchSet",3);
          break;
      }
      break;
    case DIV_SYSTEM_N163:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      newFlags.set("channels",(int)((oldFlags>>4)&7));
      if (oldFlags&128) newFlags.set("multiplex",true);
      break;
    case DIV_SYSTEM_YM2203:
    case DIV_SYSTEM_YM2203_EXT:
      switch (oldFlags&31) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
      }
      switch ((oldFlags>>5)&3) {
        case 0:
          newFlags.set("prescale",0);
          break;
        case 1:
          newFlags.set("prescale",1);
          break;
        case 2:
          newFlags.set("prescale",2);
          break;
      }
      break;
    case DIV_SYSTEM_YM2608:
    case DIV_SYSTEM_YM2608_EXT:
      switch (oldFlags&31) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
      }
      switch ((oldFlags>>5)&3) {
        case 0:
          newFlags.set("prescale",0);
          break;
        case 1:
          newFlags.set("prescale",1);
          break;
        case 2:
          newFlags.set("prescale",2);
          break;
      }
      break;
    case DIV_SYSTEM_OPL:
    case DIV_SYSTEM_OPL2:
    case DIV_SYSTEM_Y8950:
    case DIV_SYSTEM_OPL_DRUMS:
    case DIV_SYSTEM_OPL2_DRUMS:
    case DIV_SYSTEM_Y8950_DRUMS:
    case DIV_SYSTEM_YMZ280B:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
      }
      break;
    case DIV_SYSTEM_OPL3:
    case DIV_SYSTEM_OPL3_DRUMS:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
      }
      break;
    case DIV_SYSTEM_PCSPKR:
      newFlags.set("speakerType",(int)(oldFlags&3));
      break;
    case DIV_SYSTEM_RF5C68:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      switch (oldFlags>>4) {
        case 0:
          newFlags.set("chipType",0);
          break;
        case 1:
          newFlags.set("chipType",1);
          break;
      }
      break;
    case DIV_SYSTEM_VRC7:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      break;
    case DIV_SYSTEM_SFX_BEEPER:
      newFlags.set("clockSel",(int)(oldFlags&1));
      break;
    case DIV_SYSTEM_SCC:
    case DIV_SYSTEM_SCC_PLUS:
      switch (oldFlags&63) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      break;
    case DIV_SYSTEM_MSM6295:
      switch (oldFlags&63) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
        case 4:
          newFlags.set("clockSel",4);
          break;
        case 5:
          newFlags.set("clockSel",5);
          break;
        case 6:
          newFlags.set("clockSel",6);
          break;
        case 7:
          newFlags.set("clockSel",7);
          break;
        case 8:
          newFlags.set("clockSel",8);
          break;
        case 9:
          newFlags.set("clockSel",9);
          break;
        case 10:
          newFlags.set("clockSel",10);
          break;
        case 11:
          newFlags.set("clockSel",11);
          break;
        case 12:
          newFlags.set("clockSel",12);
          break;
        case 13:
          newFlags.set("clockSel",13);
          break;
        case 14:
          newFlags.set("clockSel",14);
          break;
      }
      if (oldFlags&128) newFlags.set("rateSel",true);
      break;
    case DIV_SYSTEM_MSM6258:
      switch (oldFlags) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
        case 3:
          newFlags.set("clockSel",3);
          break;
      }
      break;
    case DIV_SYSTEM_OPL4:
    case DIV_SYSTEM_OPL4_DRUMS:
      switch (oldFlags&0xff) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
        case 2:
          newFlags.set("clockSel",2);
          break;
      }
      break;
    case DIV_SYSTEM_X1_010:
      switch (oldFlags&15) {
        case 0:
          newFlags.set("clockSel",0);
          break;
        case 1:
          newFlags.set("clockSel",1);
          break;
      }
      if (oldFlags&16) newFlags.set("stereo",true);
      break;
    case DIV_SYSTEM_SOUND_UNIT:
      newFlags.set("clockSel",(int)(oldFlags&1));
      if (oldFlags&4) newFlags.set("echo",true);
      if (oldFlags&8) newFlags.set("swapEcho",true);
      newFlags.set("sampleMemSize",(int)((oldFlags>>4)&1));
      if (oldFlags&32) newFlags.set("pdm",true);
      newFlags.set("echoDelay",(int)((oldFlags>>8)&63));
      newFlags.set("echoFeedback",(int)((oldFlags>>16)&15));
      newFlags.set("echoResolution",(int)((oldFlags>>20)&15));
      newFlags.set("echoVol",(int)((oldFlags>>24)&255));
      break;
    case DIV_SYSTEM_PCM_DAC:
      if (!oldFlags) oldFlags=0x1f0000|44099;
      newFlags.set("rate",(int)((oldFlags&0xffff)+1));
      newFlags.set("outDepth",(int)((oldFlags>>16)&15));
      if (oldFlags&0x100000) newFlags.set("stereo",true);
      break;
    case DIV_SYSTEM_QSOUND:
      newFlags.set("echoDelay",(int)(oldFlags&0xfff));
      newFlags.set("echoFeedback",(int)((oldFlags>>12)&255));
      break;
    default:
      break;
  }
}

bool DivEngine::loadFur(unsigned char* file, size_t len) {
  unsigned int insPtr[256];
  unsigned int wavePtr[256];
  unsigned int samplePtr[256];
  unsigned int subSongPtr[256];
  unsigned int sysFlagsPtr[DIV_MAX_CHIPS];
  std::vector<int> patPtr;
  int numberOfSubSongs=0;
  char magic[5];
  memset(magic,0,5);
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;
    DivSubSong* subSong=ds.subsong[0];

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=reader.readS();
    logI("module version %d (0x%.2x)",ds.version,ds.version);

    if (ds.version>DIV_ENGINE_VERSION) {
      logW("this module was created with a more recent version of Furnace!");
      addWarning("this module was created with a more recent version of Furnace!");
    }

    if (ds.version<37) { // compat flags not stored back then
      ds.limitSlides=true;
      ds.linearPitch=1;
      ds.loopModality=0;
    }
    if (ds.version<43) {
      ds.properNoiseLayout=false;
      ds.waveDutyIsVol=false;
    }
    if (ds.version<45) {
      ds.resetMacroOnPorta=true;
      ds.legacyVolumeSlides=true;
      ds.compatibleArpeggio=true;
      ds.noteOffResetsSlides=true;
      ds.targetResetsSlides=true;
    }
    if (ds.version<46) {
      ds.arpNonPorta=true;
      ds.algMacroBehavior=true;
    } else {
      ds.arpNonPorta=false;
      ds.algMacroBehavior=false;
    }
    if (ds.version<49) {
      ds.brokenShortcutSlides=true;
    }
    if (ds.version<50) {
      ds.ignoreDuplicateSlides=false;
    }
    if (ds.version<62) {
      ds.stopPortaOnNoteOff=true;
    }
    if (ds.version<64) {
      ds.brokenDACMode=false;
    }
    if (ds.version<65) {
      ds.oneTickCut=false;
    }
    if (ds.version<66) {
      ds.newInsTriggersInPorta=false;
    }
    if (ds.version<69) {
      ds.arp0Reset=false;
    }
    if (ds.version<71) {
      ds.noSlidesOnFirstTick=false;
      ds.rowResetsArpPos=false;
      ds.ignoreJumpAtEnd=true;
    }
    if (ds.version<72) {
      ds.buggyPortaAfterSlide=true;
      ds.gbInsAffectsEnvelope=false;
    }
    if (ds.version<78) {
      ds.sharedExtStat=false;
    }
    if (ds.version<83) {
      ds.ignoreDACModeOutsideIntendedChannel=true;
      ds.e1e2AlsoTakePriority=false;
    }
    if (ds.version<84) {
      ds.newSegaPCM=false;
    }
    if (ds.version<85) {
      ds.fbPortaPause=true;
    }
    if (ds.version<86) {
      ds.snDutyReset=true;
    }
    if (ds.version<90) {
      ds.pitchMacroIsLinear=false;
    }
    if (ds.version<97) {
      ds.oldOctaveBoundary=true;
    }
    if (ds.version<97) { // actually should be 98 but yky uses this feature ahead of time
      ds.noOPN2Vol=true;
    }
    if (ds.version<99) {
      ds.newVolumeScaling=false;
      ds.volMacroLinger=false;
      ds.brokenOutVol=true;
    }
    if (ds.version<100) {
      ds.e1e2StopOnSameNote=false;
    }
    if (ds.version<101) {
      ds.brokenPortaArp=true;
    }
    if (ds.version<108) {
      ds.snNoLowPeriods=true;
    }
    if (ds.version<110) {
      ds.delayBehavior=1;
    }
    if (ds.version<113) {
      ds.jumpTreatment=1;
    }
    if (ds.version<115) {
      ds.autoSystem=false;
    }
    if (ds.version<117) {
      ds.disableSampleMacro=true;
    }
    if (ds.version<121) {
      ds.brokenOutVol2=false;
    }
    if (ds.version<130) {
      ds.oldArpStrategy=true;
    }
    ds.isDMF=false;

    reader.readS(); // reserved
    int infoSeek=reader.readI();

    if (!reader.seek(infoSeek,SEEK_SET)) {
      logE("couldn't seek to info header at %d!",infoSeek);
      lastError="couldn't seek to info header!";
      delete[] file;
      return false;
    }

    // read header
    reader.read(magic,4);
    if (strcmp(magic,"INFO")!=0) {
      logE("invalid info header!");
      lastError="invalid info header!";
      delete[] file;
      return false;
    }
    reader.readI();

    subSong->timeBase=reader.readC();
    subSong->speed1=reader.readC();
    subSong->speed2=reader.readC();
    subSong->arpLen=reader.readC();
    subSong->hz=reader.readF();
    subSong->pal=(subSong->hz>=53);
    subSong->customTempo=true;

    subSong->patLen=reader.readS();
    subSong->ordersLen=reader.readS();

    subSong->hilightA=reader.readC();
    subSong->hilightB=reader.readC();

    ds.insLen=reader.readS();
    ds.waveLen=reader.readS();
    ds.sampleLen=reader.readS();
    int numberOfPats=reader.readI();

    if (subSong->patLen<0) {
      logE("pattern length is negative!");
      lastError="pattern lengrh is negative!";
      delete[] file;
      return false;
    }
    if (subSong->patLen>DIV_MAX_ROWS) {
      logE("pattern length is too large!");
      lastError="pattern length is too large!";
      delete[] file;
      return false;
    }
    if (subSong->ordersLen<0) {
      logE("song length is negative!");
      lastError="song length is negative!";
      delete[] file;
      return false;
    }
    if (subSong->ordersLen>DIV_MAX_PATTERNS) {
      logE("song is too long!");
      lastError="song is too long!";
      delete[] file;
      return false;
    }
    if (ds.insLen<0 || ds.insLen>256) {
      logE("invalid instrument count!");
      lastError="invalid instrument count!";
      delete[] file;
      return false;
    }
    if (ds.waveLen<0 || ds.waveLen>256) {
      logE("invalid wavetable count!");
      lastError="invalid wavetable count!";
      delete[] file;
      return false;
    }
    if (ds.sampleLen<0 || ds.sampleLen>256) {
      logE("invalid sample count!");
      lastError="invalid sample count!";
      delete[] file;
      return false;
    }
    if (numberOfPats<0) {
      logE("invalid pattern count!");
      lastError="invalid pattern count!";
      delete[] file;
      return false;
    }

    logD("systems:");
    for (int i=0; i<DIV_MAX_CHIPS; i++) {
      unsigned char sysID=reader.readC();
      ds.system[i]=systemFromFileFur(sysID);
      logD("- %d: %.2x (%s)",i,sysID,getSystemName(ds.system[i]));
      if (sysID!=0 && systemToFileFur(ds.system[i])==0) {
        logE("unrecognized system ID %.2x",ds.system[i]);
        lastError=fmt::sprintf("unrecognized system ID %.2x!",ds.system[i]);
        delete[] file;
        return false;
      }
      if (ds.system[i]!=DIV_SYSTEM_NULL) ds.systemLen=i+1;
    }
    int tchans=0;
    for (int i=0; i<ds.systemLen; i++) {
      tchans+=getChannelCount(ds.system[i]);
    }
    if (tchans>DIV_MAX_CHANS) {
      tchans=DIV_MAX_CHANS;
      logW("too many channels!");
    }

    // system volume
    for (int i=0; i<DIV_MAX_CHIPS; i++) {
      ds.systemVol[i]=reader.readC();
      if (ds.version<59 && ds.system[i]==DIV_SYSTEM_NES) {
        ds.systemVol[i]/=4;
      }
    }

    // system panning
    for (int i=0; i<DIV_MAX_CHIPS; i++) ds.systemPan[i]=reader.readC();

    // system props
    for (int i=0; i<DIV_MAX_CHIPS; i++) {
      sysFlagsPtr[i]=reader.readI();
    }

    // handle compound systems
    for (int i=0; i<DIV_MAX_CHIPS; i++) {
      if (ds.system[i]==DIV_SYSTEM_GENESIS ||
          ds.system[i]==DIV_SYSTEM_GENESIS_EXT ||
          ds.system[i]==DIV_SYSTEM_ARCADE) {
        for (int j=31; j>i; j--) {
          ds.system[j]=ds.system[j-1];
          ds.systemVol[j]=ds.systemVol[j-1];
          ds.systemPan[j]=ds.systemPan[j-1];
        }
        if (++ds.systemLen>DIV_MAX_CHIPS) ds.systemLen=DIV_MAX_CHIPS;

        if (ds.system[i]==DIV_SYSTEM_GENESIS) {
          ds.system[i]=DIV_SYSTEM_YM2612;
          if (i<31) {
            ds.system[i+1]=DIV_SYSTEM_SMS;
            ds.systemVol[i+1]=(((ds.systemVol[i]&127)*3)>>3)|(ds.systemVol[i]&128);
          }
        }
        if (ds.system[i]==DIV_SYSTEM_GENESIS_EXT) {
          ds.system[i]=DIV_SYSTEM_YM2612_EXT;
          if (i<31) {
            ds.system[i+1]=DIV_SYSTEM_SMS;
            ds.systemVol[i+1]=(((ds.systemVol[i]&127)*3)>>3)|(ds.systemVol[i]&128);
          }
        }
        if (ds.system[i]==DIV_SYSTEM_ARCADE) {
          ds.system[i]=DIV_SYSTEM_YM2151;
          if (i<31) {
            ds.system[i+1]=DIV_SYSTEM_SEGAPCM_COMPAT;
          }
        }
        i++;
      }
    }

    ds.name=reader.readString();
    ds.author=reader.readString();
    logI("%s by %s",ds.name.c_str(),ds.author.c_str());

    if (ds.version>=33) {
      ds.tuning=reader.readF();
    } else {
      reader.readI();
    }

    // compatibility flags
    if (ds.version>=37) {
      ds.limitSlides=reader.readC();
      ds.linearPitch=reader.readC();
      ds.loopModality=reader.readC();
      if (ds.version>=43) {
        ds.properNoiseLayout=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=43) {
        ds.waveDutyIsVol=reader.readC();
      } else {
        reader.readC();
      }

      if (ds.version>=45) {
        ds.resetMacroOnPorta=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.legacyVolumeSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.compatibleArpeggio=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.noteOffResetsSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=45) {
        ds.targetResetsSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=47) {
        ds.arpNonPorta=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=47) {
        ds.algMacroBehavior=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=49) {
        ds.brokenShortcutSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=50) {
        ds.ignoreDuplicateSlides=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=62) {
        ds.stopPortaOnNoteOff=reader.readC();
        ds.continuousVibrato=reader.readC();
      } else {
        reader.readC();
        reader.readC();
      }
      if (ds.version>=64) {
        ds.brokenDACMode=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=65) {
        ds.oneTickCut=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=66) {
        ds.newInsTriggersInPorta=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=69) {
        ds.arp0Reset=reader.readC();
      } else {
        reader.readC();
      }
    } else {
      for (int i=0; i<20; i++) reader.readC();
    }

    // pointers
    for (int i=0; i<ds.insLen; i++) {
      insPtr[i]=reader.readI();
    }
    for (int i=0; i<ds.waveLen; i++) {
      wavePtr[i]=reader.readI();
    }
    for (int i=0; i<ds.sampleLen; i++) {
      samplePtr[i]=reader.readI();
    }
    for (int i=0; i<numberOfPats; i++) patPtr.push_back(reader.readI());

    logD("reading orders (%d)...",subSong->ordersLen);
    for (int i=0; i<tchans; i++) {
      for (int j=0; j<subSong->ordersLen; j++) {
        subSong->orders.ord[i][j]=reader.readC();
      }
    }

    for (int i=0; i<tchans; i++) {
      subSong->pat[i].effectCols=reader.readC();
      if (subSong->pat[i].effectCols<1 || subSong->pat[i].effectCols>DIV_MAX_EFFECTS) {
        logE("channel %d has zero or too many effect columns! (%d)",i,subSong->pat[i].effectCols);
        lastError=fmt::sprintf("channel %d has too many effect columns! (%d)",i,subSong->pat[i].effectCols);
        delete[] file;
        return false;
      }
    }

    if (ds.version>=39) {
      for (int i=0; i<tchans; i++) {
        subSong->chanShow[i]=reader.readC();
      }

      for (int i=0; i<tchans; i++) {
        subSong->chanCollapse[i]=reader.readC();
      }

      if (ds.version<92) {
        for (int i=0; i<tchans; i++) {
          if (subSong->chanCollapse[i]>0) subSong->chanCollapse[i]=3;
        }
      }

      for (int i=0; i<tchans; i++) {
        subSong->chanName[i]=reader.readString();
      }

      for (int i=0; i<tchans; i++) {
        subSong->chanShortName[i]=reader.readString();
      }

      ds.notes=reader.readString();
    }

    if (ds.version>=59) {
      ds.masterVol=reader.readF();
    } else {
      ds.masterVol=2.0f;
    }

    if (ds.version>=70) {
      // extended compat flags
      ds.brokenSpeedSel=reader.readC();
      if (ds.version>=71) {
        ds.noSlidesOnFirstTick=reader.readC();
        ds.rowResetsArpPos=reader.readC();
        ds.ignoreJumpAtEnd=reader.readC();
      } else {
        reader.readC();
        reader.readC();
        reader.readC();
      }
      if (ds.version>=72) {
        ds.buggyPortaAfterSlide=reader.readC();
        ds.gbInsAffectsEnvelope=reader.readC();
      } else {
        reader.readC();
        reader.readC();
      }
      if (ds.version>=78) {
        ds.sharedExtStat=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=83) {
        ds.ignoreDACModeOutsideIntendedChannel=reader.readC();
        ds.e1e2AlsoTakePriority=reader.readC();
      } else {
        reader.readC();
        reader.readC();
      }
      if (ds.version>=84) {
        ds.newSegaPCM=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=85) {
        ds.fbPortaPause=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=86) {
        ds.snDutyReset=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=90) {
        ds.pitchMacroIsLinear=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=94) {
        ds.pitchSlideSpeed=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=97) {
        ds.oldOctaveBoundary=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=98) {
        ds.noOPN2Vol=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=99) {
        ds.newVolumeScaling=reader.readC();
        ds.volMacroLinger=reader.readC();
        ds.brokenOutVol=reader.readC();
      } else {
        reader.readC();
        reader.readC();
        reader.readC();
      }
      if (ds.version>=100) {
        ds.e1e2StopOnSameNote=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=101) {
        ds.brokenPortaArp=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=108) {
        ds.snNoLowPeriods=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=110) {
        ds.delayBehavior=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=113) {
        ds.jumpTreatment=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=115) {
        ds.autoSystem=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=117) {
        ds.disableSampleMacro=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=121) {
        ds.brokenOutVol2=reader.readC();
      } else {
        reader.readC();
      }
      if (ds.version>=130) {
        ds.oldArpStrategy=reader.readC();
      } else {
        reader.readC();
      }
    }

    // first song virtual tempo
    if (ds.version>=96) {
      subSong->virtualTempoN=reader.readS();
      subSong->virtualTempoD=reader.readS();
    } else {
      reader.readI();
    }

    // subsongs
    if (ds.version>=95) {
      subSong->name=reader.readString();
      subSong->notes=reader.readString();
      numberOfSubSongs=(unsigned char)reader.readC();
      reader.readC(); // reserved
      reader.readC();
      reader.readC();
      // pointers
      for (int i=0; i<numberOfSubSongs; i++) {
        subSongPtr[i]=reader.readI();
      }
    }

    // additional metadata
    if (ds.version>=103) {
      ds.systemName=reader.readString();
      ds.category=reader.readString();
      ds.nameJ=reader.readString();
      ds.authorJ=reader.readString();
      ds.systemNameJ=reader.readString();
      ds.categoryJ=reader.readString();
    } else {
      ds.systemName=getSongSystemLegacyName(ds,!getConfInt("noMultiSystem",0));
      ds.autoSystem=true;
    }

    // read system flags
    if (ds.version>=119) {
      logD("reading chip flags...");
      for (int i=0; i<DIV_MAX_CHIPS; i++) {
        if (sysFlagsPtr[i]==0) continue;

        if (!reader.seek(sysFlagsPtr[i],SEEK_SET)) {
          logE("couldn't seek to chip %d flags!",i+1);
          lastError=fmt::sprintf("couldn't seek to chip %d flags!",i+1);
          ds.unload();
          delete[] file;
          return false;
        }

        reader.read(magic,4);
        if (strcmp(magic,"FLAG")!=0) {
          logE("%d: invalid flag header!",i);
          lastError="invalid flag header!";
          ds.unload();
          delete[] file;
          return false;
        }
        reader.readI();

        String data=reader.readString();
        ds.systemFlags[i].loadFromMemory(data.c_str());
      }
    } else {
      logD("reading old chip flags...");
      for (int i=0; i<ds.systemLen; i++) {
        convertOldFlags(sysFlagsPtr[i],ds.systemFlags[i],ds.system[i]);
      }
    }

    // read subsongs
    if (ds.version>=95) {
      for (int i=0; i<numberOfSubSongs; i++) {
        ds.subsong.push_back(new DivSubSong);
        if (!reader.seek(subSongPtr[i],SEEK_SET)) {
          logE("couldn't seek to subsong %d!",i+1);
          lastError=fmt::sprintf("couldn't seek to subsong %d!",i+1);
          ds.unload();
          delete[] file;
          return false;
        }

        reader.read(magic,4);
        if (strcmp(magic,"SONG")!=0) {
          logE("%d: invalid subsong header!",i);
          lastError="invalid subsong header!";
          ds.unload();
          delete[] file;
          return false;
        }
        reader.readI();

        subSong=ds.subsong[i+1];
        subSong->timeBase=reader.readC();
        subSong->speed1=reader.readC();
        subSong->speed2=reader.readC();
        subSong->arpLen=reader.readC();
        subSong->hz=reader.readF();
        subSong->pal=(subSong->hz>=53);
        subSong->customTempo=true;

        subSong->patLen=reader.readS();
        subSong->ordersLen=reader.readS();

        subSong->hilightA=reader.readC();
        subSong->hilightB=reader.readC();

        if (ds.version>=96) {
          subSong->virtualTempoN=reader.readS();
          subSong->virtualTempoD=reader.readS();
        } else {
          reader.readI();
        }

        subSong->name=reader.readString();
        subSong->notes=reader.readString();

        logD("reading orders of subsong %d (%d)...",i+1,subSong->ordersLen);
        for (int j=0; j<tchans; j++) {
          for (int k=0; k<subSong->ordersLen; k++) {
            subSong->orders.ord[j][k]=reader.readC();
          }
        }

        for (int i=0; i<tchans; i++) {
          subSong->pat[i].effectCols=reader.readC();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanShow[i]=reader.readC();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanCollapse[i]=reader.readC();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanName[i]=reader.readString();
        }

        for (int i=0; i<tchans; i++) {
          subSong->chanShortName[i]=reader.readString();
        }
      }
    }

    // read instruments
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      logD("reading instrument %d at %x...",i,insPtr[i]);
      if (!reader.seek(insPtr[i],SEEK_SET)) {
        logE("couldn't seek to instrument %d!",i);
        lastError=fmt::sprintf("couldn't seek to instrument %d!",i);
        ds.unload();
        delete ins;
        delete[] file;
        return false;
      }
      
      if (ins->readInsData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        ds.unload();
        delete ins;
        delete[] file;
        return false;
      }

      ds.ins.push_back(ins);
    }

    // read wavetables
    for (int i=0; i<ds.waveLen; i++) {
      DivWavetable* wave=new DivWavetable;
      logD("reading wavetable %d at %x...",i,wavePtr[i]);
      if (!reader.seek(wavePtr[i],SEEK_SET)) {
        logE("couldn't seek to wavetable %d!",i);
        lastError=fmt::sprintf("couldn't seek to wavetable %d!",i);
        ds.unload();
        delete wave;
        delete[] file;
        return false;
      }

      if (wave->readWaveData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid wavetable header/data!";
        ds.unload();
        delete wave;
        delete[] file;
        return false;
      }

      ds.wave.push_back(wave);
    }

    // read samples
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;

      if (!reader.seek(samplePtr[i],SEEK_SET)) {
        logE("couldn't seek to sample %d!",i);
        lastError=fmt::sprintf("couldn't seek to sample %d!",i);
        ds.unload();
        delete sample;
        delete[] file;
        return false;
      }

      if (sample->readSampleData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid sample header/data!";
        ds.unload();
        delete sample;
        delete[] file;
        return false;
      }

      ds.sample.push_back(sample);
    }

    // read patterns
    for (int i: patPtr) {
      if (!reader.seek(i,SEEK_SET)) {
        logE("couldn't seek to pattern in %x!",i);
        lastError=fmt::sprintf("couldn't seek to pattern in %x!",i);
        ds.unload();
        delete[] file;
        return false;
      }
      reader.read(magic,4);
      logD("reading pattern in %x...",i);
      if (strcmp(magic,"PATR")!=0) {
        logE("%x: invalid pattern header!",i);
        lastError="invalid pattern header!";
        ds.unload();
        delete[] file;
        return false;
      }
      reader.readI();

      int chan=reader.readS();
      int index=reader.readS();
      int subs=0;
      if (ds.version>=95) {
        subs=reader.readS();
      } else {
        reader.readS();
      }
      reader.readS();

      logD("- %d, %d, %d",subs,chan,index);

      if (chan<0 || chan>=tchans) {
        logE("pattern channel out of range!",i);
        lastError="pattern channel out of range!";
        ds.unload();
        delete[] file;
        return false;
      }
      if (index<0 || index>(DIV_MAX_PATTERNS-1)) {
        logE("pattern index out of range!",i);
        lastError="pattern index out of range!";
        ds.unload();
        delete[] file;
        return false;
      }
      if (subs<0 || subs>=(int)ds.subsong.size()) {
        logE("pattern subsong out of range!",i);
        lastError="pattern subsong out of range!";
        ds.unload();
        delete[] file;
        return false;
      }

      DivPattern* pat=ds.subsong[subs]->pat[chan].getPattern(index,true);
      for (int j=0; j<ds.subsong[subs]->patLen; j++) {
        pat->data[j][0]=reader.readS();
        pat->data[j][1]=reader.readS();
        pat->data[j][2]=reader.readS();
        pat->data[j][3]=reader.readS();
        for (int k=0; k<ds.subsong[subs]->pat[chan].effectCols; k++) {
          pat->data[j][4+(k<<1)]=reader.readS();
          pat->data[j][5+(k<<1)]=reader.readS();
        }
      }

      if (ds.version>=51) {
        pat->name=reader.readString();
      }
    }

    if (reader.tell()<reader.size()) {
      if ((reader.tell()+1)!=reader.size()) {
        logW("premature end of song (we are at %x, but size is %x)",reader.tell(),reader.size());
      }
    }

    // convert OPM/NES instrument types
    if (ds.version<117) {
      int opnCount=0;
      int opmCount=0;
      int snCount=0;
      int nesCount=0;
      for (int i=0; i<ds.systemLen; i++) {
        switch (ds.system[i]) {
          case DIV_SYSTEM_NES:
          case DIV_SYSTEM_MMC5:
            nesCount++;
            break;
          case DIV_SYSTEM_SMS:
            snCount++;
            break;
          case DIV_SYSTEM_YM2151:
          case DIV_SYSTEM_OPZ:
            opmCount++;
            break;
          case DIV_SYSTEM_YM2610:
          case DIV_SYSTEM_YM2610_EXT:
          case DIV_SYSTEM_YM2610_FULL:
          case DIV_SYSTEM_YM2610_FULL_EXT:
          case DIV_SYSTEM_YM2610B:
          case DIV_SYSTEM_YM2610B_EXT:
          case DIV_SYSTEM_YM2203:
          case DIV_SYSTEM_YM2203_EXT:
          case DIV_SYSTEM_YM2608:
          case DIV_SYSTEM_YM2608_EXT:
          case DIV_SYSTEM_YM2612:
          case DIV_SYSTEM_YM2612_EXT:
          case DIV_SYSTEM_YM2612_DUALPCM:
          case DIV_SYSTEM_YM2612_DUALPCM_EXT:
            opnCount++;
            break;
          default:
            break;
        }
      }
      if (opmCount>opnCount) {
        for (DivInstrument* i: ds.ins) {
          if (i->type==DIV_INS_FM) i->type=DIV_INS_OPM;
        }
      }
      if (nesCount>snCount) {
        for (DivInstrument* i: ds.ins) {
          if (i->type==DIV_INS_STD) i->type=DIV_INS_NES;
        }
      }
    }

    // ExtCh compat flag
    for (int i=0; i<ds.systemLen; i++) {
      if (ds.system[i]==DIV_SYSTEM_YM2612_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2612_DUALPCM_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2610_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2610_FULL_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2610B_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2203_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2608_EXT ||
          ds.system[i]==DIV_SYSTEM_YM2612_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2203_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2608_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2610_CSM ||
          ds.system[i]==DIV_SYSTEM_YM2610B_CSM) {
        if (ds.version<125) {
          ds.systemFlags[i].set("noExtMacros",true);
        }
        if (ds.version<133) {
          ds.systemFlags[i].set("fbAllOps",true);
        }
      }
    }

    // SN noise compat
    if (ds.version<128) {
      for (int i=0; i<ds.systemLen; i++) {
        if (ds.system[i]==DIV_SYSTEM_SMS ||
            ds.system[i]==DIV_SYSTEM_T6W28) {
          ds.systemFlags[i].set("noEasyNoise",true);
        }
      }
    }

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

bool DivEngine::loadMod(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  int chCount=0;
  int ordCount=0;
  std::vector<int> patPtr;
  char magic[4]={0,0,0,0};
  short defaultVols[31];
  int sampLens[31];
  // 0=arp, 1=pslide, 2=vib, 3=trem, 4=vslide
  bool fxUsage[DIV_MAX_CHANS][5];  
  SafeReader reader=SafeReader(file,len);
  warnings="";

  memset(defaultVols,0,31*sizeof(short));
  memset(sampLens,0,31*sizeof(int));
  memset(fxUsage,0,DIV_MAX_CHANS*5*sizeof(bool));

  try {
    DivSong ds;
    ds.tuning=436.0;
    ds.version=DIV_VERSION_MOD;
    ds.linearPitch=0;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;
    ds.delayBehavior=0;

    int insCount=31;
    bool bypassLimits=false;

    // check mod magic bytes
    if (!reader.seek(1080,SEEK_SET)) {
      logD("couldn't seek to 1080");
      throw EndOfFileException(&reader,reader.tell());
    }
    if (reader.read(magic,4)!=4) {
      logD("the magic isn't complete");
      throw EndOfFileException(&reader,reader.tell());
    }
    if (memcmp(magic,"M.K.",4)==0 || memcmp(magic,"M!K!",4)==0 || memcmp(magic,"M&K!",4)==0) {
      logD("detected a ProTracker module");
      ds.systemName="Amiga";
      chCount=4;
    } else if (memcmp(magic,"CD81",4)==0 || memcmp(magic,"OKTA",4)==0 || memcmp(magic,"OCTA",4)==0) {
      logD("detected an Oktalyzer/Octalyzer/OctaMED module");
      ds.systemName="Amiga (8-channel)";
      chCount=8;
    } else if (memcmp(magic+1,"CHN",3)==0 && magic[0]>='1' && magic[0]<='9') {
      logD("detected a FastTracker module");
      ds.systemName="PC";
      chCount=magic[0]-'0';
    } else if (memcmp(magic,"FLT",3)==0 && magic[3]>='1' && magic[3]<='9') {
      logD("detected a Fairlight module");
      ds.systemName="Amiga";
      chCount=magic[3]-'0';
    } else if (memcmp(magic,"TDZ",3)==0 && magic[3]>='1' && magic[3]<='9') {
      logD("detected a TakeTracker module");
      ds.systemName="PC";
      chCount=magic[3]-'0';
    } else if ((memcmp(magic+2,"CH",2)==0 || memcmp(magic+2,"CN",2)==0)  &&
               (magic[0]>='1' && magic[0]<='9' && magic[1]>='0' && magic[1]<='9')) {
      logD("detected a Fast/TakeTracker module");
      ds.systemName="PC";
      chCount=((magic[0]-'0')*10)+(magic[1]-'0');
    } else {
      insCount=15;
      logD("possibly a Soundtracker module");
      ds.systemName="Amiga";
      chCount=4;
    }

    // song name
    if (!reader.seek(0,SEEK_SET)) {
      logD("couldn't seek to 0");
      throw EndOfFileException(&reader,reader.tell());
    }
    ds.name=reader.readString(20);
    logI("%s",ds.name);

    // samples
    logD("reading samples... (%d)",insCount);
    for (int i=0; i<insCount; i++) {
      DivSample* sample=new DivSample;
      sample->depth=DIV_SAMPLE_DEPTH_8BIT;
      sample->name=reader.readString(22);
      logD("%d: %s",i+1,sample->name);
      int slen=((unsigned short)reader.readS_BE())*2;
      sampLens[i]=slen;
      if (slen==2) slen=0;
      signed char fineTune=reader.readC()&0x0f;
      if (fineTune>=8) fineTune-=16;
      sample->rate=(int)(pow(2.0,(double)fineTune/96.0)*8363.0);
      sample->centerRate=sample->rate;
      defaultVols[i]=reader.readC();
      int loopStart=reader.readS_BE()*2;
      int loopLen=reader.readS_BE()*2;
      int loopEnd=loopStart+loopLen;
      // bunch of checks since ProTracker abuses those for one-shot samples
      if (loopStart>loopEnd || loopEnd<4 || loopLen<4) {
        loopStart=0;
        loopLen=0;
      }
      if (loopLen>=2) {
        sample->loopStart=loopStart;
        sample->loopEnd=loopEnd;
        sample->loop=(sample->loopStart>=0)&&(sample->loopEnd>=0);
      }
      sample->init(slen);
      ds.sample.push_back(sample);
    }
    ds.sampleLen=ds.sample.size();

    // orders
    ds.subsong[0]->ordersLen=ordCount=reader.readC();
    if (ds.subsong[0]->ordersLen<1 || ds.subsong[0]->ordersLen>127) {
      logD("invalid order count!");
      throw EndOfFileException(&reader,reader.tell());
    }
    unsigned char restartPos=reader.readC(); // restart position, unused
    logD("restart position byte: %.2x",restartPos);
    if (insCount==15) {
      if (restartPos>0x60 && restartPos<0x80) {
        logD("detected a Soundtracker module");
      } else {
        logD("no Soundtracker signature found");
        throw EndOfFileException(&reader,reader.tell());
      }
    }

    int patMax=0;
    for (int i=0; i<128; i++) {
      unsigned char pat=reader.readC();
      if (pat>patMax) patMax=pat;
      for (int j=0; j<chCount; j++) {
        ds.subsong[0]->orders.ord[j][i]=pat;
      }
    }

    if (insCount==15) {
      if (!reader.seek(600,SEEK_SET)) {
        logD("couldn't seek to 600");
        throw EndOfFileException(&reader,reader.tell());
      }
    } else {
      if (!reader.seek(1084,SEEK_SET)) {
        logD("couldn't seek to 1084");
        throw EndOfFileException(&reader,reader.tell());
      }
    }

    // patterns
    ds.subsong[0]->patLen=64;
    for (int ch=0; ch<chCount; ch++) {
      for (int i=0; i<5; i++) {
        fxUsage[ch][i]=false;
      }
    }
    for (int pat=0; pat<=patMax; pat++) {
      DivPattern* chpats[DIV_MAX_CHANS];
      for (int ch=0; ch<chCount; ch++) {
        chpats[ch]=ds.subsong[0]->pat[ch].getPattern(pat,true);
      }
      for (int row=0; row<64; row++) {
        for (int ch=0; ch<chCount; ch++) {
          short* dstrow=chpats[ch]->data[row];
          unsigned char data[4];
          reader.read(&data,4);
          // instrument
          short ins=(data[0]&0xf0)|(data[2]>>4);
          if (ins>0) {
            dstrow[2]=ins-1;
            dstrow[3]=defaultVols[ins-1];
          }
          // note
          int period=data[1]+((data[0]&0x0f)<<8);
          if (period>0 && period<0x0fff) {
            short note=(short)round(log2(3424.0/period)*12);
            dstrow[0]=((note-1)%12)+1;
            dstrow[1]=(note-1)/12+1;
            if (period<114) {
              bypassLimits=true;
            }
          }
          // effects are done later
          short fxtyp=data[2]&0x0f;
          short fxval=data[3];
          dstrow[4]=fxtyp;
          dstrow[5]=fxval;
          switch (fxtyp) {
            case 0:
              if (fxval!=0) fxUsage[ch][0]=true;
              break;
            case 1: case 2: case 3:
              fxUsage[ch][1]=true;
              break;
            case 4:
              fxUsage[ch][2]=true;
              break;
            case 5:
              fxUsage[ch][1]=true;
              fxUsage[ch][4]=true;
              break;
            case 6:
              fxUsage[ch][2]=true;
              fxUsage[ch][4]=true;
              break;
            case 7:
              fxUsage[ch][3]=true;
              break;
            case 10:
              if (fxval!=0) fxUsage[ch][4]=true;
              break;
          }
        }
      }
    }

    // samples
    size_t pos=reader.tell();
    logD("reading sample data...");
    for (int i=0; i<insCount; i++) {
      if (!reader.seek(pos,SEEK_SET)) {
        logD("%d: couldn't seek to %d",i,pos);
        throw EndOfFileException(&reader,reader.tell());
      }
      reader.read(ds.sample[i]->data8,ds.sample[i]->samples);
      pos+=sampLens[i];
    }

    // convert effects
    for (int ch=0; ch<=chCount; ch++) {
      unsigned char fxCols=1;
      for (int pat=0; pat<=patMax; pat++) {
        auto* data=ds.subsong[0]->pat[ch].getPattern(pat,true)->data;
        short lastPitchEffect=-1;
        short lastEffectState[5]={-1,-1,-1,-1,-1};
        short setEffectState[5]={-1,-1,-1,-1,-1};
        for (int row=0;row<64;row++) {
          const short fxUsageTyp[5]={0x00,0x01,0x04,0x07,0xFA};
          short effectState[5]={0,0,0,0,0};
          unsigned char curFxCol=0;
          short fxTyp=data[row][4];
          short fxVal=data[row][5];
          auto writeFxCol=[data,row,&curFxCol](short typ, short val) {
            data[row][4+curFxCol*2]=typ;
            data[row][5+curFxCol*2]=val;
            curFxCol++;
          };
          writeFxCol(-1,-1);
          curFxCol=0;
          switch (fxTyp) {
            case 0: // arp
              effectState[0]=fxVal;
              break;
            case 5: // vol slide + porta
              effectState[4]=fxVal;
              fxTyp=3;
              fxVal=0;
              // fall through
            case 1: // note slide up
            case 2: // note slide down
            case 3: // porta
              if (fxTyp==3 && fxVal==0) {
                if (setEffectState[1]<0) break;
                fxVal=setEffectState[1];
              }
              setEffectState[1]=fxVal;
              effectState[1]=fxVal;
              if ((effectState[1]!=lastEffectState[1]) ||
                  (fxTyp!=lastPitchEffect) ||
                  (effectState[1]!=0 && data[row][0]>0)) {
                writeFxCol(fxTyp,fxVal);
              }
              lastPitchEffect=fxTyp;
              lastEffectState[1]=fxVal;
              break;
            case 6: // vol slide + vibrato
              effectState[4]=fxVal;
              fxTyp=4;
              fxVal=0;
              // fall through
            case 4: // vibrato
              // TODO: handle 0 value?
              if (fxVal==0) {
                if (setEffectState[2]<0) break;
                fxVal=setEffectState[2];
              }
              effectState[2]=fxVal;
              setEffectState[2]=fxVal;
              break;
            case 7: // tremolo
              if (fxVal==0) {
                if (setEffectState[3]<0) break;
                fxVal=setEffectState[3];
              }
              effectState[3]=fxVal;
              setEffectState[3]=fxVal;
              break;
            case 9: // set offset
              writeFxCol(0x90,fxVal);
              break;
            case 10: // vol slide
              effectState[4]=fxVal;
              break;
            case 11: // jump to pos
              writeFxCol(fxTyp,fxVal);
              break;
            case 12: // set vol
              data[row][3]=MIN(0x40,fxVal);
              break;
            case 13: // break to row (BCD)
              writeFxCol(fxTyp,((fxVal>>4)*10)+(fxVal&15));
              break;
            case 15: // set speed
              // TODO: somehow handle VBlank tunes
              // TODO: i am so sorry
              if (fxVal>0x20 && ds.name!="klisje paa klisje") {
                writeFxCol(0xf0,fxVal);
              } else {
                writeFxCol(0x09,fxVal);
                writeFxCol(0x0f,fxVal);
              }
              break;
            case 14: // extended
              fxTyp=fxVal>>4;
              fxVal&=0x0f;
              switch (fxTyp) {
                case 0:
                  writeFxCol(0x10,!fxVal);
                  break;
                case 1: // single note slide up
                case 2: // single note slide down
                  writeFxCol(fxTyp-1+0xf1,fxVal);
                  break;
                case 9: // retrigger
                  writeFxCol(0x0c,fxVal);
                  break;
                case 10: // single vol slide up
                case 11: // single vol slide down
                  writeFxCol(fxTyp-10+0xf8,fxVal);
                  break;
                case 12: // note cut
                case 13: // note delay
                  writeFxCol(fxTyp-12+0xec,fxVal);
                  break;
              }
              break;
          }
          for (int i=0; i<5; i++) {
            // pitch slide and volume slide needs to be kept active on new note
            // even after target/max is reached
            if (fxUsage[ch][i] && (effectState[i]!=lastEffectState[i] || (effectState[i]!=0 && i==4 && data[row][3]>=0))) {
              writeFxCol(fxUsageTyp[i],effectState[i]);
            }
          }
          memcpy(lastEffectState,effectState,sizeof(effectState));
          if (curFxCol>fxCols) {
            fxCols=curFxCol;
          }
        }
      }
      ds.subsong[0]->pat[ch].effectCols=fxCols;
    }

    ds.subsong[0]->pal=false;
    ds.subsong[0]->hz=50;
    ds.subsong[0]->customTempo=false;
    ds.systemLen=(chCount+3)/4;
    for(int i=0; i<ds.systemLen; i++) {
      ds.system[i]=DIV_SYSTEM_AMIGA;
      ds.systemFlags[i].set("clockSel",1); // PAL
      ds.systemFlags[i].set("stereoSep",80);
      ds.systemFlags[i].set("bypassLimits",bypassLimits);
      ds.systemFlags[i].set("chipType",(bool)(ds.systemLen>1 || bypassLimits));
    }
    for(int i=0; i<chCount; i++) {
      ds.subsong[0]->chanShow[i]=true;
      ds.subsong[0]->chanName[i]=fmt::sprintf("Channel %d",i+1);
      ds.subsong[0]->chanShortName[i]=fmt::sprintf("C%d",i+1);
    }
    for(int i=chCount; i<ds.systemLen*4; i++) {
      ds.subsong[0]->pat[i].effectCols=1;
      ds.subsong[0]->chanShow[i]=false;
    }
    
    // instrument creation
    for(int i=0; i<insCount; i++) {
      DivInstrument* ins=new DivInstrument;
      ins->type=DIV_INS_AMIGA;
      ins->amiga.initSample=i;
      ins->name=ds.sample[i]->name;
      ds.ins.push_back(ins);
    }
    ds.insLen=ds.ins.size();
    
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
    success=true;
  } catch (EndOfFileException& e) {
    //logE("premature end of file!");
    lastError="incomplete file";
  } catch (InvalidHeaderException& e) {
    //logE("invalid info header!");
    lastError="invalid info header!";
  }
  return success;
}

unsigned char fcXORTriangle[32]={
  0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8, 0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
  0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8
};

unsigned char fcCustom1[32]={
  0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66, 0x61, 0x58, 0x53, 0x4d, 0x2c, 0x20, 0x18, 0x12,
  0x04, 0xdb, 0xd3, 0xcd, 0xc6, 0xbc, 0xb5, 0xae, 0xa8, 0xa3, 0x9d, 0x99, 0x93, 0x8e, 0x8b, 0x8a
};

unsigned char fcCustom2[32]={
  0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66, 0x5b, 0x4b, 0x43, 0x37, 0x2c, 0x20, 0x18, 0x12,
  0x04, 0xf8, 0xe8, 0xdb, 0xcf, 0xc6, 0xbe, 0xb0, 0xa8, 0xa4, 0x9e, 0x9a, 0x95, 0x94, 0x8d, 0x83
};

unsigned char fcTinyTriangle[16]={
  0x00, 0x00, 0x40, 0x60, 0x7f, 0x60, 0x40, 0x20, 0x00, 0xe0, 0xc0, 0xa0, 0x80, 0xa0, 0xc0, 0xe0
};

void generateFCPresetWave(int index, DivWavetable* wave) {
  wave->max=255;
  wave->len=32;

  switch (index) {
    case 0x00: case 0x01: case 0x02: case 0x03:
    case 0x04: case 0x05: case 0x06: case 0x07:
    case 0x08: case 0x09: case 0x0a: case 0x0b:
    case 0x0c: case 0x0d: case 0x0e: case 0x0f:
      // XOR triangle
      for (int i=0; i<32; i++) {
        wave->data[i]=(unsigned char)((fcXORTriangle[i]^0x80)^(((index+15)<i)?0x87:0x00));
      }
      break;
    case 0x10: case 0x11: case 0x12: case 0x13:
    case 0x14: case 0x15: case 0x16: case 0x17:
    case 0x18: case 0x19: case 0x1a: case 0x1b:
    case 0x1c: case 0x1d: case 0x1e: case 0x1f:
      // pulse
      for (int i=0; i<32; i++) {
        wave->data[i]=(index>i)?0x01:0xff;
      }
      break;
    case 0x20: case 0x21: case 0x22: case 0x23:
    case 0x24: case 0x25: case 0x26: case 0x27:
      // tiny pulse
      for (int i=0; i<32; i++) {
        wave->data[i]=((index-0x18)>(i&15))?0x01:0xff;
      }
      break;
    case 0x28:
    case 0x2e:
      // saw
      for (int i=0; i<32; i++) {
        wave->data[i]=i<<3;
      }
      break;
    case 0x29:
    case 0x2f:
      // tiny saw
      for (int i=0; i<32; i++) {
        wave->data[i]=(i<<4)&0xff;
      }
      break;
    case 0x2a:
      // custom 1
      for (int i=0; i<32; i++) {
        wave->data[i]=fcCustom1[i]^0x80;
      }
      break;
    case 0x2b:
      // custom 2
      for (int i=0; i<32; i++) {
        wave->data[i]=fcCustom2[i]^0x80;
      }
      break;
    case 0x2c: case 0x2d:
      // tiny triangle
      for (int i=0; i<32; i++) {
        wave->data[i]=fcTinyTriangle[i&15]^0x80;
      }
      break;
    default:
      for (int i=0; i<32; i++) {
        wave->data[i]=i;
      }
      break;
  }
}

bool DivEngine::loadFC(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[4]={0,0,0,0};
  SafeReader reader=SafeReader(file,len);
  warnings="";
  bool isFC14=false;
  unsigned int patPtr, freqMacroPtr, volMacroPtr, samplePtr, wavePtr;
  unsigned int seqLen, patLen, freqMacroLen, volMacroLen, sampleLen;

  unsigned char waveLen[80];
  //unsigned char waveLoopLen[40];

  struct FCSequence {
    unsigned char pat[4];
    signed char transpose[4];
    signed char offsetIns[4];
    unsigned char speed;
  };
  std::vector<FCSequence> seq;
  struct FCPattern {
    unsigned char note[32];
    unsigned char val[32];
  };
  std::vector<FCPattern> pat;
  struct FCMacro {
    unsigned char val[64];
  };
  std::vector<FCMacro> freqMacros;
  std::vector<FCMacro> volMacros;

  struct FCSample {
    unsigned short loopLen, len, loopStart;
  } sample[10];

  try {
    DivSong ds;
    ds.tuning=436.0;
    ds.version=DIV_VERSION_FC;
    //ds.linearPitch=0;
    //ds.pitchMacroIsLinear=false;
    //ds.noSlidesOnFirstTick=true;
    //ds.rowResetsArpPos=true;
    ds.pitchSlideSpeed=8;
    ds.ignoreJumpAtEnd=false;

    // load here
    if (!reader.seek(0,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    reader.read(magic,4);

    if (memcmp(magic,DIV_FC13_MAGIC,4)==0) {
      isFC14=false;
    } else if (memcmp(magic,DIV_FC14_MAGIC,4)==0) {
      isFC14=true;
    } else {
      logW("the magic isn't complete");
      throw EndOfFileException(&reader,reader.tell());
    }

    ds.systemLen=1;
    ds.system[0]=DIV_SYSTEM_AMIGA;
    ds.systemVol[0]=64;
    ds.systemPan[0]=0;
    ds.systemFlags[0].set("clockSel",1); // PAL
    ds.systemFlags[0].set("stereoSep",80);
    ds.systemName="Amiga";

    seqLen=reader.readI_BE();
    if (seqLen%13) {
      logW("sequence length is not multiple of 13 (%d)",seqLen);
      //throw EndOfFileException(&reader,reader.tell());
    }
    patPtr=reader.readI_BE();
    patLen=reader.readI_BE();
    if (patLen%64) {
      logW("pattern length is not multiple of 64 (%d)",patLen);
      throw EndOfFileException(&reader,reader.tell());
    }
    freqMacroPtr=reader.readI_BE();
    freqMacroLen=reader.readI_BE();
    if (freqMacroLen%64) {
      logW("freq sequence length is not multiple of 64 (%d)",freqMacroLen);
      //throw EndOfFileException(&reader,reader.tell());
    }
    volMacroPtr=reader.readI_BE();
    volMacroLen=reader.readI_BE();
    if (volMacroLen%64) {
      logW("vol sequence length is not multiple of 64 (%d)",volMacroLen);
      //throw EndOfFileException(&reader,reader.tell());
    }
    samplePtr=reader.readI_BE();
    if (isFC14) {
      wavePtr=reader.readI_BE(); // wave len
      sampleLen=0;
    } else {
      sampleLen=reader.readI_BE();
      wavePtr=0;
    }

    logD("patPtr: %x",patPtr);
    logD("patLen: %d",patLen);
    logD("freqMacroPtr: %x",freqMacroPtr);
    logD("freqMacroLen: %d",freqMacroLen);
    logD("volMacroPtr: %x",volMacroPtr);
    logD("volMacroLen: %d",volMacroLen);
    logD("samplePtr: %x",samplePtr);
    if (isFC14) {
      logD("wavePtr: %x",wavePtr);
    } else {
      logD("sampleLen: %d",sampleLen);
    }

    // sample info
    logD("samples: (%x)",reader.tell());
    for (int i=0; i<10; i++) {
      sample[i].len=reader.readS_BE();
      sample[i].loopStart=reader.readS_BE();
      sample[i].loopLen=reader.readS_BE();

      logD("- %d: %d (%d, %d)",i,sample[i].len,sample[i].loopStart,sample[i].loopLen);
    }

    // wavetable lengths
    if (isFC14) {
      logD("wavetables:");
      for (int i=0; i<80; i++) {
        waveLen[i]=(unsigned char)reader.readC();

        logD("- %d: %.4x",i,waveLen[i]);
      }
    }

    // sequences
    seqLen/=13;
    logD("reading sequences... (%d)",seqLen);
    for (unsigned int i=0; i<seqLen; i++) {
      FCSequence s;
      for (int j=0; j<4; j++) {
        s.pat[j]=reader.readC();
        s.transpose[j]=reader.readC();
        s.offsetIns[j]=reader.readC();
      }
      s.speed=reader.readC();
      seq.push_back(s);
      logV(
        "%.2x | %.2x%.2x%.2x %.2x%.2x%.2x %.2x%.2x%.2x %.2x%.2x%.2x | %.2x",
        i,
        s.pat[0],s.transpose[0],s.offsetIns[0],
        s.pat[1],s.transpose[1],s.offsetIns[1],
        s.pat[2],s.transpose[2],s.offsetIns[2],
        s.pat[3],s.transpose[3],s.offsetIns[3],
        s.speed
      );
    }

    // patterns
    if (!reader.seek(patPtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    patLen/=64;
    logD("reading patterns... (%d)",patLen);
    for (unsigned int i=0; i<patLen; i++) {
      FCPattern p;
      logV("- pattern %d",i);
      for (int j=0; j<32; j++) {
        p.note[j]=reader.readC();
        p.val[j]=reader.readC();
        //logV("%.2x | %.2x %.2x",j,p.note[j],p.val[j]);
      }
      pat.push_back(p);
    }

    // freq sequences
    if (!reader.seek(freqMacroPtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    freqMacroLen/=64;
    logD("reading freq sequences... (%d)",freqMacroLen);
    for (unsigned int i=0; i<freqMacroLen; i++) {
      FCMacro m;
      reader.read(m.val,64);
      freqMacros.push_back(m);
    }

    // vol sequences
    if (!reader.seek(volMacroPtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    volMacroLen/=64;
    logD("reading volume sequences... (%d)",volMacroLen);
    for (unsigned int i=0; i<volMacroLen; i++) {
      FCMacro m;
      reader.read(m.val,64);
      volMacros.push_back(m);
    }

    // samples
    if (!reader.seek(samplePtr,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    logD("reading samples...");
    for (int i=0; i<10; i++) {
      DivSample* s=new DivSample;
      s->depth=DIV_SAMPLE_DEPTH_8BIT;
      if (sample[i].len>0) {
        s->init(sample[i].len*2);
      }
      s->name=fmt::sprintf("Sample %d",i+1);
      if (sample[i].loopLen>1) {
        s->loopStart=sample[i].loopStart;
        s->loopEnd=sample[i].loopStart+(sample[i].loopLen*2);
        s->loop=(s->loopStart>=0)&&(s->loopEnd>=0);
      }
      reader.read(s->data8,sample[i].len*2);
      ds.sample.push_back(s);
    }
    ds.sampleLen=(int)ds.sample.size();

    // wavetables
    if (isFC14) {
      if (!reader.seek(wavePtr,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }
      logD("reading wavetables...");
      for (int i=0; i<80; i++) {
        DivWavetable* w=new DivWavetable;
        w->min=0;
        w->max=255;
        w->len=MIN(256,waveLen[i]*2);

        for (int i=0; i<256; i++) {
          w->data[i]=128;
        }
        
        if (waveLen[i]>0) {
          signed char* waveArray=new signed char[waveLen[i]*2];
          reader.read(waveArray,waveLen[i]*2);
          int howMany=waveLen[i]*2;
          if (howMany>256) howMany=256;
          for (int i=0; i<howMany; i++) {
            w->data[i]=waveArray[i]+128;
          }
          delete[] waveArray;
        } else {
          logV("empty wave %d",i);
          generateFCPresetWave(i,w);
        }

        ds.wave.push_back(w);
      }
    } else {
      // generate preset waves
      for (int i=0; i<48; i++) {
        DivWavetable* w=new DivWavetable;
        generateFCPresetWave(i,w);
        ds.wave.push_back(w);
      }
    }
    ds.waveLen=(int)ds.wave.size();

    // convert
    ds.subsong[0]->ordersLen=seqLen;
    ds.subsong[0]->patLen=32;
    ds.subsong[0]->hz=50;
    ds.subsong[0]->pal=true;
    ds.subsong[0]->customTempo=true;
    ds.subsong[0]->pat[3].effectCols=3;
    ds.subsong[0]->speed1=3;
    ds.subsong[0]->speed2=3;

    int lastIns[4];
    int lastNote[4];
    signed char lastTranspose[4];
    bool isSliding[4];

    memset(lastIns,-1,4*sizeof(int));
    memset(lastNote,-1,4*sizeof(int));
    memset(lastTranspose,0,4);
    memset(isSliding,0,4*sizeof(bool));

    for (unsigned int i=0; i<seqLen; i++) {
      for (int j=0; j<4; j++) {
        ds.subsong[0]->orders.ord[j][i]=i;
        DivPattern* p=ds.subsong[0]->pat[j].getPattern(i,true);
        if (j==3 && seq[i].speed) {
          p->data[0][6]=0x09;
          p->data[0][7]=seq[i].speed;
          p->data[0][8]=0x0f;
          p->data[0][9]=seq[i].speed;
        }

        bool ignoreNext=false;

        for (int k=0; k<32; k++) {
          FCPattern& fp=pat[seq[i].pat[j]];
          if (fp.note[k]>0 && fp.note[k]<0x49) {
            lastNote[j]=fp.note[k];
            short note=(fp.note[k]+seq[i].transpose[j])%12;
            short octave=2+((fp.note[k]+seq[i].transpose[j])/12);
            if (fp.note[k]>=0x3d) octave-=6;
            if (note==0) {
              note=12;
              octave--;
            }
            octave&=0xff;
            p->data[k][0]=note;
            p->data[k][1]=octave;
            if (isSliding[j]) {
              isSliding[j]=false;
              p->data[k][4]=2;
              p->data[k][5]=0;
            }
          } else if (fp.note[k]==0x49) {
            if (k>0) {
              p->data[k-1][4]=0x0d;
              p->data[k-1][5]=0;
            }
          } else if (k==0 && lastTranspose[j]!=seq[i].transpose[j]) {
            p->data[0][2]=lastIns[j];
            p->data[0][4]=0x03;
            p->data[0][5]=0xff;
            lastTranspose[j]=seq[i].transpose[j];

            short note=(lastNote[j]+seq[i].transpose[j])%12;
            short octave=2+((lastNote[j]+seq[i].transpose[j])/12);
            if (lastNote[j]>=0x3d) octave-=6;
            if (note==0) {
              note=12;
              octave--;
            }
            octave&=0xff;
            p->data[k][0]=note;
            p->data[k][1]=octave;
          }
          if (fp.val[k]) {
            if (ignoreNext) {
              ignoreNext=false;
            } else {
              if (fp.val[k]==0xf0) {
                p->data[k][0]=100;
                p->data[k][1]=0;
                p->data[k][2]=-1;
              } else if (fp.val[k]&0xe0) {
                if (fp.val[k]&0x40) {
                  p->data[k][4]=2;
                  p->data[k][5]=0;
                  isSliding[j]=false;
                } else if (fp.val[k]&0x80) {
                  isSliding[j]=true;
                  if (k<31) {
                    if (fp.val[k+1]&0x20) {
                      p->data[k][4]=2;
                      p->data[k][5]=fp.val[k+1]&0x1f;
                    } else {
                      p->data[k][4]=1;
                      p->data[k][5]=fp.val[k+1]&0x1f;
                    }
                    ignoreNext=true;
                  } else {
                    p->data[k][4]=2;
                    p->data[k][5]=0;
                  }
                }
              } else {
                p->data[k][2]=(fp.val[k]+seq[i].offsetIns[j])&0x3f;
                lastIns[j]=p->data[k][2];
              }
            }
          } else if (fp.note[k]>0 && fp.note[k]<0x49) {
            p->data[k][2]=seq[i].offsetIns[j];
            lastIns[j]=p->data[k][2];
          }
        }
      }
    }

    // convert instruments
    for (unsigned int i=0; i<volMacroLen; i++) {
      DivInstrument* ins=new DivInstrument;
      FCMacro& m=volMacros[i];
      
      ins->type=DIV_INS_AMIGA;
      ins->name=fmt::sprintf("Instrument %d",i);
      ins->amiga.useWave=true;
      unsigned char seqSpeed=m.val[0];
      unsigned char freqMacro=m.val[1];
      unsigned char vibSpeed=m.val[2];
      unsigned char vibDepth=m.val[3];
      unsigned char vibDelay=m.val[4];

      unsigned char lastVal=m.val[5];

      signed char loopMap[64];
      memset(loopMap,-1,64);

      signed char loopMapFreq[64];
      memset(loopMapFreq,-1,64);

      signed char loopMapWave[64];
      memset(loopMapWave,-1,64);

      // volume sequence
      ins->std.volMacro.len=0;
      for (int j=5; j<64; j++) {
        loopMap[j]=ins->std.volMacro.len;
        if (m.val[j]==0xe1) { // end
          break;
        } else if (m.val[j]==0xe0) { // loop
          if (++j>=64) break;
          ins->std.volMacro.loop=loopMap[m.val[j]&63];
          break;
        } else if (m.val[j]==0xe8) { // sustain
          if (++j>=64) break;
          unsigned char susTime=m.val[j];
          // TODO: <= or <?
          for (int k=0; k<=susTime; k++) {
            ins->std.volMacro.val[ins->std.volMacro.len]=lastVal;
            if (++ins->std.volMacro.len>=255) break;
          }
          if (ins->std.volMacro.len>=255) break;
        } else if (m.val[j]==0xe9 || m.val[j]==0xea) { // volume slide
          if (++j>=64) break;
          signed char slideStep=m.val[j];
          if (++j>=64) break;
          unsigned char slideTime=m.val[j];
          // TODO: <= or <?
          for (int k=0; k<=slideTime; k++) {
            if (slideStep>0) {
              lastVal+=slideStep;
              if (lastVal>63) lastVal=63;
            } else {
              if (-slideStep>lastVal) {
                lastVal=0;
              } else {
                lastVal-=slideStep;
              }
            }
            ins->std.volMacro.val[ins->std.volMacro.len]=lastVal;
            if (++ins->std.volMacro.len>=255) break;
          }
        } else {
          // TODO: replace with upcoming macro speed
          for (int k=0; k<MAX(1,seqSpeed); k++) {
            ins->std.volMacro.val[ins->std.volMacro.len]=m.val[j];
            lastVal=m.val[j];
            if (++ins->std.volMacro.len>=255) break;
          }
          if (ins->std.volMacro.len>=255) break;
        }
      }

      // frequency sequence
      lastVal=0;
      ins->amiga.initSample=-1;
      if (freqMacro<freqMacros.size()) {
        FCMacro& fm=freqMacros[freqMacro];
        for (int j=0; j<64; j++) {
          loopMapFreq[j]=ins->std.arpMacro.len;
          loopMapWave[j]=ins->std.waveMacro.len;
          if (fm.val[j]==0xe1) {
            break;
          } else if (fm.val[j]==0xe2 || fm.val[j]==0xe4) {
            if (++j>=64) break;
            unsigned char wave=fm.val[j];
            if (wave<10) { // sample
              if (ins->amiga.initSample==-1) {
                ins->amiga.initSample=wave;
                ins->amiga.useWave=false;
              }
            } else { // waveform
              ins->std.waveMacro.val[ins->std.waveMacro.len]=wave-10;
              ins->std.waveMacro.open=true;
              lastVal=wave;
              //if (++ins->std.arpMacro.len>=255) break;
            }
          } else if (fm.val[j]==0xe0) {
            if (++j>=64) break;
            ins->std.arpMacro.loop=loopMapFreq[fm.val[j]&63];
            ins->std.waveMacro.loop=loopMapWave[fm.val[j]&63];
            break;
          } else if (fm.val[j]==0xe3) {
            logV("unhandled vibrato!");
          } else if (fm.val[j]==0xe8) {
            logV("unhandled sustain!");
          } else if (fm.val[j]==0xe7) {
            if (++j>=64) break;
            fm=freqMacros[MIN(fm.val[j],freqMacros.size()-1)];
            j=0;
          } else if (fm.val[j]==0xe9) {
            logV("unhandled pack!");
          } else if (fm.val[j]==0xea) {
            logV("unhandled pitch!");
          } else {
            if (fm.val[j]>0x80) {
              ins->std.arpMacro.val[ins->std.arpMacro.len]=(fm.val[j]-0x80+24)^0x40000000;
            } else {
              ins->std.arpMacro.val[ins->std.arpMacro.len]=fm.val[j];
            }
            if (lastVal>=10) {
              ins->std.waveMacro.val[ins->std.waveMacro.len]=lastVal-10;
            }
            ins->std.arpMacro.open=true;
            if (++ins->std.arpMacro.len>=255) break;
            if (++ins->std.waveMacro.len>=255) break;
          }
        }
      }

      // waveform width
      if (lastVal>=10 && (unsigned int)(lastVal-10)<ds.wave.size()) {
        ins->amiga.waveLen=ds.wave[lastVal-10]->len-1;
      }

      // vibrato
      for (int j=0; j<=vibDelay; j++) {
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=0;
        if (++ins->std.pitchMacro.len>=255) break;
      }
      int vibPos=0;
      ins->std.pitchMacro.loop=ins->std.pitchMacro.len;
      do {
        vibPos+=vibSpeed;
        if (vibPos>vibDepth) vibPos=vibDepth;
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=vibPos*32;
        if (++ins->std.pitchMacro.len>=255) break;
      } while (vibPos<vibDepth);
      do {
        vibPos-=vibSpeed;
        if (vibPos<-vibDepth) vibPos=-vibDepth;
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=vibPos*32;
        if (++ins->std.pitchMacro.len>=255) break;
      } while (vibPos>-vibDepth);
      do {
        vibPos+=vibSpeed;
        if (vibPos>0) vibPos=0;
        ins->std.pitchMacro.val[ins->std.pitchMacro.len]=vibPos*32;
        if (++ins->std.pitchMacro.len>=255) break;
      } while (vibPos<0);

      ds.ins.push_back(ins);
    }
    ds.insLen=(int)ds.ins.size();

    // optimize
    ds.subsong[0]->optimizePatterns();
    ds.subsong[0]->rearrangePatterns();

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
    success=true;
  } catch (EndOfFileException& e) {
    //logE("premature end of file!");
    lastError="incomplete file";
  } catch (InvalidHeaderException& e) {
    //logE("invalid header!");
    lastError="invalid header!";
  }
  return success;
}

#define CHECK_BLOCK_VERSION(x) \
  if (blockVersion>x) { \
    logE("incompatible block version %d for %s!",blockVersion,blockName); \
    lastError="incompatible block version"; \
    delete[] file; \
    return false; \
  }

bool DivEngine::loadFTM(unsigned char* file, size_t len) {
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;
    String blockName;
    unsigned char expansions=0;
    unsigned int tchans=0;
    unsigned int n163Chans=0;
    bool hasSequence[256][8];
    unsigned char sequenceIndex[256][8];
    
    memset(hasSequence,0,256*8*sizeof(bool));
    memset(sequenceIndex,0,256*8);

    if (!reader.seek(18,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=(unsigned short)reader.readI();
    logI("module version %d (0x%.4x)",ds.version,ds.version);

    if (ds.version>0x0450) {
      logE("incompatible version %x!",ds.version);
      lastError="incompatible version";
      delete[] file;
      return false;
    }

    while (true) {
      blockName=reader.readString(3);
      if (blockName=="END") {
        // end of module
        logD("end of data");
        break;
      }

      // not the end
      reader.seek(-3,SEEK_CUR);
      blockName=reader.readString(16);
      unsigned int blockVersion=(unsigned int)reader.readI();
      unsigned int blockSize=(unsigned int)reader.readI();
      size_t blockStart=reader.tell();
      
      logD("reading block %s (version %d, %d bytes)",blockName,blockVersion,blockSize);
      if (blockName=="PARAMS") {
        CHECK_BLOCK_VERSION(6);
        unsigned int oldSpeedTempo=0;
        if (blockVersion<=1) {
          oldSpeedTempo=reader.readI();
        }
        if (blockVersion>=2) {
          expansions=reader.readC();
        }
        tchans=reader.readI();
        unsigned int pal=reader.readI();
        unsigned int customHz=reader.readI();
        unsigned int newVibrato=0;
        unsigned int speedSplitPoint=0;
        if (blockVersion>=3) {
          newVibrato=reader.readI();
        }
        if (blockVersion>=4) {
          ds.subsong[0]->hilightA=reader.readI();
          ds.subsong[0]->hilightB=reader.readI();
        }
        if (expansions&8) if (blockVersion>=5) { // N163 channels
          n163Chans=reader.readI();
        }
        if (blockVersion>=6) {
          speedSplitPoint=reader.readI();
        }

        logV("old speed/tempo: %d",oldSpeedTempo);
        logV("expansions: %x",expansions);
        logV("channels: %d",tchans);
        logV("PAL: %d",pal);
        logV("custom Hz: %d",customHz);
        logV("new vibrato: %d",newVibrato);
        logV("N163 channels: %d",n163Chans);
        logV("highlight 1: %d",ds.subsong[0]->hilightA);
        logV("highlight 2: %d",ds.subsong[0]->hilightB);
        logV("split point: %d",speedSplitPoint);

        if (customHz!=0) {
          ds.subsong[0]->hz=customHz;
        }

        // initialize channels
        int systemID=0;
        ds.system[systemID++]=DIV_SYSTEM_NES;
        if (expansions&1) {
          ds.system[systemID++]=DIV_SYSTEM_VRC6;
        }
        if (expansions&2) {
          ds.system[systemID++]=DIV_SYSTEM_VRC7;
        }
        if (expansions&4) {
          ds.system[systemID++]=DIV_SYSTEM_FDS;
        }
        if (expansions&8) {
          ds.system[systemID++]=DIV_SYSTEM_MMC5;
        }
        if (expansions&16) {
          ds.system[systemID]=DIV_SYSTEM_N163;
          ds.systemFlags[systemID++].set("channels",(int)n163Chans);
        }
        if (expansions&32) {
          ds.system[systemID]=DIV_SYSTEM_AY8910;
          ds.systemFlags[systemID++].set("chipType",2); // Sunsoft 5B
        }
        ds.systemLen=systemID;

        unsigned int calcChans=0;
        for (int i=0; i<ds.systemLen; i++) {
          calcChans+=getChannelCount(ds.system[i]);
        }
        if (calcChans!=tchans) {
          logE("channel counts do not match! %d != %d",tchans,calcChans);
          lastError="channel counts do not match";
          delete[] file;
          return false;
        }
        if (tchans>DIV_MAX_CHANS) {
          tchans=DIV_MAX_CHANS;
          logW("too many channels!");
        }
      } else if (blockName=="INFO") {
        CHECK_BLOCK_VERSION(1);
        ds.name=reader.readString(32);
        ds.author=reader.readString(32);
        ds.copyright=reader.readString(32);
      } else if (blockName=="HEADER") {
        CHECK_BLOCK_VERSION(3);
        unsigned char totalSongs=reader.readC();
        logV("%d songs:",totalSongs+1);
        for (int i=0; i<=totalSongs; i++) {
          String subSongName=reader.readString();
          logV("- %s",subSongName);
        }
        for (unsigned int i=0; i<tchans; i++) {
          unsigned char chID=reader.readC();
          logV("for channel ID %d",chID);
          for (int j=0; j<=totalSongs; j++) {
            unsigned char effectCols=reader.readC();
            if (j==0) {
              ds.subsong[0]->pat[i].effectCols=effectCols+1;
            }
            logV("- song %d has %d effect columns",j,effectCols);
          }
        }
      } else if (blockName=="INSTRUMENTS") {
        CHECK_BLOCK_VERSION(6);
        ds.insLen=reader.readI();
        if (ds.insLen<0 || ds.insLen>256) {
          logE("too many instruments/out of range!");
          lastError="too many instruments/out of range";
          delete[] file;
          return false;
        }

        for (int i=0; i<ds.insLen; i++) {
          DivInstrument* ins=new DivInstrument;
          ds.ins.push_back(ins);
        }

        logV("instruments:");
        for (int i=0; i<ds.insLen; i++) {
          unsigned int insIndex=reader.readI();
          if (insIndex>=ds.ins.size()) {
            logE("instrument index %d is out of range!",insIndex);
            lastError="instrument index out of range";
            delete[] file;
            return false;
          }

          DivInstrument* ins=ds.ins[insIndex];
          unsigned char insType=reader.readC();
          switch (insType) {
            case 1:
              ins->type=DIV_INS_NES;
              break;
            case 2: // TODO: tell VRC6 and VRC6 saw instruments apart
              ins->type=DIV_INS_VRC6;
              break;
            case 3:
              ins->type=DIV_INS_OPLL;
              break;
            case 4:
              ins->type=DIV_INS_FDS;
              break;
            case 5:
              ins->type=DIV_INS_N163;
              break;
            case 6: // 5B?
              ins->type=DIV_INS_AY;
              break;
            default: {
              logE("%d: invalid instrument type %d",insIndex,insType);
              lastError="invalid instrument type";
              delete[] file;
              return false;
            }
          }

          // instrument data
          switch (ins->type) {
            case DIV_INS_NES: {
              unsigned int totalSeqs=reader.readI();
              if (totalSeqs>5) {
                logE("%d: too many sequences!",insIndex);
                lastError="too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j=0; j<totalSeqs; j++) {
                hasSequence[insIndex][j]=reader.readC();
                sequenceIndex[insIndex][j]=reader.readC();
              }

              const int dpcmNotes=(blockVersion>=2)?96:72;
              for (int j=0; j<dpcmNotes; j++) {
                ins->amiga.noteMap[j].map=(short)((unsigned char)reader.readC())-1;
                ins->amiga.noteMap[j].freq=(unsigned char)reader.readC();
                if (blockVersion>=6) {
                  reader.readC(); // DMC value
                }
              }
              break;
            }
            case DIV_INS_VRC6: {
              unsigned int totalSeqs=reader.readI();
              if (totalSeqs>4) {
                logE("%d: too many sequences!",insIndex);
                lastError="too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j=0; j<totalSeqs; j++) {
                hasSequence[insIndex][j]=reader.readC();
                sequenceIndex[insIndex][j]=reader.readC();
              }
              break;
            }
            case DIV_INS_OPLL: {
              ins->fm.opllPreset=(unsigned int)reader.readI();
              // TODO
              break;
            }
            case DIV_INS_FDS: {
              DivWavetable* wave=new DivWavetable;
              wave->len=64;
              wave->max=64;
              for (int j=0; j<64; j++) {
                wave->data[j]=reader.readC();
              }
              ins->std.waveMacro.len=1;
              ins->std.waveMacro.val[0]=ds.wave.size();
              for (int j=0; j<32; j++) {
                ins->fds.modTable[j]=reader.readC()-3;
              }
              ins->fds.modSpeed=reader.readI();
              ins->fds.modDepth=reader.readI();
              reader.readI(); // this is delay. currently ignored. TODO.
              ds.wave.push_back(wave);

              ins->std.volMacro.len=reader.readC();
              ins->std.volMacro.loop=reader.readI();
              ins->std.volMacro.rel=reader.readI();
              reader.readI(); // arp mode does not apply here
              for (int j=0; j<ins->std.volMacro.len; j++) {
                ins->std.volMacro.val[j]=reader.readC();
              }

              ins->std.arpMacro.len=reader.readC();
              ins->std.arpMacro.loop=reader.readI();
              ins->std.arpMacro.rel=reader.readI();
              // TODO: get rid
              ins->std.arpMacro.mode=reader.readI();
              for (int j=0; j<ins->std.arpMacro.len; j++) {
                ins->std.arpMacro.val[j]=reader.readC();
              }

              ins->std.pitchMacro.len=reader.readC();
              ins->std.pitchMacro.loop=reader.readI();
              ins->std.pitchMacro.rel=reader.readI();
              reader.readI(); // arp mode does not apply here
              for (int j=0; j<ins->std.pitchMacro.len; j++) {
                ins->std.pitchMacro.val[j]=reader.readC();
              }

              break;
            }
            case DIV_INS_N163: {
              // TODO!
              break;
            }
            // TODO: 5B!
            default: {
              logE("%d: what's going on here?",insIndex);
              lastError="invalid instrument type";
              delete[] file;
              return false;
            }
          }

          // name
          ins->name=reader.readString((unsigned int)reader.readI());
          logV("- %d: %s",insIndex,ins->name);
        }
      } else if (blockName=="SEQUENCES") {
        CHECK_BLOCK_VERSION(6);
      } else if (blockName=="FRAMES") {
        CHECK_BLOCK_VERSION(3);
      } else if (blockName=="PATTERNS") {
        CHECK_BLOCK_VERSION(5);
      } else if (blockName=="DPCM SAMPLES") {
        CHECK_BLOCK_VERSION(1);
      } else if (blockName=="SEQUENCES_VRC6") {
        // where are the 5B and FDS sequences?
        CHECK_BLOCK_VERSION(6);
      } else if (blockName=="SEQUENCES_N163") {
        CHECK_BLOCK_VERSION(1);
      } else if (blockName=="COMMENTS") {
        CHECK_BLOCK_VERSION(1);
      } else {
        logE("block %s is unknown!",blockName);
        lastError="unknown block "+blockName;
        delete[] file;
        return false;
      }

      if ((reader.tell()-blockStart)!=blockSize) {
        logE("block %s is incomplete!",blockName);
        lastError="incomplete block "+blockName;
        delete[] file;
        return false;
      }
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

bool DivEngine::load(unsigned char* f, size_t slen) {
  unsigned char* file;
  size_t len;
  if (slen<18) {
    logE("too small!");
    lastError="file is too small";
    delete[] f;
    return false;
  }

  if (!systemsRegistered) registerSystems();

  // step 1: try loading as a zlib-compressed file
  logD("trying zlib...");
  try {
    z_stream zl;
    memset(&zl,0,sizeof(z_stream));

    zl.avail_in=slen;
    zl.next_in=(Bytef*)f;
    zl.zalloc=NULL;
    zl.zfree=NULL;
    zl.opaque=NULL;

    int nextErr;
    nextErr=inflateInit(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logD("zlib error: unknown! %d",nextErr);
      } else {
        logD("zlib error: %s",zl.msg);
      }
      inflateEnd(&zl);
      lastError="not a .dmf/.fur song";
      throw NotZlibException(0);
    }

    std::vector<InflateBlock*> blocks;
    while (true) {
      InflateBlock* ib=new InflateBlock(DIV_READ_SIZE);
      zl.next_out=ib->buf;
      zl.avail_out=ib->len;

      nextErr=inflate(&zl,Z_SYNC_FLUSH);
      if (nextErr!=Z_OK && nextErr!=Z_STREAM_END) {
        if (zl.msg==NULL) {
          logD("zlib error: unknown error! %d",nextErr);
          lastError="unknown decompression error";
        } else {
          logD("zlib inflate: %s",zl.msg);
          lastError=fmt::sprintf("decompression error: %s",zl.msg);
        }
        for (InflateBlock* i: blocks) delete i;
        blocks.clear();
        delete ib;
        inflateEnd(&zl);
        throw NotZlibException(0);
      }
      ib->blockSize=ib->len-zl.avail_out;
      blocks.push_back(ib);
      if (nextErr==Z_STREAM_END) {
        break;
      }
    }
    nextErr=inflateEnd(&zl);
    if (nextErr!=Z_OK) {
      if (zl.msg==NULL) {
        logD("zlib end error: unknown error! %d",nextErr);
        lastError="unknown decompression finish error";
      } else {
        logD("zlib end: %s",zl.msg);
        lastError=fmt::sprintf("decompression finish error: %s",zl.msg);
      }
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      throw NotZlibException(0);
    }

    size_t finalSize=0;
    size_t curSeek=0;
    for (InflateBlock* i: blocks) {
      finalSize+=i->blockSize;
    }
    if (finalSize<1) {
      logD("compressed too small!");
      lastError="file too small";
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      throw NotZlibException(0);
    }
    file=new unsigned char[finalSize];
    for (InflateBlock* i: blocks) {
      memcpy(&file[curSeek],i->buf,i->blockSize);
      curSeek+=i->blockSize;
      delete i;
    }
    blocks.clear();
    len=finalSize;
    delete[] f;
  } catch (NotZlibException& e) {
    logD("not zlib. loading as raw...");
    file=f;
    len=slen;
  }

  // step 2: try loading as .fur or .dmf
  if (memcmp(file,DIV_DMF_MAGIC,16)==0) {
    return loadDMF(file,len); 
  } else if (memcmp(file,DIV_FTM_MAGIC,18)==0) {
    return loadFTM(file,len);
  } else if (memcmp(file,DIV_FUR_MAGIC,16)==0) {
    return loadFur(file,len);
  } else if (memcmp(file,DIV_FC13_MAGIC,4)==0 || memcmp(file,DIV_FC14_MAGIC,4)==0) {
    return loadFC(file,len);
  }

  // step 3: try loading as .mod
  if (loadMod(f,slen)) {
    delete[] f;
    return true;
  }
  
  // step 4: not a valid file
  logE("not a valid module!");
  lastError="not a compatible song";
  delete[] file;
  return false;
}

struct PatToWrite {
  unsigned short subsong, chan, pat;
  PatToWrite(unsigned short s, unsigned short c, unsigned short p):
    subsong(s),
    chan(c),
    pat(p) {}
};

SafeWriter* DivEngine::saveFur(bool notPrimary) {
  saveLock.lock();
  std::vector<int> subSongPtr;
  std::vector<int> sysFlagsPtr;
  std::vector<int> insPtr;
  std::vector<int> wavePtr;
  std::vector<int> samplePtr;
  std::vector<int> patPtr;
  size_t ptrSeek, subSongPtrSeek, sysFlagsPtrSeek, blockStartSeek, blockEndSeek;
  size_t subSongIndex=0;
  DivSubSong* subSong=song.subsong[subSongIndex];
  warnings="";

  // fail if values are out of range
  /*
  if (subSong->ordersLen>DIV_MAX_PATTERNS) {
    logE("maximum song length is %d!",DIV_MAX_PATTERNS);
    lastError=fmt::sprintf("maximum song length is %d",DIV_MAX_PATTERNS);
    return NULL;
  }
  if (subSong->patLen>DIV_MAX_ROWS) {
    logE("maximum pattern length is %d!",DIV_MAX_ROWS);
    lastError=fmt::sprintf("maximum pattern length is %d",DIV_MAX_ROWS);
    return NULL;
  }
  */
  if (song.ins.size()>256) {
    logE("maximum number of instruments is 256!");
    lastError="maximum number of instruments is 256";
    saveLock.unlock();
    return NULL;
  }
  if (song.wave.size()>256) {
    logE("maximum number of wavetables is 256!");
    lastError="maximum number of wavetables is 256";
    saveLock.unlock();
    return NULL;
  }
  if (song.sample.size()>256) {
    logE("maximum number of samples is 256!");
    lastError="maximum number of samples is 256";
    saveLock.unlock();
    return NULL;
  }

  if (!notPrimary) {
    song.isDMF=false;
    song.version=DIV_ENGINE_VERSION;
  }

  SafeWriter* w=new SafeWriter;
  w->init();
  /// HEADER
  // write magic
  w->write(DIV_FUR_MAGIC,16);

  // write version
  w->writeS(DIV_ENGINE_VERSION);

  // reserved
  w->writeS(0);

  // song info pointer
  w->writeI(32);

  // reserved
  w->writeI(0);
  w->writeI(0);

  // high short is channel
  // low short is pattern number
  std::vector<PatToWrite> patsToWrite;
  if (getConfInt("saveUnusedPatterns",0)==1) {
    for (int i=0; i<chans; i++) {
      for (size_t j=0; j<song.subsong.size(); j++) {
        DivSubSong* subs=song.subsong[j];
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (subs->pat[i].data[k]==NULL) continue;
          patsToWrite.push_back(PatToWrite(j,i,k));
        }
      }
    }
  } else {
    bool alreadyAdded[DIV_MAX_PATTERNS];
    for (int i=0; i<chans; i++) {
      for (size_t j=0; j<song.subsong.size(); j++) {
        DivSubSong* subs=song.subsong[j];
        memset(alreadyAdded,0,DIV_MAX_PATTERNS*sizeof(bool));
        for (int k=0; k<subs->ordersLen; k++) {
          if (alreadyAdded[subs->orders.ord[i][k]]) continue;
          patsToWrite.push_back(PatToWrite(j,i,subs->orders.ord[i][k]));
          alreadyAdded[subs->orders.ord[i][k]]=true;
        }
      }
    }
  }

  /// SONG INFO
  w->write("INFO",4);
  blockStartSeek=w->tell();
  w->writeI(0);

  w->writeC(subSong->timeBase);
  w->writeC(subSong->speed1);
  w->writeC(subSong->speed2);
  w->writeC(subSong->arpLen);
  w->writeF(subSong->hz);
  w->writeS(subSong->patLen);
  w->writeS(subSong->ordersLen);
  w->writeC(subSong->hilightA);
  w->writeC(subSong->hilightB);
  w->writeS(song.insLen);
  w->writeS(song.waveLen);
  w->writeS(song.sampleLen);
  w->writeI(patsToWrite.size());

  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    if (i>=song.systemLen) {
      w->writeC(0);
    } else {
      w->writeC(systemToFileFur(song.system[i]));
    }
  }

  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    w->writeC(song.systemVol[i]);
  }

  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    w->writeC(song.systemPan[i]);
  }

  // chip flags (we'll seek here later)
  sysFlagsPtrSeek=w->tell();
  for (int i=0; i<DIV_MAX_CHIPS; i++) {
    w->writeI(0);
  }

  // song name
  w->writeString(song.name,false);
  // song author
  w->writeString(song.author,false);

  w->writeF(song.tuning);
  
  // compatibility flags
  w->writeC(song.limitSlides);
  w->writeC(song.linearPitch);
  w->writeC(song.loopModality);
  w->writeC(song.properNoiseLayout);
  w->writeC(song.waveDutyIsVol);
  w->writeC(song.resetMacroOnPorta);
  w->writeC(song.legacyVolumeSlides);
  w->writeC(song.compatibleArpeggio);
  w->writeC(song.noteOffResetsSlides);
  w->writeC(song.targetResetsSlides);
  w->writeC(song.arpNonPorta);
  w->writeC(song.algMacroBehavior);
  w->writeC(song.brokenShortcutSlides);
  w->writeC(song.ignoreDuplicateSlides);
  w->writeC(song.stopPortaOnNoteOff);
  w->writeC(song.continuousVibrato);
  w->writeC(song.brokenDACMode);
  w->writeC(song.oneTickCut);
  w->writeC(song.newInsTriggersInPorta);
  w->writeC(song.arp0Reset);

  ptrSeek=w->tell();
  // instrument pointers (we'll seek here later)
  for (int i=0; i<song.insLen; i++) {
    w->writeI(0);
  }

  // wavetable pointers (we'll seek here later)
  for (int i=0; i<song.waveLen; i++) {
    w->writeI(0);
  }

  // sample pointers (we'll seek here later)
  for (int i=0; i<song.sampleLen; i++) {
    w->writeI(0);
  }

  // pattern pointers (we'll seek here later)
  for (size_t i=0; i<patsToWrite.size(); i++) {
    w->writeI(0);
  }

  for (int i=0; i<chans; i++) {
    for (int j=0; j<subSong->ordersLen; j++) {
      w->writeC(subSong->orders.ord[i][j]);
    }
  }

  for (int i=0; i<chans; i++) {
    w->writeC(subSong->pat[i].effectCols);
  }

  for (int i=0; i<chans; i++) {
    w->writeC(subSong->chanShow[i]);
  }

  for (int i=0; i<chans; i++) {
    w->writeC(subSong->chanCollapse[i]);
  }

  for (int i=0; i<chans; i++) {
    w->writeString(subSong->chanName[i],false);
  }

  for (int i=0; i<chans; i++) {
    w->writeString(subSong->chanShortName[i],false);
  }

  w->writeString(song.notes,false);

  w->writeF(song.masterVol);

  // extended compat flags
  w->writeC(song.brokenSpeedSel);
  w->writeC(song.noSlidesOnFirstTick);
  w->writeC(song.rowResetsArpPos);
  w->writeC(song.ignoreJumpAtEnd);
  w->writeC(song.buggyPortaAfterSlide);
  w->writeC(song.gbInsAffectsEnvelope);
  w->writeC(song.sharedExtStat);
  w->writeC(song.ignoreDACModeOutsideIntendedChannel);
  w->writeC(song.e1e2AlsoTakePriority);
  w->writeC(song.newSegaPCM);
  w->writeC(song.fbPortaPause);
  w->writeC(song.snDutyReset);
  w->writeC(song.pitchMacroIsLinear);
  w->writeC(song.pitchSlideSpeed);
  w->writeC(song.oldOctaveBoundary);
  w->writeC(song.noOPN2Vol);
  w->writeC(song.newVolumeScaling);
  w->writeC(song.volMacroLinger);
  w->writeC(song.brokenOutVol);
  w->writeC(song.e1e2StopOnSameNote);
  w->writeC(song.brokenPortaArp);
  w->writeC(song.snNoLowPeriods);
  w->writeC(song.delayBehavior);
  w->writeC(song.jumpTreatment);
  w->writeC(song.autoSystem);
  w->writeC(song.disableSampleMacro);
  w->writeC(song.brokenOutVol2);
  w->writeC(song.oldArpStrategy);

  // first subsong virtual tempo
  w->writeS(subSong->virtualTempoN);
  w->writeS(subSong->virtualTempoD);
  
  // subsong list
  w->writeString(subSong->name,false);
  w->writeString(subSong->notes,false);
  w->writeC((unsigned char)(song.subsong.size()-1));
  w->writeC(0); // reserved
  w->writeC(0);
  w->writeC(0);
  subSongPtrSeek=w->tell();
  // subsong pointers (we'll seek here later)
  for (size_t i=0; i<(song.subsong.size()-1); i++) {
    w->writeI(0);
  }

  // additional metadata
  w->writeString(song.systemName,false);
  w->writeString(song.category,false);
  w->writeString(song.nameJ,false);
  w->writeString(song.authorJ,false);
  w->writeString(song.systemNameJ,false);
  w->writeString(song.categoryJ,false);

  blockEndSeek=w->tell();
  w->seek(blockStartSeek,SEEK_SET);
  w->writeI(blockEndSeek-blockStartSeek-4);
  w->seek(0,SEEK_END);

  /// SUBSONGS
  for (subSongIndex=1; subSongIndex<song.subsong.size(); subSongIndex++) {
    subSong=song.subsong[subSongIndex];
    subSongPtr.push_back(w->tell());
    w->write("SONG",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeC(subSong->timeBase);
    w->writeC(subSong->speed1);
    w->writeC(subSong->speed2);
    w->writeC(subSong->arpLen);
    w->writeF(subSong->hz);
    w->writeS(subSong->patLen);
    w->writeS(subSong->ordersLen);
    w->writeC(subSong->hilightA);
    w->writeC(subSong->hilightB);
    w->writeS(subSong->virtualTempoN);
    w->writeS(subSong->virtualTempoD);

    w->writeString(subSong->name,false);
    w->writeString(subSong->notes,false);

    for (int i=0; i<chans; i++) {
      for (int j=0; j<subSong->ordersLen; j++) {
        w->writeC(subSong->orders.ord[i][j]);
      }
    }

    for (int i=0; i<chans; i++) {
      w->writeC(subSong->pat[i].effectCols);
    }

    for (int i=0; i<chans; i++) {
      w->writeC(subSong->chanShow[i]);
    }

    for (int i=0; i<chans; i++) {
      w->writeC(subSong->chanCollapse[i]);
    }

    for (int i=0; i<chans; i++) {
      w->writeString(subSong->chanName[i],false);
    }

    for (int i=0; i<chans; i++) {
      w->writeString(subSong->chanShortName[i],false);
    }

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// CHIP FLAGS
  for (int i=0; i<song.systemLen; i++) {
    String data=song.systemFlags[i].toString();
    if (data.empty()) {
      sysFlagsPtr.push_back(0);
      continue;
    }

    sysFlagsPtr.push_back(w->tell());
    w->write("FLAG",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeString(data,false);

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// INSTRUMENT
  for (int i=0; i<song.insLen; i++) {
    DivInstrument* ins=song.ins[i];
    insPtr.push_back(w->tell());
    logV("writing instrument %d...",i);
    ins->putInsData2(w,false);
  }

  /// WAVETABLE
  for (int i=0; i<song.waveLen; i++) {
    DivWavetable* wave=song.wave[i];
    wavePtr.push_back(w->tell());
    wave->putWaveData(w);
  }

  /// SAMPLE
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    samplePtr.push_back(w->tell());
    sample->putSampleData(w);
  }

  /// PATTERN
  for (PatToWrite& i: patsToWrite) {
    DivPattern* pat=song.subsong[i.subsong]->pat[i.chan].getPattern(i.pat,false);
    patPtr.push_back(w->tell());
    w->write("PATR",4);
    blockStartSeek=w->tell();
    w->writeI(0);

    w->writeS(i.chan);
    w->writeS(i.pat);
    w->writeS(i.subsong);

    w->writeS(0); // reserved

    for (int j=0; j<song.subsong[i.subsong]->patLen; j++) {
      w->writeS(pat->data[j][0]); // note
      w->writeS(pat->data[j][1]); // octave
      w->writeS(pat->data[j][2]); // instrument
      w->writeS(pat->data[j][3]); // volume
#ifdef TA_BIG_ENDIAN
      for (int k=0; k<song.subsong[i.subsong]->pat[i.chan].effectCols*2; k++) {
        w->writeS(pat->data[j][4+k]);
      }
#else
      w->write(&pat->data[j][4],2*song.subsong[i.subsong]->pat[i.chan].effectCols*2); // effects
#endif
    }

    w->writeString(pat->name,false);

    blockEndSeek=w->tell();
    w->seek(blockStartSeek,SEEK_SET);
    w->writeI(blockEndSeek-blockStartSeek-4);
    w->seek(0,SEEK_END);
  }

  /// POINTERS
  w->seek(ptrSeek,SEEK_SET);

  for (int i=0; i<song.insLen; i++) {
    w->writeI(insPtr[i]);
  }

  // wavetable pointers
  for (int i=0; i<song.waveLen; i++) {
    w->writeI(wavePtr[i]);
  }

  // sample pointers
  for (int i=0; i<song.sampleLen; i++) {
    w->writeI(samplePtr[i]);
  }

  // pattern pointers
  for (int i: patPtr) {
    w->writeI(i);
  }

  // subsong pointers
  w->seek(subSongPtrSeek,SEEK_SET);
  for (size_t i=0; i<(song.subsong.size()-1); i++) {
    w->writeI(subSongPtr[i]);
  }

  // flag pointers
  w->seek(sysFlagsPtrSeek,SEEK_SET);
  for (size_t i=0; i<sysFlagsPtr.size(); i++) {
    w->writeI(sysFlagsPtr[i]);
  }

  saveLock.unlock();
  return w;
}

SafeWriter* DivEngine::saveDMF(unsigned char version) {
  // fail if version is not supported
  if (version<24 || version>26) {
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
  
  w->writeC(curSubSong->timeBase);
  w->writeC(curSubSong->speed1);
  w->writeC(curSubSong->speed2);
  w->writeC(curSubSong->pal);
  w->writeC(curSubSong->customTempo);
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
      if (sys!=DIV_SYSTEM_GB) {
        int realVolMacroLen=i->std.volMacro.len;
        if (realVolMacroLen>127) realVolMacroLen=127;
        w->writeC(realVolMacroLen);
        if ((sys==DIV_SYSTEM_C64_6581 || sys==DIV_SYSTEM_C64_8580) && i->c64.volIsCutoff) {
          for (int j=0; j<realVolMacroLen; j++) {
            w->writeI(i->std.volMacro.val[j]+18);
          }
        } else {
          for (int j=0; j<realVolMacroLen; j++) {
            w->writeI(i->std.volMacro.val[j]);
          }
        }
        if (realVolMacroLen>0) {
          w->writeC(i->std.volMacro.loop);
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
        w->writeI(i->std.waveMacro.val[j]);
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
        w->writeC(i->c64.volIsCutoff);
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

