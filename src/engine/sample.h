#include "../ta-utils.h"

struct DivSample {
  String name;
  int length, rate, loopStart, loopOffP;
  signed char vol, pitch;
  unsigned char depth;
  short* data;
  unsigned int rendLength, adpcmRendLength, rendOff, rendOffP;
  short* rendData;
  unsigned char* adpcmRendData;

  bool save(const char* path);
  DivSample():
    name(""),
    length(0),
    rate(32000),
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
    rendData(NULL),
    adpcmRendData(NULL) {}
  ~DivSample();
};
