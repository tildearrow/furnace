#include "../dispatch.h"

void DivDispatch::acquire(short** buf, size_t start, size_t len) {
}

void DivDispatch::tick() {
}

int DivDispatch::dispatch(DivCommand c) {
  return 1;
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate) {
  return 0;
}
