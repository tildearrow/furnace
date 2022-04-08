#include "waveSynth.h"
#include "engine.h"
#include "instrument.h"

bool DivWaveSynth::activeChanged() {
  if (activeChangedB) {
    activeChangedB=false;
    return true;
  }
  return false;
}

bool DivWaveSynth::tick() {
  bool updated=first;
  first=false;
  if (!state.enabled) return updated;

  if (--divCounter<=0) {
    // run effect
    switch (state.effect) {
      case DIV_WS_INVERT:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=height-output[pos];
          if (++pos>=width) pos=0;
        }
        updated=true;
        break;
    }
    divCounter=state.rateDivider;
  }
  
  return updated;
}

void DivWaveSynth::changeWave1(int num) {
  DivWavetable* w1=e->getWave(num);
  for (int i=0; i<width; i++) {
    if (w1->max<1 || w1->len<1) {
      wave1[i]=0;
      output[i]=0;
    } else {
      int data=w1->data[i*w1->len/width]*height/w1->max;
      if (data<0) data=0;
      if (data>height) data=height;
      wave1[i]=data;
      output[i]=data;
    }
  }
  first=true;
}

void DivWaveSynth::changeWave2(int num) {
  DivWavetable* w2=e->getWave(num);
  for (int i=0; i<width; i++) {
    if (w2->max<1 || w2->len<1) {
      wave2[i]=0;
    } else {
      int data=w2->data[i*w2->len/width]*height/w2->max;
      if (data<0) data=0;
      if (data>height) data=height;
      wave2[i]=data;
    }
  }
  first=true;
}

void DivWaveSynth::setEngine(DivEngine* engine) {
  e=engine;
}

void DivWaveSynth::init(DivInstrument* which, int w, int h, bool insChanged) {
  width=w;
  height=h;
  if (width<0) width=0;
  if (width>256) width=256;
  if (e==NULL) return;
  if (which==NULL) {
    if (state.enabled) activeChangedB=true;
    state=DivInstrumentWaveSynth();
    return;
  }
  if (!which->ws.enabled) {
    if (state.enabled) activeChangedB=true;
    state=DivInstrumentWaveSynth();
    return;
  } else {
    if (!state.enabled) activeChangedB=true;
  }
  state=which->ws;
  if (insChanged || !state.global) {
    pos=0;
    stage=0;
    divCounter=1+state.rateDivider;
    first=true;

    changeWave1(state.wave1);
    changeWave2(state.wave2);
  }
}