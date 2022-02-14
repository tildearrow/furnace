#include "taAudio.h"
#include <SDL.h>

class TAAudioSDL: public TAAudio {
  SDL_AudioSpec ac, ar;
  SDL_AudioDeviceID ai;
  bool audioSysStarted;

  public:
    void onProcess(unsigned char* buf, int nframes);

    void* getContext();
    bool quit();
    bool setRun(bool run);
    std::vector<String> listAudioDevices();
    bool init(TAAudioDesc& request, TAAudioDesc& response);
    TAAudioSDL():
      audioSysStarted(false) {}
};
