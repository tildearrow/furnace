/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#include <string.h>
#include "../ta-log.h"
#include "pipe.h"

void taPipeThread(void* inst) {
  TAAudioPipe* in=(TAAudioPipe*)inst;
  in->runThread();
}

void TAAudioPipe::runThread() {
  while (running) {
    onProcess((unsigned char*)sbuf,desc.bufsize);
  }
}

void TAAudioPipe::onProcess(unsigned char* buf, int nframes) {
  if (audioProcCallback!=NULL) {
    if (midiIn!=NULL) midiIn->gather();
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,desc.bufsize);
  }
  short* sb=(short*)buf;

  if (sb==NULL) return;

  for (size_t j=0; j<desc.bufsize; j++) {
    for (size_t i=0; i<desc.outChans; i++) {
      if (outBufs[i][j]<-1.0) outBufs[i][j]=-1.0;
      if (outBufs[i][j]>1.0) outBufs[i][j]=1.0;
      sb[j*desc.outChans+i]=outBufs[i][j]*32767.0;
    }
  }

  fwrite(sb,2,desc.bufsize*desc.outChans,stdout);
  fflush(stdout);
}

void* TAAudioPipe::getContext() {
  return NULL;
}

bool TAAudioPipe::quit() {
  if (!initialized) return false;

  if (running) {
    running=false;
    if (outThread) {
      outThread->join();
      delete outThread;
      outThread=NULL;
    }
  }

  for (int i=0; i<desc.outChans; i++) {
    delete[] outBufs[i];
  }

  delete[] outBufs;

  if (sbuf) {
    delete[] sbuf;
    sbuf=NULL;
  }
  
  initialized=false;
  return true;
}

bool TAAudioPipe::setRun(bool run) {
  if (!initialized) return false;

  if (running!=run) {
    running=run;
    if (running) {
      outThread=new std::thread(taPipeThread,this);
    } else if (outThread) {
      outThread->join();
      delete outThread;
      outThread=NULL;
    }
  }

  return running;
}

std::vector<String> TAAudioPipe::listAudioDevices() {
  std::vector<String> ret;

  ret.push_back("stdout");

  return ret;
}

bool TAAudioPipe::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) {
    logE("audio already initialized");
    return false;
  }

  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;
  response=desc;

  logV("opening stdout for audio...");

  if (desc.outChans>0) {
    outBufs=new float*[desc.outChans];
    for (int i=0; i<desc.outChans; i++) {
      outBufs[i]=new float[desc.bufsize];
    }
      
    sbuf=new short[desc.bufsize*desc.outChans];
  } else {
    sbuf=NULL;
  }

  response=desc;
  initialized=true;
  return true;
}
