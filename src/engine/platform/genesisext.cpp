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
  printf("HANDLE: %d %d %d %d\n",c.cmd,c.chan,c.value,c.value2);
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
    default:
      break;
  }
  return 1;
}

void DivPlatformGenesisExt::tick() {
  DivPlatformGenesis::tick();
}
