#include "ta-log.h"

int logLevel=LOGLEVEL_INFO;

int logD(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_DEBUG) return 0;
#ifdef _WIN32
  printf("[debug] ");
#else
  printf("\x1b[1;34m[debug]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  fflush(stdout);
  return ret;
}

int logI(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_INFO) return 0;
#ifdef _WIN32
  printf("[info] ");
#else
  printf("\x1b[1;32m[info]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

int logW(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_WARN) return 0;
#ifdef _WIN32
  printf("[warning] ");
#else
  printf("\x1b[1;33m[warning]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

int logE(const char* format, ...) {
  va_list va;
  int ret;
  if (logLevel<LOGLEVEL_ERROR) return 0;
#ifdef _WIN32
  printf("[ERROR] ");
#else
  printf("\x0b[1;31m[ERROR]\x1b[m ");
#endif
  va_start(va,format);
  ret=vprintf(format,va);
  va_end(va);
  return ret;
}

