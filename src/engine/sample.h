#include "../ta-utils.h"

struct DivSample {
  String name;
  int length, rate, centerRate, loopStart, loopOffP;
  signed char vol, pitch;
  // valid values are:
  // - 0: ZX Spectrum overlay drum (1-bit PCM)
  // - 1: 1-bit NES DPCM
  // - 4: BRR
  // - 5: raw ADPCM-A
  // - 6: raw ADPCM-B
  // - 8: 8-bit PCM
  // - 16: 16-bit PCM
  unsigned char depth;
  short* data;
  unsigned int rendLength, adpcmRendLength, rendOff, rendOffP, rendOffContiguous;
  short* rendData;
  unsigned char* adpcmRendData;

  bool save(const char* path);
  DivSample():
    name(""),
    length(0),
    rate(32000),
    centerRate(8363),
    loopStart(-1),
    loopOffP(0),
    vol(0),
    pitch(0),
    depth(16),
    data(NULL),
    rendLength(0),
    adpcmRendLength(0),
    rendOff(0),
    rendOffP(0),
    rendOffContiguous(0),
    rendData(NULL),
    adpcmRendData(NULL) {}
  ~DivSample();
};
