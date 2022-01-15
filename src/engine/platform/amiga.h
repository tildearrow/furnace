#ifndef _PCE_H
#define _PCE_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"

class DivPlatformAmiga: public DivDispatch {
  struct Channel {
    int freq, baseFreq, pitch;
    unsigned int audLoc;
    unsigned short audLen;
    unsigned int audPos;
    signed char audDat;
    int sample;
    unsigned char ins, note;
    bool active, insChanged, freqChanged, keyOn, keyOff, inPorta;
    signed char vol, outVol;
    DivMacroInt std;
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      audLoc(0),
      audLen(0),
      audPos(0),
      audDat(0),
      sample(-1),
      ins(-1),
      note(0),
      active(false),
      insChanged(true),
      freqChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      vol(64),
      outVol(64) {}
  };
  Channel chan[4];
  bool isMuted[4];

  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void setPAL(bool pal);
    void notifyInsDeletion(void* ins);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
    void quit();
};

#endif
