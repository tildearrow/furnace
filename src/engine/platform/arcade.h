#ifndef _ARCADE_H
#define _ARCADE_H
#include "../dispatch.h"
#include "../instrument.h"
#include <queue>
#include "../../../extern/opm/opm.h"
#include "sound/ymfm/ymfm_opm.h"

class DivArcadeInterface: public ymfm::ymfm_interface {

};

class DivPlatformArcade: public DivDispatch {
  protected:
    struct Channel {
      DivInstrumentFM state;
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch;
      unsigned char ins;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, furnacePCM;
      int vol;
      unsigned char chVolL, chVolR;

      struct PCMChannel {
        int sample;
        unsigned int pos; // <<8
        unsigned short len;
        unsigned char freq;
        PCMChannel(): sample(-1), pos(0), len(0), freq(0) {}
      } pcm;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), furnacePCM(false), vol(0), chVolL(127), chVolR(127) {}
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
    unsigned char sampleBank;
    unsigned char lastBusy;
    unsigned char amDepth, pmDepth;

    ymfm::ym2151* fm_ymfm;
    ymfm::ym2151::output_data out_ymfm;
    DivArcadeInterface iface;

    bool extMode, useYMFM;

    bool isMuted[13];
  
    short oldWrites[256];
    short pendingWrites[256];

    int octave(int freq);
    int toFreq(int freq);

    void acquire_nuked(short* bufL, short* bufR, size_t start, size_t len);
    void acquire_ymfm(short* bufL, short* bufR, size_t start, size_t len);
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    void notifyInsChange(int ins);
    bool isStereo();
    void setYMFM(bool use);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
    void quit();
    ~DivPlatformArcade();
};
#endif
