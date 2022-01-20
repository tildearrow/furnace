#ifndef _NES_H
#define _NES_H

#include "../dispatch.h"
#include "../macroInt.h"

class DivPlatformNES: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, prevFreq, note;
    unsigned char ins, duty, sweep;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta, furnaceDac;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      prevFreq(65535),
      note(0),
      ins(-1),
      duty(0),
      sweep(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      sweepChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      furnaceDac(false),
      vol(15),
      outVol(15),
      wave(-1) {}
  };
  Channel chan[5];
  bool isMuted[5];
  int dacPeriod, dacRate;
  unsigned int dacPos;
  int dacSample;
  unsigned char sampleBank;
  struct NESAPU* nes;

  float freqBase;

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void setPAL(bool pal);
    void notifyInsDeletion(void* ins);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
    void quit();
    ~DivPlatformNES();
};

#endif
