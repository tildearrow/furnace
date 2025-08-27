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

#define _USE_MATH_DEFINES
#include "dispatch.h"
#include "song.h"
#include "engine.h"
#include "instrument.h"
#include "safeReader.h"
#include "workPool.h"
#include "../ta-log.h"
#include "../fileutils.h"
#ifdef HAVE_SDL2
#include "../audio/sdlAudio.h"
#endif
#include <stdexcept>
#ifdef HAVE_JACK
#include "../audio/jack.h"
#endif
#ifdef HAVE_PA
#include "../audio/pa.h"
#endif
#include "../audio/pipe.h"
#include <math.h>
#include <float.h>
#include <fmt/printf.h>
#include <chrono>

void process(void* u, float** in, float** out, int inChans, int outChans, unsigned int size) {
  ((DivEngine*)u)->nextBuf(in,out,inChans,outChans,size);
}

const char* DivEngine::getEffectDesc(unsigned char effect, int chan, bool notNull) {
  switch (effect) {
    case 0x00:
      return _("00xy: Arpeggio");
    case 0x01:
      return _("01xx: Pitch slide up");
    case 0x02:
      return _("02xx: Pitch slide down");
    case 0x03:
      return _("03xx: Portamento");
    case 0x04:
      return _("04xy: Vibrato (x: speed; y: depth)");
    case 0x05:
      return _("05xy: Volume slide + vibrato (compatibility only!)");
    case 0x06:
      return _("06xy: Volume slide + portamento (compatibility only!)");
    case 0x07:
      return _("07xy: Tremolo (x: speed; y: depth)");
    case 0x08:
      return _("08xy: Set panning (x: left; y: right)");
    case 0x09:
      return _("09xx: Set groove pattern (speed 1 if no grooves exist)");
    case 0x0a:
      return _("0Axy: Volume slide (0y: down; x0: up)");
    case 0x0b:
      return _("0Bxx: Jump to pattern");
    case 0x0c:
      return _("0Cxx: Retrigger");
    case 0x0d:
      return _("0Dxx: Jump to next pattern");
    case 0x0f:
      return _("0Fxx: Set speed (speed 2 if no grooves exist)");
    case 0x80:
      return _("80xx: Set panning (00: left; 80: center; FF: right)");
    case 0x81:
      return _("81xx: Set panning (left channel)");
    case 0x82:
      return _("82xx: Set panning (right channel)");
    case 0x83:
      return _("83xy: Panning slide (x0: left; 0y: right)");
    case 0x84:
      return _("84xy: Panbrello (x: speed; y: depth)");
    case 0x88:
      return _("88xy: Set panning (rear channels; x: left; y: right)");
      break;
    case 0x89:
      return _("89xx: Set panning (rear left channel)");
      break;
    case 0x8a:
      return _("8Axx: Set panning (rear right channel)");
      break;
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
      return _("Cxxx: Set tick rate (hz)");
    case 0xd3:
      return _("D3xx: Volume portamento");
    case 0xd4:
      return _("D4xx: Volume portamento (fast)");
    case 0xdc:
      return _("DCxx: Delayed mute");
    case 0xe0:
      return _("E0xx: Set arp speed");
    case 0xe1:
      return _("E1xy: Note slide up (x: speed; y: semitones)");
    case 0xe2:
      return _("E2xy: Note slide down (x: speed; y: semitones)");
    case 0xe3:
      return _("E3xx: Set vibrato shape");
    case 0xe4:
      return _("E4xx: Set vibrato range");
    case 0xe5:
      return _("E5xx: Set pitch (80: center)");
    case 0xe6:
      return _("E6xy: Quick legato (x: time (0-7 up; 8-F down); y: semitones)");
    case 0xe7:
      return _("E7xx: Macro release");
    case 0xe8:
      return _("E8xy: Quick legato up (x: time; y: semitones)");
    case 0xe9:
      return _("E9xy: Quick legato down (x: time; y: semitones)");
    case 0xea:
      return _("EAxx: Legato");
    case 0xeb:
      return _("EBxx: Set LEGACY sample mode bank");
    case 0xec:
      return _("ECxx: Note cut");
    case 0xed:
      return _("EDxx: Note delay");
    case 0xee:
      return _("EExx: Send external command");
    case 0xf0:
      return _("F0xx: Set tick rate (bpm)");
    case 0xf1:
      return _("F1xx: Single tick pitch up");
    case 0xf2:
      return _("F2xx: Single tick pitch down");
    case 0xf3:
      return _("F3xx: Fine volume slide up");
    case 0xf4:
      return _("F4xx: Fine volume slide down");
    case 0xf5:
      return _("F5xx: Disable macro (see manual)");
    case 0xf6:
      return _("F6xx: Enable macro (see manual)");
    case 0xf7:
      return _("F7xx: Restart macro (see manual)");
    case 0xf8:
      return _("F8xx: Single tick volume up");
    case 0xf9:
      return _("F9xx: Single tick volume down");
    case 0xfa:
      return _("FAxx: Fast volume slide (0y: down; x0: up)");
    case 0xfc:
      return _("FCxx: Note release");
    case 0xfd:
      return _("FDxx: Set virtual tempo numerator");
    case 0xfe:
      return _("FExx: Set virtual tempo denominator");
    case 0xff:
      return _("FFxx: Stop song");
    default:
      if ((effect&0xf0)==0x90) {
        if (song.oldSampleOffset) {
          return _("9xxx: Set sample offset*256");
        }
        switch (effect) {
          case 0x90:
            return _("90xx: Set sample offset (first byte)");
          case 0x91:
            return _("91xx: Set sample offset (second byte, ×256)");
          case 0x92:
            return _("92xx: Set sample offset (third byte, ×65536)");
        }
      } else if (chan>=0 && chan<chans) {
        DivSysDef* sysDef=sysDefs[sysOfChan[chan]];
        auto iter=sysDef->effectHandlers.find(effect);
        if (iter!=sysDef->effectHandlers.end()) {
          return iter->second.description;
        }
        iter=sysDef->postEffectHandlers.find(effect);
        if (iter!=sysDef->postEffectHandlers.end()) {
          return iter->second.description;
        }
        iter=sysDef->preEffectHandlers.find(effect);
        if (iter!=sysDef->preEffectHandlers.end()) {
          return iter->second.description;
        }
      }
      break;
  }
  return notNull?_("Invalid effect"):NULL;
}

void DivEngine::walkSong(int& loopOrder, int& loopRow, int& loopEnd) {
  if (curSubSong!=NULL) {
    curSubSong->walk(loopOrder,loopRow,loopEnd,chans,song.jumpTreatment,song.ignoreJumpAtEnd);
  }
}

void DivEngine::findSongLength(int loopOrder, int loopRow, double fadeoutLen, int& rowsForFadeout, bool& hasFFxx, std::vector<int>& orders, int& length) {
  if (curSubSong!=NULL) {
    curSubSong->findLength(loopOrder,loopRow,fadeoutLen,rowsForFadeout,hasFFxx,orders,song.grooves,length,chans,song.jumpTreatment,song.ignoreJumpAtEnd);
  }
}

#define EXPORT_BUFSIZE 2048

double DivEngine::benchmarkPlayback() {
  float* outBuf[2];
  outBuf[0]=new float[EXPORT_BUFSIZE];
  outBuf[1]=new float[EXPORT_BUFSIZE];

  curOrder=0;
  prevOrder=0;
  remainingLoops=1;
  playSub(false);

  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();

  // benchmark
  while (playing) {
    nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
  }

  std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();

  delete[] outBuf[0];
  delete[] outBuf[1];

  double t=(double)(std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count())/1000000.0;
  printf("[RESULT] %fs\n",t);
  return t;
}

double DivEngine::benchmarkSeek() {
  double t[20];
  curOrder=curSubSong->ordersLen-1;
  prevOrder=curSubSong->ordersLen-1;

  // benchmark
  for (int i=0; i<20; i++) {
    std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
    playSub(false);     
    std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();
    t[i]=(double)(std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count())/1000000.0;
    printf("[#%d] %fs\n",i+1,t[i]);
  }

  double tMin=DBL_MAX;
  double tMax=0.0;
  double tAvg=0.0;
  for (int i=0; i<20; i++) {
    if (t[i]<tMin) tMin=t[i];
    if (t[i]>tMax) tMax=t[i];
    tAvg+=t[i];
  }
  tAvg/=20.0;

  printf("[RESULT] min %fs max %fs average %fs\n",tMin,tMax,tAvg);
  return tAvg;
}

void DivEngine::notifyInsChange(int ins) {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyInsChange(ins);
  }
  BUSY_END;
}

void DivEngine::notifyWaveChange(int wave) {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyWaveChange(wave);
  }
  BUSY_END;
}

int DivEngine::loadSampleROM(String path, ssize_t expectedSize, unsigned char*& ret) {
  ret=NULL;
  if (path.empty()) {
    return 0;
  }
  logI("loading ROM %s...",path);
  FILE* f=ps_fopen(path.c_str(),"rb");
  if (f==NULL) {
    logE("error: %s",strerror(errno));
    lastError=strerror(errno);
    return -1;
  }
  if (fseek(f,0,SEEK_END)<0) {
    logE("size error: %s",strerror(errno));
    lastError=fmt::sprintf(_("on seek: %s"),strerror(errno));
    fclose(f);
    return -1;
  }
  ssize_t len=ftell(f);
  if (len==(SIZE_MAX>>1)) {
    logE("could not get file length: %s",strerror(errno));
    lastError=fmt::sprintf(_("on pre tell: %s"),strerror(errno));
    fclose(f);
    return -1;
  }
  if (len<1) {
    if (len==0) {
      logE("that file is empty!");
      lastError=_("file is empty");
    } else {
      logE("tell error: %s",strerror(errno));
      lastError=fmt::sprintf(_("on tell: %s"),strerror(errno));
    }
    fclose(f);
    return -1;
  }
  if (len!=expectedSize) {
    logE("ROM size mismatch, expected: %d bytes, was: %d bytes", expectedSize, len);
    lastError=fmt::sprintf(_("ROM size mismatch, expected: %d bytes, was: %d"), expectedSize, len);
    return -1;
  }
  if (fseek(f,0,SEEK_SET)<0) {
    logE("size error: %s",strerror(errno));
    lastError=fmt::sprintf(_("on get size: %s"),strerror(errno));
    fclose(f);
    return -1;
  }
  unsigned char* file=new unsigned char[len];
  if (fread(file,1,(size_t)len,f)!=(size_t)len) {
    logE("read error: %s",strerror(errno));
    lastError=fmt::sprintf(_("on read: %s"),strerror(errno));
    fclose(f);
    delete[] file;
    return -1;
  }
  fclose(f);
  ret=file;
  return 0;
}

unsigned int DivEngine::getSampleFormatMask() {
  unsigned int formatMask=1U<<16; // 16-bit is always on
  for (int i=0; i<song.systemLen; i++) {
    const DivSysDef* s=getSystemDef(song.system[i]);
    if (s==NULL) continue;
    formatMask|=s->sampleFormatMask;
  }
  return formatMask;
}

int DivEngine::loadSampleROMs() {
  if (yrw801ROM!=NULL) {
    delete[] yrw801ROM;
    yrw801ROM=NULL;
  }
  if (tg100ROM!=NULL) {
    delete[] tg100ROM;
    tg100ROM=NULL;
  }
  if (mu5ROM!=NULL) {
    delete[] mu5ROM;
    mu5ROM=NULL;
  }
  int error=0;
  error+=loadSampleROM(getConfString("yrw801Path",""), 0x200000, yrw801ROM);
  error+=loadSampleROM(getConfString("tg100Path",""), 0x200000, tg100ROM);
  error+=loadSampleROM(getConfString("mu5Path",""), 0x200000, mu5ROM);
  return error;
}

void DivEngine::renderSamplesP(int whichSample) {
  BUSY_BEGIN;
  renderSamples(whichSample);
  BUSY_END;
}

void DivEngine::renderSamples(int whichSample) {
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;

  logD("rendering samples...");

  // step 0: make sample format mask
  unsigned int formatMask=1U<<16; // 16-bit is always on
  for (int i=0; i<song.systemLen; i++) {
    const DivSysDef* s=getSystemDef(song.system[i]);
    if (s==NULL) continue;
    formatMask|=s->sampleFormatMask;
  }

  // step 1: render samples
  if (whichSample==-1) {
    for (int i=0; i<song.sampleLen; i++) {
      song.sample[i]->render(formatMask);
    }
  } else if (whichSample>=0 && whichSample<song.sampleLen) {
    song.sample[whichSample]->render(formatMask);
  }

  // step 2: render samples to dispatch
  for (int i=0; i<song.systemLen; i++) {
    if (disCont[i].dispatch!=NULL) {
      disCont[i].dispatch->renderSamples(i);
    }
  }
}

String DivEngine::decodeSysDesc(String desc) {
  DivConfig newDesc;
  bool hasVal=false;
  bool negative=false;
  int val=0;
  int curStage=0;
  int sysID=0;
  float sysVol=0;
  float sysPan=0;
  int sysFlags=0;
  int curSys=0;
  desc+=' '; // ha
  for (char i: desc) {
    switch (i) {
      case ' ':
        if (hasVal) {
          if (negative) val=-val;
          switch (curStage) {
            case 0:
              sysID=val;
              curStage++;
              break;
            case 1:
              sysVol=(float)val/64.0f;
              curStage++;
              break;
            case 2:
              sysPan=(float)val/127.0f;
              curStage++;
              break;
            case 3:
              sysFlags=val;

              if (sysID!=0) {
                if (sysVol<-1.0f) sysVol=-1.0f;
                if (sysVol>1.0f) sysVol=1.0f;
                if (sysPan<-1.0f) sysPan=-1.0f;
                if (sysPan>1.0f) sysPan=1.0f;
                newDesc.set(fmt::sprintf("id%d",curSys),sysID);
                newDesc.set(fmt::sprintf("vol%d",curSys),sysVol);
                newDesc.set(fmt::sprintf("pan%d",curSys),sysPan);
                newDesc.set(fmt::sprintf("fr%d",curSys),0.0f);
                DivConfig newFlagsC;
                newFlagsC.clear();
                convertOldFlags((unsigned int)sysFlags,newFlagsC,systemFromFileFur(sysID));
                newDesc.set(fmt::sprintf("flags%d",curSys),newFlagsC.toBase64());
                curSys++;
              }

              curStage=0;
              break;
          }
          hasVal=false;
          negative=false;
          val=0;
        }
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        val=(val*10)+(i-'0');
        hasVal=true;
        break;
      case '-':
        if (!hasVal) negative=true;
        break;
    }
  }

  return newDesc.toBase64();
}

void DivEngine::initSongWithDesc(const char* description, bool inBase64, bool oldVol) {
  int chanCount=0;
  DivConfig c;
  if (inBase64) {
    c.loadFromBase64(description);
  } else {
    c.loadFromMemory(description);
  }
  int index=0;
  for (; index<DIV_MAX_CHIPS; index++) {
    song.system[index]=systemFromFileFur(c.getInt(fmt::sprintf("id%d",index),0));
    if (song.system[index]==DIV_SYSTEM_NULL) {
      break;
    }
    chanCount+=getChannelCount(song.system[index]);
    if (chanCount>=DIV_MAX_CHANS) {
      song.system[index]=DIV_SYSTEM_NULL;
      break;
    }
    song.systemVol[index]=c.getFloat(fmt::sprintf("vol%d",index),1.0f);
    song.systemPan[index]=c.getFloat(fmt::sprintf("pan%d",index),0.0f);
    song.systemPanFR[index]=c.getFloat(fmt::sprintf("fr%d",index),0.0f);
    song.systemFlags[index].clear();

    if (oldVol) {
      song.systemVol[index]/=64.0f;
      song.systemPan[index]/=127.0f;
    }

    String flags=c.getString(fmt::sprintf("flags%d",index),"");
    song.systemFlags[index].loadFromBase64(flags.c_str());
  }
  song.systemLen=index;
  
  // extra attributes
  song.subsong[0]->hz=c.getDouble("tickRate",60.0);
  if (song.subsong[0]->hz<1.0) song.subsong[0]->hz=1.0;
  if (song.subsong[0]->hz>999.0) song.subsong[0]->hz=999.0;

  curChanMask=c.getIntList("chanMask",{});
  for (unsigned char i:curChanMask) {
    int j=i-1;
    if (j<0) j=0;
    if (j>DIV_MAX_CHANS) j=DIV_MAX_CHANS-1;
    curSubSong->chanShow[j]=false;
    curSubSong->chanShowChanOsc[j]=false;
  }

  song.author=getConfString("defaultAuthorName","");
}

void DivEngine::createNew(const char* description, String sysName, bool inBase64) {
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.unload();
  song=DivSong();
  changeSong(0);
  if (description!=NULL) {
    initSongWithDesc(description,inBase64);
  }
  if (sysName=="") {
    song.systemName=getSongSystemLegacyName(song,!getConfInt("noMultiSystem",0));
  } else {
    song.systemName=sysName;
  }
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
}

void DivEngine::createNewFromDefaults() {
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.unload();
  song=DivSong();
  changeSong(0);

  String preset=getConfString("initialSys2","");
  bool oldVol=getConfInt("configVersion",DIV_ENGINE_VERSION)<135;
  if (preset.empty()) {
    // try loading old preset
    logD("trying to load old preset");
    preset=decodeSysDesc(getConfString("initialSys",""));
    oldVol=false;
  }
  logD("preset size %ld",preset.size());
  if (preset.size()>0 && (preset.size()&3)==0) {
    initSongWithDesc(preset.c_str(),true,oldVol);
  }
  String sysName=getConfString("initialSysName","");
  if (sysName=="") {
    song.systemName=getSongSystemLegacyName(song,!getConfInt("noMultiSystem",0));
  } else {
    song.systemName=sysName;
  }

  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
}

void DivEngine::swapChannels(int src, int dest) {
  logV("swapping channel %d with %d",src,dest);
  if (src==dest) {
    logV("not swapping channels because it's the same channel!",src,dest);
    return;
  }

  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    curOrders->ord[dest][i]^=curOrders->ord[src][i];
    curOrders->ord[src][i]^=curOrders->ord[dest][i];
    curOrders->ord[dest][i]^=curOrders->ord[src][i];

    DivPattern* prev=curPat[src].data[i];
    curPat[src].data[i]=curPat[dest].data[i];
    curPat[dest].data[i]=prev;
  }

  curPat[src].effectCols^=curPat[dest].effectCols;
  curPat[dest].effectCols^=curPat[src].effectCols;
  curPat[src].effectCols^=curPat[dest].effectCols;

  String prevChanName=curSubSong->chanName[src];
  String prevChanShortName=curSubSong->chanShortName[src];
  bool prevChanShow=curSubSong->chanShow[src];
  bool prevChanShowChanOsc=curSubSong->chanShowChanOsc[src];
  unsigned char prevChanCollapse=curSubSong->chanCollapse[src];

  curSubSong->chanName[src]=curSubSong->chanName[dest];
  curSubSong->chanShortName[src]=curSubSong->chanShortName[dest];
  curSubSong->chanShow[src]=curSubSong->chanShow[dest];
  curSubSong->chanShowChanOsc[src]=curSubSong->chanShowChanOsc[dest];
  curSubSong->chanCollapse[src]=curSubSong->chanCollapse[dest];
  curSubSong->chanName[dest]=prevChanName;
  curSubSong->chanShortName[dest]=prevChanShortName;
  curSubSong->chanShow[dest]=prevChanShow;
  curSubSong->chanShowChanOsc[dest]=prevChanShowChanOsc;
  curSubSong->chanCollapse[dest]=prevChanCollapse;
}

void DivEngine::stompChannel(int ch) {
  logV("stomping channel %d",ch);
  for (int i=0; i<DIV_MAX_PATTERNS; i++) {
    curOrders->ord[ch][i]=0;
  }
  curPat[ch].wipePatterns();
  curPat[ch].effectCols=1;
  curSubSong->chanName[ch]="";
  curSubSong->chanShortName[ch]="";
  curSubSong->chanShow[ch]=true;
  curSubSong->chanShowChanOsc[ch]=true;
  curSubSong->chanCollapse[ch]=false;
}

void DivEngine::changeSong(size_t songIndex) {
  if (songIndex>=song.subsong.size()) return;
  curSubSong=song.subsong[songIndex];
  curPat=song.subsong[songIndex]->pat;
  curOrders=&song.subsong[songIndex]->orders;
  curSubSongIndex=songIndex;
  curOrder=0;
  curRow=0;
  prevOrder=0;
  prevRow=0;
}

void DivEngine::moveAsset(std::vector<DivAssetDir>& dir, int before, int after) {
  if (before<0 || after<0) return;
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase matching entry
      if (i.entries[j]==before) {
        i.entries[j]=after;
      } else if (i.entries[j]==after) {
        i.entries[j]=before;
      }
    }
  }
}

void DivEngine::removeAsset(std::vector<DivAssetDir>& dir, int entry) {
  if (entry<0) return;
  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase matching entry
      if (i.entries[j]==entry) {
        i.entries.erase(i.entries.begin()+j);
        j--;
      } else if (i.entries[j]>entry) {
        i.entries[j]--;
      }
    }
  }
}

void DivEngine::checkAssetDir(std::vector<DivAssetDir>& dir, size_t entries) {
  bool* inAssetDir=new bool[entries];
  memset(inAssetDir,0,entries*sizeof(bool));

  for (DivAssetDir& i: dir) {
    for (size_t j=0; j<i.entries.size(); j++) {
      // erase invalid entry
      if (i.entries[j]<0 || i.entries[j]>=(int)entries) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }

      // erase duplicate entry
      if (inAssetDir[i.entries[j]]) {
        i.entries.erase(i.entries.begin()+j);
        j--;
        continue;
      }
      
      // mark entry as present
      inAssetDir[i.entries[j]]=true;
    }
  }

  // get unsorted directory
  DivAssetDir* unsortedDir=NULL;
  for (DivAssetDir& i: dir) {
    if (i.name.empty()) {
      unsortedDir=&i;
      break;
    }
  }

  // add missing items to unsorted directory
  for (size_t i=0; i<entries; i++) {
    if (!inAssetDir[i]) {
      // create unsorted directory if it doesn't exist
      if (unsortedDir==NULL) {
        dir.push_back(DivAssetDir(""));
        unsortedDir=&(*dir.rbegin());
      }
      unsortedDir->entries.push_back(i);
    }
  }

  delete[] inAssetDir;
}

void DivEngine::swapChannelsP(int src, int dest) {
  if (src<0 || src>=chans) return;
  if (dest<0 || dest>=chans) return;
  BUSY_BEGIN;
  saveLock.lock();
  swapChannels(src,dest);
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::changeSongP(size_t index) {
  if (index>=song.subsong.size()) return;
  if (index==curSubSongIndex) return;
  stop();
  BUSY_BEGIN;
  saveLock.lock();
  changeSong(index);
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addSubSong() {
  if (song.subsong.size()>=127) return -1;
  BUSY_BEGIN;
  saveLock.lock();
  song.subsong.push_back(new DivSubSong);
  for (unsigned char i:curChanMask) {
    int j=i-1;
    if (j<0) j=0;
    if (j>DIV_MAX_CHANS) j=DIV_MAX_CHANS-1;
    song.subsong.back()->chanShow[j]=false;
    song.subsong.back()->chanShowChanOsc[j]=false;
  }
  saveLock.unlock();
  BUSY_END;
  return song.subsong.size()-1;
}

int DivEngine::duplicateSubSong(int index) {
  if (song.subsong.size()>=127) return -1;
  BUSY_BEGIN;
  saveLock.lock();
  DivSubSong* theCopy=new DivSubSong;
  DivSubSong* theOrig=song.subsong[index];

  theCopy->name=theOrig->name;
  theCopy->notes=theOrig->notes;
  theCopy->hilightA=theOrig->hilightA;
  theCopy->hilightB=theOrig->hilightB;
  theCopy->timeBase=theOrig->timeBase;
  theCopy->arpLen=theOrig->arpLen;
  theCopy->speeds=theOrig->speeds;
  theCopy->virtualTempoN=theOrig->virtualTempoN;
  theCopy->virtualTempoD=theOrig->virtualTempoD;
  theCopy->hz=theOrig->hz;
  theCopy->patLen=theOrig->patLen;
  theCopy->ordersLen=theOrig->ordersLen;
  theCopy->orders=theOrig->orders;
  
  memcpy(theCopy->chanShow,theOrig->chanShow,DIV_MAX_CHANS*sizeof(bool));
  memcpy(theCopy->chanShowChanOsc,theOrig->chanShowChanOsc,DIV_MAX_CHANS*sizeof(bool));
  memcpy(theCopy->chanCollapse,theOrig->chanCollapse,DIV_MAX_CHANS);

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    theCopy->chanName[i]=theOrig->chanName[i];
    theCopy->chanShortName[i]=theOrig->chanShortName[i];

    theCopy->pat[i].effectCols=theOrig->pat[i].effectCols;

    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      if (theOrig->pat[i].data[j]==NULL) continue;
      DivPattern* origPat=theOrig->pat[i].getPattern(j,false);
      DivPattern* copyPat=theCopy->pat[i].getPattern(j,true);
      origPat->copyOn(copyPat);
    }
  }

  song.subsong.push_back(theCopy);
  
  saveLock.unlock();
  BUSY_END;
  return song.subsong.size()-1;
}

bool DivEngine::removeSubSong(int index) {
  if (song.subsong.size()<=1) return false;
  stop();
  BUSY_BEGIN;
  saveLock.lock();
  song.subsong[index]->clearData();
  delete song.subsong[index];
  song.subsong.erase(song.subsong.begin()+index);
  changeSong(0);
  saveLock.unlock();
  BUSY_END;
  return true;
}

void DivEngine::moveSubSongUp(size_t index) {
  if (index<1 || index>=song.subsong.size()) return;
  BUSY_BEGIN;
  saveLock.lock();

  if (index==curSubSongIndex) {
    curSubSongIndex--;
  } else if (index-1==curSubSongIndex) {
    curSubSongIndex++;
  }

  DivSubSong* prev=song.subsong[index-1];
  song.subsong[index-1]=song.subsong[index];
  song.subsong[index]=prev;

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::moveSubSongDown(size_t index) {
  if (index>=song.subsong.size()-1) return;
  BUSY_BEGIN;
  saveLock.lock();

  if (index==curSubSongIndex) {
    curSubSongIndex++;
  } else if (index+1==curSubSongIndex) {
    curSubSongIndex--;
  }

  DivSubSong* prev=song.subsong[index+1];
  song.subsong[index+1]=song.subsong[index];
  song.subsong[index]=prev;

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::clearSubSongs() {
  BUSY_BEGIN;
  saveLock.lock();
  song.clearSongData();
  changeSong(0);
  curOrder=0;
  prevOrder=0;
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::delUnusedIns() {
  BUSY_BEGIN;
  saveLock.lock();

  bool isUsed[256];
  memset(isUsed,0,256*sizeof(bool));

  // scan
  for (int i=0; i<chans; i++) {
    for (size_t j=0; j<song.subsong.size(); j++) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (song.subsong[j]->pat[i].data[k]==NULL) continue;
        for (int l=0; l<song.subsong[j]->patLen; l++) {
          if (song.subsong[j]->pat[i].data[k]->data[l][2]>=0 && song.subsong[j]->pat[i].data[k]->data[l][2]<256) {
            isUsed[song.subsong[j]->pat[i].data[k]->data[l][2]]=true;
          }
        }
      }
    }
  }
  
  // delete
  for (int i=0; i<song.insLen; i++) {
    if (!isUsed[i]) {
      delInstrumentUnsafe(i);
      // rotate
      for (int j=i; j<255; j++) {
        isUsed[j]=isUsed[j+1];
      }
      isUsed[255]=true;
      i--;
    }
  }

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::delUnusedWaves() {
  BUSY_BEGIN;
  saveLock.lock();

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::delUnusedSamples() {
  if (song.sample.empty()) return;

  BUSY_BEGIN;
  saveLock.lock();

  bool* isUsed=new bool[song.sample.size()];
  memset(isUsed,0,song.sample.size()*sizeof(bool));
  int isUsedMax=((int)song.sample.size())-1;

  // scan in instruments
  for (DivInstrument* i: song.ins) {
    if ((i->type==DIV_INS_PCE && i->amiga.useSample) ||
        i->type==DIV_INS_MSM6258 ||
        i->type==DIV_INS_MSM6295 ||
        i->type==DIV_INS_ADPCMA ||
        i->type==DIV_INS_ADPCMB ||
        i->type==DIV_INS_SEGAPCM ||
        i->type==DIV_INS_QSOUND ||
        i->type==DIV_INS_YMZ280B ||
        i->type==DIV_INS_RF5C68 ||
        i->type==DIV_INS_AMIGA ||
        i->type==DIV_INS_MULTIPCM ||
        (i->type==DIV_INS_MIKEY && i->amiga.useSample) ||
        (i->type==DIV_INS_X1_010 && i->amiga.useSample) ||
        (i->type==DIV_INS_SWAN && i->amiga.useSample) ||
        (i->type==DIV_INS_AY && i->amiga.useSample) ||
        (i->type==DIV_INS_AY8930 && i->amiga.useSample) ||
        (i->type==DIV_INS_VRC6 && i->amiga.useSample) ||
        (i->type==DIV_INS_SU && i->amiga.useSample) ||
        i->type==DIV_INS_SNES ||
        i->type==DIV_INS_ES5506 ||
        i->type==DIV_INS_K007232 ||
        i->type==DIV_INS_GA20 ||
        i->type==DIV_INS_K053260 ||
        i->type==DIV_INS_C140 ||
        i->type==DIV_INS_C219 ||
        i->type==DIV_INS_NDS ||
        i->type==DIV_INS_GBA_DMA ||
        i->type==DIV_INS_GBA_MINMOD) {
      if (i->amiga.initSample>=0 && i->amiga.initSample<song.sampleLen) {
        isUsed[i->amiga.initSample]=true;
      }
      if (i->amiga.useNoteMap) {
        for (int j=0; j<120; j++) {
          if (i->amiga.noteMap[j].map>=0 && i->amiga.noteMap[j].map<song.sampleLen) {
            isUsed[i->amiga.noteMap[j].map]=true;
          }
        }
      }
    }
  }

  // scan in pattern (legacy sample mode)
  // disabled because it is unreliable
  /*
  for (DivSubSong* i: song.subsong) {
    for (int j=0; j<getTotalChannelCount(); j++) {
      bool is17On=false;
      int bank=0;
      for (int k=0; k<i->ordersLen; k++) {
        DivPattern* p=i->pat[j].getPattern(i->orders.ord[j][k],false);
        for (int l=0; l<i->patLen; l++) {
          for (int m=0; m<i->pat[j].effectCols; m++) {
            if (p->data[l][4+(m<<1)]==0x17) {
              is17On=(p->data[l][5+(m<<1)]>0);
            }
            if (p->data[l][4+(m<<1)]==0xeb) {
              bank=p->data[l][5+(m<<1)];
              if (bank==-1) bank=0;
            }
          }
          if (is17On) {
            if (p->data[l][1]!=0 || p->data[l][0]!=0) {
              if (p->data[l][0]<=12) {
                int note=(12*bank)+(p->data[l][0]%12);
                if (note<256) isUsed[note]=true;
              }
            }
          }
        }
      }
    }
  }*/

  // delete
  for (int i=0; i<song.sampleLen; i++) {
    if (!isUsed[i]) {
      delSampleUnsafe(i,false);
      // rotate
      for (int j=i; j<isUsedMax; j++) {
        isUsed[j]=isUsed[j+1];
      }
      isUsed[isUsedMax]=true;
      i--;
    }
  }

  // render
  renderSamples();

  delete[] isUsed;

  saveLock.unlock();
  BUSY_END;
}

bool DivEngine::changeSystem(int index, DivSystem which, bool preserveOrder) {
  if (index<0 || index>=song.systemLen) {
    lastError=_("invalid index");
    return false;
  }
  if (chans-getChannelCount(song.system[index])+getChannelCount(which)>DIV_MAX_CHANS) {
    lastError=fmt::sprintf(_("max number of total channels is %d"),DIV_MAX_CHANS);
    return false;
  }

  int chanCount=chans;
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();

  if (!preserveOrder) {
    int firstChan=0;
    int chanMovement=getChannelCount(which)-getChannelCount(song.system[index]);
    while (dispatchOfChan[firstChan]!=index) firstChan++;
    int lastChan=firstChan+getChannelCount(song.system[index]);
    if (chanMovement!=0) {
      if (chanMovement>0) {
        // add channels
        for (int i=chanCount+chanMovement-1; i>=lastChan+chanMovement; i--) {
          swapChannels(i,i-chanMovement);
        }
        for (int i=lastChan; i<lastChan+chanMovement; i++) {
          stompChannel(i);
        }
      } else {
        // remove channels
        for (int i=lastChan+chanMovement; i<lastChan; i++) {
          stompChannel(i);
        }
        for (int i=lastChan+chanMovement; i<chanCount+chanMovement; i++) {
          swapChannels(i,i-chanMovement);
        }
      }
    }
  }

  song.system[index]=which;
  song.systemFlags[index].clear();
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;

  return true;
}

bool DivEngine::addSystem(DivSystem which) {
  if (song.systemLen>=DIV_MAX_CHIPS) {
    lastError=fmt::sprintf(_("max number of systems is %d"),DIV_MAX_CHIPS);
    return false;
  }
  if (chans+getChannelCount(which)>DIV_MAX_CHANS) {
    lastError=fmt::sprintf(_("max number of total channels is %d"),DIV_MAX_CHANS);
    return false;
  }
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.system[song.systemLen]=which;
  song.systemVol[song.systemLen]=1.0;
  song.systemPan[song.systemLen]=0;
  song.systemPanFR[song.systemLen]=0;
  song.systemFlags[song.systemLen++].clear();
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  if (song.patchbayAuto) {
    autoPatchbay();
  } else {
    int i=song.systemLen-1;
    if (disCont[i].dispatch!=NULL) {
      unsigned int outs=disCont[i].dispatch->getOutputCount();
      if (outs>16) outs=16;
      if (outs<2) {
        song.patchbay.reserve(DIV_MAX_OUTPUTS);
        for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
          song.patchbay.push_back((i<<20)|j);
        }
      } else {
        if (outs>0) song.patchbay.reserve(outs);
        for (unsigned int j=0; j<outs; j++) {
          song.patchbay.push_back((i<<20)|(j<<16)|j);
        }
      }
    }
  }
  saveLock.unlock();
  renderSamples();
  reset();
  BUSY_END;
  return true;
}

bool DivEngine::duplicateSystem(int index, bool pat, bool end) {
  if (index<0 || index>=song.systemLen) {
    lastError=_("invalid index");
    return false;
  }
  if (song.systemLen>=DIV_MAX_CHIPS) {
    lastError=fmt::sprintf(_("max number of systems is %d"),DIV_MAX_CHIPS);
    return false;
  }
  if (chans+getChannelCount(song.system[index])>DIV_MAX_CHANS) {
    lastError=fmt::sprintf(_("max number of total channels is %d"),DIV_MAX_CHANS);
    return false;
  }
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.system[song.systemLen]=song.system[index];
  song.systemVol[song.systemLen]=song.systemVol[index];
  song.systemPan[song.systemLen]=song.systemPan[index];
  song.systemPanFR[song.systemLen]=song.systemPanFR[index];
  song.systemFlags[song.systemLen++]=song.systemFlags[index];
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  if (song.patchbayAuto) {
    autoPatchbay();
  } else {
    int i=song.systemLen-1;
    if (disCont[i].dispatch!=NULL) {
      unsigned int outs=disCont[i].dispatch->getOutputCount();
      if (outs>16) outs=16;
      if (outs<2) {
        song.patchbay.reserve(DIV_MAX_OUTPUTS);
        for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
          song.patchbay.push_back((i<<20)|j);
        }
      } else {
        if (outs>0) song.patchbay.reserve(outs);
        for (unsigned int j=0; j<outs; j++) {
          song.patchbay.push_back((i<<20)|(j<<16)|j);
        }
      }
    }
  }

  // duplicate patterns
  if (pat) {
    int srcChan=0;
    int destChan=0;
    for (int i=0; i<index; i++) {
      srcChan+=getChannelCount(song.system[i]);
    }
    for (int i=0; i<song.systemLen-1; i++) {
      destChan+=getChannelCount(song.system[i]);
    }
    for (DivSubSong* i: song.subsong) {
      for (int j=0; j<getChannelCount(song.system[index]); j++) {
        i->pat[destChan+j].effectCols=i->pat[srcChan+j].effectCols;
        i->chanShow[destChan+j]=i->chanShow[srcChan+j];
        i->chanShowChanOsc[destChan+j]=i->chanShowChanOsc[srcChan+j];
        i->chanCollapse[destChan+j]=i->chanCollapse[srcChan+j];
        i->chanName[destChan+j]=i->chanName[srcChan+j];
        i->chanShortName[destChan+j]=i->chanShortName[srcChan+j];
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (i->pat[srcChan+j].data[k]!=NULL) {
            i->pat[srcChan+j].data[k]->copyOn(i->pat[destChan+j].getPattern(k,true));
          }
        }

        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          i->orders.ord[destChan+j][k]=i->orders.ord[srcChan+j][k];
        }
      }
    }
  }
  saveLock.unlock();
  renderSamples();
  reset();
  BUSY_END;

  if (!end) {
    quitDispatch();
    BUSY_BEGIN;
    saveLock.lock();

    for (int i=song.systemLen-1; i>index; i--) {
      swapSystemUnsafe(i,i-1,false);
    }

    recalcChans();
    saveLock.unlock();
    BUSY_END;
    initDispatch();
    BUSY_BEGIN;
    renderSamples();
    reset();
    BUSY_END;
  }
  return true;
}

// TODO: maybe issue with subsongs?
bool DivEngine::removeSystem(int index, bool preserveOrder) {
  if (song.systemLen<=1) {
    lastError=_("cannot remove the last one");
    return false;
  }
  if (index<0 || index>=song.systemLen) {
    lastError=_("invalid index");
    return false;
  }
  int chanCount=chans;
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();

  if (!preserveOrder) {
    int firstChan=0;
    while (dispatchOfChan[firstChan]!=index) firstChan++;
    for (int i=0; i<getChannelCount(song.system[index]); i++) {
      stompChannel(i+firstChan);
    }
    for (int i=firstChan+getChannelCount(song.system[index]); i<chanCount; i++) {
      swapChannels(i,i-getChannelCount(song.system[index]));
    }
  }

  // patchbay
  for (size_t i=0; i<song.patchbay.size(); i++) {
    if (((song.patchbay[i]>>20)&0xfff)==(unsigned int)index) {
      song.patchbay.erase(song.patchbay.begin()+i);
      i--;
    }
  }

  song.system[index]=DIV_SYSTEM_NULL;
  song.systemLen--;
  for (int i=index; i<song.systemLen; i++) {
    song.system[i]=song.system[i+1];
    song.systemVol[i]=song.systemVol[i+1];
    song.systemPan[i]=song.systemPan[i+1];
    song.systemPanFR[i]=song.systemPanFR[i+1];
    song.systemFlags[i]=song.systemFlags[i+1];
  }
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
  return true;
}

void DivEngine::swapSystemUnsafe(int src, int dest, bool preserveOrder) {
  if (!preserveOrder) {
    // move channels
    unsigned char unswappedChannels[DIV_MAX_CHANS];
    unsigned char swappedChannels[DIV_MAX_CHANS];
    std::vector<std::vector<int>> swapList;
    std::vector<int> chanList;

    int tchans=0;

    for (int i=0; i<song.systemLen; i++) {
      tchans+=getChannelCount(song.system[i]);
    }

    memset(unswappedChannels,0,DIV_MAX_CHANS);
    memset(swappedChannels,0,DIV_MAX_CHANS);
    
    for (int i=0; i<tchans; i++) {
      unswappedChannels[i]=i;
    }

    // prepare swap list
    int index=0;
    if (song.systemLen>0) swapList.reserve(song.systemLen);
    for (int i=0; i<song.systemLen; i++) {
      chanList.clear();
      const int channelCount=getChannelCount(song.system[i]);
      if (channelCount>0) chanList.reserve(channelCount);
      for (int j=0; j<channelCount; j++) {
        chanList.push_back(index);
        index++;
      }
      swapList.push_back(chanList);
    }
    swapList[src].swap(swapList[dest]);

    // unfold it
    index=0;
    for (std::vector<int>& i: swapList) {
      for (int& j: i) {
        swappedChannels[index++]=j;
      }
    }

    // swap channels
    logV("swap list:");
    for (int i=0; i<tchans; i++) {
      logV("- %d -> %d",unswappedChannels[i],swappedChannels[i]);
    }

    for (size_t i=0; i<song.subsong.size(); i++) {
      DivOrders prevOrders=song.subsong[i]->orders;
      DivPattern* prevPat[DIV_MAX_CHANS][DIV_MAX_PATTERNS];
      unsigned char prevEffectCols[DIV_MAX_CHANS];
      String prevChanName[DIV_MAX_CHANS];
      String prevChanShortName[DIV_MAX_CHANS];
      bool prevChanShow[DIV_MAX_CHANS];
      bool prevChanShowChanOsc[DIV_MAX_CHANS];
      unsigned char prevChanCollapse[DIV_MAX_CHANS];

      for (int j=0; j<tchans; j++) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          prevPat[j][k]=song.subsong[i]->pat[j].data[k];
        }
        prevEffectCols[j]=song.subsong[i]->pat[j].effectCols;

        prevChanName[j]=song.subsong[i]->chanName[j];
        prevChanShortName[j]=song.subsong[i]->chanShortName[j];
        prevChanShow[j]=song.subsong[i]->chanShow[j];
        prevChanShowChanOsc[j]=song.subsong[i]->chanShowChanOsc[j];
        prevChanCollapse[j]=song.subsong[i]->chanCollapse[j];
      }

      for (int j=0; j<tchans; j++) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          song.subsong[i]->orders.ord[j][k]=prevOrders.ord[swappedChannels[j]][k];
          song.subsong[i]->pat[j].data[k]=prevPat[swappedChannels[j]][k];
        }

        song.subsong[i]->pat[j].effectCols=prevEffectCols[swappedChannels[j]];
        song.subsong[i]->chanName[j]=prevChanName[swappedChannels[j]];
        song.subsong[i]->chanShortName[j]=prevChanShortName[swappedChannels[j]];
        song.subsong[i]->chanShow[j]=prevChanShow[swappedChannels[j]];
        song.subsong[i]->chanShowChanOsc[j]=prevChanShowChanOsc[swappedChannels[j]];
        song.subsong[i]->chanCollapse[j]=prevChanCollapse[swappedChannels[j]];
      }
    }
  }

  DivSystem srcSystem=song.system[src];
  float srcVol=song.systemVol[src];
  float srcPan=song.systemPan[src];
  float srcPanFR=song.systemPanFR[src];

  song.system[src]=song.system[dest];
  song.system[dest]=srcSystem;

  song.systemVol[src]=song.systemVol[dest];
  song.systemVol[dest]=srcVol;

  song.systemPan[src]=song.systemPan[dest];
  song.systemPan[dest]=srcPan;

  song.systemPanFR[src]=song.systemPanFR[dest];
  song.systemPanFR[dest]=srcPanFR;

  // I am kinda scared to use std::swap
  DivConfig oldFlags=song.systemFlags[src];
  song.systemFlags[src]=song.systemFlags[dest];
  song.systemFlags[dest]=oldFlags;

  // patchbay
  for (unsigned int& i: song.patchbay) {
    if (((i>>20)&0xfff)==(unsigned int)src) {
      i=(i&(~0xfff00000))|((unsigned int)dest<<20);
    } else if (((i>>20)&0xfff)==(unsigned int)dest) {
      i=(i&(~0xfff00000))|((unsigned int)src<<20);
    }
  }
}

bool DivEngine::swapSystem(int src, int dest, bool preserveOrder) {
  if (src==dest) {
    lastError=_("source and destination are equal");
    return false;
  }
  if (src<0 || src>=song.systemLen) {
    lastError=_("invalid source index");
    return false;
  }
  if (dest<0 || dest>=song.systemLen) {
    lastError=_("invalid destination index");
    return false;
  }
  //int chanCount=chans;
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();

  swapSystemUnsafe(src,dest,preserveOrder);

  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
  return true;
}

void DivEngine::poke(int sys, unsigned int addr, unsigned short val) {
  if (sys<0 || sys>=song.systemLen) return;
  BUSY_BEGIN;
  disCont[sys].dispatch->poke(addr,val);
  BUSY_END;
}

void DivEngine::poke(int sys, std::vector<DivRegWrite>& wlist) {
  if (sys<0 || sys>=song.systemLen) return;
  BUSY_BEGIN;
  disCont[sys].dispatch->poke(wlist);
  BUSY_END;
}

String DivEngine::getLastError() {
  return lastError;
}

String DivEngine::getWarnings() {
  return warnings;
}

String DivEngine::getPlaybackDebugInfo() {
  return fmt::sprintf(
    "curOrder: %d\n"
    "prevOrder: %d\n"
    "curRow: %d\n"
    "prevRow: %d\n"
    "ticks: %d\n"
    "subticks: %d\n"
    "totalLoops: %d\n"
    "lastLoopPos: %d\n"
    "nextSpeed: %d\n"
    "divider: %f\n"
    "cycles: %d\n"
    "clockDrift: %f\n"
    "midiClockCycles: %d\n"
    "midiClockDrift: %f\n"
    "midiTimeCycles: %d\n"
    "midiTimeDrift: %f\n"
    "changeOrd: %d\n"
    "changePos: %d\n"
    "totalSeconds: %d\n"
    "totalTicks: %d\n"
    "totalTicksR: %d\n"
    "curMidiClock: %d\n"
    "curMidiTime: %d\n"
    "totalCmds: %d\n"
    "lastCmds: %d\n"
    "cmdsPerSecond: %d\n"
    "globalPitch: %d\n"
    "extValue: %d\n"
    "tempoAccum: %d\n"
    "totalProcessed: %d\n"
    "bufferPos: %d\n",
    curOrder,prevOrder,curRow,prevRow,ticks,subticks,totalLoops,lastLoopPos,nextSpeed,divider,cycles,clockDrift,
    midiClockCycles,midiClockDrift,midiTimeCycles,midiTimeDrift,changeOrd,changePos,totalSeconds,totalTicks,
    totalTicksR,curMidiClock,curMidiTime,totalCmds,lastCmds,cmdsPerSecond,globalPitch,
    (int)extValue,(int)tempoAccum,(int)totalProcessed,(int)bufferPos
  );
}

DivInstrument* DivEngine::getIns(int index, DivInstrumentType fallbackType) {
  if (index==-2 && tempIns!=NULL) {
    return tempIns;
  }
  if (index<0 || index>=song.insLen) {
    switch (fallbackType) {
      case DIV_INS_OPLL:
        return &song.nullInsOPLL;
        break;
      case DIV_INS_OPL:
        return &song.nullInsOPL;
        break;
      case DIV_INS_OPL_DRUMS:
        return &song.nullInsOPLDrums;
        break;
      case DIV_INS_ESFM:
        return &song.nullInsESFM;
        break;
      default:
        break;
    }
    return &song.nullIns;
  }
  return song.ins[index];
}

DivWavetable* DivEngine::getWave(int index) {
  if (index<0 || index>=song.waveLen) {
    if (song.waveLen>0) {
      return song.wave[0];
    } else {
      return &song.nullWave;
    }
  }
  return song.wave[index];
}

DivSample* DivEngine::getSample(int index) {
  if (index<0 || index>=song.sampleLen) return &song.nullSample;
  return song.sample[index];
}

DivDispatch* DivEngine::getDispatch(int index) {
  if (index<0 || index>=song.systemLen) return NULL;
  return disCont[index].dispatch;
}

void DivEngine::setLoops(int loops) {
  remainingLoops=loops;
}

DivChannelState* DivEngine::getChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return &chan[ch];
}

unsigned short DivEngine::getChanPan(int ch) {
  if (ch<0 || ch>=chans) return 0;
  return disCont[dispatchOfChan[ch]].dispatch->getPan(dispatchChanOfChan[ch]);
}

void* DivEngine::getDispatchChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return disCont[dispatchOfChan[ch]].dispatch->getChanState(dispatchChanOfChan[ch]);
}

void DivEngine::getChanPaired(int ch, std::vector<DivChannelPair>& ret) {
  if (ch<0 || ch>=chans) return;
  disCont[dispatchOfChan[ch]].dispatch->getPaired(dispatchChanOfChan[ch],ret);
}

DivChannelModeHints DivEngine::getChanModeHints(int ch) {
  if (ch<0 || ch>=chans) return DivChannelModeHints();
  return disCont[dispatchOfChan[ch]].dispatch->getModeHints(dispatchChanOfChan[ch]);
}

unsigned char* DivEngine::getRegisterPool(int sys, int& size, int& depth) {
  if (sys<0 || sys>=song.systemLen) return NULL;
  if (disCont[sys].dispatch==NULL) return NULL;
  size=disCont[sys].dispatch->getRegisterPoolSize();
  depth=disCont[sys].dispatch->getRegisterPoolDepth();
  return disCont[sys].dispatch->getRegisterPool();
}

DivMacroInt* DivEngine::getMacroInt(int chan) {
  if (chan<0 || chan>=chans) return NULL;
  return disCont[dispatchOfChan[chan]].dispatch->getChanMacroInt(dispatchChanOfChan[chan]);
}

DivSamplePos DivEngine::getSamplePos(int chan) {
  if (chan<0 || chan>=chans) return DivSamplePos();
  return disCont[dispatchOfChan[chan]].dispatch->getSamplePos(dispatchChanOfChan[chan]);
}

DivDispatchOscBuffer* DivEngine::getOscBuffer(int chan) {
  if (chan<0 || chan>=chans) return NULL;
  return disCont[dispatchOfChan[chan]].dispatch->getOscBuffer(dispatchChanOfChan[chan]);
}

void DivEngine::enableCommandStream(bool enable) {
  cmdStreamEnabled=enable;
}

void DivEngine::getCommandStream(std::vector<DivCommand>& where) {
  BUSY_BEGIN;
  where.clear();
  where.reserve(cmdStream.size());
  for (DivCommand& i: cmdStream) {
    where.push_back(i);
  }
  cmdStream.clear();
  BUSY_END;
}

void DivEngine::playSub(bool preserveDrift, int goalRow) {
  logV("playSub() called");
  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  reset();
  if (preserveDrift && curOrder==0) {
    logV("preserveDrift && curOrder is true");
    return;
  }
  bool oldRepeatPattern=repeatPattern;
  repeatPattern=false;
  int goal=curOrder;
  curOrder=0;
  curRow=0;
  prevOrder=0;
  prevRow=0;
  stepPlay=0;
  if (curSubSong!=NULL) curSubSong->arpLen=1;
  int prevDrift, prevMidiClockDrift, prevMidiTimeDrift;
  prevDrift=clockDrift;
  prevMidiClockDrift=midiClockDrift;
  prevMidiTimeDrift=midiTimeDrift;
  clockDrift=0;
  cycles=0;
  midiClockCycles=0;
  midiClockDrift=0;
  midiTimeCycles=0;
  midiTimeDrift=0;
  if (!preserveDrift) {
    ticks=1;
    tempoAccum=0;
    totalTicks=0;
    totalTicksOff=0;
    totalSeconds=0;
    totalTicksR=0;
    curMidiClock=0;
    curMidiTime=0;
    curMidiTimeCode=0;
    curMidiTimePiece=0;
    totalLoops=0;
    lastLoopPos=-1;
  }
  endOfSong=false;
  // whaaaaa?
  curSpeed=0;
  playing=true;
  skipping=true;
  memset(walked,0,8192);
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(true);
  logV("goal: %d goalRow: %d",goal,goalRow);
  while (playing && curOrder<goal) {
    if (nextTick(preserveDrift)) {
      skipping=false;
      cmdStream.clear();
      for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
      if (goal>0 || goalRow>0) {
        for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->forceIns();
      }
      return;
    }
    if (!preserveDrift) {
      runMidiClock(cycles);
      runMidiTime(cycles);
    }
  }
  int oldOrder=curOrder;
  while (playing && (curRow<goalRow || ticks>1)) {
    if (nextTick(preserveDrift)) {
      skipping=false;
      cmdStream.clear();
      for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
      if (goal>0 || goalRow>0) {
        for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->forceIns();
      }
      return;
    }
    if (!preserveDrift) {
      runMidiClock(cycles);
      runMidiTime(cycles);
    }
    if (oldOrder!=curOrder) break;
    if (ticks-((tempoAccum+virtualTempoN)/MAX(1,virtualTempoD))<1 && curRow>=goalRow) break;
  }
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  if (goal>0 || goalRow>0) {
    for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->forceIns();
  }
  for (int i=0; i<chans; i++) {
    chan[i].cut=-1;
    chan[i].cutType=0;
  }
  repeatPattern=oldRepeatPattern;
  if (preserveDrift) {
    clockDrift=prevDrift;
    midiClockDrift=prevMidiClockDrift;
    midiTimeDrift=prevMidiTimeDrift;
  } else {
    clockDrift=0;
    cycles=0;
    midiClockCycles=0;
    midiClockDrift=0;
    midiTimeCycles=0;
    midiTimeDrift=0;
    if (curMidiTime>0) {
      curMidiTime--;
    }
    if (curMidiClock>0) {
      curMidiClock--;
    }
    curMidiTimePiece=0;
  }
  if (!preserveDrift) {
    ticks=1;
    subticks=1;
    prevOrder=curOrder;
    prevRow=curRow;
    prevSpeed=nextSpeed;
    tempoAccum=0;
  }
  skipping=false;
  cmdStream.clear();
  std::chrono::high_resolution_clock::time_point timeEnd=std::chrono::high_resolution_clock::now();
  logV("playSub() took %dµs",std::chrono::duration_cast<std::chrono::microseconds>(timeEnd-timeStart).count());
}

/*
int DivEngine::calcBaseFreq(double clock, double divider, int note, bool period) {
  double base=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(note+3)/12.0);
  return period?
         round((clock/base)/divider):
         base*(divider/clock);
}*/

double DivEngine::calcBaseFreq(double clock, double divider, int note, bool period) {
  if (song.linearPitch==2) { // full linear
    return (note<<7);
  }
  double base=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(note+3)/12.0);
  return period?
         (clock/base)/divider:
         base*(divider/clock);
}

#define CONVERT_FNUM_BLOCK(bf,bits,note) \
  double tuning=song.tuning; \
  if (tuning<400.0) tuning=400.0; \
  if (tuning>500.0) tuning=500.0; \
  int boundaryBottom=tuning*pow(2.0,0.25)*(divider/clock); \
  int boundaryTop=2.0*tuning*pow(2.0,0.25)*(divider/clock); \
  while (boundaryTop>((1<<bits)-1)) { \
    boundaryTop>>=1; \
    boundaryBottom>>=1; \
  } \
  int block=(note)/12; \
  if (block<0) block=0; \
  if (block>7) block=7; \
  bf>>=block; \
  if (bf<0) bf=0; \
  /* octave boundaries */ \
  while (bf>0 && bf<boundaryBottom && block>0) { \
    bf<<=1; \
    block--; \
  } \
  if (bf>boundaryTop) { \
    while (block<7 && bf>boundaryTop) { \
      bf>>=1; \
      block++; \
    } \
    if (bf>((1<<bits)-1)) { \
      bf=(1<<bits)-1; \
    } \
  } \
  /* logV("f-num: %d block: %d",bf,block); */ \
  return bf|(block<<bits);

#define CONVERT_FNUM_FIXEDBLOCK(bf,bits,block) \
  bf>>=(block); \
  if (bf<0) bf=0; \
  if (bf>((1<<(bits))-1)) { \
    bf=(1<<(bits))-1; \
  } \
  return bf|((block)<<(bits));

int DivEngine::calcBaseFreqFNumBlock(double clock, double divider, int note, int bits, int fixedBlock) {
  if (song.linearPitch==2) { // full linear
    return (note<<7);
  }
  int bf=calcBaseFreq(clock,divider,note,false);
  if (fixedBlock>0) {
    CONVERT_FNUM_FIXEDBLOCK(bf,bits,fixedBlock-1);
  } else {
    CONVERT_FNUM_BLOCK(bf,bits,note);
  }
}

int DivEngine::calcFreq(int base, int pitch, int arp, bool arpFixed, bool period, int octave, int pitch2, double clock, double divider, int blockBits, int fixedBlock) {
  if (song.linearPitch==2) {
    // do frequency calculation here
    int nbase=base+pitch+pitch2;
    if (!song.oldArpStrategy) {
      if (arpFixed) {
        nbase=(arp<<7)+pitch+pitch2;
      } else {
        nbase+=arp<<7;
      }
    }
    double fbase=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(nbase+384)/(128.0*12.0));
    int bf=period?
           round((clock/fbase)/divider):
           round(fbase*(divider/clock));
    if (blockBits>0) {
      if (fixedBlock>0) {
        CONVERT_FNUM_FIXEDBLOCK(bf,blockBits,fixedBlock-1);
      } else {
        CONVERT_FNUM_BLOCK(bf,blockBits,nbase>>7);
      }
    } else {
      return bf;
    }
  }
  if (song.linearPitch==1) {
    // global pitch multiplier
    int whatTheFuck=(1024+(globalPitch<<6)-(globalPitch<0?globalPitch-6:0));
    if (whatTheFuck<1) whatTheFuck=1; // avoids division by zero but please kill me
    if (song.pitchMacroIsLinear) {
      pitch+=pitch2;
    }
    pitch+=2048;
    if (pitch<0) pitch=0;
    if (pitch>4095) pitch=4095;
    int ret=period?
              ((base*(reversePitchTable[pitch]))/whatTheFuck):
              (((base*(pitchTable[pitch]))>>10)*whatTheFuck)/1024;
    if (!song.pitchMacroIsLinear) {
      ret+=period?(-pitch2):pitch2;
    }
    return ret;
  }
  return period?
           base-pitch-pitch2:
           base+((pitch*octave)>>1)+pitch2;
}

int DivEngine::calcArp(int note, int arp, int offset) {
  if (arp<0) {
    if (!(arp&0x40000000)) return (arp|0x40000000)+offset;
  } else {
    if (arp&0x40000000) return (arp&(~0x40000000))+offset;
  }
  return note+arp;
}

int DivEngine::convertPanSplitToLinear(unsigned int val, unsigned char bits, int range) {
  int panL=val>>bits;
  int panR=val&((1<<bits)-1);
  int diff=panR-panL;
  float pan=0.5f;
  if (diff!=0) {
    pan=(1.0f+((float)diff/(float)MAX(panL,panR)))*0.5f;
  }
  return pan*range;
}

int DivEngine::convertPanSplitToLinearLR(unsigned char left, unsigned char right, int range) {
  return convertPanSplitToLinear((left<<8)|right,8,range);
}

unsigned int DivEngine::convertPanLinearToSplit(int val, unsigned char bits, int range) {
  if (val<0) val=0;
  if (val>range) val=range;
  int maxV=(1<<bits)-1;
  int panL=(((range-val)*maxV*2))/range;
  int panR=((val)*maxV*2)/range;
  if (panL>maxV) panL=maxV;
  if (panR>maxV) panR=maxV;
  return (panL<<bits)|panR;
}

bool DivEngine::play() {
  BUSY_BEGIN_SOFT;
  curOrder=prevOrder;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  shallStop=false;
  if (stepPlay==0) {
    freelance=false;
    playSub(false);
  } else {
    stepPlay=0;
  }
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    keyHit[i]=false;
  }
  curMidiTimePiece=0;
  if (output) if (!skipping && output->midiOut!=NULL) {
    if (midiOutClock) {
      output->midiOut->send(TAMidiMessage(TA_MIDI_POSITION,(curMidiClock>>7)&0x7f,curMidiClock&0x7f));
    }
    if (midiOutTime) {
      TAMidiMessage msg;
      msg.type=TA_MIDI_SYSEX;
      msg.sysExData.reset(new unsigned char[10],std::default_delete<unsigned char[]>());
      msg.sysExLen=10;
      unsigned char* msgData=msg.sysExData.get();
      int actualTime=curMidiTime;
      int timeRate=midiOutTimeRate;
      int drop=0;
      if (timeRate<1 || timeRate>4) {
        if (curSubSong->hz>=47.98 && curSubSong->hz<=48.02) {
          timeRate=1;
        } else if (curSubSong->hz>=49.98 && curSubSong->hz<=50.02) {
          timeRate=2;
        } else if (curSubSong->hz>=59.9 && curSubSong->hz<=60.11) {
          timeRate=4;
        } else {
          timeRate=4;
        }
      }

      switch (timeRate) {
        case 1: // 24
          msgData[5]=(actualTime/(60*60*24))%24;
          msgData[6]=(actualTime/(60*24))%60;
          msgData[7]=(actualTime/24)%60;
          msgData[8]=actualTime%24;
          break;
        case 2: // 25
          msgData[5]=(actualTime/(60*60*25))%24;
          msgData[6]=(actualTime/(60*25))%60;
          msgData[7]=(actualTime/25)%60;
          msgData[8]=actualTime%25;
          break;
        case 3: // 29.97 (NTSC drop)
          // drop
          drop=((actualTime/(30*60))-(actualTime/(30*600)))*2;
          actualTime+=drop;

          msgData[5]=(actualTime/(60*60*30))%24;
          msgData[6]=(actualTime/(60*30))%60;
          msgData[7]=(actualTime/30)%60;
          msgData[8]=actualTime%30;
          break;
        case 4: // 30 (NTSC non-drop)
        default:
          msgData[5]=(actualTime/(60*60*30))%24;
          msgData[6]=(actualTime/(60*30))%60;
          msgData[7]=(actualTime/30)%60;
          msgData[8]=actualTime%30;
          break;
      }

      msgData[5]|=(timeRate-1)<<5;

      msgData[0]=0xf0;
      msgData[1]=0x7f;
      msgData[2]=0x7f;
      msgData[3]=0x01;
      msgData[4]=0x01;
      msgData[9]=0xf7;
      output->midiOut->send(msg);
    }
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_PLAY,0,0));
  }
  bool didItPlay=playing;
  BUSY_END;
  return didItPlay;
}

bool DivEngine::playToRow(int row) {
  BUSY_BEGIN_SOFT;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  freelance=false;
  playSub(false,row);
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    keyHit[i]=false;
  }
  bool didItPlay=playing;
  BUSY_END;
  return didItPlay;
}

void DivEngine::stepOne(int row) {
  if (!isPlaying()) {
    BUSY_BEGIN_SOFT;
    freelance=false;
    playSub(false,row);
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      keyHit[i]=false;
    }
  } else {
    BUSY_BEGIN;
  }
  stepPlay=2;
  ticks=1;
  prevOrder=curOrder;
  prevRow=curRow;
  BUSY_END;
}

void DivEngine::stop() {
  BUSY_BEGIN;
  freelance=false;
  if (!playing) {
    //Send midi panic
    if (output) if (output->midiOut!=NULL) {
      output->midiOut->send(TAMidiMessage(TA_MIDI_CONTROL,0x7B,0));
      logV("Midi panic sent");
    }
  }
  playing=false;
  extValuePresent=false;
  endOfSong=false; // what?
  stepPlay=0;
  curOrder=prevOrder;
  curRow=prevRow;
  remainingLoops=-1;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->notifyPlaybackStop();
  }
  if (output) if (output->midiOut!=NULL) {
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_STOP,0,0));
    for (int i=0; i<chans; i++) {
      if (chan[i].curMidiNote>=0) {
        output->midiOut->send(TAMidiMessage(0x80|(i&15),chan[i].curMidiNote,0));
      }
    }
  }

  // reset all chan oscs
  for (int i=0; i<chans; i++) {
    DivDispatchOscBuffer* buf=disCont[dispatchOfChan[i]].dispatch->getOscBuffer(dispatchChanOfChan[i]);
    if (buf!=NULL) {
      buf->reset();
    }
  }
  BUSY_END;
}

void DivEngine::halt() {
  BUSY_BEGIN;
  halted=true;
  BUSY_END;
}

void DivEngine::resume() {
  BUSY_BEGIN;
  halted=false;
  haltOn=DIV_HALT_NONE;
  BUSY_END;
}

void DivEngine::haltWhen(DivHaltPositions when) {
  BUSY_BEGIN;
  halted=false;
  haltOn=when;
  BUSY_END;
}

bool DivEngine::isHalted() {
  return halted;
}

const char** DivEngine::getRegisterSheet(int sys) {
  if (sys<0 || sys>=song.systemLen) return NULL;
  return disCont[sys].dispatch->getRegisterSheet();
}

void DivEngine::recalcChans() {
  bool isInsTypePossible[DIV_INS_MAX];
  chans=0;
  int chanIndex=0;
  memset(isInsTypePossible,0,DIV_INS_MAX*sizeof(bool));
  for (int i=0; i<song.systemLen; i++) {
    int chanCount=getChannelCount(song.system[i]);
    int firstChan=chans;
    chans+=chanCount;
    for (int j=0; j<chanCount; j++) {
      sysOfChan[chanIndex]=song.system[i];
      dispatchOfChan[chanIndex]=i;
      dispatchChanOfChan[chanIndex]=j;
      dispatchFirstChan[chanIndex]=firstChan;
      chanIndex++;

      if (sysDefs[song.system[i]]!=NULL) {
        if (sysDefs[song.system[i]]->chanInsType[j][0]!=DIV_INS_NULL) {
          isInsTypePossible[sysDefs[song.system[i]]->chanInsType[j][0]]=true;
        }

        if (sysDefs[song.system[i]]->chanInsType[j][1]!=DIV_INS_NULL) {
          isInsTypePossible[sysDefs[song.system[i]]->chanInsType[j][1]]=true;
        }
      }
    }
  }

  possibleInsTypes.clear();
  for (int i=0; i<DIV_INS_MAX; i++) {
    if (isInsTypePossible[i]) possibleInsTypes.push_back((DivInstrumentType)i);
  }

  checkAssetDir(song.insDir,song.ins.size());
  checkAssetDir(song.waveDir,song.wave.size());
  checkAssetDir(song.sampleDir,song.sample.size());

  hasLoadedSomething=true;
}

void DivEngine::reset() {
  if (output) if (output->midiOut!=NULL) {
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_STOP,0,0));
    for (int i=0; i<chans; i++) {
      if (chan[i].curMidiNote>=0) {
        output->midiOut->send(TAMidiMessage(0x80|(i&15),chan[i].curMidiNote,0));
      }
    }
  }
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    chan[i]=DivChannelState();
    if (i<chans) chan[i].volMax=(disCont[dispatchOfChan[i]].dispatch->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,dispatchChanOfChan[i]))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
    if (song.linearPitch==0) chan[i].vibratoFine=4;
  }
  extValue=0;
  extValuePresent=0;
  speeds=curSubSong->speeds;
  virtualTempoN=curSubSong->virtualTempoN;
  virtualTempoD=curSubSong->virtualTempoD;
  firstTick=false;
  shallStop=false;
  shallStopSched=false;
  pendingMetroTick=0;
  elapsedBars=0;
  elapsedBeats=0;
  nextSpeed=speeds.val[0];
  divider=curSubSong->hz;
  globalPitch=0;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].dispatch->reset();
    disCont[i].clear();
  }
}

void DivEngine::syncReset() {
  BUSY_BEGIN;
  reset();
  BUSY_END;
}

const int sampleRates[6]={
  4000, 8000, 11025, 16000, 22050, 32000
};

int DivEngine::fileToDivRate(int frate) {
  if (frate<0) frate=0;
  if (frate>5) frate=5;
  return sampleRates[frate];
}

int DivEngine::divToFileRate(int drate) {
  if (drate>26000) {
    return 5;
  } else if (drate>18000) {
    return 4;
  } else if (drate>14000) {
    return 3;
  } else if (drate>9500) {
    return 2;
  } else if (drate>6000) {
    return 1;
  } else {
    return 0;
  }
  return 4;
}

void DivEngine::testFunction() {
  logI("it works!");
}

int DivEngine::getEffectiveSampleRate(int rate) {
  if (rate<1) return 0;
  switch (song.system[0]) {
    case DIV_SYSTEM_YMU759:
      return 8000;
    case DIV_SYSTEM_YM2612: case DIV_SYSTEM_YM2612_EXT:
      return 1278409/(1280000/rate);
    case DIV_SYSTEM_PCE:
      return 1789773/(1789773/rate);
    case DIV_SYSTEM_SEGAPCM: case DIV_SYSTEM_SEGAPCM_COMPAT:
      return (31250*MIN(255,(rate*255/31250)))/255;
    case DIV_SYSTEM_QSOUND:
      return (24038*MIN(65535,(rate*4096/24038)))/4096;
    case DIV_SYSTEM_YM2610: case DIV_SYSTEM_YM2610_EXT: case DIV_SYSTEM_YM2610_FULL: case DIV_SYSTEM_YM2610_FULL_EXT: case DIV_SYSTEM_YM2610B: case DIV_SYSTEM_YM2610B_EXT:
      return 18518;
    case DIV_SYSTEM_VERA:
      return (48828*MIN(128,(rate*128/48828)))/128;
    case DIV_SYSTEM_X1_010:
      return (31250*MIN(255,(rate*16/31250)))/16; // TODO: support variable clock case
    case DIV_SYSTEM_ES5506:
      return (31250*MIN(131071,(rate*2048/31250)))/2048; // TODO: support variable clock, channel limit case
    default:
      break;
  }
  return rate;
}

void DivEngine::previewSample(int sample, int note, int pStart, int pEnd) {
  BUSY_BEGIN;
  previewSampleNoLock(sample,note,pStart,pEnd);
  BUSY_END;
}

void DivEngine::stopSamplePreview() {
  BUSY_BEGIN;
  stopSamplePreviewNoLock();
  BUSY_END;
}

void DivEngine::previewWave(int wave, int note) {
  BUSY_BEGIN;
  previewWaveNoLock(wave,note);
  BUSY_END;
}

void DivEngine::stopWavePreview() {
  BUSY_BEGIN;
  stopWavePreviewNoLock();
  BUSY_END;
}

void DivEngine::previewSampleNoLock(int sample, int note, int pStart, int pEnd) {
  sPreview.pBegin=pStart;
  sPreview.pEnd=pEnd;
  sPreview.dir=false;
  if (sample<0 || sample>=(int)song.sample.size()) {
    sPreview.sample=-1;
    sPreview.pos=0;
    sPreview.dir=false;
    return;
  }
  blip_clear(samp_bb);
  double rate=song.sample[sample]->centerRate;
  if (note>=0) {
    rate=(pow(2.0,(double)(note)/12.0)*((double)song.sample[sample]->centerRate)*0.0625);
    if (rate<=0) rate=song.sample[sample]->centerRate;
  }
  if (rate<100) rate=100;
  double rateOrig=rate;
  sPreview.rateMul=1;
  while (sPreview.rateMul<0x40000000 && rate<got.rate) {
    sPreview.rateMul<<=1;
    rate*=2.0;
  }
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.rate=rateOrig;
  sPreview.pos=(sPreview.pBegin>=0)?sPreview.pBegin:0;
  sPreview.posSub=0;
  sPreview.sample=sample;
  sPreview.wave=-1;
  sPreview.dir=false;
}

void DivEngine::stopSamplePreviewNoLock() {
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
}

void DivEngine::previewWaveNoLock(int wave, int note) {
  if (wave<0 || wave>=(int)song.wave.size()) {
    sPreview.wave=-1;
    sPreview.pos=0;
    sPreview.dir=false;
    return;
  }
  if (song.wave[wave]->len<=0) {
    return;
  }
  blip_clear(samp_bb);
  double rate=song.wave[wave]->len*((song.tuning*0.0625)*pow(2.0,(double)(note+3)/12.0));
  if (rate<100) rate=100;
  double rateOrig=rate;
  sPreview.rateMul=1;
  while (sPreview.rateMul<0x40000000 && rate<got.rate) {
    sPreview.rateMul<<=1;
    rate*=2.0;
  }
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.rate=rateOrig;
  sPreview.pos=0;
  sPreview.posSub=0;
  sPreview.sample=-1;
  sPreview.wave=wave;
  sPreview.dir=false;
}

void DivEngine::stopWavePreviewNoLock() {
  sPreview.wave=-1;
  sPreview.pos=0;
  sPreview.dir=false;
}

bool DivEngine::isPreviewingSample() {
  return (sPreview.sample>=0 && sPreview.sample<(int)song.sample.size() && sPreview.pos!=sPreview.pEnd);
}

int DivEngine::getSamplePreviewSample() {
  return sPreview.sample;
}

int DivEngine::getSamplePreviewPos() {
  return sPreview.pos;
}

double DivEngine::getSamplePreviewRate() {
  return sPreview.rate;
}

double DivEngine::getCenterRate() {
  return song.oldCenterRate?8363.0:8372.0;
}

String DivEngine::getConfigPath() {
  return configPath;
}

int DivEngine::getMaxVolumeChan(int ch) {
  return chan[ch].volMax>>8;
}

int DivEngine::mapVelocity(int ch, float vel) {
  if (ch<0) return 0;
  if (ch>=chans) return 0;
  if (disCont[dispatchOfChan[ch]].dispatch==NULL) return 0;
  return disCont[dispatchOfChan[ch]].dispatch->mapVelocity(dispatchChanOfChan[ch],vel);
}

float DivEngine::getGain(int ch, int vol) {
  if (ch<0) return 0;
  if (ch>=chans) return 0;
  if (disCont[dispatchOfChan[ch]].dispatch==NULL) return 0;
  return disCont[dispatchOfChan[ch]].dispatch->getGain(dispatchChanOfChan[ch],vol);
}

unsigned char DivEngine::getOrder() {
  return prevOrder;
}

int DivEngine::getRow() {
  return prevRow;
}

void DivEngine::getPlayPos(int& order, int& row) {
  playPosLock.lock();
  order=prevOrder;
  row=prevRow;
  playPosLock.unlock();
}

void DivEngine::getPlayPosTick(int& order, int& row, int& tick, int& speed) {
  playPosLock.lock();
  order=prevOrder;
  row=prevRow;
  tick=ticks;
  speed=prevSpeed;
  playPosLock.unlock();
}

int DivEngine::getElapsedBars() {
  return elapsedBars;
}

int DivEngine::getElapsedBeats() {
  return elapsedBeats;
}

size_t DivEngine::getCurrentSubSong() {
  return curSubSongIndex;
}

const DivGroovePattern& DivEngine::getSpeeds() {
  return speeds;
}

float DivEngine::getHz() {
  return curSubSong->hz;
}

float DivEngine::getCurHz() {
  return divider;
}

short DivEngine::getVirtualTempoN() {
  return virtualTempoN;
}

short DivEngine::getVirtualTempoD() {
  return virtualTempoD;
}

void DivEngine::virtualTempoChanged() {
  BUSY_BEGIN;
  virtualTempoN=curSubSong->virtualTempoN;
  virtualTempoD=curSubSong->virtualTempoD;
  BUSY_END;
}

int DivEngine::getTotalSeconds() {
  return totalSeconds;
}

int DivEngine::getTotalTicks() {
  return totalTicks;
}

bool DivEngine::getRepeatPattern() {
  return repeatPattern;
}

void DivEngine::setRepeatPattern(bool value) {
  BUSY_BEGIN;
  repeatPattern=value;
  BUSY_END;
}

bool DivEngine::hasExtValue() {
  return extValuePresent;
}

unsigned char DivEngine::getExtValue() {
  return extValue;
}

bool DivEngine::isPlaying() {
  return (playing && !freelance);
}

bool DivEngine::isRunning() {
  return playing;
}

bool DivEngine::isStepping() {
  return !(stepPlay==0);
}

bool DivEngine::isChannelMuted(int chan) {
  return isMuted[chan];
}

void DivEngine::toggleMute(int chan) {
  muteChannel(chan,!isMuted[chan]);
}

void DivEngine::toggleSolo(int chan) {
  bool solo=false;
  for (int i=0; i<chans; i++) {
    if (i==chan) {
      solo=true;
      continue;
    } else {
      if (!isMuted[i]) {
        solo=false;
        break;
      }
    }
  }
  BUSY_BEGIN;
  if (!solo) {
    for (int i=0; i<chans; i++) {
      isMuted[i]=(i!=chan);
      if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
        disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
      }
    }
  } else {
    for (int i=0; i<chans; i++) {
      isMuted[i]=false;
      if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
        disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
      }
    }
  }
  BUSY_END;
}

void DivEngine::muteChannel(int chan, bool mute) {
  BUSY_BEGIN;
  isMuted[chan]=mute;
  if (disCont[dispatchOfChan[chan]].dispatch!=NULL) {
    disCont[dispatchOfChan[chan]].dispatch->muteChannel(dispatchChanOfChan[chan],isMuted[chan]);
  }
  BUSY_END;
}

void DivEngine::unmuteAll() {
  BUSY_BEGIN;
  for (int i=0; i<chans; i++) {
    isMuted[i]=false;
    if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
      disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],isMuted[i]);
    }
  }
  BUSY_END;
}

void DivEngine::dumpSongInfo() {
  printf(
    "SONG INFORMATION\n"
    "- name: %s\n"
    "- author: %s\n"
    "- album: %s\n"
    "- system: %s\n"
    "- %d ins, %d waves, %d samples\n"
    "<<<\n%s\n>>>\n\n",
    song.name.c_str(),
    song.author.c_str(),
    song.category.c_str(),
    song.systemName.c_str(),
    song.insLen,
    song.waveLen,
    song.sampleLen,
    song.notes.c_str()
  );

  printf("SUB-SONGS\n");
  int index=0;
  for (DivSubSong* i: song.subsong) {
    printf(
      "=== %d: %s\n"
      "<<<\n%s\n>>>\n",
      index,
      i->name.c_str(),
      i->notes.c_str()
    );
    index++;
  }

  if (!song.ins.empty()) {
    printf("\nINSTRUMENTS\n");
    index=0;
    for (DivInstrument* i: song.ins) {
      printf(
        "- %d: %s\n",
        index,
        i->name.c_str()
      );
      index++;
    }
  }

  if (!song.sample.empty()) {
    printf("\nSAMPLES\n");
    index=0;
    for (DivSample* i: song.sample) {
      printf(
        "- %d: %s\n",
        index,
        i->name.c_str()
      );
      index++;
    }
  }
}

int DivEngine::addInstrument(int refChan, DivInstrumentType fallbackType) {
  if (song.ins.size()>=256) return -1;
  BUSY_BEGIN;
  DivInstrument* ins=new DivInstrument;
  int insCount=(int)song.ins.size();
  DivInstrumentType prefType;
  if (refChan>chans) {
    refChan=chans-1;
  }
  if (refChan<0) {
    prefType=fallbackType;
  } else {
    prefType=getPreferInsType(refChan);
  }
  switch (prefType) {
    case DIV_INS_OPLL:
      *ins=song.nullInsOPLL;
      break;
    case DIV_INS_OPL:
      *ins=song.nullInsOPL;
      break;
    case DIV_INS_OPL_DRUMS:
      *ins=song.nullInsOPLDrums;
      break;
    case DIV_INS_ESFM:
      *ins=song.nullInsESFM;
      break;
    default:
      break;
  }
  if (refChan>=0) {
    if (sysOfChan[refChan]==DIV_SYSTEM_QSOUND) {
      *ins=song.nullInsQSound;
    }
  }
  ins->name=fmt::sprintf(_("Instrument %d"),insCount);
  if (prefType!=DIV_INS_NULL) {
    ins->type=prefType;
  }
  saveLock.lock();
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  checkAssetDir(song.insDir,song.ins.size());
  saveLock.unlock();
  bool hasSampleInst=false;
  for (int s=0; s<song.systemLen; s++) {
    if (disCont[s].dispatch->hasSampleInstHeader()) {
      hasSampleInst=true;
    }
  }
  if (hasSampleInst) {
    renderSamplesP();
  }
  BUSY_END;
  return insCount;
}

int DivEngine::addInstrumentPtr(DivInstrument* which) {
  if (song.ins.size()>=256) {
    delete which;
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  song.ins.push_back(which);
  song.insLen=song.ins.size();
  checkAssetDir(song.insDir,song.ins.size());
  checkAssetDir(song.waveDir,song.wave.size());
  checkAssetDir(song.sampleDir,song.sample.size());
  saveLock.unlock();
  bool hasSampleInst=false;
  for (int s=0; s<song.systemLen; s++) {
    if (disCont[s].dispatch->hasSampleInstHeader()) {
      hasSampleInst=true;
    }
  }
  if (hasSampleInst) {
    renderSamplesP();
  }
  BUSY_END;
  return song.insLen;
}

void DivEngine::loadTempIns(DivInstrument* which) {
  BUSY_BEGIN;
  if (tempIns==NULL) {
    tempIns=new DivInstrument;
  }
  *tempIns=*which;
  BUSY_END;
}

void DivEngine::delInstrumentUnsafe(int index) {
  if (index>=0 && index<(int)song.ins.size()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].dispatch->notifyInsDeletion(song.ins[index]);
    }
    delete song.ins[index];
    song.ins.erase(song.ins.begin()+index);
    song.insLen=song.ins.size();
    for (int i=0; i<chans; i++) {
      for (size_t j=0; j<song.subsong.size(); j++) {
        for (int k=0; k<DIV_MAX_PATTERNS; k++) {
          if (song.subsong[j]->pat[i].data[k]==NULL) continue;
          for (int l=0; l<song.subsong[j]->patLen; l++) {
            if (song.subsong[j]->pat[i].data[k]->data[l][2]>index) {
              song.subsong[j]->pat[i].data[k]->data[l][2]--;
            }
          }
        }
      }
    }
    removeAsset(song.insDir,index);
    checkAssetDir(song.insDir,song.ins.size());
    bool hasSampleInst=false;
    for (int s=0; s<song.systemLen; s++) {
      if (disCont[s].dispatch->hasSampleInstHeader()) {
        hasSampleInst=true;
      }
    }
    if (hasSampleInst) {
      renderSamplesP();
    }
  }
}

void DivEngine::delInstrument(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  delInstrumentUnsafe(index);
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addWave() {
  if (song.wave.size()>=32768) {
    lastError=_("too many wavetables!");
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  DivWavetable* wave=new DivWavetable;
  int waveCount=(int)song.wave.size();
  song.wave.push_back(wave);
  song.waveLen=waveCount+1;
  checkAssetDir(song.waveDir,song.wave.size());
  saveLock.unlock();
  BUSY_END;
  return waveCount;
}

int DivEngine::addWavePtr(DivWavetable* which) {
  if (song.wave.size()>=32768) {
    lastError=_("too many wavetables!");
    delete which;
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  int waveCount=(int)song.wave.size();
  song.wave.push_back(which);
  song.waveLen=waveCount+1;
  checkAssetDir(song.waveDir,song.wave.size());
  saveLock.unlock();
  BUSY_END;
  return song.waveLen;
}

DivWavetable* DivEngine::waveFromFile(const char* path, bool addRaw) {
  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    lastError=fmt::sprintf("%s",strerror(errno));
    return NULL;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    fclose(f);
    lastError=fmt::sprintf(_("could not seek to end: %s"),strerror(errno));
    return NULL;
  }
  len=ftell(f);
  if (len<0) {
    fclose(f);
    lastError=fmt::sprintf(_("could not determine file size: %s"),strerror(errno));
    return NULL;
  }
  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    lastError=_("file size is invalid!");
    return NULL;
  }
  if (len==0) {
    fclose(f);
    lastError=_("file is empty");
    return NULL;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    fclose(f);
    lastError=fmt::sprintf(_("could not seek to beginning: %s"),strerror(errno));
    return NULL;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire wavetable file buffer!");
    delete[] buf;
    lastError=fmt::sprintf(_("could not read entire file: %s"),strerror(errno));
    return NULL;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceTable=false;
  try {
    reader.read(magic,16);
    if (memcmp("-Furnace waveta-",magic,16)==0) {
      isFurnaceTable=true;
    }
  } catch (EndOfFileException& e) {
    reader.seek(0,SEEK_SET);
  }

  DivWavetable* wave=new DivWavetable;
  try {
    if (isFurnaceTable) {
      reader.seek(16,SEEK_SET);
      short version=reader.readS();
      reader.readS(); // reserved
      reader.seek(20,SEEK_SET);
      if (wave->readWaveData(reader,version)!=DIV_DATA_SUCCESS) {
        lastError=_("invalid wavetable header/data!");
        delete wave;
        delete[] buf;
        return NULL;
      }
    } else {
      try {
        // read as .dmw
        reader.seek(0,SEEK_SET);
        int len=reader.readI();
        logD("wave length %d",len);
        if (len<=0 || len>256) {
          throw EndOfFileException(&reader,reader.size());
        }
        wave->len=len;
        wave->max=(unsigned char)reader.readC();
        if (wave->max==255) { // new wavetable format
          unsigned char waveVersion=reader.readC();
          logI("reading modern .dmw...");
          logD("wave version %d",waveVersion);
          wave->max=(unsigned char)reader.readC();
          for (int i=0; i<len; i++) {
            wave->data[i]=reader.readI();
          }
        } else if (reader.size()==(size_t)(len+5)) {
          // read as .dmw
          logI("reading .dmw...");
          if (len>256) len=256;
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
          }
        } else {
          // read as binary
          if (addRaw) {
            logI("reading binary...");
            len=reader.size();
            if (len>256) len=256;
            reader.seek(0,SEEK_SET);
            for (int i=0; i<len; i++) {
              wave->data[i]=(unsigned char)reader.readC();
              if (wave->max<wave->data[i]) wave->max=wave->data[i];
            }
            wave->len=len;
          } else {
            delete wave;
            delete[] buf;
            return NULL;
          }
        }
      } catch (EndOfFileException& e) {
        // read as binary
        if (addRaw) {
          len=reader.size();
          logI("reading binary for being too small...");
          if (len>256) len=256;
          reader.seek(0,SEEK_SET);
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
            if (wave->max<wave->data[i]) wave->max=wave->data[i];
          }
          wave->len=len;
        } else {
          delete wave;
          delete[] buf;
          return NULL;
        }
      }
    }
  } catch (EndOfFileException& e) {
    delete wave;
    delete[] buf;
    lastError=_("premature end of file");
    return NULL;
  }
  
  return wave;
}

void DivEngine::delWaveUnsafe(int index) {
  if (index>=0 && index<(int)song.wave.size()) {
    delete song.wave[index];
    song.wave.erase(song.wave.begin()+index);
    song.waveLen=song.wave.size();
    removeAsset(song.waveDir,index);
    checkAssetDir(song.waveDir,song.wave.size());
  }
}

void DivEngine::delWave(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  delWaveUnsafe(index);
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addSample() {
  if (song.sample.size()>=32768) {
    lastError=_("too many samples!");
    return -1;
  }
  BUSY_BEGIN;
  saveLock.lock();
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=fmt::sprintf(_("Sample %d"),sampleCount);
  sample->centerRate=getCenterRate();
  song.sample.push_back(sample);
  song.sampleLen=sampleCount+1;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  checkAssetDir(song.sampleDir,song.sample.size());
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return sampleCount;
}

int DivEngine::addSamplePtr(DivSample* which) {
  if (song.sample.size()>=32768) {
    lastError=_("too many samples!");
    delete which;
    return -1;
  }
  int sampleCount=(int)song.sample.size();
  BUSY_BEGIN;
  saveLock.lock();
  song.sample.push_back(which);
  song.sampleLen=sampleCount+1;
  checkAssetDir(song.sampleDir,song.sample.size());
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return sampleCount;
}

void DivEngine::delSampleUnsafe(int index, bool render) {
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  if (index>=0 && index<(int)song.sample.size()) {
    delete song.sample[index];
    song.sample.erase(song.sample.begin()+index);
    song.sampleLen=song.sample.size();
    removeAsset(song.sampleDir,index);
    checkAssetDir(song.sampleDir,song.sample.size());

    // compensate
    for (DivInstrument* i: song.ins) {
      if (i->amiga.initSample==index) {
        i->amiga.initSample=-1;
      } else if (i->amiga.initSample>index) {
        i->amiga.initSample--;
      }
      for (int j=0; j<120; j++) {
        if (i->amiga.noteMap[j].map==index) {
          i->amiga.noteMap[j].map=-1;
        } else if (i->amiga.noteMap[j].map>index) {
          i->amiga.noteMap[j].map--;
        }
      }
    }

    if (render) renderSamples();
  }
}

void DivEngine::delSample(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  delSampleUnsafe(index);
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::addOrder(int pos, bool duplicate, bool where) {
  unsigned char order[DIV_MAX_CHANS];
  if (curSubSong->ordersLen>=(DIV_MAX_PATTERNS-1)) return;
  memset(order,0,DIV_MAX_CHANS);
  BUSY_BEGIN_SOFT;
  if (duplicate) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      order[i]=curOrders->ord[i][pos];
    }
  } else {
    bool used[DIV_MAX_PATTERNS];
    for (int i=0; i<chans; i++) {
      memset(used,0,sizeof(bool)*DIV_MAX_PATTERNS);
      for (int j=0; j<curSubSong->ordersLen; j++) {
        used[curOrders->ord[i][j]]=true;
      }
      order[i]=(DIV_MAX_PATTERNS-1);
      for (int j=0; j<DIV_MAX_PATTERNS; j++) {
        if (!used[j]) {
          order[i]=j;
          break;
        }
      }
    }
  }
  if (where) { // at the end
    saveLock.lock();
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      curOrders->ord[i][curSubSong->ordersLen]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
  } else { // after current order
    saveLock.lock();
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      for (int j=curSubSong->ordersLen; j>pos; j--) {
        curOrders->ord[i][j]=curOrders->ord[i][j-1];
      }
      curOrders->ord[i][pos+1]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
    curOrder=pos+1;
    prevOrder=curOrder;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  BUSY_END;
}

void DivEngine::deepCloneOrder(int pos, bool where) {
  unsigned char order[DIV_MAX_CHANS];
  if (curSubSong->ordersLen>=(DIV_MAX_PATTERNS-1)) return;
  warnings="";
  BUSY_BEGIN_SOFT;
  for (int i=0; i<chans; i++) {
    bool didNotFind=true;
    logD("channel %d",i);
    order[i]=curOrders->ord[i][pos];
    // find free slot
    for (int j=0; j<DIV_MAX_PATTERNS; j++) {
      logD("finding free slot in %d...",j);
      if (curPat[i].data[j]==NULL) {
        int origOrd=order[i];
        order[i]=j;
        DivPattern* oldPat=curPat[i].getPattern(origOrd,false);
        DivPattern* pat=curPat[i].getPattern(j,true);
        memcpy(pat->data,oldPat->data,DIV_MAX_ROWS*DIV_MAX_COLS*sizeof(short));
        logD("found at %d",j);
        didNotFind=false;
        break;
      }
    }
    if (didNotFind) {
      addWarning(fmt::sprintf(_("no free patterns in channel %d!"),i));
    }
  }
  if (where) { // at the end
    saveLock.lock();
    for (int i=0; i<chans; i++) {
      curOrders->ord[i][curSubSong->ordersLen]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
  } else { // after current order
    saveLock.lock();
    for (int i=0; i<chans; i++) {
      for (int j=curSubSong->ordersLen; j>pos; j--) {
        curOrders->ord[i][j]=curOrders->ord[i][j-1];
      }
      curOrders->ord[i][pos+1]=order[i];
    }
    curSubSong->ordersLen++;
    saveLock.unlock();
    if (pos<=curOrder) curOrder++;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  BUSY_END;
}

void DivEngine::deleteOrder(int pos) {
  if (curSubSong->ordersLen<=1) return;
  BUSY_BEGIN_SOFT;
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    for (int j=pos; j<curSubSong->ordersLen; j++) {
      curOrders->ord[i][j]=curOrders->ord[i][j+1];
    }
  }
  curSubSong->ordersLen--;
  saveLock.unlock();
  if (curOrder>pos) curOrder--;
  if (curOrder>=curSubSong->ordersLen) curOrder=curSubSong->ordersLen-1;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::moveOrderUp(int& pos) {
  BUSY_BEGIN_SOFT;
  if (pos<1) {
    BUSY_END;
    return;
  }
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    curOrders->ord[i][pos]^=curOrders->ord[i][pos-1];
    curOrders->ord[i][pos-1]^=curOrders->ord[i][pos];
    curOrders->ord[i][pos]^=curOrders->ord[i][pos-1];
  }
  saveLock.unlock();
  if (curOrder==pos) {
    curOrder--;
  }
  pos--;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::moveOrderDown(int& pos) {
  BUSY_BEGIN_SOFT;
  if (pos>=curSubSong->ordersLen-1) {
    BUSY_END;
    return;
  }
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    curOrders->ord[i][pos]^=curOrders->ord[i][pos+1];
    curOrders->ord[i][pos+1]^=curOrders->ord[i][pos];
    curOrders->ord[i][pos]^=curOrders->ord[i][pos+1];
  }
  saveLock.unlock();
  if (curOrder==pos) {
    curOrder++;
  }
  pos++;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::exchangeIns(int one, int two) {
  for (int i=0; i<chans; i++) {
    for (size_t j=0; j<song.subsong.size(); j++) {
      for (int k=0; k<DIV_MAX_PATTERNS; k++) {
        if (song.subsong[j]->pat[i].data[k]==NULL) continue;
        for (int l=0; l<song.subsong[j]->patLen; l++) {
          if (song.subsong[j]->pat[i].data[k]->data[l][2]==one) {
            song.subsong[j]->pat[i].data[k]->data[l][2]=two;
          } else if (song.subsong[j]->pat[i].data[k]->data[l][2]==two) {
            song.subsong[j]->pat[i].data[k]->data[l][2]=one;
          }
        }
      }
    }
  }
}

void DivEngine::exchangeWave(int one, int two) {
  // TODO
}

void DivEngine::exchangeSample(int one, int two) {
  for (DivInstrument* i: song.ins) {
    if (i->amiga.initSample==one) {
      i->amiga.initSample=two;
    } else if (i->amiga.initSample==two) {
      i->amiga.initSample=one;
    }
    for (int j=0; j<120; j++) {
      if (i->amiga.noteMap[j].map==one) {
        i->amiga.noteMap[j].map=two;
      } else if (i->amiga.noteMap[j].map==two) {
        i->amiga.noteMap[j].map=one;
      }
    }
  }
}

bool DivEngine::moveInsUp(int which) {
  if (which<1 || which>=(int)song.ins.size()) return false;
  BUSY_BEGIN;
  DivInstrument* prev=song.ins[which];
  saveLock.lock();
  song.ins[which]=song.ins[which-1];
  song.ins[which-1]=prev;
  moveAsset(song.insDir,which,which-1);
  exchangeIns(which,which-1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveWaveUp(int which) {
  if (which<1 || which>=(int)song.wave.size()) return false;
  BUSY_BEGIN;
  DivWavetable* prev=song.wave[which];
  saveLock.lock();
  song.wave[which]=song.wave[which-1];
  song.wave[which-1]=prev;
  moveAsset(song.waveDir,which,which-1);
  exchangeWave(which,which-1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveSampleUp(int which) {
  if (which<1 || which>=(int)song.sample.size()) return false;
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  DivSample* prev=song.sample[which];
  saveLock.lock();
  song.sample[which]=song.sample[which-1];
  song.sample[which-1]=prev;
  moveAsset(song.sampleDir,which,which-1);
  exchangeSample(which,which-1);
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return true;
}

bool DivEngine::moveInsDown(int which) {
  if (which<0 || which>=((int)song.ins.size())-1) return false;
  BUSY_BEGIN;
  DivInstrument* prev=song.ins[which];
  saveLock.lock();
  song.ins[which]=song.ins[which+1];
  song.ins[which+1]=prev;
  exchangeIns(which,which+1);
  moveAsset(song.insDir,which,which+1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveWaveDown(int which) {
  if (which<0 || which>=((int)song.wave.size())-1) return false;
  BUSY_BEGIN;
  DivWavetable* prev=song.wave[which];
  saveLock.lock();
  song.wave[which]=song.wave[which+1];
  song.wave[which+1]=prev;
  exchangeWave(which,which+1);
  moveAsset(song.waveDir,which,which+1);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveSampleDown(int which) {
  if (which<0 || which>=((int)song.sample.size())-1) return false;
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  DivSample* prev=song.sample[which];
  saveLock.lock();
  song.sample[which]=song.sample[which+1];
  song.sample[which+1]=prev;
  exchangeSample(which,which+1);
  moveAsset(song.sampleDir,which,which+1);
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return true;
}

bool DivEngine::swapInstruments(int a, int b) {
  if (a<0 || a>=(int)song.ins.size() || b<0 || b>=(int)song.ins.size()) return false;
  BUSY_BEGIN;
  DivInstrument* temp=song.ins[a];
  saveLock.lock();
  song.ins[a]=song.ins[b];
  song.ins[b]=temp;
  moveAsset(song.insDir,a,b);
  exchangeIns(a,b);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::swapWaves(int a, int b) {
  if (a<0 || a>=(int)song.wave.size() || b<0 || b>=(int)song.wave.size()) return false;
  BUSY_BEGIN;
  DivWavetable* temp=song.wave[a];
  saveLock.lock();
  song.wave[a]=song.wave[b];
  song.wave[b]=temp;
  exchangeWave(a,b);
  moveAsset(song.waveDir,a,b);
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::swapSamples(int a, int b) {
  if (a<0 || a>=(int)song.sample.size() || b<0 || b>=(int)song.sample.size()) return false;
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  sPreview.dir=false;
  DivSample* temp=song.sample[a];
  saveLock.lock();
  song.sample[a]=song.sample[b];
  song.sample[b]=temp;
  exchangeSample(a,b);
  moveAsset(song.sampleDir,a,b);
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return true;
}


void DivEngine::autoPatchbay() {
  song.patchbay.clear();
  for (unsigned int i=0; i<song.systemLen; i++) {
    if (disCont[i].dispatch==NULL) continue;

    unsigned int outs=disCont[i].dispatch->getOutputCount();
    if (outs>16) outs=16;
    if (outs<2) {
      song.patchbay.reserve(DIV_MAX_OUTPUTS);
      for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
        song.patchbay.push_back((i<<20)|j);
      }
    } else {
      song.patchbay.reserve(outs);
      for (unsigned int j=0; j<outs; j++) {
        song.patchbay.push_back((i<<20)|(j<<16)|j);
      }
    }
  }

  // wave/sample preview
  song.patchbay.reserve(DIV_MAX_OUTPUTS);
  for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
    song.patchbay.push_back(0xffd00000|j);
  }

  // metronome
  song.patchbay.reserve(DIV_MAX_OUTPUTS);
  for (unsigned int j=0; j<DIV_MAX_OUTPUTS; j++) {
    song.patchbay.push_back(0xffe00000|j);
  }
}

void DivEngine::autoPatchbayP() {
  BUSY_BEGIN;
  saveLock.lock();
  autoPatchbay();
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::recalcPatchbay() {

}

bool DivEngine::patchConnect(unsigned int src, unsigned int dest) {
  unsigned int armed=(src<<16)|(dest&0xffff);
  for (unsigned int i: song.patchbay) {
    if (i==armed) return false;
  }
  BUSY_BEGIN;
  saveLock.lock();
  song.patchbay.push_back(armed);
  song.patchbayAuto=false;
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::patchDisconnect(unsigned int src, unsigned int dest) {
  unsigned int armed=(src<<16)|(dest&0xffff);
  for (auto i=song.patchbay.begin(); i!=song.patchbay.end(); i++) {
    if (*i==armed) {
      BUSY_BEGIN;
      saveLock.lock();
      song.patchbay.erase(i);
      song.patchbayAuto=false;
      saveLock.unlock();
      BUSY_END;
      return true;
    }
  }
  return false;
}

void DivEngine::patchDisconnectAll(unsigned int portSet) {
  BUSY_BEGIN;
  saveLock.lock();

  if (portSet&0x1000) {
    portSet&=0xfff;

    for (size_t i=0; i<song.patchbay.size(); i++) {
      if ((song.patchbay[i]&0xfff0)==(portSet<<4)) {
        song.patchbay.erase(song.patchbay.begin()+i);
        i--;
      }
    }
  } else {
    portSet&=0xfff;

    for (size_t i=0; i<song.patchbay.size(); i++) {
      if ((song.patchbay[i]&0xfff00000)==(portSet<<20)) {
        song.patchbay.erase(song.patchbay.begin()+i);
        i--;
      }
    }
  }

  saveLock.unlock();
  BUSY_END;
}

void DivEngine::noteOn(int chan, int ins, int note, int vol) {
  if (chan<0 || chan>=chans) return;
  BUSY_BEGIN;
  pendingNotes.push_back(DivNoteEvent(chan,ins,note,vol,true));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  BUSY_END;
}

void DivEngine::noteOff(int chan) {
  if (chan<0 || chan>=chans) return;
  BUSY_BEGIN;
  pendingNotes.push_back(DivNoteEvent(chan,-1,-1,-1,false));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  BUSY_END;
}

bool DivEngine::autoNoteOn(int ch, int ins, int note, int vol) {
  bool isViable[DIV_MAX_CHANS];
  bool canPlayAnyway=false;
  bool notInViableChannel=false;
  if (midiBaseChan<0) midiBaseChan=0;
  if (midiBaseChan>=chans) midiBaseChan=chans-1;
  int finalChan=midiBaseChan;
  int finalChanType=getChannelType(finalChan);

  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }

  // 1. check which channels are viable for this instrument
  DivInstrument* insInst=getIns(ins);
  if (getPreferInsType(finalChan)!=insInst->type && getPreferInsSecondType(finalChan)!=insInst->type && getPreferInsType(finalChan)!=DIV_INS_NULL) notInViableChannel=true;
  for (int i=0; i<chans; i++) {
    if (ins==-1 || ins>=song.insLen || getPreferInsType(i)==insInst->type || (getPreferInsType(i)==DIV_INS_NULL && finalChanType==DIV_CH_NOISE) || getPreferInsSecondType(i)==insInst->type) {
      if (insInst->type==DIV_INS_OPL) {
        if (insInst->fm.ops==2 || getChannelType(i)==DIV_CH_OP) {
          isViable[i]=true;
          canPlayAnyway=true;
        } else {
          isViable[i]=false;
        }
      } else {
        isViable[i]=true;
        canPlayAnyway=true;
      }
    } else {
      isViable[i]=false;
    }
  }

  if (!canPlayAnyway) return false;

  // 2. find a free channel
  do {
    if ((!midiPoly) || (isViable[finalChan] && chan[finalChan].midiNote==-1 && (insInst->type==DIV_INS_OPL || getChannelType(finalChan)==finalChanType || notInViableChannel))) {
      chan[finalChan].midiNote=note;
      chan[finalChan].midiAge=midiAgeCounter++;
      pendingNotes.push_back(DivNoteEvent(finalChan,ins,note,vol,true));
      return true;
    }
    if (++finalChan>=chans) {
      finalChan=0;
    }
  } while (finalChan!=midiBaseChan);

  // 3. find the oldest channel
  int candidate=finalChan;
  do {
    if (isViable[finalChan] && (insInst->type==DIV_INS_OPL || getChannelType(finalChan)==finalChanType || notInViableChannel) && chan[finalChan].midiAge<chan[candidate].midiAge) {
      candidate=finalChan;
    }
    if (++finalChan>=chans) {
      finalChan=0;
    }
  } while (finalChan!=midiBaseChan);

  chan[candidate].midiNote=note;
  chan[candidate].midiAge=midiAgeCounter++;
  pendingNotes.push_back(DivNoteEvent(candidate,ins,note,vol,true));
  return true;
}

void DivEngine::autoNoteOff(int ch, int note, int vol) {
  if (!playing) {
    return;
  }
  //if (ch<0 || ch>=chans) return;
  for (int i=0; i<chans; i++) {
    if (chan[i].midiNote==note) {
      pendingNotes.push_back(DivNoteEvent(i,-1,-1,-1,false));
      chan[i].midiNote=-1;
    }
  }
}

void DivEngine::autoNoteOffAll() {
  if (!playing) {
    return;
  }
  for (int i=0; i<chans; i++) {
    if (chan[i].midiNote!=-1) {
      pendingNotes.push_back(DivNoteEvent(i,-1,-1,-1,false));
      chan[i].midiNote=-1;
    }
  }
}

void DivEngine::setAutoNotePoly(bool poly) {
  midiPoly=poly;
}

void DivEngine::setOrder(unsigned char order) {
  BUSY_BEGIN_SOFT;
  curOrder=order;
  if (order>=curSubSong->ordersLen) curOrder=0;
  prevOrder=curOrder;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::updateSysFlags(int system, bool restart, bool render) {
  BUSY_BEGIN_SOFT;
  disCont[system].dispatch->setFlags(song.systemFlags[system]);
  disCont[system].setRates(got.rate);
  if (render) renderSamples();

  // patchbay
  if (song.patchbayAuto) {
    saveLock.lock();
    autoPatchbay();
    saveLock.unlock();
  }

  if (restart) {
    if (isPlaying()) {
      playSub(false);
    } else if (freelance) {
      reset();
    }
  }
  BUSY_END;
}

void DivEngine::setSongRate(float hz) {
  BUSY_BEGIN;
  saveLock.lock();
  curSubSong->hz=hz;
  divider=curSubSong->hz;
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::setAudio(DivAudioEngines which) {
  audioEngine=which;
}

void DivEngine::setView(DivStatusView which) {
  view=which;
}

bool DivEngine::getMetronome() {
  return metronome;
}

void DivEngine::setMetronome(bool enable) {
  metronome=enable;
  metroAmp=0;
}

void DivEngine::setMetronomeVol(float vol) {
  metroVol=vol;
}

void DivEngine::setSamplePreviewVol(float vol) {
  previewVol=vol;
}

void DivEngine::setConsoleMode(bool enable, bool statusOut) {
  consoleMode=enable;
  disableStatusOut=!statusOut;
}

bool DivEngine::switchMaster(bool full) {
  logI("switching output...");
  deinitAudioBackend(true);
  if (full) {
    quitDispatch();
    initDispatch();
  }
  if (renderPool!=NULL) {
    delete renderPool;
    renderPool=NULL;
  }
  if (initAudioBackend()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].setRates(got.rate);
      disCont[i].setQuality(lowQuality,dcHiPass);
    }
    if (!output->setRun(true)) {
      logE("error while activating audio!");
      return false;
    }
  } else {
    return false;
  }
  renderSamples();
  return true;
}

void DivEngine::setMidiBaseChan(int chan) {
  if (chan<0 || chan>=chans) chan=0;
  midiBaseChan=chan;
}

void DivEngine::setMidiDirect(bool value) {
  midiIsDirect=value;
}

void DivEngine::setMidiDirectProgram(bool value) {
  midiIsDirectProgram=value;
}

void DivEngine::setMidiVolExp(float value) {
  midiVolExp=value;
}

void DivEngine::setMidiCallback(std::function<int(const TAMidiMessage&)> what) {
  midiCallback=what;
}

void DivEngine::setMidiDebug(bool enable) {
  midiDebug=enable;
}

bool DivEngine::sendMidiMessage(TAMidiMessage& msg) {
  if (output==NULL) {
    logW("output is NULL!");
    return false;
  }
  if (output->midiOut==NULL) {
    logW("MIDI output is NULL!");
    return false;
  }
  BUSY_BEGIN;
  logD("sending MIDI message...");
  bool ret=(output->midiOut->send(msg));
  BUSY_END;
  return ret;
}

void DivEngine::synchronized(const std::function<void()>& what) {
  BUSY_BEGIN;
  what();
  BUSY_END;
}

void DivEngine::synchronizedSoft(const std::function<void()>& what) {
  BUSY_BEGIN_SOFT;
  what();
  BUSY_END;
}

void DivEngine::lockSave(const std::function<void()>& what) {
  saveLock.lock();
  what();
  saveLock.unlock();
}

void DivEngine::lockEngine(const std::function<void()>& what) {
  BUSY_BEGIN;
  saveLock.lock();
  what();
  saveLock.unlock();
  BUSY_END;
}

TAAudioDesc& DivEngine::getAudioDescWant() {
  return want;
}

TAAudioDesc& DivEngine::getAudioDescGot() {
  return got;
}

std::vector<String>& DivEngine::getAudioDevices() {
  return audioDevs;
}

std::vector<String>& DivEngine::getMidiIns() {
  return midiIns;
}

std::vector<String>& DivEngine::getMidiOuts() {
  return midiOuts;
}

void DivEngine::rescanAudioDevices() {
  audioDevs.clear();
  if (output!=NULL) {
    audioDevs=output->listAudioDevices();
  }
}

void DivEngine::rescanMidiDevices() {
  if (output!=NULL) {
    logV("re-scanning midi...");
    if (output->midiIn!=NULL) {
      midiIns=output->midiIn->listDevices();
    }
    if (output->midiOut!=NULL) {
      midiOuts=output->midiOut->listDevices();
    }
  }
}

void DivEngine::initDispatch(bool isRender) {
  BUSY_BEGIN;
  logV("initializing dispatch...");
  if (isRender) logI("render cores set");

  lowQuality=getConfInt("audioQuality",0);
  dcHiPass=getConfInt("audioHiPass",1);

  if (lowQuality) {
    blip_add_delta=blip_add_delta_fast;
  } else {
    blip_add_delta=blip_add_delta_slow;
  }

  for (int i=0; i<song.systemLen; i++) {
    disCont[i].init(song.system[i],this,getChannelCount(song.system[i]),got.rate,song.systemFlags[i],isRender);
    disCont[i].setRates(got.rate);
    disCont[i].setQuality(lowQuality,dcHiPass);
  }
  if (song.patchbayAuto) {
    saveLock.lock();
    autoPatchbay();
    saveLock.unlock();
  }
  recalcChans();
  BUSY_END;
}

void DivEngine::quitDispatch() {
  BUSY_BEGIN;
  logV("terminating dispatch...");
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].quit();
  }
  cycles=0;
  clockDrift=0;
  midiClockCycles=0;
  midiClockDrift=0;
  midiTimeCycles=0;
  midiTimeDrift=0;
  chans=0;
  playing=false;
  curSpeed=0;
  endOfSong=false;
  stepPlay=0;
  ticks=0;
  tempoAccum=0;
  curRow=0;
  curOrder=0;
  prevRow=0;
  prevOrder=0;
  nextSpeed=3;
  changeOrd=-1;
  changePos=0;
  totalTicks=0;
  totalTicksOff=0;
  totalSeconds=0;
  totalTicksR=0;
  curMidiClock=0;
  curMidiTime=0;
  curMidiTimeCode=0;
  curMidiTimePiece=0;
  totalCmds=0;
  lastCmds=0;
  cmdsPerSecond=0;
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
  }
  if (renderPool!=NULL) {
    delete renderPool;
    renderPool=NULL;
  }
  BUSY_END;
}

bool DivEngine::initAudioBackend() {
  // load values
  logI("initializing audio.");
  if (audioEngine==DIV_AUDIO_NULL) {
    if (getConfString("audioEngine","SDL")=="JACK") {
      audioEngine=DIV_AUDIO_JACK;
    } else if (getConfString("audioEngine","SDL")=="PortAudio") {
      audioEngine=DIV_AUDIO_PORTAUDIO;
    } else {
      audioEngine=DIV_AUDIO_SDL;
    }
  }

#ifdef HAVE_SDL2
  if (audioEngine==DIV_AUDIO_SDL) {
    String audioDriver=getConfString("sdlAudioDriver","");
    if (!audioDriver.empty()) {
      SDL_SetHint(SDL_HINT_AUDIODRIVER,audioDriver.c_str());
    }
  }
#endif

  forceMono=getConfInt("forceMono",0);
  clampSamples=getConfInt("clampSamples",0);
  lowLatency=getConfInt("lowLatency",0);
  metroVol=(float)(getConfInt("metroVol",100))/100.0f;
  previewVol=(float)(getConfInt("sampleVol",50))/100.0f;
  midiOutClock=getConfInt("midiOutClock",0);
  midiOutTime=getConfInt("midiOutTime",0);
  midiOutTimeRate=getConfInt("midiOutTimeRate",0);
  midiOutProgramChange=getConfInt("midiOutProgramChange",0);
  midiOutMode=getConfInt("midiOutMode",DIV_MIDI_MODE_NOTE);
  if (metroVol<0.0f) metroVol=0.0f;
  if (metroVol>2.0f) metroVol=2.0f;
  if (previewVol<0.0f) previewVol=0.0f;
  if (previewVol>1.0f) previewVol=1.0f;
  renderPoolThreads=getConfInt("renderPoolThreads",0);

  if (lowLatency) logI("using low latency mode.");

  switch (audioEngine) {
    case DIV_AUDIO_JACK:
#ifndef HAVE_JACK
      logE("Furnace was not compiled with JACK support!");
      setConf("audioEngine","SDL");
      saveConf();
#ifdef HAVE_SDL2
      output=new TAAudioSDL;
#else
      logE("Furnace was not compiled with SDL support either!");
      output=new TAAudio;
#endif
#else
      output=new TAAudioJACK;
#endif
      break;
    case DIV_AUDIO_PORTAUDIO:
#ifndef HAVE_PA
      logE("Furnace was not compiled with PortAudio!");
      setConf("audioEngine","SDL");
      saveConf();
#ifdef HAVE_SDL2
      output=new TAAudioSDL;
#else
      logE("Furnace was not compiled with SDL support either!");
      output=new TAAudio;
#endif
#else
      output=new TAAudioPA;
#endif
      break;
    case DIV_AUDIO_SDL:
#ifdef HAVE_SDL2
      output=new TAAudioSDL;
#else
      logE("Furnace was not compiled with SDL support!");
      output=new TAAudio;
#endif
      break;
    case DIV_AUDIO_PIPE:
      output=new TAAudioPipe;
      break;
    case DIV_AUDIO_DUMMY:
      output=new TAAudio;
      break;
    default:
      logE("invalid audio engine!");
      return false;
  }

  logV("listing audio devices");
  audioDevs=output->listAudioDevices();

  want.deviceName=getConfString("audioDevice","");
  want.bufsize=getConfInt("audioBufSize",1024);
  want.rate=getConfInt("audioRate",44100);
  want.fragments=2;
  want.inChans=0;
  want.outChans=getConfInt("audioChans",2);
  want.outFormat=TA_AUDIO_FORMAT_F32;
  want.wasapiEx=getConfInt("wasapiEx",0);
  want.name="Furnace";

  if (want.outChans<1) want.outChans=1;
  if (want.outChans>16) want.outChans=16;

  logV("setting callback");
  output->setCallback(process,this);

  logV("calling init");
  if (!output->init(want,got)) {
    logE("error while initializing audio!");
    delete output;
    output=NULL;
    audioEngine=DIV_AUDIO_NULL;
    return false;
  }

  logV("allocating oscBuf...");
  for (int i=0; i<got.outChans; i++) {
    if (oscBuf[i]==NULL) {
      oscBuf[i]=new float[32768];
    }
    memset(oscBuf[i],0,32768*sizeof(float));
  }

  logI("initializing MIDI.");
  if (output->initMidi(false)) {
    midiIns=output->midiIn->listDevices();
    midiOuts=output->midiOut->listDevices();
  } else {
    logW("error while initializing MIDI!");
  }
  if (output->midiIn) {
    String inName=getConfString("midiInDevice","");
    if (!inName.empty()) {
      // try opening device
      logI("opening MIDI input.");
      if (!output->midiIn->openDevice(inName)) {
        logW("could not open MIDI input device!");
      }
    } else {
      logV("no MIDI input device selected.");
    }
  }
  if (output->midiOut) {
    String outName=getConfString("midiOutDevice","");
    if (!outName.empty()) {
      // try opening device
      logI("opening MIDI output.");
      if (!output->midiOut->openDevice(outName)) {
        logW("could not open MIDI output device!");
      }
    } else {
      logV("no MIDI output device selected.");
    }
  }

  logV("initAudioBackend done");
  return true;
}

bool DivEngine::deinitAudioBackend(bool dueToSwitchMaster) {
  if (output!=NULL) {
    logI("closing audio output.");
    output->quit();
    if (output->midiIn) {
      if (output->midiIn->isDeviceOpen()) {
        logI("closing MIDI input.");
        output->midiIn->closeDevice();
      }
    }
    if (output->midiOut) {
      if (output->midiOut->isDeviceOpen()) {
        logI("closing MIDI output.");
        output->midiOut->closeDevice();
      }
    }
    output->quitMidi();
    delete output;
    output=NULL;
    if (dueToSwitchMaster) {
      audioEngine=DIV_AUDIO_NULL;
    }
  }
  return true;
}

bool DivEngine::prePreInit() {
  // init config
  initConfDir();
  logD("config path: %s",configPath.c_str());

  configLoaded=true;
  return loadConf();
}

bool DivEngine::preInit(bool noSafeMode) {
  bool wantSafe=false;
  if (!configLoaded) prePreInit();

  logI("Furnace version " DIV_VERSION ".");

  // register systems
  if (!systemsRegistered) registerSystems();

  // register ROM exports
  if (!romExportsRegistered) registerROMExports();

  // TODO: re-enable with a better approach
  // see issue #1581
  /*
  if (!noSafeMode) {
    String safeModePath=configPath+DIR_SEPARATOR_STR+"safemode";
    if (touchFile(safeModePath.c_str())==-EEXIST) {
      wantSafe=true;
    }
  }
  */

  String logPath=configPath+DIR_SEPARATOR_STR+"furnace.log";
  startLogFile(logPath.c_str());

  if (!conf.has("opn1Core")) {
    if (conf.has("opnCore")) {
      conf.set("opn1Core",conf.getString("opnCore",""));
    }
  }
  if (!conf.has("opnaCore")) {
    if (conf.has("opnCore")) {
      conf.set("opnaCore",conf.getString("opnCore",""));
    }
  }
  if (!conf.has("opnbCore")) {
    if (conf.has("opnCore")) {
      conf.set("opnbCore",conf.getString("opnCore",""));
    }
  }

#ifdef HAVE_SDL2
  String audioDriver=getConfString("sdlAudioDriver","");
  if (!audioDriver.empty()) {
    SDL_SetHint(SDL_HINT_AUDIODRIVER,audioDriver.c_str());
  }
#endif

  if (wantSafe) {
    logW("requesting safe mode.");
  }

  return wantSafe;
}

void DivEngine::everythingOK() {
  // TODO: re-enable with a better approach
  // see issue #1581
  /*
  String safeModePath=configPath+DIR_SEPARATOR_STR+"safemode";
  deleteFile(safeModePath.c_str());
  */
}

bool DivEngine::init() {
  loadSampleROMs();

  // set default system preset
  if (!hasLoadedSomething) {
    logD("setting default preset");
    String preset=getConfString("initialSys2","");
    bool oldVol=getConfInt("configVersion",DIV_ENGINE_VERSION)<135;
    if (preset.empty()) {
      // try loading old preset
      logD("trying to load old preset");
      preset=decodeSysDesc(getConfString("initialSys",""));
      oldVol=false;
    }
    logD("preset size %ld",preset.size());
    if (preset.size()>0 && (preset.size()&3)==0) {
      initSongWithDesc(preset.c_str(),true,oldVol);
    }
    String sysName=getConfString("initialSysName","");
    if (sysName=="") {
      song.systemName=getSongSystemLegacyName(song,!getConfInt("noMultiSystem",0));
    } else {
      song.systemName=sysName;
    }
    hasLoadedSomething=true;
  }

  // init the rest of engine
  bool haveAudio=false;
  if (!initAudioBackend()) {
    logE("no audio output available!");
  } else {
    haveAudio=true;
  }

  logV("creating blip_buf");

  samp_bb=blip_new(32768);
  if (samp_bb==NULL) {
    logE("not enough memory!");
    return false;
  }
  blip_set_dc(samp_bb,0);

  samp_bbOut=new short[32768];

  samp_bbIn=new short[32768];
  samp_bbInLen=32768;

  metroBuf=new float[8192];
  metroBufLen=8192;

  logV("setting blip rate of samp_bb (%f)",got.rate);
  
  blip_set_rates(samp_bb,44100,got.rate);

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }
  for (int i=0; i<128; i++) {
    tremTable[i]=255*0.5*(1.0-cos(((double)i/128.0)*(2*M_PI)));
  }
  for (int i=0; i<4096; i++) {
    reversePitchTable[i]=round(1024.0*pow(2.0,(2048.0-(double)i)/(12.0*128.0)));
    pitchTable[i]=round(1024.0*pow(2.0,((double)i-2048.0)/(12.0*128.0)));
  }

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
    keyHit[i]=false;
  }

  initDispatch();
  renderSamples();
  reset();
  active=true;

  if (!haveAudio) {
    return false;
  } else {
    if (output==NULL) {
      logE("output is NULL!");
      return false;
    }
    if (!output->setRun(true)) {
      logE("error while activating!");
      return false;
    }
  }
  return true;
}

bool DivEngine::quit(bool saveConfig) {
  deinitAudioBackend();
  quitDispatch();
  if (saveConfig) {
    logI("saving config.");
    saveConf();
  }
  active=false;
  for (int i=0; i<DIV_MAX_OUTPUTS; i++) {
    if (oscBuf[i]!=NULL) delete[] oscBuf[i];
  }
  if (metroBuf!=NULL) {
    delete[] metroBuf;
    metroBuf=NULL;
    metroBufLen=0;
  }
  if (yrw801ROM!=NULL) delete[] yrw801ROM;
  if (tg100ROM!=NULL) delete[] tg100ROM;
  if (mu5ROM!=NULL) delete[] mu5ROM;
  song.unload();
  return true;
}
