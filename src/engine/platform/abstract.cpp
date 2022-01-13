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

bool DivDispatch::keyOffAffectsPorta(int ch) {
  return false;
}

int DivDispatch::getPortaFloor(int ch) {
  return 0x00;
}

void DivDispatch::setPAL(bool pal) {
}

void DivDispatch::setSkipRegisterWrites(bool value) {
  skipRegisterWrites=value;
}

void DivDispatch::notifyInsDeletion(void* ins) {

}

void DivDispatch::forceIns() {
  
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate, bool pal) {
  return 0;
}

void DivDispatch::quit() {
}

DivDispatch::~DivDispatch() {
}