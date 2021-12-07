#include "taAudio.h"
#include <sndfile.h>

class TAAudioFile: public TAAudio {
  float** iInBufs;
  float** iOutBufs;

  SNDFILE* file;
  SF_INFO si;

  public:
    void onProcess(unsigned char* buf, int nframes);

    void* getContext();
    bool quit();
    bool setRun(bool run);
    bool init(TAAudioDesc& request, TAAudioDesc& response);
};
