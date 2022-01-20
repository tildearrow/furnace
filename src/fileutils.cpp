#include "fileutils.h"
#ifdef _WIN32
#include "utfutils.h"
#endif

FILE* ps_fopen(const char* path, const char* mode) {
#ifdef _WIN32
  return _wfopen(utf8To16(path).c_str(),utf8To16(mode).c_str());
#else
  return fopen(path,mode);
#endif
}
