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
#include "jack.h"
#include "../ta-log.h"

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
  for (int i=0; i<desc.inChans; i++) {
    iInBufs[i]=(float*)jack_port_get_buffer(ai[i],nframes);
    if (nframes>desc.bufsize) {
      delete[] inBufs[i];
      inBufs[i]=new float[nframes];
    }
    memcpy(iInBufs[i],inBufs[i],nframes*sizeof(float));
  }
  for (int i=0; i<desc.outChans; i++) {
    if (nframes>desc.bufsize) {
      delete[] outBufs[i];
      outBufs[i]=new float[nframes];
    }
  }
  if (audioProcCallback!=NULL) {
    if (midiIn!=NULL) midiIn->gather();
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,nframes);
  }
  for (int i=0; i<desc.outChans; i++) {
    iOutBufs[i]=(float*)jack_port_get_buffer(ao[i],nframes);
    memcpy(iOutBufs[i],outBufs[i],nframes*sizeof(float));
  }
  if (nframes!=desc.bufsize) {
    desc.bufsize=nframes;
  }
}

void* TAAudioJACK::getContext() {
  return (void*)ac;
}

bool TAAudioJACK::quit() {
  if (!initialized) return false;

  if (running) {
    running=false;
    if (jack_deactivate(ac)) {
      logE("could not deactivate!");
      return false;
    }
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

  if (iInBufs!=NULL) delete[] iInBufs;
  if (iOutBufs!=NULL) delete[] iOutBufs;
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
    if (jack_activate(ac)) {
      logE("could not activate!");
      return false;
    }
    
    for (int i=0; i<desc.outChans; i++) {
      jack_connect(ac,(desc.name+String(":out")+std::to_string(i)).c_str(),(String("system:playback_")+std::to_string(i+1)).c_str());
    }
    running=true;
  } else {
    if (jack_deactivate(ac)) {
      logE("could not deactivate!");
      return true;
    }
    running=false;
  }
  return running;
}

String TAAudioJACK::printStatus(jack_status_t status) {
  String ret;
  if (status&JackFailure) {
    ret+="failure. ";
  }
  if (status&JackInvalidOption) {
    ret+="invalid option ";
  }
  if (status&JackNameNotUnique) {
    ret+="name not unique ";
  }
  if (status&JackServerStarted) {
    ret+="server started ";
  }
  if (status&JackServerFailed) {
    ret+="server failed ";
  }
  if (status&JackServerError) {
    ret+="server error ";
  }
  if (status&JackNoSuchClient) {
    ret+="no such client ";
  }
  if (status&JackLoadFailure) {
    ret+="load failure ";
  }
  if (status&JackInitFailure) {
    ret+="init failure ";
  }
  if (status&JackShmFailure) {
    ret+="shared memory failure ";
  }
  if (status&JackVersionError) {
    ret+="version error ";
  }
  if (status&JackBackendError) {
    ret+="backend error ";
  }
  if (status&JackClientZombie) {
    ret+="client is zombie ";
  }
  if (ret.empty()) {
    ret="no status";
  } else {
    if (ret[ret.size()-1]==' ') {
      ret.resize(ret.size()-1);
    }
  }
  return ret;
}

bool TAAudioJACK::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) return false;
  int haveJACK=have_libjack();
  if (haveJACK==-1) {
    logE("JACK library not initialized!");
    return false;
  }
  if (haveJACK==-2) {
    logE("JACK not installed!");
    return false;
  }
  if (haveJACK!=0) {
    logE("JACK symbol error!");
    return false;
  }
  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;

  jack_status_t as;
  ac=jack_client_open(desc.name.c_str(),JackNoStartServer,&as);
  if (ac==NULL) {
    logE("error while opening client! (%s)",printStatus(as));
    return false;
  }

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
