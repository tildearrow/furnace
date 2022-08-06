#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

class SoundUnit {
  signed char SCsine[256];
  signed char SCtriangle[256];
  signed char SCpantabL[256];
  signed char SCpantabR[256];
  unsigned int ocycle[8];
  unsigned int cycle[8];
  int rcycle[8];
  unsigned int lfsr[8];
  signed char ns[8];
  int fns[8];
  int nsL[8];
  int nsR[8];
  int nslow[8];
  int nshigh[8];
  int nsband[8];
  int tnsL, tnsR;
  unsigned short oldfreq[8];
  unsigned short oldflags[8];
  unsigned int pcmSize;
  public:
    unsigned short resetfreq[8];
    unsigned short voldcycles[8];
    unsigned short volicycles[8];
    unsigned short fscycles[8];
    unsigned char sweep[8];
    unsigned short swvolt[8];
    unsigned short swfreqt[8];
    unsigned short swcutt[8];
    unsigned short pcmdec[8];
    struct SUChannel {
      unsigned short freq;
      signed char vol;
      signed char pan;
      union {
        unsigned short flags;
        struct {
          unsigned char shape: 3;
          unsigned char pcm: 1;
          unsigned char ring: 1;
          unsigned char fmode: 3;
          unsigned char resosc: 1;
          unsigned char resfilt: 1;
          unsigned char pcmloop: 1;
          unsigned char restim: 1;
          unsigned char swfreq: 1;
          unsigned char swvol: 1;
          unsigned char swcut: 1;
          unsigned char padding: 1;
        };
      } flags;
      unsigned short cutoff;
      unsigned char duty;
      unsigned char reson;
      unsigned short pcmpos;
      unsigned short pcmbnd;
      unsigned short pcmrst;
      struct {
        unsigned short speed;
        unsigned char amt: 7;
        unsigned char dir: 1;
        unsigned char bound;
      } swfreq;
      struct {
        unsigned short speed;
        unsigned char amt: 5;
        unsigned char dir: 1;
        unsigned char loop: 1;
        unsigned char loopi: 1;
        unsigned char bound;
      } swvol;
      struct {
        unsigned short speed;
        unsigned char amt: 7;
        unsigned char dir: 1;
        unsigned char bound;
      } swcut;
      unsigned short wc;
      unsigned short restimer;
    } chan[8];
    signed char pcm[65536];
    bool muted[8];
    void Write(unsigned char addr, unsigned char data);
    void NextSample(short* l, short* r);
    inline int GetSample(int ch) {
      int ret=(nsL[ch]+nsR[ch])>>1;
      if (ret<-32768) ret=-32768;
      if (ret>32767) ret=32767;
      return ret;
    }
    void Init(int sampleMemSize=8192);
    void Reset();
    SoundUnit();
};
