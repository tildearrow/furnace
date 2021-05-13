#include "../dispatch.h"
#include <queue>
#include "../../../extern/Nuked-OPN2/ym3438.h"

class DivPlatformGenesis: public DivDispatch {
  struct Channel {
    unsigned short freq;
    unsigned char ins;
    signed char konCycles;
    bool active, insChanged;
    signed char vol;
    unsigned char pan;
    Channel(): freq(0), ins(0), active(false), insChanged(true), vol(0), pan(3) {}
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

  public:
    void acquire(short& l, short& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};
