#include "taAudio.h"

void TAAudio::setSampleRateChangeCallback(void (*callback)(SampleRateChangeEvent)) {
  sampleRateChanged=callback;
}

void TAAudio::setBufferSizeChangeCallback(void (*callback)(BufferSizeChangeEvent)) {
  bufferSizeChanged=callback;
}

void TAAudio::setCallback(void (*callback)(void*,float**,float**,int,int,unsigned int), void* user) {
  audioProcCallback=callback;
  audioProcCallbackUser=user;
}

void* TAAudio::getContext() {
  return NULL;
}

bool TAAudio::quit() {
  return true;
}

bool TAAudio::setRun(bool run) {
  running=run;
  return running;
}

std::vector<String> TAAudio::listAudioDevices() {
  return std::vector<String>();
}

bool TAAudio::init(TAAudioDesc& request, TAAudioDesc& response) {
  response=request;
  return true;
}

TAAudio::~TAAudio() {
}