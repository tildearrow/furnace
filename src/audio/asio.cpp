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
#include "asio.h"
#include "../ta-log.h"

static TAAudioASIO* callbackInstance=NULL;

static void _onBufferSwitch(long index, ASIOBool isDirect) {
  if (callbackInstance==NULL) return;
  callbackInstance->onProcess(index);
}

static void _onSampleRate(ASIOSampleRate rate) {
  if (callbackInstance==NULL) return;
  callbackInstance->onSampleRate(*(double*)(&rate));
}

static long _onMessage(long type, long value, void* msg, double* opt) {
  return 0;
}

void TAAudioASIO::onSampleRate(double rate) {
  sampleRateChanged(SampleRateChangeEvent(rate));
}

void TAAudioASIO::onBufferSize(int bufsize) {
  bufferSizeChanged(BufferSizeChangeEvent(bufsize));
}

void TAAudioASIO::onProcess(int index) {
  if (audioProcCallback!=NULL) {
    if (midiIn!=NULL) midiIn->gather();
    audioProcCallback(audioProcCallbackUser,inBufs,outBufs,desc.inChans,desc.outChans,desc.bufsize);
  }

  // upload here...
  for (int i=0; i<totalChans; i++) {
    if (chanInfo[i].isInput==ASIOTrue) continue;
    int ch=chanInfo[i].channel;
    if (ch>=desc.outChans) continue;
    float* srcBuf=outBufs[ch];

    switch (chanInfo[i].type) {
      // little-endian
      case ASIOSTInt16LSB: {
        short* buf=(short*)bufInfo[i].buffers[index];
        for (unsigned int j=0; j<desc.bufsize; j++) {
          buf[j]=CLAMP(srcBuf[j],-1.0,1.0)*32767.0f;
        }
        break;
      }
      // TODO: how does this work? it's vaguely described in the docs
      case ASIOSTInt24LSB: {
        break;
      }
      case ASIOSTInt32LSB: {
        int* buf=(int*)bufInfo[i].buffers[index];
        for (unsigned int j=0; j<desc.bufsize; j++) {
          int val=CLAMP(srcBuf[j],-1.0,1.0)*8388608.0f;
          if (val<-8388608) val=-8388608;
          if (val>8388607) val=-8388607;
          val<<=8;
          buf[j]=val;
        }
        break;
      }
      case ASIOSTFloat32LSB: {
        float* buf=(float*)bufInfo[i].buffers[index];
        for (unsigned int j=0; j<desc.bufsize; j++) {
          buf[j]=srcBuf[j];
        }
        break;
      }
      case ASIOSTFloat64LSB: {
        double* buf=(double*)bufInfo[i].buffers[index];
        for (unsigned int j=0; j<desc.bufsize; j++) {
          buf[j]=srcBuf[j];
        }
        break;
      }

      // TODO: implement these formats D:
      // big-endian
      case ASIOSTInt16MSB: {
        break;
      }
      case ASIOSTInt24MSB: {
        break;
      }
      case ASIOSTInt32MSB: {
        break;
      }
      case ASIOSTFloat32MSB: {
        break;
      }
      case ASIOSTFloat64MSB: {
        break;
      }

      // what the hell..............
      case ASIOSTInt32LSB16: {
        break;
      }
      case ASIOSTInt32LSB18: {
        break;
      }
      case ASIOSTInt32LSB20: {
        break;
      }
      case ASIOSTInt32LSB24: {
        break;
      }
      case ASIOSTInt32MSB16: {
        break;
      }
      case ASIOSTInt32MSB18: {
        break;
      }
      case ASIOSTInt32MSB20: {
        break;
      }
      case ASIOSTInt32MSB24: {
        break;
      }
      default: // unsupported
        break;
    }
  }

  /*if (nframes!=desc.bufsize) {
    desc.bufsize=nframes;
  }*/
}

void* TAAudioASIO::getContext() {
  return (void*)&driverInfo;
}

bool TAAudioASIO::quit() {
  if (!initialized) return false;

  if (running) {
    logV("CRASH: STOPPING NOW (QUIT)......");
    ASIOStop();
    running=false;
  }

  logV("CRASH: ASIODisposeBuffers()");
  ASIODisposeBuffers();
  
  logV("CRASH: erase inBufs");
  for (int i=0; i<desc.inChans; i++) {
    delete[] inBufs[i];
    inBufs[i]=NULL;
  }
  logV("CRASH: erase outBufs");
  for (int i=0; i<desc.outChans; i++) {
    delete[] outBufs[i];
    outBufs[i]=NULL;
  }

  logV("CRASH: erase arrays");
  if (inBufs!=NULL) {
    delete[] inBufs;
    inBufs=NULL;
  }
  if (outBufs!=NULL) {
    delete[] outBufs;
    outBufs=NULL;
  }

  logV("CRASH: ASIOExit()");
  ASIOExit();
  logV("CRASH: removeCurrentDriver()");
  drivers.removeCurrentDriver();

  logV("CRASH: reset callback instance");
  callbackInstance=NULL;
  initialized=false;

  return true;
}

bool TAAudioASIO::setRun(bool run) {
  if (!initialized) return false;
  if (running==run) {
    return running;
  }
  if (run) {
    if (ASIOStart()!=ASE_OK) return false;
    running=true;
  } else {
    // does it matter whether stop was successful?
    logV("CRASH: STOPPING NOW......");
    ASIOStop();
    running=false;
  }
  return running;
}

bool TAAudioASIO::init(TAAudioDesc& request, TAAudioDesc& response) {
  if (initialized) return false;
  if (callbackInstance) {
    logE("can't initialize more than one output!");
    return false;
  }

  desc=request;
  desc.outFormat=TA_AUDIO_FORMAT_F32;

  if (desc.deviceName.empty()) {
    // load first driver if not specified
    logV("getting driver names...");
    if (!driverNamesInit) {
      for (int i=0; i<ASIO_DRIVER_MAX; i++) {
        // 64 just in case
        driverNames[i]=new char[64];
      }
      driverNamesInit=true;
    }
    driverCount=drivers.getDriverNames(driverNames,ASIO_DRIVER_MAX);

    // quit if we couldn't find any drivers
    if (driverCount<1) {
      logE("no ASIO drivers available");
      return false;
    }

    desc.deviceName=driverNames[0];
  }

  // load driver
  logV("loading ASIO driver... (%s)",desc.deviceName);
  strncpy(deviceNameCopy,desc.deviceName.c_str(),63);
  if (!drivers.loadDriver(deviceNameCopy)) {
    logE("failed to load ASIO driver!");
    return false;
  }

  // init
  memset(&driverInfo,0,sizeof(driverInfo));
  memset(bufInfo,0,sizeof(ASIOBufferInfo)*ASIO_CHANNEL_MAX*2);
  memset(chanInfo,0,sizeof(ASIOChannelInfo)*ASIO_CHANNEL_MAX*2);
  driverInfo.asioVersion=2;
  driverInfo.sysRef=NULL;
  ASIOError result=ASIOInit(&driverInfo);

  if (result!=ASE_OK) {
    logE("could not init device!");
    drivers.removeCurrentDriver();
    return false;
  }

  // setup callbacks
  // unfortunately, only one TAAudio instance may exist at a time when using ASIO...
  callbackInstance=this;
  memset(&callbacks,0,sizeof(ASIOCallbacks));
  callbacks.bufferSwitch=_onBufferSwitch;
  callbacks.sampleRateDidChange=_onSampleRate;
  callbacks.asioMessage=_onMessage;

  // get driver information
  long maxInChans=0;
  long maxOutChans=0;

  result=ASIOGetChannels(&maxInChans,&maxOutChans);
  if (result!=ASE_OK) {
    logE("could not get channel count!");
    ASIOExit();
    drivers.removeCurrentDriver();
    return false;
  }

  if (maxInChans>ASIO_CHANNEL_MAX) maxInChans=ASIO_CHANNEL_MAX;
  if (maxOutChans>ASIO_CHANNEL_MAX) maxOutChans=ASIO_CHANNEL_MAX;

  if (desc.inChans>maxInChans) desc.inChans=maxInChans;
  if (desc.outChans>maxOutChans) desc.outChans=maxOutChans;

  long minBufSize=0;
  long maxBufSize=0;
  long actualBufSize=0;
  long bufSizeGranularity=0;
  result=ASIOGetBufferSize(&minBufSize,&maxBufSize,&actualBufSize,&bufSizeGranularity);
  if (result!=ASE_OK) {
    logE("could not get buffer size!");
    ASIOExit();
    drivers.removeCurrentDriver();
    return false;
  }

  ASIOSampleRate outRate;
  result=ASIOGetSampleRate(&outRate);
  if (result!=ASE_OK) {
    logE("could not get sample rate!");
    ASIOExit();
    drivers.removeCurrentDriver();
    return false;
  }
  desc.rate=*(double*)(&outRate);

  totalChans=0;

  if (desc.inChans>0) {
    inBufs=new float*[desc.inChans];
    for (int i=0; i<desc.inChans; i++) {
      chanInfo[totalChans].channel=i;
      chanInfo[totalChans].isInput=ASIOTrue;
      ASIOGetChannelInfo(&chanInfo[totalChans]);
      bufInfo[totalChans].channelNum=i;
      bufInfo[totalChans++].isInput=ASIOTrue;
      inBufs[i]=new float[actualBufSize];
    }
  }
  if (desc.outChans>0) {
    outBufs=new float*[desc.outChans];
    for (int i=0; i<desc.outChans; i++) {
      chanInfo[totalChans].channel=i;
      chanInfo[totalChans].isInput=ASIOFalse;
      ASIOGetChannelInfo(&chanInfo[totalChans]);
      bufInfo[totalChans].channelNum=i;
      bufInfo[totalChans++].isInput=ASIOFalse;
      outBufs[i]=new float[actualBufSize];
    }
  }

  result=ASIOCreateBuffers(bufInfo,totalChans,actualBufSize,&callbacks);
  if (result!=ASE_OK) {
    logE("could not create buffers!");
    if (inBufs!=NULL) {
      for (int i=0; i<desc.inChans; i++) {
        if (inBufs[i]!=NULL) {
          delete[] inBufs[i];
          inBufs[i]=NULL;
        }
      }
      delete[] inBufs;
      inBufs=NULL;
    }
    if (outBufs!=NULL) {
      for (int i=0; i<desc.outChans; i++) {
        if (outBufs[i]!=NULL) {
          delete[] outBufs[i];
          outBufs[i]=NULL;
        }
      }
      delete[] outBufs;
      outBufs=NULL;
    }
    ASIOExit();
    drivers.removeCurrentDriver();
    return false;
  }

  desc.bufsize=actualBufSize;
  desc.fragments=2;

  response=desc;
  initialized=true;
  return true;
}

std::vector<String> TAAudioASIO::listAudioDevices() {
  std::vector<String> ret;

  if (!driverNamesInit) {
    for (int i=0; i<ASIO_DRIVER_MAX; i++) {
      // 64 just in case
      driverNames[i]=new char[64];
    }
    driverNamesInit=true;
  }
  driverCount=drivers.getDriverNames(driverNames,ASIO_DRIVER_MAX);
  for (int i=0; i<driverCount; i++) {
    ret.push_back(driverNames[i]);
  }

  return ret;
}
