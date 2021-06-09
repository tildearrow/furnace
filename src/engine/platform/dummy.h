#include "../dispatch.h"

// the dummy platform outputs square waves, interprets STD instruments and plays samples.
// used when a DivDispatch for a system is not found.
class DivPlatformDummy: public DivDispatch {
  struct Channel {
    unsigned short freq;
    unsigned short pos;
    bool active;
    unsigned char vol;
    signed char amp;
    Channel(): freq(0), pos(0), active(false), vol(0), amp(64) {}
  };
  Channel chan[17];
  unsigned char chans;
  public:
    void acquire(int& l, int& r);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};
