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
  
  SafeReader reader=SafeReader(file,len);
  warnings="";

  try {
    DivSong ds;
    ds.version=DIV_VERSION_XM;
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

    logV("orders len: %d",ordersLen);
    logV("pattern count: %d",patCount);
    logV("used tracker: %x",usedTracker);
    logV("compatible with: %x",compatTracker);
    logV("flags: %x",flags);
    logV("special: %x",special);

    logV("global vol: %d",globalVol);
    logV("master vol: %d",masterVol);
    logV("speed: %d",initSpeed);
    logV("tempo: %d",initTempo);
    logV("pan sep: %d",panSep);

    unsigned short commentLen=reader.readS();
    unsigned int commentPtr=reader.readI();

    logV("comment len: %d",commentLen);
    logV("comment ptr: %x",commentPtr);

    reader.readI(); // reserved

    reader.read(chanPan,64);
    reader.read(chanVol,64);

    reader.read(orders,ordersLen);

    for (int i=0; i<ordersLen; i++) {
      for (int j=0; j<DIV_MAX_CHANS; j++) {
        ds.subsong[0]->orders.ord[j][i]=orders[i];
      }
    }
    ds.subsong[0]->ordersLen=ordersLen;

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
            short r;
            if (convert&2) {
              l=reader.readS_BE();
              r=reader.readS_BE();
            } else {
              l=reader.readS();
              r=reader.readS();
            }
            if (!(convert&1)) {
              l^=0x8000;
              r^=0x8000;
            }
            s->data16[i]=(l+r)>>1;
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
            signed char r=reader.readC();
            if (!(convert&1)) {
              l^=0x80;
              r^=0x80;
            }
            s->data8[i]=(l+r)>>1;
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
      unsigned char mask[64];
      unsigned char note[64];
      unsigned char ins[64];
      unsigned char vol[64];
      unsigned char effect[64];
      unsigned char effectVal[64];
      int curRow=0;

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
          curRow++;
          if (curRow>=patRows) break;
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
          if (note[chan]==255) { // note off
            p->data[curRow][0]=100;
            p->data[curRow][1]=0;
          } else if (note[chan]==254) { // note release
            p->data[curRow][0]=101;
            p->data[curRow][1]=0;
          } else if (note[chan]<120) {
            p->data[curRow][0]=note[chan]%12;
            p->data[curRow][1]=note[chan]/12;
            if (p->data[curRow][0]==0) {
              p->data[curRow][0]=12;
              p->data[curRow][1]--;
            }
          }
        }
        if (hasIns) {
          p->data[curRow][2]=ins[chan]-1;
        }
        if (hasVol) {
          p->data[curRow][3]=vol[chan];
        }
        if (hasEffect) {
          //p->data[curRow][4]=effect[chan];
          //p->data[curRow][5]=effectVal[chan];
        }
      }
    }

    for (int i=0; i<(maxChan+32)>>5; i++) {
      ds.system[i]=DIV_SYSTEM_ES5506;
      ds.systemFlags[i].set("amigaVol",true);
      if (ds.linearPitch!=2) {
        ds.systemFlags[i].set("amigaPitch",true);
      }
    }
    ds.systemLen=(maxChan+32)>>5;

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

