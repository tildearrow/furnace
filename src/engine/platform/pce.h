#ifndef _PCE_H
#define _PCE_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "sound/pce_psg.h"

class DivPlatformPCE: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch;
    unsigned char ins, note, duty, pan;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      ins(-1),
      note(0),
      duty(0),
      pan(255),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      vol(31),
      wave(-1) {}
  };
  Channel chan[6];
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char lastPan;

  int tempL, tempR, cycles, curChan, delay;
  PCE_PSG* pce;
  void updateWave(int ch);
  public:
    void acquire(int& l, int& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};

#endif
