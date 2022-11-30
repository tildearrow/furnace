#define _USE_MATH_DEFINES
#include "su.h"
#include <string.h>

#define minval(a,b) (((a)<(b))?(a):(b))
#define maxval(a,b) (((a)>(b))?(a):(b))

#define FILVOL chan[4].special1C
#define ILCTRL chan[4].special1D
#define ILSIZE chan[5].special1C
#define FIL1 chan[5].special1D
#define IL1 chan[6].special1C
#define IL2 chan[6].special1D
#define IL0 chan[7].special1C
#define MVOL chan[7].special1D

void SoundUnit::NextSample(short* l, short* r) {
  // run channels
  for (int i=0; i<8; i++) {
    if (chan[i].vol==0 && !chan[i].flags.swvol) {
      fns[i]=0;
      continue;
    }
    if (chan[i].flags.pcm) {
      ns[i]=pcm[chan[i].pcmpos];
    } else switch (chan[i].flags.shape) {
      case 0:
        ns[i]=(((cycle[i]>>15)&127)>chan[i].duty)*127;
        break;
      case 1:
        ns[i]=cycle[i]>>14;
        break;
      case 2:
        ns[i]=SCsine[(cycle[i]>>14)&255];
        break;
      case 3:
        ns[i]=SCtriangle[(cycle[i]>>14)&255];
        break;
      case 4: case 5:
        ns[i]=(lfsr[i]&1)*127;
        break;
      case 6:
        ns[i]=((((cycle[i]>>15)&127)>chan[i].duty)*127)^(short)SCsine[(cycle[i]>>14)&255];
        break;
      case 7:
        ns[i]=((((cycle[i]>>15)&127)>chan[i].duty)*127)^(short)SCtriangle[(cycle[i]>>14)&255];
        break;
    }

    if (chan[i].flags.ring) {
      ns[i]=(ns[i]*ns[(i+1)&7])>>7;
    }
    
    if (chan[i].flags.pcm) {
      if (chan[i].freq>0x8000) {
        pcmdec[i]+=0x8000;
      } else {
        pcmdec[i]+=chan[i].freq;
      }
      if (pcmdec[i]>=32768) {
        pcmdec[i]-=32768;
        if (chan[i].pcmpos<chan[i].pcmbnd) {
          chan[i].pcmpos++;
          if (chan[i].pcmpos==chan[i].pcmbnd) {
            if (chan[i].flags.pcmloop) {
              chan[i].pcmpos=chan[i].pcmrst;
            }
          }
          chan[i].pcmpos&=(pcmSize-1);
        } else if (chan[i].flags.pcmloop) {
          chan[i].pcmpos=chan[i].pcmrst;
        }
      }
    } else {
      ocycle[i]=cycle[i];
      if (chan[i].flags.shape==5) {
        switch ((chan[i].duty>>4)&3) {
          case 0:
            cycle[i]+=chan[i].freq*1-(chan[i].freq>>3);
            break;
          case 1:
            cycle[i]+=chan[i].freq*2-(chan[i].freq>>3);
            break;
          case 2:
            cycle[i]+=chan[i].freq*4-(chan[i].freq>>3);
            break;
          case 3:
            cycle[i]+=chan[i].freq*8-(chan[i].freq>>3);
            break;
        }
      } else {
        cycle[i]+=chan[i].freq;
      }
      if ((cycle[i]&0xf80000)!=(ocycle[i]&0xf80000)) {
        if (chan[i].flags.shape==4) {
          lfsr[i]=(lfsr[i]>>1|(((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ^ (lfsr[i] >> 5) ) & 1)<<31);
        } else {
          switch ((chan[i].duty>>4)&3) {
            case 0:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i] >> 3) ^ (lfsr[i] >> 4) ) & 1)<<5);
              break;
            case 1:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i] >> 2) ^ (lfsr[i] >> 3) ) & 1)<<5);
              break;
            case 2:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ) & 1)<<5);
              break;
            case 3:
              lfsr[i]=(lfsr[i]>>1|(((lfsr[i]) ^ (lfsr[i] >> 2) ^ (lfsr[i] >> 3) ^ (lfsr[i] >> 5) ) & 1)<<5);
              break;
          }
          if ((lfsr[i]&63)==0) {
            lfsr[i]=0xaaaa;
          }
        }
      }
      if (chan[i].flags.restim) {
        if (--rcycle[i]<=0) {
          cycle[i]=0;
          rcycle[i]=chan[i].restimer;
          lfsr[i]=0xaaaa;
        }
      }
    }
    fns[i]=ns[i]*chan[i].vol*(chan[i].flags.pcm?4:2);
    if (chan[i].flags.fmode!=0) {
      int ff=chan[i].cutoff;
      nslow[i]=nslow[i]+(((ff)*nsband[i])>>16);
      nshigh[i]=fns[i]-nslow[i]-(((256-chan[i].reson)*nsband[i])>>8);
      nsband[i]=(((ff)*nshigh[i])>>16)+nsband[i];
      fns[i]=(((chan[i].flags.fmode&1)?(nslow[i]):(0))+((chan[i].flags.fmode&2)?(nshigh[i]):(0))+((chan[i].flags.fmode&4)?(nsband[i]):(0)));
    }
    nsL[i]=(fns[i]*SCpantabL[(unsigned char)chan[i].pan])>>8;
    nsR[i]=(fns[i]*SCpantabR[(unsigned char)chan[i].pan])>>8;
    oldfreq[i]=chan[i].freq;
    oldflags[i]=chan[i].flags.flags;
    if (chan[i].flags.swvol) {
      if (--swvolt[i]<=0) {
        swvolt[i]=chan[i].swvol.speed;
        if (chan[i].swvol.dir) {
          chan[i].vol+=chan[i].swvol.amt;
          if (chan[i].vol>chan[i].swvol.bound && !chan[i].swvol.loop) {
            chan[i].vol=chan[i].swvol.bound;
          }
          if (chan[i].vol&0x80) {
            if (chan[i].swvol.loop) {
              if (chan[i].swvol.loopi) {
                chan[i].swvol.dir=!chan[i].swvol.dir;
                chan[i].vol=0xff-chan[i].vol;
              } else {
                chan[i].vol&=~0x80;
              }
            } else {
              chan[i].vol=0x7f;
            }
          }
        } else {
          chan[i].vol-=chan[i].swvol.amt;
          if (chan[i].vol&0x80) {
            if (chan[i].swvol.loop) {
              if (chan[i].swvol.loopi) {
                chan[i].swvol.dir=!chan[i].swvol.dir;
                chan[i].vol=-chan[i].vol;
              } else {
                chan[i].vol&=~0x80;
              }
            } else {
              chan[i].vol=0x0;
            }
          }
          if (chan[i].vol<chan[i].swvol.bound && !chan[i].swvol.loop) {
            chan[i].vol=chan[i].swvol.bound;
          }
        }
      }
    }
    if (chan[i].flags.swfreq) {
      if (--swfreqt[i]<=0) {
        swfreqt[i]=chan[i].swfreq.speed;
        if (chan[i].swfreq.dir) {
          if (chan[i].freq>(0xffff-chan[i].swfreq.amt)) {
            chan[i].freq=0xffff;
          } else {
            chan[i].freq=(chan[i].freq*(0x80+chan[i].swfreq.amt))>>7;
            if ((chan[i].freq>>8)>chan[i].swfreq.bound) {
              chan[i].freq=chan[i].swfreq.bound<<8;
            }
          }
        } else {
          if (chan[i].freq<chan[i].swfreq.amt) {
            chan[i].freq=0;
          } else {
            chan[i].freq=(chan[i].freq*(0xff-chan[i].swfreq.amt))>>8;
            if ((chan[i].freq>>8)<chan[i].swfreq.bound) {
              chan[i].freq=chan[i].swfreq.bound<<8;
            }
          }
        }
      }
    }
    if (chan[i].flags.swcut) {
      if (--swcutt[i]<=0) {
        swcutt[i]=chan[i].swcut.speed;
        if (chan[i].swcut.dir) {
          if (chan[i].cutoff>(0xffff-chan[i].swcut.amt)) {
            chan[i].cutoff=0xffff;
          } else {
            chan[i].cutoff+=chan[i].swcut.amt;
            if ((chan[i].cutoff>>8)>chan[i].swcut.bound) {
              chan[i].cutoff=chan[i].swcut.bound<<8;
            }
          }
        } else {
          if (chan[i].cutoff<chan[i].swcut.amt) {
            chan[i].cutoff=0;
          } else {
            chan[i].cutoff=((2048-(unsigned int)chan[i].swcut.amt)*(unsigned int)chan[i].cutoff)>>11;
            if ((chan[i].cutoff>>8)<chan[i].swcut.bound) {
              chan[i].cutoff=chan[i].swcut.bound<<8;
            }
          }
        }
      }
    }
    if (chan[i].flags.resosc) {
      cycle[i]=0;
      rcycle[i]=chan[i].restimer;
      ocycle[i]=0;
      chan[i].flags.resosc=0;
    }
    if (muted[i]) {
      nsL[i]=0;
      nsR[i]=0;
    }
  }

  // mix
  tnsL=(nsL[0]+nsL[1]+nsL[2]+nsL[3]+nsL[4]+nsL[5]+nsL[6]+nsL[7])>>2;
  tnsR=(nsR[0]+nsR[1]+nsR[2]+nsR[3]+nsR[4]+nsR[5]+nsR[6]+nsR[7])>>2;

  IL1=minval(32767,maxval(-32767,tnsL))>>8;
  IL2=minval(32767,maxval(-32767,tnsR))>>8;

  // write input lines to sample memory
  if (ILSIZE&64) {
    if (++ilBufPeriod>=((1+(FIL1>>4))<<2)) {
      ilBufPeriod=0;
      unsigned short ilLowerBound=pcmSize-((1+(ILSIZE&63))<<7);
      short next;
      if (ilBufPos<ilLowerBound) ilBufPos=ilLowerBound;
      switch (ILCTRL&3) {
        case 0:
          ilFeedback0=ilFeedback1=pcm[ilBufPos];
          next=((signed char)IL0)+((pcm[ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          pcm[ilBufPos]=next;
          if (++ilBufPos>=pcmSize) ilBufPos=ilLowerBound;
          break;
        case 1:
          ilFeedback0=ilFeedback1=pcm[ilBufPos];
          next=((signed char)IL1)+((pcm[ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          pcm[ilBufPos]=next;
          if (++ilBufPos>=pcmSize) ilBufPos=ilLowerBound;
          break;
        case 2:
          ilFeedback0=ilFeedback1=pcm[ilBufPos];
          next=((signed char)IL2)+((pcm[ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          pcm[ilBufPos]=next;
          if (++ilBufPos>=pcmSize) ilBufPos=ilLowerBound;
          break;
        case 3:
          ilFeedback0=pcm[ilBufPos];
          next=((signed char)IL1)+((pcm[ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          pcm[ilBufPos]=next;
          if (++ilBufPos>=pcmSize) ilBufPos=ilLowerBound;
          ilFeedback1=pcm[ilBufPos];
          next=((signed char)IL2)+((pcm[ilBufPos]*(FIL1&15))>>4);
          if (next<-128) next=-128;
          if (next>127) next=127;
          pcm[ilBufPos]=next;
          if (++ilBufPos>=pcmSize) ilBufPos=ilLowerBound;
          break;
      }
    }
    if (ILCTRL&4) {
      if (ILSIZE&128) {
        tnsL+=ilFeedback1*(signed char)FILVOL;
        tnsR+=ilFeedback0*(signed char)FILVOL;
      } else {
        tnsL+=ilFeedback0*(signed char)FILVOL;
        tnsR+=ilFeedback1*(signed char)FILVOL;
      }
    }
  }

  if (dsOut) {
    tnsL=minval(32767,maxval(-32767,tnsL<<1));
    tnsR=minval(32767,maxval(-32767,tnsR<<1));

    short accumL=0;
    short accumR=0;

    for (int i=0; i<4; i++) {
      if ((tnsL>>8)==0 && dsCounterL>0) dsCounterL=0;
      dsCounterL+=tnsL>>8;
      if (dsCounterL>=0) {
        accumL+=4095;
        dsCounterL-=127;
      } else {
        accumL+=-4095;
        dsCounterL+=127;
      }

      if ((tnsR>>8)==0 && dsCounterR>0) dsCounterR=0;
      dsCounterR+=tnsR>>8;
      if (dsCounterR>=0) {
        accumR+=4095;
        dsCounterR-=127;
      } else {
        accumR+=-4095;
        dsCounterR+=127;
      }
    }

    *l=accumL;
    *r=accumR;
  } else {
    *l=minval(32767,maxval(-32767,tnsL));
    *r=minval(32767,maxval(-32767,tnsR));
  }
}

void SoundUnit::Init(int sampleMemSize, bool dsOutMode) {
  pcmSize=sampleMemSize;
  dsOut=dsOutMode;
  Reset();
  memset(pcm,0,pcmSize);
  for (int i=0; i<256; i++) {
    SCsine[i]=sin((i/128.0f)*M_PI)*127;
    SCtriangle[i]=(i>127)?(255-i):(i);
    SCpantabL[i]=127;
    SCpantabR[i]=127;
  }
  for (int i=0; i<128; i++) {
    SCpantabL[i]=127-i;
    SCpantabR[128+i]=i-1;
  }
  SCpantabR[128]=0;
}

void SoundUnit::Reset() {
  for (int i=0; i<8; i++) {
    ocycle[i]=0;
    cycle[i]=0;
    rcycle[i]=0;
    resetfreq[i]=0;
    voldcycles[i]=0;
    volicycles[i]=0;
    fscycles[i]=0;
    sweep[i]=0;
    ns[i]=0;
    fns[i]=0;
    nsL[i]=0;
    nsR[i]=0;
    nslow[i]=0;
    nshigh[i]=0;
    nsband[i]=0;
    swvolt[i]=1;
    swfreqt[i]=1;
    swcutt[i]=1;
    lfsr[i]=0xaaaa;
    oldfreq[i]=0;
    oldflags[i]=0;
    pcmdec[i]=0;
  }
  dsCounterL=0;
  dsCounterR=0;
  tnsL=0;
  tnsR=0;
  ilBufPos=0;
  ilBufPeriod=0;
  ilFeedback0=0;
  ilFeedback1=0;
  memset(chan,0,sizeof(SUChannel)*8);
}

#ifdef TA_BIG_ENDIAN
const unsigned char suBERemap[32]={
  0x01, 0x00, 0x02, 0x03, 0x05, 0x04, 0x07, 0x06, 0x08, 0x09, 0x0b, 0x0a, 0x0d, 0x0c, 0x0f, 0x0e,
  0x11, 0x10, 0x12, 0x13, 0x15, 0x14, 0x16, 0x17, 0x19, 0x18, 0x1a, 0x1b, 0x1c, 0x1d, 0x1f, 0x1e
};
#endif

void SoundUnit::Write(unsigned char addr, unsigned char data) {
#ifdef TA_BIG_ENDIAN
  // remap
  addr=(addr&0xe0)|(suBERemap[addr&0x1f]);
#endif
  ((unsigned char*)chan)[addr]=data;
}

SoundUnit::SoundUnit() {
  Init(65536); // default
  for (int i=0; i<8; i++) {
    muted[i]=false;
  }
}
