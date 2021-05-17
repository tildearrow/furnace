#include "../dispatch.h"

#include "genesis.h"

class DivPlatformGenesisExt: public DivPlatformGenesis {
  struct Channel {
    unsigned char freqH, freqL;
    int freq, baseFreq, pitch;
    unsigned char ins;
    signed char konCycles;
    bool active, insChanged, freqChanged, keyOn, keyOff;
    signed char vol;
    unsigned char pan;
    Channel(): freqH(0), freqL(0), freq(0), baseFreq(0), pitch(0), ins(-1), active(false), insChanged(true), freqChanged(false), keyOn(false), keyOff(false), vol(0), pan(3) {}
  };
  Channel opChan[4];
  public:
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};
