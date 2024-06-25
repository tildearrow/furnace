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

  bool doesPitchSlide[64];
  bool doesVibrato[64];
  bool doesPanning[64];
  bool doesVolSlide[64];
  bool doesArp[64];

  memset(doesPitchSlide,0,64*sizeof(bool));
  memset(doesVibrato,0,64*sizeof(bool));
  memset(doesPanning,0,64*sizeof(bool));
  memset(doesVolSlide,0,64*sizeof(bool));
  memset(doesArp,0,64*sizeof(bool));
  
  SafeReader reader=SafeReader(file,len);
  warnings="";

  memset(chanPan,0,64);
  memset(chanVol,0,64);
  memset(orders,0,256);
  memset(patLen,0,256*sizeof(unsigned short));

  try {
    DivSong ds;
    ds.version=DIV_VERSION_IT;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;
    ds.pitchSlideSpeed=4;

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

    ds.name=reader.readString(26);

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
      ds.linearPitch=2;
    } else {
      ds.linearPitch=0;
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

    for (int i=0; i<ds.insLen; i++) {
      insPtr[i]=reader.readI();
    }

    for (int i=0; i<ds.sampleLen; i++) {
      samplePtr[i]=reader.readI();
    }

    for (int i=0; i<patCount; i++) {
      patPtr[i]=reader.readI();
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

      String dosName=reader.readString(12);

      reader.readC(); // 0

      unsigned char initCut=255;
      unsigned char initRes=255;

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

        unsigned short fadeOut=reader.readS();

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

        unsigned short fadeOut=reader.readS();

        logV("fadeOut: %d",fadeOut);

        reader.readC();
        reader.readC();

        unsigned char globalVol=reader.readC();
        unsigned char defPan=reader.readC();

        logV("globalVol: %d",globalVol);
        logV("defPan: %d",defPan);

        // vol/pan randomization
        reader.readC();
        reader.readC();

        // version/sample count/x
        reader.readI();
      }

      ins->name=reader.readString(26);

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
      }

      // TODO: envelopes...

      ds.ins.push_back(ins);
    }

    // read samples
    for (int i=0; i<ds.sampleLen; i++) {
      DivSample* s=new DivSample;

      if (samplePtr[i]==0) {
        ds.sample.push_back(s);
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
        logE("invalid sample header!");
        lastError="invalid sample header";
        delete s;
        delete[] file;
        return false;
      }

      String dosName=reader.readString(12);

      reader.readC(); // 0

      unsigned char globalVol=reader.readC();
      unsigned char flags=reader.readC();
      unsigned char sampleVol=reader.readC();

      logV("volumes: %d %d",globalVol,sampleVol);

      s->name=reader.readString(26);

      unsigned char convert=reader.readC();
      unsigned char defPan=reader.readC();

      logV("defPan: %d",defPan);

      if (flags&2) {
        s->depth=DIV_SAMPLE_DEPTH_16BIT;
      } else {
        s->depth=DIV_SAMPLE_DEPTH_8BIT;
      }

      if (flags&8) {
        logE("sample decompression not implemented!");
        lastError="sample decompression not implemented";
        delete s;
        delete[] file;
        return false;
      }

      s->init((unsigned int)reader.readI());
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

      ds.sample.push_back(s);
    }

    // read patterns
    int maxChan=0;
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
      bool did[64];

      if (patPtr[i]==0) continue;

      unsigned char mask[64];
      unsigned char note[64];
      unsigned char ins[64];
      unsigned char vol[64];
      unsigned char effect[64];
      unsigned char effectVal[64];
      int curRow=0;
      bool mustCommitInitial=true;

      memset(effectCol,4,64);
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
      memset(did,0,64*sizeof(bool));

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

            if ((effectCol[j]>>1)-2>ds.subsong[0]->pat[j].effectCols) {
              ds.subsong[0]->pat[j].effectCols=(effectCol[j]>>1)-1;
            }
          }

          curRow++;
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
          memset(did,0,64);
          mustCommitInitial=false;
          if (curRow>=patRows) {
            if (curRow>0) {
              // place end of pattern marker
              DivPattern* p=ds.subsong[0]->pat[0].getPattern(i,true);
              p->data[curRow-1][effectCol[0]++]=0x0d;
              p->data[curRow-1][effectCol[0]++]=0;

              if ((effectCol[0]>>1)-2>ds.subsong[0]->pat[0].effectCols) {
                ds.subsong[0]->pat[0].effectCols=(effectCol[0]>>1)-1;
              }
            }
            break;
          }
          continue;
        }

        if (chan&128) {
          mask[chan&63]=reader.readC();
        }
        chan&=63;

        if (chan>maxChan) maxChan=chan;

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
            p->data[curRow][0]=101;
            p->data[curRow][1]=0;
          } else if (note[chan]==254) { // note off
            p->data[curRow][0]=100;
            p->data[curRow][1]=0;
          } else if (note[chan]<120) {
            p->data[curRow][0]=note[chan]%12;
            p->data[curRow][1]=note[chan]/12;
            if (p->data[curRow][0]==0) {
              p->data[curRow][0]=12;
              p->data[curRow][1]--;
            }
          } else { // note fade, but Furnace does not have that
            p->data[curRow][0]=102;
            p->data[curRow][1]=0;
          }
        }
        if (hasIns) {
          p->data[curRow][2]=ins[chan]-1;
        }
        if (hasVol) {
          p->data[curRow][3]=vol[chan];
        }
        if (hasEffect) {
          switch (effect[chan]+'A'-1) {
            case 'A': // speed
              p->data[curRow][effectCol[chan]++]=0x0f;
              p->data[curRow][effectCol[chan]++]=effectVal[chan];
              break;
            case 'B': // go to order
              p->data[curRow][effectCol[chan]++]=0x0b;
              p->data[curRow][effectCol[chan]++]=orders[effectVal[chan]];
              break;
            case 'C': // next order
              p->data[curRow][effectCol[chan]++]=0x0d;
              p->data[curRow][effectCol[chan]++]=effectVal[chan];
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
              portaType[chan]=2;
              porting[chan]=true;
              break;
            case 'F': // pitch up
              if (effectVal[chan]!=0) {
                portaStatus[chan]=effectVal[chan];
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
            case 'M': // channel vol (extension)
              break;
            case 'N': // channel vol slide (extension)
              break;
            case 'O': // offset
              p->data[curRow][effectCol[chan]++]=0x91;
              p->data[curRow][effectCol[chan]++]=effectVal[chan];
              break;
            case 'P': // pan slide (extension)
              break;
            case 'Q': // retrigger
              p->data[curRow][effectCol[chan]++]=0x0c;
              p->data[curRow][effectCol[chan]++]=effectVal[chan]&15;
              break;
            case 'R': // tremolo
              break;
            case 'S': // special...
              switch (effectVal[chan]>>4) {
                case 0xc:
                  p->data[curRow][effectCol[chan]++]=0xec;
                  p->data[curRow][effectCol[chan]++]=effectVal[chan]&15;
                  break;
                case 0xd:
                  p->data[curRow][effectCol[chan]++]=0xed;
                  p->data[curRow][effectCol[chan]++]=effectVal[chan]&15;
                  break;
              }
              break;
            case 'T': // tempo
              p->data[curRow][effectCol[chan]++]=0xf0;
              p->data[curRow][effectCol[chan]++]=effectVal[chan];
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
            case 'X': // panning (extension)
              p->data[curRow][effectCol[chan]++]=0x80;
              p->data[curRow][effectCol[chan]++]=effectVal[chan];
              break;
            case 'Y': // panbrello (extension)
              break;
            case 'Z': // MIDI macro (extension)
              break;
          }
        }
      }
    }

    logV("maxChan: %d",maxChan);

    // set channel visibility
    for (int i=maxChan+1; i<((maxChan+32)&(~31)); i++) {
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
      if (ds.linearPitch!=2) {
        ds.systemFlags[i].set("amigaPitch",true);
      }
    }
    ds.systemLen=(maxChan+32)>>5;

    // find subsongs
    ds.findSubSongs();    

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

