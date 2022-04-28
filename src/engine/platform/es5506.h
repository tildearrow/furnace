/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2022 tildearrow and contributors
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

#ifndef _ES5506_H
#define _ES5506_H

#pragma once

#include "../dispatch.h"
#include "../engine.h"
#include <queue>
#include "../macroInt.h"
#include "../sample.h"
#include "sound/es550x/es5506.hpp"

class DivPlatformES5506: public DivDispatch, public es550x_intf {
  struct Channel {
    struct PCM {
      int index;
      double freqOffs;
      bool reversed;
      unsigned int bank;
      unsigned int start;
      unsigned int end;
      unsigned int length;
      unsigned int loopStart;
      unsigned int loopEnd;
      DivSampleLoopMode loopMode;
      PCM():
        index(-1),
        freqOffs(1.0),
        reversed(false),
        bank(0),
        start(0),
        end(0),
        length(0),
        loopStart(0),
        loopEnd(0),
        loopMode(DIV_SAMPLE_LOOPMODE_ONESHOT) {}
    } pcm;
    int freq, baseFreq, pitch, pitch2, note, ins, sample, wave;
    bool active, insChanged, freqChanged, volChanged, keyOn, keyOff, inPorta, useWave, isReverseLoop;

    struct FilterChanged { // Filter changed flags
      union { // pack flag bits in single byte
        struct { // flag bits
          unsigned char mode: 1; // Filter mode
          unsigned char k1: 1; // K1
          unsigned char k2: 1; // K2
          unsigned char dummy: 5; // dummy for bit padding
        };
        unsigned char changed; // Packed flags are stored here
      };

      FilterChanged():
        changed(0) {}
    } filterChanged;

    struct EnvChanged { // Envelope changed flags
      union { // pack flag bits in single byte
        struct { // flag bits
          unsigned char ecount: 1; // Envelope count
          unsigned char lVRamp: 1; // Left volume Ramp
          unsigned char rVRamp: 1; // Right volume Ramp
          unsigned char k1Ramp: 1; // K1 Ramp w/Slow flag
          unsigned char k2Ramp: 1; // K2 Ramp w/Slow flag
          unsigned char dummy: 3; // dummy for bit padding
        };
        unsigned char changed; // Packed flags are stored here
      };

      EnvChanged():
        changed(0) {}
    } envChanged;

    signed int k1Offs, k2Offs;
    int vol, lVol, rVol;
    int outVol, outLVol, outRVol;
    int resLVol, resRVol;
    DivInstrumentES5506::Filter filter;
    DivInstrumentES5506::Envelope envelope;
    DivMacroInt std;
    void macroInit(DivInstrument* which) {
      std.init(which);
      pitch2=0;
      if (std.ex1.mode==2) {
        k1Offs=0;
      }
      if (std.ex1.mode==2) {
        k2Offs=0;
      }
    }
    Channel():
      freq(0),
      baseFreq(0),
      pitch(0),
      pitch2(0),
      note(0),
      ins(-1),
      sample(-1),
      wave(-1),
      active(false),
      insChanged(true),
      freqChanged(false),
      volChanged(false),
      keyOn(false),
      keyOff(false),
      inPorta(false),
      useWave(false),
      isReverseLoop(false),
      k1Offs(0),
      k2Offs(0),
      vol(0xff),
      lVol(0xf),
      rVol(0xf),
      outVol(0xffff),
      outLVol(0xffff),
      outRVol(0xffff),
      resLVol(0xffff),
      resRVol(0xffff) {}
  };
  Channel chan[32];
  bool isMuted[32];
  struct QueuedHostIntf {
      unsigned char step;
      unsigned char addr;
      unsigned int val;
      unsigned int mask;
      unsigned int* read;
      unsigned short delay;
      bool isRead;
      QueuedHostIntf(unsigned char s, unsigned char a, unsigned int v, unsigned int m=(unsigned int)(~0), unsigned short d=0):
        step(s),
        addr(a),
        val(v),
        mask(m),
        read(NULL),
        delay(0),
        isRead(false) {}
      QueuedHostIntf(unsigned char s, unsigned char a, unsigned int* r, unsigned int m=(unsigned int)(~0), unsigned short d=0):
        step(s),
        addr(a),
        val(0),
        mask(m),
        read(r),
        delay(d),
        isRead(true) {}
  };
  std::queue<QueuedHostIntf> hostIntf32;
  std::queue<QueuedHostIntf> hostIntf8;
  int cycle, curPage;
  unsigned char maskedVal;
  unsigned int irqv;
  bool isMasked, isReaded;
  bool irqTrigger;

  unsigned char initChanMax, chanMax;

  es5506_core es5506;
  unsigned char regPool[4*16*128]; // 7 bit page x 16 registers per page x 32 bit per registers

  friend void putDispatchChan(void*,int,int);

  public:
	  virtual void e_pin(bool state) override;     // E output

	  virtual void irqb(bool state) override; // IRQB output
	  virtual s16 read_sample(u8 voice, u8 bank, u32 address) override {
      if (parent->es5506Mem==NULL) return 0;
      return parent->es5506Mem[((bank&3)<<21)|(address&0x1fffff)];
    }

    virtual void acquire(short* bufL, short* bufR, size_t start, size_t len) override;
    virtual int dispatch(DivCommand c) override;
    virtual void* getChanState(int chan) override;
    virtual unsigned char* getRegisterPool() override;
    virtual int getRegisterPoolSize() override;
    virtual void reset() override;
    virtual void forceIns() override;
    virtual void tick(bool sysTick=true) override;
    virtual void muteChannel(int ch, bool mute) override;
    virtual bool isStereo() override;
    virtual bool keyOffAffectsArp(int ch) override;
    virtual void setFlags(unsigned int flags) override;
    virtual void notifyInsChange(int ins) override;
    virtual void notifyWaveChange(int wave) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual const char** getRegisterSheet() override;
    virtual const char* getEffectName(unsigned char effect) override;
    virtual int init(DivEngine* parent, int channels, int sugRate, unsigned int flags) override;
    virtual void quit() override;
    DivPlatformES5506():
      DivDispatch(),
      es550x_intf(),
      es5506(*this) {}
};

#endif
