#include "waveSynth.h"
#include "engine.h"

bool DivWaveSynth::tick() {
  bool updated=first;
  first=false;

  
  return updated;
}

void DivWaveSynth::setEngine(DivEngine* engine) {
  e=engine;
}

void DivWaveSynth::init(DivInstrument* which, int w, int h, bool insChanged) {
  if (e==NULL) return;
  if (which==NULL) {
    state=DivInstrumentWaveSynth();
  }
  state=which->ws;
  width=w;
  height=h;
  pos=0;
  stage=0;
  divCounter=0;
  first=true;

  DivWavetable* w1=e->getWave(state.wave1);
  DivWavetable* w2=e->getWave(state.wave2);
  for (int i=0; i<width; i++) {
    if (w1->max<1 || w1->len<1) {
      wave1[i]=0;
    } else {
      int data=w1->data[i*w1->len/width]*height/w1->max;
      if (data<0) data=0;
      if (data>31) data=31;
      wave1[i]=data;
    }
  }

  for (int i=0; i<width; i++) {
    if (w2->max<1 || w2->len<1) {
      wave2[i]=0;
    } else {
      int data=w2->data[i*w2->len/width]*height/w2->max;
      if (data<0) data=0;
      if (data>31) data=31;
      wave2[i]=data;
    }
  }
}