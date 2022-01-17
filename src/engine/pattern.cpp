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

SafeReader* DivPattern::compile(int len, int fxRows) {
  SafeWriter w;
  w.init();
  short lastNote, lastOctave, lastInstr, lastVolume, lastEffect[8], lastEffectVal[8];
  unsigned char rows=0;

  lastNote=0;
  lastOctave=0;
  lastInstr=-1;
  lastVolume=-1;
  memset(lastEffect,-1,8*sizeof(short));
  memset(lastEffectVal,-1,8*sizeof(short));

  for (int i=0; i<len; i++) {
    unsigned char mask=0;
    if (data[i][0]!=-1) {
      lastNote=data[i][0];
      lastOctave=data[i][1];
      mask|=128;
    }
    if (data[i][2]!=-1 && data[i][2]!=lastInstr) {
      lastInstr=data[i][2];
      mask|=32;
    }
    if (data[i][3]!=-1 && data[i][3]!=lastVolume) {
      lastVolume=data[i][3];
      mask|=64;
    }
    for (int j=0; j<fxRows; j++) {
      if (data[i][4+(j<<1)]!=-1) {
        lastEffect[j]=data[i][4+(j<<1)];
        lastEffectVal[j]=data[i][5+(j<<1)];
        mask=(mask&0xf8)|j;
      }
    }

    if (!mask) {
      rows++;
      continue;
    }

    if (rows!=0) {
      w.writeC(rows);
    }
    rows=1;

    w.writeC(mask);
    if (mask&128) {
      if (lastNote==100) {
        w.writeC(-128);
      } else {
        w.writeC(lastNote+(lastOctave*12));
      }
    }
    if (mask&64) {
      w.writeC(lastVolume);
    }
    if (mask&32) {
      w.writeC(lastInstr);
    }
    for (int j=0; j<(mask&7); j++) {
      w.writeC(lastEffect[j]);
      if (lastEffectVal[j]==-1) {
        w.writeC(0);
      } else {
        w.writeC(lastEffectVal[j]);
      }
    }
  }
  w.writeC(rows);
  w.writeC(0);

  return w.toReader();
}

DivChannelData::DivChannelData():
  effectRows(1) {
  memset(data,0,128*sizeof(void*));
}
