#include "../dispatch.h"

void DivDispatch::acquire(short* bufL, short* bufR, size_t start, size_t len) {
}

void DivDispatch::tick() {
}

int DivDispatch::dispatch(DivCommand c) {
  return 1;
}

bool DivDispatch::isStereo() {
  return false;
}

bool DivDispatch::keyOffAffectsArp(int ch) {
  return false;
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate) {
  return 0;
}
