#ifndef _C64_H
#define _C64_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/c64/sid.h"

class DivPlatformC64: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, prevFreq;
    unsigned char ins, note, duty, sweep;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta, onTheKey;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      prevFreq(65535),
      ins(-1),
      note(0),
      duty(0),
      sweep(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      sweepChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      onTheKey(false),
      vol(15),
      wave(-1) {}
  };
  Channel chan[3];

  SID sid;

  void updateWave();
  public:
    void acquire(int& l, int& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
    void setChipModel(bool is6581);
};

#endif
