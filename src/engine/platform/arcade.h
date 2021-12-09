#ifndef _ARCADE_H
#define _ARCADE_H
#include "../dispatch.h"
#include <queue>
#include "../../../extern/opm/opm.h"

class DivPlatformArcade: public DivDispatch {
  protected:
    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch;
      unsigned char ins;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause;
      int vol;
      unsigned char chVolL, chVolR;

      struct PCMChannel {
        int sample;
        unsigned int pos; // <<8
        unsigned short len;
        unsigned char freq;
        PCMChannel(): sample(-1), pos(0), len(0), freq(0) {}
      } pcm;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), vol(0), chVolL(127), chVolR(127) {}
    };
    Channel chan[13];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    opm_t fm;
    int delay;
    int pcmL, pcmR, pcmCycles;
    unsigned char lastBusy;

    bool extMode;
  
    short oldWrites[256];
    short pendingWrites[256];

    int octave(int freq);
    int toFreq(int freq);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void tick();
    bool isStereo();
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
};
#endif
