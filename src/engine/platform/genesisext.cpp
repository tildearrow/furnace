#include "genesisext.h"
#include <math.h>

int DivPlatformGenesisExt::dispatch(DivCommand c) {
  if (c.chan<2) {
    return DivPlatformGenesis::dispatch(c);
  }
  if (c.chan>5) {
    c.chan-=3;
    return DivPlatformGenesis::dispatch(c);
  }
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].freq=16.4f*pow(2.0f,((float)c.value/12.0f));
      chan[c.chan].active=true;
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformGenesisExt::tick() {
  DivPlatformGenesis::tick();
}

int DivPlatformGenesisExt::init(DivEngine* parent, int channels, int sugRate) {
  DivPlatformGenesis::init(parent,channels,sugRate);

  extMode=true;
  return 13;
}
