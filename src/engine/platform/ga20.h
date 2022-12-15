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

#ifndef _GA20_H
#define _GA20_H

#include "../dispatch.h"
#include <queue>
#include "../macroInt.h"
#include "sound/ga20/iremga20.h"

class DivPlatformGA20: public DivDispatch, public iremga20_intf {
  struct Channel: public SharedChannel<int> {
    int prevFreq;
    unsigned int audPos;
    int sample;
    bool volumeChanged, setPos;
    int resVol;
    int macroVolMul;
    Channel():
      SharedChannel<int>(255),
      prevFreq(-1),
      audPos(0),
      sample(-1),
      volumeChanged(false),
      setPos(false),
      resVol(255),
      macroVolMul(64) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    unsigned short delay;
    QueuedWrite(unsigned short a, unsigned char v, unsigned short d=1):
      addr(a),
      val(v),
      delay(d) {}
  };
  std::queue<QueuedWrite> writes;
  unsigned int sampleOffGA20[256];
  bool sampleLoaded[256];

  int delay;

  short* ga20Buf[4];
  size_t ga20BufLen;

  unsigned char* sampleMem;
  size_t sampleMemLen;
  iremga20_device ga20;
  unsigned char regPool[32];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  void chWrite(unsigned char ch, unsigned int addr, unsigned char val);
  public:
    virtual u8 read_byte(u32 address) override;
    virtual void acquire(short* bufL, short* bufR, size_t start, size_t len) override;
    virtual int dispatch(DivCommand c) override;
    virtual void* getChanState(int chan) override;
    virtual DivMacroInt* getChanMacroInt(int ch) override;
    virtual DivDispatchOscBuffer* getOscBuffer(int chan) override;
    virtual unsigned char* getRegisterPool() override;
    virtual int getRegisterPoolSize() override;
    virtual void reset() override;
    virtual void forceIns() override;
    virtual void tick(bool sysTick=true) override;
    virtual void muteChannel(int ch, bool mute) override;
    virtual bool isStereo() override;
    virtual void notifyInsChange(int ins) override;
    virtual void notifyWaveChange(int wave) override;
    virtual void notifyInsDeletion(void* ins) override;
    virtual void setFlags(const DivConfig& flags) override;
    virtual void poke(unsigned int addr, unsigned short val) override;
    virtual void poke(std::vector<DivRegWrite>& wlist) override;
    virtual const char** getRegisterSheet() override;
    virtual const void* getSampleMem(int index = 0) override;
    virtual size_t getSampleMemCapacity(int index = 0) override;
    virtual size_t getSampleMemUsage(int index = 0) override;
    virtual bool isSampleLoaded(int index, int sample) override;
    virtual void renderSamples(int chipID) override;
    virtual int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags) override;
    virtual void quit() override;
    DivPlatformGA20():
      DivDispatch(),
      iremga20_intf(),
      ga20(*this) {}
};

#endif
