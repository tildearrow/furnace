#include "sample.h"
#include "../ta-log.h"
#include <string.h>
#include <sndfile.h>

const int sampleRates[6]={
  4000, 8000, 11025, 16000, 22050, 32000
};

bool DivSample::save(const char* path) {
  SNDFILE* f;
  SF_INFO si;
  memset(&si,0,sizeof(SF_INFO));

  if (length<1) return false;

  si.channels=1;
  si.samplerate=sampleRates[rate];
  if (depth==16) {
    si.format=SF_FORMAT_PCM_16|SF_FORMAT_WAV;
  } else {
    si.format=SF_FORMAT_PCM_U8|SF_FORMAT_WAV;
  }

  f=sf_open(path,SFM_WRITE,&si);

  if (f==NULL) {
    logE("could not open wave file for saving! %s\n",sf_error_number(sf_error(f)));
    return false;
  }

  if (depth==16) {
    sf_writef_short(f,data,length);
  } else {
    short* cbuf=new short[length];
    for (int i=0; i<length; i++) {
      cbuf[i]=(data[i]<<8)^0x8000;
    }
    sf_writef_short(f,cbuf,length);
    delete[] cbuf;
  }
  sf_close(f);

  return true;
}

DivSample::~DivSample() {
  if (data) delete[] data;
  if (rendData) delete[] rendData;
  if (adpcmRendData) delete[] adpcmRendData;
}
