#ifndef _GB_H
#define _GB_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/gb/gb.h"

class DivPlatformGB: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, note;
    unsigned char ins, duty, sweep;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
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
      vol(15),
      outVol(15),
      wave(-1) {}
  };
  Channel chan[4];
  bool isMuted[4];
  unsigned char lastPan;

  GB_gameboy_t* gb;
  unsigned char procMute();
  void updateWave();
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool isStereo();
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformGB();
};

#endif
