/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _FMSHARED_OPN_H
#define _FMSHARED_OPN_H

#include "fmsharedbase.h"
#include "../../../extern/opn/ym3438.h"
#include "sound/ymfm/ymfm_opn.h"

#define PLEASE_HELP_ME(_targetChan,blk) \
  int boundaryBottom=parent->calcBaseFreq(chipClock,CHIP_FREQBASE,0,false); \
  int boundaryTop=parent->calcBaseFreq(chipClock,CHIP_FREQBASE,12,false); \
  int destFreq=NOTE_FNUM_BLOCK(c.value2,11,blk); \
  int newFreq; \
  bool return2=false; \
  if (_targetChan.portaPause) { \
    if (parent->song.oldOctaveBoundary) { \
      if ((_targetChan.portaPauseFreq&0xf800)>(_targetChan.baseFreq&0xf800)) { \
        _targetChan.baseFreq=((_targetChan.baseFreq&0x7ff)>>1)|(_targetChan.portaPauseFreq&0xf800); \
      } else { \
        _targetChan.baseFreq=((_targetChan.baseFreq&0x7ff)<<1)|(_targetChan.portaPauseFreq&0xf800); \
      } \
      c.value*=2; \
    } else { \
      _targetChan.baseFreq=_targetChan.portaPauseFreq; \
    } \
  } \
  if (destFreq>_targetChan.baseFreq) { \
    newFreq=_targetChan.baseFreq+c.value; \
    if (newFreq>=destFreq) { \
      newFreq=destFreq; \
      return2=true; \
    } \
  } else { \
    newFreq=_targetChan.baseFreq-c.value; \
    if (newFreq<=destFreq) { \
      newFreq=destFreq; \
      return2=true; \
    } \
  } \
  /* check for octave boundary */ \
  /* what the heck! */ \
  if (!_targetChan.portaPause) { \
    if ((newFreq&0x7ff)>boundaryTop && (newFreq&0xf800)<0x3800) { \
      if (parent->song.fbPortaPause) { \
        _targetChan.portaPauseFreq=(boundaryBottom)|((newFreq+0x800)&0xf800); \
        _targetChan.portaPause=true; \
        break; \
      } else { \
        newFreq=((newFreq&0x7ff)>>1)|((newFreq+0x800)&0xf800); \
      } \
    } \
    if ((newFreq&0x7ff)<boundaryBottom && (newFreq&0xf800)>0) { \
      if (parent->song.fbPortaPause) { \
        _targetChan.portaPauseFreq=newFreq=(boundaryTop-1)|((newFreq-0x800)&0xf800); \
        _targetChan.portaPause=true; \
        break; \
      } else { \
        newFreq=((newFreq&0x7ff)<<1)|((newFreq-0x800)&0xf800); \
      } \
    } \
  } \
  _targetChan.portaPause=false; \
  _targetChan.freqChanged=true; \
  _targetChan.baseFreq=newFreq; \
  if (return2) { \
    _targetChan.inPorta=false; \
    return 2; \
  }

#define IS_EXTCH_MUTED (isOpMuted[0] && isOpMuted[1] && isOpMuted[2] && isOpMuted[3])

class DivOPNInterface: public ymfm::ymfm_interface {
  int setA, setB;
  int countA, countB;

  public:
    void clock(int cycles=144);
    void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks);
    DivOPNInterface():
      ymfm::ymfm_interface(),
      countA(0),
      countB(0) {}
};

class DivPlatformOPN: public DivPlatformFMBase {
  protected:
    const unsigned short ADDR_MULT_DT=0x30;
    const unsigned short ADDR_TL=0x40;
    const unsigned short ADDR_RS_AR=0x50;
    const unsigned short ADDR_AM_DR=0x60;
    const unsigned short ADDR_DT2_D2R=0x70;
    const unsigned short ADDR_SL_RR=0x80;
    const unsigned short ADDR_SSG=0x90;
    const unsigned short ADDR_FREQ=0xa0;
    const unsigned short ADDR_FREQH=0xa4;
    const unsigned short ADDR_FB_ALG=0xb0;
    const unsigned short ADDR_LRAF=0xb4;

    const unsigned short opOffs[4]={
      0x00, 0x04, 0x08, 0x0c
    };

    struct OPNChannel: public FMChannel {
      unsigned char psgMode, autoEnvNum, autoEnvDen;
      bool furnacePCM;
      int sample, macroVolMul;

      OPNChannel():
        FMChannel(),
        psgMode(1),
        autoEnvNum(0),
        autoEnvDen(0),
        furnacePCM(false),
        sample(-1),
        macroVolMul(255) {}
    };

    struct OPNChannelStereo: public OPNChannel {
      unsigned char pan;
      OPNChannelStereo():
        OPNChannel(),
        pan(3) {}
    };

    struct OPNOpChannel: public SharedChannel<int> {
      unsigned char freqH, freqL;
      int portaPauseFreq;
      signed char konCycles;
      bool mask, hardReset;
      OPNOpChannel():
        SharedChannel<int>(0),
        freqH(0),
        freqL(0),
        portaPauseFreq(0),
        konCycles(0),
        mask(true),
        hardReset(false) {}
    };

    struct OPNOpChannelStereo: public OPNOpChannel {
    unsigned char pan;
      OPNOpChannelStereo():
        OPNOpChannel(),
        pan(3) {}
    };

    int extChanOffs, psgChanOffs, adpcmAChanOffs, adpcmBChanOffs, chanNum; // i really wanted to keep this constant...

    double fmFreqBase;
    unsigned int fmDivBase;
    unsigned int ayDiv;
    unsigned char csmChan;
    unsigned char lfoValue;
    unsigned char lastExtChPan;
    unsigned short ssgVol;
    unsigned short fmVol;
    bool extSys, fbAllOps;
    unsigned char useCombo;

    DivConfig ayFlags;

    friend void putDispatchChip(void*,int);
    friend void putDispatchChan(void*,int,int);
    DivPlatformOPN(int ext, int psg, int adpcmA, int adpcmB, int chanCount, double f=9440540.0, unsigned int d=72, unsigned int a=32, bool isExtSys=false, unsigned char cc=255):
      DivPlatformFMBase(),
      extChanOffs(ext),
      psgChanOffs(psg),
      adpcmAChanOffs(adpcmA),
      adpcmBChanOffs(adpcmB),
      chanNum(chanCount),
      fmFreqBase(f),
      fmDivBase(d),
      ayDiv(a),
      csmChan(cc),
      lfoValue(0),
      lastExtChPan(3),
      ssgVol(128),
      fmVol(256),
      extSys(isExtSys),
      fbAllOps(false),
      useCombo(0) {}
  public:
    void setCombo(unsigned char combo) {
      useCombo=combo;
    }
    virtual int mapVelocity(int ch, float vel) {
      if (ch==csmChan) return vel*127.0;
      if (ch==adpcmBChanOffs) return vel*255.0;
      if (ch>=adpcmAChanOffs) {
        if (vel==0) return 0;
        if (vel>=1.0) return 31;
        return CLAMP(round(32.0-(56.0-log2(vel*127.0)*8.0)),0,31);
      }
      if (ch>=psgChanOffs) return round(15.0*pow(vel,0.33));
      return DivPlatformFMBase::mapVelocity(ch,vel);
    }
    virtual float getGain(int ch, int vol) {
      if (vol==0) return 0;
      if (ch==csmChan) return 1;
      if (ch==adpcmBChanOffs) return (float)vol/255.0;
      if (ch>=adpcmAChanOffs) {
        return 1.0/pow(10.0,(float)(31-vol)*0.75/20.0);
      }
      if (ch>=psgChanOffs) {
        return 1.0/pow(10.0,(float)(15-vol)*1.5/20.0);
      }
      return DivPlatformFMBase::getGain(ch,vol);
    }

};

#endif
