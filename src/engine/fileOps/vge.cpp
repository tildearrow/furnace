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

#include "tfmCommon.h"

struct VGEParsePatternInfo {
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
  unsigned char loopPos;
};

void VGEParsePattern(struct VGEParsePatternInfo info) {
  // PATTERN DATA FORMAT (not described properly in the documentation)
  // for each channel in a pattern:
  //  - note data (256 bytes)
  //  - volume data (256 bytes, values always 0x00-0x1F)
  //  - instrument number data (256 bytes)
  //  - effect number (256 bytes, values 0x0-0x23 (to represent 0-F and G-Z))
  //  - effect value (256 bytes)
  //  - extra 3 effects (1536 bytes 256x3x2)
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
      info.reader->skip((256 * 11) * 10);
      continue;
    }

    logD("parsing pattern %d",i);
    for (int j=0; j<10; j++) {
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

      int numEffectsCol=4;
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

  bool chArpeggio[10]={false};
  bool chVibrato[10]={false};
  bool chPorta[10]={false};
  bool chVolumeSlide[10]={false};
  int lastPatSeen=0;

  for (int i=0; i<info.ds->subsong[0]->ordersLen; i++) {
    // this is if the last pattern is used more than once
    if (info.orderList[i] == info.orderList[info.ds->subsong[0]->ordersLen - 1]) {
      lastPatSeen++;
    }
    for (int j=0; j<10; j++) {
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
    for (int i=0;i<10;i++) {
      int lastPatNum=info.ds->subsong[0]->orders.ord[i][info.ds->subsong[0]->ordersLen - 1];
      DivPattern* newPat=info.ds->subsong[0]->pat[i].getPattern(info.maxPat,true);
      DivPattern* lastPat=info.ds->subsong[0]->pat[i].getPattern(lastPatNum, false);
      lastPat->copyOn(newPat);

      info.ds->subsong[0]->orders.ord[i][info.ds->subsong[0]->ordersLen - 1] = info.maxPat;
      newPat->data[info.patLens[lastPatNum]-1][4+(usedEffectsCol*4)] = 0x0B;
      newPat->data[info.patLens[lastPatNum]-1][5+(usedEffectsCol*4)] = info.loopPos;
      info.ds->subsong[0]->pat[i].data[info.maxPat] = newPat;
    }
  } else {
    for (int i=0;i<10;i++) {
      int lastPatNum=info.ds->subsong[0]->orders.ord[i][info.ds->subsong[0]->ordersLen - 1];
      DivPattern* lastPat=info.ds->subsong[0]->pat[i].getPattern(lastPatNum, false);
      lastPat->data[info.patLens[lastPatNum]-1][4+(usedEffectsCol*4)] = 0x0B;
      lastPat->data[info.patLens[lastPatNum]-1][5+(usedEffectsCol*4)] = info.loopPos;
    }
  }
}

bool DivEngine::loadVGE(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  TFMRLEReader reader=TFMRLEReader(file,len);

  try {
    DivSong ds;
    ds.version=DIV_VERSION_TFE;
    ds.systemName="Sega Genesis/Mega Drive or TurboSound FM";
    ds.systemLen=2;

    ds.system[0]=DIV_SYSTEM_YM2612;
    ds.system[1]=DIV_SYSTEM_SMS;
    ds.loopModality=1;

    unsigned char magic[8]={0};

    reader.readNoRLE(magic,8);
    if (memcmp(magic,DIV_VGE_MAGIC,8)!=0) throw InvalidHeaderException();

    // Read the size of the header
    unsigned int headerSize=reader.readINoRLE();
    unsigned int sampleDescSize=reader.readINoRLE();

    // Read the clock rates
    ds.systemFlags[0].set("customClock", reader.readINoRLE());
    ds.systemFlags[1].set("customClock", reader.readINoRLE());
    ds.subsong[0]->hz = reader.readSNoRLE();

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

    unsigned short globalPCMQuality=reader.readSNoRLE();

    ds.createdDate=TFMparseDate(reader.readSNoRLE());
    ds.revisionDate=TFMparseDate(reader.readSNoRLE());

    // TODO: use this for something, number of saves
    (void)reader.readSNoRLE();

    ds.subsong[0]->ordersLen=reader.readCNoRLE();

    // order loop position
    unsigned char loopPos = reader.readCNoRLE();

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

      for (int j=0; j<10; j++) {
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
        reader.skip(43);
        continue;
      }

      insMaps[i]->fm.alg=reader.readC();
      insMaps[i]->fm.fb=reader.readC();
      unsigned char ams_fms=reader.readC();
      insMaps[i]->fm.fms = ams_fms&0xF;
      insMaps[i]->fm.ams = ams_fms>>4;

      for (int j=0; j<4; j++) {
        insMaps[i]->fm.op[j].mult=reader.readC();
        insMaps[i]->fm.op[j].dt=reader.readC();
        insMaps[i]->fm.op[j].tl=reader.readC()^0x7F;
        insMaps[i]->fm.op[j].rs=reader.readC();
        insMaps[i]->fm.op[j].ar=reader.readC()^0x1F;
        unsigned char dr=reader.readC()^0x1F;
        insMaps[i]->fm.op[j].dr=dr&0x7F;
        insMaps[i]->fm.op[j].am=dr>>7;
        insMaps[i]->fm.op[j].d2r=reader.readC()^0x1F;
        insMaps[i]->fm.op[j].rr=reader.readC()^0xF;
        insMaps[i]->fm.op[j].sl=reader.readC();
        insMaps[i]->fm.op[j].ssgEnv=reader.readC();
      }
    }

    // sample instrument data
    // TODO: actually implement this.
    reader.skip(255 * 16);
    reader.skip(255 * 4);

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

    struct VGEParsePatternInfo info;
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
    info.loopPos=loopPos;
    VGEParsePattern(info);

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
