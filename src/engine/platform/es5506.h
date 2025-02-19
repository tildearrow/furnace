/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "../dispatch.h"
#include "../engine.h"
#include "../../fixedQueue.h"
#include "../macroInt.h"
#include "../sample.h"
#include "vgsound_emu/src/es550x/es5506.hpp"

class DivPlatformES5506: public DivDispatch, public es550x_intf {
  struct Channel : public SharedChannel<int> {
    struct PCM {
      bool isNoteMap;
      int index, next;
      int note;
      double freqOffs;
      double nextFreqOffs;
      bool pause, direction;
      unsigned int bank;
      unsigned int start;
      unsigned int end;
      unsigned int length;
      unsigned int loopStart;
      unsigned int loopEnd;
      unsigned int nextPos;
      bool setPos;
      DivSampleLoopMode loopMode;
      PCM():
        isNoteMap(false),
        index(-1),
        next(-1),
        note(0),
        freqOffs(1.0),
        nextFreqOffs(1.0),
        pause(false),
        direction(false),
        bank(0),
        start(0),
        end(0),
        length(0),
        loopStart(0),
        loopEnd(0),
        nextPos(0),
        setPos(false),
        loopMode(DIV_SAMPLE_LOOP_MAX) {}
    } pcm;
    int nextFreq, nextNote, currNote, wave;
    int volMacroMax, panMacroMax;
    bool useWave, isReverseLoop;
    unsigned int cr;

    struct NoteChanged { // Note changed flags
      union { // pack flag bits in single byte
        struct { // flag bits
          unsigned char offs: 1; // frequency offset
          unsigned char note: 1; // note
          unsigned char freq: 1; // base frequency
          unsigned char dummy: 5; // dummy for bit padding
        };
        unsigned char changed; // Packed flags are stored here
      };

      NoteChanged() :
        changed(0) {}
    } noteChanged;

    struct VolChanged { // Volume changed flags
      union { // pack flag bits in single byte
        struct { // flag bits
          unsigned char lVol: 1; // left volume
          unsigned char rVol: 1; // right volume
          unsigned char ca: 1; // Channel assignment
          unsigned char dummy: 5; // dummy for bit padding
        };
        unsigned char changed; // Packed flags are stored here
      };

      VolChanged() :
        changed(0) {}
    } volChanged;

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

    struct PCMChanged {
      union {
        struct {
          unsigned char index: 1; // sample index
          unsigned char slice: 1; // transwave slice
          unsigned char position: 1; // sample position in memory
          unsigned char loopBank: 1; // Loop mode and Bank
          unsigned char dummy: 4; // dummy for bit padding
        };
        unsigned char changed;
      };
      PCMChanged():
        changed(0) {}
    } pcmChanged;

    struct Overwrite {
      DivInstrumentES5506::Filter filter;
      DivInstrumentES5506::Envelope envelope;

      struct State {
        // overwrited flag
        union {
          struct {
            unsigned char mode: 1; // filter mode
            unsigned char k1: 1; // k1
            unsigned char k2: 1; // k2
            unsigned char ecount: 1; // envelope count
            unsigned char lVRamp: 1; // left volume ramp
            unsigned char rVRamp: 1; // right volume ramp
            unsigned char k1Ramp: 1; // k1 ramp
            unsigned char k2Ramp: 1; // k2 ramp
          };
          unsigned char overwrited;
        };
        State():
          overwrited(0) {}
      } state;

      Overwrite():
        filter(DivInstrumentES5506::Filter()),
        envelope(DivInstrumentES5506::Envelope()),
        state(State()) {}
    } overwrite;

    unsigned char ca;
    signed int k1Offs, k2Offs;
    signed int k1Slide, k2Slide;
    signed int k1Prev, k2Prev;
    int lVol, rVol;
    int outLVol, outRVol;
    int resLVol, resRVol;
    signed int oscOut;
    DivInstrumentES5506::Filter filter;
    DivInstrumentES5506::Envelope envelope;
    Channel():
      SharedChannel<int>(0xff),
      pcm(PCM()),
      nextFreq(0),
      nextNote(0),
      currNote(0),
      wave(-1),
      volMacroMax(0xfff),
      panMacroMax(0xfff),
      useWave(false),
      isReverseLoop(false),
      cr(0),
      noteChanged(NoteChanged()),
      volChanged(VolChanged()),
      filterChanged(FilterChanged()),
      envChanged(EnvChanged()),
      pcmChanged(PCMChanged()),
      overwrite(Overwrite()),
      ca(0),
      k1Offs(0),
      k2Offs(0),
      k1Slide(0),
      k2Slide(0),
      k1Prev(0xffff),
      k2Prev(0xffff),
      lVol(0xff),
      rVol(0xff),
      outLVol(0xfff),
      outRVol(0xfff),
      resLVol(0xfff),
      resRVol(0xfff),
      oscOut(0),
      filter(DivInstrumentES5506::Filter()),
      envelope(DivInstrumentES5506::Envelope()) {
        outVol=0xfff;
      }
  };
  Channel chan[32];
  DivDispatchOscBuffer* oscBuf[32];
  bool isMuted[32];
  signed short* sampleMem; // ES5506 uses 16 bit data bus for samples
  size_t sampleMemLen;
  unsigned int sampleOffES5506[256];
  bool sampleLoaded[256];
  struct QueuedHostIntf {
      unsigned char state;
      unsigned char step;
      unsigned char addr;
      unsigned int val;
      unsigned int mask;
      unsigned int* read;
      unsigned short delay;
      bool isRead;
      QueuedHostIntf():
        state(0),
        step(0),
        addr(0),
        val(0),
        mask(0),
        read(NULL),
        delay(0),
        isRead(false) {}
      QueuedHostIntf(unsigned char s, unsigned char a, unsigned int v, unsigned int m=(unsigned int)(~0), unsigned short d=0):
        state(0),
        step(s),
        addr(a),
        val(v),
        mask(m),
        read(NULL),
        delay(0),
        isRead(false) {}
      QueuedHostIntf(unsigned char st, unsigned char s, unsigned char a, unsigned int* r, unsigned int m=(unsigned int)(~0), unsigned short d=0):
        state(st),
        step(s),
        addr(a),
        val(0),
        mask(m),
        read(r),
        delay(d),
        isRead(true) {}
  };
  FixedQueue<QueuedHostIntf,2048> hostIntf32;
  FixedQueue<QueuedHostIntf,2048> hostIntf8;
  int cycle, curPage, volScale;
  unsigned char maskedVal;
  unsigned int irqv;
  bool isMasked, isReaded;
  bool irqTrigger, amigaVol, amigaPitch;
  unsigned int curCR;

  unsigned char initChanMax, chanMax;

  es5506_core es5506;
  DivMemoryComposition memCompo;
  unsigned char regPool[4*16*128]; // 7 bit page x 16 registers per page x 32 bit per registers

  void updateNoteChangesAsNeeded(int ch);

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    virtual void e_pin(bool state) override;     // E output
    virtual void irqb(bool state) override; // IRQB output
    virtual s16 read_sample(u8 bank, u32 address) override {
      if (sampleMem==NULL) return 0;
      return sampleMem[((bank&3)<<21)|(address&0x1fffff)];
    }

    virtual void acquire(short** buf, size_t len) override;
    virtual int dispatch(DivCommand c) override;
    virtual void* getChanState(int chan) override;
    virtual DivMacroInt* getChanMacroInt(int ch) override;
    virtual unsigned short getPan(int chan) override;
    virtual DivDispatchOscBuffer* getOscBuffer(int chan) override;
    virtual unsigned char* getRegisterPool() override;
    virtual int getRegisterPoolSize() override;
    virtual void reset() override;
    virtual void forceIns() override;
    virtual void tick(bool sysTick=true) override;
    virtual void muteChannel(int ch, bool mute) override;
    virtual int getOutputCount() override;
    virtual bool keyOffAffectsArp(int ch) override;
    virtual void setFlags(const DivConfig& flags) override;
    virtual void notifyInsChange(int ins) override;
    virtual void notifyWaveChange(int wave) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual const void* getSampleMem(int index = 0) override;
    virtual size_t getSampleMemCapacity(int index = 0) override;
    virtual size_t getSampleMemUsage(int index = 0) override;
    virtual bool isSampleLoaded(int index, int sample) override;
    virtual const DivMemoryComposition* getMemCompo(int index) override;
    virtual void renderSamples(int sysID) override;
    virtual const char** getRegisterSheet() override;
    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) override;
    virtual void quit() override;
    DivPlatformES5506():
      DivDispatch(),
      es550x_intf(),
      es5506(*this) {}
};

#endif
