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

bool DivEngine::loadXM(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  char magic[32];
  SafeReader reader=SafeReader(file,len);
  warnings="";

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

    ds.name=reader.readString(20);

    // 0x1a
    reader.readC();

    String trackerName=reader.readString(20);
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

    logV("channels: %d",totalChans);

    logV("repeat pos: %d",loopPos);

    logV("reading orders...");
    for (int i=0; i<256; i++) {
      unsigned char val=reader.readC();
      for (int j=0; j<DIV_MAX_CHANS; j++) {
        ds.subsong[0]->orders.ord[j][i]=val;
      }
    }

    logV("seeking to %x...",headerSeek);

    if (!reader.seek(headerSeek,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }

    // read patterns
    logD("reading patterns...");
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
            if (note>=96) {
              p->data[j][0]=100;
              p->data[j][1]=0;
            } else {
              p->data[j][0]=note%12;
              p->data[j][1]=note/12;
              if (p->data[j][0]==0) {
                p->data[j][0]=12;
                p->data[j][1]=(unsigned char)(p->data[j][1]-1);
              }
            }
          }
          if (hasIns) {
            ins=reader.readC();
            p->data[j][2]=((int)ins)-1;
          }
          if (hasVol) {
            vol=reader.readC();
            p->data[j][3]=vol;
          }
          if (hasEffect) {
            effect=reader.readC();
            p->data[j][4]=effect;
          }
          if (hasEffectVal) {
            effectVal=reader.readC();
            p->data[j][5]=effectVal;
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
      DivInstrument* ins=new DivInstrument;
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

