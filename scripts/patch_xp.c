// patch GetTickCount64 for running on Windows XP
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  unsigned char* buf;
  FILE* f=fopen(argv[1],"rb");
  if (f==NULL) {
    perror("open");
    return 1;
  }

  fseek(f,0,SEEK_END);

  size_t size=ftell(f);
  buf=malloc(size);

  fseek(f,0,SEEK_SET);

  fread(buf,1,size,f);
  fclose(f);

  // patch
  size_t remain=size;
  unsigned char* buf1=buf;
  while (size>=14) {
    unsigned char* where=memchr(buf1,'G',remain);
    if (where==NULL) {
      printf("GetTickCount64 not found\n");
      break;
    }

    if (memcmp(where,"GetTickCount64",14)==0) {
      printf("found GetTickCount64 - patching...\n");
      where[12]=0;
      where[13]=0;
      break;
    }

    buf1=where+1;
    remain=size-(buf1-buf);
  }

  // write
  FILE* of=fopen(argv[1],"wb");
  if (f==NULL) {
    perror("open write");
    return 1;
  }

  fwrite(buf,1,size,of);
  fclose(of);

  return 0;
}
