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
#include "song.h"
#include <zlib.h>
#include <fmt/printf.h>

#define DIV_READ_SIZE 131072
#define DIV_DMF_MAGIC ".DelekDefleMask."
#define DIV_FUR_MAGIC "-Furnace module-"

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

bool DivEngine::loadDMF(unsigned char* file, size_t len) {
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;

    ds.nullWave.len=32;
    for (int i=0; i<32; i++) {
      ds.nullWave.data[i]=15;
    }

    ds.isDMF=true;

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!\n");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=(unsigned char)reader.readC();
    logI("module version %d (0x%.2x)\n",ds.version,ds.version);
    if (ds.version>0x19) {
      logE("this version is not supported by Furnace yet!\n");
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
      ds.system[0]=systemFromFile(sys);
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
      logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());
      logI("has YMU-specific data:\n");
      logI("- carrier: %s\n",ds.carrier.c_str());
      logI("- category: %s\n",ds.category.c_str());
      logI("- vendor: %s\n",ds.vendor.c_str());
      logI("- writer: %s\n",ds.writer.c_str());
      logI("- composer: %s\n",ds.composer.c_str());
      logI("- arranger: %s\n",ds.arranger.c_str());
      logI("- copyright: %s\n",ds.copyright.c_str());
      logI("- management group: %s\n",ds.manGroup.c_str());
      logI("- management info: %s\n",ds.manInfo.c_str());
      logI("- created on: %s\n",ds.createdDate.c_str());
      logI("- revision date: %s\n",ds.revisionDate.c_str());
    } else {
      ds.name=reader.readString((unsigned char)reader.readC());
      ds.author=reader.readString((unsigned char)reader.readC());
      logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());
    }

    // compatibility flags
    ds.limitSlides=true;
    ds.linearPitch=true;
    ds.loopModality=0;
    ds.properNoiseLayout=false;
    ds.waveDutyIsVol=false;
    ds.resetMacroOnPorta=true;
    ds.legacyVolumeSlides=true;
    ds.compatibleArpeggio=true;
    ds.noteOffResetsSlides=true;
    ds.targetResetsSlides=true;
    ds.arpNonPorta=false;
    ds.algMacroBehavior=false;
    ds.brokenShortcutSlides=false;
    ds.ignoreDuplicateSlides=true;

    // 1.1 compat flags
    if (ds.version>24) {
      ds.waveDutyIsVol=true;
      ds.legacyVolumeSlides=false;
    }

    // Neo Geo detune
    if (ds.system[0]==DIV_SYSTEM_YM2610 || ds.system[0]==DIV_SYSTEM_YM2610_EXT) {
      ds.tuning=443.23;
    }

    logI("reading module data...\n");
    if (ds.version>0x0c) {
      ds.hilightA=reader.readC();
      ds.hilightB=reader.readC();
    }

    ds.timeBase=reader.readC();
    ds.speed1=reader.readC();
    if (ds.version>0x03) {
      ds.speed2=reader.readC();
      ds.pal=reader.readC();
      ds.hz=(ds.pal)?60:50;
      ds.customTempo=reader.readC();
    } else {
      ds.speed2=ds.speed1;
    }
    if (ds.version>0x0a) {
      String hz=reader.readString(3);
      if (ds.customTempo) {
        try {
          ds.hz=std::stoi(hz);
        } catch (std::exception& e) {
          logW("invalid custom Hz!\n");
          ds.hz=60;
        }
      }
    }
    if (ds.version>0x17) {
      ds.patLen=reader.readI();
    } else {
      ds.patLen=(unsigned char)reader.readC();
    }
    ds.ordersLen=(unsigned char)reader.readC();

    if (ds.version<20 && ds.version>3) {
      ds.arpLen=reader.readC();
    } else {
      ds.arpLen=1;
    }

    if (ds.system[0]==DIV_SYSTEM_YMU759) {
      switch (ds.timeBase) {
        case 0:
          ds.hz=248;
          break;
        case 1:
          ds.hz=200;
          break;
        case 2:
          ds.hz=100;
          break;
        case 3:
          ds.hz=50;
          break;
        case 4:
          ds.hz=25;
          break;
        case 5:
          ds.hz=20;
          break;
        default:
          ds.hz=248;
          break;
      }
      ds.customTempo=true;
      ds.timeBase=0;
      addWarning("Yamaha YMU759 emulation is not currently possible!");
    }

    if (ds.system[0]==DIV_SYSTEM_SMS_OPLL) {
      addWarning("Master System FM expansion is not emulated yet. wait for 0.6!");
    }

    if (ds.system[0]==DIV_SYSTEM_NES_VRC7) {
      addWarning("Konami VRC7 is not emulated yet. wait for 0.6!");
    }

    logI("reading pattern matrix (%d)...\n",ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system[0]); i++) {
      for (int j=0; j<ds.ordersLen; j++) {
        ds.orders.ord[i][j]=reader.readC();
        if (ds.version>0x18) { // 1.1 pattern names
          ds.pat[i].getPattern(j,true)->name=reader.readString((unsigned char)reader.readC());
        }
      }
    }

    if (ds.version>0x03) {
      ds.insLen=(unsigned char)reader.readC();
    } else {
      ds.insLen=16;
    }
    logI("reading instruments (%d)...\n",ds.insLen);
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      if (ds.version>0x03) {
        ins->name=reader.readString((unsigned char)reader.readC());
      }
      logD("%d name: %s\n",i,ins->name.c_str());
      if (ds.version<0x0b) {
        // instruments in ancient versions were all FM or STD.
        ins->mode=1;
      } else {
        ins->mode=reader.readC();
      }
      ins->type=ins->mode?DIV_INS_FM:DIV_INS_STD;
      if (ds.system[0]==DIV_SYSTEM_GB) {
        ins->type=DIV_INS_GB;
      }
      if (ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) {
        ins->type=DIV_INS_C64;
      }
      if (ds.system[0]==DIV_SYSTEM_YM2610 || ds.system[0]==DIV_SYSTEM_YM2610_EXT) {
        if (!ins->mode) {
          ins->type=DIV_INS_AY;
        }
        ins->std.dutyMacroHeight=31;
        ins->std.waveMacroHeight=7;
      }
      if (ds.system[0]==DIV_SYSTEM_PCE) {
        ins->type=DIV_INS_PCE;
        ins->std.volMacroHeight=31;
      }
      if ((ds.system[0]==DIV_SYSTEM_SMS_OPLL || ds.system[0]==DIV_SYSTEM_NES_VRC7) && ins->type==DIV_INS_FM) {
        ins->type=DIV_INS_OPLL;
      }

      if (ins->mode) { // FM
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
        if (ins->fm.ops!=2 && ins->fm.ops!=4) {
          logE("invalid op count %d. did we read it wrong?\n",ins->fm.ops);
          lastError="file is corrupt or unreadable at operators";
          delete[] file;
          return false;
        }
        ins->fm.ams=reader.readC();

        for (int j=0; j<ins->fm.ops; j++) {
          ins->fm.op[j].am=reader.readC();
          ins->fm.op[j].ar=reader.readC();
          if (ds.version<0x13) {
            ins->fm.op[j].dam=reader.readC();
          }
          ins->fm.op[j].dr=reader.readC();
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
          if (ds.version>0x03) {
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

          logD("OP%d: AM %d AR %d DAM %d DR %d DVB %d EGT %d KSL %d MULT %d RR %d SL %d SUS %d TL %d VIB %d WS %d RS %d DT %d D2R %d SSG-EG %d\n",j,
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
      } else { // STD
        if (ds.system[0]!=DIV_SYSTEM_GB || ds.version<0x12) {
          ins->std.volMacroLen=reader.readC();
          for (int j=0; j<ins->std.volMacroLen; j++) {
            if (ds.version<0x0e) {
              ins->std.volMacro[j]=reader.readC();
            } else {
              ins->std.volMacro[j]=reader.readI();
            }
          }
          if (ins->std.volMacroLen>0) {
            ins->std.volMacroOpen=true;
            ins->std.volMacroLoop=reader.readC();
          } else {
            ins->std.volMacroOpen=false;
          }
        }

        ins->std.arpMacroLen=reader.readC();
        for (int j=0; j<ins->std.arpMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.arpMacro[j]=reader.readC();
          } else {
            ins->std.arpMacro[j]=reader.readI();
          }
        }
        if (ins->std.arpMacroLen>0) {
          ins->std.arpMacroLoop=reader.readC();
          ins->std.arpMacroOpen=true;
        } else {
          ins->std.arpMacroOpen=false;
        }
        if (ds.version>0x0f) {
          ins->std.arpMacroMode=reader.readC();
        }
        if (!ins->std.arpMacroMode) {
          for (int j=0; j<ins->std.arpMacroLen; j++) {
            ins->std.arpMacro[j]-=12;
          }
        }

        ins->std.dutyMacroLen=reader.readC();
        for (int j=0; j<ins->std.dutyMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.dutyMacro[j]=reader.readC();
          } else {
            ins->std.dutyMacro[j]=reader.readI();
          }
          if ((ds.system[0]==DIV_SYSTEM_C64_8580 || ds.system[0]==DIV_SYSTEM_C64_6581) && ins->std.dutyMacro[j]>24) {
            ins->std.dutyMacro[j]=24;
          }
        }
        if (ins->std.dutyMacroLen>0) {
          ins->std.dutyMacroOpen=true;
          ins->std.dutyMacroLoop=reader.readC();
        } else {
          ins->std.dutyMacroOpen=false;
        }

        ins->std.waveMacroLen=reader.readC();
        for (int j=0; j<ins->std.waveMacroLen; j++) {
          if (ds.version<0x0e) {
            ins->std.waveMacro[j]=reader.readC();
          } else {
            ins->std.waveMacro[j]=reader.readI();
          }
        }
        if (ins->std.waveMacroLen>0) {
          ins->std.waveMacroOpen=true;
          ins->std.waveMacroLoop=reader.readC();
        } else {
          ins->std.waveMacroOpen=false;
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
        }

        if (ds.system[0]==DIV_SYSTEM_GB && ds.version>0x11) {
          ins->gb.envVol=reader.readC();
          ins->gb.envDir=reader.readC();
          ins->gb.envLen=reader.readC();
          ins->gb.soundLen=reader.readC();
          ins->std.volMacroOpen=false;

          logD("GB data: vol %d dir %d len %d sl %d\n",ins->gb.envVol,ins->gb.envDir,ins->gb.envLen,ins->gb.soundLen);
        } else if (ds.system[0]==DIV_SYSTEM_GB) {
          // try to convert macro to envelope
          if (ins->std.volMacroLen>0) {
            ins->gb.envVol=ins->std.volMacro[0];
            if (ins->std.volMacro[0]<ins->std.volMacro[1]) {
              ins->gb.envDir=true;
            }
            if (ins->std.volMacro[ins->std.volMacroLen-1]==0) {
              ins->gb.soundLen=ins->std.volMacroLen*2;
            }
          }
          addWarning("Game Boy volume macros converted to envelopes. may not be perfect!");
        }
      }

      ds.ins.push_back(ins);
    }

    if (ds.version>0x0b) {
      ds.waveLen=(unsigned char)reader.readC();
      logI("reading wavetables (%d)...\n",ds.waveLen);
      for (int i=0; i<ds.waveLen; i++) {
        DivWavetable* wave=new DivWavetable;
        wave->len=(unsigned char)reader.readI();
        if (ds.system[0]==DIV_SYSTEM_GB) {
          wave->max=15;
        }
        if (wave->len>33) {
          logE("invalid wave length %d. are we doing something wrong?\n",wave->len);
          lastError="file is corrupt or unreadable at wavetables";
          delete[] file;
          return false;
        }
        logD("%d length %d\n",i,wave->len);
        for (int j=0; j<wave->len; j++) {
          if (ds.version<0x0e) {
            wave->data[j]=reader.readC();
          } else {
            wave->data[j]=reader.readI();
          }
        }
        ds.wave.push_back(wave);
      }
    }

    logI("reading patterns (%d channels, %d orders)...\n",getChannelCount(ds.system[0]),ds.ordersLen);
    for (int i=0; i<getChannelCount(ds.system[0]); i++) {
      DivChannelData& chan=ds.pat[i];
      if (ds.version<0x0a) {
        chan.effectRows=1;
      } else {
        chan.effectRows=reader.readC();
      }
      logD("%d fx rows: %d\n",i,chan.effectRows);
      if (chan.effectRows>4 || chan.effectRows<1) {
        logE("invalid effect row count %d. are you sure everything is ok?\n",chan.effectRows);
        lastError="file is corrupt or unreadable at effect rows";
        delete[] file;
        return false;
      }
      for (int j=0; j<ds.ordersLen; j++) {
        DivPattern* pat=chan.getPattern(ds.orders.ord[i][j],true);
        for (int k=0; k<ds.patLen; k++) {
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
          if (pat->data[k][0]==0 && pat->data[k][1]!=0) {
            logD("what? %d:%d:%d note %d octave %d\n",i,j,k,pat->data[k][0],pat->data[k][1]);
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
          for (int l=0; l<chan.effectRows; l++) {
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
        }
      }
    }

    ds.sampleLen=(unsigned char)reader.readC();
    logI("reading samples (%d)...\n",ds.sampleLen);
    if (ds.version<0x0b && ds.sampleLen>0) { // TODO what is this for?
      reader.readC();
    }
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* sample=new DivSample;
      sample->length=reader.readI();
      if (sample->length<0) {
        logE("invalid sample length %d. are we doing something wrong?\n",sample->length);
        lastError="file is corrupt or unreadable at samples";
        delete[] file;
        return false;
      }
      if (ds.version>0x16) {
        sample->name=reader.readString((unsigned char)reader.readC());
      } else {
        sample->name="";
      }
      logD("%d name %s (%d)\n",i,sample->name.c_str(),sample->length);
      if (ds.version<0x0b) {
        sample->rate=22050;
        sample->pitch=0;
        sample->vol=0;
      } else {
        sample->rate=fileToDivRate(reader.readC());
        sample->pitch=reader.readC();
        sample->vol=reader.readC();
      }
      if (ds.version>0x15) {
        sample->depth=reader.readC();
      } else {
        sample->depth=16;
      }
      if (sample->length>0) {
        if (ds.version<0x0b) {
          sample->data=new short[1+(sample->length/2)];
          reader.read(sample->data,sample->length);
          sample->length/=2;
        } else {
          sample->data=new short[sample->length];
          reader.read(sample->data,sample->length*2);
        }
      }
      ds.sample.push_back(sample);
    }

    if (reader.tell()<reader.size()) {
      if ((reader.tell()+1)!=reader.size()) {
        logW("premature end of song (we are at %x, but size is %x)\n",reader.tell(),reader.size());
      }
    }

    // handle compound systems
    if (ds.system[0]==DIV_SYSTEM_GENESIS) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_YM2612;
      ds.system[1]=DIV_SYSTEM_SMS;
      ds.systemVol[1]=24;
    }
    if (ds.system[0]==DIV_SYSTEM_GENESIS_EXT) {
      ds.systemLen=2;
      ds.system[0]=DIV_SYSTEM_YM2612_EXT;
      ds.system[1]=DIV_SYSTEM_SMS;
      ds.systemVol[1]=24;
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

    if (active) quitDispatch();
    isBusy.lock();
    song.unload();
    song=ds;
    recalcChans();
    renderSamples();
    isBusy.unlock();
    if (active) {
      initDispatch();
      syncReset();
    }
  } catch (EndOfFileException e) {
    logE("premature end of file!\n");
    lastError="incomplete file";
    delete[] file;
    return false;
  }
  delete[] file;
  return true;
}

bool DivEngine::loadFur(unsigned char* file, size_t len) {
  int insPtr[256];
  int wavePtr[256];
  int samplePtr[256];
  std::vector<int> patPtr;
  char magic[5];
  memset(magic,0,5);
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;

    if (!reader.seek(16,SEEK_SET)) {
      logE("premature end of file!\n");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=reader.readS();
    logI("module version %d (0x%.2x)\n",ds.version,ds.version);

    if (ds.version>DIV_ENGINE_VERSION) {
      logW("this module was created with a more recent version of Furnace!\n");
      addWarning("this module was created with a more recent version of Furnace!");
    }

    if (ds.version<37) { // compat flags not stored back then
      ds.limitSlides=true;
      ds.linearPitch=true;
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
    ds.isDMF=false;

    reader.readS(); // reserved
    int infoSeek=reader.readI();

    reader.seek(infoSeek,SEEK_SET);

    // read header
    reader.read(magic,4);
    if (strcmp(magic,"INFO")!=0) {
      logE("invalid info header!\n");
      lastError="invalid info header!";
      delete[] file;
      return false;
    }
    reader.readI();

    ds.timeBase=reader.readC();
    ds.speed1=reader.readC();
    ds.speed2=reader.readC();
    ds.arpLen=reader.readC();
    ds.hz=reader.readF();
    ds.pal=(ds.hz>=53);
    if (ds.hz!=50 && ds.hz!=60) ds.customTempo=true;

    ds.patLen=reader.readS();
    ds.ordersLen=reader.readS();

    ds.hilightA=reader.readC();
    ds.hilightB=reader.readC();

    ds.insLen=reader.readS();
    ds.waveLen=reader.readS();
    ds.sampleLen=reader.readS();
    int numberOfPats=reader.readI();

    for (int i=0; i<32; i++) {
      ds.system[i]=systemFromFile(reader.readC());
      if (ds.system[i]!=DIV_SYSTEM_NULL) ds.systemLen=i+1;
    }
    int tchans=0;
    for (int i=0; i<ds.systemLen; i++) {
      tchans+=getChannelCount(ds.system[i]);
    }
    if (tchans>DIV_MAX_CHANS) tchans=DIV_MAX_CHANS;

    // system volume
    for (int i=0; i<32; i++) ds.systemVol[i]=reader.readC();

    // system panning
    for (int i=0; i<32; i++) ds.systemPan[i]=reader.readC();

    // system props
    for (int i=0; i<32; i++) {
      ds.systemFlags[i]=reader.readI();
    }

    // handle compound systems
    for (int i=0; i<32; i++) {
      if (ds.system[i]==DIV_SYSTEM_GENESIS ||
          ds.system[i]==DIV_SYSTEM_GENESIS_EXT ||
          ds.system[i]==DIV_SYSTEM_ARCADE) {
        for (int j=31; j>i; j--) {
          ds.system[j]=ds.system[j-1];
          ds.systemVol[j]=ds.systemVol[j-1];
          ds.systemPan[j]=ds.systemPan[j-1];
        }
        if (++ds.systemLen>32) ds.systemLen=32;

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
    logI("%s by %s\n",ds.name.c_str(),ds.author.c_str());

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
      for (int i=0; i<6; i++) reader.readC();
    } else {
      for (int i=0; i<20; i++) reader.readC();
    }

    // pointers
    reader.read(insPtr,ds.insLen*4);
    reader.read(wavePtr,ds.waveLen*4);
    reader.read(samplePtr,ds.sampleLen*4);
    for (int i=0; i<numberOfPats; i++) patPtr.push_back(reader.readI());

    for (int i=0; i<tchans; i++) {
      for (int j=0; j<ds.ordersLen; j++) {
        ds.orders.ord[i][j]=reader.readC();
      }
    }

    for (int i=0; i<tchans; i++) {
      ds.pat[i].effectRows=reader.readC();
    }

    if (ds.version>=39) {
      for (int i=0; i<tchans; i++) {
        ds.chanShow[i]=reader.readC();
      }

      for (int i=0; i<tchans; i++) {
        ds.chanCollapse[i]=reader.readC();
      }

      for (int i=0; i<tchans; i++) {
        ds.chanName[i]=reader.readString();
      }

      for (int i=0; i<tchans; i++) {
        ds.chanShortName[i]=reader.readString();
      }

      ds.notes=reader.readString();
    }

    // read instruments
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      reader.seek(insPtr[i],SEEK_SET);
      
      if (ins->readInsData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        delete ins;
        delete[] file;
        return false;
      }

      ds.ins.push_back(ins);
    }

    // read wavetables
    for (int i=0; i<ds.waveLen; i++) {
      DivWavetable* wave=new DivWavetable;
      reader.seek(wavePtr[i],SEEK_SET);

      if (wave->readWaveData(reader,ds.version)!=DIV_DATA_SUCCESS) {
        lastError="invalid wavetable header/data!";
        delete wave;
        delete[] file;
        return false;
      }

      ds.wave.push_back(wave);
    }

    // read samples
    for (int i=0; i<ds.sampleLen; i++) {
      reader.seek(samplePtr[i],SEEK_SET);
      reader.read(magic,4);
      if (strcmp(magic,"SMPL")!=0) {
        logE("%d: invalid sample header!\n",i);
        lastError="invalid sample header!";
        delete[] file;
        return false;
      }
      reader.readI();
      DivSample* sample=new DivSample;

      sample->name=reader.readString();
      sample->length=reader.readI();
      sample->rate=reader.readI();
      sample->vol=reader.readS();
      sample->pitch=reader.readS();
      sample->depth=reader.readC();

      // reserved
      reader.readC();

      // while version 32 stored this value, it was unused.
      if (ds.version>=38) {
        sample->centerRate=(unsigned short) reader.readS();
      } else {
        reader.readS();
      }

      if (ds.version>=19) {
        sample->loopStart=reader.readI();
      } else {
        reader.readI();
      }

      sample->data=new short[sample->length];
      reader.read(sample->data,2*sample->length);

      ds.sample.push_back(sample);
    }

    // read patterns
    for (int i: patPtr) {
      reader.seek(i,SEEK_SET);
      reader.read(magic,4);
      if (strcmp(magic,"PATR")!=0) {
        logE("%x: invalid pattern header!\n",i);
        lastError="invalid pattern header!";
        delete[] file;
        return false;
      }
      reader.readI();

      int chan=reader.readS();
      int index=reader.readS();
      reader.readI();

      DivPattern* pat=ds.pat[chan].getPattern(index,true);
      for (int j=0; j<ds.patLen; j++) {
        pat->data[j][0]=reader.readS();
        pat->data[j][1]=reader.readS();
        pat->data[j][2]=reader.readS();
        pat->data[j][3]=reader.readS();
        for (int k=0; k<ds.pat[chan].effectRows; k++) {
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
        logW("premature end of song (we are at %x, but size is %x)\n",reader.tell(),reader.size());
      }
    }

    if (active) quitDispatch();
    isBusy.lock();
    song.unload();
    song=ds;
    recalcChans();
    renderSamples();
    isBusy.unlock();
    if (active) {
      initDispatch();
      syncReset();
    }
  } catch (EndOfFileException e) {
    logE("premature end of file!\n");
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
  if (slen<16) {
    logE("too small!");
    lastError="file is too small";
    delete[] f;
    return false;
  }
  if (memcmp(f,DIV_DMF_MAGIC,16)!=0 && memcmp(f,DIV_FUR_MAGIC,16)!=0) {
    logD("loading as zlib...\n");
    // try zlib
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
        logE("zlib error: unknown! %d\n",nextErr);
      } else {
        logE("zlib error: %s\n",zl.msg);
      }
      inflateEnd(&zl);
      delete[] f;
      lastError="not a .dmf song";
      return false;
    }

    std::vector<InflateBlock*> blocks;
    while (true) {
      InflateBlock* ib=new InflateBlock(DIV_READ_SIZE);
      zl.next_out=ib->buf;
      zl.avail_out=ib->len;

      nextErr=inflate(&zl,Z_SYNC_FLUSH);
      if (nextErr!=Z_OK && nextErr!=Z_STREAM_END) {
        if (zl.msg==NULL) {
          logE("zlib error: unknown error! %d\n",nextErr);
          lastError="unknown decompression error";
        } else {
          logE("zlib inflate: %s\n",zl.msg);
          lastError=fmt::sprintf("decompression error: %s",zl.msg);
        }
        for (InflateBlock* i: blocks) delete i;
        blocks.clear();
        delete ib;
        inflateEnd(&zl);
        delete[] f;
        return false;
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
        logE("zlib end error: unknown error! %d\n",nextErr);
        lastError="unknown decompression finish error";
      } else {
        logE("zlib end: %s\n",zl.msg);
        lastError=fmt::sprintf("decompression finish error: %s",zl.msg);
      }
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      delete[] f;
      return false;
    }

    size_t finalSize=0;
    size_t curSeek=0;
    for (InflateBlock* i: blocks) {
      finalSize+=i->blockSize;
    }
    if (finalSize<1) {
      logE("compressed too small!\n");
      lastError="file too small";
      for (InflateBlock* i: blocks) delete i;
      blocks.clear();
      delete[] f;
      return false;
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
  } else {
    logD("loading as uncompressed\n");
    file=(unsigned char*)f;
    len=slen;
  }
  if (memcmp(file,DIV_DMF_MAGIC,16)==0) {
    return loadDMF(file,len); 
  } else if (memcmp(file,DIV_FUR_MAGIC,16)==0) {
    return loadFur(file,len);
  }
  logE("not a valid module!\n");
  lastError="not a compatible song";
  delete[] file;
  return false;
}

SafeWriter* DivEngine::saveFur() {
  int insPtr[256];
  int wavePtr[256];
  int samplePtr[256];
  std::vector<int> patPtr;
  size_t ptrSeek;
  warnings="";

  song.isDMF=false;
  song.version=DIV_ENGINE_VERSION;

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
  std::vector<int> patsToWrite;
  bool alreadyAdded[256];
  for (int i=0; i<chans; i++) {
    memset(alreadyAdded,0,256*sizeof(bool));
    for (int j=0; j<song.ordersLen; j++) {
      if (alreadyAdded[song.orders.ord[i][j]]) continue;
      patsToWrite.push_back((i<<16)|song.orders.ord[i][j]);
      alreadyAdded[song.orders.ord[i][j]]=true;
    }
  }

  /// SONG INFO
  w->write("INFO",4);
  w->writeI(0);

  w->writeC(song.timeBase);
  w->writeC(song.speed1);
  w->writeC(song.speed2);
  w->writeC(song.arpLen);
  w->writeF(song.hz);
  w->writeS(song.patLen);
  w->writeS(song.ordersLen);
  w->writeC(song.hilightA);
  w->writeC(song.hilightB);
  w->writeS(song.insLen);
  w->writeS(song.waveLen);
  w->writeS(song.sampleLen);
  w->writeI(patsToWrite.size());

  for (int i=0; i<32; i++) {
    if (i>=song.systemLen) {
      w->writeC(0);
    } else {
      w->writeC(systemToFile(song.system[i]));
    }
  }

  for (int i=0; i<32; i++) {
    w->writeC(song.systemVol[i]);
  }

  for (int i=0; i<32; i++) {
    w->writeC(song.systemPan[i]);
  }

  for (int i=0; i<32; i++) {
    w->writeI(song.systemFlags[i]);
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
  for (int i=0; i<6; i++) {
    w->writeC(0);
  }

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
    for (int j=0; j<song.ordersLen; j++) {
      w->writeC(song.orders.ord[i][j]);
    }
  }

  for (int i=0; i<chans; i++) {
    w->writeC(song.pat[i].effectRows);
  }

  for (int i=0; i<chans; i++) {
    w->writeC(song.chanShow[i]);
  }

  for (int i=0; i<chans; i++) {
    w->writeC(song.chanCollapse[i]);
  }

  for (int i=0; i<chans; i++) {
    w->writeString(song.chanName[i],false);
  }

  for (int i=0; i<chans; i++) {
    w->writeString(song.chanShortName[i],false);
  }

  w->writeString(song.notes,false);

  /// INSTRUMENT
  for (int i=0; i<song.insLen; i++) {
    DivInstrument* ins=song.ins[i];
    insPtr[i]=w->tell();
    ins->putInsData(w);
  }

  /// WAVETABLE
  for (int i=0; i<song.waveLen; i++) {
    DivWavetable* wave=song.wave[i];
    wavePtr[i]=w->tell();
    wave->putWaveData(w);
  }

  /// SAMPLE
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];
    samplePtr[i]=w->tell();
    w->write("SMPL",4);
    w->writeI(0);

    w->writeString(sample->name,false);
    w->writeI(sample->length);
    w->writeI(sample->rate);
    w->writeS(sample->vol);
    w->writeS(sample->pitch);
    w->writeC(sample->depth);
    w->writeC(0);
    w->writeS(sample->centerRate);
    w->writeI(sample->loopStart);

    w->write(sample->data,sample->length*2);
  }

  /// PATTERN
  for (int i: patsToWrite) {
    DivPattern* pat=song.pat[i>>16].getPattern(i&0xffff,false);
    patPtr.push_back(w->tell());
    w->write("PATR",4);
    w->writeI(0);

    w->writeS(i>>16);
    w->writeS(i&0xffff);

    w->writeI(0); // reserved

    for (int j=0; j<song.patLen; j++) {
      w->writeS(pat->data[j][0]); // note
      w->writeS(pat->data[j][1]); // octave
      w->writeS(pat->data[j][2]); // instrument
      w->writeS(pat->data[j][3]); // volume
      w->write(&pat->data[j][4],2*song.pat[i>>16].effectRows*2); // effects
    }

    w->writeString(pat->name,false);
  }

  /// POINTERS
  w->seek(ptrSeek,SEEK_SET);

  for (int i=0; i<song.insLen; i++) {
    w->writeI(insPtr[i]);
  }

  // wavetable pointers (we'll seek here later)
  for (int i=0; i<song.waveLen; i++) {
    w->writeI(wavePtr[i]);
  }

  // sample pointers (we'll seek here later)
  for (int i=0; i<song.sampleLen; i++) {
    w->writeI(samplePtr[i]);
  }

  // pattern pointers (we'll seek here later)
  for (int i: patPtr) {
    w->writeI(i);
  }

  return w;
}

SafeWriter* DivEngine::saveDMF(unsigned char version) {
  // fail if version is not supported
  if (version<24 || version>25) {
    logE("cannot save in this version!\n");
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
  }
  // fail if more than one system
  if (!isFlat && song.systemLen!=1) {
      logE("cannot save multiple systems in this format!\n");
      lastError="multiple systems not possible on .dmf";
      return NULL;
    }
  // fail if this is an YMU759 song
  if (song.system[0]==DIV_SYSTEM_YMU759) {
    logE("cannot save YMU759 song!\n");
    lastError="YMU759 song saving is not supported";
    return NULL;
  }
  // fail if the system is SMS+OPLL and version<25
  if (version<25 && song.system[0]==DIV_SYSTEM_SMS && song.system[1]==DIV_SYSTEM_OPLL) {
    logE("Master System FM expansion not supported in 1.0/legacy .dmf!\n");
    lastError="Master System FM expansion not supported in 1.0/legacy .dmf!";
    return NULL;
  }
  // fail if the system is Furnace-exclusive
  if (!isFlat && systemToFile(song.system[0])&0x80) {
    logE("cannot save Furnace-exclusive system song!\n");
    lastError="this system is not possible on .dmf";
    return NULL;
  }
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
    w->writeC(systemToFile(DIV_SYSTEM_GENESIS));
    sys=DIV_SYSTEM_GENESIS;
  } else if (song.system[0]==DIV_SYSTEM_YM2612_EXT && song.system[1]==DIV_SYSTEM_SMS) {
    w->writeC(systemToFile(DIV_SYSTEM_GENESIS_EXT));
    sys=DIV_SYSTEM_GENESIS_EXT;
  } else if (song.system[0]==DIV_SYSTEM_YM2151 && song.system[1]==DIV_SYSTEM_SEGAPCM_COMPAT) {
    w->writeC(systemToFile(DIV_SYSTEM_ARCADE));
    sys=DIV_SYSTEM_ARCADE;
  } else if (song.system[0]==DIV_SYSTEM_SMS && song.system[1]==DIV_SYSTEM_OPLL) {
    w->writeC(systemToFile(DIV_SYSTEM_SMS_OPLL));
    sys=DIV_SYSTEM_SMS_OPLL;
  } else {
    w->writeC(systemToFile(song.system[0]));
    sys=song.system[0];
  }

  // song info
  w->writeString(song.name,true);
  w->writeString(song.author,true);
  w->writeC(song.hilightA);
  w->writeC(song.hilightB);
  
  w->writeC(song.timeBase);
  w->writeC(song.speed1);
  w->writeC(song.speed2);
  w->writeC(song.pal);
  w->writeC(song.customTempo);
  char customHz[4];
  memset(customHz,0,4);
  snprintf(customHz,4,"%d",song.hz);
  w->write(customHz,3);
  w->writeI(song.patLen);
  w->writeC(song.ordersLen);

  for (int i=0; i<chans; i++) {
    for (int j=0; j<song.ordersLen; j++) {
      w->writeC(song.orders.ord[i][j]);
      if (version>=25) {
        DivPattern* pat=song.pat[i].getPattern(j,false);
        w->writeString(pat->name,true);
      }
    }
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
    if (i->type==DIV_INS_FM) {
      addWarning("no FM macros in .dmf format");
      break;
    }
  }

  w->writeC(song.ins.size());
  for (DivInstrument* i: song.ins) {
    w->writeString(i->name,true);

    // safety check
    if (!isFMSystem(sys) && i->mode) {
      i->mode=0;
    }
    if (!isSTDSystem(sys) && i->mode==0) {
      i->mode=1;
    }

    w->writeC(i->mode);
    if (i->mode) { // FM
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
        if (sys==DIV_SYSTEM_SMS_OPLL && j==0) {
          w->writeC(i->fm.opllPreset);
        } else {
          w->writeC(op.dt2);
        }
        if (sys==DIV_SYSTEM_SMS_OPLL) {
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
        w->writeC(i->std.volMacroLen);
        w->write(i->std.volMacro,4*i->std.volMacroLen);
        if (i->std.volMacroLen>0) {
          w->writeC(i->std.volMacroLoop);
        }
      }

      w->writeC(i->std.arpMacroLen);
      if (i->std.arpMacroMode) {
        w->write(i->std.arpMacro,4*i->std.arpMacroLen);
      } else {
        for (int j=0; j<i->std.arpMacroLen; j++) {
          w->writeI(i->std.arpMacro[j]+12);
        }
      }
      if (i->std.arpMacroLen>0) {
        w->writeC(i->std.arpMacroLoop);
      }
      w->writeC(i->std.arpMacroMode);

      w->writeC(i->std.dutyMacroLen);
      w->write(i->std.dutyMacro,4*i->std.dutyMacroLen);
      if (i->std.dutyMacroLen>0) {
        w->writeC(i->std.dutyMacroLoop);
      }

      w->writeC(i->std.waveMacroLen);
      w->write(i->std.waveMacro,4*i->std.waveMacroLen);
      if (i->std.waveMacroLen>0) {
        w->writeC(i->std.waveMacroLoop);
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

        logW("duty and cutoff precision will be lost!\n");
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
    w->write(i->data,4*i->len);
  }

  for (int i=0; i<getChannelCount(sys); i++) {
    w->writeC(song.pat[i].effectRows);

    for (int j=0; j<song.ordersLen; j++) {
      DivPattern* pat=song.pat[i].getPattern(song.orders.ord[i][j],false);
      for (int k=0; k<song.patLen; k++) {
        w->writeS(pat->data[k][0]); // note
        w->writeS(pat->data[k][1]); // octave
        w->writeS(pat->data[k][3]); // volume
        w->write(&pat->data[k][4],2*song.pat[i].effectRows*2); // effects
        w->writeS(pat->data[k][2]); // instrument
      }
    }
  }

  if (song.sample.size()>0) {
    addWarning("samples' rates will be rounded to nearest compatible value");
  }

  w->writeC(song.sample.size());
  for (DivSample* i: song.sample) {
    w->writeI(i->length);
    w->writeString(i->name,true);
    w->writeC(divToFileRate(i->rate));
    w->writeC(i->pitch);
    w->writeC(i->vol);
    w->writeC(i->depth);
    w->write(i->data,2*i->length);
  }

  return w;
}

