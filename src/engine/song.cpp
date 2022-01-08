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

  for (int i=0; i<DIV_MAX_CHANS; i++) {
    pat[i].wipePatterns();
  }
}
