#include "../dispatch.h"

void DivDispatch::acquire(short& l, short& r) {
  l=0;
  r=0;
}

void DivDispatch::tick() {
}

int DivDispatch::dispatch(DivCommand c) {
  return 1;
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate) {
  return 0;
}
