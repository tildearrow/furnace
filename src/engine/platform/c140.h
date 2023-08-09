/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

#ifndef _C140_H
#define _C140_H

#include "../dispatch.h"
#include "sound/c140.h"
#include "../fixedQueue.h"

class DivPlatformC140: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    struct VolumeUpdate {
      union { // pack flag bits in single byte
        struct { // flag bits
          unsigned char left : 1;  // left volume
          unsigned char right : 1; // right volume
          unsigned char reserved : 6;
        };
        unsigned char changed = 0; // Packed flags are stored here
      };

      VolumeUpdate():
        changed(0) {}
    } volumeChanged;

    unsigned int audPos;
    int sample, wave;
    bool setPos;
    int chPanL, chPanR;
    int chVolL, chVolR;
    int macroVolMul;
    Channel():
      SharedChannel<int>(255),
      volumeChanged(VolumeUpdate()),
      audPos(0),
      sample(-1),
      wave(-1),
      setPos(false),
      chPanL(255),
      chPanR(255),
      chVolL(255),
      chVolR(255),
      macroVolMul(64) {}
  };
  Channel chan[24];
  DivDispatchOscBuffer* oscBuf[24];
  bool isMuted[24];
  int chipType;
  unsigned int sampleOff[256];
  bool sampleLoaded[256];

  signed short* sampleMem;
  size_t sampleMemLen;
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    bool addrOrVal;
    QueuedWrite(): addr(0), val(0), addrOrVal(false) {}
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v), addrOrVal(false) {}
  };
  FixedQueue<QueuedWrite,2048> writes;
  struct c140_t c140;
  unsigned char regPool[512];
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquire(short** buf, size_t len);
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
    int getOutputCount();
    void setChipModel(int type);
    void notifyInsChange(int ins);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    const void* getSampleMem(int index = 0);
    size_t getSampleMemCapacity(int index = 0);
    size_t getSampleMemUsage(int index = 0);
    bool isSampleLoaded(int index, int sample);
    void renderSamples(int chipID);
    void setFlags(const DivConfig& flags);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
  private:
};

#endif
