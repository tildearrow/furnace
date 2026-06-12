/**
 * Furnace Tracker - oscilloscope video render helper
 * Engine-side render loop: takes over audio backend, renders song,
 * fires per-frame callback with audio samples so GUI can render pixels.
 */

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

  float* outBuf[2];
  outBuf[0]=new float[OSC_EXPORT_BUFSIZE];
  outBuf[1]=new float[OSC_EXPORT_BUFSIZE];

  // frame accumulation buffers
  float* frameBuf[2];
  frameBuf[0]=new float[samplesPerFrame];
  frameBuf[1]=new float[samplesPerFrame];

  // take over audio (same as runExportThread)
  deinitAudioBackend();
  stop();
  freelance=false;
  setOrder(0);
  repeatPattern=false;
  remainingLoops=loops; // counts down each loop; -1 = unlimited
  playSub(false);
  freelance=false;

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

  // flush partial last frame (zero-padded)
  if (frameSamplePos>0) {
    for (int i=frameSamplePos; i<samplesPerFrame; i++) {
      frameBuf[0][i]=0.0f;
      frameBuf[1][i]=0.0f;
    }
    frameCallback(frameIdx++, frameBuf, samplesPerFrame);
  }

  logI("OscVideoRender: rendered %d frames",frameIdx);

  delete[] frameBuf[1];
  delete[] frameBuf[0];
  delete[] outBuf[1];
  delete[] outBuf[0];

  // restore audio backend (same as runExportThread)
  if (initAudioBackend()) {
    for (int i=0; i<song.systemLen; i++) {
      disCont[i].setRates(got.rate);
      disCont[i].setQuality(lowQuality,dcHiPass);
    }
    if (!output->setRun(true)) {
      logE("OscVideoRender: error while reactivating audio!");
    }
  }

  return true;
}
