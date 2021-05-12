#include "../dispatch.h"

// the dummy platform outputs square waves, interprets STD instruments and plays samples.
// used when a DivDispatch for a system is not found.
class DivPlatformDummy: public DivDispatch {
  public:
    void acquire(short& l, short& r);
    int dispatch(DivCommand c);
    int init(DivEngine* parent, int channels, int sugRate);
};