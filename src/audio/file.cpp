#include <sndfile.h>
#include <string.h>
#include "file.h"
#include "taAudio.h"

void TAAudioFile::onProcess(unsigned char* buf, int nframes) {
  if (audioProcCallback!=NULL) {
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,desc.bufsize);
  }
  /*
  float* fbuf=(float*)buf;
  for (size_t j=0; j<desc.bufsize; j++) {
    for (size_t i=0; i<desc.outChans; i++) {
      fbuf[j*desc.outChans+i]=outBufs[i][j];
    }
  }*/
}

void* TAAudioFile::getContext() {
  return file;
}

bool TAAudioFile::quit() {
  if (!initialized) return false;

  if (file!=NULL) {
    sf_close(file);
    file=NULL;
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

bool TAAudioFile::setRun(bool run) {
  if (!initialized) return false;
  SDL_PauseAudioDevice(ai,!run);
  running=run;
  return running;
}

bool TAAudioFile::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) return false;

  TAAudioDesc desc=request;

  if (desc.inChans>0) return false;

  memset(si,0,sizeof(SF_INFO));
  si.channels=request.outChans;
  si.samplerate=request.rate;
  switch (request.outFormat) {
    case TA_AUDIO_FORMAT_F32:
      si.format=SF_FORMAT_FLOAT;
      break;
    case TA_AUDIO_FORMAT_F64:
      si.format=SF_FORMAT_DOUBLE;
      break;
    case TA_AUDIO_FORMAT_U8:
      si.format=SF_FORMAT_PCM_U8;
      break;
    case TA_AUDIO_FORMAT_S8:
      si.format=SF_FORMAT_PCM_S8;
      break;
    case TA_AUDIO_FORMAT_S16:
      si.format=SF_FORMAT_PCM_16;
      break;
    case TA_AUDIO_FORMAT_S32:
      si.format=SF_FORMAT_PCM_32;
      break;
    default:
      return false;
  }
  si.format|=SF_FORMAT_WAV;

  file=sf_open(request.name.c_str(),SFM_WRITE,&si);
  if (file==NULL) return false;

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
