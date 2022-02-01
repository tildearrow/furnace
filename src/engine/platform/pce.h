#ifndef _PCE_H
#define _PCE_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "sound/pce_psg.h"

class DivPlatformPCE: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch, note;
    int dacPeriod, dacRate;
    unsigned int dacPos;
    int dacSample;
    unsigned char ins, pan;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta, noise, pcm, furnaceDac;
    signed char vol, outVol, wave;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      note(0),
      dacPeriod(0),
      dacRate(0),
      dacPos(0),
      dacSample(-1),
      ins(-1),
      pan(255),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      noise(false),
      pcm(false),
      furnaceDac(false),
      vol(31),
      outVol(31),
      wave(-1) {}
  };
  Channel chan[6];
  bool isMuted[6];
  struct QueuedWrite {
      unsigned char addr;
      unsigned char val;
      QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned char lastPan;

  int cycles, curChan, delay;
  int tempL[32];
  int tempR[32];
  unsigned char sampleBank;
  PCE_PSG* pce;
  void updateWave(int ch);
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
    bool keyOffAffectsArp(int ch);
    void setFlags(unsigned int flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    int init(DivEngine* parent, int channels, int sugRate, unsigned int flags);
    void quit();
    ~DivPlatformPCE();
};

#endif
