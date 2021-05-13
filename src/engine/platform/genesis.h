#include "../dispatch.h"
#include <queue>
#include "../../../extern/Nuked-OPN2/ym3438.h"

class DivPlatformGenesis: public DivDispatch {
  struct Channel {
    unsigned char freqH, freqL;
    unsigned char ins;
    signed char konCycles;
    bool active, insChanged, keyOn;
    signed char vol;
    unsigned char pan;
    Channel(): freqH(0), freqL(0), ins(0), active(false), insChanged(true), keyOn(false), vol(0), pan(3) {}
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
  int delay;
  unsigned char lastBusy;

  bool dacMode;
  int dacPeriod;
  int dacRate;
  int dacPos;
  int dacSample;

  short pendingWrites[512];

  public:
    void acquire(short& l, short& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};
