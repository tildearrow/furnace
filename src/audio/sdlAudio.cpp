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

#include <string.h>
#include "../ta-log.h"
#include "sdlAudio.h"

void taSDLProcess(void* inst, unsigned char* buf, int nframes) {
  TAAudioSDL* in=(TAAudioSDL*)inst;
  in->onProcess(buf,nframes);
}

void TAAudioSDL::onProcess(unsigned char* buf, int nframes) {
  unsigned int unframes=nframes/(sizeof(float)*MAX(1,desc.outChans));
  if (nframes<0) {
    logE("nframes is negative! (%d)",nframes);
    return;
  }
  if (desc.outChans<1) {
    logE("outChans is less than 1!");
    return;
  }
  for (int i=0; i<desc.inChans; i++) {
    if (unframes>desc.bufsize) {
      delete[] inBufs[i];
      inBufs[i]=new float[unframes];
    }
  }
  for (int i=0; i<desc.outChans; i++) {
    if (unframes>desc.bufsize) {
      delete[] outBufs[i];
      outBufs[i]=new float[unframes];
    }
  }
  if (audioProcCallback!=NULL) {
    if (midiIn!=NULL) midiIn->gather();
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,unframes);
  }
  float* fbuf=(float*)buf;
  for (size_t j=0; j<unframes; j++) {
    for (size_t i=0; i<desc.outChans; i++) {
      fbuf[j*desc.outChans+i]=outBufs[i][j];
    }
  }
  if (unframes!=desc.bufsize) {
    desc.bufsize=unframes;
  }
}

void* TAAudioSDL::getContext() {
  return (void*)&ac;
}

bool TAAudioSDL::quit() {
  if (!initialized) return false;

  if (ai!=0) {
    SDL_CloseAudioDevice(ai);
  }

  if (running) {
    running=false;
  }

  for (int i=0; i<desc.outChans; i++) {
    delete[] outBufs[i];
  }

  delete[] outBufs;
  
  initialized=false;
  return true;
}

bool TAAudioSDL::setRun(bool run) {
  if (!initialized) return false;
  SDL_PauseAudioDevice(ai,!run);
  running=run;
  return running;
}

std::vector<String> TAAudioSDL::listAudioDevices() {
  std::vector<String> ret;
  if (!audioSysStarted) {
    if (SDL_Init(SDL_INIT_AUDIO)<0) {
      logE("could not initialize SDL to list audio devices");
    } else {
      audioSysStarted=true;
    }
  }

  int count=SDL_GetNumAudioDevices(false);
  if (count<0) return ret;

  for (int i=0; i<count; i++) {
    const char* devName=SDL_GetAudioDeviceName(i,false);
    if (devName!=NULL) {
      ret.push_back(String(devName));
    }
  }

  return ret;
}

bool TAAudioSDL::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) {
    logE("audio already initialized");
    return false;
  }
  if (!audioSysStarted) {
    if (SDL_Init(SDL_INIT_AUDIO)<0) {
      logE("could not initialize SDL");
      return false;
    }
    audioSysStarted=true;
  }

  const char* audioDriver=SDL_GetCurrentAudioDriver();
  if (audioDriver==NULL) {
    logD("SDL audio driver: NULL!");
  } else {
    logD("SDL audio driver: %s",audioDriver);
  }

  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;

  ac.freq=desc.rate;
#ifdef TA_BIG_ENDIAN
  ac.format=AUDIO_F32MSB;
#else
  ac.format=AUDIO_F32;
#endif
  ac.channels=desc.outChans;
  ac.samples=desc.bufsize;
  ac.callback=taSDLProcess;
  ac.userdata=this;

  logV("opening audio device...");
  ai=SDL_OpenAudioDevice(request.deviceName.empty()?NULL:request.deviceName.c_str(),0,&ac,&ar,0);
  if (ai==0) {
    logE("could not open audio device: %s",SDL_GetError());
    return false;
  }

  const char* backendName=SDL_GetCurrentAudioDriver();

  desc.deviceName=request.deviceName;
  if (backendName==NULL) {
    desc.name="";
  } else {
    desc.name=backendName;
  }
  desc.rate=ar.freq;
  desc.inChans=0;
  desc.outChans=ar.channels;
  desc.bufsize=ar.samples;
  desc.fragments=1;

  logV("got info: %d channels, %d bufsize",desc.outChans,desc.bufsize);

  if (desc.outChans>0) {
    outBufs=new float*[desc.outChans];
    for (int i=0; i<desc.outChans; i++) {
      outBufs[i]=new float[desc.bufsize];
    }
  }

  response=desc;
  initialized=true;
  return true;
}
