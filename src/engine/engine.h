#include "song.h"
#include "dispatch.h"
#include "../audio/taAudio.h"

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
  int ticks, curRow, curOrder;
  std::vector<DivChannelState> chan;

  public:
    // load a .dmf.
    bool load(void* f, size_t length);
    // save as .dmf.
    bool save(FILE* f);

    // play
    void play();

    // initialize the engine.
    bool init();    
};
