#include "dummy.h"

void DivPlatformDummy::acquire(short& l, short& r) {
  l=0;
  r=0;
}

int DivPlatformDummy::dispatch(DivCommand c) {
  return 1;
}

int DivPlatformDummy::init(DivEngine* p, int channels, int sugRate) {
  parent=p;
  rate=sugRate;
  return channels;
}