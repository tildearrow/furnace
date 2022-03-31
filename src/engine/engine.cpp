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

#include "dataErrors.h"
#include "song.h"
#include <functional>
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

const char* DivEngine::getEffectDesc(unsigned char effect, int chan) {
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
      }
      else if (chan>=0 && chan<chans) {
        const char* ret=disCont[dispatchOfChan[chan]].dispatch->getEffectName(effect);
        if (ret!=NULL) return ret;
      }
      break;
  }
  return "Invalid effect";
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
        for (int l=0; l<song.pat[k].effectRows; l++) {
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
        logE("could not open file for writing! (%s)\n",sf_strerror(NULL));
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

      logI("rendering to file...\n");

      while (playing) {
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        for (int i=0; i<EXPORT_BUFSIZE; i++) {
          outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]));
          outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]));
        }
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d\n",totalProcessed,EXPORT_BUFSIZE);
        }
        if (sf_writef_float(sf,outBuf[2],totalProcessed)!=(int)totalProcessed) {
          logE("error: failed to write entire buffer!\n");
          break;
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      if (sf_close(sf)!=0) {
        logE("could not close audio file!\n");
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!\n");
        }
      }
      logI("done!\n");
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
        logI("- %s\n",fname[i].c_str());
        sf[i]=sf_open(fname[i].c_str(),SFM_WRITE,&si[i]);
        if (sf[i]==NULL) {
          logE("could not open file for writing! (%s)\n",sf_strerror(NULL));
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

      logI("rendering to files...\n");

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
            logE("error: total processed is bigger than export bufsize! (%d) %d>%d\n",i,totalProcessed,EXPORT_BUFSIZE);
          }
          if (sf_writef_short(sf[i],sysBuf,totalProcessed)!=(int)totalProcessed) {
            logE("error: failed to write entire buffer! (%d)\n",i);
            break;
          }
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] sysBuf;

      for (int i=0; i<song.systemLen; i++) {
        if (sf_close(sf[i])!=0) {
          logE("could not close audio file!\n");
        }
      }
      exporting=false;

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!\n");
        }
      }
      logI("done!\n");
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

      logI("rendering to files...\n");
      
      for (int i=0; i<chans; i++) {
        SNDFILE* sf;
        SF_INFO si;
        String fname=fmt::sprintf("%s_c%02d.wav",exportPath,i+1);
        logI("- %s\n",fname.c_str());
        si.samplerate=got.rate;
        si.channels=2;
        si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

        sf=sf_open(fname.c_str(),SFM_WRITE,&si);
        if (sf==NULL) {
          logE("could not open file for writing! (%s)\n",sf_strerror(NULL));
          break;
        }

        for (int j=0; j<chans; j++) {
          bool mute=(j!=i);
          isMuted[j]=mute;
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
            logE("error: total processed is bigger than export bufsize! %d>%d\n",totalProcessed,EXPORT_BUFSIZE);
          }
          if (sf_writef_float(sf,outBuf[2],totalProcessed)!=(int)totalProcessed) {
            logE("error: failed to write entire buffer!\n");
            break;
          }
        }

        if (sf_close(sf)!=0) {
          logE("could not close audio file!\n");
        }
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
          logE("error while activating audio!\n");
        }
      }
      logI("done!\n");
      break;
    }
  }
}

bool DivEngine::saveAudio(const char* path, int loops, DivAudioExportModes mode) {
  exportPath=path;
  exportMode=mode;
  exporting=true;
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
      logW("out of ADPCM-A memory for sample %d!\n",i);
      break;
    }
    if (memPos+paddedLen>=16777216) {
      memcpy(adpcmAMem+memPos,s->dataA,16777216-memPos);
      logW("out of ADPCM-A memory for sample %d!\n",i);
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
      logW("out of ADPCM-B memory for sample %d!\n",i);
      break;
    }
    if (memPos+paddedLen>=16777216) {
      memcpy(adpcmBMem+memPos,s->dataB,16777216-memPos);
      logW("out of ADPCM-B memory for sample %d!\n",i);
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
      logW("out of QSound PCM memory for sample %d!\n",i);
      break;
    }
    if (memPos+length>=16777216) {
      for (unsigned int i=0; i<16777216-(memPos+length); i++) {
        qsoundMem[(memPos+i)^0x8000]=s->data8[i];
      }
      logW("out of QSound PCM memory for sample %d!\n",i);
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
      logW("out of X1-010 memory for sample %d!\n",i);
      break;
    }
    if (memPos+paddedLen>=1048576) {
      memcpy(x1_010Mem+memPos,s->data8,1048576-memPos);
      logW("out of X1-010 memory for sample %d!\n",i);
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

void DivEngine::changeSystem(int index, DivSystem which) {
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
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

bool DivEngine::removeSystem(int index) {
  if (song.systemLen<=1) {
    lastError="cannot remove the last one";
    return false;
  }
  if (index<0 || index>=song.systemLen) {
    lastError="invalid index";
    return false;
  }
  quitDispatch();
  BUSY_BEGIN;
  saveLock.lock();
  song.system[index]=DIV_SYSTEM_NULL;
  song.systemLen--;
  for (int i=index; i<song.systemLen; i++) {
    song.system[i]=song.system[i+1];
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

DivInstrument* DivEngine::getIns(int index) {
  if (index<0 || index>=song.insLen) return &song.nullIns;
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
  }
  skipping=false;
  cmdStream.clear();
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

int DivEngine::calcFreq(int base, int pitch, bool period, int octave) {
  if (song.linearPitch) {
    // global pitch multiplier
    int whatTheFuck=(1024+(globalPitch<<6)-(globalPitch<0?globalPitch-6:0));
    if (whatTheFuck<1) whatTheFuck=1; // avoids division by zero but please kill me
    pitch+=2048;
    if (pitch<0) pitch=0;
    if (pitch>4095) pitch=4095;
    return period?
            ((base*(reversePitchTable[pitch]))/whatTheFuck):
            (((base*(pitchTable[pitch]))>>10)*whatTheFuck)/1024;
  }
  return period?
           base-pitch:
           base+((pitch*octave)>>1);
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
  chans=0;
  int chanIndex=0;
  for (int i=0; i<song.systemLen; i++) {
    int chanCount=getChannelCount(song.system[i]);
    chans+=chanCount;
    for (int j=0; j<chanCount; j++) {
      sysOfChan[chanIndex]=song.system[i];
      dispatchOfChan[chanIndex]=i;
      dispatchChanOfChan[chanIndex]=j;
      chanIndex++;
    }
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
  ins->name=fmt::sprintf("Instrument %d",insCount);
  ins->type=getPreferInsType(refChan);
  saveLock.lock();
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  saveLock.unlock();
  BUSY_END;
  return insCount;
}

enum DivInsFormats {
  DIV_INSFORMAT_DMP,
  DIV_INSFORMAT_TFI,
  DIV_INSFORMAT_VGI,
  DIV_INSFORMAT_FTI,
  DIV_INSFORMAT_BTI,
  DIV_INSFORMAT_S3I,
  DIV_INSFORMAT_SBI,
};

// TODO: re-organize this function to:
// - support replacing instruments
// - support instrument formats which contain multiple instruments
bool DivEngine::addInstrumentFromFile(const char* path) {
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

  FILE* f=ps_fopen(path,"rb");
  if (f==NULL) {
    lastError=strerror(errno);
    return false;
  }
  unsigned char* buf;
  ssize_t len;
  if (fseek(f,0,SEEK_END)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  len=ftell(f);
  if (len<0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  if (len==0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  if (fseek(f,0,SEEK_SET)!=0) {
    lastError=strerror(errno);
    fclose(f);
    return false;
  }
  buf=new unsigned char[len];
  if (fread(buf,1,len,f)!=(size_t)len) {
    logW("did not read entire instrument file buffer!\n");
    lastError="did not read entire instrument file!";
    delete[] buf;
    return false;
  }
  fclose(f);

  SafeReader reader=SafeReader(buf,len);

  unsigned char magic[16];
  bool isFurnaceInstr=false;
  try {
    reader.read(magic,16);
    if (memcmp("-Furnace instr.-",magic,16)==0) {
      isFurnaceInstr=true;
    }
  } catch (EndOfFileException& e) {
    reader.seek(0,SEEK_SET);
  }

  DivInstrument* ins=new DivInstrument;
  if (isFurnaceInstr) {
    try {
      short version=reader.readS();
      reader.readS(); // reserved

      if (version>DIV_ENGINE_VERSION) {
        warnings="this instrument is made with a more recent version of Furnace!";
      }

      unsigned int dataPtr=reader.readI();
      reader.seek(dataPtr,SEEK_SET);

      if (ins->readInsData(reader,version)!=DIV_DATA_SUCCESS) {
        lastError="invalid instrument header/data!";
        delete ins;
        delete[] buf;
        return false;
      }
    } catch (EndOfFileException& e) {
      lastError="premature end of file";
      logE("premature end of file!\n");
      delete ins;
      delete[] buf;
      return false;
    }
  } else { // read as a different format
    const char* ext=strrchr(path,'.');
    DivInsFormats format=DIV_INSFORMAT_DMP;
    if (ext!=NULL) {
      String extS;
      for (; *ext; ext++) {
        char i=*ext;
        if (i>='A' && i<='Z') {
          i+='a'-'A';
        }
        extS+=i;
      }
      if (extS==String(".dmp")) {
        format=DIV_INSFORMAT_DMP;
      } else if (extS==String(".tfi")) {
        format=DIV_INSFORMAT_TFI;
      } else if (extS==String(".vgi")) {
        format=DIV_INSFORMAT_VGI;
      } else if (extS==String(".fti")) {
        format=DIV_INSFORMAT_FTI;
      } else if (extS==String(".bti")) {
        format=DIV_INSFORMAT_BTI;
      } else if (extS==String(".s3i")) {
        format = DIV_INSFORMAT_S3I;
      } else if (extS==String(".sbi")) {
        format = DIV_INSFORMAT_SBI;
      }
    }

    // TDOO these really should be re-organized to separate functions per instrument file type.
    switch (format) {
      case DIV_INSFORMAT_DMP: {
        // this is a ridiculous mess
        unsigned char version=0;
        unsigned char sys=0;
        try {
          reader.seek(0,SEEK_SET);
          version=reader.readC();
        } catch (EndOfFileException& e) {
          lastError="premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return false;
        }

        if (version>11) {
          lastError="unknown instrument version!";
          delete ins;
          delete[] buf;
          return false;
        }

        ins->name=stripPath;

        if (version>=11) { // 1.0
          try {
            sys=reader.readC();
      
            switch (sys) {
              case 1: // YMU759
                ins->type=DIV_INS_FM;
                break;
              case 2: // Genesis
                ins->type=DIV_INS_FM;
                break;
              case 3: // SMS
                ins->type=DIV_INS_STD;
                break;
              case 4: // Game Boy
                ins->type=DIV_INS_GB;
                break;
              case 5: // PC Engine
                ins->type=DIV_INS_PCE;
                break;
              case 6: // NES
                ins->type=DIV_INS_STD;
                break;
              case 7: case 0x17: // C64
                ins->type=DIV_INS_C64;
                break;
              case 8: // Arcade
                ins->type=DIV_INS_FM;
                break;
              default:
                lastError="unknown instrument type!";
                delete ins;
                delete[] buf;
                return false;
                break;
            }
          } catch (EndOfFileException& e) {
            lastError="premature end of file";
            logE("premature end of file!\n");
            delete ins;
            delete[] buf;
            return false;
          }
        }

        try {
          bool mode=true;
          if (version>1) {
            mode=reader.readC();
            if (mode==0) {
              if (version<11) {
                ins->type=DIV_INS_STD;
              }
            } else {
              ins->type=DIV_INS_FM;
            }
          } else {
            ins->type=DIV_INS_FM;
          }

          if (mode) { // FM
            if (version<10) {
              if (version>1) {
                ins->fm.ops=reader.readC()?4:2;
              } else {
                ins->fm.ops=reader.readC()?2:4;
              }
            }
            if (version>1) { // HELP! in which version of the format did we start storing FMS!
              ins->fm.fms=reader.readC();
            }
            ins->fm.fb=reader.readC();
            ins->fm.alg=reader.readC();
            // DITTO
            if (sys!=1) ins->fm.ams=reader.readC();

            for (int j=0; j<ins->fm.ops; j++) {
              ins->fm.op[j].mult=reader.readC();
              ins->fm.op[j].tl=reader.readC();
              ins->fm.op[j].ar=reader.readC();
              ins->fm.op[j].dr=reader.readC();
              ins->fm.op[j].sl=reader.readC();
              ins->fm.op[j].rr=reader.readC();
              ins->fm.op[j].am=reader.readC();
              // what the hell how do I tell!
              if (sys==1) { // YMU759
                ins->fm.op[j].ws=reader.readC();
                ins->fm.op[j].ksl=reader.readC();
                ins->fm.op[j].vib=reader.readC();
                ins->fm.op[j].egt=reader.readC();
                ins->fm.op[j].sus=reader.readC();
                ins->fm.op[j].ksr=reader.readC();
                ins->fm.op[j].dvb=reader.readC();
                ins->fm.op[j].dam=reader.readC();
              } else {
                ins->fm.op[j].rs=reader.readC();
                ins->fm.op[j].dt=reader.readC();
                ins->fm.op[j].dt2=ins->fm.op[j].dt>>4;
                ins->fm.op[j].dt&=15;
                ins->fm.op[j].d2r=reader.readC();
                ins->fm.op[j].ssgEnv=reader.readC();
              }
            }
          } else { // STD
            if (ins->type!=DIV_INS_GB) {
              ins->std.volMacroLen=reader.readC();
              if (version>5) {
                for (int i=0; i<ins->std.volMacroLen; i++) {
                  ins->std.volMacro[i]=reader.readI();
                }
              } else {
                for (int i=0; i<ins->std.volMacroLen; i++) {
                  ins->std.volMacro[i]=reader.readC();
                }
              }
              if (version<11) for (int i=0; i<ins->std.volMacroLen; i++) {
                if (ins->std.volMacro[i]>15 && ins->type==DIV_INS_STD) ins->type=DIV_INS_PCE;
              }
              if (ins->std.volMacroLen>0) {
                ins->std.volMacroOpen=true;
                ins->std.volMacroLoop=reader.readC();
              } else {
                ins->std.volMacroOpen=false;
              }
            }

            ins->std.arpMacroLen=reader.readC();
            if (version>5) {
              for (int i=0; i<ins->std.arpMacroLen; i++) {
                ins->std.arpMacro[i]=reader.readI();
              }
            } else {
              for (int i=0; i<ins->std.arpMacroLen; i++) {
                ins->std.arpMacro[i]=reader.readC();
              }
            }
            if (ins->std.arpMacroLen>0) {
              ins->std.arpMacroOpen=true;
              ins->std.arpMacroLoop=reader.readC();
            } else {
              ins->std.arpMacroOpen=false;
            }
            if (version>8) { // TODO: when?
              ins->std.arpMacroMode=reader.readC();
            }

            ins->std.dutyMacroLen=reader.readC();
            if (version>5) {
              for (int i=0; i<ins->std.dutyMacroLen; i++) {
                ins->std.dutyMacro[i]=reader.readI();
              }
            } else {
              for (int i=0; i<ins->std.dutyMacroLen; i++) {
                ins->std.dutyMacro[i]=reader.readC();
              }
            }
            if (ins->std.dutyMacroLen>0) {
              ins->std.dutyMacroOpen=true;
              ins->std.dutyMacroLoop=reader.readC();
            } else {
              ins->std.dutyMacroOpen=false;
            }

            ins->std.waveMacroLen=reader.readC();
            if (version>5) {
              for (int i=0; i<ins->std.waveMacroLen; i++) {
                ins->std.waveMacro[i]=reader.readI();
              }
            } else {
              for (int i=0; i<ins->std.waveMacroLen; i++) {
                ins->std.waveMacro[i]=reader.readC();
              }
            }
            if (ins->std.waveMacroLen>0) {
              ins->std.waveMacroOpen=true;
              ins->std.waveMacroLoop=reader.readC();
            } else {
              ins->std.waveMacroOpen=false;
            }

            if (ins->type==DIV_INS_C64) {
              ins->c64.triOn=reader.readC();
              ins->c64.sawOn=reader.readC();
              ins->c64.pulseOn=reader.readC();
              ins->c64.noiseOn=reader.readC();

              ins->c64.a=reader.readC();
              ins->c64.d=reader.readC();
              ins->c64.s=reader.readC();
              ins->c64.r=reader.readC();

              ins->c64.duty=(reader.readC()*4095)/100;

              ins->c64.ringMod=reader.readC();
              ins->c64.oscSync=reader.readC();
              ins->c64.toFilter=reader.readC();
              if (version<0x07) { // TODO: UNSURE
                ins->c64.volIsCutoff=reader.readI();
              } else {
                ins->c64.volIsCutoff=reader.readC();
              }
              ins->c64.initFilter=reader.readC();

              ins->c64.res=reader.readC();
              ins->c64.cut=(reader.readC()*2047)/100;
              ins->c64.hp=reader.readC();
              ins->c64.bp=reader.readC();
              ins->c64.lp=reader.readC();
              ins->c64.ch3off=reader.readC();
            }
            if (ins->type==DIV_INS_GB) {
              ins->gb.envVol=reader.readC();
              ins->gb.envDir=reader.readC();
              ins->gb.envLen=reader.readC();
              ins->gb.soundLen=reader.readC();
            }
          }
        } catch (EndOfFileException& e) {
          lastError="premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return false;
        }
        break;
      }
      case DIV_INSFORMAT_TFI:
        try {
          reader.seek(0,SEEK_SET);

          ins->type=DIV_INS_FM;
          ins->name=stripPath;
          
          ins->fm.alg=reader.readC();
          ins->fm.fb=reader.readC();

          for (int i=0; i<4; i++) {
            DivInstrumentFM::Operator& op=ins->fm.op[i];

            op.mult=reader.readC();
            op.dt=reader.readC();
            op.tl=reader.readC();
            op.rs=reader.readC();
            op.ar=reader.readC();
            op.dr=reader.readC();
            op.d2r=reader.readC();
            op.rr=reader.readC();
            op.sl=reader.readC();
            op.ssgEnv=reader.readC();
          }
        } catch (EndOfFileException& e) {
          lastError="premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return false;
        }
        break;
      case DIV_INSFORMAT_VGI:
        try {
          reader.seek(0,SEEK_SET);

          ins->type=DIV_INS_FM;
          ins->name=stripPath;
          
          ins->fm.alg=reader.readC();
          ins->fm.fb=reader.readC();
          unsigned char fmsams=reader.readC();
          ins->fm.fms=fmsams&7;
          ins->fm.ams=fmsams>>4;

          for (int i=0; i<4; i++) {
            DivInstrumentFM::Operator& op=ins->fm.op[i];

            op.mult=reader.readC();
            op.dt=reader.readC();
            op.tl=reader.readC();
            op.rs=reader.readC();
            op.ar=reader.readC();
            op.dr=reader.readC();
            if (op.dr&0x80) {
              op.am=1;
              op.dr&=0x7f;
            }
            op.d2r=reader.readC();
            op.rr=reader.readC();
            op.sl=reader.readC();
            op.ssgEnv=reader.readC();
          }
        } catch (EndOfFileException& e) {
          lastError="premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return false;
        }
        break;
      case DIV_INSFORMAT_FTI:
        break;
      case DIV_INSFORMAT_BTI:
        break;
      case DIV_INSFORMAT_S3I:
        try {
          reader.seek(0, SEEK_SET);

          uint8_t s3i_type = reader.readC();

          if (s3i_type >= 2) {
            ins->type = DIV_INS_OPL;
            // skip internal filename - we'll use the long name description
            reader.seek(12, SEEK_CUR);

            // skip reserved bytes
            reader.seek(3, SEEK_CUR);

            // 12-byte opl value
            uint8_t s3i_Mcharacteristics = reader.readC();
            uint8_t s3i_Ccharacteristics = reader.readC();
            uint8_t s3i_Mscaling_output = reader.readC();
            uint8_t s3i_Cscaling_output = reader.readC();
            uint8_t s3i_Meg_AD = reader.readC();
            uint8_t s3i_Ceg_AD = reader.readC();
            uint8_t s3i_Meg_SR = reader.readC();
            uint8_t s3i_Ceg_SR = reader.readC();
            uint8_t s3i_Mwave = reader.readC();
            uint8_t s3i_Cwave = reader.readC();
            uint8_t s3i_FeedConnect = reader.readC();
            
            DivInstrumentFM::Operator& opM = ins->fm.op[0];
            DivInstrumentFM::Operator& opC = ins->fm.op[1];
            ins->fm.ops = 2;
            opM.mult = s3i_Mcharacteristics & 0xF;
            opM.ksr = ((s3i_Mcharacteristics >> 4) & 0x1);
            opM.sus = ((s3i_Mcharacteristics >> 5) & 0x1);
            opM.vib = ((s3i_Mcharacteristics >> 6) & 0x1);
            opM.am = ((s3i_Mcharacteristics >> 7) & 0x1);
            opM.tl = s3i_Mscaling_output & 0x3F;
            opM.ksl = ((s3i_Mscaling_output >> 6) & 0x3);
            opM.ar = ((s3i_Meg_AD >> 4) & 0xF);
            opM.dr = (s3i_Meg_AD & 0xF);
            opM.rr = (s3i_Meg_SR & 0xF);
            opM.sl = ((s3i_Meg_SR >> 4) & 0xF);
            opM.ws = s3i_Mwave;

            ins->fm.alg = (s3i_FeedConnect & 0x1);
            ins->fm.fb = ((s3i_FeedConnect >> 1) & 0x7);

            opC.mult = s3i_Ccharacteristics & 0xF;
            opC.ksr = ((s3i_Ccharacteristics >> 4) & 0x1);
            opC.sus = ((s3i_Ccharacteristics >> 5) & 0x1);
            opC.vib = ((s3i_Ccharacteristics >> 6) & 0x1);
            opC.am = ((s3i_Ccharacteristics >> 7) & 0x1);
            opC.tl = s3i_Cscaling_output & 0x3F;
            opC.ksl = ((s3i_Cscaling_output >> 6) & 0x3);
            opC.ar = ((s3i_Ceg_AD >> 4) & 0xF);
            opC.dr = (s3i_Ceg_AD & 0xF);
            opC.rr = (s3i_Ceg_SR & 0xF);
            opC.sl = ((s3i_Ceg_SR >> 4) & 0xF);
            opC.ws = s3i_Cwave;

            // Skip more stuff we don't need
            reader.seek(21, SEEK_CUR);
          } else {
            logE("S3I PCM samples currently not supported.");
          }
          ins->name = reader.readString(28);
          int s3i_signature = reader.readI();

          if (s3i_signature != 0x49524353) {
            logW("S3I signature invalid.");
          };
        }
        catch (EndOfFileException& e) {
          lastError = "premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return false;
        }
        break;
      case DIV_INSFORMAT_SBI:
        try {
          reader.seek(0, SEEK_SET);
          ins->type = DIV_INS_OPL;

          int sbi_header = reader.readI();
          // SBI header determines format
          bool is_2op = (sbi_header == 0x1A494253); // SBI\x1A
          bool is_4op = (sbi_header == 0x1A504F34); // 4OP\x1A
          bool is_6op = (sbi_header == 0x1A504F36); // 6OP\x1A - Freq Monster 801-specific

          // 32-byte null terminated instrument name
          ins->name = reader.readString(32);

          // 2op SBI
          uint8_t sbi_Mcharacteristics = reader.readC();
          uint8_t sbi_Ccharacteristics = reader.readC();
          uint8_t sbi_Mscaling_output = reader.readC();
          uint8_t sbi_Cscaling_output = reader.readC();
          uint8_t sbi_Meg_AD = reader.readC();
          uint8_t sbi_Ceg_AD = reader.readC();
          uint8_t sbi_Meg_SR = reader.readC();
          uint8_t sbi_Ceg_SR = reader.readC();
          uint8_t sbi_Mwave = reader.readC();
          uint8_t sbi_Cwave = reader.readC();
          uint8_t sbi_FeedConnect = reader.readC();

          // 4op SBI
          uint8_t sbi_M4characteristics;
          uint8_t sbi_C4characteristics;
          uint8_t sbi_M4scaling_output;
          uint8_t sbi_C4scaling_output;
          uint8_t sbi_M4eg_AD;
          uint8_t sbi_C4eg_AD;
          uint8_t sbi_M4eg_SR;
          uint8_t sbi_C4eg_SR;
          uint8_t sbi_M4wave;
          uint8_t sbi_C4wave;
          uint8_t sbi_4opConnect;
          
          if (is_2op) {
            DivInstrumentFM::Operator& opM = ins->fm.op[0];
            DivInstrumentFM::Operator& opC = ins->fm.op[1];
            ins->fm.ops = 2;
            opM.mult = sbi_Mcharacteristics & 0xF;
            opM.ksr = ((sbi_Mcharacteristics >> 4) & 0x1);
            opM.sus = ((sbi_Mcharacteristics >> 5) & 0x1);
            opM.vib = ((sbi_Mcharacteristics >> 6) & 0x1);
            opM.am = ((sbi_Mcharacteristics >> 7) & 0x1);
            opM.tl = sbi_Mscaling_output & 0x3F;
            opM.ksl = ((sbi_Mscaling_output >> 6) & 0x3);
            opM.ar = ((sbi_Meg_AD >> 4) & 0xF);
            opM.dr = (sbi_Meg_AD & 0xF);
            opM.rr = (sbi_Meg_SR & 0xF);
            opM.sl = ((sbi_Meg_SR >> 4) & 0xF);
            opM.ws = sbi_Mwave;

            ins->fm.alg = (sbi_FeedConnect & 0x1);
            ins->fm.fb = ((sbi_FeedConnect >> 1) & 0x7);

            opC.mult = sbi_Ccharacteristics & 0xF;
            opC.ksr = ((sbi_Ccharacteristics >> 4) & 0x1);
            opC.sus = ((sbi_Ccharacteristics >> 5) & 0x1);
            opC.vib = ((sbi_Ccharacteristics >> 6) & 0x1);
            opC.am = ((sbi_Ccharacteristics >> 7) & 0x1);
            opC.tl = sbi_Cscaling_output & 0x3F;
            opC.ksl = ((sbi_Cscaling_output >> 6) & 0x3);
            opC.ar = ((sbi_Ceg_AD >> 4) & 0xF);
            opC.dr = (sbi_Ceg_AD & 0xF);
            opC.rr = (sbi_Ceg_SR & 0xF);
            opC.sl = ((sbi_Ceg_SR >> 4) & 0xF);
            opC.ws = sbi_Cwave;

            // Ignore rest of file - rest is 'reserved padding'.
            reader.seek(0, SEEK_END);
          }

          if (is_4op || is_6op) {
            // Operator placement is different so need to place in correct registers.
            // Note: 6op is an unofficial extension of 4op SBIs by Darron Broad (Freq Monster 801).
            // We'll only use the 4op portion here for pure OPL3.
            DivInstrumentFM::Operator& opM = ins->fm.op[0];
            DivInstrumentFM::Operator& opC = ins->fm.op[2];
            DivInstrumentFM::Operator& opM4 = ins->fm.op[1];
            DivInstrumentFM::Operator& opC4 = ins->fm.op[3];
            ins->fm.ops = 4;

            sbi_M4characteristics = reader.readC();
            sbi_C4characteristics = reader.readC();
            sbi_M4scaling_output = reader.readC();
            sbi_C4scaling_output = reader.readC();
            sbi_M4eg_AD = reader.readC();
            sbi_C4eg_AD = reader.readC();
            sbi_M4eg_SR = reader.readC();
            sbi_C4eg_SR = reader.readC();
            sbi_M4wave = reader.readC();
            sbi_C4wave = reader.readC();
            sbi_4opConnect = reader.readC();
            
            ins->fm.alg = (sbi_FeedConnect & 0x1) | ((sbi_4opConnect & 0x1) << 1);
            ins->fm.fb = ((sbi_FeedConnect >> 1) & 0x7);

            opM.mult = sbi_Mcharacteristics & 0xF;
            opM.ksr = ((sbi_Mcharacteristics >> 4) & 0x1);
            opM.sus = ((sbi_Mcharacteristics >> 5) & 0x1);
            opM.vib = ((sbi_Mcharacteristics >> 6) & 0x1);
            opM.am = ((sbi_Mcharacteristics >> 7) & 0x1);
            opM.tl = sbi_Mscaling_output & 0x3F;
            opM.ksl = ((sbi_Mscaling_output >> 6) & 0x3);
            opM.ar = ((sbi_Meg_AD >> 4) & 0xF);
            opM.dr = (sbi_Meg_AD & 0xF);
            opM.rr = (sbi_Meg_SR & 0xF);
            opM.sl = ((sbi_Meg_SR >> 4) & 0xF);
            opM.ws = sbi_Mwave;

            opC.mult = sbi_Ccharacteristics & 0xF;
            opC.ksr = ((sbi_Ccharacteristics >> 4) & 0x1);
            opC.sus = ((sbi_Ccharacteristics >> 5) & 0x1);
            opC.vib = ((sbi_Ccharacteristics >> 6) & 0x1);
            opC.am = ((sbi_Ccharacteristics >> 7) & 0x1);
            opC.tl = sbi_Cscaling_output & 0x3F;
            opC.ksl = ((sbi_Cscaling_output >> 6) & 0x3);
            opC.ar = ((sbi_Ceg_AD >> 4) & 0xF);
            opC.dr = (sbi_Ceg_AD & 0xF);
            opC.rr = (sbi_Ceg_SR & 0xF);
            opC.sl = ((sbi_Ceg_SR >> 4) & 0xF);
            opC.ws = sbi_Cwave;
            
            opM4.mult = sbi_M4characteristics & 0xF;
            opM4.ksr = ((sbi_M4characteristics >> 4) & 0x1);
            opM4.sus = ((sbi_M4characteristics >> 5) & 0x1);
            opM4.vib = ((sbi_M4characteristics >> 6) & 0x1);
            opM4.am = ((sbi_M4characteristics >> 7) & 0x1);
            opM4.tl = sbi_M4scaling_output & 0x3F;
            opM4.ksl = ((sbi_M4scaling_output >> 6) & 0x3);
            opM4.ar = ((sbi_M4eg_AD >> 4) & 0xF);
            opM4.dr = (sbi_M4eg_AD & 0xF);
            opM4.rr = (sbi_M4eg_SR & 0xF);
            opM4.sl = ((sbi_M4eg_SR >> 4) & 0xF);
            opM4.ws = sbi_M4wave;

            opC4.mult = sbi_C4characteristics & 0xF;
            opC4.ksr = ((sbi_C4characteristics >> 4) & 0x1);
            opC4.sus = ((sbi_C4characteristics >> 5) & 0x1);
            opC4.vib = ((sbi_C4characteristics >> 6) & 0x1);
            opC4.am = ((sbi_C4characteristics >> 7) & 0x1);
            opC4.tl = sbi_C4scaling_output & 0x3F;
            opC4.ksl = ((sbi_C4scaling_output >> 6) & 0x3);
            opC4.ar = ((sbi_C4eg_AD >> 4) & 0xF);
            opC4.dr = (sbi_C4eg_AD & 0xF);
            opC4.rr = (sbi_C4eg_SR & 0xF);
            opC4.sl = ((sbi_C4eg_SR >> 4) & 0xF);
            opC4.ws = sbi_C4wave;

            // Ignore rest of file once we've read in all we need.
            // Note: Freq Monster 801 adds a ton of other additional fields irrelevant to chip registers.
            reader.seek(0, SEEK_END);
          }

        } catch (EndOfFileException& e) {
          lastError = "premature end of file";
          logE("premature end of file!\n");
          delete ins;
          delete[] buf;
          return false;
        }
        break;
    }

    if (reader.tell()<reader.size()) {
      addWarning("https://github.com/tildearrow/furnace/issues/84");
      addWarning("there is more data at the end of the file! what happened here!");
      addWarning(fmt::sprintf("exactly %d bytes, if you are curious",reader.size()-reader.tell()));
    }
  }

  BUSY_BEGIN;
  saveLock.lock();
  int insCount=(int)song.ins.size();
  song.ins.push_back(ins);
  song.insLen=insCount+1;
  saveLock.unlock();
  BUSY_END;
  return true;
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
      for (int j=0; j<128; j++) {
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
    logW("did not read entire wavetable file buffer!\n");
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
          logI("reading modern .dmw...\n");
          logD("wave version %d\n",waveVersion);
          wave->max=reader.readC();
          for (int i=0; i<len; i++) {
            wave->data[i]=reader.readI();
          }
        } else if (reader.size()==(size_t)(len+5)) {
          // read as .dmw
          logI("reading .dmw...\n");
          if (len>256) len=256;
          for (int i=0; i<len; i++) {
            wave->data[i]=(unsigned char)reader.readC();
          }
        } else {
          // read as binary
          logI("reading binary...\n");
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
        logI("reading binary for being too small...\n");
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
    logW("sample read size mismatch!\n");
  }
  DivSample* sample=new DivSample;
  int sampleCount=(int)song.sample.size();
  const char* sName=strrchr(path,DIR_SEPARATOR);
  if (sName==NULL) {
    sName=path;
  } else {
    sName++;
  }
  sample->name=sName;

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
  if (song.ordersLen>=0x7e) return;
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
      order[i]=0x7e;
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
  if (song.ordersLen>=0x7e) return;
  warnings="";
  BUSY_BEGIN_SOFT;
  for (int i=0; i<chans; i++) {
    bool didNotFind=true;
    logD("channel %d\n",i);
    order[i]=song.orders.ord[i][curOrder];
    // find free slot
    for (int j=0; j<128; j++) {
      logD("finding free slot in %d...\n",j);
      if (song.pat[i].data[j]==NULL) {
        int origOrd=order[i];
        order[i]=j;
        DivPattern* oldPat=song.pat[i].getPattern(origOrd,false);
        DivPattern* pat=song.pat[i].getPattern(j,true);
        memcpy(pat->data,oldPat->data,256*32*sizeof(short));
        logD("found at %d\n",j);
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
    for (int j=0; j<128; j++) {
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
  //if (ch<0 || ch>=chans) return;
  if (midiBaseChan<0) midiBaseChan=0;
  if (midiBaseChan>=chans) midiBaseChan=chans-1;
  int finalChan=midiBaseChan;

  if (!playing) {
    reset();
    freelance=true;
    playing=true;
  }

  do {
    if ((ins==-1 || getPreferInsType(finalChan)==getIns(ins)->type) && chan[finalChan].midiNote==-1) {
      chan[finalChan].midiNote=note;
      pendingNotes.push(DivNoteEvent(finalChan,ins,note,vol,true));
      break;
    }
    if (++finalChan>=chans) {
      finalChan=0;
    }
  } while (finalChan!=midiBaseChan);
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
      logE("error while activating audio!\n");
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
    logI("creating config dir...\n"); \
    if (mkdir(configPath.c_str(),0755)<0) { \
      logW("could not make config dir! (%s)\n",strerror(errno)); \
      configPath="."; \
    } \
  }

#define CHECK_CONFIG_DIR() \
  configPath+="/.config"; \
  if (stat(configPath.c_str(),&st)<0) { \
    logI("creating user config dir...\n"); \
    if (mkdir(configPath.c_str(),0755)<0) { \
      logW("could not make user config dir! (%s)\n",strerror(errno)); \
      configPath="."; \
    } \
  } \
  if (configPath!=".") { \
    configPath+="/furnace"; \
    if (stat(configPath.c_str(),&st)<0) { \
      logI("creating config dir...\n"); \
      if (mkdir(configPath.c_str(),0755)<0) { \
        logW("could not make config dir! (%s)\n",strerror(errno)); \
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

  switch (audioEngine) {
    case DIV_AUDIO_JACK:
#ifndef HAVE_JACK
      logE("Furnace was not compiled with JACK support!\n");
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
      logE("invalid audio engine!\n");
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
    logE("error while initializing audio!\n");
    delete output;
    output=NULL;
    audioEngine=DIV_AUDIO_NULL;
    return false;
  }

  if (output->initMidi(false)) {
    midiIns=output->midiIn->listDevices();
    midiOuts=output->midiOut->listDevices();
  } else {
    logW("error while initializing MIDI!\n");
  }
  if (output->midiIn) {
    String inName=getConfString("midiInDevice","");
    if (!inName.empty()) {
      // try opening device
      logI("opening MIDI input.\n");
      if (!output->midiIn->openDevice(inName)) {
        logW("could not open MIDI input device!\n");
      }
    }
  }
  if (output->midiOut) {
    String outName=getConfString("midiOutDevice","");
    if (!outName.empty()) {
      // try opening device
      logI("opening MIDI output.\n");
      if (!output->midiOut->openDevice(outName)) {
        logW("could not open MIDI output device!\n");
      }
    }
  }

  return true;
}

bool DivEngine::deinitAudioBackend() {
  if (output!=NULL) {
    if (output->midiIn) {
      if (output->midiIn->isDeviceOpen()) {
        logI("closing MIDI input.\n");
        output->midiIn->closeDevice();
      }
    }
    if (output->midiOut) {
      if (output->midiOut->isDeviceOpen()) {
        logI("closing MIDI output.\n");
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
      logW("unable to determine config directory! (%s)\n",strerror(errno));
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
  logD("config path: %s\n",configPath.c_str());

  loadConf();

  // init the rest of engine
  bool haveAudio=false;
  if (!initAudioBackend()) {
    logE("no audio output available!\n");
  } else {
    haveAudio=true;
  }

  samp_bb=blip_new(32768);
  if (samp_bb==NULL) {
    logE("not enough memory!\n");
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
      logE("error while activating!\n");
      return false;
    }
  }
  return true;
}

bool DivEngine::quit() {
  deinitAudioBackend();
  quitDispatch();
  logI("saving config.\n");
  saveConf();
  active=false;
  delete[] oscBuf[0];
  delete[] oscBuf[1];
  return true;
}
