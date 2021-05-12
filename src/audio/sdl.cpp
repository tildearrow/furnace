#include <string.h>
#include "sdl.h"

void taSDLProcess(void* inst, unsigned char* buf, int nframes) {
  TAAudioSDL* in=(TAAudioSDL*)inst;
  in->onProcess(buf,nframes);
}

void TAAudioSDL::onProcess(unsigned char* buf, int nframes) {
  if (audioProcCallback!=NULL) {
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,desc.bufsize);
  }
  float* fbuf=(float*)buf;
  for (size_t j=0; j<desc.bufsize; j++) {
    for (size_t i=0; i<desc.outChans; i++) {
      fbuf[j*desc.outChans+i]=outBufs[i][j];
    }
  }
}

void* TAAudioSDL::getContext() {
  return (void*)&ac;
}

bool TAAudioSDL::quit() {
  if (!initialized) return false;

  SDL_CloseAudioDevice(ai);

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

bool TAAudioSDL::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) return false;
  if (SDL_Init(SDL_INIT_AUDIO)<0) return false;

  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;

  ac.freq=desc.rate;
  ac.format=AUDIO_F32;
  ac.channels=desc.outChans;
  ac.samples=desc.bufsize;
  ac.callback=taSDLProcess;
  ac.userdata=this;

  ai=SDL_OpenAudioDevice(NULL,0,&ac,&ar,SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
  if (ai==0) return false;

  desc.name="";
  desc.rate=ar.freq;
  desc.inChans=0;
  desc.outChans=ar.channels;
  desc.bufsize=ar.samples;
  desc.fragments=1;

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
