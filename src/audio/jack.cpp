#include <string.h>
#include "jack.h"

int taJACKonSampleRate(jack_nframes_t rate, void* inst) {
  TAAudioJACK* in=(TAAudioJACK*)inst;
  in->onSampleRate(rate);
  return 0;
}

int taJACKonBufferSize(jack_nframes_t bufsize, void* inst) {
  TAAudioJACK* in=(TAAudioJACK*)inst;
  in->onBufferSize(bufsize);
  return 0;
}

int taJACKProcess(jack_nframes_t nframes, void* inst) {
  TAAudioJACK* in=(TAAudioJACK*)inst;
  in->onProcess(nframes);
  return 0;
}

void TAAudioJACK::onSampleRate(jack_nframes_t rate) {
  if (sampleRateChanged!=NULL) {
    sampleRateChanged(SampleRateChangeEvent(rate));
  }
}

void TAAudioJACK::onBufferSize(jack_nframes_t bufsize) {
  if (bufferSizeChanged!=NULL) {
    bufferSizeChanged(BufferSizeChangeEvent(bufsize));
  }
}

void TAAudioJACK::onProcess(jack_nframes_t nframes) {
  if (audioProcCallback!=NULL) {
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,desc.bufsize);
  }
  for (int i=0; i<desc.inChans; i++) {
    iInBufs[i]=(float*)jack_port_get_buffer(ai[i],nframes);
    memcpy(iInBufs[i],inBufs[i],desc.bufsize*sizeof(float));
  }
  for (int i=0; i<desc.outChans; i++) {
    iOutBufs[i]=(float*)jack_port_get_buffer(ao[i],nframes);
    memcpy(iOutBufs[i],outBufs[i],desc.bufsize*sizeof(float));
  }
}

void* TAAudioJACK::getContext() {
  return (void*)ac;
}

bool TAAudioJACK::quit() {
  if (!initialized) return false;

  if (running) {
    running=false;
    if (jack_deactivate(ac)) return false;
  }
  
  for (int i=0; i<desc.inChans; i++) {
    jack_port_unregister(ac,ai[i]);
    ai[i]=NULL;
    delete[] inBufs[i];
  }
  for (int i=0; i<desc.outChans; i++) {
    jack_port_unregister(ac,ao[i]);
    ao[i]=NULL;
    delete[] outBufs[i];
  }

  delete[] iInBufs;
  delete[] iOutBufs;
  delete[] inBufs;
  delete[] outBufs;
  delete[] ai;
  delete[] ao;
  
  jack_client_close(ac);
  ac=NULL;

  initialized=false;

  return true;
}

bool TAAudioJACK::setRun(bool run) {
  if (!initialized) return false;
  if (running==run) {
    return running;
  }
  if (run) {
    if (jack_activate(ac)) return false;
    
    for (int i=0; i<desc.outChans; i++) {
      jack_connect(ac,(desc.name+String(":out")+std::to_string(i)).c_str(),(String("system:playback_")+std::to_string(i+1)).c_str());
    }
    running=true;
  } else {
    if (jack_deactivate(ac)) return true;
    running=false;
  }
  return running;
}

bool TAAudioJACK::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) return false;
  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;

  jack_status_t as;
  ac=jack_client_open(desc.name.c_str(),JackNoStartServer,&as);
  if (ac==NULL) return false;

  desc.name=String(jack_get_client_name(ac));
  jack_set_sample_rate_callback(ac,taJACKonSampleRate,this);
  jack_set_buffer_size_callback(ac,taJACKonBufferSize,this);
  jack_set_process_callback(ac,taJACKProcess,this);

  jack_nframes_t count=jack_get_buffer_size(ac);
  desc.bufsize=count;
  desc.fragments=1;

  jack_nframes_t sampleRate=jack_get_sample_rate(ac);
  desc.rate=sampleRate;

  if (desc.inChans>0) {
    inBufs=new float*[desc.inChans];
    iInBufs=new float*[desc.inChans];
    ai=new jack_port_t*[desc.inChans];
    for (int i=0; i<desc.inChans; i++) {
      ai[i]=jack_port_register(ac,(String("in")+std::to_string(i)).c_str(),JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput,0);
      if (ai[i]==NULL) {
        desc.inChans=i;
        break;
      }
      inBufs[i]=new float[count];
    }
  }
  if (desc.outChans>0) {
    outBufs=new float*[desc.outChans];
    iOutBufs=new float*[desc.outChans];
    ao=new jack_port_t*[desc.outChans];
    for (int i=0; i<desc.outChans; i++) {
      ao[i]=jack_port_register(ac,(String("out")+std::to_string(i)).c_str(),JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
      if (ao[i]==NULL) {
        desc.outChans=i;
        break;
      }
      outBufs[i]=new float[count];
    }
  }

  response=desc;
  initialized=true;
  return true;
}