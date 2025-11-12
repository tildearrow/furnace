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

bool DivEngine::loadMod(unsigned char* file, size_t len) {
  struct InvalidHeaderException {};
  bool success=false;
  int chCount=0;
  int ordCount=0;
  std::vector<int> patPtr;
  char magic[4]={0,0,0,0};
  short defaultVols[31];
  int sampLens[31];
  // 0=arp, 1=pslide, 2=vib, 3=trem, 4=vslide
  bool fxUsage[DIV_MAX_CHANS][5];  
  SafeReader reader=SafeReader(file,len);
  warnings="";

  memset(defaultVols,0,31*sizeof(short));
  memset(sampLens,0,31*sizeof(int));
  memset(fxUsage,0,DIV_MAX_CHANS*5*sizeof(bool));

  try {
    DivSong ds;
    ds.tuning=436.0;
    ds.version=DIV_VERSION_MOD;
    ds.linearPitch=0;
    ds.noSlidesOnFirstTick=true;
    ds.rowResetsArpPos=true;
    ds.ignoreJumpAtEnd=false;
    ds.delayBehavior=0;

    int insCount=31;
    bool bypassLimits=false;

    // check mod magic bytes
    if (!reader.seek(1080,SEEK_SET)) {
      logD("couldn't seek to 1080");
      throw EndOfFileException(&reader,reader.tell());
    }
    if (reader.read(magic,4)!=4) {
      logD("the magic isn't complete");
      throw EndOfFileException(&reader,reader.tell());
    }
    if (memcmp(magic,"M.K.",4)==0 || memcmp(magic,"M!K!",4)==0 || memcmp(magic,"M&K!",4)==0) {
      logD("detected a ProTracker module");
      ds.systemName="Amiga";
      chCount=4;
    } else if (memcmp(magic,"CD81",4)==0 || memcmp(magic,"OKTA",4)==0 || memcmp(magic,"OCTA",4)==0) {
      logD("detected an Oktalyzer/Octalyzer/OctaMED module");
      ds.systemName="Amiga (8-channel)";
      chCount=8;
    } else if (memcmp(magic+1,"CHN",3)==0 && magic[0]>='1' && magic[0]<='9') {
      logD("detected a FastTracker module");
      ds.systemName="PC";
      chCount=magic[0]-'0';
    } else if (memcmp(magic,"FLT",3)==0 && magic[3]>='1' && magic[3]<='9') {
      logD("detected a Fairlight module");
      ds.systemName="Amiga";
      chCount=magic[3]-'0';
    } else if (memcmp(magic,"TDZ",3)==0 && magic[3]>='1' && magic[3]<='9') {
      logD("detected a TakeTracker module");
      ds.systemName="PC";
      chCount=magic[3]-'0';
    } else if ((memcmp(magic+2,"CH",2)==0 || memcmp(magic+2,"CN",2)==0)  &&
               (magic[0]>='1' && magic[0]<='9' && magic[1]>='0' && magic[1]<='9')) {
      logD("detected a Fast/TakeTracker module");
      ds.systemName="PC";
      chCount=((magic[0]-'0')*10)+(magic[1]-'0');
    } else {
      insCount=15;
      logD("possibly a Soundtracker module");
      ds.systemName="Amiga";
      chCount=4;
    }

    // song name
    if (!reader.seek(0,SEEK_SET)) {
      logD("couldn't seek to 0");
      throw EndOfFileException(&reader,reader.tell());
    }
    ds.name=reader.readStringLatin1(20);
    logI("%s",ds.name);

    // samples
    logD("reading samples... (%d)",insCount);
    ds.sample.reserve(insCount);
    for (int i=0; i<insCount; i++) {
      DivSample* sample=new DivSample;
      sample->depth=DIV_SAMPLE_DEPTH_8BIT;
      sample->name=reader.readStringLatin1(22);
      logD("%d: %s",i+1,sample->name);
      int slen=((unsigned short)reader.readS_BE())*2;
      sampLens[i]=slen;
      if (slen==2) slen=0;
      signed char fineTune=reader.readC()&0x0f;
      if (fineTune>=8) fineTune-=16;
      sample->centerRate=(int)(pow(2.0,(double)fineTune/96.0)*8363.0);
      defaultVols[i]=reader.readC();
      int loopStart=reader.readS_BE()*2;
      int loopLen=reader.readS_BE()*2;
      int loopEnd=loopStart+loopLen;
      // bunch of checks since ProTracker abuses those for one-shot samples
      if (loopStart>loopEnd || loopEnd<4 || loopLen<4) {
        loopStart=0;
        loopLen=0;
      }
      if (loopLen>=2) {
        sample->loopStart=loopStart;
        sample->loopEnd=loopEnd;
        sample->loop=(sample->loopStart>=0)&&(sample->loopEnd>=0);
      }
      sample->init(slen);
      ds.sample.push_back(sample);
    }
    ds.sampleLen=ds.sample.size();

    // orders
    ds.subsong[0]->ordersLen=ordCount=(unsigned char)reader.readC();
    if (ds.subsong[0]->ordersLen<1 || ds.subsong[0]->ordersLen>128) {
      logD("invalid order count!");
      throw EndOfFileException(&reader,reader.tell());
    }
    unsigned char restartPos=reader.readC(); // restart position, unused
    logD("restart position byte: %.2x",restartPos);
    if (insCount==15) {
      if (restartPos>0x60 && restartPos<0x80) {
        logD("detected a Soundtracker module");
      } else {
        logD("no Soundtracker signature found");
        throw EndOfFileException(&reader,reader.tell());
      }
    }

    int patMax=0;
    for (int i=0; i<128; i++) {
      unsigned char pat=reader.readC();
      if (pat>patMax) patMax=pat;
      for (int j=0; j<chCount; j++) {
        ds.subsong[0]->orders.ord[j][i]=pat;
      }
    }

    if (insCount==15) {
      if (!reader.seek(600,SEEK_SET)) {
        logD("couldn't seek to 600");
        throw EndOfFileException(&reader,reader.tell());
      }
    } else {
      if (!reader.seek(1084,SEEK_SET)) {
        logD("couldn't seek to 1084");
        throw EndOfFileException(&reader,reader.tell());
      }
    }

    // patterns
    ds.subsong[0]->patLen=64;
    for (int ch=0; ch<chCount; ch++) {
      for (int i=0; i<5; i++) {
        fxUsage[ch][i]=false;
      }
    }
    for (int pat=0; pat<=patMax; pat++) {
      DivPattern* chpats[DIV_MAX_CHANS];
      for (int ch=0; ch<chCount; ch++) {
        chpats[ch]=ds.subsong[0]->pat[ch].getPattern(pat,true);
      }
      for (int row=0; row<64; row++) {
        for (int ch=0; ch<chCount; ch++) {
          short* dstrowN=chpats[ch]->newData[row];
          unsigned char data[4];
          reader.read(&data,4);
          // instrument
          short ins=(data[0]&0xf0)|(data[2]>>4);
          if (ins>0) {
            dstrowN[DIV_PAT_INS]=ins-1;
            dstrowN[DIV_PAT_VOL]=defaultVols[ins-1];
          }
          // note
          int period=data[1]+((data[0]&0x0f)<<8);
          if (period>0 && period<0x0fff) {
            short note=(short)round(log2(3424.0/period)*12);
            dstrowN[DIV_PAT_NOTE]=note+72;
            if (period<114) {
              bypassLimits=true;
            }
          }
          // effects are done later
          short fxtyp=data[2]&0x0f;
          short fxval=data[3];
          dstrowN[DIV_PAT_FX(0)]=fxtyp;
          dstrowN[DIV_PAT_FXVAL(0)]=fxval;
          switch (fxtyp) {
            case 0:
              if (fxval!=0) fxUsage[ch][0]=true;
              break;
            case 1: case 2: case 3:
              fxUsage[ch][1]=true;
              break;
            case 4:
              fxUsage[ch][2]=true;
              break;
            case 5:
              fxUsage[ch][1]=true;
              fxUsage[ch][4]=true;
              break;
            case 6:
              fxUsage[ch][2]=true;
              fxUsage[ch][4]=true;
              break;
            case 7:
              fxUsage[ch][3]=true;
              break;
            case 10:
              if (fxval!=0) fxUsage[ch][4]=true;
              break;
          }
        }
      }
    }

    // samples
    size_t pos=reader.tell();
    logD("reading sample data...");
    for (int i=0; i<insCount; i++) {
      logV("- %d: %d %d %d",i,pos,ds.sample[i]->samples,sampLens[i]);
      if (!reader.seek(pos,SEEK_SET)) {
        logD("%d: couldn't seek to %d",i,pos);
        throw EndOfFileException(&reader,reader.tell());
      }
      reader.read(ds.sample[i]->data8,ds.sample[i]->samples);
      pos+=sampLens[i];
    }

    // convert effects
    logD("converting module...");
    for (int ch=0; ch<=chCount; ch++) {
      unsigned char fxCols=1;
      for (int pat=0; pat<=patMax; pat++) {
        auto* newData=ds.subsong[0]->pat[ch].getPattern(pat,true)->newData;
        short lastPitchEffect=-1;
        short lastEffectState[5]={-1,-1,-1,-1,-1};
        short setEffectState[5]={-1,-1,-1,-1,-1};
        for (int row=0;row<64;row++) {
          const short fxUsageTyp[5]={0x00,0x01,0x04,0x07,0xFA};
          short effectState[5]={0,0,0,0,0};
          unsigned char curFxCol=0;
          short fxTyp=newData[row][DIV_PAT_FX(0)];
          short fxVal=newData[row][DIV_PAT_FXVAL(0)];
          auto writeFxCol=[newData,row,&curFxCol](short typ, short val) {
            newData[row][DIV_PAT_FX(curFxCol)]=typ;
            newData[row][DIV_PAT_FXVAL(curFxCol)]=val;
            curFxCol++;
          };
          writeFxCol(-1,-1);
          curFxCol=0;
          switch (fxTyp) {
            case 0: // arp
              effectState[0]=fxVal;
              break;
            case 5: // vol slide + porta
              effectState[4]=fxVal;
              fxTyp=3;
              fxVal=0;
              // fall through
            case 1: // note slide up
            case 2: // note slide down
            case 3: // porta
              if (fxTyp==3 && fxVal==0) {
                if (setEffectState[1]<0) break;
                fxVal=setEffectState[1];
              }
              setEffectState[1]=fxVal;
              effectState[1]=fxVal;
              if ((effectState[1]!=lastEffectState[1]) ||
                  (fxTyp!=lastPitchEffect) ||
                  (effectState[1]!=0 && newData[row][DIV_PAT_NOTE]>-1)) {
                writeFxCol(fxTyp,fxVal);
              }
              lastPitchEffect=fxTyp;
              lastEffectState[1]=fxVal;
              break;
            case 6: // vol slide + vibrato
              effectState[4]=fxVal;
              fxTyp=4;
              fxVal=0;
              // fall through
            case 4: // vibrato
              // TODO: handle 0 value?
              if (fxVal==0) {
                if (setEffectState[2]<0) break;
                fxVal=setEffectState[2];
              }
              effectState[2]=fxVal;
              setEffectState[2]=fxVal;
              break;
            case 7: // tremolo
              if (fxVal==0) {
                if (setEffectState[3]<0) break;
                fxVal=setEffectState[3];
              }
              effectState[3]=fxVal;
              setEffectState[3]=fxVal;
              break;
            case 9: // set offset
              writeFxCol(0x91,fxVal);
              break;
            case 10: // vol slide
              effectState[4]=fxVal;
              break;
            case 11: // jump to pos
              writeFxCol(fxTyp,fxVal);
              break;
            case 12: // set vol
              newData[row][DIV_PAT_VOL]=MIN(0x40,fxVal);
              break;
            case 13: // break to row (BCD)
              writeFxCol(fxTyp,((fxVal>>4)*10)+(fxVal&15));
              break;
            case 15: // set speed
              // TODO: somehow handle VBlank tunes
              // TODO: i am so sorry
              if (fxVal>0x20 && ds.name!="klisje paa klisje") {
                writeFxCol(0xf0,fxVal);
              } else {
                writeFxCol(0x0f,fxVal);
              }
              break;
            case 14: // extended
              fxTyp=fxVal>>4;
              fxVal&=0x0f;
              switch (fxTyp) {
                case 0:
                  writeFxCol(0x10,!fxVal);
                  break;
                case 1: // single note slide up
                case 2: // single note slide down
                  writeFxCol(fxTyp-1+0xf1,fxVal);
                  break;
                case 4: // vibrato waveform
                  switch (fxVal&3) {
                    case 0: // sine
                      writeFxCol(0xe3,0x00);
                      break;
                    case 1: // ramp down
                      writeFxCol(0xe3,0x05);
                      break;
                    case 2: // square
                    case 3: 
                      writeFxCol(0xe3,0x06);
                      break;
                  }
                break;
                case 9: // retrigger
                  writeFxCol(0x0c,fxVal);
                  break;
                case 10: // single vol slide up
                case 11: // single vol slide down
                  writeFxCol(fxTyp-10+0xf8,fxVal);
                  break;
                case 12: // note cut
                case 13: // note delay
                  writeFxCol(fxTyp-12+0xec,fxVal);
                  break;
              }
              break;
          }
          for (int i=0; i<5; i++) {
            // pitch slide and volume slide needs to be kept active on new note
            // even after target/max is reached
            if (fxUsage[ch][i] && (effectState[i]!=lastEffectState[i] || (effectState[i]!=0 && i==4 && newData[row][DIV_PAT_VOL]>=0))) {
              writeFxCol(fxUsageTyp[i],effectState[i]);
            }
          }
          memcpy(lastEffectState,effectState,sizeof(effectState));
          if (curFxCol>fxCols) {
            fxCols=curFxCol;
          }
        }
      }
      ds.subsong[0]->pat[ch].effectCols=fxCols;
    }

    ds.subsong[0]->hz=50;
    ds.systemLen=(chCount+3)/4;
    for(int i=0; i<ds.systemLen; i++) {
      ds.system[i]=DIV_SYSTEM_AMIGA;
      ds.systemFlags[i].set("clockSel",1); // PAL
      ds.systemFlags[i].set("stereoSep",80);
      ds.systemFlags[i].set("bypassLimits",bypassLimits);
      ds.systemFlags[i].set("chipType",(bool)(ds.systemLen>1 || bypassLimits));
    }
    for(int i=0; i<chCount; i++) {
      ds.subsong[0]->chanShow[i]=true;
      ds.subsong[0]->chanShowChanOsc[i]=true;
      ds.subsong[0]->chanName[i]=fmt::sprintf("Channel %d",i+1);
      ds.subsong[0]->chanShortName[i]=fmt::sprintf("C%d",i+1);
    }
    for(int i=chCount; i<ds.systemLen*4; i++) {
      ds.subsong[0]->pat[i].effectCols=1;
      ds.subsong[0]->chanShow[i]=false;
      ds.subsong[0]->chanShowChanOsc[i]=false;
    }
    
    // instrument creation
    ds.ins.reserve(insCount);
    for(int i=0; i<insCount; i++) {
      DivInstrument* ins=new DivInstrument;
      ins->type=DIV_INS_AMIGA;
      ins->amiga.initSample=i;
      ins->name=ds.sample[i]->name;
      ds.ins.push_back(ins);
    }
    ds.insLen=ds.ins.size();

    // find subsongs
    ds.recalcChans();
    ds.findSubSongs();
    
    if (active) quitDispatch();
    BUSY_BEGIN_SOFT;
    saveLock.lock();
    song.unload();
    song=ds;
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
    //logE("invalid info header!");
    lastError="invalid info header!";
  }
  return success;
}

