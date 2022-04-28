/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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
#include "engine.h"
#include "instrument.h"
#include "safeReader.h"
#include "../ta-log.h"
#include "../fileutils.h"
#include "../audio/sdl.h"
#include <stdexcept>
#ifndef _WIN32
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>
#endif
#ifdef HAVE_JACK
#include "../audio/jack.h"
#endif
#include <math.h>
#include <sndfile.h>
#include <fmt/printf.h>

void process(void* u, float** in, float** out, int inChans, int outChans, unsigned int size) {
  ((DivEngine*)u)->nextBuf(in,out,inChans,outChans,size);
}

const char* DivEngine::getEffectDesc(unsigned char effect, int chan, bool notNull) {
  switch (effect) {
    case 0x00:
      return "00xy: Arpeggio";
    case 0x01:
      return "01xx: Pitch slide up";
    case 0x02:
      return "02xx: Pitch slide down";
    case 0x03:
      return "03xx: Portamento";
    case 0x04:
      return "04xy: Vibrato (x: speed; y: depth)";
    case 0x07:
      return "07xy: Tremolo (x: speed; y: depth)";
    case 0x08:
      return "08xy: Set panning (x: left; y: right)";
    case 0x09:
      return "09xx: Set speed 1";
    case 0x0a:
      return "0Axy: Volume slide (0y: down; x0: up)";
    case 0x0b:
      return "0Bxx: Jump to pattern";
    case 0x0c:
      return "0Cxx: Retrigger";
    case 0x0d:
      return "0Dxx: Jump to next pattern";
    case 0x0f:
      return "0Fxx: Set speed 2";
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
      return "Cxxx: Set tick rate (hz)";
    case 0xe0:
      return "E0xx: Set arp speed";
    case 0xe1:
      return "E1xy: Note slide up (x: speed; y: semitones)";
    case 0xe2:
      return "E2xy: Note slide down (x: speed; y: semitones)";
    case 0xe3:
      return "E3xx: Set vibrato shape (0: up/down; 1: up only; 2: down only)";
    case 0xe4:
      return "E4xx: Set vibrato range";
    case 0xe5:
      return "E5xx: Set pitch (80: center)";
    case 0xea:
      return "EAxx: Legato";
    case 0xeb:
      return "EBxx: Set sample bank";
    case 0xec:
      return "ECxx: Note cut";
    case 0xed:
      return "EDxx: Note delay";
    case 0xee:
      return "EExx: Send external command";
    case 0xef:
      return "EFxx: Set global tuning (quirky!)";
    case 0xf0:
      return "F0xx: Set tick rate (bpm)";
    case 0xf1:
      return "F1xx: Single tick note slide up";
    case 0xf2:
      return "F2xx: Single tick note slide down";
    case 0xf3:
      return "F3xx: Fine volume slide up";
    case 0xf4:
      return "F4xx: Fine volume slide down";
    case 0xf8:
      return "F8xx: Single tick volume slide up";
    case 0xf9:
      return "F9xx: Single tick volume slide down";
    case 0xfa:
      return "FAxx: Fast volume slide (0y: down; x0: up)";
    case 0xff:
      return "FFxx: Stop song";
    default:
      if ((effect&0xf0)==0x90) {
        return "9xxx: Set sample offset*256";
      } else if (chan>=0 && chan<chans) {
        const char* ret=disCont[dispatchOfChan[chan]].dispatch->getEffectName(effect);
        if (ret!=NULL) return ret;
      }
      break;
  }
  return notNull?"Invalid effect":NULL;
}

void DivEngine::walkSong(int& loopOrder, int& loopRow, int& loopEnd) {
  loopOrder=0;
  loopRow=0;
  loopEnd=-1;
  int nextOrder=-1;
  int nextRow=0;
  int effectVal=0;
  DivPattern* pat[DIV_MAX_CHANS];
  for (int i=0; i<song.ordersLen; i++) {
    for (int j=0; j<chans; j++) {
      pat[j]=song.pat[j].getPattern(song.orders.ord[j][i],false);
    }
    for (int j=nextRow; j<song.patLen; j++) {
      nextRow=0;
      for (int k=0; k<chans; k++) {
        for (int l=0; l<song.pat[k].effectCols; l++) {
          effectVal=pat[k]->data[j][5+(l<<1)];
          if (effectVal<0) effectVal=0;
          if (pat[k]->data[j][4+(l<<1)]==0x0d) {
            if (nextOrder==-1 && (i<song.ordersLen-1 || !song.ignoreJumpAtEnd)) {
              nextOrder=i+1;
              nextRow=effectVal;
            }
          } else if (pat[k]->data[j][4+(l<<1)]==0x0b) {
            if (nextOrder==-1) {
              nextOrder=effectVal;
              nextRow=0;
            }
          }
        }
      }
      if (nextOrder!=-1) {
        if (nextOrder<=i) {
          loopOrder=nextOrder;
          loopRow=nextRow;
          loopEnd=i;
          return;
        }
        i=nextOrder-1;
        nextOrder=-1;
        break;
      }
    }
  }
}

void _runExportThread(DivEngine* caller) {
  caller->runExportThread();
}

bool DivEngine::isExporting() {
  return exporting;
}

#define EXPORT_BUFSIZE 2048

void DivEngine::runExportThread() {
  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      SNDFILE* sf;
      SF_INFO si;
      si.samplerate=got.rate;
      si.channels=2;
      si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

      sf=sf_open(exportPath.c_str(),SFM_WRITE,&si);
      if (sf==NULL) {
        logE("could not open file for writing! (%s)",sf_strerror(NULL));
        exporting=false;
        return;
      }

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to file...");

      while (playing) {
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        for (int i=0; i<EXPORT_BUFSIZE; i++) {
          outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]));
          outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]));
        }
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
        }
        if (sf_writef_float(sf,outBuf[2],totalProcessed)!=(int)totalProcessed) {
          logE("error: failed to write entire buffer!");
          break;
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      if (sf_close(sf)!=0) {
        logE("could not close audio file!");
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      SNDFILE* sf[32];
      SF_INFO si[32];
      String fname[32];
      for (int i=0; i<song.systemLen; i++) {
        sf[i]=NULL;
        si[i].samplerate=got.rate;
        if (disCont[i].dispatch->isStereo()) {
          si[i].channels=2;
        } else {
          si[i].channels=1;
        }
        si[i].format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
      }

      for (int i=0; i<song.systemLen; i++) {
        fname[i]=fmt::sprintf("%s_s%02d.wav",exportPath,i+1);
        logI("- %s",fname[i].c_str());
        sf[i]=sf_open(fname[i].c_str(),SFM_WRITE,&si[i]);
        if (sf[i]==NULL) {
          logE("could not open file for writing! (%s)",sf_strerror(NULL));
          for (int j=0; j<i; j++) {
            sf_close(sf[i]);
          }
          return;
        }
      }

      float* outBuf[2];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      short* sysBuf=new short[EXPORT_BUFSIZE*2];

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to files...");

      while (playing) {
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        for (int i=0; i<song.systemLen; i++) {
          for (int j=0; j<EXPORT_BUFSIZE; j++) {
            if (!disCont[i].dispatch->isStereo()) {
              sysBuf[j]=disCont[i].bbOut[0][j];
            } else {
              sysBuf[j<<1]=disCont[i].bbOut[0][j];
              sysBuf[1+(j<<1)]=disCont[i].bbOut[1][j];
            }
          }
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! (%d) %d>%d",i,totalProcessed,EXPORT_BUFSIZE);
          }
          if (sf_writef_short(sf[i],sysBuf,totalProcessed)!=(int)totalProcessed) {
            logE("error: failed to write entire buffer! (%d)",i);
            break;
          }
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] sysBuf;

      for (int i=0; i<song.systemLen; i++) {
        if (sf_close(sf[i])!=0) {
          logE("could not close audio file!");
        }
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      // take control of audio output
      deinitAudioBackend();

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];
      int loopCount=remainingLoops;

      logI("rendering to files...");
      
      for (int i=0; i<chans; i++) {
        SNDFILE* sf;
        SF_INFO si;
        String fname=fmt::sprintf("%s_c%02d.wav",exportPath,i+1);
        logI("- %s",fname.c_str());
        si.samplerate=got.rate;
        si.channels=2;
        si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

        sf=sf_open(fname.c_str(),SFM_WRITE,&si);
        if (sf==NULL) {
          logE("could not open file for writing! (%s)",sf_strerror(NULL));
          break;
        }

        for (int j=0; j<chans; j++) {
          bool mute=(j!=i);
          isMuted[j]=mute;
        }
        if (getChannelType(i)==5) {
          for (int j=i; j<chans; j++) {
            if (getChannelType(j)!=5) break;
            isMuted[j]=false;
          }
        }
        for (int j=0; j<chans; j++) {
          if (disCont[dispatchOfChan[j]].dispatch!=NULL) {
            disCont[dispatchOfChan[j]].dispatch->muteChannel(dispatchChanOfChan[j],isMuted[j]);
          }
        }
        
        curOrder=0;
        remainingLoops=loopCount;
        playSub(false);

        while (playing) {
          nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
          for (int j=0; j<EXPORT_BUFSIZE; j++) {
            outBuf[2][j<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][j]));
            outBuf[2][1+(j<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][j]));
          }
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
          }
          if (sf_writef_float(sf,outBuf[2],totalProcessed)!=(int)totalProcessed) {
            logE("error: failed to write entire buffer!");
            break;
          }
        }

        if (sf_close(sf)!=0) {
          logE("could not close audio file!");
        }

        if (getChannelType(i)==5) {
          i++;
          while (true) {
            if (i>=chans) break;
            if (getChannelType(i)!=5) break;
          }
          i--;
        }

        if (stopExport) break;
      }
      exporting=false;

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      for (int i=0; i<chans; i++) {
        isMuted[i]=false;
        if (disCont[dispatchOfChan[i]].dispatch!=NULL) {
          disCont[dispatchOfChan[i]].dispatch->muteChannel(dispatchChanOfChan[i],false);
        }
      }

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      break;
    }
  }
  stopExport=false;
}

bool DivEngine::saveAudio(const char* path, int loops, DivAudioExportModes mode) {
  exportPath=path;
  exportMode=mode;
  if (exportMode!=DIV_EXPORT_MODE_ONE) {
    // remove extension
    String lowerCase=exportPath;
    for (char& i: lowerCase) {
      if (i>='A' && i<='Z') i+='a'-'A';
    }
    size_t extPos=lowerCase.rfind(".wav");
    if (extPos!=String::npos) {
      exportPath=exportPath.substr(0,extPos);
    }
  }
  exporting=true;
  stopExport=false;
  stop();
  repeatPattern=false;
  setOrder(0);
  remainingLoops=loops;
  exportThread=new std::thread(_runExportThread,this);
  return true;
}

void DivEngine::waitAudioFile() {
  if (exportThread!=NULL) {
    exportThread->join();
  }
}

bool DivEngine::haltAudioFile() {
  stopExport=true;
  stop();
  return true;
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

void DivEngine::renderSamplesP() {
  BUSY_BEGIN;
  renderSamples();
  BUSY_END;
}

void DivEngine::renderSamples() {
  sPreview.sample=-1;
  sPreview.pos=0;

  // step 1: render samples
  for (int i=0; i<song.sampleLen; i++) {
    song.sample[i]->render();
  }

  // step 2: allocate ADPCM-A samples
  if (adpcmAMem==NULL) adpcmAMem=new unsigned char[16777216];

  size_t memPos=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    int paddedLen=(s->lengthA+255)&(~0xff);
    if ((memPos&0xf00000)!=((memPos+paddedLen)&0xf00000)) {
      memPos=(memPos+0xfffff)&0xf00000;
    }
    if (memPos>=16777216) {
      logW("out of ADPCM-A memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=16777216) {
      memcpy(adpcmAMem+memPos,s->dataA,16777216-memPos);
      logW("out of ADPCM-A memory for sample %d!",i);
    } else {
      memcpy(adpcmAMem+memPos,s->dataA,paddedLen);
    }
    s->offA=memPos;
    memPos+=paddedLen;
  }
  adpcmAMemLen=memPos+256;

  // step 2: allocate ADPCM-B samples
  if (adpcmBMem==NULL) adpcmBMem=new unsigned char[16777216];

  memPos=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    int paddedLen=(s->lengthB+255)&(~0xff);
    if ((memPos&0xf00000)!=((memPos+paddedLen)&0xf00000)) {
      memPos=(memPos+0xfffff)&0xf00000;
    }
    if (memPos>=16777216) {
      logW("out of ADPCM-B memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=16777216) {
      memcpy(adpcmBMem+memPos,s->dataB,16777216-memPos);
      logW("out of ADPCM-B memory for sample %d!",i);
    } else {
      memcpy(adpcmBMem+memPos,s->dataB,paddedLen);
    }
    s->offB=memPos;
    memPos+=paddedLen;
  }
  adpcmBMemLen=memPos+256;

  // step 4: allocate qsound pcm samples
  if (qsoundMem==NULL) qsoundMem=new unsigned char[16777216];
  memset(qsoundMem,0,16777216);

  memPos=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    int length=s->length8;
    if (length>65536-16) {
      length=65536-16;
    }
    if ((memPos&0xff0000)!=((memPos+length)&0xff0000)) {
      memPos=(memPos+0xffff)&0xff0000;
    }
    if (memPos>=16777216) {
      logW("out of QSound PCM memory for sample %d!",i);
      break;
    }
    if (memPos+length>=16777216) {
      for (unsigned int i=0; i<16777216-(memPos+length); i++) {
        qsoundMem[(memPos+i)^0x8000]=s->data8[i];
      }
      logW("out of QSound PCM memory for sample %d!",i);
    } else {
      for (int i=0; i<length; i++) {
        qsoundMem[(memPos+i)^0x8000]=s->data8[i];
      }
    }
    s->offQSound=memPos^0x8000;
    memPos+=length+16;
  }
  qsoundMemLen=memPos+256;

  // step 4: allocate x1-010 pcm samples
  if (x1_010Mem==NULL) x1_010Mem=new unsigned char[1048576];
  memset(x1_010Mem,0,1048576);

  memPos=0;
  for (int i=0; i<song.sampleLen; i++) {
    DivSample* s=song.sample[i];
    int paddedLen=(s->length8+4095)&(~0xfff);
    // fit sample bank size to 128KB for Seta 2 external bankswitching logic (not emulated yet!)
    if (paddedLen>131072) {
      paddedLen=131072;
    }
    if ((memPos&0xfe0000)!=((memPos+paddedLen)&0xfe0000)) {
      memPos=(memPos+0x1ffff)&0xfe0000;
    }
    if (memPos>=1048576) {
      logW("out of X1-010 memory for sample %d!",i);
      break;
    }
    if (memPos+paddedLen>=1048576) {
      memcpy(x1_010Mem+memPos,s->data8,1048576-memPos);
      logW("out of X1-010 memory for sample %d!",i);
    } else {
      memcpy(x1_010Mem+memPos,s->data8,paddedLen);
    }
    s->offX1_010=memPos;
    memPos+=paddedLen;
  }
  x1_010MemLen=memPos+256;
}

void DivEngine::createNew(const int* description) {
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.unload();
  song=DivSong();
  if (description!=NULL) {
    if (description[0]!=0) {
      int index=0;
      for (int i=0; description[i]; i+=4) {
        song.system[index]=(DivSystem)description[i];
        song.systemVol[index]=description[i+1];
        song.systemPan[index]=description[i+2];
        song.systemFlags[index]=description[i+3];
        index++;
        if (index>=32) break;
      }
      song.systemLen=index;
    }
  }
  recalcChans();
  renderSamples();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  reset();
  BUSY_END;
}

void DivEngine::swapChannels(int src, int dest) {
  logV("swapping channel %d with %d",src,dest);
  if (src==dest) {
    logV("not swapping channels because it's the same channel!",src,dest);
    return;
  }

  for (int i=0; i<256; i++) {
    song.orders.ord[dest][i]^=song.orders.ord[src][i];
    song.orders.ord[src][i]^=song.orders.ord[dest][i];
    song.orders.ord[dest][i]^=song.orders.ord[src][i];

    DivPattern* prev=song.pat[src].data[i];
    song.pat[src].data[i]=song.pat[dest].data[i];
    song.pat[dest].data[i]=prev;
  }

  song.pat[src].effectCols^=song.pat[dest].effectCols;
  song.pat[dest].effectCols^=song.pat[src].effectCols;
  song.pat[src].effectCols^=song.pat[dest].effectCols;

  String prevChanName=song.chanName[src];
  String prevChanShortName=song.chanShortName[src];
  bool prevChanShow=song.chanShow[src];
  bool prevChanCollapse=song.chanCollapse[src];

  song.chanName[src]=song.chanName[dest];
  song.chanShortName[src]=song.chanShortName[dest];
  song.chanShow[src]=song.chanShow[dest];
  song.chanCollapse[src]=song.chanCollapse[dest];
  song.chanName[dest]=prevChanName;
  song.chanShortName[dest]=prevChanShortName;
  song.chanShow[dest]=prevChanShow;
  song.chanCollapse[dest]=prevChanCollapse;
}

void DivEngine::stompChannel(int ch) {
  logV("stomping channel %d",ch);
  for (int i=0; i<256; i++) {
    song.orders.ord[ch][i]=0;
  }
  song.pat[ch].wipePatterns();
  song.pat[ch].effectCols=1;
  song.chanName[ch]="";
  song.chanShortName[ch]="";
  song.chanShow[ch]=true;
  song.chanCollapse[ch]=false;
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

void DivEngine::changeSystem(int index, DivSystem which, bool preserveOrder) {
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
  song.systemFlags[index]=0;
  recalcChans();
  saveLock.unlock();
  BUSY_END;
  initDispatch();
  BUSY_BEGIN;
  renderSamples();
  reset();
  BUSY_END;
}

bool DivEngine::addSystem(DivSystem which) {
  if (song.systemLen>32) {
    lastError="max number of systems is 32";
    return false;
  }
  // this was DIV_MAX_CHANS but I am setting it to 63 for now due to an ImGui limitation
  if (chans+getChannelCount(which)>63) {
    lastError="max number of total channels is 63";
    return false;
  }
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.system[song.systemLen]=which;
  song.systemVol[song.systemLen]=64;
  song.systemPan[song.systemLen]=0;
  song.systemFlags[song.systemLen++]=0;
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

bool DivEngine::removeSystem(int index, bool preserveOrder) {
  if (song.systemLen<=1) {
    lastError="cannot remove the last one";
    return false;
  }
  if (index<0 || index>=song.systemLen) {
    lastError="invalid index";
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

  song.system[index]=DIV_SYSTEM_NULL;
  song.systemLen--;
  for (int i=index; i<song.systemLen; i++) {
    song.system[i]=song.system[i+1];
    song.systemVol[i]=song.systemVol[i+1];
    song.systemPan[i]=song.systemPan[i+1];
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

void DivEngine::setLoops(int loops) {
  remainingLoops=loops;
}

DivChannelState* DivEngine::getChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return &chan[ch];
}

void* DivEngine::getDispatchChanState(int ch) {
  if (ch<0 || ch>=chans) return NULL;
  return disCont[dispatchOfChan[ch]].dispatch->getChanState(dispatchChanOfChan[ch]);
}

unsigned char* DivEngine::getRegisterPool(int sys, int& size, int& depth) {
  if (sys<0 || sys>=song.systemLen) return NULL;
  if (disCont[sys].dispatch==NULL) return NULL;
  size=disCont[sys].dispatch->getRegisterPoolSize();
  depth=disCont[sys].dispatch->getRegisterPoolDepth();
  return disCont[sys].dispatch->getRegisterPool();
}

void DivEngine::enableCommandStream(bool enable) {
  cmdStreamEnabled=enable;
}

void DivEngine::getCommandStream(std::vector<DivCommand>& where) {
  BUSY_BEGIN;
  where.clear();
  for (DivCommand& i: cmdStream) {
    where.push_back(i);
  }
  cmdStream.clear();
  BUSY_END;
}

void DivEngine::playSub(bool preserveDrift, int goalRow) {
  std::chrono::high_resolution_clock::time_point timeStart=std::chrono::high_resolution_clock::now();
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  reset();
  if (preserveDrift && curOrder==0) return;
  bool oldRepeatPattern=repeatPattern;
  repeatPattern=false;
  int goal=curOrder;
  curOrder=0;
  curRow=0;
  stepPlay=0;
  int prevDrift;
  prevDrift=clockDrift;
  clockDrift=0;
  cycles=0;
  if (preserveDrift) {
    endOfSong=false;
  } else {
    ticks=1;
    totalTicks=0;
    totalSeconds=0;
    totalTicksR=0;
  }
  speedAB=false;
  playing=true;
  skipping=true;
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(true);
  while (playing && curOrder<goal) {
    if (nextTick(preserveDrift)) {
      skipping=false;
      return;
    }
  }
  int oldOrder=curOrder;
  while (playing && curRow<goalRow) {
    if (nextTick(preserveDrift)) {
      skipping=false;
      return;
    }
    if (oldOrder!=curOrder) break;
  }
  for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->setSkipRegisterWrites(false);
  if (goal>0 || goalRow>0) {
    for (int i=0; i<song.systemLen; i++) disCont[i].dispatch->forceIns();
  }
  for (int i=0; i<chans; i++) {
    chan[i].cut=-1;
  }
  repeatPattern=oldRepeatPattern;
  if (preserveDrift) {
    clockDrift=prevDrift;
  } else {
    clockDrift=0;
    cycles=0;
  }
  if (!preserveDrift) {
    ticks=1;
    subticks=1;
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
  double base=(period?(song.tuning*0.0625):song.tuning)*pow(2.0,(float)(note+3)/12.0);
  return period?
         (clock/base)/divider:
         base*(divider/clock);
}

unsigned short DivEngine::calcBaseFreqFNumBlock(double clock, double divider, int note, int bits) {
  int bf=calcBaseFreq(clock,divider,note,false);
  int block=note/12;
  if (block<0) block=0;
  if (block>7) block=7;
  bf>>=block;
  if (bf<0) bf=0;
  // octave boundaries
  while (bf>0 && bf<644 && block>0) {
    bf<<=1;
    block--;
  }
  if (bf>1288) {
    while (block<7) {
      bf>>=1;
      block++;
    }
    if (bf>((1<<bits)-1)) {
      bf=(1<<bits)-1;
    }
  }
  return bf|(block<<bits);
}

int DivEngine::calcFreq(int base, int pitch, bool period, int octave, int pitch2) {
  if (song.linearPitch) {
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

void DivEngine::play() {
  BUSY_BEGIN_SOFT;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  if (stepPlay==0) {
    freelance=false;
    playSub(false);
  } else {
    stepPlay=0;
  }
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    keyHit[i]=false;
  }
  if (output) if (!skipping && output->midiOut!=NULL) {
    int pos=totalTicksR/6;
    output->midiOut->send(TAMidiMessage(TA_MIDI_POSITION,(pos>>7)&0x7f,pos&0x7f));
    output->midiOut->send(TAMidiMessage(TA_MIDI_MACHINE_PLAY,0,0));
  }
  BUSY_END;
}

void DivEngine::playToRow(int row) {
  BUSY_BEGIN_SOFT;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
  freelance=false;
  playSub(false,row);
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    keyHit[i]=false;
  }
  BUSY_END;
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
  BUSY_END;
}

void DivEngine::stop() {
  BUSY_BEGIN;
  freelance=false;
  playing=false;
  extValuePresent=false;
  stepPlay=0;
  remainingLoops=-1;
  sPreview.sample=-1;
  sPreview.wave=-1;
  sPreview.pos=0;
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
    chans+=chanCount;
    for (int j=0; j<chanCount; j++) {
      sysOfChan[chanIndex]=song.system[i];
      dispatchOfChan[chanIndex]=i;
      dispatchChanOfChan[chanIndex]=j;
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
}

void DivEngine::reset() {
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    chan[i]=DivChannelState();
    if (i<chans) chan[i].volMax=(disCont[dispatchOfChan[i]].dispatch->dispatch(DivCommand(DIV_CMD_GET_VOLMAX,dispatchChanOfChan[i]))<<8)|0xff;
    chan[i].volume=chan[i].volMax;
    if (!song.linearPitch) chan[i].vibratoFine=4;
  }
  extValue=0;
  extValuePresent=0;
  speed1=song.speed1;
  speed2=song.speed2;
  firstTick=false;
  nextSpeed=speed1;
  divider=60;
  if (song.customTempo) {
    divider=song.hz;
  } else {
    if (song.pal) {
      divider=60;
    } else {
      divider=50;
    }
  }
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
    default:
      break;
  }
  return rate;
}

void DivEngine::previewSample(int sample, int note) {
  BUSY_BEGIN;
  if (sample<0 || sample>=(int)song.sample.size()) {
    sPreview.sample=-1;
    sPreview.pos=0;
    BUSY_END;
    return;
  }
  blip_clear(samp_bb);
  double rate=song.sample[sample]->rate;
  if (note>=0) {
    rate=(song.tuning*pow(2.0,(double)(note+3)/12.0)*((double)song.sample[sample]->centerRate/8363.0));
    if (rate<=0) rate=song.sample[sample]->rate;
  }
  if (rate<100) rate=100;
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.pos=0;
  sPreview.sample=sample;
  sPreview.wave=-1;
  BUSY_END;
}

void DivEngine::stopSamplePreview() {
  BUSY_BEGIN;
  sPreview.sample=-1;
  sPreview.pos=0;
  BUSY_END;
}

void DivEngine::previewWave(int wave, int note) {
  BUSY_BEGIN;
  if (wave<0 || wave>=(int)song.wave.size()) {
    sPreview.wave=-1;
    sPreview.pos=0;
    BUSY_END;
    return;
  }
  if (song.wave[wave]->len<=0) {
    BUSY_END;
    return;
  }
  blip_clear(samp_bb);
  double rate=song.wave[wave]->len*((song.tuning*0.0625)*pow(2.0,(double)(note+3)/12.0));
  if (rate<100) rate=100;
  blip_set_rates(samp_bb,rate,got.rate);
  samp_prevSample=0;
  sPreview.pos=0;
  sPreview.sample=-1;
  sPreview.wave=wave;
  BUSY_END;
}

void DivEngine::stopWavePreview() {
  BUSY_BEGIN;
  sPreview.wave=-1;
  sPreview.pos=0;
  BUSY_END;
}

String DivEngine::getConfigPath() {
  return configPath;
}

int DivEngine::getMaxVolumeChan(int ch) {
  return chan[ch].volMax>>8;
}

unsigned char DivEngine::getOrder() {
  return curOrder;
}

int DivEngine::getRow() {
  return curRow;
}

unsigned char DivEngine::getSpeed1() {
  return speed1;
}

unsigned char DivEngine::getSpeed2() {
  return speed2;
}

float DivEngine::getHz() {
  if (song.customTempo) {
    return song.hz;
  } else if (song.pal) {
    return 60.0;
  } else {
    return 50.0;
  }
  return 60.0;
}

float DivEngine::getCurHz() {
  return divider;
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

int DivEngine::addInstrument(int refChan) {
  BUSY_BEGIN;
  DivInstrument* ins=new DivInstrument;
  int insCount=(int)song.ins.size();
  DivInstrumentType prefType=getPreferInsType(refChan);
  switch (prefType) {
    case DIV_INS_OPLL:
      *ins=song.nullInsOPLL;
      break;
    case DIV_INS_OPL:
      *ins=song.nullInsOPL;
      break;
    default:
      break;
  }
  ins->name=fmt::sprintf("Instrument %d",insCount);
  ins->type=prefType;
  saveLock.lock();
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  saveLock.unlock();
  BUSY_END;
  return insCount;
}

int DivEngine::addInstrumentPtr(DivInstrument* which) {
  BUSY_BEGIN;
  saveLock.lock();
  song.ins.push_back(which);
  song.insLen=song.ins.size();
  saveLock.unlock();
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

void DivEngine::delInstrument(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  if (index>=0 && index<(int)song.ins.size()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].dispatch->notifyInsDeletion(song.ins[index]);
    }
    delete song.ins[index];
    song.ins.erase(song.ins.begin()+index);
    song.insLen=song.ins.size();
    for (int i=0; i<chans; i++) {
      for (int j=0; j<256; j++) {
        if (song.pat[i].data[j]==NULL) continue;
        for (int k=0; k<song.patLen; k++) {
          if (song.pat[i].data[j]->data[k][2]>index) {
            song.pat[i].data[j]->data[k][2]--;
          }
        }
      }
    }
  }
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addWave() {
  BUSY_BEGIN;
  saveLock.lock();
  DivWavetable* wave=new DivWavetable;
  int waveCount=(int)song.wave.size();
  song.wave.push_back(wave);
  song.waveLen=waveCount+1;
  saveLock.unlock();
  BUSY_END;
  return waveCount;
}

bool DivEngine::addWaveFromFile(const char* path) {
  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    return false;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    fclose(f);
    return false;
  }
  len=ftell(f);
  if (len<0) {
    fclose(f);
    return false;
  }
  if (len==(SIZE_MAX>>1)) {
    fclose(f);
    return false;
  }
  if (len==0) {
    fclose(f);
    return false;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    fclose(f);
    return false;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire wavetable file buffer!");
    delete[] buf;
    return false;
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
        lastError="invalid wavetable header/data!";
        delete wave;
        delete[] buf;
        return false;
      }
    } else {
      try {
        // read as .dmw
        reader.seek(0,SEEK_SET);
        int len=reader.readI();
        wave->max=(unsigned char)reader.readC();
        if (wave->max==255) { // new wavetable format
          unsigned char waveVersion=reader.readC();
          logI("reading modern .dmw...");
          logD("wave version %d",waveVersion);
          wave->max=reader.readC();
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
          logI("reading binary...");
          len=reader.size();
          if (len>256) len=256;
          reader.seek(0,SEEK_SET);
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
            if (wave->max<wave->data[i]) wave->max=wave->data[i];
          }
          wave->len=len;
        }
      } catch (EndOfFileException& e) {
        // read as binary
        len=reader.size();
        logI("reading binary for being too small...");
        if (len>256) len=256;
        reader.seek(0,SEEK_SET);
        for (int i=0; i<len; i++) {
          wave->data[i]=(unsigned char)reader.readC();
          if (wave->max<wave->data[i]) wave->max=wave->data[i];
        }
        wave->len=len;
      }
    }
  } catch (EndOfFileException& e) {
    delete wave;
    delete[] buf;
    return false;
  }
  
  BUSY_BEGIN;
  saveLock.lock();
  int waveCount=(int)song.wave.size();
  song.wave.push_back(wave);
  song.waveLen=waveCount+1;
  saveLock.unlock();
  BUSY_END;
  return true;
}

void DivEngine::delWave(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  if (index>=0 && index<(int)song.wave.size()) {
    delete song.wave[index];
    song.wave.erase(song.wave.begin()+index);
    song.waveLen=song.wave.size();
  }
  saveLock.unlock();
  BUSY_END;
}

int DivEngine::addSample() {
  BUSY_BEGIN;
  saveLock.lock();
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=fmt::sprintf("Sample %d",sampleCount);
  song.sample.push_back(sample);
  song.sampleLen=sampleCount+1;
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return sampleCount;
}

int DivEngine::addSampleFromFile(const char* path) {
  BUSY_BEGIN;
  warnings="";

  const char* pathRedux=strrchr(path,DIR_SEPARATOR);
  if (pathRedux==NULL) {
    pathRedux=path;
  } else {
    pathRedux++;
  }
  String stripPath;
  const char* pathReduxEnd=strrchr(pathRedux,'.');
  if (pathReduxEnd==NULL) {
    stripPath=pathRedux;
  } else {
    for (const char* i=pathRedux; i!=pathReduxEnd && (*i); i++) {
      stripPath+=*i;
    }
  }

  const char* ext=strrchr(path,'.');
  if (ext!=NULL) {
    String extS;
    for (; *ext; ext++) {
      char i=*ext;
      if (i>='A' && i<='Z') {
        i+='a'-'A';
      }
      extS+=i;
    }
    if (extS==String(".dmc")) { // read as .dmc
      size_t len=0;
      DivSample* sample=new DivSample;
      int sampleCount=(int)song.sample.size();
      sample->name=stripPath;

      FILE* f=fopen(path,"rb");
      if (f==NULL) {
        BUSY_END;
        lastError=fmt::sprintf("could not open file! (%s)",strerror(errno));
        delete sample;
        return -1;
      }

      if (fseek(f,0,SEEK_END)<0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not get file length! (%s)",strerror(errno));
        delete sample;
        return -1;
      }

      len=ftell(f);

      if (len==0) {
        fclose(f);
        BUSY_END;
        lastError="file is empty!";
        delete sample;
        return -1;
      }

      if (len==(SIZE_MAX>>1)) {
        fclose(f);
        BUSY_END;
        lastError="file is invalid!";
        delete sample;
        return -1;
      }

      if (fseek(f,0,SEEK_SET)<0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not seek to beginning of file! (%s)",strerror(errno));
        delete sample;
        return -1;
      }

      sample->rate=33144;
      sample->centerRate=33144;
      sample->depth=1;
      sample->init(len*8);

      if (fread(sample->dataDPCM,1,len,f)==0) {
        fclose(f);
        BUSY_END;
        lastError=fmt::sprintf("could not read file! (%s)",strerror(errno));
        delete sample;
        return -1;
      }

      saveLock.lock();
      song.sample.push_back(sample);
      song.sampleLen=sampleCount+1;
      saveLock.unlock();
      renderSamples();
      BUSY_END;
      return sampleCount;
    }
  }

  SF_INFO si;
  SNDFILE* f=sf_open(path,SFM_READ,&si);
  if (f==NULL) {
    BUSY_END;
    int err=sf_error(NULL);
    if (err==SF_ERR_SYSTEM) {
      lastError=fmt::sprintf("could not open file! (%s %s)",sf_error_number(err),strerror(errno));
    } else {
      lastError=fmt::sprintf("could not open file! (%s)",sf_error_number(err));
    }
    return -1;
  }
  if (si.frames>16777215) {
    lastError="this sample is too big! max sample size is 16777215.";
    sf_close(f);
    BUSY_END;
    return -1;
  }
  short* buf=new short[si.channels*si.frames];
  if (sf_readf_short(f,buf,si.frames)!=si.frames) {
    logW("sample read size mismatch!");
  }
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  sample->name=stripPath;

  int index=0;
  if ((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8) {
    sample->depth=8;
  } else {
    sample->depth=16;
  }
  sample->init(si.frames);
  for (int i=0; i<si.frames*si.channels; i+=si.channels) {
    int averaged=0;
    for (int j=0; j<si.channels; j++) {
      averaged+=buf[i+j];
    }
    averaged/=si.channels;
    if (((si.format&SF_FORMAT_SUBMASK)==SF_FORMAT_PCM_U8)) {
      sample->data8[index++]=averaged>>8;
    } else {
      sample->data16[index++]=averaged;
    }
  }
  delete[] buf;
  sample->rate=si.samplerate;
  if (sample->rate<4000) sample->rate=4000;
  if (sample->rate>96000) sample->rate=96000;
  sample->centerRate=si.samplerate;

  SF_INSTRUMENT inst;
  if (sf_command(f, SFC_GET_INSTRUMENT, &inst, sizeof(inst)) == SF_TRUE)
  {
    // There's no documentation on libsndfile detune range, but the code
    // implies -50..50. Yet when loading a file you can get a >50 value.
    if(inst.detune > 50)
      inst.detune = inst.detune - 100;
    short pitch = ((0x3c-inst.basenote)*100) + inst.detune;
    sample->centerRate=si.samplerate*pow(2.0,pitch/(12.0 * 100.0));
    if(inst.loop_count && inst.loops[0].mode == SF_LOOP_FORWARD)
    {
      sample->loopStart=inst.loops[0].start;
      if(inst.loops[0].end < (unsigned int)sampleCount)
        sampleCount=inst.loops[0].end;
    }
  }

  if (sample->centerRate<4000) sample->centerRate=4000;
  if (sample->centerRate>64000) sample->centerRate=64000;
  sf_close(f);
  saveLock.lock();
  song.sample.push_back(sample);
  song.sampleLen=sampleCount+1;
  saveLock.unlock();
  renderSamples();
  BUSY_END;
  return sampleCount;
}

void DivEngine::delSample(int index) {
  BUSY_BEGIN;
  saveLock.lock();
  if (index>=0 && index<(int)song.sample.size()) {
    delete song.sample[index];
    song.sample.erase(song.sample.begin()+index);
    song.sampleLen=song.sample.size();
    renderSamples();
  }
  saveLock.unlock();
  BUSY_END;
}

void DivEngine::addOrder(bool duplicate, bool where) {
  unsigned char order[DIV_MAX_CHANS];
  if (song.ordersLen>=0xff) return;
  BUSY_BEGIN_SOFT;
  if (duplicate) {
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      order[i]=song.orders.ord[i][curOrder];
    }
  } else {
    bool used[256];
    for (int i=0; i<chans; i++) {
      memset(used,0,sizeof(bool)*256);
      for (int j=0; j<song.ordersLen; j++) {
        used[song.orders.ord[i][j]]=true;
      }
      order[i]=0xff;
      for (int j=0; j<256; j++) {
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
      song.orders.ord[i][song.ordersLen]=order[i];
    }
    song.ordersLen++;
    saveLock.unlock();
  } else { // after current order
    saveLock.lock();
    for (int i=0; i<DIV_MAX_CHANS; i++) {
      for (int j=song.ordersLen; j>curOrder; j--) {
        song.orders.ord[i][j]=song.orders.ord[i][j-1];
      }
      song.orders.ord[i][curOrder+1]=order[i];
    }
    song.ordersLen++;
    saveLock.unlock();
    curOrder++;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  BUSY_END;
}

void DivEngine::deepCloneOrder(bool where) {
  unsigned char order[DIV_MAX_CHANS];
  if (song.ordersLen>=0xff) return;
  warnings="";
  BUSY_BEGIN_SOFT;
  for (int i=0; i<chans; i++) {
    bool didNotFind=true;
    logD("channel %d",i);
    order[i]=song.orders.ord[i][curOrder];
    // find free slot
    for (int j=0; j<256; j++) {
      logD("finding free slot in %d...",j);
      if (song.pat[i].data[j]==NULL) {
        int origOrd=order[i];
        order[i]=j;
        DivPattern* oldPat=song.pat[i].getPattern(origOrd,false);
        DivPattern* pat=song.pat[i].getPattern(j,true);
        memcpy(pat->data,oldPat->data,256*32*sizeof(short));
        logD("found at %d",j);
        didNotFind=false;
        break;
      }
    }
    if (didNotFind) {
      addWarning(fmt::sprintf("no free patterns in channel %d!",i));
    }
  }
  if (where) { // at the end
    saveLock.lock();
    for (int i=0; i<chans; i++) {
      song.orders.ord[i][song.ordersLen]=order[i];
    }
    song.ordersLen++;
    saveLock.unlock();
  } else { // after current order
    saveLock.lock();
    for (int i=0; i<chans; i++) {
      for (int j=song.ordersLen; j>curOrder; j--) {
        song.orders.ord[i][j]=song.orders.ord[i][j-1];
      }
      song.orders.ord[i][curOrder+1]=order[i];
    }
    song.ordersLen++;
    saveLock.unlock();
    curOrder++;
    if (playing && !freelance) {
      playSub(false);
    }
  }
  BUSY_END;
}

void DivEngine::deleteOrder() {
  if (song.ordersLen<=1) return;
  BUSY_BEGIN_SOFT;
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    for (int j=curOrder; j<song.ordersLen; j++) {
      song.orders.ord[i][j]=song.orders.ord[i][j+1];
    }
  }
  song.ordersLen--;
  saveLock.unlock();
  if (curOrder>=song.ordersLen) curOrder=song.ordersLen-1;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::moveOrderUp() {
  BUSY_BEGIN_SOFT;
  if (curOrder<1) {
    BUSY_END;
    return;
  }
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder-1];
    song.orders.ord[i][curOrder-1]^=song.orders.ord[i][curOrder];
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder-1];
  }
  saveLock.unlock();
  curOrder--;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::moveOrderDown() {
  BUSY_BEGIN_SOFT;
  if (curOrder>=song.ordersLen-1) {
    BUSY_END;
    return;
  }
  saveLock.lock();
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder+1];
    song.orders.ord[i][curOrder+1]^=song.orders.ord[i][curOrder];
    song.orders.ord[i][curOrder]^=song.orders.ord[i][curOrder+1];
  }
  saveLock.unlock();
  curOrder++;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::exchangeIns(int one, int two) {
  for (int i=0; i<chans; i++) {
    for (int j=0; j<256; j++) {
      if (song.pat[i].data[j]==NULL) continue;
      for (int k=0; k<song.patLen; k++) {
        if (song.pat[i].data[j]->data[k][2]==one) {
          song.pat[i].data[j]->data[k][2]=two;
        } else if (song.pat[i].data[j]->data[k][2]==two) {
          song.pat[i].data[j]->data[k][2]=one;
        }
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
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveSampleUp(int which) {
  if (which<1 || which>=(int)song.sample.size()) return false;
  BUSY_BEGIN;
  DivSample* prev=song.sample[which];
  saveLock.lock();
  song.sample[which]=song.sample[which-1];
  song.sample[which-1]=prev;
  saveLock.unlock();
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
  saveLock.unlock();
  BUSY_END;
  return true;
}

bool DivEngine::moveSampleDown(int which) {
  if (which<0 || which>=((int)song.sample.size())-1) return false;
  BUSY_BEGIN;
  DivSample* prev=song.sample[which];
  saveLock.lock();
  song.sample[which]=song.sample[which+1];
  song.sample[which+1]=prev;
  saveLock.unlock();
  BUSY_END;
  return true;
}

void DivEngine::noteOn(int chan, int ins, int note, int vol) {
  if (chan<0 || chan>=chans) return;
  BUSY_BEGIN;
  pendingNotes.push(DivNoteEvent(chan,ins,note,vol,true));
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
  pendingNotes.push(DivNoteEvent(chan,-1,-1,-1,false));
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  BUSY_END;
}

void DivEngine::autoNoteOn(int ch, int ins, int note, int vol) {
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
  if (getPreferInsType(finalChan)!=insInst->type && getPreferInsSecondType(finalChan)!=insInst->type) notInViableChannel=true;
  for (int i=0; i<chans; i++) {
    if (ins==-1 || ins>=song.insLen || getPreferInsType(i)==insInst->type || getPreferInsSecondType(i)==insInst->type) {
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

  if (!canPlayAnyway) return;

  // 2. find a free channel
  do {
    if (isViable[finalChan] && chan[finalChan].midiNote==-1 && (insInst->type==DIV_INS_OPL || getChannelType(finalChan)==finalChanType || notInViableChannel)) {
      chan[finalChan].midiNote=note;
      chan[finalChan].midiAge=midiAgeCounter++;
      pendingNotes.push(DivNoteEvent(finalChan,ins,note,vol,true));
      return;
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
  pendingNotes.push(DivNoteEvent(candidate,ins,note,vol,true));
}

void DivEngine::autoNoteOff(int ch, int note, int vol) {
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  //if (ch<0 || ch>=chans) return;
  for (int i=0; i<chans; i++) {
    if (chan[i].midiNote==note) {
      pendingNotes.push(DivNoteEvent(i,-1,-1,-1,false));
      chan[i].midiNote=-1;
    }
  }
}

void DivEngine::autoNoteOffAll() {
  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }
  for (int i=0; i<chans; i++) {
    if (chan[i].midiNote!=-1) {
      pendingNotes.push(DivNoteEvent(i,-1,-1,-1,false));
      chan[i].midiNote=-1;
    }
  }
}

void DivEngine::setOrder(unsigned char order) {
  BUSY_BEGIN_SOFT;
  curOrder=order;
  if (order>=song.ordersLen) curOrder=0;
  if (playing && !freelance) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::setSysFlags(int system, unsigned int flags, bool restart) {
  BUSY_BEGIN_SOFT;
  saveLock.lock();
  song.systemFlags[system]=flags;
  saveLock.unlock();
  disCont[system].dispatch->setFlags(song.systemFlags[system]);
  disCont[system].setRates(got.rate);
  if (restart && isPlaying()) {
    playSub(false);
  }
  BUSY_END;
}

void DivEngine::setSongRate(float hz, bool pal) {
  BUSY_BEGIN;
  saveLock.lock();
  song.pal=!pal;
  song.hz=hz;
  // what?
  song.customTempo=true;
  divider=60;
  if (song.customTempo) {
    divider=song.hz;
  } else {
    if (song.pal) {
      divider=60;
    } else {
      divider=50;
    }
  }
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

void DivEngine::setConsoleMode(bool enable) {
  consoleMode=enable;
}

bool DivEngine::switchMaster() {
  deinitAudioBackend();
  quitDispatch();
  initDispatch();
  if (initAudioBackend()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].setRates(got.rate);
      disCont[i].setQuality(lowQuality);
    }
    if (!output->setRun(true)) {
      logE("error while activating audio!");
      return false;
    }
  } else {
    return false;
  }
  return true;
}

void DivEngine::setMidiBaseChan(int chan) {
  if (chan<0 || chan>=chans) chan=0;
  midiBaseChan=chan;
}

void DivEngine::setMidiDirect(bool value) {
  midiIsDirect=value;
}

void DivEngine::setMidiCallback(std::function<int(const TAMidiMessage&)> what) {
  midiCallback=what;
}

void DivEngine::synchronized(const std::function<void()>& what) {
  BUSY_BEGIN;
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
    if (output->midiIn!=NULL) {
      midiIns=output->midiIn->listDevices();
    }
    if (output->midiOut!=NULL) {
      midiOuts=output->midiOut->listDevices();
    }
  }
}

void DivEngine::initDispatch() {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].init(song.system[i],this,getChannelCount(song.system[i]),got.rate,song.systemFlags[i]);
    disCont[i].setRates(got.rate);
    disCont[i].setQuality(lowQuality);
  }
  recalcChans();
  BUSY_END;
}

void DivEngine::quitDispatch() {
  BUSY_BEGIN;
  for (int i=0; i<song.systemLen; i++) {
    disCont[i].quit();
  }
  cycles=0;
  clockDrift=0;
  chans=0;
  playing=false;
  speedAB=false;
  endOfSong=false;
  ticks=0;
  curRow=0;
  curOrder=0;
  nextSpeed=3;
  changeOrd=-1;
  changePos=0;
  totalTicks=0;
  totalSeconds=0;
  totalTicksR=0;
  totalCmds=0;
  lastCmds=0;
  cmdsPerSecond=0;
  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
  }
  BUSY_END;
}

#define CHECK_CONFIG_DIR_MAC() \
  configPath+="/Library/Application Support/Furnace"; \
  if (stat(configPath.c_str(),&st)<0) { \
    logI("creating config dir..."); \
    if (mkdir(configPath.c_str(),0755)<0) { \
      logW("could not make config dir! (%s)",strerror(errno)); \
      configPath="."; \
    } \
  }

#define CHECK_CONFIG_DIR() \
  configPath+="/.config"; \
  if (stat(configPath.c_str(),&st)<0) { \
    logI("creating user config dir..."); \
    if (mkdir(configPath.c_str(),0755)<0) { \
      logW("could not make user config dir! (%s)",strerror(errno)); \
      configPath="."; \
    } \
  } \
  if (configPath!=".") { \
    configPath+="/furnace"; \
    if (stat(configPath.c_str(),&st)<0) { \
      logI("creating config dir..."); \
      if (mkdir(configPath.c_str(),0755)<0) { \
        logW("could not make config dir! (%s)",strerror(errno)); \
        configPath="."; \
      } \
    } \
  }

bool DivEngine::initAudioBackend() {
  // load values
  if (audioEngine==DIV_AUDIO_NULL) {
    if (getConfString("audioEngine","SDL")=="JACK") {
      audioEngine=DIV_AUDIO_JACK;
    } else {
      audioEngine=DIV_AUDIO_SDL;
    }
  }

  lowQuality=getConfInt("audioQuality",0);
  forceMono=getConfInt("forceMono",0);
  lowLatency=getConfInt("lowLatency",0);
  metroVol=(float)(getConfInt("metroVol",100))/100.0f;
  if (metroVol<0.0f) metroVol=0.0f;
  if (metroVol>2.0f) metroVol=2.0f;

  if (lowLatency) logI("using low latency mode.");

  switch (audioEngine) {
    case DIV_AUDIO_JACK:
#ifndef HAVE_JACK
      logE("Furnace was not compiled with JACK support!");
      setConf("audioEngine","SDL");
      saveConf();
      output=new TAAudioSDL;
#else
      output=new TAAudioJACK;
#endif
      break;
    case DIV_AUDIO_SDL:
      output=new TAAudioSDL;
      break;
    case DIV_AUDIO_DUMMY:
      output=new TAAudio;
      break;
    default:
      logE("invalid audio engine!");
      return false;
  }

  audioDevs=output->listAudioDevices();

  want.deviceName=getConfString("audioDevice","");
  want.bufsize=getConfInt("audioBufSize",1024);
  want.rate=getConfInt("audioRate",44100);
  want.fragments=2;
  want.inChans=0;
  want.outChans=2;
  want.outFormat=TA_AUDIO_FORMAT_F32;
  want.name="Furnace";

  output->setCallback(process,this);

  if (!output->init(want,got)) {
    logE("error while initializing audio!");
    delete output;
    output=NULL;
    audioEngine=DIV_AUDIO_NULL;
    return false;
  }

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
    }
  }

  return true;
}

bool DivEngine::deinitAudioBackend() {
  if (output!=NULL) {
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
    output->quit();
    delete output;
    output=NULL;
    audioEngine=DIV_AUDIO_NULL;
  }
  return true;
}

#ifdef _WIN32
#include "winStuff.h"
#endif

bool DivEngine::init() {
  // register systems
  if (!systemsRegistered) registerSystems();
  
  // init config
#ifdef _WIN32
  configPath=getWinConfigPath();
#else
  struct stat st;
  char* home=getenv("HOME");
  if (home==NULL) {
    int uid=getuid();
    struct passwd* entry=getpwuid(uid);
    if (entry==NULL) {
      logW("unable to determine config directory! (%s)",strerror(errno));
      configPath=".";
    } else {
      configPath=entry->pw_dir;
#ifdef __APPLE__
      CHECK_CONFIG_DIR_MAC();
#else
      CHECK_CONFIG_DIR();
#endif
    }
  } else {
    configPath=home;
#ifdef __APPLE__
    CHECK_CONFIG_DIR_MAC();
#else
    CHECK_CONFIG_DIR();
#endif
  }
#endif
  logD("config path: %s",configPath.c_str());

  loadConf();

  // init the rest of engine
  bool haveAudio=false;
  if (!initAudioBackend()) {
    logE("no audio output available!");
  } else {
    haveAudio=true;
  }

  samp_bb=blip_new(32768);
  if (samp_bb==NULL) {
    logE("not enough memory!");
    return false;
  }

  samp_bbOut=new short[32768];

  samp_bbIn=new short[32768];
  samp_bbInLen=32768;
  
  blip_set_rates(samp_bb,44100,got.rate);

  for (int i=0; i<64; i++) {
    vibTable[i]=127*sin(((double)i/64.0)*(2*M_PI));
  }
  for (int i=0; i<4096; i++) {
    reversePitchTable[i]=round(1024.0*pow(2.0,(2048.0-(double)i)/(12.0*128.0)));
    pitchTable[i]=round(1024.0*pow(2.0,((double)i-2048.0)/(12.0*128.0)));
  }

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    isMuted[i]=0;
    keyHit[i]=false;
  }

  oscBuf[0]=new float[32768];
  oscBuf[1]=new float[32768];

  initDispatch();
  reset();
  active=true;

  if (!haveAudio) {
    return false;
  } else {
    if (!output->setRun(true)) {
      logE("error while activating!");
      return false;
    }
  }
  return true;
}

bool DivEngine::quit() {
  deinitAudioBackend();
  quitDispatch();
  logI("saving config.");
  saveConf();
  active=false;
  delete[] oscBuf[0];
  delete[] oscBuf[1];
  return true;
}
