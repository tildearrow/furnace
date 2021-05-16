#include "../dispatch.h"

#include "genesis.h"

class DivPlatformGenesisExt: public DivPlatformGenesis {
  public:
    int dispatch(DivCommand c);
    void tick();
};
