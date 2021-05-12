#include <stdio.h>
#include <mutex>
#include "ta-log.h"
#include "engine/engine.h"

#define DIV_VERSION "dev1"

DivEngine e;

std::mutex m;

int main(int argc, char** argv) {
  if (argc<2) {
    logI("usage: %s file\n",argv[0]);
    return 1;
  }
  logI("divorce " DIV_VERSION "\n");
  logI("loading module...\n");
  FILE* f=fopen(argv[1],"rb");
  if (f==NULL) {
    perror("error");
    return 1;
  }
  if (fseek(f,0,SEEK_END)<0) {
    perror("size error");
    fclose(f);
    return 1;
  }
  ssize_t len=ftell(f);
  if (len<1) {
    if (len==0) {
      printf("that file is empty!\n");
    } else {
      perror("tell error");
    }
    fclose(f);
    return 1;
  }
  unsigned char* file=new unsigned char[len];
  if (fseek(f,0,SEEK_SET)<0) {
    perror("size error");
    fclose(f);
    return 1;
  }
  if (fread(file,1,(size_t)len,f)!=(size_t)len) {
    perror("read error");
    fclose(f);
    return 1;
  }
  fclose(f);
  if (!e.load((void*)file,(size_t)len)) {
    logE("could not open file!\n");
    return 1;
  }
  if (!e.init()) {
    logE("could not initialize engine!\n");
    return 1;
  }
  logI("loaded! :o\n");
  while (true) {
    logI("locking...\n");
    e.play();
    m.lock();
    m.lock();
  }
  return 0;
}
