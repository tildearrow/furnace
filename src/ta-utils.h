#ifndef _TA_UTILS_H
#define _TA_UTILS_H
#include <stdio.h>
#include <string.h>
#include <string>

typedef std::string String;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#endif
