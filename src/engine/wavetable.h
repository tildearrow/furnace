#ifndef _WAVETABLE_H
#define _WAVETABLE_H
#include "safeWriter.h"

struct DivWavetable {
  int len, min, max;
  int data[256];

  void putWaveData(SafeWriter* w);
  void readWaveData(SafeReader& reader, short version);
  bool save(const char* path);
  DivWavetable():
    len(32),
    min(0),
    max(31) {
    for (int i=0; i<256; i++) {
      data[i]=i;
    }
  }
};

#endif