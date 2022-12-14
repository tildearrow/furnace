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

#ifndef _PCSPKR_H
#define _PCSPKR_H

#include "../dispatch.h"
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

class DivPlatformPCSpeaker: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    Channel():
      SharedChannel<signed char>(15) {}
  };
  Channel chan[1];
  DivDispatchOscBuffer* oscBuf;
  std::thread* realOutThread;
  std::mutex realOutSelfLock;
  std::condition_variable realOutCond;
  bool realOutQuit;
  struct RealQueueVal {
    int tv_sec, tv_nsec;
    unsigned short val;
    RealQueueVal(int sec, int nsec, unsigned short v):
      tv_sec(sec),
      tv_nsec(nsec),
      val(v) {}
  };
  std::queue<RealQueueVal> realQueue;
  std::mutex realQueueLock;
  bool isMuted[1];
  bool on, flip, lastOn, realOutEnabled;
  int pos, speakerType, beepFD, realOutMethod;
  float low, band;
  float low2, high2, band2;
  float low3, band3;
  unsigned short freq, lastFreq;
  unsigned char regPool[2];

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void beepFreq(int freq, int delay=0);

  void acquire_unfilt(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_cone(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_piezo(short* bufL, short* bufR, size_t start, size_t len);
  void acquire_real(short* bufL, short* bufR, size_t start, size_t len);

  public:
    void pcSpeakerThread();
    void acquire(short* bufL, short* bufR, size_t start, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void notifyPlaybackStop();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformPCSpeaker();
};

#endif
