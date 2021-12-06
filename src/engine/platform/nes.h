#ifndef _NES_H
#define _NES_H

#include "../dispatch.h"
#include "../macroInt.h"

class DivPlatformNES: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, prevFreq;
    unsigned char ins, note, duty, sweep;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta;
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
      vol(15),
      wave(-1) {}
  };
  Channel chan[5];
  int dacPeriod, dacRate, dacPos, dacSample;
  unsigned char lastPan;

  void updateWave();
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};

#endif
