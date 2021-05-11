#include "taAudio.h"

void TAAudio::setSampleRateChangeCallback(void (*callback)(SampleRateChangeEvent)) {
  sampleRateChanged=callback;
}

void TAAudio::setBufferSizeChangeCallback(void (*callback)(BufferSizeChangeEvent)) {
  bufferSizeChanged=callback;
}

void TAAudio::setCallback(void (*callback)(float**,float**,int,int,unsigned int)) {
  audioProcCallback=callback;
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

bool TAAudio::init(TAAudioDesc& request, TAAudioDesc& response) {
  return false;
}