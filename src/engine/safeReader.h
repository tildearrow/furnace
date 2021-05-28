#ifndef _SAFEREADER_H
#define _SAFEREADER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../ta-utils.h"

class SafeReader;

struct EndOfFileException {
  SafeReader* reader;
  size_t finalSize;
  EndOfFileException(SafeReader* r, size_t fs):
    reader(r),
    finalSize(fs) {}
};

class SafeReader {
  unsigned char* buf;
  size_t len;

  size_t curSeek;

  public:
    bool seek(ssize_t where, int whence);
    size_t tell();
    size_t size();

    int read(void* where, size_t count);

    // these functions may throw EndOfFileException.
    signed char readC();
    short readS();
    short readS_BE();
    int readI();
    int readI_BE();
    int64_t readL();
    int64_t readL_BE();
    float readF();
    float readF_BE();
    double readD();
    double readD_BE();
    String readString();
    String readString(size_t len);

    SafeReader(void* b, size_t l):
      buf((unsigned char*)b),
      len(l),
      curSeek(0) {}
};

#endif
