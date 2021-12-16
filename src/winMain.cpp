#include "utfutils.h"

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, PSTR args, int state) {
  int argc=0;
  wchar_t** argw=CommandLineToArgvW(GetCommandLineW(),&argc);
  char** argv=new char*[argc+1];
  argv[argc]=NULL;
  for (int i=0; i<argc; i++) {
    std::string str=utf16To8(argw[i]);
    argv[i]=new char[str.size()+1];
    strcpy(argv[i],str.c_str());
  }
  return main(argc,argv);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prevInst, PWSTR args, int state) {
  int argc=0;
  wchar_t** argw=CommandLineToArgvW(args,&argc);
  char** argv=new char*[argc+1];
  argv[argc]=NULL;
  for (int i=0; i<argc; i++) {
    std::string str=utf16To8(argw[i]);
    argv[i]=new char[str.size()+1];
    strcpy(argv[i],str.c_str());
  }
  return main(argc,argv);
}
