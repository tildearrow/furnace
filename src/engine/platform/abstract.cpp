#include "../dispatch.h"

void DivDispatch::acquire(short* bufL, short* bufR, size_t start, size_t len) {
}

void DivDispatch::tick() {
}

void* DivDispatch::getChanState(int chan) {
  return NULL;
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

const char* DivDispatch::getEffectName(unsigned char effect) {
  return NULL;
}

void DivDispatch::setFlags(unsigned int flags) {
}

void DivDispatch::setSkipRegisterWrites(bool value) {
  skipRegisterWrites=value;
}

void DivDispatch::notifyInsChange(int ins) {

}

void DivDispatch::notifyWaveChange(int ins) {

}

void DivDispatch::notifyInsDeletion(void* ins) {

}

void DivDispatch::forceIns() {
  
}

void DivDispatch::toggleRegisterDump(bool enable) {
  dumpWrites=enable;
}

std::vector<DivRegWrite>& DivDispatch::getRegisterWrites() {
  return regWrites;
}

void DivDispatch::poke(unsigned int addr, unsigned short val) {

}

void DivDispatch::poke(std::vector<DivRegWrite>& wlist) {
  
}

int DivDispatch::init(DivEngine* p, int channels, int sugRate, unsigned int flags) {
  return 0;
}

void DivDispatch::quit() {
}

DivDispatch::~DivDispatch() {
}
