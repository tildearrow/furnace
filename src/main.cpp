#include <stdio.h>
#include "ta-log.h"
#include "engine/engine.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define DIV_VERSION "dev6"

DivEngine e;

int main(int argc, char** argv) {
#ifdef _WIN32
  HANDLE winin=GetStdHandle(STD_INPUT_HANDLE);
  HANDLE winout=GetStdHandle(STD_OUTPUT_HANDLE);
  int termprop=0;
  int termpropi=0;
  GetConsoleMode(winout,(LPDWORD)&termprop);
  GetConsoleMode(winin,(LPDWORD)&termpropi);
  termprop|=ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  termpropi&=~ENABLE_LINE_INPUT;
  SetConsoleMode(winout,termprop);
  SetConsoleMode(winin,termpropi);
#endif
  if (argc<2) {
    logI("usage: %s file\n",argv[0]);
    return 1;
  }
  logI("Furnace version " DIV_VERSION ".\n");
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
  if (len==0x7fffffffffffffff) {
    perror("could not get file length");
    fclose(f);
    return 1;
  }
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
  logI("playing...\n");
  e.play();
  while (true) {
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500000);
#endif
  }
  return 0;
}
