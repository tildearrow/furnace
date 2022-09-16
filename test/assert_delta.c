#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#define BUF_SIZE 8192

// usage: assert_delta file
// return values:
// - 0: pass (file is silence)
// - 1: fail (noise found)
// - 2: command line error
// - 3: file open error
int main(int argc, char** argv) {
  if (argc<2) return 2;

  SF_INFO si;
  memset(&si,0,sizeof(SF_INFO));
  SNDFILE* sf=sf_open(argv[1],SFM_READ,&si);
  if (sf==NULL) {
    fprintf(stderr,"open: %s\n",sf_strerror(NULL));
    return 3;
  }

  if (si.channels<1) {
    fprintf(stderr,"invalid channel count\n");
    return 3;
  }

  float* buf=malloc(BUF_SIZE*si.channels*sizeof(float));

  sf_count_t totalRead=0;
  size_t seekPos=0;
  while ((totalRead=sf_readf_float(sf,buf,BUF_SIZE))!=0) {
    for (int i=0; i<totalRead*si.channels; i++) {
      if (buf[i]!=0.0f) {
        printf("%ld\n",seekPos+(i/si.channels));
        sf_close(sf);
        free(buf);
        return 1;
      }
    }
    seekPos+=BUF_SIZE;
  }

  sf_close(sf);
  free(buf);
  return 0;
}
