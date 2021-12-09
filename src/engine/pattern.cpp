#include "engine.h"

static DivPattern emptyPat;

DivPattern::DivPattern() {
  memset(data,0,256*16);
}

DivPattern* DivChannelData::getPattern(int index, bool create) {
  if (data[index]==NULL) {
    if (create) {
      data[index]=new DivPattern;
    } else {
      return &emptyPat;
    }
  }
  return data[index];
}

DivChannelData::DivChannelData() {
  memset(data,0,128*sizeof(void*));
}
