/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

extern "C" {
#include "../../../extern/itcompress/compression.h"
}

static const unsigned char volPortaSlide[10]={
  0, 1, 4, 8, 16, 32, 64, 96, 128, 255
};

void readEnvelope(SafeReader& reader, DivInstrument* ins, int env) {
  unsigned char flags=reader.readC();
  unsigned char numPoints=reader.readC();
  unsigned char loopStart=reader.readC();
  unsigned char loopEnd=reader.readC();
  unsigned char susStart=reader.readC();
  unsigned char susEnd=reader.readC();

  if (numPoints>25) numPoints=25;

  if (loopStart>=numPoints) loopStart=numPoints-1;
  if (loopEnd>=numPoints) loopEnd=numPoints-1;
  if (susStart>=numPoints) susStart=numPoints-1;
  if (susEnd>=numPoints) susEnd=numPoints-1;

  unsigned short pointTime[25];
  signed char pointVal[25];

  for (int i=0; i<25; i++) {
    pointVal[i]=reader.readC();
    pointTime[i]=reader.readS();
  }

  // x
  reader.readC();

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
    case 2: // pitch or cutoff
      if (flags&128) {
        target=&ins->std.ex1Macro; // ES5506 filter
      } else {
        target=&ins->std.pitchMacro;
      }
      break;
  }
  target->len=0;
  int point=0;
  bool pointJustBegan=true;
  // mark loop end as end of envelope
  if (flags&2) {
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
      if (flags&2) { // loop
        if (point==loopStart && (!(flags&4) || susStart==susEnd || loopStart>=susEnd)) {
          if (loopStart!=loopEnd) {
            target->loop=i;
          }
        }
      }
      if (flags&4) { // sustain
        if (susStart!=susEnd) { // sustain loop
          if (point==susStart) {
            target->loop=i;
          }
        }
        if (point==susEnd) {
          target->rel=i;
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

    if (env==2) {
      if (flags&128) {
        p0+=32;
        p1+=32;
        p0*=512;
        p1*=512;
      } else {
        p0*=64;
        p1*=64;
      }
    }

    target->len=i+1;
    target->val[i]=p0+(((p1-p0)*curTime)/timeDiff);
  }

  // split L/R
  if (env==1) {
    for (int i=0; i<ins->std.panLMacro.len; i++) {
      int val=ins->std.panLMacro.val[i];
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

bool DivEngine::loadIT(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[4];

  unsigned char chanPan[64];
  unsigned char chanVol[64];

  unsigned char orders[256];

  unsigned int insPtr[256];
  unsigned int samplePtr[256];
  unsigned int patPtr[256];

  unsigned short patLen[256];

  unsigned char defVol[256];
  unsigned char defPan[256];
  unsigned char defPanIns[256];
  unsigned char noteMap[256][128];

  bool doesPitchSlide[64];
  bool doesVibrato[64];
  bool doesPanning[64];
  bool doesVolSlide[64];
  bool doesPanSlide[64];
  bool doesArp[64];
  bool doesTremolo[64];
  bool doesPanbrello[64];

  memset(doesPitchSlide,0,64*sizeof(bool));
  memset(doesVibrato,0,64*sizeof(bool));
  memset(doesPanning,0,64*sizeof(bool));
  memset(doesVolSlide,0,64*sizeof(bool));
  memset(doesPanSlide,0,64*sizeof(bool));
  memset(doesArp,0,64*sizeof(bool));
  memset(doesTremolo,0,64*sizeof(bool));
  memset(doesPanbrello,0,64*sizeof(bool));
  
  SafeReader reader=SafeReader(file,len);
  warnings="";

  memset(chanPan,0,64);
  memset(chanVol,0,64);
  memset(orders,0,256);
  memset(patLen,0,256*sizeof(unsigned short));

  memset(defVol,0,256);
  memset(defPan,0,256);
  memset(defPanIns,128,256);
  memset(noteMap,0,256*128);

  try {
    DivSong ds;
    ds.version=DIV_VERSION_IT;
    ds.compatFlags.noSlidesOnFirstTick=true;
    ds.compatFlags.rowResetsArpPos=true;
    ds.compatFlags.ignoreJumpAtEnd=false;
    ds.compatFlags.pitchSlideSpeed=8;

    logV("Impulse Tracker module");

    // load here
    if (!reader.seek(0,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    reader.read(magic,4);

    if (memcmp(magic,DIV_IT_MAGIC,4)!=0) {
      logW("the magic isn't complete");
      throw EndOfFileException(&reader,reader.tell());
    }

    ds.name=reader.readStringLatin1(26);

    unsigned char hilight1=reader.readC();
    unsigned char hilight2=reader.readC();
    logV("highlights: %d %d",hilight1,hilight2);

    unsigned short ordersLen=reader.readS();
    ds.insLen=reader.readS();
    ds.sampleLen=reader.readS();
    unsigned short patCount=reader.readS();
    unsigned short usedTracker=reader.readS();
    unsigned short compatTracker=reader.readS();
    unsigned short flags=reader.readS();
    unsigned short special=reader.readS();

    if (ds.insLen<0 || ds.insLen>256) {
      logE("too many instruments!");
      lastError="too many instruments";
      delete[] file;
      return false;
    }

    if (ds.sampleLen<0 || ds.sampleLen>256) {
      logE("too many samples!");
      lastError="too many samples";
      delete[] file;
      return false;
    }

    if (patCount>256) {
      logE("too many patterns!");
      lastError="too many patterns";
      delete[] file;
      return false;
    }

    if (flags&8) {
      ds.compatFlags.linearPitch=1;
    } else {
      ds.compatFlags.linearPitch=0;
    }

    unsigned char globalVol=reader.readC();
    unsigned char masterVol=reader.readC();
    unsigned char initSpeed=reader.readC();
    unsigned char initTempo=reader.readC();
    unsigned char panSep=reader.readC();
    reader.readC(); // PWD - unused

    ds.subsong[0]->hz=(double)initTempo/2.5;
    ds.subsong[0]->speeds.val[0]=initSpeed;
    ds.subsong[0]->speeds.len=1;

    logV("used tracker: %x",usedTracker);
    logV("compatible with: %x",compatTracker);
    logV("flags: %x",flags);
    logV("special: %x",special);

    logV("global vol: %d",globalVol);
    logV("master vol: %d",masterVol);
    logV("pan sep: %d",panSep);

    unsigned short commentLen=reader.readS();
    unsigned int commentPtr=reader.readI();

    logV("comment len: %d",commentLen);
    logV("comment ptr: %x",commentPtr);

    reader.readI(); // reserved

    reader.read(chanPan,64);
    reader.read(chanVol,64);

    logD("reading orders...");
    size_t curSubSong=0;
    int curOrder1=0;
    ds.subsong[curSubSong]->ordersLen=0;
    bool subSongIncreased=false;
    for (int i=0; i<ordersLen; i++) {
      unsigned char nextOrder=reader.readC();
      orders[i]=curOrder1;
      
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
        curOrder1=0;
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
      curOrder1++;
    }

    for (int i=0; i<ds.insLen; i++) {
      insPtr[i]=reader.readI();
    }

    for (int i=0; i<ds.sampleLen; i++) {
      samplePtr[i]=reader.readI();
    }

    for (int i=0; i<patCount; i++) {
      patPtr[i]=reader.readI();
    }

    // skip edit history if present
    if (special&2) {
      logD("skipping edit history...");
      unsigned short editHistSize=reader.readS();
      if (editHistSize>0) {
        if (!reader.seek(editHistSize*8,SEEK_CUR)) {
          logV("what? I wasn't expecting that from you.");
        }
      }
    }

    // read extension blocks, if any
    logD("looking for extensions...");
    bool hasExtensions=true;
    while (hasExtensions) {
      char extType[4];
      unsigned int extSize=0;
      memset(extType,0,4);

      reader.read(extType,4);
      extSize=reader.readI();

      if (memcmp(extType,"PNAM",4)==0) {
        logV("found MPT extension: pattern names");
        // check whether this block is valid
        if (extSize>patCount*32) {
          logV("block may not be valid");
          break;
        }
        // read pattern names
        logV("reading pattern names...");
        for (unsigned int i=0; i<(extSize>>5); i++) {
          DivPattern* p=ds.subsong[0]->pat[0].getPattern(i,true);
          p->name=reader.readStringLatin1(32);
        }
      } else if (memcmp(extType,"CNAM",4)==0) {
        logV("found MPT extension: channel names");
        // check whether this block is valid
        if (extSize>DIV_MAX_CHANS*20) {
          logV("block may not be valid");
          break;
        }
        // read channel names
        logV("reading channel names...");
        for (unsigned int i=0; i<(extSize>>5); i++) {
          String chanName=reader.readStringLatin1(20);
          for (DivSubSong* j: ds.subsong) {
            j->chanName[i]=chanName;
          }
        }
      } else if (memcmp(extType,"CHFX",4)==0) {
        logV("found MPT extension: channel effects");
        // skip (stop if we cannot seek)
        if (!reader.seek(extSize,SEEK_CUR)) {
          break;
        }
      } else if (
        extType[0]=='F' &&
        (extType[1]=='X' || (extType[1]>='0' && extType[1]<='9')) &&
        (extType[2]>='0' && extType[2]<='9') &&
        (extType[3]>='0' && extType[3]<='9')
      ) { // effect slot
        logV("found MPT extension: effect slot");
        // skip (stop if we cannot seek)
        if (!reader.seek(extSize,SEEK_CUR)) {
          break;
        }
      } else {
        logV("no further extensions found... %.2x%.2x%.2x%.2x",extType[0],extType[1],extType[2],extType[3]);
        hasExtensions=false;
      }
    }

    // read song comment
    logD("reading song comment...");
    if (reader.seek(commentPtr,SEEK_SET)) {
      try {
        String comment=reader.readStringLatin1Special(commentLen);

        ds.notes="";
        ds.notes.reserve(comment.size());

        for (char& i: comment) {
          if (i=='\r') {
            ds.notes+='\n';
          } else {
            ds.notes+=i;
          }
        }
      } catch (EndOfFileException& e) {
        logW("couldn't read song comment due to premature end of file.");
      }
    } else {
      logW("couldn't seek to comment!");
    }
    

    // read instruments
    for (int i=0; i<ds.insLen; i++) {
      DivInstrument* ins=new DivInstrument;
      ins->type=DIV_INS_ES5506;

      if (insPtr[i]==0) {
        ds.ins.push_back(ins);
        continue;
      }

      logV("reading instrument %d...",i);
      if (!reader.seek(insPtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete ins;
        delete[] file;
        return false;
      }

      reader.read(magic,4);

      if (memcmp(magic,"IMPI",4)!=0) {
        logE("invalid instrument header!");
        lastError="invalid instrument header";
        delete ins;
        delete[] file;
        return false;
      }

      String dosName=reader.readStringLatin1(12);

      reader.readC(); // 0

      unsigned char initCut=255;
      unsigned char initRes=255;
      unsigned char insVol=128;
      unsigned short fadeOut=256;

      if (compatTracker<0x200) { // old format
        unsigned char flags=reader.readC();
        unsigned char volLoopStart=reader.readC();
        unsigned char volLoopEnd=reader.readC();
        unsigned char susLoopStart=reader.readC();
        unsigned char susLoopEnd=reader.readC();

        logV("flags: %x",flags);
        logV("volLoopStart: %d",volLoopStart);
        logV("volLoopEnd: %d",volLoopEnd);
        logV("susLoopStart: %d",susLoopStart);
        logV("susLoopEnd: %d",susLoopEnd);

        reader.readS(); // x

        fadeOut=reader.readS();

        logV("fadeOut: %d",fadeOut);

        // NNA/duplicate check - not supported by Furnace...
        reader.readC();
        reader.readC();

        // version/sample count/x
        reader.readI();
      } else { // new format
        // NNA/duplicate check - not supported by Furnace...
        reader.readC();
        reader.readC();
        reader.readC();

        fadeOut=reader.readS();

        logV("fadeOut: %d",fadeOut);

        reader.readC();
        reader.readC();

        insVol=reader.readC();
        defPanIns[i]=reader.readC();

        // vol/pan randomization
        reader.readC();
        reader.readC();

        // version/sample count/x
        reader.readI();
      }

      ins->name=reader.readStringLatin1(26);

      if (compatTracker<0x200) { // old format
        // x
        reader.readC();
        reader.readC();
        reader.readC();
        reader.readC();
        reader.readC();
        reader.readC();
      } else { // new format
        // filter params
        initCut=reader.readC();
        initRes=reader.readC();

        // MIDI stuff - ignored
        reader.readI();
      }

      logV("filter: %d/%d",initCut,initRes);

      // note map
      ins->amiga.useNoteMap=true;
      for (int j=0; j<120; j++) {
        ins->amiga.noteMap[j].freq=(unsigned char)reader.readC();
        ins->amiga.noteMap[j].map=reader.readC()-1;
        noteMap[i][j]=ins->amiga.noteMap[j].map;
      }

      // envelopes...
      if (compatTracker<0x200) { // old format
        // TODO
      } else {
        readEnvelope(reader,ins,0);
        readEnvelope(reader,ins,1);
        readEnvelope(reader,ins,2);
      }

      // scale envelope to global volume
      if (insVol>128) insVol=128;
      if (ins->std.volMacro.len==0) {
        ins->std.volMacro.len=1;
        ins->std.volMacro.val[0]=insVol>>1;
      } else {
        for (int j=0; j<ins->std.volMacro.len; j++) {
          ins->std.volMacro.val[i]=(ins->std.volMacro.val[i]*insVol)>>7;
        }
      }

      // add note fade if there isn't a release point in the volume envelope
      if (ins->std.volMacro.len>0 && ins->std.volMacro.rel>=ins->std.volMacro.len) {
        ins->std.volMacro.len--;
        int initial=ins->std.volMacro.val[ins->std.volMacro.len];
        ins->std.volMacro.rel=ins->std.volMacro.len;
        for (int i=1024; ins->std.volMacro.len<255; i-=fadeOut) {
          ins->std.volMacro.val[ins->std.volMacro.len++]=(initial*i)>>10;
          if (i<0) break;
        }
      }

      // set filter macro to cutoff value if no filter envelope is defined
      if (ins->std.ex1Macro.len==0 && initCut&0x80) {
        if ((initCut&0x7f)==0x7f) {
          ins->std.ex1Macro.len=1;
          ins->std.ex1Macro.val[0]=65535;
        } else {
          ins->std.ex1Macro.len=1;
          ins->std.ex1Macro.val[0]=1024+(initCut&0x7f)*400;
        }
      } else {
        ins->std.ex1Macro.len=1;
        ins->std.ex1Macro.val[0]=65535;
      }

      ds.ins.push_back(ins);
    }

    // read samples
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* s=new DivSample;

      if (samplePtr[i]==0) {
        ds.sample.push_back(s);

        // does the song not use instruments?
        // create instrument then
        if (ds.insLen==0) {
          DivInstrument* ins=new DivInstrument;
          ins->type=DIV_INS_ES5506;
          ds.ins.push_back(ins);
        }
        continue;
      }

      logV("reading sample %d...",i);
      if (!reader.seek(samplePtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete s;
        delete[] file;
        return false;
      }

      reader.read(magic,4);

      if (memcmp(magic,"IMPS",4)!=0) {
        logW("invalid sample header!");
        lastError="invalid sample header";
        delete s;
        delete[] file;
        return false;
      }

      String dosName=reader.readStringLatin1(12);

      reader.readC(); // 0

      unsigned char sampleVol=reader.readC();
      unsigned char flags=reader.readC();
      defVol[i]=reader.readC();

      s->name=reader.readStringLatin1(26);

      unsigned char convert=reader.readC();
      defPan[i]=reader.readC();

      if (flags&2) {
        s->depth=DIV_SAMPLE_DEPTH_16BIT;
      } else {
        s->depth=DIV_SAMPLE_DEPTH_8BIT;
      }

      unsigned int sampleLen=reader.readI();

      if (sampleLen>16777216) {
        logE("abnormal sample size! %x",reader.tell());
        lastError="bad sample size";
        delete[] file;
        return false;
      }

      s->init(sampleLen);
      s->loopStart=reader.readI();
      s->loopEnd=reader.readI();
      s->centerRate=reader.readI()/2;
      if (flags&16) {
        s->loop=true;
        if (flags&64) {
          s->loopMode=DIV_SAMPLE_LOOP_PINGPONG;
        } else {
          s->loopMode=DIV_SAMPLE_LOOP_FORWARD;
        }
      }
      if (flags&32) {
        s->loop=true;
        if (flags&128) {
          s->loopMode=DIV_SAMPLE_LOOP_PINGPONG;
        } else {
          s->loopMode=DIV_SAMPLE_LOOP_FORWARD;
        }
        s->loopStart=reader.readI();
        s->loopEnd=reader.readI();
      } else {
        reader.readI();
        reader.readI();
      }

      unsigned int dataPtr=reader.readI();

      unsigned char vibSpeed=reader.readC();
      unsigned char vibDepth=reader.readC();
      unsigned char vibRate=reader.readC();
      unsigned char vibShape=reader.readC();

      logV("vibrato: %d %d %d %d",vibSpeed,vibDepth,vibRate,vibShape);

      if (s->samples>0) {
        logD("seek to %x...",dataPtr);
        if (!reader.seek(dataPtr,SEEK_SET)) {
          logE("premature end of file!");
          lastError="incomplete file";
          delete s;
          delete[] file;
          return false;
        }
      } else {
        logD("seek not needed...");
      }

      logV("reading sample data (%d)",s->samples);

      if (flags&8) { // compressed sample
        unsigned int ret=0;
        logV("decompression begin... (%d)",s->samples);
        if (flags&4) {
          logW("STEREO!");
          if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
            logV("16-bit");
            short* outData=new short[s->samples*2];
            ret=it_decompress16(outData,s->samples,&file[reader.tell()],len-reader.tell(),(convert&4)?1:0,(flags&4)?2:1);
            for (unsigned int i=0; i<s->samples; i++) {
              s->data16[i]=(outData[i<<1]+outData[1+(i<<1)])>>1;
            }
            delete[] outData;
          } else {
            logV("8-bit");
            signed char* outData=new signed char[s->samples*2];
            ret=it_decompress8(outData,s->samples,&file[reader.tell()],len-reader.tell(),(convert&4)?1:0,(flags&4)?2:1);
            for (unsigned int i=0; i<s->samples; i++) {
              s->data8[i]=(outData[i<<1]+outData[1+(i<<1)])>>1;
            }
            delete[] outData;
          }
        } else {
          if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
            logV("16-bit");
            ret=it_decompress16(s->data16,s->samples,&file[reader.tell()],len-reader.tell(),(convert&4)?1:0,(flags&4)?2:1);
          } else {
            logV("8-bit");
            ret=it_decompress8(s->data8,s->samples,&file[reader.tell()],len-reader.tell(),(convert&4)?1:0,(flags&4)?2:1);
          }
        }
        logV("got: %d",ret);
      } else {
        try {
          if (s->depth==DIV_SAMPLE_DEPTH_16BIT) {
            if (flags&4) { // downmix stereo
              for (unsigned int i=0; i<s->samples; i++) {
                short l;
                if (convert&2) {
                  l=reader.readS_BE();
                } else {
                  l=reader.readS();
                }
                if (!(convert&1)) {
                  l^=0x8000;
                }
                s->data16[i]=l;
              }
              for (unsigned int i=0; i<s->samples; i++) {
                short r;
                if (convert&2) {
                  r=reader.readS_BE();
                } else {
                  r=reader.readS();
                }
                if (!(convert&1)) {
                  r^=0x8000;
                }
                s->data16[i]=(s->data16[i]+r)>>1;
              }
            } else {
              for (unsigned int i=0; i<s->samples; i++) {
                if (convert&2) {
                  s->data16[i]=reader.readS_BE()^((convert&1)?0:0x8000);
                } else {
                  s->data16[i]=reader.readS()^((convert&1)?0:0x8000);
                }
              }
            }
          } else {
            if (flags&4) { // downmix stereo
              for (unsigned int i=0; i<s->samples; i++) {
                signed char l=reader.readC();
                if (!(convert&1)) {
                  l^=0x80;
                }
                s->data8[i]=l;
              }
              for (unsigned int i=0; i<s->samples; i++) {
                signed char r=reader.readC();
                if (!(convert&1)) {
                  r^=0x80;
                }
                s->data8[i]=(s->data8[i]+r)>>1;
              }
            } else {
              for (unsigned int i=0; i<s->samples; i++) {
                s->data8[i]=reader.readC()^((convert&1)?0:0x80);
              }
            }
          }
        } catch (EndOfFileException& e) {
          logW("premature end of file...");
        }
      }

      // scale sample if necessary
      if (s->samples>0) {
        if (sampleVol>64) sampleVol=64;
        if (sampleVol<64) {
          // convert to 16-bit
          if (s->depth==DIV_SAMPLE_DEPTH_8BIT) {
            s->convert(DIV_SAMPLE_DEPTH_16BIT,0);
          }

          // then scale
          for (unsigned int i=0; i<s->samples; i++) {
            s->data16[i]=(s->data16[i]*sampleVol)>>6;
          }
        }
      }

      // does the song not use instruments?
      // create instrument then
      if (ds.insLen==0) {
        DivInstrument* ins=new DivInstrument;
        ins->name=s->name;
        ins->type=DIV_INS_ES5506;
        ins->amiga.initSample=i;
        ds.ins.push_back(ins);
      }

      ds.sample.push_back(s);
    }

    // scan pattern data for effect use
    int maxChan=0;
    for (int i=0; i<patCount; i++) {
      if (patPtr[i]==0) continue;

      unsigned char mask[64];
      unsigned char note[64];
      unsigned char ins[64];
      unsigned char vol[64];
      unsigned char effect[64];
      unsigned char effectVal[64];
      int readRow=0;

      memset(mask,0,64);
      memset(note,0,64);
      memset(ins,0,64);
      memset(vol,0,64);
      memset(effect,0,64);
      memset(effectVal,0,64);

      logV("scanning pattern %d...",i);
      if (!reader.seek(patPtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }

      unsigned int dataLen=(unsigned short)reader.readS();
      unsigned short patRows=reader.readS();

      patLen[i]=patRows;

      if (patRows>DIV_MAX_ROWS) {
        logE("too many rows! %d",patRows);
        lastError="too many rows";
        delete[] file;
        return false;
      }

      reader.readI(); // x

      dataLen+=reader.tell();

      // read pattern data
      while (reader.tell()<dataLen) {
        unsigned char chan=reader.readC();
        bool hasVol=false;
        bool hasEffect=false;

        if (chan==0) {
          readRow++;
          if (readRow>=patRows) {
            break;
          }
          continue;
        }

        if (chan&128) {
          mask[(chan-1)&63]=reader.readC();
        }
        chan=(chan-1)&63;

        if (chan>maxChan) maxChan=chan;

        if (mask[chan]&1) {
          note[chan]=reader.readC();
        }
        if (mask[chan]&2) {
          ins[chan]=reader.readC();
        }
        if (mask[chan]&4) {
          vol[chan]=reader.readC();
          hasVol=true;
        }
        if (mask[chan]&8) {
          effect[chan]=reader.readC();
          effectVal[chan]=reader.readC();
          hasEffect=true;
        }
        if (mask[chan]&64) {
          hasVol=true;
        }
        if (mask[chan]&128) {
          hasEffect=true;
        }

        if (hasVol) {
          if (vol[chan]>64) {
            if (vol[chan]>=65 && vol[chan]<=74) { // fine vol up
              doesVolSlide[chan]=true;
            } else if (vol[chan]>=75 && vol[chan]<=84) { // fine vol down
              doesVolSlide[chan]=true;
            } else if (vol[chan]>=85 && vol[chan]<=94) { // vol slide up
              doesVolSlide[chan]=true;
            } else if (vol[chan]>=95 && vol[chan]<=104) { // vol slide down
              doesVolSlide[chan]=true;
            } else if (vol[chan]>=105 && vol[chan]<=114) { // pitch down
              doesPitchSlide[chan]=true;
            } else if (vol[chan]>=115 && vol[chan]<=124) { // pitch up
              doesPitchSlide[chan]=true;
            } else if (vol[chan]>=193 && vol[chan]<=202) { // porta
              doesPitchSlide[chan]=true;
            } else if (vol[chan]>=203 && vol[chan]<=212) { // vibrato
              doesVibrato[chan]=true;
            }
          }
        }
        if (hasEffect) {
          switch (effect[chan]+'A'-1) {
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
            case 'P': // pan slide
              doesPanSlide[chan]=true;
              break;
            case 'R': // tremolo
              doesTremolo[chan]=true;
              break;
            case 'U': // fine vibrato
              doesVibrato[chan]=true;
              break;
            case 'Y': // panbrello
              doesPanbrello[chan]=true;
              break;
          }
        }
      }
    }

    // read patterns
    for (int i=0; i<patCount; i++) {
      unsigned char effectCol[64];
      unsigned char vibStatus[64];
      bool vibStatusChanged[64];
      bool vibing[64];
      bool vibingOld[64];
      unsigned char volSlideStatus[64];
      bool volSlideStatusChanged[64];
      bool volSliding[64];
      bool volSlidingOld[64];
      unsigned char portaStatus[64];
      bool portaStatusChanged[64];
      bool porting[64];
      bool portingOld[64];
      unsigned char portaType[64];
      unsigned char arpStatus[64];
      bool arpStatusChanged[64];
      bool arping[64];
      bool arpingOld[64];
      unsigned char panStatus[64];
      bool panStatusChanged[64];
      bool panning[64];
      bool panningOld[64];
      unsigned char tremStatus[64];
      bool tremStatusChanged[64];
      bool treming[64];
      bool tremingOld[64];
      unsigned char panSlideStatus[64];
      bool panSlideStatusChanged[64];
      bool panSliding[64];
      bool panSlidingOld[64];
      bool did[64];
      unsigned char lastRetrig[64];

      if (patPtr[i]==0) continue;

      unsigned char mask[64];
      unsigned char note[64];
      unsigned char ins[64];
      unsigned char vol[64];
      unsigned char effect[64];
      unsigned char effectVal[64];
      int readRow=0;
      bool mustCommitInitial=true;

      memset(effectCol,0,64);
      memset(vibStatus,0,64);
      memset(vibStatusChanged,0,64*sizeof(bool));
      memset(vibing,0,64*sizeof(bool));
      memset(vibingOld,0,64*sizeof(bool));
      memset(volSlideStatus,0,64);
      memset(volSlideStatusChanged,0,64*sizeof(bool));
      memset(volSliding,0,64*sizeof(bool));
      memset(volSlidingOld,0,64*sizeof(bool));
      memset(portaStatus,0,64);
      memset(portaStatusChanged,0,64*sizeof(bool));
      memset(porting,0,64*sizeof(bool));
      memset(portingOld,0,64*sizeof(bool));
      memset(portaType,0,64);
      memset(arpStatus,0,64);
      memset(arpStatusChanged,0,64*sizeof(bool));
      memset(arping,0,64*sizeof(bool));
      memset(arpingOld,0,64*sizeof(bool));
      memset(panStatus,0,64);
      memset(panStatusChanged,0,64*sizeof(bool));
      memset(panning,0,64*sizeof(bool));
      memset(panningOld,0,64*sizeof(bool));
      memset(tremStatus,0,64);
      memset(tremStatusChanged,0,64*sizeof(bool));
      memset(treming,0,64*sizeof(bool));
      memset(tremingOld,0,64*sizeof(bool));
      memset(panSlideStatus,0,64);
      memset(panSlideStatusChanged,0,64*sizeof(bool));
      memset(panSliding,0,64*sizeof(bool));
      memset(panSlidingOld,0,64*sizeof(bool));
      memset(did,0,64*sizeof(bool));
      memset(lastRetrig,0,64);

      memset(mask,0,64);
      memset(note,0,64);
      memset(ins,0,64);
      memset(vol,0,64);
      memset(effect,0,64);
      memset(effectVal,0,64);

      logV("reading pattern %d...",i);
      if (!reader.seek(patPtr[i],SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete[] file;
        return false;
      }

      unsigned int dataLen=(unsigned short)reader.readS();
      unsigned short patRows=reader.readS();

      patLen[i]=patRows;

      if (patRows>DIV_MAX_ROWS) {
        logE("too many rows! %d",patRows);
        lastError="too many rows";
        delete[] file;
        return false;
      }

      reader.readI(); // x

      dataLen+=reader.tell();

      // read pattern data
      while (reader.tell()<dataLen) {
        unsigned char chan=reader.readC();
        bool hasNote=false;
        bool hasIns=false;
        bool hasVol=false;
        bool hasEffect=false;

        if (chan==0) {
          // commit effects
          for (int j=0; j<64; j++) {
            DivPattern* p=ds.subsong[0]->pat[j].getPattern(i,true);
            if (vibing[j]!=vibingOld[j] || vibStatusChanged[j]) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x04;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=vibing[j]?vibStatus[j]:0;
              doesVibrato[j]=true;
            } else if (doesVibrato[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x04;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if (volSliding[j]!=volSlidingOld[j] || volSlideStatusChanged[j]) {
              if (volSlideStatus[j]>=0xf1 && volSliding[j]) {
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0xf9;
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=volSlideStatus[j]&15;
                volSliding[j]=false;
              } else if ((volSlideStatus[j]&15)==15 && volSlideStatus[j]>=0x10 && volSliding[j]) {
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0xf8;
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=volSlideStatus[j]>>4;
                volSliding[j]=false;
              } else {
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0xfa;
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=volSliding[j]?volSlideStatus[j]:0;
              }
              doesVolSlide[j]=true;
            } else if (doesVolSlide[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0xfa;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if (porting[j]!=portingOld[j] || portaStatusChanged[j]) {
              if (portaStatus[j]>=0xe0 && portaType[j]!=3 && porting[j]) {
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=portaType[j]|0xf0;
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=(portaStatus[j]&15)*((portaStatus[j]>=0xf0)?1:1);
                porting[j]=false;
              } else {
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=portaType[j];
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=porting[j]?portaStatus[j]:0;
              }
              doesPitchSlide[j]=true;
            } else if (doesPitchSlide[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x01;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if (arping[j]!=arpingOld[j] || arpStatusChanged[j]) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x00;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=arping[j]?arpStatus[j]:0;
              doesArp[j]=true;
            } else if (doesArp[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x00;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if (treming[j]!=tremingOld[j] || tremStatusChanged[j]) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x07;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=treming[j]?tremStatus[j]:0;
              doesTremolo[j]=true;
            } else if (doesTremolo[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x07;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if (panning[j]!=panningOld[j] || panStatusChanged[j]) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x84;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=panning[j]?panStatus[j]:0;
              doesPanbrello[j]=true;
            } else if (doesPanbrello[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x84;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if (panSliding[j]!=panSlidingOld[j] || panSlideStatusChanged[j]) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x83;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=panSliding[j]?panSlideStatus[j]:0;
              doesPanSlide[j]=true;
            } else if (doesPanSlide[j] && mustCommitInitial) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0x83;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[j]++]=0;
            }

            if ((effectCol[j]>>1)>=ds.subsong[0]->pat[j].effectCols) {
              ds.subsong[0]->pat[j].effectCols=(effectCol[j]>>1)+1;
            }
          }

          readRow++;
          memcpy(vibingOld,vibing,64*sizeof(bool));
          memcpy(volSlidingOld,volSliding,64*sizeof(bool));
          memcpy(portingOld,porting,64*sizeof(bool));
          memcpy(arpingOld,arping,64*sizeof(bool));
          memcpy(panningOld,panning,64*sizeof(bool));
          memcpy(tremingOld,treming,64*sizeof(bool));
          memcpy(panSlidingOld,panSliding,64*sizeof(bool));
          memset(vibStatusChanged,0,64*sizeof(bool));
          memset(volSlideStatusChanged,0,64*sizeof(bool));
          memset(portaStatusChanged,0,64*sizeof(bool));
          memset(arpStatusChanged,0,64*sizeof(bool));
          memset(panStatusChanged,0,64*sizeof(bool));
          memset(tremStatusChanged,0,64*sizeof(bool));
          memset(panSlideStatusChanged,0,64*sizeof(bool));
          memset(vibing,0,64*sizeof(bool));
          memset(volSliding,0,64*sizeof(bool));
          memset(porting,0,64*sizeof(bool));
          memset(arping,0,64*sizeof(bool));
          memset(panning,0,64*sizeof(bool));
          memset(treming,0,64*sizeof(bool));
          memset(panSliding,0,64*sizeof(bool));
          memset(did,0,64);
          mustCommitInitial=false;
          if (readRow>=patRows) {
            if (readRow>0) {
              // place end of pattern marker
              DivPattern* p=ds.subsong[0]->pat[0].getPattern(i,true);
              p->newData[readRow-1][DIV_PAT_FX(0)+effectCol[0]++]=0x0d;
              p->newData[readRow-1][DIV_PAT_FX(0)+effectCol[0]++]=0;

              if ((effectCol[0]>>1)>=ds.subsong[0]->pat[0].effectCols) {
                ds.subsong[0]->pat[0].effectCols=(effectCol[0]>>1)+1;
              }
            }
            break;
          }
          memset(effectCol,0,64);
          continue;
        }

        if (chan&128) {
          mask[(chan-1)&63]=reader.readC();
        }
        chan=(chan-1)&63;

        if (mask[chan]&1) {
          note[chan]=reader.readC();
          hasNote=true;
        }
        if (mask[chan]&2) {
          ins[chan]=reader.readC();
          hasIns=true;
        }
        if (mask[chan]&4) {
          vol[chan]=reader.readC();
          hasVol=true;
        }
        if (mask[chan]&8) {
          effect[chan]=reader.readC();
          effectVal[chan]=reader.readC();
          hasEffect=true;
        }
        if (mask[chan]&16) {
          hasNote=true;
        }
        if (mask[chan]&32) {
          hasIns=true;
        }
        if (mask[chan]&64) {
          hasVol=true;
        }
        if (mask[chan]&128) {
          hasEffect=true;
        }

        DivPattern* p=ds.subsong[0]->pat[chan].getPattern(i,true);

        if (hasNote) {
          if (note[chan]==255) { // note release
            p->newData[readRow][DIV_PAT_NOTE]=DIV_NOTE_REL;
          } else if (note[chan]==254) { // note off
            p->newData[readRow][DIV_PAT_NOTE]=DIV_NOTE_OFF;
          } else if (note[chan]<120) {
            p->newData[readRow][DIV_PAT_NOTE]=note[chan]+60;
          } else { // note fade, but Furnace does not support that
            p->newData[readRow][DIV_PAT_NOTE]=DIV_MACRO_REL;
          }
        }
        if (hasIns) {
          p->newData[readRow][DIV_PAT_INS]=ins[chan]-1;
          if ((note[chan]<120 || ds.insLen==0) && ins[chan]>0) {
            unsigned char targetPan=0;
            if (ds.insLen==0) {
              targetPan=defPan[(ins[chan]-1)&255];
            } else {
              targetPan=defPan[noteMap[(ins[chan]-1)&255][note[chan]]];
              if (!(targetPan&128)) {
                targetPan=defPanIns[(ins[chan]-1)&255]^0x80;
              }
            }
            if (targetPan&128) {
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x80;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=CLAMP((targetPan&127)<<2,0,255);
            }
          }

          if (hasNote && (note[chan]<120 || ds.insLen==0) && ins[chan]>0) {
            if (ds.insLen==0) {
              p->newData[readRow][DIV_PAT_VOL]=defVol[(ins[chan]-1)&255];
            } else {
              p->newData[readRow][DIV_PAT_VOL]=defVol[noteMap[(ins[chan]-1)&255][note[chan]]];
            }
          }
        }
        if (hasVol) {
          if (vol[chan]<=64) {
            p->newData[readRow][DIV_PAT_VOL]=vol[chan];
          } else { // effects in volume column
            if (vol[chan]>=128 && vol[chan]<=192) { // panning
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x80;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=CLAMP((vol[chan]-128)<<2,0,255);
            } else if (vol[chan]>=65 && vol[chan]<=74) { // fine vol up
            } else if (vol[chan]>=75 && vol[chan]<=84) { // fine vol down
            } else if (vol[chan]>=85 && vol[chan]<=94) { // vol slide up
              if ((vol[chan]-85)!=0) {
                volSlideStatus[chan]=(vol[chan]-85)<<4;
                volSlideStatusChanged[chan]=true;
              }
              if (hasNote || hasIns) {
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
            } else if (vol[chan]>=95 && vol[chan]<=104) { // vol slide down
              if ((vol[chan]-95)!=0) {
                volSlideStatus[chan]=vol[chan]-95;
                volSlideStatusChanged[chan]=true;
              }
              if (hasNote || hasIns) {
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
            } else if (vol[chan]>=105 && vol[chan]<=114) { // pitch down
            } else if (vol[chan]>=115 && vol[chan]<=124) { // pitch up
            } else if (vol[chan]>=193 && vol[chan]<=202) { // porta
              unsigned char portaVal=volPortaSlide[vol[chan]-193];
              if (portaVal!=0) {
                portaStatus[chan]=portaVal;
                portaStatusChanged[chan]=true;
              }
              if (portaType[chan]!=3) {
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=3;
              porting[chan]=true;
            } else if (vol[chan]>=203 && vol[chan]<=212) { // vibrato
              if ((vol[chan]-203)!=0) {
                vibStatus[chan]&=0xf0;
                vibStatus[chan]|=(vol[chan]-203);
                vibStatusChanged[chan]=true;
              }
              vibing[chan]=true;
            }
          }
        }
        if (hasEffect) {
          switch (effect[chan]+'A'-1) {
            case 'A': // speed
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0f;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan];
              break;
            case 'B': // go to order
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0b;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=orders[effectVal[chan]];
              break;
            case 'C': // next order
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0d;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan];
              break;
            case 'D': // vol slide
              if (effectVal[chan]!=0) {
                volSlideStatus[chan]=effectVal[chan];
                volSlideStatusChanged[chan]=true;
              }
              if (hasIns) {
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
              break;
            case 'E': // pitch down
              if (effectVal[chan]!=0) {
                portaStatus[chan]=effectVal[chan];
                portaStatusChanged[chan]=true;
              }
              if (portaType[chan]!=2) {
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=2;
              porting[chan]=true;
              break;
            case 'F': // pitch up
              if (effectVal[chan]!=0) {
                portaStatus[chan]=effectVal[chan];
                portaStatusChanged[chan]=true;
              }
              if (portaType[chan]!=1) {
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=1;
              porting[chan]=true;
              break;
            case 'G': // porta
              if (effectVal[chan]!=0) {
                portaStatus[chan]=effectVal[chan];
                portaStatusChanged[chan]=true;
              }
              if (portaType[chan]!=3 || hasNote) {
                portaStatusChanged[chan]=true;
              }
              portaType[chan]=3;
              porting[chan]=true;
              break;
            case 'H': // vibrato
              if (effectVal[chan]!=0) {
                vibStatus[chan]=effectVal[chan];
                vibStatusChanged[chan]=true;
              }
              vibing[chan]=true;
              break;
            case 'I': // tremor (!)
              break;
            case 'J': // arp
              if (effectVal[chan]!=0) {
                arpStatus[chan]=effectVal[chan];
                arpStatusChanged[chan]=true;
              }
              arping[chan]=true;
              break;
            case 'K': // vol slide + vibrato
              if (effectVal[chan]!=0) {
                volSlideStatus[chan]=effectVal[chan];
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
              vibing[chan]=true;
              break;
            case 'L': // vol slide + porta
              if (effectVal[chan]!=0) {
                volSlideStatus[chan]=effectVal[chan];
                volSlideStatusChanged[chan]=true;
              }
              volSliding[chan]=true;
              porting[chan]=true;
              portaType[chan]=3;
              break;
            case 'M': // channel vol
              break;
            case 'N': // channel vol slide
              break;
            case 'O': // offset
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x91;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan];
              break;
            case 'P': // pan slide
              if (effectVal[chan]!=0) {
                panSlideStatus[chan]=effectVal[chan];
                panSlideStatusChanged[chan]=true;
              }
              panSliding[chan]=true;
              break;
            case 'Q': // retrigger
              if (effectVal[chan]!=0) {
                lastRetrig[chan]=effectVal[chan];
              }
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0c;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=lastRetrig[chan]&15;
              break;
            case 'R': // tremolo
              if (effectVal[chan]!=0) {
                tremStatus[chan]=effectVal[chan];
                tremStatusChanged[chan]=true;
              }
              treming[chan]=true;
              break;
            case 'S': // special...
              switch (effectVal[chan]>>4) {
                case 0x3: // vibrato waveform
                  switch (effectVal[chan]&3) {
                    case 0x0: // sine
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xe3;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x00;
                      break;
                    case 0x1: // ramp down
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xe3;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x05;
                      break;
                    case 0x2: // square
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xe3;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x06;
                      break;
                    case 0x3: // random
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xe3;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x07;
                      break;
                  }
                  break;
                case 0x7:
                  switch (effectVal[chan]&15) {
                    case 0x7: // volume envelope off
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf5;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x00;
                      break;
                    case 0x8: // volume envelope on
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf6;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x00;
                      break;
                    case 0x9: // panning envelope off
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf5;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0c;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf5;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0d;
                      break;
                    case 0xa: // panning envelope on
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf6;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0c;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf6;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x0d;
                      break;
                    case 0xb: // pitch envelope off
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf5;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x04;
                      break;
                    case 0xc: //pitch envelope on
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf6;
                      p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x04;
                      break;
                  }
                  break;
                case 0x8: // panning
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x80;
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=(effectVal[chan]&15)<<4;
                  break;
                case 0xa: // offset (high nibble)
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x92;
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan]&15;
                  break;
                case 0xc: // note cut
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xec;
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan]&15;
                  break;
                case 0xd: // note delay
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xed;
                  p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan]&15;
                  break;
              }
              break;
            case 'T': // tempo
              if (effectVal[chan]>=0x20) {
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0xf0;
                p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan];
              }
              break;
            case 'U': // fine vibrato
              if (effectVal[chan]!=0) {
                vibStatus[chan]=effectVal[chan];
                vibStatusChanged[chan]=true;
              }
              vibing[chan]=true;
              break;
            case 'V': // global volume (!)
              break;
            case 'W': // global volume slide (!)
              break;
            case 'X': // panning
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=0x80;
              p->newData[readRow][DIV_PAT_FX(0)+effectCol[chan]++]=effectVal[chan];
              break;
            case 'Y': // panbrello
              if (effectVal[chan]!=0) {
                panStatus[chan]=effectVal[chan];
                panStatusChanged[chan]=true;
              }
              panning[chan]=true;
              break;
            case 'Z': // MIDI macro
              break;
          }
        }
      }
    }

    maxChan++;

    ds.insLen=ds.ins.size();

    logV("maxChan: %d",maxChan);

    // set channel visibility
    for (int i=maxChan; i<((maxChan+32)&(~31)); i++) {
      ds.subsong[0]->chanShow[i]=false;
      ds.subsong[0]->chanShowChanOsc[i]=false;
    }

    // copy patterns to the rest of subsongs
    int copiesMade=0;
    for (size_t i=1; i<ds.subsong.size(); i++) {
      bool usedPat[256];
      memset(usedPat,0,256*sizeof(bool));
      for (int j=0; j<ds.subsong[i]->ordersLen; j++) {
        usedPat[ds.subsong[i]->orders.ord[0][j]]=true;
      }
      for (int j=0; j<maxChan; j++) {
        for (int k=0; k<patCount; k++) {
          if (!usedPat[k]) continue;
          if (ds.subsong[0]->pat[j].data[k]) {
            ds.subsong[0]->pat[j].data[k]->copyOn(ds.subsong[i]->pat[j].getPattern(k,true));
            copiesMade++;
          }
        }
        ds.subsong[i]->pat[j].effectCols=ds.subsong[0]->pat[j].effectCols;
      }
      ds.subsong[i]->speeds=ds.subsong[0]->speeds;
      ds.subsong[i]->hz=ds.subsong[0]->hz;
      memcpy(ds.subsong[i]->chanShow,ds.subsong[0]->chanShow,DIV_MAX_CHANS*sizeof(bool));
      memcpy(ds.subsong[i]->chanShowChanOsc,ds.subsong[0]->chanShowChanOsc,DIV_MAX_CHANS*sizeof(bool));
    }

    logV("copies made %d",copiesMade);

    // set pattern lengths
    for (size_t i=0; i<ds.subsong.size(); i++) {
      unsigned short patLenMax=0;
      for (int j=0; j<ds.subsong[i]->ordersLen; j++) {
        unsigned short nextLen=patLen[ds.subsong[i]->orders.ord[0][j]];
        if (patLenMax<nextLen) {
          patLenMax=nextLen;
        }
      }
      ds.subsong[i]->patLen=patLenMax;
    }

    // set systems
    for (int i=0; i<(maxChan+32)>>5; i++) {
      ds.system[i]=DIV_SYSTEM_ES5506;
      ds.systemFlags[i].set("amigaVol",true);
      if (!ds.compatFlags.linearPitch) {
        ds.systemFlags[i].set("amigaPitch",true);
      }
    }
    ds.systemLen=(maxChan+32)>>5;
    ds.systemName="PC";

    // find subsongs
    ds.initDefaultSystemChans();
    ds.recalcChans();
    ds.findSubSongs();

    // populate subsongs with default panning values
    for (size_t i=0; i<ds.subsong.size(); i++) {
      for (int j=0; j<maxChan; j++) {
        DivPattern* p=ds.subsong[i]->pat[j].getPattern(ds.subsong[i]->orders.ord[j][0],true);
        for (int k=0; k<DIV_MAX_EFFECTS; k++) {
          if (p->newData[0][DIV_PAT_FX(k)]==0x80) {
            // give up if there's a panning effect already
            break;
          }
          if (p->newData[0][DIV_PAT_FX(k)]==-1) {
            if ((chanPan[j]&127)==100) {
              // should be surround...
              p->newData[0][DIV_PAT_FX(k)]=0x80;
              p->newData[0][DIV_PAT_FXVAL(k)]=0x80;
            } else {
              p->newData[0][DIV_PAT_FX(k)]=0x80;
              p->newData[0][DIV_PAT_FXVAL(k)]=CLAMP((chanPan[j]&127)<<2,0,255);
            }
            if (ds.subsong[i]->pat[j].effectCols<=k) ds.subsong[i]->pat[j].effectCols=k+1;
            break;
          }
        }
      }
    }

    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
    hasLoadedSomething=true;
    changeSong(0);
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

