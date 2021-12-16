#ifndef _UTFUTILS_H
#define _UTFUTILS_H
#include "ta-utils.h"

size_t utf8len(const char* s);
size_t utf8clen(const char* s);
size_t utf8pos(const char* s, size_t inpos);
size_t utf8cpos(const char* s, size_t inpos);
size_t utf8findcpos(const char* s, float inpos);
char utf8csize(const unsigned char* c);

WString utf8To16(const char* in);
String utf16To8(const wchar_t* in);

#endif
