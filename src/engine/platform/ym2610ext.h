#include "../dispatch.h"

#include "ym2610.h"

class DivPlatformYM2610Ext: public DivPlatformYM2610 {
  struct OpChannel {
    unsigned char freqH, freqL;
    int freq, baseFreq, pitch;
    unsigned char ins;
    signed char konCycles;
    bool active, insChanged, freqChanged, keyOn, keyOff, portaPause;
    int vol;
    unsigned char pan;
    OpChannel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), portaPause(false), vol(0), pan(3) {}
  };
  OpChannel opChan[4];
  public:
    int dispatch(DivCommand c);
    void reset();
    void tick();
    bool keyOffAffectsArp(int ch);
    int init(DivEngine* parent, int channels, int sugRate, bool pal);
    void quit();
    ~DivPlatformYM2610Ext();
};
