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

void readEnvelope(DivInstrument* ins, int env, unsigned char flags, unsigned char numPoints, unsigned char loopStart, unsigned char loopEnd, unsigned char susPoint, short* points) {
  if (numPoints>24) numPoints=24;

  if (loopStart>=numPoints) loopStart=numPoints-1;
  if (loopEnd>=numPoints) loopEnd=numPoints-1;
  if (susPoint>=numPoints) susPoint=numPoints-1;

  unsigned short pointTime[12];
  short pointVal[12];

  for (int i=0; i<12; i++) {
    pointTime[i]=points[i<<1];
    pointVal[i]=points[1|(i<<1)];
  }

  // don't process if there aren't any points or if the envelope is disabled
  if (numPoints<1) return;
  if (!(flags&1)) return;

  // convert into macro, or try to
  DivInstrumentMacro* target=NULL;
  switch (env) {
    case 0: // volume
      target=&ins->std.volMacro;
      break;
    case 1: // panning (split later)
      target=&ins->std.panLMacro;
      break;
  }
  target->len=0;
  int point=0;
  bool pointJustBegan=true;
  // mark loop end as end of envelope
  if (flags&4) {
    if (loopEnd<numPoints) numPoints=loopEnd+1;
  }
  for (int i=0; i<255; i++) {
    int curPoint=MIN(point,numPoints-1);
    int nextPoint=MIN(point+1,numPoints-1);
    int p0=pointVal[curPoint];
    int p1=pointVal[nextPoint];
    while (i>pointTime[nextPoint]) {
      point++;
      pointJustBegan=true;
      curPoint=MIN(point,numPoints-1);
      nextPoint=MIN(point+1,numPoints-1);
      p0=pointVal[curPoint];
      p1=pointVal[nextPoint];
      if ((point+1)>=numPoints) {
        break;
      }
    }
    if (pointJustBegan) {
      pointJustBegan=false;
      if (flags&4) { // loop
        if (point==loopStart) {
          if (loopStart!=loopEnd) {
            target->loop=i;
          }
        }
      }
      if (flags&2) { // sustain
        if (point==susPoint) {
          target->rel=MAX(i-1,0);
        }
      }
    }
    if ((point+1)>=numPoints) {
      target->len=i;
      //target->val[i]=p0;
      break;
    }
    int timeDiff=pointTime[nextPoint]-pointTime[curPoint];
    int curTime=i-pointTime[curPoint];
    if (timeDiff<1) timeDiff=1;
    if (curTime<0) curTime=0;

    target->len=i+1;
    target->val[i]=p0+(((p1-p0)*curTime)/timeDiff);
  }

  // split L/R
  if (env==1) {
    for (int i=0; i<ins->std.panLMacro.len; i++) {
      int val=ins->std.panLMacro.val[i]-32;
      if (val==0) {
        ins->std.panLMacro.val[i]=4095;
        ins->std.panRMacro.val[i]=4095;
      } else if (val>0) { // pan right
        ins->std.panLMacro.val[i]=4095*pow(1.0-((double)val/64.0),0.25);
        ins->std.panRMacro.val[i]=4095;
      } else { // pan left
        ins->std.panLMacro.val[i]=4095;
        ins->std.panRMacro.val[i]=4095*pow(1.0+((double)val/64.0),0.25);
      }
    }
    ins->std.panRMacro.len=ins->std.panLMacro.len;
    ins->std.panRMacro.loop=ins->std.panLMacro.loop;
    ins->std.panRMacro.rel=ins->std.panLMacro.rel;
  }
}

bool DivEngine::loadXM(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[32];
  unsigned char sampleVol[256][256];
  unsigned char samplePan[256][256];
  unsigned char noteMap[256][128];

  unsigned short patLen[256];

  bool doesPitchSlide[128];
  bool doesVibrato[128];
  bool doesPanning[128];
  bool doesVolSlide[128];
  bool doesPanSlide[128];
  bool doesArp[128];
  bool doesTremolo[128];
  bool doesPanbrello[128];

  SafeReader reader=SafeReader(file,len);
  warnings="";

  memset(sampleVol,0,256*256);
  memset(samplePan,0,256*256);
  memset(noteMap,0,256*128);

  memset(patLen,0,256*sizeof(unsigned short));

  memset(doesPitchSlide,0,128*sizeof(bool));
  memset(doesVibrato,0,128*sizeof(bool));
  memset(doesPanning,0,128*sizeof(bool));
  memset(doesVolSlide,0,128*sizeof(bool));
  memset(doesPanSlide,0,128*sizeof(bool));
  memset(doesArp,0,128*sizeof(bool));
  memset(doesTremolo,0,128*sizeof(bool));
  memset(doesPanbrello,0,128*sizeof(bool));

  try {
    DivSong ds;
    ds.version=DIV_VERSION_XM;
    //ds.linearPitch=0;
    //ds.pitchMacroIsLinear=false;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;
    ds.pitchSlideSpeed=12;

    logV("Extended Module");

    // load here
    if (!reader.seek(0,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    reader.read(magic,17);

    if (memcmp(magic,DIV_XM_MAGIC,17)!=0) {
      logW("invalid magic");
      throw EndOfFileException(&reader,reader.tell());
    }

    ds.name=reader.readStringLatin1(20);

    // 0x1a
    reader.readC();

    String trackerName=reader.readStringLatin1(20);
    unsigned short trackerVer=reader.readS();

    if (trackerName!="") logV("made with %s",trackerName);
    logV("version %x",trackerVer);

    unsigned int headerSeek=reader.tell();
    headerSeek+=reader.readI();

    ds.subsong[0]->ordersLen=(unsigned short)reader.readS();
    ds.subsong[0]->patLen=1;
    unsigned short loopPos=reader.readS();
    unsigned short totalChans=reader.readS();
    unsigned short patCount=reader.readS();
    ds.insLen=(unsigned short)reader.readS();
    ds.linearPitch=(reader.readS()&1)?2:0;
    ds.subsong[0]->speeds.val[0]=reader.readS();
    ds.subsong[0]->speeds.len=1;
    double bpm=(unsigned short)reader.readS();
    ds.subsong[0]->hz=(double)bpm/2.5;

    if (ds.subsong[0]->ordersLen>256) {
      logE("invalid order count!");
      lastError="invalid order count";
      delete[] file;
      return false;
    }

    if (patCount>256) {
      logE("too many patterns!");
      lastError="too many patterns";
      delete[] file;
      return false;
    }

    if (ds.insLen<0 || ds.insLen>256) {
      logE("invalid instrument count!");
      lastError="invalid instrument count";
      delete[] file;
      return false;
    }

    logV("channels: %d",totalChans);

    if (totalChans>127) {
      logE("invalid channel count!");
      lastError="invalid channel count";
      delete[] file;
      return false;
    }

    logV("repeat pos: %d",loopPos);

    logV("reading orders...");
    for (int i=0; i<256; i++) {
      unsigned char val=reader.readC();
      for (int j=0; j<DIV_MAX_CHANS; j++) {
        ds.subsong[0]->orders.ord[j][i]=val;
      }
    }

    for (int i=0; i<(totalChans+31)>>5; i++) {
      ds.system[i]=DIV_SYSTEM_ES5506;
      ds.systemFlags[i].set("amigaVol",true);
      ds.systemFlags[i].set("amigaPitch",(ds.linearPitch==0));
      ds.systemFlags[i].set("volScale",3900);
    }
    ds.systemLen=(totalChans+31)>>5;

    size_t patBegin=headerSeek;
    logV("seeking to %x...",patBegin);

    if (!reader.seek(patBegin,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }

    // scan pattern data for effect use
    logD("scanning patterns...");
    for (unsigned short i=0; i<patCount; i++) {
      logV("pattern %d",i);
      headerSeek=reader.tell();
      headerSeek+=reader.readI();

      unsigned char packType=reader.readC();
      if (packType!=0) {
        logE("unknown packing type %d!",packType);
        lastError="unknown packing type";
        ds.unload();
        delete[] file;
        return false;
      }

      unsigned short totalRows=reader.readS();
      logV("total rows: %d",totalRows);
      if (totalRows>ds.subsong[0]->patLen) ds.subsong[0]->patLen=totalRows;
      patLen[i]=totalRows;

      if (totalRows>256) {
        logE("too many rows! %d",totalRows);
        lastError="too many rows";
        delete[] file;
        return false;
      }

      unsigned int packedSeek=headerSeek+(unsigned short)reader.readS();

      logV("seeking to %x...",headerSeek);
      if (!reader.seek(headerSeek,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }

      // read data
      for (int j=0; j<totalRows; j++) {
        for (int k=0; k<totalChans; k++) {
          unsigned char note=reader.readC();
          unsigned char vol=0;
          unsigned char effect=0;
          unsigned char effectVal=0;
          bool hasNote=false;
          bool hasIns=false;
          bool hasVol=false;
          bool hasEffect=false;
          bool hasEffectVal=false;

          if (note&0x80) { // packed
            hasNote=note&1;
            hasIns=note&2;
            hasVol=note&4;
            hasEffect=note&8;
            hasEffectVal=note&16;
            if (hasNote) {
              note=reader.readC();
            }
          } else { // unpacked
            hasNote=true;
            hasIns=true;
            hasVol=true;
            hasEffect=true;
            hasEffectVal=true;
          }

          if (hasIns) {
            reader.readC();
          }
          if (hasVol) {
            vol=reader.readC();
            switch (vol>>4) {
              case 0x6: // vol slide down
                doesVolSlide[k]=true;
                break;
              case 0x7: // vol slide up
                doesVolSlide[k]=true;
                break;
              case 0x8: // vol slide down (fine)
                doesVolSlide[k]=true;
                break;
              case 0x9: // vol slide up (fine)
                doesVolSlide[k]=true;
                break;
              case 0xa: // vibrato speed
                doesVibrato[k]=true;
                break;
              case 0xb: // vibrato depth
                doesVibrato[k]=true;
                break;
              case 0xc: // panning
                doesPanning[k]=true;
                break;
              case 0xd: // pan slide left
                doesPanSlide[k]=true;
                break;
              case 0xe: // pan slide right
                doesPanSlide[k]=true;
                break;
              case 0xf: // porta
                doesPitchSlide[k]=true;
                break;
            }
          }
          if (hasEffect) {
            effect=reader.readC();
            switch (effect) {
              case 0:
                doesArp[k]=true;
                break;
              case 1:
                doesPitchSlide[k]=true;
                break;
              case 2:
                doesPitchSlide[k]=true;
                break;
              case 3:
                doesPitchSlide[k]=true;
                break;
              case 4:
                doesVibrato[k]=true;
                break;
              case 5:
                doesPitchSlide[k]=true;
                doesVolSlide[k]=true;
                break;
              case 6:
                doesVibrato[k]=true;
                doesVolSlide[k]=true;
                break;
              case 7:
                doesTremolo[k]=true;
                break;
              case 8:
                doesPanning[k]=true;
                break;
              case 0xe:
                doesPanning[k]=true;
                break;
              case 0x19: // P
                doesPanSlide[k]=true;
                break;
              case 0x21: // X
                doesPitchSlide[k]=true;
                break;
              case 0x22: // Y
                doesPanbrello[k]=true;
                break;
            }
          }
          if (hasEffectVal) {
            effectVal=reader.readC();
            if (effect==0xe) {
              switch (effectVal>>4) {
                case 1:
                  doesPitchSlide[k]=true;
                  break;
                case 2:
                  doesPitchSlide[k]=true;
                  break;
                case 0xa:
                  doesVolSlide[k]=true;
                  break;
                case 0xb:
                  doesVolSlide[k]=true;
                  break;
              }
            }
          }
        }
      }

      logV("seeking to %x...",packedSeek);
      if (!reader.seek(packedSeek,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }
    }

    // read instruments
    for (int i=0; i<ds.insLen; i++) {
      short volEnv[24];
      short panEnv[48];

      DivInstrument* ins=new DivInstrument;
      logD("instrument %d",i);
      headerSeek=reader.tell();
      headerSeek+=reader.readI();

      ins->name=reader.readStringLatin1(22);
      ins->type=DIV_INS_ES5506;
      ins->amiga.useNoteMap=true;

      unsigned char insType=reader.readC();

      /*
      if (insType!=0) {
        logE("unknown instrument type!");
        lastError="unknown instrument type";
        delete ins;
        song.unload();
        delete[] file;
        return false;
      }*/

      logV("type: %d",insType);

      unsigned short sampleCount=reader.readS();
      logV("%d samples",sampleCount);

      if (sampleCount>0) {
        unsigned int sampleHeaderSize=reader.readI();
        logV("sample header size: %d",sampleHeaderSize);
        for (int j=0; j<96; j++) {
          unsigned char nextMap=reader.readC();
          ins->amiga.noteMap[j].map=ds.sample.size()+nextMap;
          noteMap[i][j]=nextMap;
        }

        for (int j=0; j<24; j++) {
          volEnv[j]=reader.readS();
        }
        for (int j=0; j<24; j++) {
          panEnv[j]=reader.readS();
        }

        unsigned char volEnvLen=reader.readC();
        unsigned char panEnvLen=reader.readC();
        unsigned char volSusPoint=reader.readC();
        unsigned char volLoopStart=reader.readC();
        unsigned char volLoopEnd=reader.readC();
        unsigned char panSusPoint=reader.readC();
        unsigned char panLoopStart=reader.readC();
        unsigned char panLoopEnd=reader.readC();
        unsigned char volType=reader.readC();
        unsigned char panType=reader.readC();

        unsigned char vibType=reader.readC();
        unsigned char vibSweep=reader.readC();
        unsigned char vibDepth=reader.readC();
        unsigned char vibRate=reader.readC();

        unsigned short volFade=reader.readS();
        reader.readS(); // reserved

        logV("vibrato: %d %d %d %d",vibType,vibSweep,vibDepth,vibRate);
        
        // convert envelopes
        readEnvelope(ins,0,volType,volEnvLen,volLoopStart,volLoopEnd,volSusPoint,volEnv);
        readEnvelope(ins,1,panType,panEnvLen,panLoopStart,panLoopEnd,panSusPoint,panEnv);

        if (volType&1) {
          // add fade-out
          int cur=64;
          if (ins->std.volMacro.len>0) {
            cur=ins->std.volMacro.val[ins->std.volMacro.len-1];
          }
          for (int fadeOut=32767; fadeOut>0 && ins->std.volMacro.len<254; fadeOut-=volFade) {
            ins->std.volMacro.val[ins->std.volMacro.len++]=(cur*fadeOut)>>15;
          }
          if (ins->std.volMacro.len<255) {
            ins->std.volMacro.val[ins->std.volMacro.len++]=0;
          }
        } else {
          // add a one-tick macro to make note release happy
          ins->std.volMacro.val[0]=64;
          ins->std.volMacro.val[1]=0;
          ins->std.volMacro.rel=0;
          ins->std.volMacro.len=2;
        }

        if (!reader.seek(headerSeek,SEEK_SET)) {
          logE("premature end of file!");
          lastError="incomplete file";
          delete[] file;
          return false;
        }

        // read samples for this instrument
        std::vector<DivSample*> toAdd;
        for (int j=0; j<sampleCount; j++) {
          DivSample* s=new DivSample;

          unsigned int numSamples=reader.readI();
          if (numSamples>16777216) {
            logE("abnormal sample size! %x",reader.tell());
            lastError="bad sample size";
            delete s;
            delete[] file;
            return false;
          }

          s->loopStart=reader.readI();
          s->loopEnd=reader.readI()+s->loopStart;

          sampleVol[i][j]=reader.readC();

          signed char fine=reader.readC();
          unsigned char flags=reader.readC();
          samplePan[i][j]=reader.readC();
          signed char note=reader.readC();

          switch (flags&3) {
            case 0:
              s->loop=false;
              break;
            case 1:
              s->loop=true;
              s->loopMode=DIV_SAMPLE_LOOP_FORWARD;
              break;
            case 2:
              s->loop=true;
              s->loopMode=DIV_SAMPLE_LOOP_PINGPONG;
              break;
          }

          if (s->loopStart>s->loopEnd) {
            s->loopStart^=s->loopEnd;
            s->loopEnd^=s->loopStart;
            s->loopStart^=s->loopEnd;
          }

          if (flags&16) {
            s->loopStart>>=1;
            s->loopEnd>>=1;
          }

          reader.readC(); // reserved

          s->centerRate=8363.0*pow(2.0,((double)note+((double)fine/128.0))/12.0);

          s->name=reader.readStringLatin1(22);
          s->depth=(flags&16)?DIV_SAMPLE_DEPTH_16BIT:DIV_SAMPLE_DEPTH_8BIT;
          if (flags&16) {
            numSamples>>=1;
          }
          s->init(numSamples);

          // seek here???
          toAdd.push_back(s);
        }

        for (int j=0; j<sampleCount; j++) {
          DivSample* s=toAdd[j];

          // load sample data
          if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
            short next=0;
            for (unsigned int i=0; i<s->samples; i++) {
              next+=reader.readS();
              s->data16[i]=next;
            }
          } else {
            signed char next=0;
            for (unsigned int i=0; i<s->samples; i++) {
              next+=reader.readC();
              s->data8[i]=next;
            }
          }
        }

        for (DivSample* i: toAdd) {
          ds.sample.push_back(i);
        }
        toAdd.clear();
      } else {
        if (!reader.seek(headerSeek,SEEK_SET)) {
          logE("premature end of file!");
          lastError="incomplete file";
          delete[] file;
          return false;
        }
      }
      
      ds.ins.push_back(ins);
    }

    if (!reader.seek(patBegin,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }

    // read patterns
    logD("reading patterns...");
    for (unsigned short i=0; i<patCount; i++) {
      unsigned char effectCol[128];
      unsigned char vibStatus[128];
      bool vibStatusChanged[128];
      bool vibing[128];
      bool vibingOld[128];
      unsigned char volSlideStatus[128];
      bool volSlideStatusChanged[128];
      bool volSliding[128];
      bool volSlidingOld[128];
      unsigned char portaStatus[128];
      bool portaStatusChanged[128];
      bool porting[128];
      bool portingOld[128];
      unsigned char portaType[128];
      unsigned char arpStatus[128];
      bool arpStatusChanged[128];
      bool arping[128];
      bool arpingOld[128];
      unsigned char panStatus[128];
      bool panStatusChanged[128];
      bool panning[128];
      bool panningOld[128];
      unsigned char tremStatus[128];
      bool tremStatusChanged[128];
      bool treming[128];
      bool tremingOld[128];
      unsigned char panSlideStatus[128];
      bool panSlideStatusChanged[128];
      bool panSliding[128];
      bool panSlidingOld[128];
      unsigned char lastNote[128];

      bool mustCommitInitial=true;

      memset(effectCol,4,128);
      memset(vibStatus,0,128);
      memset(vibStatusChanged,0,128*sizeof(bool));
      memset(vibing,0,128*sizeof(bool));
      memset(vibingOld,0,128*sizeof(bool));
      memset(volSlideStatus,0,128);
      memset(volSlideStatusChanged,0,128*sizeof(bool));
      memset(volSliding,0,128*sizeof(bool));
      memset(volSlidingOld,0,128*sizeof(bool));
      memset(portaStatus,0,128);
      memset(portaStatusChanged,0,128*sizeof(bool));
      memset(porting,0,128*sizeof(bool));
      memset(portingOld,0,128*sizeof(bool));
      memset(portaType,0,128);
      memset(arpStatus,0,128);
      memset(arpStatusChanged,0,128*sizeof(bool));
      memset(arping,0,128*sizeof(bool));
      memset(arpingOld,0,128*sizeof(bool));
      memset(panStatus,0,128);
      memset(panStatusChanged,0,128*sizeof(bool));
      memset(panning,0,128*sizeof(bool));
      memset(panningOld,0,128*sizeof(bool));
      memset(tremStatus,0,128);
      memset(tremStatusChanged,0,128*sizeof(bool));
      memset(treming,0,128*sizeof(bool));
      memset(tremingOld,0,128*sizeof(bool));
      memset(panSlideStatus,0,128);
      memset(panSlideStatusChanged,0,128*sizeof(bool));
      memset(panSliding,0,128*sizeof(bool));
      memset(panSlidingOld,0,128*sizeof(bool));
      memset(lastNote,0,128);

      logV("pattern %d",i);
      headerSeek=reader.tell();
      headerSeek+=reader.readI();

      unsigned char packType=reader.readC();
      if (packType!=0) {
        logE("unknown packing type %d!",packType);
        lastError="unknown packing type";
        ds.unload();
        delete[] file;
        return false;
      }

      unsigned short totalRows=reader.readS();
      logV("total rows: %d",totalRows);
      if (totalRows>ds.subsong[0]->patLen) ds.subsong[0]->patLen=totalRows;
      patLen[i]=totalRows;

      if (totalRows>256) {
        logE("too many rows! %d",totalRows);
        lastError="too many rows";
        delete[] file;
        return false;
      }

      unsigned int packedSeek=headerSeek+(unsigned short)reader.readS();

      logV("seeking to %x...",headerSeek);
      if (!reader.seek(headerSeek,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }

      // read data
      for (int j=0; j<totalRows; j++) {
        for (int k=0; k<totalChans; k++) {
          DivPattern* p=ds.subsong[0]->pat[k].getPattern(i,true);

          unsigned char note=reader.readC();
          unsigned char ins=0;
          unsigned char vol=0;
          unsigned char effect=0;
          unsigned char effectVal=0;
          bool hasNote=false;
          bool hasIns=false;
          bool hasVol=false;
          bool hasEffect=false;
          bool hasEffectVal=false;
          bool writePanning=false;

          if (note&0x80) { // packed
            hasNote=note&1;
            hasIns=note&2;
            hasVol=note&4;
            hasEffect=note&8;
            hasEffectVal=note&16;
            if (hasNote) {
              note=reader.readC();
            }
          } else { // unpacked
            hasNote=true;
            hasIns=true;
            hasVol=true;
            hasEffect=true;
            hasEffectVal=true;
          }

          if (hasNote) {
            if (note!=0) {
              lastNote[k]=note;
              if (note>96) {
                p->data[j][0]=101;
                p->data[j][1]=0;
              } else {
                note--;
                p->data[j][0]=note%12;
                p->data[j][1]=note/12;
                if (p->data[j][0]==0) {
                  p->data[j][0]=12;
                  p->data[j][1]=(unsigned char)(p->data[j][1]-1);
                }
              }
            }
          }
          if (hasIns) {
            ins=reader.readC();
            p->data[j][2]=((int)ins)-1;
            // default volume
            if (lastNote[k]<96 && ins>0) {
              p->data[j][3]=sampleVol[(ins-1)&255][noteMap[(ins-1)&255][lastNote[k]&127]];
            }
            writePanning=true;
          }
          if (hasVol) {
            vol=reader.readC();
            if (vol>=0x10 && vol<=0x50) {
              p->data[j][3]=vol-0x10;
            } else { // effects in volume column
              switch (vol>>4) {
                case 0x6: // vol slide down
                  if ((vol&15)!=0) {
                    volSlideStatus[k]=vol&15;
                    volSlideStatusChanged[k]=true;
                  }
                  if (hasNote || hasIns) {
                    volSlideStatusChanged[k]=true;
                  }
                  volSliding[k]=true;
                  break;
                case 0x7: // vol slide up
                  if ((vol&15)!=0) {
                    volSlideStatus[k]=(vol&15)<<4;
                    volSlideStatusChanged[k]=true;
                  }
                  if (hasNote || hasIns) {
                    volSlideStatusChanged[k]=true;
                  }
                  volSliding[k]=true;
                  break;
                case 0x8: // vol slide down (fine)
                  if ((vol&15)!=0) {
                    volSlideStatus[k]=0xf0|(vol&15);
                    volSlideStatusChanged[k]=true;
                  }
                  if (hasNote || hasIns) {
                    volSlideStatusChanged[k]=true;
                  }
                  volSliding[k]=true;
                  break;
                case 0x9: // vol slide up (fine)
                  if ((vol&15)!=0) {
                    volSlideStatus[k]=((vol&15)<<4)|0xf;
                    volSlideStatusChanged[k]=true;
                  }
                  if (hasNote || hasIns) {
                    volSlideStatusChanged[k]=true;
                  }
                  volSliding[k]=true;
                  break;
                case 0xa: // vibrato speed
                  if ((vol&15)!=0) {
                    vibStatus[k]&=0x0f;
                    vibStatus[k]|=(vol&15)<<4;
                    vibStatusChanged[k]=true;
                  }
                  vibing[k]=true;
                  break;
                case 0xb: // vibrato depth
                  if ((vol&15)!=0) {
                    vibStatus[k]&=0xf0;
                    vibStatus[k]|=vol&15;
                    vibStatusChanged[k]=true;
                  }
                  vibing[k]=true;
                  break;
                case 0xc: // panning
                  p->data[j][effectCol[k]++]=0x80;
                  if ((vol&15)==8) {
                    p->data[j][effectCol[k]++]=0x80;
                  } else {
                    p->data[j][effectCol[k]++]=(vol&15)|((vol&15)<<4);
                  }
                  writePanning=false;
                  break;
                case 0xd: // pan slide left
                  if ((vol&15)!=0) {
                    panSlideStatus[k]&=0x0f;
                    panSlideStatus[k]|=(vol&15)<<4;
                    panSlideStatusChanged[k]=true;
                  }
                  panSliding[k]=true;
                  break;
                case 0xe: // pan slide right
                  if ((vol&15)!=0) {
                    panSlideStatus[k]&=0xf0;
                    panSlideStatus[k]|=vol&15;
                    panSlideStatusChanged[k]=true;
                  }
                  panSliding[k]=true;
                  break;
                case 0xf: // porta
                  break;
              }
            }
          }
          if (hasEffect) {
            effect=reader.readC();
          }
          if (hasEffectVal) {
            effectVal=reader.readC();
          }

          if (hasEffect || (hasEffectVal && effectVal!=0)) {
            switch (effect) {
              case 0: // arp
                if (effectVal!=0) {
                  arpStatus[k]=(effectVal>>4)|(effectVal<<4);
                  arpStatusChanged[k]=true;
                }
                arping[k]=true;
                break;
              case 1: // pitch up
                if (effectVal!=0) {
                  portaStatus[k]=effectVal;
                  portaStatusChanged[k]=true;
                }
                if (portaType[k]!=1) {
                  portaStatusChanged[k]=true;
                }
                portaType[k]=1;
                porting[k]=true;
                break;
              case 2: // pitch down
                if (effectVal!=0) {
                  portaStatus[k]=effectVal;
                  portaStatusChanged[k]=true;
                }
                if (portaType[k]!=2) {
                  portaStatusChanged[k]=true;
                }
                portaType[k]=2;
                porting[k]=true;
                break;
              case 3: // porta
                if (effectVal!=0) {
                  portaStatus[k]=effectVal;
                  portaStatusChanged[k]=true;
                }
                if (portaType[k]!=3) {
                  portaStatusChanged[k]=true;
                }
                portaType[k]=3;
                porting[k]=true;
                break;
              case 4: // vibrato
                if (effectVal!=0) {
                  if ((effectVal&0xf0)==0) { // only change depth
                    vibStatus[k]&=0xf0;
                    vibStatus[k]|=effectVal&0x0f;
                  } else if ((effectVal&0x0f)==0) { // only change speed
                    vibStatus[k]&=0x0f;
                    vibStatus[k]|=effectVal&0xf0;
                  } else {
                    vibStatus[k]=effectVal;
                  }
                  vibStatusChanged[k]=true;
                }
                vibing[k]=true;
                break;
              case 5: // vol slide + porta
                if (effectVal!=0) {
                  volSlideStatus[k]=effectVal;
                  volSlideStatusChanged[k]=true;
                }
                volSliding[k]=true;
                porting[k]=true;
                portaType[k]=3;
                break;
              case 6: // vol slide + vibrato
                if (effectVal!=0) {
                  volSlideStatus[k]=effectVal;
                  volSlideStatusChanged[k]=true;
                }
                volSliding[k]=true;
                vibing[k]=true;
                break;
              case 7: // tremolo
                if (effectVal!=0) {
                  tremStatus[k]=effectVal;
                  tremStatusChanged[k]=true;
                }
                treming[k]=true;
                break;
              case 8: // panning
                p->data[j][effectCol[k]++]=0x80;
                p->data[j][effectCol[k]++]=effectVal;
                writePanning=false;
                break;
              case 9: // offset
                if (hasNote) {
                  p->data[j][effectCol[k]++]=0x91;
                  p->data[j][effectCol[k]++]=effectVal;
                }
                break;
              case 0xa: // vol slide
                if (effectVal!=0) {
                  volSlideStatus[k]=effectVal;
                  volSlideStatusChanged[k]=true;
                }
                if (hasNote || hasIns) {
                  volSlideStatusChanged[k]=true;
                }
                volSliding[k]=true;
                break;
              case 0xb: // go to order
                p->data[j][effectCol[k]++]=0x0b;
                p->data[j][effectCol[k]++]=effectVal;
                break;
              case 0xc: // set volume
                p->data[j][3]=effectVal;
                break;
              case 0xd: // next order
                p->data[j][effectCol[k]++]=0x0d;
                p->data[j][effectCol[k]++]=effectVal;
                break;
              case 0xe: // special...
                // TODO: implement the rest
                switch (effectVal>>4) {
                  case 0x5:
                    p->data[j][effectCol[k]++]=0xe5;
                    p->data[j][effectCol[k]++]=(effectVal&15)<<4;
                    break;
                  case 0xc:
                    p->data[j][effectCol[k]++]=0xec;
                    p->data[j][effectCol[k]++]=effectVal&15;
                    break;
                  case 0xd:
                    p->data[j][effectCol[k]++]=0xed;
                    p->data[j][effectCol[k]++]=effectVal&15;
                    break;
                }
                break;
              case 0xf: // speed/tempp
                if (effectVal>=0x20) {
                  p->data[j][effectCol[k]++]=0xf0;
                } else {
                  p->data[j][effectCol[k]++]=0x0f;
                }
                p->data[j][effectCol[k]++]=effectVal;
                break;
              case 0x10: // G: global volume (!)
                break;
              case 0x11: // H: global volume slide (!)
                break;
              case 0x14: // K: key off
                p->data[j][effectCol[k]++]=0xe7;
                p->data[j][effectCol[k]++]=effectVal;
                break;
              case 0x15: // L: set envelope position (!)
                break;
              case 0x19: // P: pan slide
                if (effectVal!=0) {
                  panSlideStatus[k]=effectVal;
                  panSlideStatusChanged[k]=true;
                }
                panSliding[k]=true;
                break;
              case 0x1b: // R: retrigger
                p->data[j][effectCol[k]++]=0x0c;
                p->data[j][effectCol[k]++]=effectVal;
                break;
              case 0x1d: // T: tremor (!)
                break;
              case 0x21: // X: extra fine volume
                break;
              case 0x22: // Y: panbrello (extension)
                if (effectVal!=0) {
                  panStatus[k]=effectVal;
                  panStatusChanged[k]=true;
                }
                panning[k]=true;
                break;
            }
          }

          if (writePanning && hasNote && note<96 && ins>0) {
            p->data[j][effectCol[k]++]=0x80;
            p->data[j][effectCol[k]++]=samplePan[(ins-1)&255][noteMap[(ins-1)&255][note&127]];
          }
        }

        // commit effects
        for (int k=0; k<totalChans; k++) {
          DivPattern* p=ds.subsong[0]->pat[k].getPattern(i,true);
          if (vibing[k]!=vibingOld[k] || vibStatusChanged[k]) {
            p->data[j][effectCol[k]++]=0x04;
            p->data[j][effectCol[k]++]=vibing[k]?vibStatus[k]:0;
            doesVibrato[k]=true;
          } else if (doesVibrato[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0x04;
            p->data[j][effectCol[k]++]=0;
          }

          if (volSliding[k]!=volSlidingOld[k] || volSlideStatusChanged[k]) {
            if (volSlideStatus[k]>=0xf1 && volSliding[k]) {
              p->data[j][effectCol[k]++]=0xf9;
              p->data[j][effectCol[k]++]=volSlideStatus[k]&15;
              volSliding[k]=false;
            } else if ((volSlideStatus[k]&15)==15 && volSlideStatus[k]>=0x10 && volSliding[k]) {
              p->data[j][effectCol[k]++]=0xf8;
              p->data[j][effectCol[k]++]=volSlideStatus[k]>>4;
              volSliding[k]=false;
            } else {
              p->data[j][effectCol[k]++]=0xfa;
              p->data[j][effectCol[k]++]=volSliding[k]?volSlideStatus[k]:0;
            }
            doesVolSlide[k]=true;
          } else if (doesVolSlide[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0xfa;
            p->data[j][effectCol[k]++]=0;
          }

          if (porting[k]!=portingOld[k] || portaStatusChanged[k]) {
            if (portaStatus[k]>=0xe0 && portaType[k]!=3 && porting[k]) {
              p->data[j][effectCol[k]++]=portaType[k]|0xf0;
              p->data[j][effectCol[k]++]=(portaStatus[k]&15)*((portaStatus[k]>=0xf0)?1:1);
              porting[k]=false;
            } else {
              p->data[j][effectCol[k]++]=portaType[k];
              p->data[j][effectCol[k]++]=porting[k]?portaStatus[k]:0;
            }
            doesPitchSlide[k]=true;
          } else if (doesPitchSlide[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0x01;
            p->data[j][effectCol[k]++]=0;
          }

          if (arping[k]!=arpingOld[k] || arpStatusChanged[k]) {
            p->data[j][effectCol[k]++]=0x00;
            p->data[j][effectCol[k]++]=arping[k]?arpStatus[k]:0;
            doesArp[k]=true;
          } else if (doesArp[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0x00;
            p->data[j][effectCol[k]++]=0;
          }

          if (treming[k]!=tremingOld[k] || tremStatusChanged[k]) {
            p->data[j][effectCol[k]++]=0x07;
            p->data[j][effectCol[k]++]=treming[k]?tremStatus[k]:0;
            doesTremolo[k]=true;
          } else if (doesTremolo[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0x07;
            p->data[j][effectCol[k]++]=0;
          }

          if (panning[k]!=panningOld[k] || panStatusChanged[k]) {
            p->data[j][effectCol[k]++]=0x84;
            p->data[j][effectCol[k]++]=panning[k]?panStatus[k]:0;
            doesPanbrello[k]=true;
          } else if (doesPanbrello[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0x84;
            p->data[j][effectCol[k]++]=0;
          }

          if (panSliding[k]!=panSlidingOld[k] || panSlideStatusChanged[k]) {
            p->data[j][effectCol[k]++]=0x83;
            p->data[j][effectCol[k]++]=panSliding[k]?panSlideStatus[k]:0;
            doesPanSlide[k]=true;
          } else if (doesPanSlide[k] && mustCommitInitial) {
            p->data[j][effectCol[k]++]=0x83;
            p->data[j][effectCol[k]++]=0;
          }

          if ((effectCol[k]>>1)-2>ds.subsong[0]->pat[k].effectCols) {
            ds.subsong[0]->pat[k].effectCols=(effectCol[k]>>1)-1;
          }
        }

        memset(effectCol,4,64);
        memcpy(vibingOld,vibing,64*sizeof(bool));
        memcpy(volSlidingOld,volSliding,64*sizeof(bool));
        memcpy(portingOld,porting,64*sizeof(bool));
        memcpy(arpingOld,arping,64*sizeof(bool));
        memset(vibStatusChanged,0,64*sizeof(bool));
        memset(volSlideStatusChanged,0,64*sizeof(bool));
        memset(portaStatusChanged,0,64*sizeof(bool));
        memset(arpStatusChanged,0,64*sizeof(bool));
        memset(vibing,0,64*sizeof(bool));
        memset(volSliding,0,64*sizeof(bool));
        memset(porting,0,64*sizeof(bool));
        memset(arping,0,64*sizeof(bool));
        mustCommitInitial=false;
        if (j==totalRows-1) {
          // place end of pattern marker
          DivPattern* p=ds.subsong[0]->pat[0].getPattern(i,true);
          p->data[j][effectCol[0]++]=0x0d;
          p->data[j][effectCol[0]++]=0;

          if ((effectCol[0]>>1)-2>ds.subsong[0]->pat[0].effectCols) {
            ds.subsong[0]->pat[0].effectCols=(effectCol[0]>>1)-1;
          }
        }
      }

      logV("seeking to %x...",packedSeek);
      if (!reader.seek(packedSeek,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }
    }

    ds.sampleLen=ds.sample.size();
    if (ds.sampleLen>256) {
      logE("too many samples!");
      lastError="too many samples";
      ds.unload();
      delete[] file;
      return false;
    }

    // set channel visibility
    for (int i=totalChans; i<((totalChans+32)&(~31)); i++) {
      ds.subsong[0]->chanShow[i]=false;
      ds.subsong[0]->chanShowChanOsc[i]=false;
    }

    // find subsongs
    ds.findSubSongs(totalChans);

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

