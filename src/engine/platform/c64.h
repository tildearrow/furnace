#ifndef _C64_H
#define _C64_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/c64/sid.h"

class DivPlatformC64: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, prevFreq, testWhen;
    unsigned char ins, note, sweep, wave;
    short duty;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta;
    signed char vol, outVol;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      prevFreq(65535),
      testWhen(0),
      ins(-1),
      note(0),
      sweep(0),
      wave(0),
      duty(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      sweepChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      vol(15) {}
  };
  Channel chan[3];

  SID sid;

  void updateWave();
  public:
    void acquire(short** buf, size_t start, size_t len);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
    void setChipModel(bool is6581);
};

#endif
