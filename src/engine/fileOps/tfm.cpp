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

class TFMRLEReader;

struct TFMEndOfFileException {
  TFMRLEReader* reader;
  size_t finalSize;
  TFMEndOfFileException(TFMRLEReader* r, size_t fs):
    reader(r),
    finalSize(fs) {}
};


class TFMRLEReader {
  const unsigned char* buf;
  size_t len;
  size_t curSeek;
  bool inTag;
  int tagLenLeft;
  signed char tagChar;

  void decodeRLE(signed char prevChar) {
    int lenShift=0;
    tagLenLeft=0;
    signed char rleTag=0;
    do {
      rleTag=readC();
      tagLenLeft|=(rleTag&0x7F)<<lenShift;
      lenShift+=7;
      logD("RLE tag: %X, len shift: %d, len left: %d",rleTag,lenShift,tagLenLeft);
    } while (!(rleTag&0x80));
    // sync back since we've already read one character
    tagLenLeft--;
    inTag=true;
    tagChar=prevChar;
  }

public:
  TFMRLEReader(const void* b, size_t l) :
    buf((const unsigned char*)b),
    len(l),
    curSeek(0),
    inTag(false),
    tagLenLeft(0),
    tagChar(0) {}

  // these functions may throw TFMEndOfFileException
  signed char readC() {
    if (inTag) {
      if (!tagLenLeft) {
        inTag=false;
        return readC();
      }
      tagLenLeft--;
      logD("one char RLE decompressed, tag left: %d, char: %d",tagLenLeft,tagChar);
      return tagChar;
    }
    if (curSeek>len) throw TFMEndOfFileException(this,len);

    unsigned char ret=buf[curSeek++];

    // MISLEADING DOCUMENTATION: while TFM music maker's documentation says if the next byte
    // is zero, then it's not a tag but just 0x80 (for example: 0x00 0x80 0x00 = 0x00 0x80)
    // this is actually wrong
    // through research and experimentation, there are times that TFM music maker
    // will use 0x80 0x00 for actual tags (for example: 0x00 0x80 0x00 0x84 = 512 times 0x00
    // in certain parts of the header and footer)
    // TFM music maker actually uses double 0x80 to escape the 0x80
    // for example: 0xDA 0x80 0x80 0x00 0x23 = 0xDA 0x80 0x00 0x23)
    if (ret==0x80 && curSeek+1<=len) {
      if (buf[curSeek+1]!=0x80) {
        decodeRLE(buf[curSeek-2]);
        tagLenLeft--;
        return tagChar;
      } else {
        // to avoid outputting the extra 0x80
        curSeek++;
        return ret;
      }
    }
    return ret;
  }

  signed char readCNoRLE() {
    if (curSeek+1>len) throw TFMEndOfFileException(this,len);
    return buf[curSeek++];
  }

  void read(unsigned char* b, size_t l) {
    int i=0;
    while(l--) {
      unsigned char nextChar=readC();
      b[i++]=nextChar;
      logD("read next char: %x, index: %d",nextChar,i);
    }
  }

  void readNoRLE(unsigned char *b, size_t l) {
    int i=0;
    while (l--) {
      b[i++]=buf[curSeek++];
      if (curSeek>len) throw TFMEndOfFileException(this,len);
    }
  }

  short readS() {
    return readC()|readC()<<8;
  }

  short readSNoRLE() {
    if (curSeek+2>len) throw TFMEndOfFileException(this,len);
    short ret=buf[curSeek]|buf[curSeek+1]<<8;
    curSeek+=2;
    return ret;
  }

  void skip(size_t l) {
    // quick and dirty
    while (l--) {
      logD("skipping l %d",l);
      readC();
    }
  }

};

String TFMparseDate(short date) {
  return fmt::sprintf("%02d.%02d.%02d",date>>11,(date>>7)&0xF,date&0x7F);
}

bool DivEngine::loadTFM(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  TFMRLEReader reader=TFMRLEReader(file,len);

  try {
    DivSong ds;
    ds.systemName="Sega Genesis/Mega Drive or TurboSound FM";
    ds.subsong[0]->hz=50;
    ds.systemLen = 1;

    ds.system[0]=DIV_SYSTEM_YM2612;
    unsigned char magic[8]={0};

    reader.readNoRLE(magic, 8);
    if (memcmp(magic,DIV_TFM_MAGIC,8)!=0) throw InvalidHeaderException();

    unsigned char speedEven=reader.readCNoRLE();
    unsigned char speedOdd=reader.readCNoRLE();
    unsigned char interleaveFactor=reader.readCNoRLE();

    // TODO: due to limitations with the groove pattern, only interleave factors up to 8
    // are allowed in furnace
    if (interleaveFactor>8) {
      logW("interleave factor is bigger than 8, speed information may be inaccurate");
      interleaveFactor=8;
    }

    if (speedEven==speedOdd) {
      ds.subsong[0]->speeds.val[0]=speedEven;
      ds.subsong[0]->speeds.len=1;
    } else {
      for (int i=0; i<interleaveFactor; i++) {
        ds.subsong[0]->speeds.val[i]=speedEven;
        ds.subsong[0]->speeds.val[i+interleaveFactor]=speedOdd;
      }
      ds.subsong[0]->speeds.len=interleaveFactor*2;
    }
    ds.subsong[0]->ordersLen=reader.readCNoRLE();

    // order loop position, unused
    (void)reader.readCNoRLE();

    ds.createdDate=TFMparseDate(reader.readSNoRLE());
    ds.revisionDate=TFMparseDate(reader.readSNoRLE());

    // TODO: use this for something, number of saves
    (void)reader.readSNoRLE();

    unsigned char buffer[384];

    // author
    logD("parsing author");
    reader.read(buffer,64);
    ds.author=String((const char*)buffer,strnlen((const char*)buffer,64));
    memset(buffer, 0, 64);

    // name
    logD("parsing name");
    reader.read(buffer,64);
    ds.name=String((const char*)buffer,strnlen((const char*)buffer,64));
    memset(buffer, 0, 64);

    // notes
    logD("parsing notes");
    reader.read(buffer,384);
    String notes((const char*)buffer,strnlen((const char*)buffer,384));

    // fix \r\n to \n
    for (auto& c : notes) {
      if (c=='\r') {
        notes.erase(c,1);
      }
    }

    // order list
    logD("parsing order list");
    unsigned char orderList[256];
    reader.read(orderList,256);

    bool patExists[256];
    unsigned char maxPat=0;
    for (int i=0; i<ds.subsong[0]->ordersLen; i++) {
      patExists[orderList[i]]=true;
      if (maxPat<orderList[i]) maxPat=orderList[i];

      for (int j=0; j<6; j++) {
        ds.subsong[0]->orders.ord[j][i]=orderList[i];
        ds.subsong[0]->pat[j].data[orderList[i]]=new DivPattern;
      }
    }

    DivInstrument* insMaps[256];
    int insNumMaps[256];

    // instrument names
    logD("parsing instruments");
    unsigned char insName[16];
    int insCount=0;
    for (int i=0; i<255; i++) {
      reader.read(insName,16);

      if (memcmp(insName,"\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",16)==0) {
        logD("instrument unused");
        insNumMaps[i]=i;
        insMaps[i]=NULL;
        continue;
      }

      DivInstrument* ins=new DivInstrument;
      ins->type=DIV_INS_FM;
      ins->name=String((const char*)insName,strnlen((const char*)insName,16));
      ds.ins.push_back(ins);

      insNumMaps[i]=insCount;
      insCount++;

      insMaps[i]=ins;
    }

    ds.insLen=insCount;

    // instrument data
    for (int i=0; i<255; i++) {
      if (!insMaps[i]) {
        reader.skip(42);
        continue;
      }

      insMaps[i]->fm.alg=reader.readC();
      insMaps[i]->fm.fb=reader.readC();

      for (int j=0; j<4; j++) {
        insMaps[i]->fm.op[j].mult=reader.readC();
        insMaps[i]->fm.op[j].dt=reader.readC();
        insMaps[i]->fm.op[j].tl=reader.readC()^0x7F;
        insMaps[i]->fm.op[j].rs=reader.readC();
        insMaps[i]->fm.op[j].ar=reader.readC()^0x1F;
        insMaps[i]->fm.op[j].dr=reader.readC()^0x1F;
        insMaps[i]->fm.op[j].d2r=reader.readC()^0x1F;
        insMaps[i]->fm.op[j].rr=reader.readC()^0xF;
        insMaps[i]->fm.op[j].sl=reader.readC();
        insMaps[i]->fm.op[j].ssgEnv=reader.readC();
      }
    }

    ds.notes=notes;

    unsigned char patLens[256];
    int maxPatLen=0;
    reader.read(patLens, 256);
    for (int i=0;i<256;i++) {
      if (patLens[i]==0) {
        maxPatLen=256;
        break;
      } else if (patLens[i]>maxPatLen) {
        maxPatLen=patLens[i];
      }
    }

    ds.subsong[0]->patLen=maxPatLen;

    // PATTERN DATA FORMAT (not described properly in the documentation)
    // for each channel in a pattern:
    //  - note data (256 bytes)
    //  - volume data (256 bytes, values always 0x00-0x1F)
    //  - instrument number data (256 bytes)
    //  - effect number (256 bytes, values 0x0-0x23 (to represent 0-F and G-Z))
    //  - effect value (256 bytes)
    //  - padding(?) (1536 bytes, always set to 0)
    // notes are stored as an inverted value of note+octave*12
    // key-offs are stored in the note data as 0x01
    unsigned char patDataBuf[256];

    for (int i=0; i<256; i++) {
      if (i>maxPat) break;
      else if (!patExists[i]) {
        logD("skipping pattern %d", i);
        reader.skip(16896);
        continue;
      }

      logD("parsing pattern %d", i);
      for (int j=0; j<6; j++) {
        DivPattern* pat = ds.subsong[0]->pat[j].data[i];

        // notes
        reader.read(patDataBuf, 256);

        logD("parsing notes of pattern %d channel %d",i,j);
        for (int k=0; k<256; k++) {
          if (patDataBuf[k]==0) continue;
          else if (patDataBuf[k]==1) {
            // note off
            pat->data[k][0]=100;
          } else {
            unsigned char invertedNote=~patDataBuf[k];
            pat->data[k][0]=invertedNote%12;
            pat->data[k][1]=(invertedNote/12)-1;

            if (pat->data[k][0]==0) {
              pat->data[k][0]=12;
              pat->data[k][1]--;
            }
          }
        }

        // put a "jump to next pattern" effect if the pattern is smaller than the maximum pattern lengths
        if (patLens[i]!=0 && patLens[i]<ds.subsong[0]->patLen) {
          if (pat->data[patLens[i]-1][4]==-1 && pat->data[patLens[i]-1][5]==-1) {
            pat->data[patLens[i]-1][4]=0x0D;
            pat->data[patLens[i]-1][5]=0x00;
          }
          pat->data[patLens[i]][4]=0x0D;
          pat->data[patLens[i]][5]=0x00;
        }
        // volume
        reader.read(patDataBuf,256);

        logD("parsing volumes of pattern %d channel %d",i,j);
        for (int k=0; k<256; k++) {
          if (patDataBuf[k]==0) continue;
          else pat->data[k][3]=0x41+(patDataBuf[k]*2);
        }

        // instrument
        reader.read(patDataBuf,256);

        logD("parsing instruments of pattern %d channel %d",i,j);
        for (int k=0; k<256; k++) {
          if (patDataBuf[k]==0) continue;
          pat->data[k][2]=insNumMaps[patDataBuf[k]-1];
        }

        // effects

        unsigned char effectNum[256];
        unsigned char effectVal[256];
        reader.read(effectNum,256);
        reader.read(effectVal,256);

        for (int k=0; k<256; k++) {
          switch (effectNum[k]) {
          case 0:
            // arpeggio or no effect (if effect val is 0)
            if (effectVal[k]==0) break;
            pat->data[k][4]=effectNum[k];
            pat->data[k][5]=effectVal[k];
            break;
          case 1:
            // pitch slide up
          case 2:
            // pitch slide down
          case 3:
            // portamento
          case 4:
            // vibrato
            pat->data[k][4]=effectNum[k];
            pat->data[k][5]=effectVal[k];
            break;
          case 5:
            // poramento + volume slide
            pat->data[k][4]=0x06;
            pat->data[k][5]=effectVal[k];
            break;
          case 6:
            // vibrato + volume slide
            pat->data[k][4]=0x05;
            pat->data[k][5]=effectVal[k];
            break;
          default:
            break;
          }
        }

        reader.skip(1536);
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
    success=true;
  } catch(TFMEndOfFileException& e) {
    lastError="incomplete file!";
  } catch(InvalidHeaderException& e) {
    lastError="invalid info header!";
  }

  return success;
}
