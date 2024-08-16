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

#include <string.h>
#include "../ta-log.h"
#include "pa.h"
#ifdef _WIN32
#include <pa_win_wasapi.h>
#endif
#include <fmt/printf.h>

int taPAProcess(const void* in, void* out, unsigned long nframes, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags flags, void* inst) {
  TAAudioPA* instance=(TAAudioPA*)inst;
  return instance->onProcess(in,out,nframes,timeInfo,flags);
}

int TAAudioPA::onProcess(const void* in, void* out, unsigned long nframes, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags flags) {
  for (int i=0; i<desc.inChans; i++) {
    if (nframes>desc.bufsize) {
      delete[] inBufs[i];
      inBufs[i]=new float[nframes];
    }
  }
  for (int i=0; i<desc.outChans; i++) {
    if (nframes>desc.bufsize) {
      delete[] outBufs[i];
      outBufs[i]=new float[nframes];
    }
  }
  if (nframes!=desc.bufsize) {
    desc.bufsize=nframes;
  }

  if (audioProcCallback!=NULL) {
    if (midiIn!=NULL) midiIn->gather();
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,desc.bufsize);
  }
  float* fbuf=(float*)out;
  for (size_t j=0; j<desc.bufsize; j++) {
    for (size_t i=0; i<desc.outChans; i++) {
      fbuf[j*desc.outChans+i]=outBufs[i][j];
    }
  }
  return 0;
}

void* TAAudioPA::getContext() {
  return (void*)&ac;
}

bool TAAudioPA::quit() {
  if (!initialized) return false;

  Pa_CloseStream(ac);

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

bool TAAudioPA::setRun(bool run) {
  if (!initialized) return false;
  if (running==run) return running;
  PaError status;
  if (run) {
    status=Pa_StartStream(ac);
  } else {
    status=Pa_StopStream(ac);
  }
  if (status!=paNoError) {
    logW("error while setting run status: %s",Pa_GetErrorText(status));
    return running;
  }
  running=run;
  return running;
}

std::vector<String> TAAudioPA::listAudioDevices() {
  std::vector<String> ret;
  if (!audioSysStarted) {
    PaError status=Pa_Initialize();
    if (status!=paNoError) {
      logE("could not initialize PortAudio to list audio devices");
      return ret;
    } else {
      audioSysStarted=true;
    }
  }

  int count=Pa_GetDeviceCount();
  if (count<0) return ret;

  for (int i=0; i<count; i++) {
    const PaDeviceInfo* devInfo=Pa_GetDeviceInfo(i);
    if (devInfo==NULL) continue;
    if (devInfo->maxOutputChannels<1) continue;

    String devName;

    const PaHostApiInfo* driverInfo=Pa_GetHostApiInfo(devInfo->hostApi);
    if (driverInfo==NULL) {
      devName+="[???] ";
    } else {
      devName+=fmt::sprintf("[%s] ",driverInfo->name);
    }

    if (devInfo->name!=NULL) {
      devName+=devInfo->name;
      ret.push_back(devName);
    }
  }

  return ret;
}

bool TAAudioPA::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) {
    logE("audio already initialized");
    return false;
  }
  PaError status;

  if (!audioSysStarted) {
    status=Pa_Initialize();
    if (status!=paNoError) {
      logE("could not initialize PortAudio");
      return false;
    } else {
      audioSysStarted=true;
    }
  }

  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;

  const PaDeviceInfo* devInfo=NULL;
  const PaHostApiInfo* driverInfo=NULL;
  int outDeviceID=0;

  if (desc.deviceName.empty()) {
    outDeviceID=Pa_GetDefaultOutputDevice();
    devInfo=Pa_GetDeviceInfo(outDeviceID);
    if (devInfo!=NULL) {
      driverInfo=Pa_GetHostApiInfo(devInfo->hostApi);
    }
  } else {
    int count=Pa_GetDeviceCount();
    bool found=false;
    if (count<0) {
      logE("audio device not found");
      return false;
    }

    for (int i=0; i<count; i++) {
      devInfo=Pa_GetDeviceInfo(i);
      if (devInfo==NULL) continue;
      if (devInfo->maxOutputChannels<1) continue;

      String devName;

      driverInfo=Pa_GetHostApiInfo(devInfo->hostApi);
      if (driverInfo==NULL) {
        devName+="[???] ";
      } else {
        devName+=fmt::sprintf("[%s] ",driverInfo->name);
      }

      if (devInfo->name!=NULL) {
        devName+=devInfo->name;
      } else {
        continue;
      }

      if (devName==desc.deviceName) {
        outDeviceID=i;
        found=true;
        break;
      }
    }
    if (!found) {
      logE("audio device not found");
      return false;
    }
  }

  // check output channels and sample rate
  if (devInfo!=NULL) {
    if (desc.outChans>devInfo->maxOutputChannels) desc.outChans=devInfo->maxOutputChannels;
  }

  PaStreamParameters outParams;
  outParams.device=outDeviceID;
  outParams.channelCount=desc.outChans;
  outParams.sampleFormat=paFloat32;
  outParams.suggestedLatency=(double)(desc.bufsize*desc.fragments)/desc.rate;
  outParams.hostApiSpecificStreamInfo=NULL;

  if (driverInfo!=NULL) {
#ifdef _WIN32
    if (driverInfo->type==paWASAPI) {
      logV("setting WASAPI-specific flags");
      PaWasapiStreamInfo* wasapiInfo=new PaWasapiStreamInfo;
      memset(wasapiInfo,0,sizeof(PaWasapiStreamInfo));
      wasapiInfo->size=sizeof(PaWasapiStreamInfo);
      wasapiInfo->hostApiType=paWASAPI;
      wasapiInfo->version=1;
      wasapiInfo->flags=paWinWasapiThreadPriority;
      wasapiInfo->threadPriority=eThreadPriorityProAudio;
      wasapiInfo->streamCategory=eAudioCategoryMedia;
      wasapiInfo->streamOption=eStreamOptionRaw;

      if (desc.wasapiEx) {
        wasapiInfo->flags|=paWinWasapiExclusive;
      }

      outParams.hostApiSpecificStreamInfo=wasapiInfo;
    }
#endif
  }

  logV("opening audio device...");
  status=Pa_OpenStream(
    &ac,
    NULL,
    &outParams,
    desc.rate,
    0,
    paClipOff|paDitherOff,
    taPAProcess,
    this
  );
  if (status!=paNoError) {
    logE("could not open audio device: %s",Pa_GetErrorText(status));
    return false;
  }

  desc.deviceName=devInfo->name;
  desc.inChans=0;

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
