#ifndef _C64_H
#define _C64_H

#include "../dispatch.h"
#include "../macroInt.h"
#include "sound/c64/sid.h"

class DivPlatformC64: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, prevFreq, testWhen, note;
    unsigned char ins, sweep, wave, attack, decay, sustain, release;
    short duty;
    bool active, insChanged, freqChanged, sweepChanged, keyOn, keyOff, inPorta, filter;
    bool resetMask, resetFilter, resetDuty, ring, sync;
    signed char vol, outVol;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      prevFreq(65535),
      testWhen(0),
      note(0),
      ins(-1),
      sweep(0),
      wave(0),
      attack(0),
      decay(0),
      sustain(0),
      release(0),
      duty(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      sweepChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      filter(false),
      resetMask(false),
      resetFilter(false),
      resetDuty(false),
      ring(false),
      sync(false),
      vol(15) {}
  };
  Channel chan[3];
  bool isMuted[3];

  unsigned char filtControl, filtRes, vol;
  int filtCut, resetTime;

  SID sid;

  friend void putDispatchChan(void*,int,int);

  void updateFilter();
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    void setFlags(unsigned int flags);
    void notifyInsChange(int ins);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void setChipModel(bool is6581);
    void quit();
    ~DivPlatformC64();
};

#endif
