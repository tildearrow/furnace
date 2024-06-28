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

static void readSbiOpData(sbi_t& sbi, SafeReader& reader) {
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
}

bool DivEngine::loadS3M(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[4]={0,0,0,0};
  SafeReader reader=SafeReader(file,len);
  warnings="";

  unsigned char chanSettings[32];
  unsigned int insPtr[256];
  unsigned int patPtr[256];
  unsigned char chanPan[16];
  unsigned char chanMap[32];
  unsigned char defVol[256];

  unsigned char orders[256];

  bool doesPitchSlide[32];
  bool doesVibrato[32];
  bool doesPanning[32];
  bool doesVolSlide[32];
  bool doesArp[32];

  memset(chanSettings,0,32);
  memset(insPtr,0,256*sizeof(unsigned int));
  memset(patPtr,0,256*sizeof(unsigned int));
  memset(chanPan,0,16);
  memset(chanMap,0,32);
  memset(defVol,0,256);
  memset(orders,0,256);

  memset(doesPitchSlide,0,32*sizeof(bool));
  memset(doesVibrato,0,32*sizeof(bool));
  memset(doesPanning,0,32*sizeof(bool));
  memset(doesVolSlide,0,32*sizeof(bool));
  memset(doesArp,0,32*sizeof(bool));

  try {
    DivSong ds;
    ds.version=DIV_VERSION_S3M;
    ds.linearPitch=0;
    ds.pitchMacroIsLinear=false;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;
    ds.pitchSlideSpeed=12;

    logV("Scream Tracker 3 module");

    // load here
    if (!reader.seek(0x2c,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    reader.read(magic,4);

    if (memcmp(magic,DIV_S3M_MAGIC,4)!=0) {
      logW("the magic isn't complete");
      throw EndOfFileException(&reader,reader.tell());
    }

    if (!reader.seek(0,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }

    ds.name=reader.readStringLatin1(28);
    
    reader.readC(); // 0x1a
    if (reader.readC()!=16) {
      logW("type is wrong!");
    }
    reader.readS(); // x

    unsigned short ordersLen=reader.readS();
    ds.insLen=reader.readS();

    if (ordersLen>256) {
      logE("invalid order count!");
      lastError="invalid order count!";
      delete[] file;
      return false;
    }

    logV("orders: %d",ordersLen);
    logV("instruments: %d",ds.insLen);

    if (ds.insLen<0 || ds.insLen>256) {
      logE("invalid instrument count!");
      lastError="invalid instrument count!";
      delete[] file;
      return false;
    }

    unsigned short patCount=reader.readS();

    logV("patterns: %d",patCount);

    if (patCount>256) {
      logE("invalid pattern count!");
      lastError="invalid pattern count!";
      delete[] file;
      return false;
    }

    unsigned short flags=reader.readS();
    unsigned short version=reader.readS();
    bool signedSamples=(reader.readS()==1);

    logV("flags: %x",flags);
    logV("version: %x",flags);
    if (signedSamples) {
      logV("signed samples: yes");
    } else {
      logV("signed samples: no");
    }

    if ((flags&64) || version==0x1300) {
      ds.noSlidesOnFirstTick=false;
    }

    reader.readI(); // "SCRM"

    unsigned char globalVol=reader.readC();

    ds.subsong[0]->speeds.val[0]=(unsigned char)reader.readC();
    ds.subsong[0]->speeds.len=1;
    unsigned char tempo=reader.readC();
    ds.subsong[0]->hz=((double)tempo)/2.5;

    unsigned char masterVol=reader.readC();

    logV("masterVol: %d",masterVol);
    logV("signedSamples: %d",signedSamples);
    logV("globalVol: %d",globalVol);

    reader.readC(); // UC
    unsigned char defaultPan=(unsigned char)reader.readC();

    logV("defaultPan: %d",defaultPan);

    reader.readS(); // reserved
    reader.readI();
    reader.readI(); // the last 2 bytes is Special. we don't read that.

    reader.read(chanSettings,32);

    logD("channel settings:");
    for (int i=0; i<32; i++) {
      logV("%d. %x",i,chanSettings[i]);
    }

    logD("reading orders...");
    size_t curSubSong=0;
    ds.subsong[curSubSong]->ordersLen=0;
    bool subSongIncreased=false;
    for (int i=0; i<ordersLen; i++) {
      unsigned char nextOrder=reader.readC();
      orders[i]=curOrder;
      
      // skip +++ order
      if (nextOrder==254) {
        logV("- +++");
        continue;
      }
      // next subsong
      if (nextOrder==255) {
        logV("- end of song");
        if (!subSongIncreased) {
          curSubSong++;
          subSongIncreased=true;
        }
        curOrder=0;
        continue;
      }
      subSongIncreased=false;
      if (ds.subsong.size()<=curSubSong) {
        ds.subsong.push_back(new DivSubSong);
        ds.subsong[curSubSong]->ordersLen=0;
        ds.subsong[curSubSong]->name=fmt::sprintf("Order %.2X",i);
      }
      logV("- %.2x",nextOrder);
      for (int j=0; j<DIV_MAX_CHANS; j++) {
        ds.subsong[curSubSong]->orders.ord[j][ds.subsong[curSubSong]->ordersLen]=nextOrder;
      }
      ds.subsong[curSubSong]->ordersLen++;
      curOrder++;
    }

    logD("reading ins pointers...");
    for (int i=0; i<ds.insLen; i++) {
      insPtr[i]=((unsigned short)reader.readS())*16;
      logV("- %.2x",insPtr[i]);
    }

    logD("reading pat pointers...");
    for (int i=0; i<patCount; i++) {
      patPtr[i]=((unsigned short)reader.readS())*16;
      logV("- %.2x",patPtr[i]);
    }

    if (defaultPan==252) {
      logV("reading channel panning settings");
      reader.read(chanPan,16);
    } else {
      logV("default channel pan");
      memset(chanPan,16,16);
    }

    // determine chips to use
    ds.systemLen=0;

    bool hasPCM=false;
    bool hasFM=false;
    int numChans=0;

    for (int i=0; i<32; i++) {
      if (chanSettings[i]==255) continue;
      if ((chanSettings[i]&127)>=32) continue;
      if ((chanSettings[i]&127)>=16) {
        hasFM=true;
      } else {
        hasPCM=true;
        numChans++;
      }

      if (hasFM && hasPCM) break;
    }

    int pcmChan=hasFM?9:0;
    int fmChan=hasPCM?32:0;
    int invalidChan=40;

    for (int i=0; i<32; i++) {
      if (chanSettings[i]==255) {
        chanMap[i]=invalidChan++;
        continue;
      }
      if ((chanSettings[i]&127)>=32) {
        chanMap[i]=invalidChan++;
        continue;
      }
      if ((chanSettings[i]&127)>=16) {
        chanMap[i]=fmChan++;
      } else {
        chanMap[i]=pcmChan++;
      }
    }

    if (hasPCM) {
      for (int i=pcmChan; i<32; i++) {
        ds.subsong[0]->chanShow[i]=false;
        ds.subsong[0]->chanShowChanOsc[i]=false;
      }
    }

    logV("numChans: %d",numChans);

    ds.systemName="PC";
    if (hasPCM) {
      ds.system[ds.systemLen]=DIV_SYSTEM_ES5506;
      ds.systemVol[ds.systemLen]=(float)globalVol/64.0;
      ds.systemPan[ds.systemLen]=0;
      ds.systemFlags[ds.systemLen].set("volScale",3900);
      ds.systemFlags[ds.systemLen].set("amigaVol",true);
      ds.systemFlags[ds.systemLen].set("amigaPitch",true);
      ds.systemLen++;
    }
    if (hasFM) {
      ds.system[ds.systemLen]=DIV_SYSTEM_OPL2;
      ds.systemVol[ds.systemLen]=1.0f;
      ds.systemPan[ds.systemLen]=0;
      ds.systemLen++;
    }

    // load instruments/samples
    ds.ins.reserve(ds.insLen);
    for (int i=0; i<ds.insLen; i++) {
      logV("reading instrument %d...",i);
      DivInstrument* ins=new DivInstrument;
      if (insPtr[i]==0) {
        ins->type=DIV_INS_ES5506;
        ds.ins.push_back(ins);
        DivSample* emptySample=new DivSample;
        ds.sample.push_back(emptySample);
        continue;
      }

      if (!reader.seek(insPtr[i]+0x4c,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete ins;
        delete[] file;
        return false;
      }

      reader.read(magic,4);

      if (memcmp(magic,"SCRS",4)==0) {
        ins->type=DIV_INS_ES5506;
      } else if (memcmp(magic,"SCRI",4)==0) {
        ins->type=DIV_INS_OPL;
      } else {
        logW("odd magic!");
        ins->type=DIV_INS_ES5506;

        // read the instrument name anyway
        if (!reader.seek(insPtr[i]+48,SEEK_SET)) {
          logE("premature end of file!");
          lastError="incomplete file";
          delete ins;
          delete[] file;
          return false;
        }

        String name=reader.readStringLatin1(28);
        ins->name=name;

        ds.ins.push_back(ins);
        continue;
      }

      if (!reader.seek(insPtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete ins;
        delete[] file;
        return false;
      }

      unsigned char type=reader.readC();

      if (ins->type==DIV_INS_ES5506) {
        if (type>1) {
          logE("invalid instrument type! %d",type);
          lastError="invalid instrument!";
          delete ins;
          delete[] file;
          return false;
        }
      } else {
        if (type<2) {
          logE("invalid instrument type! %d",type);
          lastError="invalid instrument!";
          delete ins;
          delete[] file;
          return false;
        }
      }

      String dosName=reader.readStringLatin1(12);

      if (ins->type==DIV_INS_ES5506) {
        unsigned int memSeg=0;
        memSeg=(unsigned char)reader.readC();
        memSeg|=((unsigned short)reader.readS())<<8;

        memSeg>>=4; // ???

        logV("memSeg: %x",memSeg);

        unsigned int length=reader.readI();

        logV("length: %x",length);

        DivSample* s=new DivSample;

        s->loopStart=reader.readI();
        s->loopEnd=reader.readI();
        defVol[i]=reader.readC();

        reader.readC(); // x

        bool isPacked=reader.readC();
        unsigned char flags=reader.readC();

        s->centerRate=reader.readI();

        // reserved
        reader.readI(); // x
        reader.readI();
        reader.readI();

        String name=reader.readStringLatin1(28);
        s->name=dosName;
        ins->name=name;

        // "SCRS"
        reader.readI();

        // read sample data
        if (!reader.seek(memSeg,SEEK_SET)) {
          logE("premature end of file!");
          lastError="incomplete file";
          delete ins;
          delete s;
          delete[] file;
          return false;
        }

        s->loop=flags&1;
        s->depth=(flags&4)?DIV_SAMPLE_DEPTH_16BIT:DIV_SAMPLE_DEPTH_8BIT;
        if (length>0) {
          s->init(length);
        }

        if (isPacked) {
          logE("ADPCM not supported!");
          lastError="ADPCM sample";
          delete ins;
          delete s;
          delete[] file;
          return false;
        }

        if (flags&2) {
          // downmix stereo
          if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
            for (unsigned int i=0; i<s->samples; i++) {
              short l=reader.readS();
              if (!signedSamples) {
                l^=0x8000;
              }
              s->data16[i]=l;
            }
            for (unsigned int i=0; i<s->samples; i++) {
              short r=reader.readS();
              if (!signedSamples) {
                r^=0x8000;
              }
              s->data16[i]=(s->data16[i]+r)>>1;
            }
          } else {
            for (unsigned int i=0; i<s->samples; i++) {
              signed char l=reader.readC();
              if (!signedSamples) {
                l^=0x80;
              }
              s->data8[i]=l;
            }
            for (unsigned int i=0; i<s->samples; i++) {
              signed char r=reader.readC();
              if (!signedSamples) {
                r^=0x80;
              }
              s->data8[i]=(s->data8[i]+r)>>1;
            }
          }
        } else {
          if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
            for (unsigned int i=0; i<s->samples; i++) {
              s->data16[i]=reader.readS();
              if (!signedSamples) {
                s->data16[i]^=0x8000;
              }
            }
          } else {
            for (unsigned int i=0; i<s->samples; i++) {
              s->data8[i]=reader.readC();
              if (!signedSamples) {
                s->data8[i]^=0x80;
              }
            }
          }
        }

        ins->amiga.initSample=ds.sample.size();
        ds.sample.push_back(s);
      } else {
        ins->fm.ops=2;

        // reserved
        reader.readC();
        reader.readC();
        reader.readC();

        // OPL data
        sbi_t s3i;
        readSbiOpData(s3i,reader);
       
        // taken from S3I loading code
        DivInstrumentFM::Operator& opM=ins->fm.op[0];
        DivInstrumentFM::Operator& opC=ins->fm.op[1];
        ins->fm.ops = 2;
        opM.mult=s3i.Mcharacteristics&15;
        opM.ksr=((s3i.Mcharacteristics>>4)&1);
        opM.sus=((s3i.Mcharacteristics>>5)&1);
        opM.vib=((s3i.Mcharacteristics>>6)&1);
        opM.am=((s3i.Mcharacteristics>>7)&1);
        opM.tl=s3i.Mscaling_output&63;
        opM.ksl=((s3i.Mscaling_output>>6)&3);
        opM.ar=((s3i.Meg_AD >> 4)&15);
        opM.dr=(s3i.Meg_AD&15);
        opM.rr=(s3i.Meg_SR&15);
        opM.sl=((s3i.Meg_SR>>4)&15);
        opM.ws=s3i.Mwave;

        ins->fm.alg=(s3i.FeedConnect&1);
        ins->fm.fb=((s3i.FeedConnect>>1)&7);

        opC.mult=s3i.Ccharacteristics&15;
        opC.ksr=((s3i.Ccharacteristics>>4)&1);
        opC.sus=((s3i.Ccharacteristics>>5)&1);
        opC.vib=((s3i.Ccharacteristics>>6)&1);
        opC.am=((s3i.Ccharacteristics>>7)&1);
        opC.tl=s3i.Cscaling_output&63;
        opC.ksl=((s3i.Cscaling_output>>6)&3);
        opC.ar=((s3i.Ceg_AD>>4)&15);
        opC.dr=(s3i.Ceg_AD&15);
        opC.rr=(s3i.Ceg_SR&15);
        opC.sl=((s3i.Ceg_SR>>4)&15);
        opC.ws=s3i.Cwave;

        // unused
        reader.readC();

        defVol[i]=reader.readC();
        // what?
        unsigned char dsk=reader.readC();

        // x
        reader.readS();

        // oh no, we've got a problem here...
        // C-2 speed
        reader.readI();

        // x
        reader.seek(12,SEEK_CUR);

        String name=reader.readStringLatin1(28);
        ins->name=name;

        // "SCRI"
        reader.readI();
        
        logV("dsk: %d",dsk);
      }

      ds.ins.push_back(ins);
    }
    ds.sampleLen=ds.sample.size();

    // scan pattern data for effect use
    for (int i=0; i<patCount; i++) {
      logV("scanning pattern %d...",i);
      if (!reader.seek(patPtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        ds.unload();
        delete[] file;
        return false;
      }

      unsigned short dataLen=reader.readS();
      unsigned int dataEnd=reader.tell()+dataLen;

      while (reader.tell()<dataEnd) {
        unsigned char what=reader.readC();

        if (what==0) {
          curRow++;
          if (curRow>=64) break;
          continue;
        }

        unsigned char chan=what&31;
        bool hasNoteIns=what&32;
        bool hasVol=what&64;
        bool hasEffect=what&128;

        if (hasNoteIns) {
          reader.readC();
          reader.readC();
        }
        if (hasVol) {
          reader.readC();
        }
        if (hasEffect) {
          unsigned char effect=reader.readC();
          reader.readC(); // effect val

          switch (effect+'A'-1) {
            case 'D': // vol slide
              doesVolSlide[chan]=true;
              break;
            case 'E': // pitch down
              doesPitchSlide[chan]=true;
              break;
            case 'F': // pitch up
              doesPitchSlide[chan]=true;
              break;
            case 'G': // porta
              doesPitchSlide[chan]=true;
              break;
            case 'H': // vibrato
              doesVibrato[chan]=true;
              break;
            case 'J': // arp
              doesArp[chan]=true;
              break;
            case 'K': // vol slide + vibrato
              doesVolSlide[chan]=true;
              doesVibrato[chan]=true;
              break;
            case 'L': // vol slide + porta
              doesVolSlide[chan]=true;
              doesPitchSlide[chan]=true;
              break;
          }
        }
      }
    }

    // load pattern data
    for (int i=0; i<patCount; i++) {
      unsigned char effectCol[32];
      unsigned char vibStatus[32];
      bool vibStatusChanged[32];
      bool vibing[32];
      bool vibingOld[32];
      unsigned char volSlideStatus[32];
      bool volSlideStatusChanged[32];
      bool volSliding[32];
      bool volSlidingOld[32];
      unsigned char portaStatus[32];
      bool portaStatusChanged[32];
      bool porting[32];
      bool portingOld[32];
      unsigned char portaType[32];
      unsigned char arpStatus[32];
      bool arpStatusChanged[32];
      bool arping[32];
      bool arpingOld[32];
      bool did[32];

      if (patPtr[i]==0) continue;

      logV("reading pattern %d...",i);
      if (!reader.seek(patPtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        ds.unload();
        delete[] file;
        return false;
      }

      unsigned short dataLen=reader.readS();
      unsigned int dataEnd=reader.tell()+dataLen;

      logV("length: %d",dataLen);

      int curRow=0;
      bool mustCommitInitial=true;

      memset(effectCol,4,32);
      memset(vibStatus,0,32);
      memset(vibStatusChanged,0,32*sizeof(bool));
      memset(vibing,0,32*sizeof(bool));
      memset(vibingOld,0,32*sizeof(bool));
      memset(volSlideStatus,0,32);
      memset(volSlideStatusChanged,0,32*sizeof(bool));
      memset(volSliding,0,32*sizeof(bool));
      memset(volSlidingOld,0,32*sizeof(bool));
      memset(portaStatus,0,32);
      memset(portaStatusChanged,0,32*sizeof(bool));
      memset(porting,0,32*sizeof(bool));
      memset(portingOld,0,32*sizeof(bool));
      memset(portaType,0,32);
      memset(arpStatus,0,32);
      memset(arpStatusChanged,0,32*sizeof(bool));
      memset(arping,0,32*sizeof(bool));
      memset(arpingOld,0,32*sizeof(bool));
      memset(did,0,32*sizeof(bool));

      while (reader.tell()<dataEnd) {
        unsigned char what=reader.readC();

        if (what==0) {
          // commit effects
          for (int j=0; j<32; j++) {
            DivPattern* p=ds.subsong[0]->pat[chanMap[j]].getPattern(i,true);
            if (vibing[j]!=vibingOld[j] || vibStatusChanged[j]) {
              p->data[curRow][effectCol[j]++]=0x04;
              p->data[curRow][effectCol[j]++]=vibing[j]?vibStatus[j]:0;
              doesVibrato[j]=true;
            } else if (doesVibrato[j] && mustCommitInitial) {
              p->data[curRow][effectCol[j]++]=0x04;
              p->data[curRow][effectCol[j]++]=0;
            }

            if (volSliding[j]!=volSlidingOld[j] || volSlideStatusChanged[j]) {
              if (volSlideStatus[j]>=0xf1 && volSliding[j]) {
                p->data[curRow][effectCol[j]++]=0xf9;
                p->data[curRow][effectCol[j]++]=volSlideStatus[j]&15;
                volSliding[j]=false;
              } else if ((volSlideStatus[j]&15)==15 && volSlideStatus[j]>=0x10 && volSliding[j]) {
                p->data[curRow][effectCol[j]++]=0xf8;
                p->data[curRow][effectCol[j]++]=volSlideStatus[j]>>4;
                volSliding[j]=false;
              } else {
                p->data[curRow][effectCol[j]++]=0xfa;
                p->data[curRow][effectCol[j]++]=volSliding[j]?volSlideStatus[j]:0;
              }
              doesVolSlide[j]=true;
            } else if (doesVolSlide[j] && mustCommitInitial) {
              p->data[curRow][effectCol[j]++]=0xfa;
              p->data[curRow][effectCol[j]++]=0;
            }

            if (porting[j]!=portingOld[j] || portaStatusChanged[j]) {
              if (portaStatus[j]>=0xe0 && portaType[j]!=3 && porting[j]) {
                p->data[curRow][effectCol[j]++]=portaType[j]|0xf0;
                p->data[curRow][effectCol[j]++]=(portaStatus[j]&15)*((portaStatus[j]>=0xf0)?1:1);
                porting[j]=false;
              } else {
                p->data[curRow][effectCol[j]++]=portaType[j];
                p->data[curRow][effectCol[j]++]=porting[j]?portaStatus[j]:0;
              }
              doesPitchSlide[j]=true;
            } else if (doesPitchSlide[j] && mustCommitInitial) {
              p->data[curRow][effectCol[j]++]=0x01;
              p->data[curRow][effectCol[j]++]=0;
            }

            if (arping[j]!=arpingOld[j] || arpStatusChanged[j]) {
              p->data[curRow][effectCol[j]++]=0x00;
              p->data[curRow][effectCol[j]++]=arping[j]?arpStatus[j]:0;
              doesArp[j]=true;
            } else if (doesArp[j] && mustCommitInitial) {
              p->data[curRow][effectCol[j]++]=0x00;
              p->data[curRow][effectCol[j]++]=0;
            }

            if (effectCol[j]>=4+8*2) {
              logE("oh crap!");
            }

            if ((effectCol[j]>>1)-2>ds.subsong[0]->pat[j].effectCols) {
              ds.subsong[0]->pat[chanMap[j]].effectCols=(effectCol[j]>>1)-1;
            }
          }

          curRow++;
          memset(effectCol,4,32);
          memcpy(vibingOld,vibing,32*sizeof(bool));
          memcpy(volSlidingOld,volSliding,32*sizeof(bool));
          memcpy(portingOld,porting,32*sizeof(bool));
          memcpy(arpingOld,arping,32*sizeof(bool));
          memset(vibStatusChanged,0,32*sizeof(bool));
          memset(volSlideStatusChanged,0,32*sizeof(bool));
          memset(portaStatusChanged,0,32*sizeof(bool));
          memset(arpStatusChanged,0,32*sizeof(bool));
          memset(vibing,0,32*sizeof(bool));
          memset(volSliding,0,32*sizeof(bool));
          memset(porting,0,32*sizeof(bool));
          memset(arping,0,32*sizeof(bool));
          memset(did,0,32);
          mustCommitInitial=false;
          if (curRow>=64) break;
          continue;
        }

        unsigned char chan=what&31;
        bool hasNoteIns=what&32;
        bool hasVol=what&64;
        bool hasEffect=what&128;

        if (did[chan]) {
          logW("pat %d chan %d row %d: we already populated this channel!",i,chan,curRow);
        } else {
          did[chan]=true;
        }

        DivPattern* p=ds.subsong[0]->pat[chanMap[chan]].getPattern(i,true);
        if (hasNoteIns) {
          unsigned char note=reader.readC();
          unsigned char ins=reader.readC();

          if (note==254) { // note off
            p->data[curRow][0]=100;
            p->data[curRow][1]=0;
          } else if (note!=255) {
            p->data[curRow][0]=note&15;
            p->data[curRow][1]=note>>4;
            if ((note&15)==0) {
              p->data[curRow][0]=12;
              p->data[curRow][1]--;
            }
          }
          p->data[curRow][2]=(short)ins-1;
        }
        if (hasVol) {
          unsigned char vol=reader.readC();
          if (vol==255) {
            p->data[curRow][3]=-1;
          } else {
            // check for OPL channel
            if ((chanSettings[chan]&31)>=16) {
              if (vol>63) vol=63;
            } else {
              if (vol>64) vol=64;
            }
            p->data[curRow][3]=vol;
          }
        } else if (p->data[curRow][2]!=-1) {
          // populate with instrument volume
          unsigned char vol=defVol[p->data[curRow][2]&255];
          if ((chanSettings[chan]&31)>=16) {
            if (vol>63) vol=63;
          } else {
            if (vol>64) vol=64;
          }
          p->data[curRow][3]=vol;
        }
        if (hasEffect) {
          unsigned char effect=reader.readC();
          unsigned char effectVal=reader.readC();

          switch (effect+'A'-1) {
            case 'A': // speed
              p->data[curRow][effectCol[chan]++]=0x0f;
              p->data[curRow][effectCol[chan]++]=effectVal;
              break;
            case 'B': // go to order
              p->data[curRow][effectCol[chan]++]=0x0b;
              p->data[curRow][effectCol[chan]++]=orders[effectVal];
              break;
            case 'C': // next order
              p->data[curRow][effectCol[chan]++]=0x0d;
              p->data[curRow][effectCol[chan]++]=effectVal;
              break;
            case 'D': // vol slide
              if (effectVal!=0) {
                volSlideStatus[chan]=effectVal;
                volSlideStatusChanged[chan]=true;
              }
              if (hasNoteIns) {
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
              break;
            case 'E': // pitch down
              if (effectVal!=0) {
                portaStatus[chan]=effectVal;
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=2;
              porting[chan]=true;
              break;
            case 'F': // pitch up
              if (effectVal!=0) {
                portaStatus[chan]=effectVal;
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=1;
              porting[chan]=true;
              break;
            case 'G': // porta
              if (effectVal!=0) {
                portaStatus[chan]=effectVal;
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=3;
              porting[chan]=true;
              break;
            case 'H': // vibrato
              if (effectVal!=0) {
                vibStatus[chan]=effectVal;
                vibStatusChanged[chan]=true;
              }
              vibing[chan]=true;
              break;
            case 'I': // tremor (!)
              break;
            case 'J': // arp
              if (effectVal!=0) {
                arpStatus[chan]=effectVal;
                arpStatusChanged[chan]=true;
              }
              arping[chan]=true;
              break;
            case 'K': // vol slide + vibrato
              if (effectVal!=0) {
                volSlideStatus[chan]=effectVal;
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
              vibing[chan]=true;
              break;
            case 'L': // vol slide + porta
              if (effectVal!=0) {
                volSlideStatus[chan]=effectVal;
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
              porting[chan]=true;
              portaType[chan]=3;
              break;
            case 'M': // channel vol (extension)
              break;
            case 'N': // channel vol slide (extension)
              break;
            case 'O': // offset
              p->data[curRow][effectCol[chan]++]=0x91;
              p->data[curRow][effectCol[chan]++]=effectVal;
              break;
            case 'P': // pan slide (extension)
              break;
            case 'Q': // retrigger
              p->data[curRow][effectCol[chan]++]=0x0c;
              p->data[curRow][effectCol[chan]++]=effectVal&15;
              break;
            case 'R': // tremolo
              break;
            case 'S': // special...
              switch (effectVal>>4) {
                case 0xc:
                  p->data[curRow][effectCol[chan]++]=0xec;
                  p->data[curRow][effectCol[chan]++]=effectVal&15;
                  break;
                case 0xd:
                  p->data[curRow][effectCol[chan]++]=0xed;
                  p->data[curRow][effectCol[chan]++]=effectVal&15;
                  break;
              }
              break;
            case 'T': // tempo
              p->data[curRow][effectCol[chan]++]=0xf0;
              p->data[curRow][effectCol[chan]++]=effectVal;
              break;
            case 'U': // fine vibrato
              if (effectVal!=0) {
                vibStatus[chan]=effectVal;
                vibStatusChanged[chan]=true;
              }
              vibing[chan]=true;
              break;
            case 'V': // global volume (!)
              break;
            case 'W': // global volume slide (!)
              break;
            case 'X': // panning (extension)
              p->data[curRow][effectCol[chan]++]=0x80;
              p->data[curRow][effectCol[chan]++]=effectVal;
              break;
            case 'Y': // panbrello (extension)
              break;
            case 'Z': // MIDI macro (extension)
              break;
          }
        }
      }
    }

    // copy patterns to the rest of subsongs
    for (size_t i=1; i<ds.subsong.size(); i++) {
      for (int j=0; j<DIV_MAX_CHANS; j++) {
        for (int k=0; k<patCount; k++) {
          if (ds.subsong[0]->pat[j].data[k]) ds.subsong[0]->pat[j].data[k]->copyOn(ds.subsong[i]->pat[j].getPattern(k,true));
        }
        ds.subsong[i]->pat[j].effectCols=ds.subsong[0]->pat[j].effectCols;
      }
      ds.subsong[i]->speeds=ds.subsong[0]->speeds;
      ds.subsong[i]->hz=ds.subsong[0]->hz;
      memcpy(ds.subsong[i]->chanShow,ds.subsong[0]->chanShow,DIV_MAX_CHANS*sizeof(bool));
      memcpy(ds.subsong[i]->chanShowChanOsc,ds.subsong[0]->chanShowChanOsc,DIV_MAX_CHANS*sizeof(bool));
    }

    // find subsongs
    ds.findSubSongs(DIV_MAX_CHANS);

    // populate subsongs with default panning values
    if (masterVol&128) { // only in stereo mode
      for (size_t i=0; i<ds.subsong.size(); i++) {
        for (int j=0; j<16; j++) {
          DivPattern* p=ds.subsong[i]->pat[chanMap[j]].getPattern(ds.subsong[i]->orders.ord[j][0],true);
          for (int k=0; k<DIV_MAX_EFFECTS; k++) {
            if (p->data[0][4+(k<<1)]==0x80) {
              // give up if there's a panning effect already
              break;
            }
            if (p->data[0][4+(k<<1)]==-1) {
              p->data[0][4+(k<<1)]=0x80;
              if (chanPan[j]&16) {
                p->data[0][5+(k<<1)]=(j&1)?0xcc:0x33;
              } else {
                p->data[0][5+(k<<1)]=(chanPan[j]&15)|((chanPan[j]&15)<<4);
              }
              if (ds.subsong[i]->pat[chanMap[j]].effectCols<=k) ds.subsong[i]->pat[chanMap[j]].effectCols=k+1;
              break;
            }
          }
        }
      }
    }

    ds.insLen=ds.ins.size();
    ds.sampleLen=ds.sample.size();

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

