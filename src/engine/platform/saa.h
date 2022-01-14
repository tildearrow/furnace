#ifndef _SAA_H
#define _SAA_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/saa1099.h"

class DivPlatformSAA1099: public DivDispatch {
  protected:
    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch;
      unsigned char ins, note, psgMode;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta;
      int vol, outVol;
      unsigned char pan;
      DivMacroInt std;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), note(0), psgMode(1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), inPorta(false), vol(0), outVol(15), pan(3) {}
    };
    Channel chan[3];
    bool isMuted[3];
    struct QueuedWrite {
      unsigned short addr;
      unsigned char val;
      bool addrOrVal;
      QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
    };
    std::queue<QueuedWrite> writes;
    saa1099_device saa;
    unsigned char lastBusy;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    int dacPos;
    int dacSample;
    unsigned char sampleBank;

    int delay;

    bool extMode;
  
    short oldWrites[16];
    short pendingWrites[16];
    unsigned char ayEnvMode;
    unsigned short ayEnvPeriod;
    short ayEnvSlideLow;
    short ayEnvSlide;
    short* ayBuf[3];
    size_t ayBufLen;
  
  public:
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void reset();
    void forceIns();
    void tick();
    void muteChannel(int ch, bool mute);
    void setPAL(bool pal);
    bool isStereo();
    bool keyOffAffectsArp(int ch);
    void notifyInsDeletion(void* ins);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
    void quit();
};
#endif
