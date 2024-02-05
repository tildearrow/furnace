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

bool DivEngine::loadS3M(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[4]={0,0,0,0};
  SafeReader reader=SafeReader(file,len);
  warnings="";

  unsigned char chanSettings[32];
  unsigned char ord[256];
  unsigned short insPtr[256];
  unsigned short patPtr[256];
  unsigned char chanPan[16];
  unsigned char defVol[256];

  try {
    DivSong ds;
    ds.version=DIV_VERSION_S3M;
    ds.linearPitch=0;
    ds.pitchMacroIsLinear=false;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;

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

    ds.name=reader.readString(28);
    
    reader.readC(); // 0x1a
    if (reader.readC()!=16) {
      logW("type is wrong!");
    }
    reader.readS(); // x

    unsigned short ordersLen=reader.readS();
    ds.insLen=reader.readS();

    if (ds.insLen<0 || ds.insLen>256) {
      logE("invalid instrument count!");
      lastError="invalid instrument count!";
      delete[] file;
      return false;
    }

    unsigned short patCount=reader.readS();

    unsigned short flags=reader.readS();
    unsigned short version=reader.readS();
    bool signedSamples=(reader.readS()==1);

    if ((flags&64) || version==0x1300) {
      ds.noSlidesOnFirstTick=false;
    }

    reader.readI(); // "SCRM"

    unsigned char globalVol=reader.readC();

    ds.subsong[0]->speeds.val[0]=(unsigned char)reader.readC();
    ds.subsong[0]->hz=((double)reader.readC())/2.5;

    unsigned char masterVol=reader.readC();

    logV("masterVol: %d",masterVol);
    logV("signedSamples: %d",signedSamples);
    logV("globalVol: %d",globalVol);

    reader.readC(); // UC
    bool defaultPan=(((unsigned char)reader.readC())==252);

    reader.readS(); // reserved
    reader.readI();
    reader.readI(); // the last 2 bytes is Special. we don't read that.

    reader.read(chanSettings,32);

    logD("reading orders...");
    for (int i=0; i<ordersLen; i++) {
      ord[i]=reader.readC();
      logV("- %.2x",ord[i]);
    }
    // should be even
    if (ordersLen&1) reader.readC();

    logD("reading ins pointers...");
    for (int i=0; i<ds.insLen; i++) {
      insPtr[i]=reader.readS();
      logV("- %.2x",insPtr[i]);
    }

    logD("reading pat pointers...");
    for (int i=0; i<patCount; i++) {
      patPtr[i]=reader.readS();
      logV("- %.2x",patPtr[i]);
    }

    if (defaultPan) {
      reader.read(chanPan,16);
    } else {
      memset(chanPan,0,16);
    }

    // determine chips to use
    ds.systemLen=0;

    bool hasPCM=false;
    bool hasFM=false;

    for (int i=0; i<32; i++) {
      if (!(chanSettings[i]&128)) continue;
      if ((chanSettings[i]&127)>=32) continue;
      if ((chanSettings[i]&127)>=16) {
        hasFM=true;
      } else {
        hasPCM=true;
      }

      if (hasFM && hasPCM) break;
    }

    ds.systemName="PC";
    if (hasPCM) {
      ds.system[ds.systemLen]=DIV_SYSTEM_ES5506;
      ds.systemVol[ds.systemLen]=1.0f;
      ds.systemPan[ds.systemLen]=0;
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
      DivInstrument* ins=new DivInstrument;
      if (!reader.seek(0x4c+insPtr[i]*16,SEEK_SET)) {
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
        ins->type=DIV_INS_ES5506;
        ds.ins.push_back(ins);
        continue;
      }

      if (!reader.seek(insPtr[i]*16,SEEK_SET)) {
        logE("premature end of file!");
        lastError="incomplete file";
        delete ins;
        delete[] file;
        return false;
      }

      String dosName=reader.readString(13);

      if (ins->type==DIV_INS_ES5506) {
        unsigned int memSeg=0;
        memSeg=(unsigned char)reader.readC();
        memSeg|=((unsigned short)reader.readS())<<8;

        logV("memSeg: %d",memSeg);

        unsigned int length=reader.readI();

        DivSample* s=new DivSample;
        s->depth=DIV_SAMPLE_DEPTH_8BIT;
        s->init(length);

        s->loopStart=reader.readI();
        s->loopEnd=reader.readI();
        defVol[i]=reader.readC();
        
        logV("defVol: %d",defVol[i]);

        reader.readC(); // x
      } else {
        
      }

      ds.ins.push_back(ins);
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

