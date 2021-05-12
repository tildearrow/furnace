#ifndef _ENGINE_H
#define _ENGINE_H
#include "song.h"
#include "dispatch.h"
#include "../audio/taAudio.h"
#include "blip_buf.h"

struct DivChannelState {
  std::vector<DivDelayedCommand> delayed;
  int rampSpeed, portaSpeed, portaNote;
  int volSpeed;
  int vibratoDepth, vibratoRate;
  int tremoloDepth, tremoloRate;
};

class DivEngine {
  DivSong song;
  DivDispatch* dispatch;
  TAAudio* output;
  TAAudioDesc want, got;
  int chans;
  bool playing;
  bool speedAB;
  int ticks, cycles, curRow, curOrder;
  int changeOrd, changePos;
  std::vector<DivChannelState> chan;

  blip_buffer_t* bb[2];
  short temp[2], prevSample[2];
  short* bbOut[2];

  void nextOrder();
  void nextRow();
  void nextTick();

  public:
    void nextBuf(float** in, float** out, int inChans, int outChans, unsigned int size);
    // load a .dmf.
    bool load(void* f, size_t length);
    // save as .dmf.
    bool save(FILE* f);

    // play
    void play();

    // initialize the engine.
    bool init();

    DivEngine():
      chans(0),
      playing(false),
      speedAB(false),
      ticks(0),
      cycles(0),
      curRow(-1),
      curOrder(0),
      changeOrd(-1),
      changePos(0),
      temp{0,0},
      prevSample{0,0} {}
};
#endif
