#include "taAudio.h"
#include <jack/weakjack.h>
#include <jack/jack.h>

class TAAudioJACK: public TAAudio {
  jack_client_t* ac;
  jack_port_t** ai;
  jack_port_t** ao;

  float** iInBufs;
  float** iOutBufs;

  public:
    void onSampleRate(jack_nframes_t rate);
    void onBufferSize(jack_nframes_t bufsize);
    void onProcess(jack_nframes_t nframes);

    void* getContext();
    bool quit();
    bool setRun(bool run);
    bool init(TAAudioDesc& request, TAAudioDesc& response);

    TAAudioJACK():
      ac(NULL),
      ai(NULL),
      ao(NULL) {}
};
