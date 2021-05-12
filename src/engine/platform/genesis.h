#include "../dispatch.h"
#include <queue>
#include "../../../extern/Nuked-OPN2/ym3438.h"

class DivPlatformGenesis: public DivDispatch {
  struct Channel {
    unsigned short freq;
    unsigned char ins;
    bool active;
    signed char vol;
    Channel(): freq(0), ins(0), active(false), vol(0) {}
  };
  Channel chan[10];
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  std::queue<QueuedWrite> writes;
  ym3438_t fm;
  int psg;
  int delay;
  public:
    void acquire(short& l, short& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};
