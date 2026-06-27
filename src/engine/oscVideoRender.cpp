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

// oscilloscope video render helper: takes over the audio backend, renders
// the song, and fires a per-frame callback with audio samples.

#include "engine.h"
#include "../ta-log.h"

#define OSC_EXPORT_BUFSIZE 2048

bool DivEngine::renderOscVideo(int fps, int loops,
    const std::function<void(int frameIdx, float** audio, int samplesPerFrame)>& frameCallback,
    const std::function<bool()>& shouldStop) {

  int rate=(int)got.rate;
  if (rate<=0) rate=44100;
  int samplesPerFrame=rate/fps;
  if (samplesPerFrame<1) samplesPerFrame=1;

  float outBuf0[OSC_EXPORT_BUFSIZE];
  float outBuf1[OSC_EXPORT_BUFSIZE];
  float* outBuf[2]={outBuf0,outBuf1};

  std::vector<float> frameBufData((size_t)samplesPerFrame*2);
  float* frameBuf[2]={frameBufData.data(), frameBufData.data()+samplesPerFrame};

  deinitAudioBackend();
  stop();
  freelance=false;
  setOrder(0);
  repeatPattern=false;
  remainingLoops=loops;
  playSub(false);
  freelance=false;

  if (!playing) {
    logE("OscVideoRender: engine did not start");
    if (initAudioBackend()) {
      for (int i=0; i<song.systemLen; i++) {
        disCont[i].setRates(got.rate);
        disCont[i].setQuality(lowQuality,dcHiPass);
      }
      output->setRun(true);
    }
    return false;
  }

  logI("OscVideoRender: rate=%d fps=%d spf=%d",rate,fps,samplesPerFrame);

  int frameSamplePos=0;
  int frameIdx=0;

  while (playing) {
    if (shouldStop && shouldStop()) break;

    nextBuf(NULL,outBuf,0,2,OSC_EXPORT_BUFSIZE);

    int processed=(int)totalProcessed;
    if (processed>OSC_EXPORT_BUFSIZE) processed=OSC_EXPORT_BUFSIZE;

    for (int i=0; i<processed; i++) {
      frameBuf[0][frameSamplePos]=outBuf[0][i];
      frameBuf[1][frameSamplePos]=outBuf[1][i];
      frameSamplePos++;
      if (frameSamplePos>=samplesPerFrame) {
        frameCallback(frameIdx++, frameBuf, samplesPerFrame);
        frameSamplePos=0;
      }
    }
  }

  if (frameSamplePos>0) {
    for (int i=frameSamplePos; i<samplesPerFrame; i++) {
      frameBuf[0][i]=0.0f;
      frameBuf[1][i]=0.0f;
    }
    frameCallback(frameIdx++, frameBuf, samplesPerFrame);
  }

  logI("OscVideoRender: rendered %d frames",frameIdx);

  if (initAudioBackend()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].setRates(got.rate);
      disCont[i].setQuality(lowQuality,dcHiPass);
    }
    if (!output->setRun(true)) {
      logE("OscVideoRender: error while reactivating audio");
    }
  }

  return true;
}
