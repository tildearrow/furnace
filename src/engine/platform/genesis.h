#ifndef _GENESIS_H
#define _GENESIS_H
#include "../dispatch.h"
#include <queue>
#include "../../../extern/Nuked-OPN2/ym3438.h"

#include "sms.h"

class DivPlatformGenesis: public DivDispatch {
  protected:
    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch;
      unsigned char ins;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause;
      int vol;
      unsigned char pan;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), vol(0), pan(3) {}
    };
    Channel chan[10];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    ym3438_t fm;
    DivPlatformSMS psg;
    int psgClocks;
    int psgOut;
    int delay;
    unsigned char lastBusy;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    int dacPos;
    int dacSample;
    int sampleBank;

    bool extMode;
  
    short oldWrites[512];
    short pendingWrites[512];

    int octave(int freq);
    int toFreq(int freq);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void reset();
    void tick();
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
};
#endif
