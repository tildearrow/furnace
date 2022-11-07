/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

bool DivWaveSynth::tick(bool skipSubDiv) {
  bool updated=first;
  first=false;
  if (--subDivCounter>0 && !skipSubDiv) {
    return updated;
  }

  subDivCounter=e->tickMult;
  if (!state.enabled) return updated;
  if (width<1) return false;

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
      case DIV_WS_ADD:
        for (int i=0; i<=state.speed; i++) {
          output[pos]+=MIN(height,state.param1);
          if (output[pos]>=height) output[pos]-=height;
          if (++pos>=width) pos=0;
        }
        updated=true;
        break;
      case DIV_WS_SUBTRACT:
        for (int i=0; i<=state.speed; i++) {
          output[pos]-=MIN(height,state.param1);
          if (output[pos]<0) output[pos]+=height;
          if (++pos>=width) pos=0;
        }
        updated=true;
        break;
      case DIV_WS_AVERAGE:
        for (int i=0; i<=state.speed; i++) {
          int pos1=(pos+1>=width)?0:(pos+1);
          output[pos]=(128+output[pos]*(256-state.param1)+output[pos1]*state.param1)>>8;
          if (output[pos]<0) output[pos]=0;
          if (output[pos]>height) output[pos]=height;
          if (++pos>=width) pos=0;
        }
        updated=true;
        break;
      case DIV_WS_PHASE:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=wave1[(pos+stage)%width];
          if (++pos>=width) {
            pos=0;
            if (++stage>=width) stage=0;
          }
        }
        updated=true;
        break;
      case DIV_WS_CHORUS:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=(wave1[pos]+wave1[(pos+stage)%width])>>1;
          if (++pos>=width) {
            pos=0;
            stage+=state.param1;
            while (stage>=width) stage-=width;
          }
        }
        updated=true;
        break;
      case DIV_WS_WIPE:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=(stage&1)?wave1[pos]:wave2[pos];
          if (output[pos]<0) output[pos]=0;
          if (output[pos]>height) output[pos]=height;
          if (++pos>=width) {
            pos=0;
            stage=!stage;
          }
        }
        updated=true;
        break;
      case DIV_WS_FADE:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=wave1[pos]+(((wave2[pos]-wave1[pos])*stage)>>9);
          if (++pos>=width) {
            pos=0;
            stage+=1+state.param1;
            if (stage>512) stage=512;
          }
        }
        updated=true;
        break;
      case DIV_WS_PING_PONG:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=wave1[pos]+(((wave2[pos]-wave1[pos])*stage)>>8);
          if (++pos>=width) {
            pos=0;
            if (stageDir) {
              stage-=1+state.param1;
              if (stage<=0) {
                stageDir=false;
                stage=0;
              }
            } else {
              stage+=1+state.param1;
              if (stage>=256) {
                stageDir=true;
                stage=256;
              }
            }
          }
        }
        updated=true;
        break;
      case DIV_WS_OVERLAY:
        for (int i=0; i<=state.speed; i++) {
          output[pos]+=wave2[pos];
          if (output[pos]>=height) output[pos]-=height;
          if (++pos>=width) pos=0;
        }
        updated=true;
        break;
      case DIV_WS_NEGATIVE_OVERLAY:
        for (int i=0; i<=state.speed; i++) {
          output[pos]-=wave2[pos];
          if (output[pos]<0) output[pos]+=height;
          if (++pos>=width) pos=0;
        }
        updated=true;
        break;
      case DIV_WS_SLIDE:
        for (int i=0; i<=state.speed; i++) {
          int newPos=(pos+stage)%(width*2);
          if (newPos>=width) {
            output[pos]=wave2[newPos-width];
          } else {
            output[pos]=wave1[newPos];
          }
          if (++pos>=width) {
            pos=0;
            if (++stage>=width*2) stage=0;
          }
        }
        updated=true;
        break;
      case DIV_WS_MIX:
        for (int i=0; i<=state.speed; i++) {
          output[pos]=(wave1[pos]+wave2[(pos+stage)%width])>>1;
          if (++pos>=width) {
            pos=0;
            stage+=state.param1;
            while (stage>=width) stage-=width;
          }
        }
        updated=true;
        break;
      case DIV_WS_PHASE_MOD:
        for (int i=0; i<=state.speed; i++) {
          int mod=(wave2[pos]*(state.param2-stage)*width)/(64*(height+1));
          output[pos]=wave1[(pos+mod)%width];
          if (++pos>=width) {
            pos=0;
            stage+=state.param1;
            if (stage>state.param2) stage=state.param2;
          }
        }
        updated=true;
        break;
    }
    divCounter=state.rateDivider;
  }
  
  return updated;
}

void DivWaveSynth::setWidth(int val) {
  width=val;
  if (width<0) width=0;
  if (width>256) width=256;
}

void DivWaveSynth::changeWave1(int num) {
  DivWavetable* w1=e->getWave(num);
  if (width<1) return;
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
  if (width<1) return;
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

void DivWaveSynth::setEngine(DivEngine* engine, int waveFloor) {
  e=engine;
  memset(wave1,waveFloor,256);
  memset(wave2,waveFloor,256);
  for (int i=0; i<256; i++) {
    output[i]=waveFloor;
  }
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
    stageDir=false;
    divCounter=0;
    subDivCounter=0;

    changeWave1(state.wave1);
    changeWave2(state.wave2);
    tick(true);
    first=true;
  }
}
