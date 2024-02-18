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

#define CHECK_BLOCK_VERSION(x) \
  if (blockVersion>x) { \
    logW("incompatible block version %d for %s!",blockVersion,blockName); \
  }

const int ftEffectMap[]={
  -1, // none
  0x0f,
  0x0b,
  0x0d,
  0xff,
  -1, // volume? not supported in Furnace yet
  0x03,
  0x03, // unused?
  0x13,
  0x14,
  0x00,
  0x04,
  0x07,
  0xe5,
  0xed,
  0x11,
  0x01, // porta up
  0x02, // porta down
  0x12,
  0x90, // sample offset - not supported yet
  0xe1,
  0xe2,
  0x0a,
  0xec,
  0x0c,
  -1, // delayed volume - not supported yet
  0x11, // FDS
  0x12,
  0x13,
  0x20, // DPCM pitch
  0x22, // 5B
  0x24,
  0x23,
  0x21,
  -1, // VRC7 "custom patch port" - not supported?
  -1, // VRC7 "custom patch write"
  -1, // release - not supported yet
  0x09, // select groove
  -1, // transpose - not supported
  0x10, // Namco 163
  -1, // FDS vol env - not supported
  -1, // FDS auto FM - not supported yet
  -1, // phase reset - not supported
  -1, // harmonic - not supported
};

constexpr int ftEffectMapSize=sizeof(ftEffectMap)/sizeof(int);

bool DivEngine::loadFTM(unsigned char* file, size_t len) {
  SafeReader reader=SafeReader(file,len);
  warnings="";
  try {
    DivSong ds;
    String blockName;
    unsigned char expansions=0;
    unsigned int tchans=0;
    unsigned int n163Chans=0;
    bool hasSequence[256][8];
    unsigned char sequenceIndex[256][8];
    unsigned int hilightA=4;
    unsigned int hilightB=16;
    double customHz=60;
    
    memset(hasSequence,0,256*8*sizeof(bool));
    memset(sequenceIndex,0,256*8);

    if (!reader.seek(18,SEEK_SET)) {
      logE("premature end of file!");
      lastError="incomplete file";
      delete[] file;
      return false;
    }
    ds.version=(unsigned short)reader.readI();
    logI("module version %d (0x%.4x)",ds.version,ds.version);

    if (ds.version>0x0450) {
      logE("incompatible version %x!",ds.version);
      lastError="incompatible version";
      delete[] file;
      return false;
    }

    for (DivSubSong* i: ds.subsong) {
      i->clearData();
      delete i;
    }
    ds.subsong.clear();

    ds.linearPitch=0;

    while (true) {
      blockName=reader.readString(3);
      if (blockName=="END") {
        // end of module
        logD("end of data");
        break;
      }

      // not the end
      reader.seek(-3,SEEK_CUR);
      blockName=reader.readString(16);
      unsigned int blockVersion=(unsigned int)reader.readI();
      unsigned int blockSize=(unsigned int)reader.readI();
      size_t blockStart=reader.tell();
      
      logD("reading block %s (version %d, %d bytes)",blockName,blockVersion,blockSize);
      if (blockName=="PARAMS") {
        // versions 7-9 don't change anything?
        CHECK_BLOCK_VERSION(9);
        unsigned int oldSpeedTempo=0;
        if (blockVersion<=1) {
          oldSpeedTempo=reader.readI();
        }
        if (blockVersion>=2) {
          expansions=reader.readC();
        }
        tchans=reader.readI();
        unsigned int pal=reader.readI();
        if (blockVersion>=7) {
          // advanced Hz control
          int controlType=reader.readI();
          switch (controlType) {
            case 1:
              customHz=1000000.0/(double)reader.readI();
              break;
            default:
              reader.readI();
              break;
          }
        } else {
          customHz=reader.readI();
        }
        unsigned int newVibrato=0;
        bool sweepReset=false;
        unsigned int speedSplitPoint=0;
        if (blockVersion>=3) {
          newVibrato=reader.readI();
        }
        if (blockVersion>=9) {
          sweepReset=reader.readI();
        }
        if (blockVersion>=4 && blockVersion<7) {
          hilightA=reader.readI();
          hilightB=reader.readI();
        }
        if (expansions&8) if (blockVersion>=5) { // N163 channels
          n163Chans=reader.readI();
        }
        if (blockVersion>=6) {
          speedSplitPoint=reader.readI();
        }

        if (blockVersion>=8) {
          int fineTuneCents=reader.readC()*100;
          fineTuneCents+=reader.readC();

          ds.tuning=440.0*pow(2.0,(double)fineTuneCents/1200.0);
        }

        logV("old speed/tempo: %d",oldSpeedTempo);
        logV("expansions: %x",expansions);
        logV("channels: %d",tchans);
        logV("PAL: %d",pal);
        logV("custom Hz: %f",customHz);
        logV("new vibrato: %d",newVibrato);
        logV("N163 channels: %d",n163Chans);
        logV("highlight 1: %d",hilightA);
        logV("highlight 2: %d",hilightB);
        logV("split point: %d",speedSplitPoint);
        logV("sweep reset: %d",sweepReset);

        // initialize channels
        int systemID=0;
        ds.system[systemID++]=DIV_SYSTEM_NES;
        if (expansions&1) {
          ds.system[systemID++]=DIV_SYSTEM_VRC6;
        }
        if (expansions&2) {
          ds.system[systemID++]=DIV_SYSTEM_VRC7;
        }
        if (expansions&4) {
          ds.system[systemID++]=DIV_SYSTEM_FDS;
        }
        if (expansions&8) {
          ds.system[systemID++]=DIV_SYSTEM_MMC5;
        }
        if (expansions&16) {
          ds.system[systemID]=DIV_SYSTEM_N163;
          ds.systemFlags[systemID++].set("channels",(int)n163Chans);
        }
        if (expansions&32) {
          ds.system[systemID]=DIV_SYSTEM_AY8910;
          ds.systemFlags[systemID++].set("chipType",2); // Sunsoft 5B
        }
        ds.systemLen=systemID;

        unsigned int calcChans=0;
        for (int i=0; i<ds.systemLen; i++) {
          calcChans+=getChannelCount(ds.system[i]);
        }
        if (calcChans!=tchans) {
          logE("channel counts do not match! %d != %d",tchans,calcChans);
          lastError="channel counts do not match";
          delete[] file;
          return false;
        }
        if (tchans>DIV_MAX_CHANS) {
          tchans=DIV_MAX_CHANS;
          logW("too many channels!");
        }
      } else if (blockName=="INFO") {
        CHECK_BLOCK_VERSION(1);
        ds.name=reader.readString(32);
        ds.author=reader.readString(32);
        ds.category=reader.readString(32);
        ds.systemName="NES";
      } else if (blockName=="HEADER") {
        CHECK_BLOCK_VERSION(4);
        unsigned char totalSongs=reader.readC();
        logV("%d songs:",totalSongs+1);
        ds.subsong.reserve(totalSongs);
        for (int i=0; i<=totalSongs; i++) {
          String subSongName=reader.readString();
          ds.subsong.push_back(new DivSubSong);
          ds.subsong[i]->name=subSongName;
          ds.subsong[i]->hilightA=hilightA;
          ds.subsong[i]->hilightB=hilightB;
          if (customHz!=0) {
            ds.subsong[i]->hz=customHz;
          }
          logV("- %s",subSongName);
        }
        for (unsigned int i=0; i<tchans; i++) {
          // TODO: obey channel ID
          unsigned char chID=reader.readC();
          logV("for channel ID %d",chID);
          for (int j=0; j<=totalSongs; j++) {
            unsigned char effectCols=reader.readC();
            ds.subsong[j]->pat[i].effectCols=effectCols+1;
            logV("- song %d has %d effect columns",j,effectCols);
          }
        }

        if (blockVersion>=4) {
          for (int i=0; i<=totalSongs; i++) {
            ds.subsong[i]->hilightA=(unsigned char)reader.readC();
            ds.subsong[i]->hilightB=(unsigned char)reader.readC();
          }
        }
      } else if (blockName=="INSTRUMENTS") {
        CHECK_BLOCK_VERSION(6);

        reader.seek(blockSize,SEEK_CUR);

        /*
        ds.insLen=reader.readI();
        if (ds.insLen<0 || ds.insLen>256) {
          logE("too many instruments/out of range!");
          lastError="too many instruments/out of range";
          delete[] file;
          return false;
        }

        for (int i=0; i<ds.insLen; i++) {
          DivInstrument* ins=new DivInstrument;
          ds.ins.push_back(ins);
        }

        logV("instruments:");
        for (int i=0; i<ds.insLen; i++) {
          unsigned int insIndex=reader.readI();
          if (insIndex>=ds.ins.size()) {
            logE("instrument index %d is out of range!",insIndex);
            lastError="instrument index out of range";
            delete[] file;
            return false;
          }

          DivInstrument* ins=ds.ins[insIndex];
          unsigned char insType=reader.readC();
          switch (insType) {
            case 1:
              ins->type=DIV_INS_NES;
              break;
            case 2: // TODO: tell VRC6 and VRC6 saw instruments apart
              ins->type=DIV_INS_VRC6;
              break;
            case 3:
              ins->type=DIV_INS_OPLL;
              break;
            case 4:
              ins->type=DIV_INS_FDS;
              break;
            case 5:
              ins->type=DIV_INS_N163;
              break;
            case 6: // 5B?
              ins->type=DIV_INS_AY;
              break;
            default: {
              logE("%d: invalid instrument type %d",insIndex,insType);
              lastError="invalid instrument type";
              delete[] file;
              return false;
            }
          }

          // instrument data
          switch (ins->type) {
            case DIV_INS_NES: {
              unsigned int totalSeqs=reader.readI();
              if (totalSeqs>5) {
                logE("%d: too many sequences!",insIndex);
                lastError="too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j=0; j<totalSeqs; j++) {
                hasSequence[insIndex][j]=reader.readC();
                sequenceIndex[insIndex][j]=reader.readC();
              }

              const int dpcmNotes=(blockVersion>=2)?96:72;
              for (int j=0; j<dpcmNotes; j++) {
                ins->amiga.noteMap[j].map=(short)((unsigned char)reader.readC())-1;
                ins->amiga.noteMap[j].freq=(unsigned char)reader.readC();
                if (blockVersion>=6) {
                  reader.readC(); // DMC value
                }
              }
              break;
            }
            case DIV_INS_VRC6: {
              unsigned int totalSeqs=reader.readI();
              if (totalSeqs>4) {
                logE("%d: too many sequences!",insIndex);
                lastError="too many sequences";
                delete[] file;
                return false;
              }

              for (unsigned int j=0; j<totalSeqs; j++) {
                hasSequence[insIndex][j]=reader.readC();
                sequenceIndex[insIndex][j]=reader.readC();
              }
              break;
            }
            case DIV_INS_OPLL: {
              ins->fm.opllPreset=(unsigned int)reader.readI();
              // TODO
              break;
            }
            case DIV_INS_FDS: {
              DivWavetable* wave=new DivWavetable;
              wave->len=64;
              wave->max=64;
              for (int j=0; j<64; j++) {
                wave->data[j]=reader.readC();
              }
              ins->std.waveMacro.len=1;
              ins->std.waveMacro.val[0]=ds.wave.size();
              for (int j=0; j<32; j++) {
                ins->fds.modTable[j]=reader.readC()-3;
              }
              ins->fds.modSpeed=reader.readI();
              ins->fds.modDepth=reader.readI();
              reader.readI(); // this is delay. currently ignored. TODO.
              ds.wave.push_back(wave);

              ins->std.volMacro.len=reader.readC();
              ins->std.volMacro.loop=reader.readI();
              ins->std.volMacro.rel=reader.readI();
              reader.readI(); // arp mode does not apply here
              for (int j=0; j<ins->std.volMacro.len; j++) {
                ins->std.volMacro.val[j]=reader.readC();
              }

              ins->std.arpMacro.len=reader.readC();
              ins->std.arpMacro.loop=reader.readI();
              ins->std.arpMacro.rel=reader.readI();
              // TODO: get rid
              ins->std.arpMacro.mode=reader.readI();
              for (int j=0; j<ins->std.arpMacro.len; j++) {
                ins->std.arpMacro.val[j]=reader.readC();
              }

              ins->std.pitchMacro.len=reader.readC();
              ins->std.pitchMacro.loop=reader.readI();
              ins->std.pitchMacro.rel=reader.readI();
              reader.readI(); // arp mode does not apply here
              for (int j=0; j<ins->std.pitchMacro.len; j++) {
                ins->std.pitchMacro.val[j]=reader.readC();
              }

              break;
            }
            case DIV_INS_N163: {
              // TODO!
              break;
            }
            // TODO: 5B!
            default: {
              logE("%d: what's going on here?",insIndex);
              lastError="invalid instrument type";
              delete[] file;
              return false;
            }
          }

          // name
          ins->name=reader.readString((unsigned int)reader.readI());
          logV("- %d: %s",insIndex,ins->name);
        }
        */
      } else if (blockName=="SEQUENCES") {
        CHECK_BLOCK_VERSION(6);
        reader.seek(blockSize,SEEK_CUR);
      } else if (blockName=="FRAMES") {
        CHECK_BLOCK_VERSION(3);

        for (size_t i=0; i<ds.subsong.size(); i++) {
          DivSubSong* s=ds.subsong[i];

          s->ordersLen=reader.readI();
          if (blockVersion>=3) {
            s->speeds.val[0]=reader.readI();
          }
          if (blockVersion>=2) {
            s->virtualTempoN=reader.readI();
            s->patLen=reader.readI();
          }
          int why=tchans;
          if (blockVersion==1) {
            why=reader.readI();
          }
          logV("reading %d and %d orders",tchans,s->ordersLen);

          for (int j=0; j<s->ordersLen; j++) {
            for (int k=0; k<why; k++) {
              unsigned char o=reader.readC();
              logV("%.2x",o);
              s->orders.ord[k][j]=o;
            }
          }
        }
      } else if (blockName=="PATTERNS") {
        CHECK_BLOCK_VERSION(6);

        size_t blockEnd=reader.tell()+blockSize;

        if (blockVersion==1) {
          int patLenOld=reader.readI();
          for (DivSubSong* i: ds.subsong) {
            i->patLen=patLenOld;
          }
        }

        // so it appears .ftm doesn't keep track of how many patterns are stored in the file....
        while (reader.tell()<blockEnd) {
          int subs=0;
          if (blockVersion>=2) subs=reader.readI();
          int ch=reader.readI();
          int patNum=reader.readI();
          int numRows=reader.readI();

          DivPattern* pat=ds.subsong[subs]->pat[ch].getPattern(patNum,true);
          for (int i=0; i<numRows; i++) {
            unsigned int row=0;
            if (blockVersion>=2 && blockVersion<6) { // row index
              row=reader.readI();
            } else {
              row=reader.readC();
            }

            unsigned char nextNote=reader.readC();
            unsigned char nextOctave=reader.readC();
            if (nextNote==0x0d) {
              pat->data[row][0]=100;
            } else if (nextNote==0x0e) {
              pat->data[row][0]=101;
            } else if (nextNote==0x01) {
              pat->data[row][0]=12;
              pat->data[row][1]=nextOctave-1;
            } else if (nextNote==0) {
              pat->data[row][0]=0;
            } else if (nextNote<0x0d) {
              pat->data[row][0]=nextNote-1;
              pat->data[row][1]=nextOctave;
            }
            
            unsigned char nextIns=reader.readC();
            if (nextIns<0x40) {
              pat->data[row][2]=nextIns;
            } else {
              pat->data[row][2]=-1;
            }

            unsigned char nextVol=reader.readC();
            if (nextVol<0x10) {
              pat->data[row][3]=nextVol;
            } else {
              pat->data[row][3]=-1;
            }

            int effectCols=ds.subsong[subs]->pat[ch].effectCols;
            if (blockVersion>=6) effectCols=4;

            for (int j=0; j<effectCols; j++) {
              unsigned char nextEffect=reader.readC();
              unsigned char nextEffectVal=0;
              if (nextEffect!=0 || blockVersion<6) nextEffectVal=reader.readC();
              if (nextEffect==0 && nextEffectVal==0) {
                pat->data[row][4+(j*2)]=-1;
                pat->data[row][5+(j*2)]=-1;
              } else {
                if (nextEffect<ftEffectMapSize) {
                  pat->data[row][4+(j*2)]=ftEffectMap[nextEffect];
                } else {
                  pat->data[row][4+(j*2)]=-1;
                }
                pat->data[row][5+(j*2)]=nextEffectVal;
              }
            }
          }
        }
      } else if (blockName=="DPCM SAMPLES") {
        CHECK_BLOCK_VERSION(1);
        reader.seek(blockSize,SEEK_CUR);
      } else if (blockName=="SEQUENCES_VRC6") {
        // where are the 5B and FDS sequences?
        CHECK_BLOCK_VERSION(6);
        reader.seek(blockSize,SEEK_CUR);
      } else if (blockName=="SEQUENCES_N163") {
        CHECK_BLOCK_VERSION(1);
        reader.seek(blockSize,SEEK_CUR);
      } else if (blockName=="COMMENTS") {
        CHECK_BLOCK_VERSION(1);
        reader.seek(blockSize,SEEK_CUR);
      } else {
        logE("block %s is unknown!",blockName);
        lastError="unknown block "+blockName;
        delete[] file;
        return false;
      }

      if ((reader.tell()-blockStart)!=blockSize) {
        logE("block %s is incomplete!",blockName);
        lastError="incomplete block "+blockName;
        delete[] file;
        return false;
      }
    }

    addWarning("FamiTracker import is experimental!");

    ds.version=DIV_VERSION_FTM;

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
  } catch (EndOfFileException& e) {
    logE("premature end of file!");
    lastError="incomplete file";
    delete[] file;
    return false;
  }
  delete[] file;
  return true;
}

