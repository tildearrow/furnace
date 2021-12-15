#include "song.h"

void DivSong::unload() {
  for (DivInstrument* i: ins) {
    delete i;
  }
  ins.clear();

  for (DivWavetable* i: wave) {
    delete i;
  }
  wave.clear();

  for (DivSample* i: sample) {
    delete i;
  }
  sample.clear();  
}
