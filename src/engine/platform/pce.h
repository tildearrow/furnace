#ifndef _PCE_H
#define _PCE_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "sound/pce_psg.h"

class DivPlatformPCE: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch;
    int dacPeriod, dacRate, dacPos, dacSample;
    unsigned char ins, note, pan;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, noise, pcm;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      dacPeriod(0),
      dacRate(0),
      dacPos(0),
      dacSample(0),
      ins(-1),
      note(0),
      pan(255),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      noise(false),
      pcm(false),
      vol(31),
      outVol(31),
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
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void tick();
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
};

#endif
