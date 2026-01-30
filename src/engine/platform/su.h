/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#ifndef _SU_H
#define _SU_H

#include "../dispatch.h"
#include "../../fixedQueue.h"
#include "sound/su.h"

class DivPlatformSoundUnit: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    int cutoff, baseCutoff, res, control, hasOffset, sample;
    signed char pan;
    unsigned char duty;
    bool noise, pcm, phaseReset, filterPhaseReset, switchRoles;
    bool pcmLoop, timerSync, freqSweep, volSweep, cutSweep, released;
    unsigned short freqSweepP, volSweepP, cutSweepP;
    unsigned char freqSweepB, volSweepB, cutSweepB;
    unsigned char freqSweepV, volSweepV, cutSweepV;
    unsigned short syncTimer;
    signed short wave;
    unsigned short hwSeqPos;
    short hwSeqDelay;
    short cutoff_slide;
    short pw_slide;
    short virtual_duty;
    Channel():
      SharedChannel<signed char>(127),
      cutoff(16383),
      baseCutoff(16380),
      res(0),
      control(0),
      hasOffset(0),
      sample(-1),
      pan(0),
      duty(63),
      noise(false),
      pcm(false),
      phaseReset(false),
      filterPhaseReset(false),
      switchRoles(false),
      pcmLoop(false),
      timerSync(false),
      freqSweep(false),
      volSweep(false),
      cutSweep(false),
      released(false),
      freqSweepP(0),
      volSweepP(0),
      cutSweepP(0),
      freqSweepB(0),
      volSweepB(0),
      cutSweepB(0),
      freqSweepV(0),
      volSweepV(0),
      cutSweepV(0),
      syncTimer(0),
      wave(0),
      hwSeqPos(0),
      hwSeqDelay(0),
      cutoff_slide(0),
      pw_slide(0),
      virtual_duty(0) {}
  };
  Channel chan[8];
  DivDispatchOscBuffer* oscBuf[8];
  bool isMuted[8];
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,512> writes;
  unsigned char lastPan;
  bool sampleMemSize;
  unsigned char ilCtrl, ilSize, fil1;
  unsigned char initIlCtrl, initIlSize, initFil1;
  signed char echoVol, initEchoVol;
  unsigned int* sampleOffSU;
  bool* sampleLoaded;

  int cycles, curChan, delay, sysIDCache;
  short tempL;
  short tempR;
  unsigned char lfoMode, lfoSpeed;
  SoundUnit* su;
  unsigned char* sampleMem;
  size_t sampleMemLen;
  unsigned char regPool[128];
  DivMemoryComposition memCompo;
  double NOTE_SU(int ch, int note);
  void writeControl(int ch);
  void writeControlUpper(int ch);

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    int getOutputCount();
    bool hasSoftPan(int ch);
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index);
    size_t getSampleMemCapacity(int index);
    size_t getSampleMemUsage(int index);
    bool isSampleLoaded(int index, int sample);
    const DivMemoryComposition* getMemCompo(int index);
    void renderSamples(int chipID);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    DivPlatformSoundUnit();
    ~DivPlatformSoundUnit();
};

#endif
