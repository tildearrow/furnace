#include "engine.h"

static DivPattern emptyPat;

DivPattern::DivPattern() {
  memset(data,-1,256*16*sizeof(short));
  for (int i=0; i<256; i++) {
    data[i][0]=0;
    data[i][1]=0;
  }
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

void DivChannelData::wipePatterns() {
  for (int i=0; i<128; i++) {
    if (data[i]!=NULL) {
      delete data[i];
      data[i]=NULL;
    }
  }
}

DivChannelData::DivChannelData():
  effectRows(1) {
  memset(data,0,128*sizeof(void*));
}
