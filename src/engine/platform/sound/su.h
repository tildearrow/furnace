/* su.cpp/su.h - Sound Unit emulator
 * Copyright (C) 2015-2023 tildearrow
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
  short fns[8];
  short nsL[8];
  short nsR[8];
  short nslow[8];
  short nshigh[8];
  short nsband[8];
  int tnsL, tnsR;
  unsigned char ilBufPeriod;
  unsigned short ilBufPos;
  signed char ilFeedback0;
  signed char ilFeedback1;
  unsigned short oldfreq[8];
  unsigned int pcmSize;
  bool dsOut;
  unsigned char dsChannel;
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
      unsigned char flags0;
      unsigned char flags1;
      unsigned short cutoff;
      unsigned char duty;
      unsigned char reson;
      unsigned short pcmpos;
      unsigned short pcmbnd;
      unsigned short pcmrst;
      struct {
        unsigned short speed;
        unsigned char amt;
        unsigned char bound;
      } swfreq;
      struct {
        unsigned short speed;
        unsigned char amt;
        unsigned char bound;
      } swvol;
      struct {
        unsigned short speed;
        unsigned char amt;
        unsigned char bound;
      } swcut;
      unsigned char special1C;
      unsigned char special1D;
      unsigned short restimer;
    } chan[8];
    signed char pcm[65536];
    bool muted[8];
    void SetIL0(unsigned char addr);
    void Write(unsigned char addr, unsigned char data);
    void NextSample(short* l, short* r);
    inline int GetSample(int ch) {
      int ret=(nsL[ch]+nsR[ch])>>1;
      if (ret<-32768) ret=-32768;
      if (ret>32767) ret=32767;
      return ret;
    }
    void Init(int sampleMemSize=8192, bool dsOutMode=false);
    void Reset();
    SoundUnit();
};
