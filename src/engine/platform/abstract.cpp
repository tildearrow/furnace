#include "../dispatch.h"

void DivDispatch::acquire(short* bufL, short* bufR, size_t start, size_t len) {
}

void DivDispatch::tick() {
}

void* DivDispatch::getState() {
  return NULL;
}

void DivDispatch::setState(void* state) {
}

void DivDispatch::muteChannel(int ch, bool mute) {
}

int DivDispatch::dispatch(DivCommand c) {
  return 1;
}

void DivDispatch::reset() {
}

bool DivDispatch::isStereo() {
  return false;
}

bool DivDispatch::keyOffAffectsArp(int ch) {
  return false;
}

void DivDispatch::setPAL(bool pal) {
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate, bool pal) {
  return 0;
}

void DivDispatch::quit() {
}

DivDispatch::~DivDispatch() {
}