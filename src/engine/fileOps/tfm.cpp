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

  void decodeRLE(unsigned char prevChar) {
    int lenShift=0;
    tagLenLeft=0;
    unsigned char rleTag=0;

    do {
      rleTag=readCNoRLE();
      tagLenLeft|=(rleTag&0x7F)<<lenShift;
      lenShift+=7;
      logD("offset: %x, RLE tag: %X, len shift: %d, len left: %d",curSeek,rleTag,lenShift,tagLenLeft);
    } while (!(rleTag&0x80));

    if (tagLenLeft) {
      // sync back since we've already read one character
      inTag=true;
      tagLenLeft--;
      tagChar=prevChar;
    } else {
      tagChar=0x80;
    }
    logD("tag finished: len left: %d, char: %X",tagLenLeft,tagChar);
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
  unsigned char readC() {
    if (inTag) {
      if (tagLenLeft<=0) {
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
    if (ret==0x80) {
      decodeRLE(buf[curSeek-2]);
      tagLenLeft--;
      return tagChar;
    }
    return ret;
  }

  signed char readCNoRLE() {
    if (curSeek>len) throw TFMEndOfFileException(this,len);
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

  String readString(size_t l) {
    String ret;
    ret.reserve(l);
    while (l--) {
      unsigned char byte=readC();
      if (!byte) {
        skip(l);
        break;
      }
      ret += byte;
    }
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

struct TFMSpeed {
  unsigned char speedEven;
  unsigned char speedOdd;
  unsigned char interleaveFactor;

  bool operator==(const TFMSpeed &s) const {
    return speedEven==s.speedEven && speedOdd==s.speedOdd && interleaveFactor==s.interleaveFactor;
  }
};

// to make it work with map
namespace std {
  template<> struct hash<TFMSpeed>
  {
    size_t operator()(const TFMSpeed& s) const noexcept {
      return s.speedEven<<16|s.speedOdd<<8|s.interleaveFactor;
    }
  };
}

struct TFMParsePatternInfo {
  TFMRLEReader* reader;
  unsigned char maxPat;
  unsigned char* patLens;
  unsigned char* orderList;
  unsigned char speedEven;
  unsigned char speedOdd;
  unsigned char interleaveFactor;
  bool* patExists;
  DivSong* ds;
  int* insNumMaps;
  bool v2;
  unsigned char loop_pos;
};

void TFMParsePattern(struct TFMParsePatternInfo info) {
  // PATTERN DATA FORMAT (not described properly in the documentation)
  // for each channel in a pattern:
  //  - note data (256 bytes)
  //  - volume data (256 bytes, values always 0x00-0x1F)
  //  - instrument number data (256 bytes)
  //  - effect number (256 bytes, values 0x0-0x23 (to represent 0-F and G-Z))
  //  - effect value (256 bytes)
  //  - extra 3 effects (1536 bytes 256x3x2) (ONLY ON V2)
  // notes are stored as an inverted value of note+octave*12
  // key-offs are stored in the note data as 0x01
  unsigned char patDataBuf[256];
  unsigned short lastSlide=0;
  unsigned short lastVibrato=0;

  struct TFMSpeed speed;
  DivGroovePattern groove;
  speed.speedEven=info.speedEven;
  speed.speedOdd=info.speedOdd;
  speed.interleaveFactor=info.interleaveFactor;
  int speedGrooveIndex=1;

  int usedEffectsCol=0;
  std::unordered_map<TFMSpeed, int> speeds({{speed, 0}});

  // initialize the global groove pattern first
  if (speed.interleaveFactor>8) {
    logW("speed interleave factor is bigger than 8, speed information may be inaccurate");
    speed.interleaveFactor=8;
  }
  for (int i=0; i<speed.interleaveFactor; i++) {
    groove.val[i]=speed.speedEven;
    groove.val[i+speed.interleaveFactor]=speed.speedOdd;
  }
  groove.len=speed.interleaveFactor*2;

  info.ds->grooves.push_back(groove);

  for (int i=0; i<256; i++) {
    if (i>info.maxPat) break;
    else if (!info.patExists[i]) {
      logD("skipping pattern %d",i);
      info.reader->skip((info.v2) ? 16896 : 7680);
      continue;
    }

    logD("parsing pattern %d",i);
    for (int j=0; j<6; j++) {
      DivPattern* pat = info.ds->subsong[0]->pat[j].data[i];

      // notes
      info.reader->read(patDataBuf,256);

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

      // volume
      info.reader->read(patDataBuf,256);

      logD("parsing volumes of pattern %d channel %d",i,j);
      for (int k=0; k<256; k++) {
        if (patDataBuf[k]==0) continue;
        else pat->data[k][3]=0x60+patDataBuf[k];
      }

      // instrument
      info.reader->read(patDataBuf,256);

      logD("parsing instruments of pattern %d channel %d",i,j);
      for (int k=0; k<256; k++) {
        if (patDataBuf[k]==0) continue;
        pat->data[k][2]=info.insNumMaps[patDataBuf[k]-1];
      }

      // effects

      int numEffectsCol=(info.v2) ? 4 : 1;
      for (int l=0; l<numEffectsCol; l++) {
        unsigned char effectNum[256];
        unsigned char effectVal[256];
        info.reader->read(effectNum,256);
        info.reader->read(effectVal,256);

        for (int k=0; k<256; k++) {
          if (effectNum[k] || effectVal[k]) usedEffectsCol=l+1;
          switch (effectNum[k]) {
          case 0:
            // arpeggio or no effect (if effect val is 0)
            if (effectVal[k]==0) break;
            pat->data[k][4+(l*2)]=effectNum[k];
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 1:
            // pitch slide up
          case 2:
            // pitch slide down
            pat->data[k][4+(l*2)]=effectNum[k];
            if (effectVal[k]) {
              lastSlide=effectVal[k];
              pat->data[k][5+(l*2)]=effectVal[k];
            } else {
              pat->data[k][5+(l*2)]=lastSlide;
            }
            break;
          case 3:
            // portamento
          case 4:
            // vibrato
            pat->data[k][5+(l*2)]=0;
            if (effectVal[k]&0xF0) {
              pat->data[k][5+(l*2)]|=effectVal[k]&0xF0;
            } else {
              pat->data[k][5+(l*2)]|=lastVibrato&0xF0;
            }
            if (effectVal[k]&0x0F) {
              pat->data[k][5+(l*2)]|=effectVal[k]&0x0F;
            } else {
              pat->data[k][5+(l*2)]|=lastVibrato&0x0F;
            }
            pat->data[k][4+(l*2)]=effectNum[k];
            lastVibrato=pat->data[k][5+(l*2)];
            break;
          case 5:
            // poramento + volume slide
            pat->data[k][4+(l*2)]=0x06;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 6:
            // vibrato + volume slide
            pat->data[k][4+(l*2)]=0x05;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 8:
            // modify TL of operator 1
            pat->data[k][4+(l*2)]=0x12;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 9:
            // modify TL of operator 2
            pat->data[k][4+(l*2)]=0x13;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 10:
            // volume slide
            pat->data[k][4+(l*2)]=0xA;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 11:
            // multi-frequency mode of CH3 control
            // TODO
          case 12:
            // modify TL of operator 3
            pat->data[k][4+(l*2)]=0x14;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 13:
            // modify TL of operator 4
            pat->data[k][4+(l*2)]=0x15;
            pat->data[k][5+(l*2)]=effectVal[k];
            break;
          case 14:
            switch (effectVal[k]>>4) {
            case 0:
            case 1:
            case 2:
            case 3:
              // modify multiplier of operators
              pat->data[k][4+(l*2)]=0x16;
              pat->data[k][5+(l*2)]=((effectVal[k]&0xF0)+0x100)|(effectVal[k]&0xF);
              break;
            case 8:
              // pan
              pat->data[k][4+(l*2)]=0x80;
              if ((effectVal[k]&0xF)==1) {
                pat->data[k][5+(l*2)]=0;
              } else if ((effectVal[k]&0xF)==2) {
                pat->data[k][5+(l*2)]=0xFF;
              } else {
                pat->data[k][5+(l*2)]=0x80;
              }
              break;
            }
            break;
          case 15:
            // speed

            if (effectVal[k]==0) {
              // if speed is set to zero (reset to global values)
              speed.speedEven=info.speedEven;
              speed.speedOdd=info.speedOdd;
              speed.interleaveFactor=info.interleaveFactor;
            } else if (effectVal[k]>>4==0) {
              // if the top nibble is set to zero (set interleave factor)
              speed.interleaveFactor=effectVal[k]&0xF;
            } else if ((effectVal[k]>>4)==(effectVal[k]&0xF)) {
              // if both speeds are equal
              pat->data[k][4+(l*2)]=0x0F;
              unsigned char speedSet=effectVal[k]>>4;
              pat->data[k][5+(l*2)]=speedSet;
              break;
            } else {
              speed.speedEven=effectVal[k]>>4;
              speed.speedOdd=effectVal[k]&0xF;
            }

            auto speedIndex = speeds.find(speed);
            if (speedIndex != speeds.end()) {
              pat->data[k][4+(l*2)]=0x09;
              pat->data[k][5+(l*2)]=speedIndex->second;
              break;
            }
            if (speed.interleaveFactor>8) {
              logW("speed interleave factor is bigger than 8, speed information may be inaccurate");
              speed.interleaveFactor=8;
            }
            for (int i=0; i<speed.interleaveFactor; i++) {
              groove.val[i]=speed.speedEven;
              if (i+speed.interleaveFactor<16) {
                groove.val[i+speed.interleaveFactor]=speed.speedOdd;
              }
            }
            groove.len=speed.interleaveFactor*2;

            info.ds->grooves.push_back(groove);
            speeds[speed]=speedGrooveIndex;

            pat->data[k][4+(l*2)]=0x09;
            pat->data[k][5+(l*2)]=speedGrooveIndex;
            speedGrooveIndex++;
            break;
          }
        }
        info.ds->subsong[0]->pat[j].effectCols=(usedEffectsCol*2)+1;

        // put a "jump to next pattern" effect if the pattern is smaller than the maximum pattern length
        if (info.patLens[i]!=0 && info.patLens[i]<info.ds->subsong[0]->patLen) {
          pat->data[info.patLens[i]-1][4+(usedEffectsCol*4)]=0x0D;
          pat->data[info.patLens[i]-1][5+(usedEffectsCol*4)]=0x00;
        }
      }
    }
  }

  // 2nd pass: fixing pitch slides, arpeggios, etc. so the result doesn't sound weird.

  bool chArpeggio[6]={false};
  bool chVibrato[6]={false};
  bool chPorta[6]={false};
  bool chVolumeSlide[6]={false};
  int lastPatSeen=0;

  for (int i=0; i<info.ds->subsong[0]->ordersLen; i++) {
    // this is if the last pattern is used more than once
    if (info.orderList[i] == info.orderList[info.ds->subsong[0]->ordersLen - 1]) {
      lastPatSeen++;
    }
    for (int j=0; j<6; j++) {
      for (int l=0; l<usedEffectsCol; l++) {
        DivPattern* pat = info.ds->subsong[0]->pat[j].data[info.orderList[i]];
        unsigned char truePatLen=(info.patLens[info.orderList[i]]<info.ds->subsong[0]->patLen) ? info.patLens[info.orderList[i]] : info.ds->subsong[0]->patLen;

        // default instrument
        if (i==0 && pat->data[0][2]==-1) pat->data[0][2]=0;

        for (int k=0; k<truePatLen; k++) {
          if (chArpeggio[j] && pat->data[k][4+(l*2)]!=0x00 && pat->data[k][0]!=-1) {
            pat->data[k][4+usedEffectsCol*2+(l*2)]=0x00;
            pat->data[k][5+usedEffectsCol*2+(l*2)]=0;
            chArpeggio[j]=false;
          } else if (chPorta[j] && pat->data[k][4+(l*2)]!=0x03 && pat->data[k][4+(l*2)]!=0x01 && pat->data[k][4+(l*2)]!=0x02) {
            pat->data[k][4+usedEffectsCol*2+(l*2)]=0x03;
            pat->data[k][5+usedEffectsCol*2+(l*2)]=0;
            chPorta[j]=false;
          } else if (chVibrato[j] && pat->data[k][4+(l*2)]!=0x04 && pat->data[k][0]!=-1) {
            pat->data[k][4+usedEffectsCol*2+(l*2)]=0x04;
            pat->data[k][5+usedEffectsCol*2+(l*2)]=0;
            chVibrato[j]=false;
          } else if (chVolumeSlide[j] && pat->data[k][4+(l*2)]!=0x0A) {
            pat->data[k][4+usedEffectsCol*2+(l*2)]=0x0A;
            pat->data[k][5+usedEffectsCol*2+(l*2)]=0;
            chVolumeSlide[j]=false;
          }

          switch (pat->data[k][4+l]) {
          case 0:
            chArpeggio[j]=true;
            break;
          case 1:
          case 2:
          case 3:
            chPorta[j]=true;
            break;
          case 4:
            chVibrato[j]=true;
            break;
          case 0xA:
            chVolumeSlide[j]=true;
            break;
          default:
    break;
          }
        }
      }
    }
  }

  if (lastPatSeen>1) {
    // clone the last pattern 
    info.maxPat++;
    for (int i=0;i<6;i++) {
      int last_pat_num = info.ds->subsong[0]->orders.ord[i][info.ds->subsong[0]->ordersLen - 1];
      DivPattern* new_pat = new DivPattern;
      DivPattern* last_pat = info.ds->subsong[0]->pat[i].data[last_pat_num];
      last_pat->copyOn(new_pat);

      info.ds->subsong[0]->orders.ord[i][info.ds->subsong[0]->ordersLen - 1] = info.maxPat;
      new_pat->data[info.patLens[last_pat_num]-1][4+(usedEffectsCol*4)] = 0x0B;
      new_pat->data[info.patLens[last_pat_num]-1][5+(usedEffectsCol*4)] = info.loop_pos;
      info.ds->subsong[0]->pat[i].data[info.maxPat] = new_pat;
    }
  } else {
    for (int i=0;i<6;i++) {
      int last_pat_num = info.ds->subsong[0]->orders.ord[i][info.ds->subsong[0]->ordersLen - 1];
      DivPattern* last_pat = info.ds->subsong[0]->pat[i].data[last_pat_num];
      last_pat->data[info.patLens[last_pat_num]-1][4+(usedEffectsCol*4)] = 0x0B;
      last_pat->data[info.patLens[last_pat_num]-1][5+(usedEffectsCol*4)] = info.loop_pos;
    }
  }
}

bool DivEngine::loadTFMv1(unsigned char* file, size_t len) {
  // the documentation for this version is in russian only
  struct InvalidHeaderException {};
  bool success=false;
  TFMRLEReader reader=TFMRLEReader(file,len);

  try {
    DivSong ds;
    ds.version=DIV_VERSION_TFE;
    ds.systemName="Sega Genesis/Mega Drive or TurboSound FM";
    ds.subsong[0]->hz=50;
    ds.systemLen=1;

    ds.system[0]=DIV_SYSTEM_YM2612;
    ds.loopModality=1;

    unsigned char speed=reader.readCNoRLE();
    unsigned char interleaveFactor=reader.readCNoRLE();

    // TODO: due to limitations with the groove pattern, only interleave factors up to 8
    // are allowed in furnace
    if (interleaveFactor>8) {
      logW("interleave factor is bigger than 8, speed information may be inaccurate");
      interleaveFactor=8;
    }
    if ((speed>>4)==(speed&0xF)) {
      ds.subsong[0]->speeds.val[0]=speed&0xF;
      ds.subsong[0]->speeds.len=1;
    } else {
      for (int i=0; i<interleaveFactor; i++) {
        ds.subsong[0]->speeds.val[i]=speed>>4;
        ds.subsong[0]->speeds.val[i+interleaveFactor]=speed&0xF;
      }
      ds.subsong[0]->speeds.len=interleaveFactor*2;
    }
    ds.subsong[0]->ordersLen=reader.readCNoRLE();

    // order loop position, unused
    unsigned char loop_pos = reader.readCNoRLE();

    ds.createdDate=TFMparseDate(reader.readSNoRLE());
    ds.revisionDate=TFMparseDate(reader.readSNoRLE());

    // TODO: use this for something, number of saves
    (void)reader.readSNoRLE();

    // author
    logD("parsing author");
    ds.author=reader.readString(64);

    // name
    logD("parsing name");
    ds.name=reader.readString(64);

    // notes
    logD("parsing notes");
    String notes=reader.readString(384);

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
        insMaps[i]->fm.op[j].ar=reader.readC();
        insMaps[i]->fm.op[j].dr=reader.readC();
        insMaps[i]->fm.op[j].d2r=reader.readC();
        insMaps[i]->fm.op[j].rr=reader.readC();
        insMaps[i]->fm.op[j].sl=reader.readC();
        insMaps[i]->fm.op[j].ssgEnv=reader.readC();
      }
    }

    ds.notes=notes;

    unsigned char patLens[256];
    int maxPatLen=0;
    reader.read(patLens, 256);
    for (int i=0; i<256; i++) {
      if (patLens[i]==0) {
        maxPatLen=256;
        break;
      } else if (patLens[i]>maxPatLen) {
        maxPatLen=patLens[i];
      }
    }

    ds.subsong[0]->patLen=maxPatLen;

    struct TFMParsePatternInfo info;
    info.ds=&ds;
    info.insNumMaps=insNumMaps;
    info.maxPat=maxPat;
    info.patExists=patExists;
    info.orderList=orderList;
    info.speedEven=speed>>4;
    info.speedOdd=speed&0xF;
    info.interleaveFactor=interleaveFactor;
    info.patLens=patLens;
    info.reader=&reader;
    info.v2=false;
    info.loop_pos=loop_pos;
    TFMParsePattern(info);

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

  delete[] file;
  return success;
}

bool DivEngine::loadTFMv2(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  TFMRLEReader reader=TFMRLEReader(file,len);

  try {
    DivSong ds;
    ds.version=DIV_VERSION_TFE;
    ds.systemName="Sega Genesis/Mega Drive or TurboSound FM";
    ds.subsong[0]->hz=50;
    ds.systemLen=1;

    ds.system[0]=DIV_SYSTEM_YM2612;
    ds.loopModality=1;

    unsigned char magic[8]={0};

    reader.readNoRLE(magic,8);
    if (memcmp(magic,DIV_TFM_MAGIC,8)!=0) throw InvalidHeaderException();

    unsigned char speedEven=reader.readCNoRLE();
    unsigned char speedOdd=reader.readCNoRLE();
    unsigned char interleaveFactor=reader.readCNoRLE();

    // TODO: due to limitations with the groove pattern, only interleave factors up to 8
    // are allowed in furnace
    if (interleaveFactor>8) {
      addWarning("interleave factor is bigger than 8, speed information may be inaccurate");
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
    unsigned char loop_pos = reader.readCNoRLE();

    ds.createdDate=TFMparseDate(reader.readSNoRLE());
    ds.revisionDate=TFMparseDate(reader.readSNoRLE());

    // TODO: use this for something, number of saves
    (void)reader.readSNoRLE();

    // author
    logD("parsing author");
    ds.author=reader.readString(64);

    // name
    logD("parsing name");
    ds.name=reader.readString(64);

    // notes
    logD("parsing notes");
    String notes=reader.readString(384);

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
    for (int i=0; i<256; i++) {
      if (patLens[i]==0) {
        maxPatLen=256;
        break;
      } else if (patLens[i]>maxPatLen) {
        maxPatLen=patLens[i];
      }
    }

    ds.subsong[0]->patLen=maxPatLen;

    struct TFMParsePatternInfo info;
    info.ds=&ds;
    info.insNumMaps=insNumMaps;
    info.maxPat=maxPat;
    info.patExists=patExists;
    info.orderList=orderList;
    info.speedEven=speedEven;
    info.speedOdd=speedOdd;
    info.interleaveFactor=interleaveFactor;
    info.patLens=patLens;
    info.reader=&reader;
    info.v2=true;
    info.loop_pos=loop_pos;
    TFMParsePattern(info);

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
  
  delete[] file;
  return success;
}
