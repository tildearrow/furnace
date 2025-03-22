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

#ifndef _SWAN_H
#define _SWAN_H

#include "../dispatch.h"
#include "../waveSynth.h"
#include "sound/swan.h"
#include "sound/swan_mdfn.h"
#include "../../fixedQueue.h"

class DivPlatformSwan: public DivDispatch {
  struct Channel: public SharedChannel<int> {
    unsigned char pan;
    int wave;
    DivWaveSynth ws;
    Channel():
      SharedChannel<int>(15),
      pan(255),
      wave(-1) {}
  };
  Channel chan[4];
  DivDispatchOscBuffer* oscBuf[4];
  bool isMuted[4];
  bool stereo;
  bool useMdfn;
  bool pcm, sweep, furnaceDac, setPos;
  unsigned char sampleBank, noise;
  int dacPeriod, dacRate;
  unsigned int dacPos;
  int dacSample;

  unsigned char regPool[0x80];
  struct QueuedWrite {
    unsigned char addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned char a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,256> writes;
  FixedQueue<DivRegWrite,2048> postDACWrites;

  swan_sound_t ws;
  WSwan* ws_mdfn;
  
  void updateWave(int ch);
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquireDirect(blip_buffer_t** bb, size_t len);
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    unsigned short getPan(int chan);
    DivChannelModeHints getModeHints(int chan);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    void setFlags(const DivConfig& flags);
    void notifyWaveChange(int wave);
    void notifyInsDeletion(void* ins);
    int getOutputCount();
    bool hasAcquireDirect();
    void setUseMdfn(bool use);
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformSwan();
  private:
    void calcAndWriteOutVol(int ch, int env);
    void writeOutVol(int ch);
};

#endif
