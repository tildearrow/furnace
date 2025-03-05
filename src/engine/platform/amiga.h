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

#ifndef _AMIGA_H
#define _AMIGA_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "../waveSynth.h"

class DivPlatformAmiga: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    unsigned short audLen, irLocL, irLocH, irLen;
    unsigned int audPos;
    int audSub;
    unsigned char volPos;
    int sample, wave;
    bool useWave, setPos, useV, useP, dmaOn, audDatClock, writeVol, updateWave;
    DivWaveSynth ws;
    Channel():
      SharedChannel<signed char>(64),
      audLen(0),
      irLocL(0),
      irLocH(0),
      irLen(2),
      audPos(0),
      audSub(0),
      volPos(0),
      sample(-1),
      wave(-1),
      useWave(false),
      setPos(false),
      useV(false),
      useP(false),
      dmaOn(false),
      audDatClock(false),
      writeVol(true),
      updateWave(true) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  bool bypassLimits;
  bool amigaModel;
  bool filterOn;
  bool updateADKCon;
  short delay;
  short oldOut[2];

  struct Amiga {
    // register state
    bool audInt[4]; // interrupt on
    bool audIr[4]; // interrupt request
    bool audEn[4]; // audio DMA on
    bool mustDMA[4]; // audio DMA must run
    bool useP[4]; // period modulation
    bool useV[4]; // volume modulation

    bool dmaEn;

    unsigned int audLoc[4]; // address
    unsigned short audLen[4]; // length
    unsigned short audPer[4]; // period
    unsigned char audVol[4]; // volume
    signed char audDat[2][4]; // data
    signed char nextOut[4];
    unsigned short nextOut2[4];
    

    // internal state
    int audTick[4]; // tick of period
    unsigned int dmaLoc[4]; // address
    unsigned short dmaLen[4]; // position

    bool audByte[4]; // which byte of audDat to output
    bool audWord[4]; // for P/V
    bool incLoc[4]; // whether dmaLoc/dmaLen should be updated
    unsigned char volPos; // position of volume PWM
    unsigned short hPos; // horizontal position of beam
    unsigned char state[4]; // current channel state

    void write(unsigned short addr, unsigned short val);

    Amiga() {
      memset(this,0,sizeof(*this));
    }
  } amiga;

  int filter[2][4];
  int filtConst;
  int filtConstOff, filtConstOn;
  int chipMem, chipMask;

  unsigned char volTable[64][64];

  unsigned int sampleOff[256];
  bool sampleLoaded[256];

  unsigned short regPool[256];

  DivMemoryComposition memCompo;

  unsigned char* sampleMem;
  size_t sampleMemLen;

  int sep1, sep2;

  struct QueuedWrite {
    unsigned short addr;
    unsigned short val;
    QueuedWrite(): addr(0), val(9) {}
    QueuedWrite(unsigned short a, unsigned short v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,512> writes;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  friend class DivExportAmigaValidation;

  void irq(int ch);
  void rWrite(unsigned short addr, unsigned short val);
  void updateWave(int ch);

  public:
    void acquire(short** buf, size_t len);
    void acquireDirect(blip_buffer_t** bb, size_t len);
    void postProcess(short* buf, int outIndex, size_t len, int sampleRate);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    int getRegisterPoolDepth();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool keyOffAffectsArp(int ch);
    bool hasAcquireDirect();
    DivMacroInt* getChanMacroInt(int ch);
    DivSamplePos getSamplePos(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void renderSamples(int chipID);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index=0);
    size_t getSampleMemCapacity(int index=0);
    size_t getSampleMemUsage(int index=0);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
};

#endif
