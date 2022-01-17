#ifndef _SAFEWRITER_H
#define _SAFEWRITER_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "safeReader.h"
#include "../ta-utils.h"

class SafeWriter {
  bool operative;
  unsigned char* buf;
  size_t bufLen;
  size_t len;

  size_t curSeek;

  void checkSize(size_t amount);

  public:
    unsigned char* getFinalBuf();

    bool seek(ssize_t where, int whence);
    size_t tell();
    size_t size();

    int write(const void* what, size_t count);

    int writeC(signed char val);
    int writeS(short val);
    int writeS_BE(short val);
    int writeI(int val);
    int writeI_BE(int val);
    int writeL(int64_t val);
    int writeL_BE(int64_t val);
    int writeF(float val);
    int writeF_BE(float val);
    int writeD(double val);
    int writeD_BE(double val);
    int writeString(String val, bool pascal);

    void init();
    SafeReader* toReader();
    void finish();

    SafeWriter():
      operative(false),
      buf(NULL),
      bufLen(0),
      len(0),
      curSeek(0) {}
};

#endif
