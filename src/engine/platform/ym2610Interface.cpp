#include "sound/ym2610/ymfm.h"
#include "ym2610.h"
#include "../engine.h"

uint8_t DivYM2610Interface::ymfm_external_read(ymfm::access_class type, uint32_t address) {
  //printf("wants to read from %x\n",address);
  if (type!=ymfm::ACCESS_ADPCM_A) return 0;
  return parent->adpcmMem[address&0xffffff];
  /*if (12*sampleBank+(address>>16)>=parent->song.sampleLen) return 0;
  return parent->song.sample[12*sampleBank+(address>>16)]->adpcmRendData[(address&0xffff)];*/
}

void DivYM2610Interface::ymfm_external_write(ymfm::access_class type, uint32_t address, uint8_t data) {
}