#include "../dispatch.h"

// the dummy platform outputs square waves, interprets STD instruments and plays samples.
// used when a DivDispatch for a system is not found.
class DivPlatformDummy: public DivDispatch {
  struct Channel {
    unsigned short freq, baseFreq;
    short pitch;
    unsigned short pos;
    bool active, freqChanged;
    unsigned char vol;
    signed char amp;
    Channel(): freq(0), baseFreq(0), pitch(0), pos(0), active(false), freqChanged(false), vol(0), amp(64) {}
  };
  Channel chan[17];
  unsigned char chans;
  public:
    void acquire(short** buf, size_t start, size_t len);
    int dispatch(DivCommand c);
    void tick();
    int init(DivEngine* parent, int channels, int sugRate);
};
