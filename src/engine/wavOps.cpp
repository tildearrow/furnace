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

#include "engine.h"
#include "../ta-log.h"
#ifdef HAVE_SNDFILE
#include "sfWrapper.h"
#endif

#define EXPORT_BUFSIZE 2048

void _runExportThread(DivEngine* caller) {
  caller->runExportThread();
}

bool DivEngine::isExporting() {
  return exporting;
}

#ifdef HAVE_SNDFILE
void DivEngine::runExportThread() {
  size_t fadeOutSamples=got.rate*exportFadeOut;
  size_t curFadeOutSample=0;
  bool isFadingOut=false;

  switch (exportMode) {
    case DIV_EXPORT_MODE_ONE: {
      SNDFILE* sf;
      SF_INFO si;
      SFWrapper sfWrap;
      si.samplerate=got.rate;
      si.channels=2;
      si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

      sf=sfWrap.doOpen(exportPath.c_str(),SFM_WRITE,&si);
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
        size_t total=0;
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
          totalProcessed=EXPORT_BUFSIZE;
        }
        for (int i=0; i<(int)totalProcessed; i++) {
          total++;
          if (isFadingOut) {
            double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
            outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]))*mul;
            outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]))*mul;
            if (++curFadeOutSample>=fadeOutSamples) {
              playing=false;
              break;
            }
          } else {
            outBuf[2][i<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][i]));
            outBuf[2][1+(i<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][i]));
            if (lastLoopPos>-1 && i>=lastLoopPos && totalLoops>=exportLoopCount) {
              logD("start fading out...");
              isFadingOut=true;
              if (fadeOutSamples==0) break;
            }
          }
        }
        
        if (sf_writef_float(sf,outBuf[2],total)!=(int)total) {
          logE("error: failed to write entire buffer!");
          break;
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];
      delete[] outBuf[2];

      if (sfWrap.doClose()!=0) {
        logE("could not close audio file!");
      }

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality,dcHiPass);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      exporting=false;
      break;
    }
    case DIV_EXPORT_MODE_MANY_SYS: {
      SNDFILE* sf[DIV_MAX_CHIPS];
      SF_INFO si[DIV_MAX_CHIPS];
      String fname[DIV_MAX_CHIPS];
      SFWrapper sfWrap[DIV_MAX_CHIPS];
      for (int i=0; i<song.systemLen; i++) {
        sf[i]=NULL;
        si[i].samplerate=got.rate;
        si[i].channels=disCont[i].dispatch->getOutputCount();
        si[i].format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;
      }

      for (int i=0; i<song.systemLen; i++) {
        fname[i]=fmt::sprintf("%s_s%02d.wav",exportPath,i+1);
        logI("- %s",fname[i].c_str());
        sf[i]=sfWrap[i].doOpen(fname[i].c_str(),SFM_WRITE,&si[i]);
        if (sf[i]==NULL) {
          logE("could not open file for writing! (%s)",sf_strerror(NULL));
          for (int j=0; j<i; j++) {
            sfWrap[i].doClose();
          }
          return;
        }
      }

      float* outBuf[DIV_MAX_OUTPUTS];
      memset(outBuf,0,sizeof(void*)*DIV_MAX_OUTPUTS);
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      short* sysBuf[DIV_MAX_CHIPS];
      for (int i=0; i<song.systemLen; i++) {
        sysBuf[i]=new short[EXPORT_BUFSIZE*disCont[i].dispatch->getOutputCount()];
      }

      // take control of audio output
      deinitAudioBackend();
      playSub(false);

      logI("rendering to files...");

      while (playing) {
        size_t total=0;
        nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
        if (totalProcessed>EXPORT_BUFSIZE) {
          logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
          totalProcessed=EXPORT_BUFSIZE;
        }
        for (int j=0; j<(int)totalProcessed; j++) {
          total++;
          if (isFadingOut) {
            double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
            for (int i=0; i<song.systemLen; i++) {
              for (int k=0; k<si[i].channels; k++) {
                if (disCont[i].bbOut[k]==NULL) {
                  sysBuf[i][k+(j*si[i].channels)]=0;
                } else {
                  sysBuf[i][k+(j*si[i].channels)]=(double)disCont[i].bbOut[k][j]*mul;
                }
              }
            }
            if (++curFadeOutSample>=fadeOutSamples) {
              playing=false;
              break;
            }
          } else {
            for (int i=0; i<song.systemLen; i++) {
              for (int k=0; k<si[i].channels; k++) {
                if (disCont[i].bbOut[k]==NULL) {
                  sysBuf[i][k+(j*si[i].channels)]=0;
                } else {
                  sysBuf[i][k+(j*si[i].channels)]=disCont[i].bbOut[k][j];
                }
              }
            }
            if (lastLoopPos>-1 && j>=lastLoopPos && totalLoops>=exportLoopCount) {
              logD("start fading out...");
              isFadingOut=true;
              if (fadeOutSamples==0) break;
            }
          }
        }
        for (int i=0; i<song.systemLen; i++) {
          if (sf_writef_short(sf[i],sysBuf[i],total)!=(int)total) {
            logE("error: failed to write entire buffer! (%d)",i);
            break;
          }
        }
      }

      delete[] outBuf[0];
      delete[] outBuf[1];

      for (int i=0; i<song.systemLen; i++) {
        delete[] sysBuf[i];
        if (sfWrap[i].doClose()!=0) {
          logE("could not close audio file!");
        }
      }

      if (initAudioBackend()) {
        for (int i=0; i<song.systemLen; i++) {
          disCont[i].setRates(got.rate);
          disCont[i].setQuality(lowQuality,dcHiPass);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      exporting=false;
      break;
    }
    case DIV_EXPORT_MODE_MANY_CHAN: {
      // take control of audio output
      deinitAudioBackend();

      float* outBuf[3];
      outBuf[0]=new float[EXPORT_BUFSIZE];
      outBuf[1]=new float[EXPORT_BUFSIZE];
      outBuf[2]=new float[EXPORT_BUFSIZE*2];

      logI("rendering to files...");
      
      for (int i=0; i<chans; i++) {
        SNDFILE* sf;
        SF_INFO si;
        SFWrapper sfWrap;
        String fname=fmt::sprintf("%s_c%02d.wav",exportPath,i+1);
        logI("- %s",fname.c_str());
        si.samplerate=got.rate;
        si.channels=2;
        si.format=SF_FORMAT_WAV|SF_FORMAT_PCM_16;

        sf=sfWrap.doOpen(fname.c_str(),SFM_WRITE,&si);
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
        prevOrder=0;
        curFadeOutSample=0;
        lastLoopPos=-1;
        totalLoops=0;
        isFadingOut=false;
        remainingLoops=-1;
        playSub(false);

        while (playing) {
          size_t total=0;
          nextBuf(NULL,outBuf,0,2,EXPORT_BUFSIZE);
          if (totalProcessed>EXPORT_BUFSIZE) {
            logE("error: total processed is bigger than export bufsize! %d>%d",totalProcessed,EXPORT_BUFSIZE);
            totalProcessed=EXPORT_BUFSIZE;
          }
          for (int j=0; j<(int)totalProcessed; j++) {
            total++;
            if (isFadingOut) {
              double mul=(1.0-((double)curFadeOutSample/(double)fadeOutSamples));
              outBuf[2][j<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][j]))*mul;
              outBuf[2][1+(j<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][j]))*mul;
              if (++curFadeOutSample>=fadeOutSamples) {
                playing=false;
                break;
              }
            } else {
              outBuf[2][j<<1]=MAX(-1.0f,MIN(1.0f,outBuf[0][j]));
              outBuf[2][1+(j<<1)]=MAX(-1.0f,MIN(1.0f,outBuf[1][j]));
              if (lastLoopPos>-1 && j>=lastLoopPos && totalLoops>=exportLoopCount) {
                logD("start fading out...");
                isFadingOut=true;
                if (fadeOutSamples==0) break;
              }
            }
          }
          if (sf_writef_float(sf,outBuf[2],total)!=(int)total) {
            logE("error: failed to write entire buffer!");
            break;
          }
        }

        if (sfWrap.doClose()!=0) {
          logE("could not close audio file!");
        }

        if (getChannelType(i)==5) {
          i++;
          while (true) {
            if (i>=chans) break;
            if (getChannelType(i)!=5) break;
            i++;
          }
          i--;
        }

        if (stopExport) break;
      }

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
          disCont[i].setQuality(lowQuality,dcHiPass);
        }
        if (!output->setRun(true)) {
          logE("error while activating audio!");
        }
      }
      logI("done!");
      exporting=false;
      break;
    }
  }

  stopExport=false;
}
#else
void DivEngine::runExportThread() {
}
#endif

bool DivEngine::shallSwitchCores() {
  // TODO: detect whether we should
  return true;
}

bool DivEngine::saveAudio(const char* path, int loops, DivAudioExportModes mode, double fadeOutTime) {
#ifndef HAVE_SNDFILE
  logE("Furnace was not compiled with libsndfile. cannot export!");
  return false;
#else
  exportPath=path;
  exportMode=mode;
  exportFadeOut=fadeOutTime;
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
  remainingLoops=-1;

  if (shallSwitchCores()) {
    bool isMutedBefore[DIV_MAX_CHANS];
    memcpy(isMutedBefore,isMuted,DIV_MAX_CHANS*sizeof(bool));
    quitDispatch();
    initDispatch(true);
    renderSamplesP();
    for (int i=0; i<chans; i++) {
      if (isMutedBefore[i]) {
        muteChannel(i,true);
      }
    }
  }

  exportLoopCount=loops;
  exportThread=new std::thread(_runExportThread,this);
  return true;
#endif
}

void DivEngine::waitAudioFile() {
  if (exportThread!=NULL) {
    exportThread->join();
  }
}

bool DivEngine::haltAudioFile() {
  stopExport=true;
  stop();
  waitAudioFile();
  finishAudioFile();
  return true;
}

void DivEngine::finishAudioFile() {
  if (shallSwitchCores()) {
    bool isMutedBefore[DIV_MAX_CHANS];
    memcpy(isMutedBefore,isMuted,DIV_MAX_CHANS*sizeof(bool));
    quitDispatch();
    initDispatch(false);
    renderSamplesP();
    for (int i=0; i<chans; i++) {
      if (isMutedBefore[i]) {
        muteChannel(i,true);
      }
    }
  }
}

