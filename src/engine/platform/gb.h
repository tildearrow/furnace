#ifndef _GB_H
#define _GB_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/gb/gb.h"

class DivPlatformGB: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch;
    unsigned char ins, note, duty, sweep;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
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
      vol(15),
      wave(-1) {}
  };
  Channel chan[4];
  unsigned char lastPan;

  GB_gameboy_t* gb;
  void updateWave();
  public:
    void acquire(int& l, int& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};

#endif
