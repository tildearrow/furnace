#include "../dispatch.h"

// the dummy platform outputs square waves, interprets STD instruments and plays samples.
// used when a DivDispatch for a system is not found.
class DivPlatformDummy: public DivDispatch {
  public:
    void acquire(float& l, float& r);
    int dispatch(DivCommand c);
    int init(int channels);
};