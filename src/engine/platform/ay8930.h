#ifndef _AY8930_H
#define _AY8930_H
#include "../dispatch.h"
#include "../macroInt.h"
#include <queue>
#include "sound/ay8910.h"

class DivPlatformAY8930: public DivDispatch {
  protected:
    struct Channel {
      unsigned char freqH, freqL;
      int freq, baseFreq, pitch;
      unsigned char ins, note, psgMode, duty;
      signed char konCycles;
      bool active, insChanged, freqChanged, keyOn, keyOff, portaPause, inPorta;
      int vol, outVol;
      unsigned char pan;
      DivMacroInt std;
      Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), note(0), psgMode(1), duty(4), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), inPorta(false), vol(0), outVol(31), pan(3) {}
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
    ay8930_device* ay;
    unsigned char lastBusy;
  
    bool dacMode;
    int dacPeriod;
    int dacRate;
    int dacPos;
    int dacSample;
    unsigned char sampleBank;
    bool bank;

    int delay;

    bool extMode;
  
    short oldWrites[32];
    short pendingWrites[32];
    unsigned char ayEnvMode[3];
    unsigned short ayEnvPeriod[3];
    short ayEnvSlideLow[3];
    short ayEnvSlide[3];
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
