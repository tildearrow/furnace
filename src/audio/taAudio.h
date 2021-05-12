#ifndef _TAAUDIO_H
#define _TAAUDIO_H
#include "../ta-utils.h"

struct SampleRateChangeEvent {
  double rate;
  SampleRateChangeEvent(double r):
    rate(r) {}
};

struct BufferSizeChangeEvent {
  unsigned int bufsize;
  BufferSizeChangeEvent(unsigned int bs):
    bufsize(bs) {}
};

enum TAAudioFormat {
  TA_AUDIO_FORMAT_F32=0,
  TA_AUDIO_FORMAT_F64,
  TA_AUDIO_FORMAT_U8,
  TA_AUDIO_FORMAT_S8,
  TA_AUDIO_FORMAT_U16,
  TA_AUDIO_FORMAT_S16,
  TA_AUDIO_FORMAT_U32,
  TA_AUDIO_FORMAT_S32,
  TA_AUDIO_FORMAT_U16BE,
  TA_AUDIO_FORMAT_S16BE,
  TA_AUDIO_FORMAT_U32BE,
  TA_AUDIO_FORMAT_S32BE
};

struct TAAudioDesc {
  String name;
  double rate;
  unsigned int bufsize, fragments;
  unsigned char inChans, outChans;
  TAAudioFormat outFormat;

  TAAudioDesc():
    rate(0.0),
    bufsize(0),
    fragments(0),
    inChans(0),
    outChans(0),
    outFormat(TA_AUDIO_FORMAT_F32) {}
};

class TAAudio {
  protected:
    TAAudioDesc desc;
    TAAudioFormat outFormat;
    bool running, initialized;
    float** inBufs;
    float** outBufs;
    void (*audioProcCallback)(void*,float**,float**,int,int,unsigned int);
    void* audioProcCallbackUser;
    void (*sampleRateChanged)(SampleRateChangeEvent);
    void (*bufferSizeChanged)(BufferSizeChangeEvent);
  public:
    void setSampleRateChangeCallback(void (*callback)(SampleRateChangeEvent));
    void setBufferSizeChangeCallback(void (*callback)(BufferSizeChangeEvent));

    void setCallback(void (*callback)(void*,float**,float**,int,int,unsigned int), void* user);

    virtual void* getContext();
    virtual bool quit();
    virtual bool setRun(bool run);
    virtual bool init(TAAudioDesc& request, TAAudioDesc& response);

    TAAudio():
      outFormat(TA_AUDIO_FORMAT_F32),
      running(false),
      initialized(false),
      inBufs(NULL),
      outBufs(NULL),
      audioProcCallback(NULL),
      sampleRateChanged(NULL),
      bufferSizeChanged(NULL) {}
};
#endif
