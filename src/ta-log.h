#ifndef _TA_LOG_H
#define _TA_LOG_H
#include <stdio.h>
#include <stdarg.h>

#define LOGLEVEL_ERROR 0
#define LOGLEVEL_WARN 1
#define LOGLEVEL_INFO 2
#define LOGLEVEL_DEBUG 3

#define logLevel 3

int logD(const char* format, ...);
int logI(const char* format, ...);
int logW(const char* format, ...);
int logE(const char* format, ...);
#endif
