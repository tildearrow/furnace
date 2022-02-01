#include "taAudio.h"
#include <SDL.h>

class TAAudioSDL: public TAAudio {
  SDL_AudioSpec ac, ar;
  SDL_AudioDeviceID ai;

  public:
    void onProcess(unsigned char* buf, int nframes);

    void* getContext();
    bool quit();
    bool setRun(bool run);
    bool init(TAAudioDesc& request, TAAudioDesc& response);
};
