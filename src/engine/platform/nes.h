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
      sweep(8),
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
  unsigned char apuType;
  struct NESAPU* nes;

  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void setFlags(unsigned int flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformNES();
};

#endif
